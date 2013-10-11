/*
25.10.2011
setbcorbitMain: clock shift adjusted (RF2TTC+CORDE) with value from $dbctp/clockshift
 only when clock changed to LHC clock
25.10.2011
corde_shift(): maximal allowed shift is beteween -150..+150 (+-1500ps)
12.11.2011
corde_shift(): bug fixed: 150< allowed shift <150 is now: -150<shift<150
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>   //atoi
#include <unistd.h>   //usleep

#include "infolog.h"
#include "vmewrap.h"
#include "vmeblib.h"
//#include "daqlogbook.h"
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif
#include "ctplib.h"
#include "../../ttcmi/ttcmi.h"

//#include "/opt/libDAQlogbook/DAQlogbook.h"
//----------------------------------------- corde board:
#define RESET 0x24
#define ORBMAIN 0x7fbb4
char corde_base[]="0x7000000";
char corde_len[]="0x7fc00";
char corde_A32[]="A32";

w32 halfnsvme=0x140+29, cordevalvme=512;
int havemicrate=1;

void com2daq(int run, char *title, char *msg) {
int rc; char cmd[530];
if( (strlen(title)+strlen(msg)) > 500) {
  printf("Error: com2daq: too long msg: %d %s %s\n", run,title,msg);
} else {
  sprintf(cmd, "%d \"%s\" \"%s\"", run, title, msg);
  rc= dic_cmnd_service((char *)"CTPRCFG/COM2DAQ", cmd, strlen(cmd)+1);
  printf("com2daq: rc:%d title:%s\n", rc, title);
};
}

w32 corde_get(int del) {  // 1..7. VME is opened/closed with each call!
int rc,vsp=-1; w32 adr, val;
if(micratepresent()) {
  rc= vmxopenam(&vsp, corde_base, corde_len, corde_A32);
} else rc=0;
if(rc==0) {
  adr= (del-1)*4 + ORBMAIN; 
  if(micratepresent()) {
    val= vmxr32(vsp, adr);
    rc= vmxclose(vsp);
  } else {
    val= cordevalvme;
  };
} else {
  val=0xffffffff;
};
return(val);
}
void corde_set(int del, w32 val) {
int vsp=-1,rc; w32 adr;
if(micratepresent()) {
  rc= vmxopenam(&vsp, corde_base, corde_len, corde_A32);
} else rc=0;
if(rc==0) {
  adr= (del-1)*4 + ORBMAIN; // 7: BC1
  if(micratepresent()) {
    vmxw32(vsp, adr, val);
  } else {cordevalvme= val; };
  //adr= (4-1)*4 + ORBMAIN; // BC_MAIN (not needed -only BC1 delayd in CORDE)
  //vmxw32(vsp, adr, val);
  if(micratepresent()) {
    rc= vmxclose(vsp);
  };
} else {
  printf("ERROR corde_set: can't open vme\n");
};
return;
}
/*shift: in 10ps units, should be in interval -150..+150 (+-1500ps)
  rc: new value set in corde register
      0xffffffff -vme open error
      0xfffffffe -bad input (too big shift)
*/
#define corde_sleep_us 10
#define corde_ministep 5
w32 corde_shift(int del, int shift, int *origval) {  
int vsp=-1, rc,rcv; 
if(micratepresent()) {
  rcv= vmxopenam(&vsp, corde_base, corde_len, corde_A32);
} else rcv=0;
if(rcv==0) {
  int sig; w32 base,adr,lastval;
  adr= (del-1)*4 + ORBMAIN; 
  if(micratepresent()) {
    base= vmxr32(vsp, adr); 
  } else {
    base= cordevalvme;
  };
  *origval= base;
  if((shift <-150) || shift>150) {
      sig=0; rc=0xfffffffe;
  } else if(shift<0) {
    sig=-1;
    if((int)base < (-shift)) {
      sig=0; rc=0xfffffffe;
    };
  } else {
    sig=1;
    if((base+shift) > 1023 ) {
      sig=0; rc=0xfffffffe;
    };
  };
  if(sig!=0) {
    w32 val;
    val= base; lastval= base + shift;
    while(1) {
      val= val+sig*corde_ministep;
      if(((sig==1) && (val>lastval)) ||
         ((sig==-1) && (val<lastval))) val=lastval;
      if(val>1023) {
        printf("Error:corde_shift: cannot write %d>1023 into CORDE\n",val);
        break;
      };
      printf("corde_shift: writing %d\n",val);
      if(micratepresent()) {
        vmxw32(vsp, adr, val);
      } else cordevalvme= val;
      if(corde_sleep_us>0) usleep(corde_sleep_us);
      if((val==lastval)) break;
    };
    rc=lastval;
  };
  if(micratepresent()) {
    rcv= vmxclose(vsp);
  };
} else {
  printf("Error: corde_shift: can't open vme\n"); rc=0xffffffff;
};
return(rc);
}

