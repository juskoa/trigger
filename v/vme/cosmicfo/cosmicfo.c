/*BOARD ltu 0x819000 0x800 */
/* BOARD ltu "VXI0::261" 0x800 */
/*
26.08.2004 SLMstart() -if emulation is active, SLMquit() is called
25.11. If VXI, then address has to be: "VXI0::ddx"  where ddx
   is decimal number higher by 1 from corresponding LTU VXI0::lll
   address (i.e. ddx= lll+1). x can be: 1,2,3,4,5,6,7,8,9
 7.10. LTUinit() now initialises properly (before not, because
       of check the /tmp/ltu0xBASE_busy file)
 7.10. FP2ssm mode, 'L0 over TTC', modes added
       Orbit-error, PLL_RESET now possible
10.10. LTU_SW_VER 1.4
28.4.2006 LTU_SW_VER "1.4.2"   (only for trigtest@epmp3) 
LTU_SW_VER "1.4.3 17.10.2006"
5.1.2007 LTU_SW_VER "1.5.0 8.1.2007"     LTU FPGA: F3
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../../vmeb/vmeblib/lexan.h"
#include "vmewrap.h"
#include "vmeblib.h"

#define LTUMAIN
#include "cosmicfo.h"

int mallocShared(w32, int, int *);
double rnlx();
w32 dodif32(w32 before, w32 now);
void setseeds(long,int);

int  SSMsetom(w32 opmode);
w32 ERgetselector();

extern int quit;
extern char BoardBaseAddress[];
extern w32 SSMSCHEDULER;/* SSM recording planning, 2-After, 3-Before, 0-no plan */

w32 Gltuver;   // should be 0xf3

/* HIDDEN SLM SSM EmuTests FrontPanel ConfiguratioH */
/*HIDDEN SLM SSM ADC EmuTests FrontPanel ConfiguratioH ExpertConf SHM */
/*------------------------------------------------------- Configuration */
/* FGROUP Configuration GUI Flash_upgrade
*/
/* FGROUP "Configuration" GUI AD_scan
*/
/*FGROUP TOP GUI RandomGenerators
*/
/* FGROUP TOP GUI CTP_Emulator "CTP Emulator"
CTP emulator:
- start Sequence list editor
- load sequence list
- set signal error rate, enable/disable errors
  ask for 'on demand' error
- START signal generation selection
- start/break/quit Sequence execution
*/

/*FGROUP ConfiguratioH */
void prtfnames(char *directory, char*suffix);

/*FGROUP ConfiguratioH
*/
void waitKB() {
char line[100];
fgets(line,99,stdin);
}

/*FGROUP ConfiguratioH
*/
void setglobalmode();
/*FGROUP Configuration GUI setCurrents "Defaults editor"
Current settings are kept in shared memory and are accessible
by ECS through ltu_proxy or by 'vmecrate -Trigger control client'.
Current settings will be loaded into LTU/TTC boards:
   - at the start of the run through ECS or 
   - by LTUinit/TTCinit buttons (TTCinit actions are 
     part of LTUinit actions)

Save/Load memory buttons allows you to modify these current settings.
During editing, the values different from 'default values' are shown
with yellow background color.

Load defaults/Save as defaults buttons allow the modification of
settings in database. Use 'Save as defaults' button ONLY WHEN
YOU WANT CURRENT SETTINGS TO BE SAVED AND USED AFTER POWERING UP 
YOUR PARTITION.
The settings saved in database are loaded AUTOMATICALY
into Current settings and LTU/TTC boards (TTCvi+TTCrx chip) when
the crate is powered ON.
*/
/* FGROUP Configuration GUI setDefaults "Defaults(DB)"
Default settings are saved in database. 
These settings are loaded only once 
into Current settings and LTU/TTC boards (TTCvi+TTCrx chip) the when
the crate is powered ON.

*/
/*FGROUP ConfiguratioH
*/
int getsgmode();
/*FGROUP ConfiguratioH
*/
void setstdalonemode();

/*FGROUP FrontPanel
*/
void getCounters(int Ncounters, int accrual);
/*FGROUP FrontPanel
*/
void clearCounters();
/*FGROUP ConfiguratioH
rc: Dial switch value X, (char: 0,1,...,f) i.e. it can be used to form
    '0x1X' input string for setDestination() (see ltutmx.c)
*/
char getSwitchValue() {
char rc;
printf("base:%s\n",BoardBaseAddress);
rc=BoardBaseAddress[4];
return(rc);
}

