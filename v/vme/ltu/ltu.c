/*BOARD ltu 0x810000 0x800 */
/* BOARD ltu "VXI0::261" 0x800 */
/*
23.05.2008 LTUvi
6.11.2014 scthread added
*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../../vmeb/vmeblib/lexan.h"
#include "vmewrap.h"

#define LTUMAIN
#include "ltu.h"

void *mallocShared(w32, int, int *);
double rnlx();
w32 dodif32(w32 before, w32 now);
void setseeds(long,int);

int  SSMsetom(w32 opmode);
w32 ERgetselector();

extern w32 *smsltu;
extern int quit;
extern char BoardBaseAddress[];
extern w32 SSMSCHEDULER;/* SSM recording planning, 2-After, 3-Before, 0-no plan */
extern int scthread_request;
int rc_scthread=1;     // 0: ok, started
pthread_t sc_thread;   // NULL: not started

/*HIDDEN ADC SLM SSM EmuTests FrontPanel ExpertConf ConfiguratioH SHM Browser*/

/* FGROUP TOP GUI SSMbrowser
Browse LTU snapshot memories 
*/
/*FGROUP Browser
*/
void getsigSSM(int board, int bit, int frombc, int bits);
/*FGROUP Browser
*/
void finddifSSM(int board, int bit, int frombc);
/*------------------------------------------------------- Configuration */
/* FGROUP Configuration GUI Flash_upgrade
*/
/* FGROUP "Configuration" GUI AD_scan
*/
/*FGROUP TOP GUI CTP_Emulator "CTP Emulator"
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
/*FGROUP Configuration GUI CheckRateLimit "Rate limit"
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
b2: 3 ext. orbit, stdalone
    1 int. orbit, stdalone
    0 global mode
*/
void setstdalonemode(w32 b2);

