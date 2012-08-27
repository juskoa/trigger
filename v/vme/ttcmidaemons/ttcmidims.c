#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#ifdef CPLUSPLUS
#include <dis.hxx>
#else
#include <dis.h>
#endif
#include "infolog.h"
#include "vmewrap.h"
#include "udplib.h"
#include "vmeblib.h"
#include "../ctp/ctplib/ctplib.h"
#include "../ttcmi/ttcmi.h"

#define MAXCMDL 200
#define MAXLILE 20
#define MYNAME "TTCMI"

#define MICLOCKtag 1 
#define MICLOCK_TRANSITIONtag 2 
#define MICLOCK_SETtag 3 
#define SHIFTtag 4 
#define CORDE_SETtag 5 
#define QPLLtag 6 
#define DLL_RESYNCtag 7 
int MICLOCKid;
int SHIFTid,QPLLid;
int MICLOCK_TRANSITIONid;

int quit=0; 
int udpsock;
int clocktag=0;   // in agreement with clocknow
int newclocktag;  /* has to be here (thread parameter)
1..4 -clock change event
0    - DLL_RESYNC event
*/
char clocknow[MAXLILE+1]="none";  // BEAM1/2 REF LOCAL
char shiftnow[MAXLILE+1]="none";  // halfsecs corde_val
char qpllnow[MAXLILE+1]="none";  // T1122RRMM hexa (qpllstat binary)
// T: TTCrx ready   i.e. 1 ok
// BC1/2/Ref/main:  Error Locked, i.e. 01 ok
int clocktran=0; char clocktransition[MAXLILE+1]="0";

int TAGqpll_thread=88;
int nlogqpll=0;
w32 qpllstat=0; // in agreement with qpllnow

/*--------------------------------------------------------------- error_handler
A severity code: 0 - info, 1 - warning, 2 - error, 3 - fatal.
*/
void error_handler(int severity, int error_code, char *message) {
char msg1[100];
char *sev[5]={"info", "warning", "error", "fatal", "???"};
if((severity<0) || (severity>3)) {
  severity=4;
};
sprintf(msg1,"*** DIM %s: %d", sev[severity], error_code);
prtLog(msg1); prtLog(message);
}
/*--------------------------------------------------------------- exit_handler
*/
void exit_handler(int *exitcode) {
char msg1[100];
sprintf(msg1,"exit_handler exitcode:%d", *exitcode);
prtLog(msg1);
}

/*-------------------------------------------------------------- ds_stop
*/
void ds_stop() {
#define EXITSERVER 1
if(EXITSERVER==1) {
  quit=1;   // stop thread reading ctp counters
  printf("qq...\n");
  dis_remove_service(MICLOCKid);
  dis_remove_service(SHIFTid);
  dis_remove_service(QPLLid);
  dis_remove_service(MICLOCK_TRANSITIONid);
  dis_stop_serving();
  if(micratepresent()) vmeclose();
  exit(0);
};
}
void gotsignal(int signum) {
char msg[100];
switch(signum) {
case SIGUSR1:  // kill -s USR1 pid
  signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
  sprintf(msg, "got SIGUSR1 signal:%d, fflush(stdout)\n", signum);
  prtLog(msg);
  //fflush(stdout);
  break;
case SIGINT:
  signal(SIGINT, gotsignal); siginterrupt(SIGINT, 0);
  printf("got SIGINT signal, quitting...:%d\n", signum);
  quit=signum;
  break;
case SIGQUIT:
  signal(SIGQUIT, gotsignal); siginterrupt(SIGQUIT, 0);
  printf("got SIGQUIT signal, quitting...:%d\n", signum);
  quit=signum;
  break;
case SIGBUS: 
  //vmeish(); not to be called (if called, it kills dims process)
  sprintf(msg, "got SIGBUS signal:%d\n", signum); prtLog(msg);
  break; 
default:
  printf("got unknown signal:%d\n", signum);
};
}

w32 bcmvme=0, omvme=2;
extern w32 cordevalvme;
extern w32 halfnsvme;