/*FGROUP ExpertConf
Set (mode=1) or clear(mode=0) 'Timing test mode'.
In Timing test mode, L1 Data signal of the emulator
(i.e. valid only in STDALONE) is continuously toggled.

This mode is used with another (slave) LTU, connected through
special cable connecting the backplane signals of this LTU
to its CTP inputs connector. 
Slave LTU, switched to GLOBAL, can measure
(Configuration->ADC_Scan) delay, which should be kept with
current cabling.
*/
void TTmode(int mode) {
vmew32(TIMING_TEST, mode);
printf("EMU_STATUS:%x\n", vmer32(EMU_STATUS));
}
/*FGROUP ExpertConf
Normally, destination VME address for B-channel data is
TTCvi's 'B Channel Data for B-Go<2>' (0x80x0B8). 
Instead of TTCvi board, another LTU can become 'the receiver of
data sent over VME by setting destination to 0x1X 
where X is dial of receiving LTU.
Receiving LTU must have its snapshot memory
enabled for 'VME write' -> by starting setSSMVMEW() on this LTU.

Set destination to 0 for normal mode of operation (with TTCvi).
*/
void setDestination(w32 destination) {
vmew32(MASTER_MODE, destination);
}
/*FGROUP ExpertConf
Set Snapshot memory of this LTU to 'VME write' mode. This should
be done before starting 'setDestination()' on another LTU board,
redirecting MASTER vme writes to this board.
*/
void setSSMVMEW() {
vmew32(SSMcommand, 1);
return;
if(SSMsetom(1)) {   /* VME access, write */
  printf("set of Snapshot memory to 'VME write mode' not successfull\n");
};
}
#define Mega (1024*1024)
//von Tltushm *shmbase=NULL;
w32 *shmcnts=NULL;
/*FGROUP SHM
rc: 0: ok
    1: error reading counters
*/
int readCNTS2SHM() {
w32 secs, mics;
if(shmcnts==NULL) return(1);
/* for(ix=0; ix<10; ix++) {
  shmcnts[ix]=ix;
  //printf("readCNTS2SHMix:%d\n",ix);
};
return(0); */
GetMicSec(&secs, &mics);
/*
ctpc[CSTART_SPEC+2]= vmer32(L2_ORBIT_READ);
readCounters(ctpc, NCOUNTERS, 0); readTVCounters(&ctpc[CSTART_SPEC+3]);
//printf("readctpcounters: readCounters ok\n");
ctpc[CSTART_SPEC]= secs; ctpc[CSTART_SPEC+1]= mics;
//ctpc[13]= getCounter(1,13); ctpc[CSTART_L1+5]= getCounter(2,5);
*/
readCounters(shmcnts, LTUNCOUNTERS, 0);  // LTU counters
shmcnts[temperaturerp]= ReadTemperature();
shmcnts[epochsecsrp]= secs;
shmcnts[epochmicsrp]= mics;
return(0);
}
/*FGROUP Configuration GUI CheckBusy "Check/set/reset BUSYs" 
Shows: status of all the signals contributing to BUSY output
Allows: to enable/disable BUSY1, BUSY2 inputs and set/clear
        SOFTWARE BUSY
*/