/*FGROUP FrontPanel
*/
void getCounters(int Ncounters, int accrual, int bakery_customer);
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
w32 *shmcnts=NULL;
/*FGROUP SHM
rc: 0: ok
    1: error reading counters
Operation:
action: read all counters + temperature
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
/*FGROUP SimpleTests 
send L1h word ower B channel.
data: 12bits data
words: numbe of words to be send as quickly as possible
MAXW: number of words to be send in one batch (after batch is sent,
      the loop testing TTC_STATUS for empty fifo is started)
      FIFO capacity is 128 words.
*/
void sendB(w32 data, int words, int MAXW) {
//#define MAXW 120
int ix, wordssent=0;
// L1h:1 L1data:2 L2ah:3 L2adata:4 L2r:5 RoIh:6 RoIdata:7 FEEreset:8
w32 Command=1;
w32 fn=0, mics1,mics2,secs1,secs2,udif;   
float rate;
GetMicSec(&secs1, &mics1);
for(ix=0; ix<words; ix++) {
  w32 dawo;
  if((ix%MAXW)==0) {
    int ix2;
    // wait for empty fifo:
    for(ix2=0; ix2<1000; ix2++) {
      dawo= vmer32(TTC_STATUS);
      //if(dawo&2) {   // TTC FIFO full
      if(dawo&1) {   // TTC FIFO empty
        fn++; goto FEMPTY;
      };
    };
    printf("Timeout when waiting for empty fifo\n"); goto STOP;
  };
  FEMPTY:
  dawo= 0x80010000 | data | ((Command&0xf)<<12);
  vmew32(TTC_DATA, dawo); wordssent++;
};
STOP:
GetMicSec(&secs2, &mics2);
udif= DiffSecUsec(secs2,mics2,secs1,mics1);
rate= 1.*wordssent/udif;
//printf("TTCfifo full: %d times\n", fn);
printf("loops when waiting for empty FIFO: %d times\n", fn);
printf("wordssent:%d udif:%d rate[/usec]:%f\n", wordssent, udif, rate);
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
/*FGROUP ExpertConf
micsecs: time interval (in micsecs). During this time, triggers are counted
         and compared with maxtrigs limit. Options:
         999999    -read current micsecs/maxtrigs settings 
         209100/204700 -max. interval 
                   (any higher value results in 209100/204700).
         Interval is set in PERIOD_STEP micsecs slots. Minimal value is 2 slots.
maxtrigs: max. number of triggers allowed in any time interval (set by micsecs)
         Allowed values are: 0..63
enadis:  1: enable, 0: disable 'rate limit' option 
RATE_LIMIT word:
0x80000000 1:enable rate limit
0x0000ff00 period in 0.82ms steps (255 till ver. 0xb4)
0x0007ff00 period in 0.1ms steps (2047 - VALID from 19.7.2010, LTU ver 0xb5)
0x0000003f limit:max number of triggers/period
*/
void RateLimit(int micsecs, int maxtrigs, int enadis) {
//#define PERIOD_MASK 0xff
#define PERIOD_MASK 0x7ff
//#define PERIOD_STEP 820  in micsecs till 19.7.2010
#define PERIOD_STEP 100
w32 ltuv, rl;
ltuv= vmer32(LTUVERSION_ADD);
if( ltuv < 0xb3) {
  printf("LTU FPGA version:%x. Upgrade to 0xb3 or higher\n", ltuv);
  return;
};
rl= vmer32(RATE_LIMIT);
if( micsecs == 999999) {             /* retrieve current settings */
  micsecs= (((rl >> 8) & PERIOD_MASK)+1) * PERIOD_STEP;
  if(micsecs==PERIOD_STEP) micsecs=0;
  maxtrigs= rl & 0x3f;
  if( rl & 0x80000000) {
    enadis= 1;
    printf("Current RATE_LIMIT: 0x%x: micsecs:%d maxtrigs:%d\n", 
      rl, micsecs, maxtrigs);
  } else {
    enadis= 0;
    printf("RATE_LIMIT: 0x%x -rate limit is disabled.\n", rl);
  };
} else {                       /* set new rate limit */
  if( micsecs > (PERIOD_MASK*PERIOD_STEP) ) { micsecs=PERIOD_MASK*PERIOD_STEP ;
    printf("micsecs corrected to %d\n",PERIOD_MASK*PERIOD_STEP);
  };
  if( maxtrigs > 63) { maxtrigs= 63;
    printf("maxtrigs corrected to 63\n");
  };
  if( enadis == 0) {
    vmew32(RATE_LIMIT,rl&0x7fffffff);
    printf("Rate limit is disabled now.\n");
  } else {
    enadis=1;   /* changed 8..2009:
    micsecs= micsecs/PERIOD_STEP; rl= ((micsecs) << 8) | maxtrigs | 0x80000000;
    vmew32(RATE_LIMIT, rl);
    micsecs= ((rl >> 8) & PERIOD_MASK) * PERIOD_STEP;
    maxtrigs= rl & 0x3f;
    printf("oldway: RATE_LIMIT: 0x%x =  max %d triggers in %d microseconds\n", 
      rl, maxtrigs, micsecs); */
    micsecs= micsecs/PERIOD_STEP; 
    if(micsecs<=1) {micsecs=0; }
    else {micsecs=micsecs-1; };
    rl= ((micsecs) << 8) | maxtrigs | 0x80000000;
    vmew32(RATE_LIMIT, rl);
    micsecs= (((rl >> 8) & PERIOD_MASK)+1) * PERIOD_STEP;
    maxtrigs= rl & 0x3f;
    printf("RATE_LIMIT: 0x%x =  max %d triggers in %d microseconds\n", 
      rl, maxtrigs, micsecs);
  };
};
printf("%d %d %d\n", micsecs, maxtrigs, enadis);
}

/*----------------------------------------------------------- SimpleTests */

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
action: Flash memory -> FPGA
Note: never tested, probably uncomplete ...
*/
int loadFPGA() {
w32 status; int attempts=0;
//resetFM:
vmew32(FlashAccessNoIncr,0xF0);
while(1) {
  status= vmer32(FlashStatus); attempts++;
  if(status&0x80) break;
  if(attempts>=4000) break;
};
if(attempts>=4000) return(1);  // cannot resetFM
printf("resetFM ok after %d attempts\n", attempts);
status=vmer32(ConfigStatus);
vmew32(ConfigStart,0x0);
usleep(300000);   // usually takes 264milsecs
return(0);
}

/*FGROUP SimpleTests
print: 
CODE_ADD       always 0x56 for LTU
SERIAL_NUMBER  of the LTU board
VERSION_ADD    VME FPGA version firmware
LTUVERSION_ADD LTU FPGA version firmware
BC_STATUS      BC ststus (should be 0x2 == PLL locked, BC ok)
 */
void printversion() {
w32 serial, code,vmever,ltuver,pll;
code= 0xff&vmer32(CODE_ADD);
vmever= 0xff&vmer32(VERSION_ADD);
serial= 0x7f&vmer32(SERIAL_NUMBER);  // 7 bits for ltu2 (6 for ltu1)
ltuver= Gltuver;
printf("LTUcode:0x%x serial#:0x%x VME ver:0x%x LTUfpga:0x%x\n",
  code, serial, vmever, ltuver);
printf("LTUsw version:%s\n", LTU_SW_VER);
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
pll= checkSEU();
return;
}
/*FGROUP SimpleTests
*/ 
void printBusyFraction() {
printf("busy fraction: %6.4f\n", ltushm->busyfraction);
};
/* FGROUP SimpleTests
Generate trigs triggers. Each Trigger is followed by the busy signal wide
'busylength' miliseconds. Operation:
-disable both BUSY inputs
-load L2a sequence into emulator memory
-start emulation
-generate required number of triggers (Software triggers)

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
    //if(((sleepms/1000)%3)==0) {
    //  printf("Software triggers: %d\n",n);
    //};
  };
  setBUSY(0);
  if(quit !=0) {quit=0; break;};
};
SLMquit();
printf("Total number of all software triggers: %d\n",n);
*/

/*FGROUP SimpleTests
busys      -number of busy pulses to be generated
busylength -length of the pulses (us)
period     -time interval between pulses (us)
*/
void GenTrigBusy(int busys, int busylength, int period) {
int n;
for(n=0; n<busys; n++) {
  vmew32(SW_BUSY,1);
  micwait(busylength);
  vmew32(SW_BUSY,0);
  //usleep(busylength);
  usleep(period-busylength);
  if(quit !=0) {quit=0; break;};
};
}
/* FGROUP SimpleTests
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

#define SLM_BITS 0xffffffff
#define SLM_BITSN 32
/*FGROUP SLM 
filen: see SLMload (e.g. CFG/ltu/SLM/one.seq) */
int SLMcheck(char *filen) {
int rc=0;
int ixx; //int slmbitsn; 
w32 slmbits;
w32 slmdata[MAXSLMW];
if(lturun2) {
  //slmbitsn= SLM_BITSN;
  slmbits= SLM_BITS;
} else {
  //slmbitsn= 16;
  slmbits= 0xffff;
};
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
  dw= vmer32(SLM_DATA)&slmbits;
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
int rc=0; int slmbitsn;
int ixx,ix; w32 erbits, slmbits;
FILE *sa;
char l16[SLM_BITSN+1];   // 17
if(lturun2) {
  //printf("run2... Gltuver:%x\n", Gltuver);
  slmbitsn= SLM_BITSN;
  slmbits= SLM_BITS;
} else {
  //printf("run1... Gltuver:%x\n", Gltuver);
  slmbitsn= 16;
  slmbits= 0xffff;
};
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
if(sa==NULL) {
  printf("ERROR: WORK/slmasci.seq annot be opened for write\n");
  rc=4; goto ERRRET;
};
fprintf(sa,"slmasci.seq\n0\n"); 
erbits=ERgetselector();
strcpy(l16,"0000000");
for(ix=0; ix<7; ix++) {
  if((erbits & (1<<ix)) != 0) l16[ix]='1';
}; fprintf(sa,"%s\n", l16); 
for(ixx=0; ixx<MAXSLMW; ixx++) {
  w32 dw;
  dw= vmer32(SLM_DATA)&slmbits;
  /*if(ixx < 10) {
    printf("%3d: %4x\n", ixx,  dw);
  };*/
  if(lturun2) {
    strcpy(l16,"00000000000000000000000000000000");
  } else {
    strcpy(l16,"0000000000000000");
  };
  for(ix=0; ix<slmbitsn; ix++) {
    if((dw & (1<<ix)) != 0) l16[slmbitsn-1-ix]='1';
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
if(rc_scthread==0) {
  printf("Joining scthread...\n");
  if(pthread_join(sc_thread, NULL)) {
    printf("Error joining scthread\n");
  };
} else {
  printf("scthread was not started, joining skipped\n");
};
}
void *scthread(void *dummy) {
w32 ptime, pbusy,ntime=0,nbusy=0,nloop=0;
ltushm->ltucfg.flags= ltushm->ltucfg.flags | FLGscthread;
while(1) {
  float deltatime,deltasbusy;
  if(quit==888) break;
  //printf("scthread: readCNTS2SHM...\n");
  ptime= ntime; pbusy= nbusy;
  readCNTS2SHM();
  ntime= ltushm->ltucnts[LTU_TIMErp];
  nbusy= ltushm->ltucnts[SUBBUSY_TIMERrp];
  if(nloop>0) {
    deltatime= dodif32(ptime, ntime);
    deltasbusy= dodif32(pbusy, nbusy);
    ltushm->busyfraction= (1.0*deltasbusy)/deltatime;
    if( (ltushm->busyfraction<0.0) || ( ltushm->busyfraction > 1.0)) {
      printf("ERROR in scthread: %d 0x%x 0x%x 0x%x 0x%x\n",
        nloop, ptime, ntime, pbusy, nbusy);
      ltushm->busyfraction= 1.0;
    };
  };
  usleep(1000000); nloop++;
  if(nloop>0x7ffffffe) nloop=1;
};
ltushm->ltucfg.flags= ltushm->ltucfg.flags & (~FLGscthread);
return(NULL);
}
extern char BoardBaseAddress[];
/*-------------------------------------------------------------- initmain() */
/* called once, ALWAYS, at the very beginning.
*/
void initmain() {
int segid;
w32 shmkey;
Gltuver= fpgainit();
if(Gltuver==0) { return; };
smsltu=NULL;
initStatic();
shmkey= hex2int(&BoardBaseAddress[2]);
//shmbase= (w32 *)mallocShared(shmkey, 4*LTUNCOUNTERSall, &segid);
ltushm= (Tltushm *)mallocShared(shmkey, sizeof(Tltushm), &segid);
shmcnts= ltushm->ltucnts;
if(ltushm->id==0) {   //just allocated
  ltushm->id=shmkey;
  ltuDefaults(&(ltushm->ltucfg)); ttcDefaults(&(ltushm->ltucfg)); readltuttcdb(&(ltushm->ltucfg));
  printf("Shared memory allocated and initialised %x\n",shmkey);
} else {
  printf("Connected to shared memory %x\n",shmkey);
};
// ltushm->ltucfg.plist[IXltuver]= Gltuver;  can be in shm? (shm
// can be shared by tasks controlling the same LTU), so it can
//if( (ltushm->ltucfg.plist[IXltuver]&0xf0) != 0xb0 ) {
if( ((Gltuver&0xf0) != 0xb0 ) && (Gltuver!=0xf3) ) {
  printf("\n  LTU FPGA code:0x%x. 0xbX expected (LTUvi) or 0xf3\n\n", 
    Gltuver);
};
#ifdef SIMVME
  /*printf("initmain: now calling regfuns() (SIMVME mode)...\n"); */
  vmew32(BC_STATUS,0x2);
  vmew32(LTUVERSION_ADD,0xb7);
  setseeds(3,3);
  //regfuns();
#endif
if((scthread_request==1) && ((ltushm->ltucfg.flags & FLGscthread)==0)) {
  printf("Starting scthread...\n");
  //scthread_start();
  if((rc_scthread=pthread_create(&sc_thread, NULL, scthread, ltushm))) {
    printf("Error creating scthread\n");
  } else {
    printf("scthread started.\n");
  };
} else {
  printf("Counters reading thread not started (already active or not requested. req:%d).\n",
    scthread_request);
};
}
/*-------------------------------------------------------------- boardinit() */
void boardInit() {   /* called once, after initmain, if -noboardInit 
                        parameter was not given when calling ./ltu/ltu.exe */
#ifdef SIMVME
printf("LTU in SIMVME mode\n");
printversion();
#else
printversion();
#endif
ltuInit(&ltushm->ltucfg, 1, 0);
setAB(1,23); printf("Scope A: Orbit\n");
}
/*FGROUP Configuration
Initialise LTU board and TTC path (TTCvi and TTCrx).
*/
void LTUinit() {
boardInit();
}
/* This is from inputs.c, should go to lib ?
Translates Signature number to LVDST SEQ_DATA word, i.e the least
2 significant bits are zero.
lvdst2 is hexa signature
*/
int SigNum2LVDSTNum(int SigNum){
 int lvdst,lvdst2;
 if(SigNum > 0x7f){
  printf("Signature > 0x7f \n");
  return 0;
 }
 lvdst=((~SigNum)& 0x7f)<<2;
 lvdst=lvdst+(SigNum<<9);
 lvdst=lvdst+(0xb1 << 16);  // HEADER
 lvdst2=((~SigNum)& 0x7f)+(SigNum<<7)+(0xb1<<14);
 printf("SEQ_DATA=0x%x 0x%x 0x%x\n",lvdst,lvdst2,lvdst2<<2);
 return (lvdst);
}
//--------------------------------------------------------------------
void CheckSignature(int signature,w32 *sm){
 int channels[32];
 int i,offset;
 for(i=0;i<32;i++)channels[i]=0;
 offset=1;
 signature= SigNum2LVDSTNum(signature);
 for(i=0;i<6;i++){
   channels[i+offset]=signature; 
 }
 checkSignature(sm,channels,offset);
}
/* FGROUP ConfiguratioH
Testing connection between CTP FO and LTU.
Procedure:
1.) Start generation of the patterm on FO board by running the macro
    'foNboard()' from the ctpt window.
2.) Number of 1 in SSM should be equal to number of 1 from pattern
Comment:
Full pattern recognition to be implemented.
It hangs if pattern is not sent from FO!
*/
void TestConnection(){
 w32 *sm;
 w32 mode,w;
 int i,j;
 float f;
 const char *pole[]={"ORBIT","PP","L0","L1","L1data","L2s","L2data"};
 int bit[7]; 
 // 0=orbit,1=pp,2=l0,3=l1,4=l1data,5=l2s,6=l2data
 for(j=0;j<7;j++)bit[j]=0;
 mode = vmer32(STDALONE_MODE);
 if(mode != 0){
  printf("TestConnection warning: ltu in standalone mode !\n");
  printf("Changing to global.\n");
  setglobalmode();
 }
 SSMstartrec(0x12);
 usleep(100000);
 sm=smsltu;
 readSSM(sm);
 for (i=0;i<Mega;i++){
  w=sm[i];
  for(j=0;j<7;j++){
   if(w&(1<<j))bit[j]++;
  }
 }
 for(j=0;j<7;j++){
  f= bit[j]*1./(Mega);
  printf("Bit %i: %s %i %f \n",j,pole[j],bit[j],f);
 }
 {
 int signature=1;
 CheckSignature(signature,sm);
 }
 printf("DONT FORGET TO START FO generation ! \n");
}
/*ENDOFCF
*/