/*-------------------------------------------------------*/ void getclocknow() {
w32 bcm,om;
if(micratepresent()) {
  bcm= vmer32(BCmain_MAN_SELECT); om= vmer32(ORBmain_MAN_SELECT);
} else {
  bcm=bcmvme; om=omvme;
  //printf("novme vmer: bcm:%x om:%x\n", bcm, om);
}
if((bcm==3) && (om==0)) { strcpy(clocknow,"BEAM1"); clocktag=1;
} else if((bcm==2) && (om==1)) { strcpy(clocknow,"BEAM2"); clocktag=2;
} else if((bcm==1) && (om==2)) {strcpy(clocknow,"REF"); clocktag=3;
} else if((bcm==0) && (om==2)) {strcpy(clocknow,"LOCAL"); clocktag=4;
} else { strcpy(clocknow,"unknown"); clocktag=0;
};
}
/*-------------------------------------------------------*/ void getshiftnow() {
w32 halfns,cordeval;
//if(micratepresent()) {
  halfns= i2cread_delay(BC_DELAY25_BCMAIN)-0x140;
  cordeval= corde_get(CORDE_DELREG);
  sprintf(shiftnow,"%d %d", halfns, cordeval);
//} else {
//  halfns= halfnsvme; cordeval= cordevalvme;
//  sprintf(shiftnow,"%d %d", halfns, cordeval);
//  printf("novme shiftnow:%s\n", shiftnow);
//};
}
/*Input: tag: 1:BEAM1 2:BEAM2 3:REF 4:LOCAL
rc: clock set (i.e. tag, or 0: error). if rc!=tag required clock
    was not set.
---------------------------------------*/ int setbcorbit(int tag)  {
int rc;
char buffer[50];
printf("setting ttcmi clock:%d\n", tag);
if(micratepresent()==0) {
  if(tag==1) {bcmvme=3; omvme=0; 
  } else if(tag==2) {bcmvme=2; omvme=1; 
  } else if(tag==3) {bcmvme=1; omvme=2; 
  } else if(tag==4) {bcmvme=0; omvme=2; 
  } else {
    printf("novme setbcorbit: bad tag:%d bcmvme:%x omvme:%x\n", tag, bcmvme, omvme);
  return(0);
  };
} else {
 setbcorbitMain(tag);
};
rc= dis_update_service(SHIFTid);
printf("TTCMI/SHIFT update for %d clients\n", rc);  
sprintf(buffer, "mon ds005 N:%d", tag); 
rc= udpsend(udpsock, (unsigned char *)buffer, strlen(buffer)+1);
getclocknow();
return(tag);
}
/*-----------------------------------------------*/ void newclock(void *tag) {
/* this thread started with 2 events( only 1 available in one time):
- clock change
- DLL_RESYNC
*/
printf("newclock thread started. clocktran:%d tag:%d quit:%d\n", 
  clocktran, *(int *)tag, quit); fflush(stdout);
while(clocktran>=0) {
  int nclients;
  nclients= dis_update_service(MICLOCK_TRANSITIONid);
  printf("updated MICLOCK_TRANSITION clients:%d clocktran:%d\n", nclients, clocktran);
  if(clocktran==0) break;
  if(micratepresent()) {
    dtq_sleep(30);   // was 60 before 10.11.2011
  } else {
    dtq_sleep(5);   // in lab just 5secs
  };
  clocktran--; sprintf(clocktransition,"%d", clocktran);
  if(clocktran==0) {
    if(*(int *)tag==0) {
      int rc;
      char cmd[]="$VMECFDIR/ttcmidaemons/sctel.py MININF";
      DLL_RESYNC(DLL_info);
      //printf("DLL_RESYNC + clearing the scope persistance\n");
      rc= system(cmd);
    } else {
      setbcorbit(*(int *)tag); 
      nclients= dis_update_service(MICLOCKid);
      printf("updated MICLOCK clients:%d\n", nclients);
    };
  };
  if(quit==1) clocktran=0;
};
}
int authenticate(char *subdir) {   // "" or "oerjan/"
/* Only client which wrote its pid to server's 
VMEWORKDIR/WORK/miclockid file is allowed to change the clock
rc: 0:ok  !=0 -client is not allowed to change the clock
*/
int ix,rc;
FILE *con;
char *envwd;
char procid[80]; char fname[80]; char line[80]=""; char hname[31]="";
envwd= getenv("VMEWORKDIR"); sprintf(fname, "%s/WORK/%smiclockid",envwd,subdir);
rc= dis_get_client(procid);
for(ix=0; ix<80; ix++) {
  if(procid[ix]=='@') {
    procid[ix]='\0';
    strncpy(hname, &procid[ix+1],30); hname[30]='\0';
    break;
  };
};
if(( strncmp(hname,"alidcscom188",12)==0) 
   || ( strncmp(hname,"pcalicebhm10",12)==0)) {
  rc=0; goto OK;    // clock shift also from pydimserver!
};
if((con=fopen(fname,"r")) == NULL){
  printf("Cannot read %s file. \n", fname);
  return(1);
};
if(fgets(line, 80, con)==NULL) {fclose(con); return(2);};
fclose(con);
for(ix=0; ix<80; ix++) {
  if(line[ix]=='\n') line[ix]='\0';
};
if(strcmp(procid,line)==0) {
  rc=0;
} else {
  rc=3;
};
OK:
//printf("procid:%s wd:%s line:%s< hname:%s rc:%d\n", procid, envwd,line, hname, rc);
fflush(stdout);
return(rc);
}
/*-----------------*/ void CORDE_SETcmd(void *tag, void *msgv, int *size)  {
char errmsg[200];
char *msg= (char *)msgv; int rc; w32 cosh;
char sshift[20]; int shift=0, origshift; 
if(*size>19) {rc=19;}
else {rc=*size; };
strncpy(sshift,msg,rc); sshift[rc]='\0';
errno= 0;
shift= strtol(sshift, (char **)NULL, 10);
if ((errno == ERANGE && (shift == LONG_MAX || shift == LONG_MIN))
     || (errno != 0 && shift == 0)) {
  sprintf(errmsg, "Error: incorrect shift:%s, not set", sshift); prtLog(errmsg); 
  return;  
};
sprintf(errmsg, "CORDE_SETcmd: size:%d msg:%5.5s :%s:%d\n", 
  *size, msg, sshift, shift); prtLog(errmsg); 
//rc= authenticate();
rc=0; //prtLog("CORDE_SET not authenticated!\n");
if(rc!=0) {
  sprintf(errmsg, "Only miclock can change the CORDE shift\n"); prtLog(errmsg); 
  return;  
};
if(shift==0) {
  sprintf(errmsg, "Error: Bad shift:%s, not set", sshift); prtLog(errmsg); 
  return;  
};
cosh= corde_shift(CORDE_DELREG, shift, &origshift);
//sprintf(errmsg, "corde_shift(,%d, %d)", shift, origshift); prtLog(errmsg); 
if(cosh>1023) {
  sprintf(errmsg, "Error: Corde reg. not set, corde_shift rc:0x%x\n", cosh); prtLog(errmsg); 
} else {
  w32 pol,halfns; int rcdl; char line[80];
  // always, after shift resynchronize DLL on RF2TTC:
  // not here (called from miclock):
  //DLL_RESYNC(0);
  sprintf(errmsg, "corde_shift(%x, %d, ) orig:%d set to:%d.",
    CORDE_DELREG, shift, origshift, cosh); 
  prtLog(errmsg); 
  sprintf(errmsg,"CORDE shift: %d -> %d ps",origshift*10, cosh*10);
  infolog_trg(LOG_INFO, errmsg);
  // update $dbctp/clockshift,  daqlogbook and SHIFTid service:
  // not done here (can we use DIM client library here?), but in miclock.py
  // which is not correct (it can get unsync!)
  //rc= dic_cmnd_callback("CTPRCFG/RCFG", message, strlen(message)+1, callback, 33);
  rc= dis_update_service(SHIFTid);
  sprintf(errmsg,"TTCMI/SHIFT updated for %d clients\n", rc); prtLog(errmsg); 
  pol= i2cread_delay(BC_DELAY25_BCMAIN); halfns= pol-0x140;
  // update $dbctp/clockshift
  sprintf(line, "%d %d %d", halfns, cosh, origshift);
  writedbfile("/home/alice/trigger/v/vme/CFG/clockshift", line);
  //
  rcdl=daqlogbook_open();   //rcdl must be 0 if opened
  rcdl= shiftCommentInDAQ((int)halfns, origshift, (int)halfns, (int)cosh,"fine");
  sprintf(errmsg,"DAQlogbook updated (rc:%d).Corde: %d -> %d\n", rcdl,origshift, cosh);prtLog(errmsg); 
  daqlogbook_close(); 

  //rc= daqlogbook_add_comment(0,"Clock shift",daqlog);
};
}
/*-----------------*/ void DLL_RESYNCcmd(void *tag, void *msgv, int *size)  {
char errmsg[200];
char *msg= (char *)msgv; int rc; 
sprintf(errmsg, "DLL_RESYNCcmd: tag:%d size:%d msg:%5.5s\n", 
  *(int *)tag, *size, msg); prtLog(errmsg); 
rc=0; //rc= authenticate();
if(rc!=0) {
  sprintf(errmsg, "DLL_RESYNC not authenticated, but executed");prtLog(errmsg); 
};
if(clocktran!=0)  {
  sprintf(errmsg, "newclock thread already started. Trigger expert should restart ttcmidim and miclock client!"); prtLog(errmsg); 
  infolog_trgboth(LOG_FATAL, errmsg);
  return;  
};
clocktran=3; strcpy(clocktransition,"3"); newclocktag=0;
sprintf(errmsg, "newclock thread DLL_RESYNC starting. tag:%d \n", newclocktag); prtLog(errmsg); 
dim_start_thread(newclock, (void *)&newclocktag);
}
/*-----------------*/ void MICLOCK_SETcmd(void *tag, void *msgv, int *size)  {
char errmsg[200];
char *msg= (char *)msgv; int rc,rc2; 
sprintf(errmsg, "MICLOCK_SETcmd: tag:%d size:%d msg:%5.5s\n", 
  *(int *)tag, *size, msg); prtLog(errmsg); 
/* pydim client: msg not finished by 0x0 ! -that's why strncmp() used below...

if(*size >=2) {
msg[*size]='\0';   // with python client ok
//if(msg[*size-2]=='\n') { msg[*size-2]='\0'; } else { msg[*size-1]='\0'; };
};
*/
rc= authenticate(""); rc2= authenticate("oerjan/");
if((rc!=0) and (rc2!=0) ) {
  sprintf(errmsg, "Only trigger/oerjan user can change the clock"); prtLog(errmsg); 
  return;  
};
if(strncmp(msg,"qq", 2)==0) ds_stop();
if(clocktran!=0)  {
  sprintf(errmsg, "MICLOCK_SET: newclock thread already started!"); prtLog(errmsg); 
  return;  
};
if(strncmp(msg,"BEAM1", 5)==0) { newclocktag=1;
} else if(strncmp(msg,"BEAM2", 5)==0) { newclocktag=2;
} else if(strncmp(msg,"REF", 3)==0) { newclocktag=3;
} else if(strncmp(msg,"LOCAL", 5)==0) { newclocktag=4;
} else { 
  sprintf(errmsg, "bad clock request:%s ignored.\n", msg); prtLog(errmsg); 
  return; 
};
getclocknow();
if(clocktag==newclocktag) {
  sprintf(errmsg, "clock request:%s ignored (already set).\n", msg); prtLog(errmsg); 
  return; 
};
clocktran=3; strcpy(clocktransition,"3");
sprintf(errmsg, "newclock thread starting. tag:%d \n", newclocktag); prtLog(errmsg); 
dim_start_thread(newclock, (void *)&newclocktag);
}


