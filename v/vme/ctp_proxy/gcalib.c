#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "vmewrap.h"
#include "ctp.h"
#include "ctplib.h"
#include "vmeblib.h"
#define DBMAIN
#include "Tpartition.h"
#include "lexan.h"

#ifdef CPLUSPLUS
#include <dis.hxx>
#else
#include <dis.h>
#endif

#define DBGCMDS 1
#define SDD 1
#define TOF 5
#define MTR 11
#define T00 13
#define ZDC 15
#define EMC 18
#define DAQ 19

//int GenSwtrg2(int n,char trigtype, int roc, w32 BC,w32 detectors, int customer);

int TAGcalthread=11;
int quit=0; 
int threadactive=0;
w32 heartbeat=0;
w32 last_heartbeat=0xffffffff;
/* 32bit unsigned int:
 (2**32-1)/1000000./60
71.582788250000007 -i.e. >1hour if time is kept in micsecs... */

w32 beammode=0x12345677;

typedef struct t {
  w32 secs; w32 usecs;
} Ttime;
typedef struct ad {
  int deta;   //-1:not active, 1:active
  int period;      // in ms (the planning of next c. trig.). 0:NO GLOB. CALIBRATON
  int calbc;       // 3556->3011 from 31.3.2011 (or from ltu_proxy)
  int roc;         // Readout COntrol (3 bits)
  int past_prot, future_prot;
  char name[12];
  Ttime caltime;   //time of next cal. trigger. secs=0: not initialised
  Ttime lasttime;  //time of last cal. trigger. secs=0: not initialised
  w32 sent;   //number of cal. trigers sent. Cleared at the SOR
  w32 attempts; //number of cal. trigers attempts
} Tacde;

int NACTIVE;
Tacde ACTIVEDETS[NDETEC];

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

/* Operation:
- read gcalib.cfg
gcalib.cfg:
#ltuname period[ms] roc
HMPID 2000
DAQ 3000 6
----------------------------------------*/ void read_gcalibcfg() {
FILE* gcalcfg;
enum Ttokentype token;
char line[MAXLINELENGTH], value[MAXCTPINPUTLENGTH];
char em1[200]="";
gcalcfg= openFile("gcalib.cfg","r");
if(gcalcfg==NULL) {
  prtLog("gcalib cannot be read. Using defaults");
  return;
};
while(fgets(line, MAXLINELENGTH, gcalcfg)){
  int ix,det,milsec, roc;
  //printf("Decoding line:%s ",line);
  if(line[0]=='#') continue;
  if(line[0]=='\n') continue;
  ix=0; token= nxtoken(line, value, &ix);
  if(token==tSYMNAME) {
    char ltuname[20];
    strcpy(ltuname, value);
    det= findLTUdetnum(ltuname);
    if(det<0) {
      strcpy(em1,"bad LTU name in gcalib.cfg"); goto ERR; 
    };
    token=nxtoken(line, value, &ix);
    if(token==tINTNUM) {         // period in ms
      milsec= str2int(value);
    } else {strcpy(em1,"bad period (integer expected ms) in gcalib.cfg"); goto ERR; };
    ACTIVEDETS[det].period= milsec;
    token=nxtoken(line, value, &ix);
    if(token==tINTNUM) {         // roc (decimal)
      roc= str2int(value);
      ACTIVEDETS[det].roc= roc;
    } else if(token != tEOCMD) {
      strcpy(em1,"bad ROC (0-7 expected) in gcalib.cfg"); goto ERR;
    };
    sprintf(em1,"gcalib.cfg:%s %d %d", ACTIVEDETS[det].name, ACTIVEDETS[det].period,
      ACTIVEDETS[det].roc);
    prtLog(em1);
  } else {strcpy(em1,"LTU name expected"); goto ERR; };
};
ERR: 
fclose(gcalcfg); if(em1[0]!='\0') prtLog(em1); return;
};