int mem2ok=0;    // 0: not initialised
w32 mem1[LTUNCOUNTERS];   // place for 24 LTU counters
w32 mem2[LTUNCOUNTERS];
/*FGROUP SimpleTests 
Rough counting with 0.4 usecs precision (more signals in 0.4us are
counted as 1 signal).
Enable only 1 busy to get precise rate for the enabled BUSY -
transitions/secs measures resulting BUSY signal (which is OR
of enabled BUSY inputs).
*/
void inputRates() {
w32 busy1,busy2,bts,timedif;
float timeint,avrate1,avrate2, avbts;
int ic;
char secs[10];
if(mem2ok==0) {
  readCounters(mem1, LTUNCOUNTERS, 0); sleep(1); mem2ok=1;
};
readCounters(mem2, LTUNCOUNTERS, 0);
timedif= dodif32(mem1[LTU_TIMErp], mem2[LTU_TIMErp]);
timeint= timedif*0.0004;  // in milisecs
busy1= dodif32(mem1[SUBBUSY1_TIMERrp], mem2[SUBBUSY1_TIMERrp]); 
busy2= dodif32(mem1[SUBBUSY2_TIMERrp], mem2[SUBBUSY2_TIMERrp]); 
bts= dodif32(mem1[BUSY_COUNTERrp], mem2[BUSY_COUNTERrp]); 
//printf("inputRates: %d %d %d\n", timedif, busy1, busy2);
busy1= timedif-busy1; busy2= timedif-busy2;
/*busy1/2: time 'not busy' in 0.4us units. If we count
0.4us unit as 1 trigger (not truth -they are random!), the
number of triggers is: busy1/0.4
number of triggers/sec is: (busy1/0.4)/timeint*1000
*/
/*avrate1= (busy1/timeint)*1000.;   // per second
avrate2= (busy2/timeint)*1000.;*/
avrate1= (busy1/0.4/timeint)*1000.;   // per second
avrate2= (busy2/0.4/timeint)*1000.;
avbts=1000.*bts/timeint;
if(timeint>1000) {
  timeint=timeint/1000.; strcpy(secs,"secs    ");
}else { strcpy(secs,"milisecs");};
printf(
"Interval: %5.2f %s busy1(multi):%d rate per sec.:%f\n\
                         busy2(TPClaser ):%d rate per sec.:%f\n\
 transitions/secs: %f\n",
  timeint, secs, busy1, avrate1, busy2, avrate2, avbts);
for(ic=0; ic<LTUNCOUNTERS; ic++) mem1[ic]= mem2[ic];
return;
}

int blinkv=0;
/*FGROUP SimpleTests 
Click to Stop/Start blinking
front panel LEDs
  */
void TestLEDS() {
blinkv=vmer32(TEST_ADD);	
blinkv=~blinkv;
vmew32(TEST_ADD, blinkv);
}

w32 slbin=0; 
/*FGROUP ConfiguratioH 
Print 1 line string xxxx
where x is the status (0/1) of software LED word
*/
void getSWLEDS() {
int ix;
char sl[5]="0000";
slbin= vmer32(SOFT_LED);
//           slbin=slbin+1;
for(ix=0; ix<4; ix++) {
  if(slbin &(1<<ix)) sl[ix]='1';
};
printf("%s\n", sl);
}
/* FGROUP SimpleTests
Test busy logic. Both BUSY inputs on front panel should be disconnected
(i.e. they are in BUSY state).
Operation:
- clear BUSY_COUNTER and both timers
- loop over the following (number of loops is given in busys parameter):
  - start BUSY signal (as required by source parameter)
  - sleep (as required by length parameter)
  - stop BUSY signal
Parameters:
source -source of busy signal. More sources (e.g. 7 -> all)
        can be specified by combining 3 least significant bits.
        1 -Enable BUSY1
        2 -Enable BUSY2
        4 -generate software busy
busys  -number of generated BUSY signals
length -an approximate length of each BUSY signal 
        in 1 micsecs intervals
clearcnts -0 don't clear BUSY_COUNTER and both timers, and leave BUS1,2,sw
             untouched
           1       clear BUSY_COUNTER and both timers, disable
	     source at the beginning
*/
void busylogic(w32 source, int busys, int length, int clearcnts) {
int nbs;
w32 busyinputs,bt,sbt;
if(clearcnts) {
  setBUSY(0);          /* disable BUSY inputs */
  vmew32(SW_BUSY,0);   /* disable sw BUSY */
  clearCounters();   /* clear ALL  (should be only BUSY_COUNTER and both timers */
};
busyinputs= source&0x3;
for(nbs=0; nbs<busys; nbs++) {
  if( busyinputs!=0) setBUSY(busyinputs);     /* BUSY ON */
  if( (source & 0x4) !=0) vmew32(SW_BUSY, 1);
  micwait(length);
  /*
  for(sleeps=0; sleeps<length; sleeps++) {
    micwait(1);
  };
  */
  if( busyinputs!=0) setBUSY(0); 
  if( (source & 0x4) !=0) vmew32(SW_BUSY, 0);   /* BUSY OFF */
  if(quit!=0) break;
};
bt= getCounter(BUSY_TIMERrp);
sbt=getCounter(SUBBUSY1_TIMERrp);
printf("BUSY_COUNTER:%d\n   BUSY_TIMER:%d(%f ms)\nSUBBUSY1_TIMER:%d(%f ms)\n",
  getCounter(BUSY_COUNTERrp), bt, bt*0.0004, sbt, sbt*0.0004);
}
/*FGROUP SimpleTests
print: 
CODE_ADD       always 0x56 for LTU
SERIAL_NUMBER  of the LTU board
VERSION_ADD    VME FPGA version firmware
LTUVERSION_ADD LTU FPGA version firmware
BC_STATUS      BC ststus (should be 0x2 == PLL locked, BC ok)
RC: 0: OK
    1: bad CODE_ADD (exit should follow)
 */