//---------------------------------------- end of corde code
w32 i2cread_delay(w32 delayadd) {
w32 data;
if(micratepresent()) {
  data= vmer32(delayadd);
  usleep(2000); /// at least 2ms
  data= vmer32(DELAY25_REG);
} else {
  data= halfnsvme;
};
return(data);
}
void i2cset_delay(w32 delayadd, int halfns) {
if(micratepresent()) {
  halfns= halfns | 0x40;            // enable
  vmew32(delayadd, halfns);
  //vmew32(BC_DELAY25_GCR, 0x40);   // and resynchronise DLL (wait 1s if used)
} else {
  halfnsvme= halfns;
};
}
/* add Comment in daq for:
Input: hw and db values for RF2TTC/Corde dealay registers
fineshift is "": course shift (when the LOCAL/BEAM1 change)
fineshift is "fine": fine shift (only Corde board, after FLAT TOP -
                     halfns and dbhalfns not valid.
*/
void shiftCommentInDAQ(int halfns, int cordeval, 
  int dbhalfns, int dbcordeval, char *fineshift) {
int ps,dbps; char daqlog[200];
  ps= halfns*500 + cordeval*10;
  dbps= dbhalfns*500 + dbcordeval*10;
  sprintf(daqlog,"changed from %d %d(%dps) to %d %d(%dps). %s", 
    halfns,cordeval,ps, dbhalfns,dbcordeval,dbps, fineshift);
  //rcdl= daqlogbook_add_comment(0,"Clock shift",daqlog);
  com2daq(0,(char *) "Clock shift",daqlog);
return;
}

/*------------------------------------------------------------readclockshift()
*/
int readclockshift(char *mem, int maxlen) {
FILE *cf; int sp;
char fname[200]; char *environ;
environ= getenv("VMEWORKDIR");
sprintf(fname,"%s/CFG/clockshift", environ);
cf= openFile(fname,"r");
if(cf != NULL) {
  sp=fread((void *)mem, 1, maxlen-1, cf); 
  mem[sp]='\0';
  fclose(cf); 
} else {
  sp=0;
};
return(sp);
}
void setCordeshift() {
#define MAXdbhns 40
w32 pol, halfns, dbhalfns, cordeval, dbcordeval, dblast_applied; 
char dbhns[MAXdbhns]; int ldbhns;
  
  // check clockshift
  pol= i2cread_delay(BC_DELAY25_BCMAIN); halfns= pol-0x140;
  //printf("%24s 0x%x:0x%x=%d halfsns \n\n", "BC_DELAY25_BCMAIN", BC_DELAY25_BCMAIN, pol, halfns);
  cordeval= corde_get(CORDE_DELREG);
  //ldbhns= readdbfile("clockshift", dbhns, MAXdbhns);
  ldbhns= readclockshift(dbhns, MAXdbhns);
  if(ldbhns>2) {   // shortest: '0 0'
    int its;
    //dbhalfns= atoi(dbhns);
    its= sscanf(dbhns, "%d %d %d\n", &dbhalfns, &dbcordeval, &dblast_applied);
    printf("setCordeshift: clockshift db:%d %d %d hw:%d %d\n", dbhalfns, dbcordeval, dblast_applied, halfns, cordeval);
    if( (its<2) || (dbhalfns>63) || (dbcordeval>1023) ) {
      infolog_trg(LOG_ERROR, "Bad value in dbctp/clockshift file");
    } else {
      if((halfns != dbhalfns) || (cordeval != dbcordeval)) {
        i2cset_delay( BC_DELAY25_BCMAIN, dbhalfns);
        i2cset_delay( BC_DELAY25_BC1, dbhalfns);  //keep the same for BPIM
        corde_set(CORDE_DELREG, dbcordeval);
        shiftCommentInDAQ(halfns, cordeval, dbhalfns, dbcordeval,"");
      };
    };
  } else {
    char emsg[300];
    sprintf(emsg, "Bad value in $dbctp/clockshift file:%s:", dbhns);
    infolog_trgboth(LOG_ERROR, emsg);
  };
}