/*------------------------*/ void getcurtime(Ttime *ct) {
GetMicSec(&ct->secs, &ct->usecs); 
}
void addSecUsec(w32 *tsec,w32 *tusec,w32 plussec,w32 plususec) {
w32 usecs, tsecs; int ix;
usecs= *tusec + plususec; tsecs= *tsec + plussec;
for(ix=0; ix<4294 ; ix++) {   // (2**32-1)/1000000
  if(usecs>=1000000) {
    usecs= usecs-1000000; tsecs++;
  } else goto OK;
}; printf("Error 4294 in addSecUsec\n");
OK:*tsec= tsecs; *tusec= usecs;
}
/*------------------------*/ w32 diffnowbefore(Ttime *now, Ttime *before) {
w32 dif;
dif= DiffSecUsec(now->secs, now->usecs, before->secs, before->usecs);
return(dif);
}
/*------------------------*/ void addtime(Ttime *result, Ttime *delta) {
addSecUsec(&result->secs, &result->usecs, delta->secs, delta->usecs);
}
void printDET1(int ix) {
char active[20];
if(ACTIVEDETS[ix].deta!=-1) {
  strcpy(active,"ACTIVE");
} else {
  strcpy(active,"NOT ACTIVE");
};
printf("%2d: %s %s. period required:%d ms. attempts/sent:%d/%d\n", 
  ix,ACTIVEDETS[ix].name, active, ACTIVEDETS[ix].period,
  ACTIVEDETS[ix].attempts, ACTIVEDETS[ix].sent); 
}
/*--------------------*/int addDET(int det) {
int rc=0; //OK:added or was added already. 1:not added
printf("addDET:%d %s\n",det, ACTIVEDETS[det].name);
if( ACTIVEDETS[det].period==0) {
  printf("addDET:WARN %d not configured for global calibration\n",det); rc=1;
} else if(ACTIVEDETS[det].deta==1) {
  printf("addDET:WARN %d already active\n",det);
} else {
  //int bc;
  ACTIVEDETS[det].deta= 1; 
  ACTIVEDETS[det].sent= 0; ACTIVEDETS[det].attempts= 0; 
  ACTIVEDETS[det].caltime.secs= 0;   // unset
  ACTIVEDETS[det].caltime.usecs= 0;
  ACTIVEDETS[det].lasttime.secs= 0;   // unset
  NACTIVE++;
  if(NACTIVE>NDETEC) {
    printf("addDET:ERROR NACTIVE:%d\n",NACTIVE); rc=1;
  };
};
return(rc);
}
/*--------------------*/int delDET(int det) {
int rc=0;  //OK: deleted or was deleted
if(ACTIVEDETS[det].deta==-1) {
  printf("delDET:WARN %d already not active\n",det);
} else {
  ACTIVEDETS[det].deta=-1; NACTIVE--;
  printDET1(det);
  if(NACTIVE<0) {
    printf("delDET:ERROR NACTIVE:%d\n",NACTIVE); rc=1;
  };
};
return(rc);
}
/* go through SHM and add all global detectors, not yet included,i.e.
if detector is already choosen for calibration, do not replace
info in ACTIVEDETS for this detector (det's ltuproxy was contacted 
only at start!)
rc: number of active detectors
--------------------*/int shmupdateDETs() {
int ix,gdets; int actdets=0;
gdets= cshmGlobalDets();
for(ix=0; ix<NDETEC; ix++) {
  if(ACTIVEDETS[ix].period==0) continue;  // not to be calibrated (according to .cfg)
  if(ACTIVEDETS[ix].deta==-1) {   //not active check if in global run
    if((gdets & (1<<ix))==(1<<ix)) {
      if(ix==T00) {   // T0
        //w32 beammode;
        //beammode= get_DIMW32("CTPDIM/BEAMMODE");cannot be used inside callback
        if(beammode== 11) { // STABLE BEAMS
          printf("updateDETS:T0 not calibrated (STABLE BEAMS)\n");
          continue;       // we do not want to calibrate T0 during  STABLE EBAMS
        };
      }
      if(addDET(ix)==0) actdets++;
    };
  } else {   // ix is active
    printf("updateDETS:active det:%d\n", ix);
    if((gdets & (1<<ix))==0) {  //but not in global partition
      int rc;
      printf("Warning: %s active but not in global, deactivating...\n",
        ACTIVEDETS[ix].name);
      rc= delDET(ix);
    } else {
      /* ix is active (in gcalib) and is in global (info from shm)
         i.e. we do not need to add it to list of active dets! */
      //if(addDET(ix)==0) actdets++;   -removed 11.4.2012
      actdets++;  // we want to return number of active (calibrated) dets
    };
  };
};
return(actdets);
}
/*---------------------------------------------*/ void emptyDET() {
int ix;
for(ix=0; ix<NDETEC; ix++) {
  ACTIVEDETS[ix].deta=-1;
};NACTIVE=0;
}
/*---------------------------------------------*/ void initDET() {
int ix,bc,det;
// 1st phase -very first defaults
for(ix=0; ix<NDETEC; ix++) {
  ACTIVEDETS[ix].deta=-1;
  ACTIVEDETS[ix].period=0;
  ACTIVEDETS[ix].roc=0;
  ACTIVEDETS[ix].calbc=3011;     // was 3556 till 31.3.2011
  if(ctpshmbase->validLTUs[ix].name[0] != '\0'){
    strcpy(ACTIVEDETS[ix].name, ctpshmbase->validLTUs[ix].name);
  } else {
    strcpy(ACTIVEDETS[ix].name,"nocalib");
  };
};NACTIVE=0;
ACTIVEDETS[SDD].period=0;    // 50 i.e. 3 triggs (50ms) spaced 15*60000
ACTIVEDETS[TOF].period=200;  // 1000;
ACTIVEDETS[MTR].period=33000; // 10000 .. 100000
ACTIVEDETS[T00].period=1000;
ACTIVEDETS[ZDC].period=10000;    // 10000;
ACTIVEDETS[EMC].period=2000;  // 2000 (1000 .. 5000)
ACTIVEDETS[DAQ].period=0;
/*strcpy(ACTIVEDETS[SDD].name, "SDD");
strcpy(ACTIVEDETS[TOF].name, "TOF");
strcpy(ACTIVEDETS[MTR].name, "MTR");
strcpy(ACTIVEDETS[T00].name, "T0");
strcpy(ACTIVEDETS[ZDC].name, "ZDC");
strcpy(ACTIVEDETS[EMC].name, "EMC");
strcpy(ACTIVEDETS[DAQ].name, "DAQ"); */
// 2nd phase: calbc retrieved from ltuproxies (only T0 now)
det=T00;
bc= getCALIBBC(ACTIVEDETS[det].name);
if(bc>0) {
  if( ACTIVEDETS[det].calbc!= bc ) {
    printf("Detector:%s CALIBBC:%d->%d\n",ACTIVEDETS[det].name,
      ACTIVEDETS[det].calbc, bc );
      ACTIVEDETS[det].calbc= bc; 
  };
} else {
  printf("ERROR: cannot contact %s(%d) ltu proxy.CALIBBC set to:%d\n",
    ACTIVEDETS[det].name, det, ACTIVEDETS[det].calbc);
};
// 3rd phase: period (and roc?) from $dbctp/gcalib.cfg
read_gcalibcfg();
}  
/*---------------------------------------------*/ void printDETS() {
int ix;
printf("Calibration active for detectors:");
for(ix=0; ix<NDETEC; ix++) {
  if(ACTIVEDETS[ix].deta==1) {
    printf("%d ", ix);
  };
}; printf("\n");
for(ix=0; ix<NDETEC; ix++) {
  //if(ACTIVEDETS[ix].deta!=-1) {
  if(strcmp(ACTIVEDETS[ix].name,"nocalib")!=0) {
    printDET1(ix);
    continue;
  };
}; printf("\n");
}
/* Find next detector (nearest in time) waiting for cal. trigger
rc: -1 (no active dets) or index to ACTIVEDETS
-----------------------------------------------*/ int findnextcDET() {
Ttime mint={0xffffffff,0xffffffff}; int ix,ixmin=-1;
for(ix=0; ix<NDETEC; ix++) {
  if(ACTIVEDETS[ix].deta==-1) continue;   //not active
  if(ACTIVEDETS[ix].caltime.secs < mint.secs) {
    mint.secs= ACTIVEDETS[ix].caltime.secs;
    mint.usecs= ACTIVEDETS[ix].caltime.usecs;
    ixmin=ix;
  } else if(ACTIVEDETS[ix].caltime.secs == mint.secs) {
    if(ACTIVEDETS[ix].caltime.usecs < mint.usecs) {
      mint.secs= ACTIVEDETS[ix].caltime.secs;
      mint.usecs= ACTIVEDETS[ix].caltime.usecs;
      ixmin=ix;
    };
  };
}; return(ixmin);
}
/*--------------------*/ int globalcalDET(int det){ 
int gdets; int rc=0;
gdets= cshmGlobalDets();
if((gdets & (1<<det))==(1<<det)) rc=1;
return(rc);
}
/*--------------------*/ void calthread(void *tag) {
Ttime ct,deltaTtime; w32 delta; int sddburst=0;
//printf("calthread:\n");
threadactive=1;
if(DBGCMDS) {
  prtLog("calthread start.");
};
while(1) {
  int ndit;
  heartbeat++; if(heartbeat>0xfffffff0) heartbeat=0;
  ndit= findnextcDET();   // find closest one in time
  //printf("det:%s %d %d\n", ACTIVEDETS[ndit].name, ACTIVEDETS[ndit].caltime.secs, ACTIVEDETS[ndit].caltime.usecs);
  if(ndit==-1) goto STP;
  if(globalcalDET(ndit)==0) {
    delDET(ndit); if(NACTIVE==0) goto STP;
    continue;
  };
  getcurtime(&ct); 
  if( ACTIVEDETS[ndit].caltime.secs>0) {
    w32 milisecs; int rcgt;
    delta=diffnowbefore(&ACTIVEDETS[ndit].caltime, &ct);
    if(delta>100) {
      usleep(delta);
    };
    if(globalcalDET(ndit)==0) {
      delDET(ndit); if(NACTIVE==0) goto STP;
      continue;
    };
    // generate calib. trigger:
    if(cshmGlobFlag(FLGignoreGCALIB)==0) {
      w32 orbitn;
      rcgt= GenSwtrg(1, 'c', ACTIVEDETS[ndit].roc, ACTIVEDETS[ndit].calbc,
        1<<ndit, 1, &orbitn);
      if(rcgt==12345678) {
        delDET(ndit); if(NACTIVE==0) goto STP;
        continue;
      };
      ACTIVEDETS[ndit].attempts++;
    } else {
      rcgt=0; // cal. trigger not generated (disabled during start/stop part)
    };
    getcurtime(&ct); // time of cal. trigger just attempted to be generated
    // movd up ACTIVEDETS[ndit].attempts++;
    if(rcgt==1) {
      if(ACTIVEDETS[ndit].lasttime.secs==0) {
        milisecs=0;
      } else {
        milisecs= diffnowbefore(&ct, &ACTIVEDETS[ndit].lasttime)/1000;
      };
      ACTIVEDETS[ndit].sent++;
      //printf("CT:%s \t %d\t%d ms:%d\n", ACTIVEDETS[ndit].name, ct.secs, ct.usecs, milisecs);
      //printf("CT:%s ms:%d\n", ACTIVEDETS[ndit].name, milisecs);
      ACTIVEDETS[ndit].lasttime.secs= ct.secs;
      ACTIVEDETS[ndit].lasttime.usecs= ct.usecs;
    };
    // usleep(100);  // do we need this (waiting for the completion)?
    /* if(ACTIVEDETS[ndit].attempts > 200 ) {
      emptyDET(); goto STP;
    }; */
  };
  // the planning of next cal. trigger for ndit detector:
  // NextTime= CurrentTime + period[ndit] i.e.
  // IS THE SAME IN EITHER CASE (successful or unsuccsessful)
  if( ACTIVEDETS[ndit].caltime.secs==0) {   // wait 2 secs before 1st cal. event
    char msg[200];
    sprintf(msg, "1st planning:%s \t %d\t%d", ACTIVEDETS[ndit].name, ct.secs, ct.usecs);
    prtLog(msg);
    deltaTtime.secs=2; deltaTtime.usecs=0;
  } else {
    deltaTtime.secs=0; deltaTtime.usecs=1000*ACTIVEDETS[ndit].period;
  };
  if(ndit==SDD) {   // SDD 3x50ms every 15minutes
    sddburst++;
    if(sddburst==3) {
      sddburst=0;
      deltaTtime.usecs= 15*60*1000000;
    };
  };
  addtime(&ct, &deltaTtime);
  ACTIVEDETS[ndit].caltime.secs= ct.secs;
  ACTIVEDETS[ndit].caltime.usecs= ct.usecs;
  //printf("det:%s next:%d %d\n", ACTIVEDETS[ndit].name, ct.secs,ct.usecs); 
};
STP:
threadactive=0;
if(DBGCMDS) {
  prtLog("calthread stop.");
};
}
void startThread() {
dim_start_thread(calthread, (void *)&TAGcalthread);
usleep(100000);
if(threadactive==0) {
  char msg[200];
  sprintf(msg,"thread not started! exiting..."); prtLog(msg);
  //exit(8);
  quit=8;
};
if(detectfile("gcalrestart", 1) >=0) {   //debug: simulate error by file presence
  // i.e.: echo blabla >$VMEWORKDIR/gcalrestart
  char msg[200];
  //system("gcalibrestart_at.sh >/tmp/gcalibresatrt_at.log");
  system("rm gcalrestart");
  sprintf(msg,"gcalrestart removed, registered, exiting..."); prtLog(msg);
  //exit(8);
  quit=8; 
}else {
  char msg[200];
  //system("gcalibrestart_at.sh");
  sprintf(msg,"WORK/../gcalrestart not present (i.e. real crash, not simulated)"); prtLog(msg);
};
}