int printversion() {
w32 code,vmever,ltuver,pll;
code= 0xff&vmer32(CODE_ADD);
vmever= 0xff&vmer32(VERSION_ADD);
ltuver= 0xff&vmer32(LTUVERSION_ADD);
printf("LTUcode:0x%x serial#:0x%x VME ver:0x%x LTUfpga:0x%x\n",
  code, 0xff&vmer32(SERIAL_NUMBER), vmever, ltuver);
printf("LTUsw version:%s\n", LTU_SW_VER);
if( code!=0x56) {
  printf("\n UNEXPECTED LTUcode (0x56 expected) \n\n\n");
  return(1);
};
if( (ltuver!=0xcc) || (ltuver!=0xcd)) {
  printf("\n UNEXPECTED cosmicfo FPGA version:%x! \n\n", ltuver);
};
pll= vmer32(BC_STATUS)&0x7;
printf("BC_STATUS: %x: ", pll);
if( pll == BC_STATUSpll ) {
  printf("BC and Orbit input signals OK\n");
};
if( (pll & BC_STATUSpll) == 0) {
  printf("BC input signal not present\n");
};
if( (pll & BC_STATUSerr) == 0x01) {
  printf("errorneous BC -check BC LED\n");
};
if( (pll & BC_STATUSorbiterr) == BC_STATUSorbiterr ) {
  printf("errorneous Orbit input signal\n");
};
return(0);
}
/*FGROUP SimpleTests
Generate trigs triggers. Each Trigger is followed by the busy signal wide
'busylength' miliseconds. Operation:
-disable both BUSY inputs
-load L2a sequence into emulator memory
-start emulation
-generate required number of triggers (Software triggers)
*/
void GenTrigBusy(int trigs, int busylength) {
int n,sleepms=0;
setBUSY(0);
SLMquit();
SLMload("CFG/ltu/SLM/L2a.seq");
SLMstart();
for(n=0; n<trigs; n++) {
  vmew32(SOFT_TRIGGER, DUMMYVAL);
  setBUSY(1);
  if(busylength > 0) {
    usleep(busylength*1000);
    sleepms=sleepms+busylength;
    /*if(((sleepms/1000)%3)==0) {
      printf("Software triggers: %d\n",n);
    };*/
  };
  setBUSY(0);
  if(quit !=0) {quit=0; break;};
};
SLMquit();
printf("Total number of all software triggers: %d\n",n);
}
/* FGROUP CosmicFO
This is scan measurement done on Cosmic fanout LTU (FPGA code: 0xcc) -
looking for right BC_DELAY convenient for both BSY inputs.
fromd: delay from
tod: delay to
secs: seconds between two measurements
*/
void CosmicScan(int fromd, int tod, int secs) {
int ix; // delay;
w32 cnt1,cnt2;
printf("delay triggers   rate\n");
for(ix= fromd; ix<=tod; ix++) {
  float rate;
  vmew32(BC_DELAY_ADD, ix);
  cnt1= getCounter(BUSY_COUNTERrp);
  usleep(secs*1000000);
  cnt2= getCounter(BUSY_COUNTERrp);
  cnt1= dodif32(cnt1, cnt2);
  rate= 1.0*cnt1/secs;
  printf("%d %d %f\n", ix, cnt1, rate); 
};
}