/*----------------------------------------------------------- MICLOCKcaba
*/
void MICLOCKcaba(void *tag, void **msgpv, int *size, int *blabla) {
char **msgp= (char **)msgpv;
char msg[100];
// readVME:
getclocknow();
*msgp= clocknow;
*size= strlen(clocknow)+1;
sprintf(msg, "MICLOCKcaba clocknow:%s size:%d \n", clocknow, *size); prtLog(msg); 
}
/*----------------------------------------------------------- SHIFTcaba
*/
void SHIFTcaba(void *tag, void **msgpv, int *size, int *blabla) {
char **msgp= (char **)msgpv;
char msg[100];
// readVME:
getshiftnow();
*msgp= shiftnow;
*size= strlen(shiftnow)+1;
sprintf(msg, "SHIFTcaba shiftnow:%s size:%d \n", shiftnow, *size); prtLog(msg); 
}
/*----------------------------------------------------------- QPLLcaba
*/
void QPLLcaba(void *tag, void **msgpv, int *size, int *blabla) {
char **msgp= (char **)msgpv;
char msg[100];
// readVME:
*msgp= qpllnow;
*size= strlen(shiftnow)+1;
sprintf(msg, "QPLLcaba shiftnow:%s size:%d \n", shiftnow, *size); prtLog(msg); 
}

