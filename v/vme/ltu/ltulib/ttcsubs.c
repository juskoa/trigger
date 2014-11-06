#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "vmewrap.h"
#include "vmeblib.h"
#include "ltu.h"
#include "../../ttcvi/ttcvi.h"

extern char BoardBaseAddress[];
/* TTCvi board parameters: */
int vspg;
w8 Knm[240];
#define fine12dt 104.17   // Fine1/Fine2 delay step

/* 
rc: 255: bad ps value (should be in picosecs)
   <240: ok
*/
w8 ps2nm(int ps) {
int k;
k= rounddown(ps/fine12dt);
//printf("ps2nm: ps:%d k:%d rc:%d\n", ps, k, Knm[k]); 
if(k>239) return(255);
return(Knm[k]); 
}
void initStatic() {
int k,n,m;
for(n=0; n<=14; n++) {
  for(m=0; m<=15; m++) {
    w8 nm;
    k= (m*15 + n*16 + 30)%240;
    nm= (n<<4) | m;
    if(k<240) {
      Knm[k]= nm;
    } else {
      printf("ttcDefaults ERROR: k:%d nm:%d (0x%x)\n", k, nm, nm);
    };
  };
};
/*for(k=0; k<240; k++) {
  printf("k:%d nm:%d (0x%x)\n", k, Knm[k], Knm[k]);
};*/
}
/*----------------------------------------------------------- ttcDefaults()
*/
void ttcDefaults(Tltucfg *tp) {
tp->Sbgo0delay=53;   // stdalone
tp->Gbgo0delay=53;   // global
tp->ppdelay=3125;   // from ttcvi.h TTCppdelay
tp->Gppdelay=3394;
tp->FineDelay1=3126;      // is 0 value in register
tp->FineDelay2=16459;     // is 0x80 value in register
tp->CoarseDelay=0;
}
/*----------------------------------------------------------- openvmeTTCvi()
rc: vsp (vme space number to be use in 'vmx*(...' calls
    -1: vme not opened, error printed to stdout
*/
int openvmeTTCvi() {
int vsp, rc; char xcb[]="0xcb";
char TTCviba[40];    // TTCvi base address
strcpy(TTCviba, BoardBaseAddress);
if(ltuviyes) {
  printf("Error: openvmeTTCvi should not be called (no TTCvi with LTUvi)\n"); 
  return(9);
};
if(strncmp(TTCviba,"VXI0::",6)==0) {
  char c;
  c= TTCviba[8]; c= c+1;
  TTCviba[8]= c;
} else {
  TTCviba[3]= '0';
};
vsp=-1; rc= vmxopen(&vsp, TTCviba, xcb);
if(rc!=0) {
  printf("vmxopen error:%d opening TTCvi vme\n",rc);
  vsp=-1;
};
return(vsp);
}
/*----------------------------------------------------------- ttcFEEreset()
*/
int ttcFEEreset() {
int vsp, rc=-1;

if(ltuviyes) {
  vmew32(TTC_DATA, 0x80018000);   // 0x8000 data: 0x000
  rc=0;
} else {
  vsp=openvmeTTCvi();
  if(vsp!=-1) {
    vmxw32(vsp, BCDBG2, 0x80018000);
    vmxclose(vsp);
    rc=0;
  };
};
return(rc);
}
/*----------------------------------------------------------- ttcFEEcmd()
*/
int ttcFEEcmd(int Command) {
int vsp, rc=-1; w32 dawo;
dawo= 0x80010000 | ((Command&0xf)<<12);   //was <16 -bad 12.6-26.11/2008 
if(ltuviyes) {
  vmew32(TTC_DATA, dawo);
  rc=0;
} else {
  vsp=openvmeTTCvi();
  if(vsp!=-1) {
    //vmxw32(vsp, BCDBG2, 0x80018000);   // 0x8000 data: 0x000
    vmxw32(vsp, BCDBG2, dawo);   // 0x.000 data: 0x000
    rc=0;
    vmxclose(vsp);
  };
};
return(rc);
}
/*----------------------------------------------------------- ttcPPdelay()
*/
int ttcPPdelay(int bc) {
int vsp, rc=-1;
if(ltuviyes) {
  vmew32(PP_TIME, bc);
  rc=0;
} else {
  vsp=openvmeTTCvi();
  if(vsp!=-1) {
    //ltucfg.ppdelay=bc;   // defaults not changed
    vmxw16(vsp, IDel1, bc);
    vmxclose(vsp);
    rc=0;
  };
};
return(rc);
}
/*----------------------------------------------------------- ttcDelays()
*/
int ttcDelays(int Fine1ps, int Fine2ps, int Coarse) {
int vsp, rc=-1;
w32 f1val, f2val;
if(ltuvino) {
  vsp=openvmeTTCvi();
  if(vsp==-1) return(rc);
};
f1val= ps2nm(Fine1ps); f2val= ps2nm(Fine2ps);
if((f1val==255) || (f2val==255)) {
  printf("Bad value: Fine1/2 expected in picsecs (0-24000)\n");
} else {
};
if(ltuviyes) {
  vmew32(TTC_DATA, 0x80000000 | f1val);
  vmew32(TTC_DATA, 0x80000100 | f2val);
  vmew32(TTC_DATA, 0x80000200 | Coarse);
  printf("Fine1 Fine2 Coarse regs:0x%x 0x%x 0x%x\n", f1val, f2val, Coarse);
} else {
  vmxw16(vsp, BCLFACttcrxadr, 0x8000);   /* access internal regs */
  usleep(1000);
  vmxw16(vsp, BCLFACdata, 0x0000 | f1val );       /* Fine Delay1  */
  usleep(1000);    // has to be here! (Desk2 is not set without this line)
  vmxw16(vsp, BCLFACdata, 0x0100 | f2val );       /* Fine Delay2  */
  usleep(1000);
  vmxw16(vsp, BCLFACdata, 0x0200 | Coarse);       /* Coarse delay*/
  usleep(1000);
  printf("Fine1 Fine2 Coarse regs:0x%x 0x%x 0x%x(1ms between vme writes)\n", f1val, f2val, Coarse);
};
rc=0;
if(ltuvino) vmxclose(vsp);
return(rc);
}
/*----------------------------------------------------------- bgoinit()
bgc      -Bgo channel (0..3)
del, dur -delay, duration for this channel
mode     - mode of the channel
data     -data to be written into B Channel Data FIFO
Modified registers:
chan delayreg durreg modereg B-go(data)
0    0x92     0x94   0x90    0xb0      Orbit (Bgo driven)
1    0x9a     0x9c   0x98    0xb4      PP (Bgo driven)
2    0xa2     0xa4   0xa0    0xb8      L1/L2 data (FIFO driven)
*/
void bgoinit(int bgc, w16 del, w16 dur, w16 mode, w32 data) {
w32 delreg, durreg,modereg,bcdbgoreg;
w32 bgc8;
bgc8=bgc*8;
delreg= IDel0+ bgc8;
durreg= IDur0+ bgc8;
modereg= BGo0mode+ bgc8;
bcdbgoreg= BCDBG0+ bgc*4;
vmxw16(vspg, delreg, del);
vmxw16(vspg, durreg, dur);
vmxw16(vspg, modereg, mode);
if(data != 0xffffffff) {
  vmxw32(vspg, bcdbgoreg, data);
  printf("Bgo%d  delay:%d duration:%d mode:%x data:%x\n",
    bgc, del, dur, mode, data);
} else {
  printf("Bgo%d  delay:%d duration:%d mode:%x data:not set/sent\n",
    bgc, del, dur, mode);
};
}
/*---------------------------------------------------- */
w32 r16x4(w32 fromadr) {
w32 i,rc=0;
for(i=0; i<=12; i=i+4) {
  rc= rc<<8;
  rc= rc | (vmxr16(vspg, fromadr+i) & 0xff);
};
return(rc);
}