void setbcorbitMain(int maino) {
w32 bcmain,orbmain;
char msg[300]; char daqlog[90];
infolog_SetFacility((char *)"CTP");   // shoul be set in ttcmi.c main...
infolog_SetStream("",0);
//rcdl= DAQlogbook_open("trigger:trigger123@10.161.36.8/LOGBOOK");
// rcdl=daqlogbook_open();   //rcdl must be 0 if opened
//rcdl=2; printf("setbcorbitMain: DAQlogbook not used...\n");
//if(rcdl!=-1) { rcdl=0; };
bcmain= vmer32(BCmain_MAN_SELECT); orbmain= vmer32(ORBmain_MAN_SELECT);
if((maino==1) || (maino==2) || (maino==3)) {   // when LHC clock 
//if(1) {   // shift ALWAYS before clock change
/* should be thrown out and executed only when crate is powered up: */
setCordeshift();
};
if(maino==1) {
  vmew32(BCmain_MAN_SELECT, 3);  /* 3: BC1 input */
  vmew32(ORBmain_MAN_SELECT, 0); /* 0: ORB1 input */
  sprintf(msg, "Global clock: %x/%x -> BEAM1/ORB1",bcmain,orbmain);
  strcpy(daqlog,"BEAM1");
} else if(maino==2) {
  vmew32(BCmain_MAN_SELECT, 2);  /* 2: BC2 input */
  vmew32(ORBmain_MAN_SELECT, 1); /* 1: ORB2 input */
  sprintf(msg, "Global clock: %x/%x -> BEAM2/ORB2",bcmain,orbmain);
  strcpy(daqlog,"BEAM2");
} else if(maino==3) {
  vmew32(BCmain_MAN_SELECT, 1);  /* 1: BCref input */
  vmew32(ORBmain_MAN_SELECT, 2); /* 2: internal BCmain synchronized orbit gen*/
  sprintf(msg, "Global clock: %x/%x -> BCREF/ORBlocal",bcmain,orbmain);
  strcpy(daqlog,"REF");
} else if(maino==4) {
  vmew32(BCmain_MAN_SELECT, 0);  /* 1: internal 40.078 MHz clock */
  vmew32(ORBmain_MAN_SELECT, 2); /* 2: internal BCmain synchronized orbit gen*/
  sprintf(msg, "Global clock: %x/%x -> BClocal/ORBlocal",bcmain,orbmain);
  strcpy(daqlog,"LOCAL");
} else {
  printf("Bad maino input. No action\n"); return;
};
        // always, after clock change resynchronize DLL on RF2TTC:
sprintf(msg, "%s + DLL resync", msg);
DLL_RESYNC(0);        //vmew32(BC_DELAY25_GCR, 0x40);
infolog_trg(LOG_INFO, msg);
//rcdl= daqlogbook_open();
//if(rcdl==0) {
//  rcdl= daqlogbook_add_comment(0,"CLOCK",daqlog);
//  daqlogbook_close(); 
//};
com2daq(0,"CLOCK",daqlog);
}
w32 readstatus() {
w16 rc; w32 s1,s2,s3,s4,s5;
s1= vmer32(BC1_QPLL_STATUS);
s2= vmer32(BC2_QPLL_STATUS);
s3= vmer32(BCref_QPLL_STATUS);
s4= vmer32(BCmain_QPLL_STATUS);
s5= vmer32(TTCrx_status);
rc= ((s5&0x1)<<8) | ((s1&0x3)<<6) | ((s2&0x3)<<4) | ((s3&0x3)<<2) | (s4&0x3);
return(rc);
}
void DLL_RESYNC(int msg) {
if(micratepresent()==0) {
  printf("DLL_RESYNC, novme, 0x40 -> BC_DELAY25_GCR done\n");
  return;
};
vmew32(BC_DELAY25_GCR, 0x40);
if(msg==DLL_stdout) {
printf("0x40 -> BC_DELAY25_GCR done\n");
} else if(msg==DLL_daq) {
  com2daq(0,"CLOCK","DLL_RESYNC");
  //int rcdl;
  //rcdl=daqlogbook_open();   //rcdl must be 0 if opened
  //if(rcdl==0) {
  //  rcdl= daqlogbook_add_comment(0,"CLOCK","DLL_RESYNC");
  //  daqlogbook_close(); 
  //};
} else if(msg==DLL_info) {
  //infolog_SetFacility((char *)"CTP"); set in ttcmidims.c
  //infolog_SetStream("",0);
  infolog_trg(LOG_INFO, "CLOCK DLL_RESYNC done");
};
}
void micrate(int present) {
havemicrate= present;
}
int micratepresent() {
return(havemicrate);
}