/*--------------------------------------------------------------- qpll_thread
*/
void qpll_thread(void *tag) {
while(1) {   //run forever
  int rc; w32 stat; int mainerr,mainlck,bc1err,bc1lck;
  char buffer[50];
  if(envcmp("VMESITE", "ALICE")==0) {
    stat= readstatus();
  } else {
    // simulate:
    stat= qpllstat+1;
  };
  if(stat != qpllstat) {
    qpllstat= stat;
    sprintf(qpllnow,"%3.3x", qpllstat);
    rc= dis_update_service(QPLLid);
    //printf("QPLL update rc:%d qpllnow:%s\n",rc,qpllnow);
    mainerr= (qpllstat & 0x2)>>1; mainlck= (qpllstat & 0x1);
    bc1err= (qpllstat & 0x80)>>7; bc1lck= (qpllstat & 0x40)>>6;
    sprintf(buffer, "mon ds006:ds007:ds008:ds009 N:%d:%d:%d:%d", 
      mainerr, mainlck, bc1err, bc1lck);
    rc= udpsend(udpsock, (unsigned char *)buffer, strlen(buffer)+1);
    //prtLog(buffer);
  };
  nlogqpll++;
  if((nlogqpll % 600)==0) {    // 60/600:log 1/hour
    prtLog(buffer);
  };
  dtq_sleep(10);
  if(quit!=0) break;
};
}
/*--------------------------------------------------------------- ds_register
*/
void ds_register() {
int rc=0, vspRF2TTC=0;
char command[MAXCMDL];

setlinebuf(stdout);
if(micratepresent()) {
  rc= vmxopenam(&vspRF2TTC, "0xf00000", "0x100000", "A32");
  if(rc!=0) {
    printf("vmxopen TTCMI vme:%d\n", rc); exit(8);
  };
};

printf("DIM server:%s\n",MYNAME);
dis_add_error_handler(error_handler);
dis_add_exit_handler(exit_handler);
//dis_add_client_exit_handler (client_exit_handler);
printf("Commands:\n");
strcpy(command, MYNAME); strcat(command, "/MICLOCK_SET");
dis_add_cmnd(command,"C", MICLOCK_SETcmd, MICLOCK_SETtag);  printf("%s\n", command);
strcpy(command, MYNAME); strcat(command, "/CORDE_SET");
dis_add_cmnd(command,"C", CORDE_SETcmd, CORDE_SETtag);  printf("%s\n", command);
strcpy(command, MYNAME); strcat(command, "/DLL_RESYNC");
dis_add_cmnd(command,"C", DLL_RESYNCcmd, DLL_RESYNCtag);  printf("%s\n", command);

printf("\nServices:\n");
strcpy(command, MYNAME); strcat(command, "/MICLOCK");
MICLOCKid=dis_add_service(command,"C", clocknow, MAXLILE+1,
  MICLOCKcaba, MICLOCKtag);  printf("%s\n", command);
strcpy(command, MYNAME); strcat(command, "/MICLOCK_TRANSITION");
MICLOCK_TRANSITIONid=dis_add_service(command,"C", clocktransition, 2,
  NULL, MICLOCK_TRANSITIONtag);  printf("%s\n", command);
strcpy(command, MYNAME); strcat(command, "/SHIFT");
SHIFTid=dis_add_service(command,"C", shiftnow, MAXLILE+1,
  SHIFTcaba, SHIFTtag);  printf("%s\n", command);
strcpy(command, MYNAME); strcat(command, "/QPLL");
QPLLid=dis_add_service(command,"C", qpllnow, MAXLILE+1,
  QPLLcaba, QPLLtag);  printf("%s\n", command);
//MICLOCK_TRANSITIONcaba, MICLOCK_TRANSITIONtag);  printf("%s\n", command);

printf("serving...\n");
dis_start_serving(MYNAME);  
printf("Starting the thread reading BC*QPLL_STATUS regs...\n");
dim_start_thread(qpll_thread, (void *)&TAGqpll_thread);
}

int main(int argc, char **argv)  {
infolog_SetFacility((char *)"CTP");
infolog_SetStream("",0);
signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
signal(SIGQUIT, gotsignal); siginterrupt(SIGQUIT, 0);
signal(SIGBUS, gotsignal); siginterrupt(SIGBUS, 0);
if(envcmp("VMESITE", "ALICE")==0) {
  udpsock= udpopens("alidcscom188", send2PORT);
  micrate(1);
} else {
  udpsock= udpopens("pcalicebhm11", send2PORT);
  micrate(0);
};

ds_register();

while(1)  {  
  /*printf("sleeping 10secs...\n");*/
  dtq_sleep(2); //sleep(10);  
  if(quit>0) break;
  //ds_update();
};  
ds_stop();
exit(0);
}   