/*--------------------------------------- SLM and ERROR emulation */
/*FGROUP SLM 
filen: see SLMload (e.g. CFG/ltu/SLM/one.seq) */
int SLMcheck(char *filen) {
int rc=0;
int ixx;
w32 slmdata[MAXSLMW];
rc= SLMreadasci(filen, slmdata); if(rc!=0) goto ERRRET;
if(vmer32(EMU_STATUS)&0x1) {
  printf("ERROR: emulation active\n");
  rc=3; goto ERRRET;
};
if((vmer32(STDALONE_MODE)&0x1)==0) {
  printf("ERROR: GLOBAL mode active (check not possible)\n");
  rc=3; goto ERRRET;
};
vmew32(SLM_ADD_CLEAR, DUMMYVAL);   /* clear SLM address counter */
/*dw=vmer32(SLM_ADD_CLEAR); */
usleep(1000);
printf("word file SLM\n");
for(ixx=0; ixx<MAXSLMW; ixx++) {
  w32 dw;
  dw= vmer32(SLM_DATA)&0xffff;
  if(ixx < 10) {
    printf("%3d: %4x %4x\n", ixx, slmdata[ixx], dw);
  }else {
    break;
  };
  
  if(dw != slmdata[ixx]) {
    printf("ERROR in line:%d expected:%x, got:%x\n", ixx,slmdata[ixx],dw);  
    rc=4; goto ERRRET; 
  };
  
};
/*vmew32(SLM_DATA, DUMMYVAL);    force last word to be read ? */
ERRRET:
return(rc);
}

/*FGROUP SLM 
read SLM and write its contents to the file WORK/slmasci
*/
int SLMdump() {
int rc=0;
int ixx,ix; w32 erbits;
FILE *sa;
char l16[17];
if(vmer32(EMU_STATUS)&0x1) {
  printf("ERROR: emulation active, SLM cannot be read.\n");
  rc=3; goto ERRRET;
};
if((vmer32(STDALONE_MODE)&0x1)==0) {
  printf("ERROR: GLOBAL mode active (check not possible)\n");
  rc=3; goto ERRRET;
};
vmew32(SLM_ADD_CLEAR, DUMMYVAL);   /* clear SLM address counter */
/*dw=vmer32(SLM_ADD_CLEAR); */
usleep(1000);
sa= fopen("WORK/slmasci.seq","w");
fprintf(sa,"slmasci.seq\n0\n"); 
erbits=ERgetselector();
strcpy(l16,"0000000");
for(ix=0; ix<7; ix++) {
  if((erbits & (1<<ix)) != 0) l16[ix]='1';
}; fprintf(sa,"%s\n", l16); 
for(ixx=0; ixx<MAXSLMW; ixx++) {
  w32 dw;
  dw= vmer32(SLM_DATA)&0xffff;
  /*if(ixx < 10) {
    printf("%3d: %4x\n", ixx,  dw);
  };*/
  strcpy(l16,"0000000000000000");
  for(ix=0; ix<16; ix++) {
    if((dw & (1<<ix)) != 0) l16[15-ix]='1';
  };
  fprintf(sa,"%s\n", l16); 
};
fclose(sa);
/*vmew32(SLM_DATA, DUMMYVAL);    force last word to be read ? */
ERRRET:
return(rc);
}
/*FGROUP SLM
SLM memory fetch sequence during emulation test: 
  execute following steps in the loop:
- clear SLM_ADD_CLEAR
- SLMstart()
- SLMswstart(1,milsecs);
- SLMwaitemuend();
Don't start it from here, instead use:
1.
ltu/ltu.exe -noboardInit    -> the PID appears
SLMes(0)
2. from independent window, issue 'kill -s USR1 PID' if necessary
*/
void SLMes(int milsecs) {
int rc,i,ntrigs=0;
int nf,trigs,isum;
nf=0; trigs=0; isum=0;
while(1) {
  vmew32(SLM_ADD_CLEAR, DUMMYVAL);   /* clear SLM address counter */
  rc= SLMstart();
  if(rc != 0) {
    nf= nf+1;
    continue;
  } else {
    trigs= trigs+1;
  };
  i=0;
  while(1) {
    i=i+1;
    if(vmer32(EMU_STATUS)&0x1) {
      if(ntrigs!=0) {
        /* compute average */
	isum=isum+i;
      };
      break;
    };
    if(i>1000) {
      printf("em. didnot start after 1000 checks\n");
      break;
    };
  };
  SLMswstart(1,milsecs);
  SLMwaitemuend(0);
  if(quit!=0) {
    break;
  };
};
printf("trigs:%d em. did not finished:%d\n",trigs,nf);
printf("average # of passes: %f\n", 1.*isum/trigs);
}
/*FGROUP SLM
break emulation. RC: EMU_STATUS immediately after break
*/
int SLMbreak() {
w32 st;
vmew32(BREAK_SET, DUMMYVAL);
st= vmer32(EMU_STATUS); return(st);
}
/*FGROUP SLM
set BC scaled down START signal:
0: 25ns
0x3fffff:   ~ 0.1 secs
0x3ffffff:  ~ 1.7 secs
0x3fffffff: ~ 26 secs
*/
void setBCDOWN(w32 bcsd) {
vmew32(COUNT_PERIOD, bcsd);
}
/*FGROUP SLM
Set random rate of automatic START signal generation
*/
void setrate(w32 rndrate) {
vmew32(RANDOM_NUMBER, rndrate);
}
/*----------------------------------------------------ERROR emulation */
/*FGROUP SLM
return: 0 = errors disabled, or 1 ->errors enabled */
int getERenadis() {
return((vmer32(ERROR_STATUS)&0x800)>>11);
}
/*FGROUP SLM
return selector: bits[6:0] */
w32 ERgetselector() {
return(vmer32(ERROR_STATUS)&0x7f);
}
/*FGROUP SLM
demand: bits[2:0] valid values: 1-6
*/
void ERseterrrate(w32 errrate) {
vmew32(ERROR_RATE, errrate);
}
/*--------------------------------------------------- counters */
/* FGROUP SLM
vmew32(COUNTER_MASK, mask); 0xfff -> all counters + timers 
vmew32(COUNTER_CLEAR, DUMMYVAL);

void SLMclearcnts(w32 mask){
vmew32(COUNTER_MASK, mask);
vmew32(COUNTER_CLEAR, DUMMYVAL);
} */

