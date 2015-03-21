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
//von #include "ctplib.h"
#include "ttcmi.h"

int vspRFRX[3]={-1,-1};  // RFRX1,2
int vspRF2TTC=0;
//----------------------------------------- corde board (also vme/corde dir):
char corde_base[]="0x7000000";
char corde_len[]="0x7fc00";
char corde_am[]="A32";

w32 halfnsvme=0x140+29, cordevalvme=512;
/* 3:p2     RF2TTC + RFRXs
 * 0:altri1 (no RF2TTC, no RFRX) 
 * 2:altri2 (only RF2TTC)
 * 4: corde only
*/
int havemicrate=0; 

int openrfrxs() {
int ix,rc=0;
const char *rfrxbase[2]={"0x300000", "0x400000"};
char msg[200]="";
for(ix=0; ix<=1; ix++) {
  vspRFRX[ix]=-1; 
  rc= vmxopenam(&vspRFRX[ix], (char *)rfrxbase[ix], (char *)"0x100", (char *)"A24");
  //rc= vmxopenam(&vspRFRX[ix], rfrxbase[ix], "0x100", "A24");
  sprintf(msg, "vmxopen RFRX%d rc:%d vsp:%d\n", ix, rc, vspRFRX[ix]); printf(msg);
  if(rc!=0) {
    break;
  };
};
return(rc);
}

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
if(micratepresent()&0x4) {
  rc= vmxopenam(&vsp, corde_base, corde_len, corde_am);
} else rc=0;
if(rc==0) {
  adr= (del-1)*4 + CORDE_ORBMAIN; 
  if(micratepresent()&0x4) {
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
if(micratepresent()&0x4) {
  rc= vmxopenam(&vsp, corde_base, corde_len, corde_am);
} else rc=0;
if(rc==0) {
  adr= (del-1)*4 + CORDE_ORBMAIN; // 7: BC1
  if(micratepresent()&0x4) {
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
if(micratepresent()&0x4) {
  rcv= vmxopenam(&vsp, corde_base, corde_len, corde_am);
} else rcv=0;
if(rcv==0) {
  int sig; w32 base,adr,lastval;
  adr= (del-1)*4 + CORDE_ORBMAIN; 
  if(micratepresent()&0x4) {
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
/*FGROUP
- reset CORDE board
- set RF boards (0x5 for BC and 0x70 for Orbit)
- set polarity/length for all 3 RF2TTC outputs: MAIN/ORB2/ORB1
- set BC1/2 and Orbit1/2 delays
*/
void writeall() {
int ix,rc2;
//if((micratepresent()==3) || (micratepresent()==2)) {
if(micratepresent()&1 ) {          //----------------------- RFRXs
  w32 adrpol;
  w16 refsBC[3]={0x5, 0x5, 0xa0};     // a0 from 4.12.2014 (was 0x70)
  w16 refsOrbit[3]={0x5, 0x5, 0xa0};  // a0 from 4.12.2014
  printf("setting RFRXs...vspRFRX:%d %d\n", vspRFRX[0],vspRFRX[1]);
  for(ix=0; ix<3; ix++) {
    adrpol= ch1_ref + ix*0x2;
    vmxw16(vspRFRX[0], adrpol, refsBC[ix]);
    vmxw16(vspRFRX[1], adrpol, refsOrbit[ix]);
  };
  /* rc1= vmxclose(vspRFRX[0]); rc2= vmxclose(vspRFRX[1]);
  printf("vmxclose 0x300000 rc:%d 0x400000 rc:%d\n", rc1, rc2);
  */
} else {
  printf("RFRXs not set (not p2)\n");
};
if(micratepresent()&2 ) {          //----------------------- RF2TTC
  w32 adrpol, adrlen;
  printf("setting RF2TTC... vspRF2TTC:%d\n", vspRF2TTC);
  adrpol= vmer32(ORB1_DAC);
  printf("ORB1_DAC:0x%x\n", adrpol);
  vmew32(ORB1_DAC, 0xaa); vmew32(ORB2_DAC, 0xaa);
  for(ix=0; ix<3; ix++) {
    adrpol= ORBX_POLARITY+ 0x40*ix;
    adrlen= ORBX_LENGTH+ 0x40*ix;
    vmew32(adrpol, 1); vmew32(adrlen, 0x26);
  };
  /* set delays: */
  //i2cset_delay( BC_DELAY25_BCMAIN, 44);  // 44 from 15.jun2010, 0 before
  //i2cset_delay( BC_DELAY25_BCMAIN, 40);  // 40 from 10.july2010, 44 before
  //i2cset_delay( BC_DELAY25_BCMAIN, 44);  // 44 from 6.sep2010, 40 before
  //i2cset_delay( BC_DELAY25_BCMAIN, 46);  // 46 from 25.oct2010, 44 before
  //i2cset_delay( BC_DELAY25_BCMAIN, 47);  // 47 from 5.nov2010, 46 before
  //i2cset_delay( BC_DELAY25_BCMAIN, 48);  // 48 from 18.nov2010, 47 before
  //i2cset_delay( BC_DELAY25_BCMAIN, 49);  // 49 from 26.nov2010, 48 before
  //i2cset_delay( BC_DELAY25_BCMAIN, 50);  // 50 from 29.nov2010
  //i2cset_delay( BC_DELAY25_BCMAIN, 51);  // 51 from 2.dec.2010
  //i2cset_delay( BC_DELAY25_BCMAIN, 52);  // 52 from 6.dec.2010 8:20
  //i2cset_delay( BC_DELAY25_BCMAIN, 53);  // 53 from 6.dec.2010 14:30
  // from 2011 set in setbcorbitMain
  /* lets add 5ns for Orbit latch (see calibrate() ) 
  i2cset_delay(ORBIN_DELAY25_ORB1, 20);
  i2cset_delay(ORBIN_DELAY25_ORB2, 0);
  */
  i2cset_delay(ORBIN_DELAY25_ORB1, 0x18);   // from 4.12.2014 (rf2ttcscope)
  i2cset_delay(ORBIN_DELAY25_ORB2, 0x11);
  //vmew32(ORBmain_COARSE_DELAY,2);
  vmew32(ORBmain_COARSE_DELAY,3564);
  vmew32(ORB1_COARSE_DELAY,3564);
  i2cset_delay(ORBOUT_DELAY25_ORB1,11);
  i2cset_delay(ORBOUT_DELAY25_ORBMAIN,11);
  // i.e.: BC_DELAY25_BCMAIN 0x7d00c:16c
  //i2cset_delay( ORBmain_COARSE_DELAY, 0); // was and is 0
  /* all the others left as initialised by RF2TTC fy */

  vmew32(BC1_MAN_SELECT,1); vmew32(BC2_MAN_SELECT,1); vmew32(BCref_MAN_SELECT,1);
  vmew32(ORB1_MAN_SELECT, 0); vmew32(ORB2_MAN_SELECT, 0); 
  printf("BC1/2/ref and ORB1/2 connected to their external inputs\n");
  setbcorbitMain(4); printf("Using local clock\n");
  //setbcorbitBO1(1);
  //setorbitdelay(3563);
};
if(micratepresent()&0x4) {
  int vsp4;
  vsp4=-1; 
  rc2= vmxopenam(&vsp4, (char *)corde_base, (char *)corde_len,
    (char *)corde_am);
  printf("vmxopenam %s (CORDE) rc:%d vsp4:%d\n",
    corde_base, rc2, vsp4);
  vmxw32(vsp4, CORDE_RESET, 0);
  vmxw32(vsp4, CORDE_RESET, 1);
  vmxw32(vsp4, CORDE_RESET, 0);
  rc2= vmxclose(vsp4);
  printf("CORDE board reset done. vmxclose rc:%d\n",rc2);
  //printf("CORDE board reset done. vmxclose NOT done\n");
} else {
  printf("CORDE not set (not p2)\n");
};
/* vsp=-1; rc= vmxopenam(&vsp, "0x0f00000", "0x100000", "A32");
printf("rf2ttc rc:%d vsp:%d\n", rc, vsp); */
if((micratepresent()&0x3)==0) {
  printf("Realy good place to start ttcmi ? micratepresent:%d\n",micratepresent());
};
//rc= vmxclose(vsp);
}
void getRFRX(int vsp, Tchan *frs) {
int ix;
for(ix=0; ix<3; ix++) {
  w32 fhl, refadr;
  w16 flow, fhigh;
  refadr= ch1_ref +ix*2;
  frs[ix].ref= vmxr16(vsp, refadr);
  flow= vmxr16(vsp, ch1_freq_low+(ix*4));
  fhigh= vmxr16(vsp, ch1_freq_high+(ix*4));
  fhl= flow | (fhigh <<16);
  frs[ix].freq= (80*16*22)/(flow+ (fhigh*65536.));
  //printf("%d=%fMHz ", fhl, frs[ix].freq);
}; printf("\n");
}
void printRFRX(char *rfrxbase) {   // invoked ONLY from ttcmi (NOT from ttcmidims!)
int ix,rc,vsp;
float frekvs[3];
vsp=-1; rc= vmxopenam(&vsp, rfrxbase, (char *)"0x100", (char *)"A24");
printf("RFRX:%s ch1/2/3_ref:0x%x 0x%x 0x%x\n", rfrxbase,
  vmxr16(vsp, ch1_ref), vmxr16(vsp, ch2_ref), vmxr16(vsp, ch3_ref));
printf("    frekvch1/2/3: ");
for(ix=0; ix<3; ix++) {
  w32 fhl;
  w16 flow, fhigh;
  flow= vmxr16(vsp, ch1_freq_low+(ix*4));
  fhigh= vmxr16(vsp, ch1_freq_high+(ix*4));
  fhl= flow | (fhigh <<16);
  frekvs[ix]= (80*16*22)/(flow+ (fhigh*65536.));
  printf("%d=%fMHz ", fhl, frekvs[ix]);
}; printf("\n");
rc= vmxclose(vsp);
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
//cf= openFile(fname,"r");
cf= fopen(fname,"r");
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
      infolog_trg(LOG_ERROR, (char *)"Bad value in dbctp/clockshift file");
    } else {
      if((halfns != dbhalfns) || (cordeval != dbcordeval)) {
        i2cset_delay( BC_DELAY25_BCMAIN, dbhalfns);
        i2cset_delay( BC_DELAY25_BC1, dbhalfns);  //keep the same for BPIM
        corde_set(CORDE_DELREG, dbcordeval);
        shiftCommentInDAQ(halfns, cordeval, dbhalfns, dbcordeval,(char *)"");
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
infolog_SetStream((char *)"",0);
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
} else if(maino==12) {
  vmew32(BCmain_MAN_SELECT, 3);  /* 3: BC1 input */
  vmew32(ORBmain_MAN_SELECT, 1); /* 1: ORB2 input */
  sprintf(msg, "Global clock: %x/%x -> BEAM1/ORB2",bcmain,orbmain);
  strcpy(daqlog,"BEAM1/ORB2");
} else {
  printf("Bad maino input. No action\n"); return;
};
        // always, after clock change resynchronize DLL on RF2TTC:
//sprintf(msg, "%s + DLL resync", msg);
sprintf(msg, "%s note: DLL NOT resynchronised", msg);
//DLL_RESYNC(0);        //vmew32(BC_DELAY25_GCR, 0x40);
infolog_trg(LOG_INFO, msg);
//rcdl= daqlogbook_open();
//if(rcdl==0) {
//  rcdl= daqlogbook_add_comment(0,"CLOCK",daqlog);
//  daqlogbook_close(); 
//};
com2daq(0,(char *)"CLOCK",daqlog);
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
  com2daq(0,(char *)"CLOCK",(char *)"DLL_RESYNC");
  //int rcdl;
  //rcdl=daqlogbook_open();   //rcdl must be 0 if opened
  //if(rcdl==0) {
  //  rcdl= daqlogbook_add_comment(0,"CLOCK","DLL_RESYNC");
  //  daqlogbook_close(); 
  //};
} else if(msg==DLL_info) {
  //infolog_SetFacility((char *)"CTP"); set in ttcmidims.c
  //infolog_SetStream("",0);
  infolog_trg(LOG_INFO, (char *)"CLOCK DLL_RESYNC done");
};
}
void micrate(int present) {
if(present==-1) {
  if(envcmp((char *)"VMESITE", (char *)"ALICE")==0) {
    havemicrate= 7;  // 7: CORDE RF2TTC RFRXs
  } else {
    char *hn;
    hn= getenv("HOSTNAME");
    if(strcmp(hn, "altri2")==0) {
      havemicrate= 2;  // RF2TTC only
    } else {
      havemicrate= 0;
    };
  };
} else {
  havemicrate= present;
};
}
int micratepresent() {
return(havemicrate);
}