/*--------------------*/ void DOcmd(void *tag, void *msgv, int *size)  {
/* msg: if string finished by "\n\0" remove \n */
char *mymsg= (char *)msgv; int msglen=100;
char msg[101];
enum Ttokentype token; int ix; char value[100]; char em1[200];

if(DBGCMDS) {
  char logmsg[200];
  sprintf(logmsg, "DOcmd1: tag:%d size:%d mymsg:%s<-endofmsg", *(int *)tag, *size,mymsg);
  prtLog(logmsg);
};
if(*size <msglen) msglen=*size;
strncpy(msg, mymsg, msglen); msg[msglen]='\0';
/*if(msg[*size-2]=='\n') {
  msg[*size-2]='\0';
} else {
  msg[*size-1]='\0';
}; */
/*msg: 
u         -update from SHM (this cmd is issued by ctp_proxy at SOR)
a 0 2 5   -add detectors for calibration (valid only for active run)
d 0 5     -delete detectors from calibration 
*/
ix=0; token= nxtoken(msg, value, &ix);
if(token==tSYMNAME) {
  if((strcmp(value,"u")==0) ) {
    int ads;
    ads= shmupdateDETs();
    if(ads>0){
      printf("Docmd: starting thread, # of dets:%d threadactive:%d\n", 
        ads, threadactive);
      fflush(stdout);
      if(threadactive==0) {
        startThread();
      } else { // 1: 2nd global run (ok) or what?
        // perhaps, here we should stop/start active thread.
        sprintf(em1,"u:ads:%d, threadactive is 1, i.e. 2nd global?", ads); 
        //goto ERR;
        ;
      };
    };
  } else if((strcmp(value,"a")==0) || (strcmp(value,"d")==0)) {
    int det,rc; char adddel;
    adddel=value[0];
    while(1) {
      token= nxtoken(msg, value, &ix);
      if(token == tINTNUM) {
        det= str2int(value);
        if(adddel=='a') {
          rc= addDET(det);
        } else {
          rc= delDET(det);
        };
        if(rc!=0) { sprintf(em1,"addel:%c rc:%d", adddel, rc); goto ERR; };
        if(threadactive==0) {
          startThread();
        } else {
          sprintf(em1,"a:det:%d but threadactive is 1", det); goto ERR;
        };
      } else if(token == tINTNUM) {
        break;
      } else {
        strcpy(em1,"int expected: 'a/d det1 det2 ...' "); goto ERR;
      };
    };
  } else if((strcmp(value,"p")==0)) {
    printDETS();
  } else {
    strcpy(em1,"a,d or p expected as first item in message"); goto ERR; 
  };
} else {
  strcpy(em1,"a,d or p expcted as first item in message"); goto ERR;
};
return;
ERR:
printf("DOcmd ERROR:%s msg:%s\n", em1, msg);
}