/*------------------------------------ checkvmeend() */
int avrg,avrgn;
void checkvmeend() {
int ix; w16 csr1;
for(ix=0; ix<100; ix++) {
  csr1= vmxr16(vspg, CSR1);
  if((csr1 & 0x80)==0) break; // VME request is not pending 
};
avrg=avrg+ix; avrgn++;
}
/*------------------------------------------------------------ TTCrxregs() */
w16 fide1,fide2,fide3;
void TTCrxregs(Tltucfg *ltc) {
//Tltucfg *ltc = &ltushm->ltucfg;
fide1= ps2nm(  ltc->FineDelay1)&0xff;
fide2= ps2nm(  ltc->FineDelay2)&0xff;
fide3=   ltc->CoarseDelay &0xff;
if(ltuviyes) {
  vmew32(TTC_DATA, 0x80000000 | fide1);
  vmew32(TTC_DATA, 0x80000100 | fide2);
  vmew32(TTC_DATA, 0x80000200 | fide3);
  vmew32(TTC_DATA, 0x800003f9);
} else {
  checkvmeend(); vmxw16(vspg, BCLFACttcrxadr, 0x8000);/*access internal regs*/
  checkvmeend();vmxw16(vspg, BCLFACdata, 0x0000 | fide1); /* Fine Delay1  */
  checkvmeend();vmxw16(vspg, BCLFACdata, 0x0100 | fide2); /* Fine Delay2  */
  checkvmeend();vmxw16(vspg, BCLFACdata, 0x0200 |fide3);  /* Coarse delay*/
  /*25.10: last 2 bits of Control register have to be '01' -the
  trigger mode on TTCrx as used in ALICE experiment */
  checkvmeend();vmxw16(vspg, BCLFACdata, 0x03f9);/*TTCrx control reg3, f9:data*/
};
}
/*------------------------------------------------------------ TTCrxreset() */
void TTCrxreset() {
if(ltuviyes) {
  vmew32(TTC_DATA, 0x800006ff);
} else {
  vmxw16(vspg, BCLFACttcrxadr, 0x8000);   /* access internal regs */
  checkvmeend(); vmxw16(vspg, BCLFACdata, 0x06ff);       // TTCrx reset(p.21) 
};
}
/*------------------------------------------------------------ TTCinit() */
int TTCinit() {
//debug ludim:
/*
printf("debug TTCinit now ait 0sec\n");
usleep(1000000); printf("debug TTCinit 1sec\n");
usleep(1000000); printf("debug TTCinit 2\n");
usleep(1000000); printf("debug TTCinit 3\n");
usleep(1000000); printf("debug TTCinit 4\n");
usleep(1000000); printf("debug TTCinit 5\n");
usleep(1000000); printf("debug TTCinit 6\n");
//usleep(1000000); printf("debug TTCinit 7\n");
//usleep(1000000); printf("debug TTCinit 8\n");
//usleep(1000000); printf("debug TTCinit 9\n");
*/
usleep(6000000) ; printf("debug TTCinit 1st multiline in one go\n\
debug TTCinit 2nd  line in one go\n\
debug TTCinit 3rd  line in one go\n\
debug TTCinit 4th  line in one go\n\
debug TTCinit 5th  line in one go\n\
debug TTCinit 6th  line in one go\n\
");
return(1);
Tltucfg *ltc = &ltushm->ltucfg;  // always use SHM int TTCinit() -interactive
return(TTCinitgs(1, 5, ltc));    //  and TTCinit() through DIm service
}
/*------------------------------------------------------------ TTCinitgs()
Initialize TTCvi, TTCrx. 
Has to be called always after power up of TTCvi or TTCrx.
It is called automatically from ltuInit()
I: BoardBaseAddress is the corresponding LTU base address
stdalone:1 in stdalone, 0 in global
feetimeout: timeout waiting for FE ready in seconds
rc: 0: ok
   -1: vmxopen() error
    1: not ready 10ms after FEEreset
*/
int TTCinitgs(int stdalone, int feetimeout, Tltucfg *ltc) {
//Tltucfg *ltc = &ltushm->ltucfg;
int rc, ix100, wasbusy0, wasbusy1, wasbusy2, wasbusy3, wasbusy4;
w32 ltubusy, secs1,secs2,mics1,mics2, secs0,mics0,secs3,mics3;
w32 timebusy1, timebusy2, timebusy3, timebusy4, timebusy44;
w32 WaitAfterReset=20000; // in milsecs
w32 QPLLwait=2000000;
w32 busytime,busycount;
w32 cntmem[LTUNCOUNTERS];
w32 cntmem2[LTUNCOUNTERS];  char busy1note[80];
char rxreset[20];
char rxnotready[20];
//char byhis[101];
if(ltuvino) { 
  vspg= openvmeTTCvi(); 
  if(vspg==-1) return(-1);
  printf("TTCinitgs: TTCvi vme space %d opened\n",vspg);
};
avrg=0; avrgn=0;
rc=0;
if(ltuvino) {
/*--------------------------------------------------- TTCvi init*/
  vmxw16(vspg, CSR2,0xfc00); usleep(1000);
  vmxw16(vspg, CSR1,0x8047);  /* Orbit Counting, L1AFIFO reset, disable L1 */
  usleep(1000);
  //bgoinit(0,  53, 54, 0x0c, 0x00800000);  /* BCreset */
  if(stdalone!=0) {
    bgoinit(0, ltc->Sbgo0delay, 54, 0x0c, 0x00800000);  /* BCreset */
    /* BGo-1: Prepulse. */
    bgoinit(1,   ltc->ppdelay, 54, 0x08, 0x7e000000); /*Sys+User message bits: all on*/
  } else {
    bgoinit(0, ltc->Gbgo0delay, 54, 0x0c, 0x00800000);
    bgoinit(1,   ltc->Gppdelay, 54, 0x08, 0x7e000000); /*Sys+User message bits: all on*/
  };
  /* BGo-2: L1/2 messages, 0x3: front panel input disabled, async cycles */
  bgoinit(2,    0, 0, 0x03, 0xffffffff); 
};
ltubusy= vmer32(BUSY_STATUS);
if(ltubusy&0x80) { 
  printf("Warning: BUSY is ON before TTCrx init\n");
  wasbusy0=1;
}else {
  printf("BUSY is OFF before TTCrx init\n");
  wasbusy0=0;
};
/*--------------------------------------------------- TTCrx init */
GetMicSec(&secs0, &mics0);
readCounters(cntmem, LTUNCOUNTERS, 0);
//printf("counters read\n");
if(ltc->ttcrx_reset!=2) {  // INIT:2 (no reset)
  TTCrxreset();
  WaitAfterReset=20000; QPLLwait=2000000; 
  strcpy(rxreset, "TTCrx_reset");
  strcpy(rxnotready, "TTCrx not ready");
} else {
  WaitAfterReset=0; QPLLwait=0; // no need to wait (no reset)
  strcpy(rxreset, "           ");
  strcpy(rxnotready, "               ");
};
wasbusy1=0; //0: busy was not rasied during 'TTCrx not ready' interval (~ 6ms)
wasbusy2=1; //0: busy was not ON always during 'QPLL locked OFF' interval
wasbusy4=0; //1: busy was ON 10ms after FEEreset
GetMicSec(&secs1, & mics1);
// SDD needs 100BCs (2.5us) to deliver BUSY after TTCrxreset, here is SSM taken:
//   39: ChanB ZERO:0x0000 0 0x0 0x6ff 0xca  
//  139: SBUSY/1048437 
usleep(10);   //wait some time for BUSY   (see SDD comment above)
ltubusy= vmer32(BUSY_STATUS);
if(ltubusy&0x80) { wasbusy1=1; };
GetMicSec(&secs2, &mics2); 
/*1: just after TTCrxreset */
timebusy1= rounddown(DiffSecUsec(secs2,mics2,secs0,mics0)/1000.);
for(ix100=0; ix100<30; ix100++) {   // wailt 20ms after TTCrx reset
  GetMicSec(&secs2, &mics2);
  if(DiffSecUsec(secs2,mics2,secs1,mics1) >WaitAfterReset) break;
  usleep(1000);
};

//vmew32(BUSY_ENABLE,0); // just to see on scope when we finished

/* After TTCrx reset, TTCrx goes to not ready state -
TTC is not ready ~ 6milisecs after TTCrx reset (measured 31.10.2007).

SSD: 
After FE off/on it is necessary, before programming TTCrx control reg.3:
- to do hw-TTCrx reset (pin on TTCrx) or
- to disconnect/connect fibre */

TTCrxregs(ltc);
//printf("ttcsubs.c:TTCVi regs written\n");
/* following usleeps necessary for correct FineDelay2 setting */
/* BGo-0: Orbit -> BCntRes signal on TTCrx.
26.6.06: BCntRes delayed to LTU.orbit by: 53+48.
   44 leads to missing BCntRes signal
15.9.06
   49 -leads to the occassional shift of BCntRes signal. Set to 54
*/
/* now we should wait:
>1s -in case of QPLL clock used
0s -in case of ClockDes1 used */
ltubusy= vmer32(BUSY_STATUS);
if((ltubusy&0x80)==0) {
  wasbusy2=0; // BUSY OFF at the start of waiting for 'QPLL locked' signal
};
//printf("ttcsubs.c:wasbusy2:%x\n",wasbusy2);
GetMicSec(&secs3, &mics3); 
/*2: just after TTCrxconfig */
timebusy2= rounddown(DiffSecUsec(secs3,mics3,secs0,mics0)/1000.);
//printf("ttcsubs.c:timebusy2:%x\n",timebusy2);
while(1) {
  w32 dif;
  GetMicSec(&secs2, &mics2);
  dif= DiffSecUsec(secs2,mics2,secs3,mics3);
  if(dif >QPLLwait) {
    //printf("ttcsubs.c:1.1sec ok dif:%d\n", dif);
    break; 
  };
  //usleep(100000);
  //printf("secs2/mics2 secs3/mics3 dif: %d/%d %d/%d =%d\n", secs2,mics2,secs3,mics3,dif);
  usleep(300000);
};
GetMicSec(&secs2, &mics2); 
//printf("3: %d %d\n", secs2, mics2);
/*3: just before FEEreset */
timebusy3= rounddown(DiffSecUsec(secs2,mics2,secs0,mics0)/1000.);
ltubusy= vmer32(BUSY_STATUS);
if((ltubusy&0x80)==0) {wasbusy3=0;
} else {
  wasbusy3=1;
};
if(ltuviyes) {
  vmew32(TTC_DATA, 0x80018000);
} else {
  checkvmeend(); vmxw32(vspg, BCDBG2, 0x80018000);  // ttcFEEreset
};
/*vmxw32(vspg, BCDBG2, 0x80018000); just check. It comes even
  without checkvmeend() , cca 1.2us after the 1st one */
GetMicSec(&secs1, &mics1);
while(1) {
  usleep(10000);
  GetMicSec(&secs2, &mics2);
  timebusy44= DiffSecUsec(secs2,mics2,secs1,mics1);
  ltubusy= vmer32(BUSY_STATUS);
  if((ltubusy&0x80)==0) break;
  if(w32toint(timebusy44) >(feetimeout*1000000)) {
    wasbusy4=1; break;    // BUSY ON after FEEreset timeout
  };
};
//printf("ttcsubs.c:feetimeout:%d\n", feetimeout);
/* 4: when READY or BUSY too long (timeout) */
//printf("dbg4: %d %d\n", secs2, mics2);
timebusy4= rounddown(DiffSecUsec(secs2,mics2,secs0,mics0)/1000.);
if(ltuvino) vmxclose(vspg);

printf("FineDelay1:%dps (TTCrx:0x%x) FineDelay2:%dps (TTCrx:0x%x) CoarseDelay:0x%x\n",
  ltc->FineDelay1, fide1, ltc->FineDelay2, fide2, fide3);
/*
printf("Average ix in waitvmeend:%f\n", 1.0*avrg/avrgn);
printf("BUSY after TTCrx_reset (1 char ~ 500 micsecs):\n");
for(ix100=0; ix100<maxix100; ix100++) {
  printf("%c", byhis[ix100]);
  if(((ix100+1) % 10)==0) printf("\n");
}; priiiintf("%s\n", byhis);*/
if(wasbusy1==0) {
  printf("Error: BUSY not raised during 'TTCrx READY' off\n");
} else {
  printf("OK: BUSY raised during 'TTCrx READY' off\n");
};
if(wasbusy2==0) {
  printf("Warning: BUSY not raised during 'QPLL LOCKED' off (if QPLL clock is used)\n");
} else {
  printf("BUSY raised during 'QPLL LOCKED' off\n");
};
if(wasbusy4==1) {
  rc=1;
  printf("Error: BUSY ON %d milsecs after FEEreset\n", timebusy44/1000);
};
printf(" milsecs   action           BUSY\n");
printf("       0   %s      %d\n", rxreset, wasbusy0);
printf(" %7d   %s  %d  \n", timebusy1, rxnotready, wasbusy1);
printf(" %7d   TTCrx config     %d\n", timebusy2, wasbusy2);
printf(" %7d   FEE reset        %d\n", timebusy3, wasbusy3);
printf(" %7d                    %d\n", timebusy4, wasbusy4);
/* we cannot use accruals here! (because of using them in counters.py widget)*/
readCounters(cntmem2, LTUNCOUNTERS, 0);
busytime= dodif32(cntmem[BUSY_TIMERrp], cntmem2[BUSY_TIMERrp]);
busycount= dodif32(cntmem[BUSY_COUNTERrp], cntmem2[BUSY_COUNTERrp]);
busy1note[0]='\0';
if((busytime>0) || (busycount>0)) {
  float micsecs; int ts;
  wasbusy1=1;
  ts= busycount; micsecs= busytime*0.4;
  sprintf(busy1note,"Busy_ts:%d ON:%7.1f micsecs",ts,micsecs);
};
printf("%s\n",busy1note);
return(rc);
}