/*FGROUP TOP GUI Counters
Counters monitor */

/*FGROUP TOP GUI Snapshot_memory "Snapshot Memory" */

/*FGROUP TOP GUI ScopeAB "Scope Signals"
Signal selection for front panel 
A,B outputs
*/

void endmain() {
#ifdef SIMVME
#else
//undoreservation();
#endif
}

extern char BoardBaseAddress[];
/*-------------------------------------------------------------- initmain() */
void initmain() {   /* called once, ALWAYS, at the very beginning */
int segid;
w32 shmkey;
initStatic();
shmkey= hex2int(&BoardBaseAddress[2]);
//shmbase= (w32 *)mallocShared(shmkey, 4*LTUNCOUNTERSall, &segid);
ltushm= (Tltushm *)mallocShared(shmkey, sizeof(Tltushm), &segid);
shmcnts= ltushm->ltucnts;
if(ltushm->id==0) {   //just allocated
  ltushm->id=shmkey;
  ltuDefaults(&(ltushm->ltucfg)); ttcDefaults(&(ltushm->ltucfg)); readltuttcdb(&(ltushm->ltucfg));
} else {
  printf("Connected to shared memory %x\n",shmkey);
};
#ifdef SIMVME
  /*printf("initmain: now calling regfuns() (SIMVME mode)...\n"); */
  vmew32(BC_STATUS,0x2);
  setseeds(3,3);
  //regfuns();
#endif
}
/*-------------------------------------------------------------- boardinit() */
void boardInit() {   /* called once, after initmain, if -noboardInit 
			parameter was not given when calling ./ltu/ltu.exe */
#ifdef SIMVME
printf("LTU reservation not made in SIMVME mode\n");
#else
w32 code,rc;
code= 0xff&vmer32(CODE_ADD);
Gltuver= 0xff&vmer32(LTUVERSION_ADD);
if(code==0x56) {
  if(Gltuver==0xff) {
    /* LTU FPGA configuration, if not configured: */
    loadFPGA(0);
    Gltuver= 0xff&vmer32(LTUVERSION_ADD);
  };
} else {
  printf("Incorrect base address or board. Board:0x%x expected:0x56 ver:0x%x\n",    code, Gltuver);
  exit(8);
};
rc= printversion();
if(rc!=0) {
  printf("Exiting...\n"); exit(8);
};
#endif
ltuInit(&ltushm->ltucfg);
}
/*FGROUP Configuration
Initialise LTU board and TTC path (TTCvi and TTCrx).
*/
void LTUinit() {
boardInit();
}
/*ENDOFCF
*/