/*---------------------------------------------*/ void registerDIM() {
#define MYNAME "CTPCALIB"
char command[100];
printf("DIM server:%s\n",MYNAME);
/*dis_add_error_handler(error_handler);
dis_add_exit_handler(exit_handler);
dis_add_client_exit_handler (client_exit_handler); */
printf("Commands:\n");
strcpy(command, MYNAME); strcat(command, "/DO");
dis_add_cmnd(command,"C", DOcmd, 88);  printf("%s\n", command);
printf("serving...\n");
dis_start_serving(MYNAME);  
}
void stopDIM() {
dis_stop_serving();
}

/*------------------------------------*/ int main(int argc, char **argv)  {
int rc,ads;
/*
if(argc<3) {
  printf("Usage: ltuserver LTU_name base\n\
where LTU_name is detector name\n\
      base is the base address of LTU (e.g. 0x811000)\n\
"); exit(8);
}; 
if(argc>1) {
  if(strcmp(argv[1],"no1min")==0) {
    dimsflags=dimsflags | NO1MINFLAG;
  };
};*/
setlinebuf(stdout);
signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
signal(SIGQUIT, gotsignal); siginterrupt(SIGQUIT, 0);
signal(SIGBUS, gotsignal); siginterrupt(SIGBUS, 0);
prtLog("gcalib started...");
rc= vmeopen("0x820000", "0xd000");
if(rc!=0) {
  printf("vmeopen CTP vme:%d\n", rc); exit(8);
};
cshmInit();
initDET(); // has to be after cshmInit()
checkCTP(); 
printf("No initCTP. initCTP left to be done by main ctp-proxy when started\n"); 
//initCTP();
registerDIM();
beammode= get_DIMW32("CTPDIM/BEAMMODE");  //cannot be used inside callback
ads= shmupdateDETs();  // added from 18.11.2010
if(ads>0){
  if(threadactive==0) {
    startThread();
  } else {
    printf("ads:%d but threadactive is 1 at the start", ads);   //cannot happen
  };
};
while(1)  {  
  /* the activity of calthread is checked here:
    if threadactive==1 & heartbeat did not change, the calthread
    is not active in spite of threadactive is claiming it is active!
  */
  if(threadactive==1) {
    if(heartbeat == last_heartbeat) {
      prtLog("ERROR: heartbeat is quiet, setting threadactive to 0.");
      threadactive=0;
    };
  };
  //printf("sleeping 40secs...\n");
  last_heartbeat= heartbeat;
  /*dtq_sleep(2); */
  sleep(40);  // should be more than max. cal.trig period (33s for muon_trg)
  if(detectfile("gcalrestart", 1) >=0) { 
    char msg[200];
    sprintf(msg,"gcalrestart exists"); prtLog(msg);
    system("rm gcalrestart");
    sprintf(msg,"main: gcalrestart removed, exiting..."); prtLog(msg);
    quit=8; 
  };
  if(quit>0) break;
  beammode= get_DIMW32("CTPDIM/BEAMMODE");  //cannot be used inside callback
  //ds_update();
};  
stopDIM(); cshmDetach();
vmeclose();
exit(0);
}   

