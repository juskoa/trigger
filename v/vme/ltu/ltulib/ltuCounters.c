#include <stdio.h>
#include <unistd.h>    /* usleep() */
#include <string.h>
#include "vmewrap.h"
#include "vmeblib.h"
#include "ltu.h"

/*---------------------------------------------------------------- Counters */
w32 bufc1[LTUNCOUNTERS];    // these are not necessary if we do not need accruals
w32 bufc2[LTUNCOUNTERS];
w32 *curprev[2]={bufc1,bufc2};

/* read counters to memory mem[]. 
Input:    
mem        pointer to w32 array
NCNTS      # of counters to be read (LVDST or for LTU:LTUNCOUNTERS)
accrual:== 1 -return accruals (i.e. difference between this and last read)
           ! ONLY 1 process can use 'accrual' feature at 1 time !
        != 1 -return current values
NCNTS counters are placed in mem
*/
void readCounters(w32 *mem, int NCNTS, int accrual) {
int cix;
int memshift;
//printf("readCounters: %d %d\n", NCNTS, accrual);
vmew32(COPYCOUNT,DUMMYVAL); 
usleep(8); // allow 8 micsecs for copying counters to VME accessible memory
vmew32(COPYCLEARADD,DUMMYVAL);
memshift=0;
if(accrual==1) {
  w32 *bufp;
  bufp= curprev[0]; curprev[0]= curprev[1]; curprev[1]= bufp;
  for(cix=memshift; cix<NCNTS+memshift; cix++) {
    w32 cur,prev,dif;
    curprev[1][cix]= vmer32(COPYREAD);
    cur= curprev[1][cix]; prev= curprev[0][cix];
    if(cur >= prev) {
      dif= cur-prev;
    } else {
      dif= (0xffffffff - prev) + cur +1;
    };
    mem[cix]= dif;
  };
} else {
  for(cix=memshift; cix<NCNTS+memshift; cix++) {
    w32 cv;
    cv= vmer32(COPYREAD); // cv= cix+1;
    //mem[cix]=curprev[1][cix]=cv;
    mem[cix]= cv;
    //printf("readCounters: %d:0x%x\n", cix, cv);
  };
/*    printf("readCounters: %d..%d\n", memshift, NCNTS+memshift-1); */
};
/*printf("cnts 13 165:%d %d\n", mem[13], mem[165]); */
}
/* reladr: from 0...
*/
w32 getCounter(int reladr) {
int cix;
w32 mem[LTUNCOUNTERS];   // the must: LTUNCOUNTERS >= LVDST_NCOUNTERS
vmew32(COPYCOUNT,DUMMYVAL); 
usleep(8); // allow 8 micsecs for copying counters to VME accessible memory
vmew32(COPYCLEARADD,DUMMYVAL);
for(cix=0; cix<=reladr; cix++) {
  mem[cix]= vmer32(COPYREAD);
};
return(mem[reladr]);
}

/* Print all counters to stdout (1 per line)
NCNTS: number of counters to be read + 1 (rel. position of last counter)
if accrual==1, than print accruals
bakery_customer: has to be 2 when reading from GUI (2
is shared with ctp-counters vmeb/counters.py).
BUT NOT USED YET for ltu, here it is just for compatibility
with vmeb/counters.py call: getCounters(...
24.9.2014:
- use shm for reading if available (in the same time ltuproxy's cthred
  arranged to read counters every second, i.e. getCounters cannot
  return fresh value within shiorter interval)
- accrual is not valid any more, always abs. values returned
*/
void getCounters(int NCNTS, int accrual, int bakery_customer) {
int cix; // Tltushm *ltushm;  is in ltu.h (EXTERN)
w32 buffer[NCNTS];
accrual=0;   // return ALWAYS ABSOLUTE values
if(ltushm != NULL) {
  if(ltushm->ltucnts[LTU_TIMErp] != 0) { 
    w32 *ltucs;
    ltucs= ltushm->ltucnts;
    for(cix=0; cix<NCNTS; cix++) {
      buffer[cix]= ltucs[cix]; // if(cix==1) buffer[cix]= (w32)ltucs;
    };
  } else {
    // shm found, but seems counters not being read 1/sec by ltuproxy
    readCounters(buffer, NCNTS, accrual);
  };
} else {
  // shm not available (should never happen!)
  printf("ERROR: getCounters(): shm not available for ltu, reading ltu\n");
  readCounters(buffer, NCNTS, accrual);
};
for(cix=0; cix<NCNTS; cix++) {
  //printf("0x%x\n",curprev[1][cix]);
  printf("0x%x\n",buffer[cix]);
}; //printf("\n");
}
/*--------------------------------------------------- Clear all counters
*/
void clearCounters() {
vmew32(CLEARCOUNTER,1); 
usleep(4);
vmew32(CLEARCOUNTER,0);
printf("LTU counters cleared.\n");
}
/*--------------------------------------------- measureBusy100ms()
*/
int measureBusy100ms() {
w32 mem1[LTUNCOUNTERS];   // place for 24 LTU counters
w32 mem2[LTUNCOUNTERS];
w32 busys, busys1, transitions;
int averageInt;
float ms100, dead, average;
readCounters(mem1, LTUNCOUNTERS, 0); usleep(100000);
readCounters(mem2, LTUNCOUNTERS, 0);
dead= dodif32(mem1[SUBBUSY_TIMERrp], mem2[SUBBUSY_TIMERrp])*0.4;  //micsecs
ms100= dodif32(mem1[LTU_TIMErp], mem2[LTU_TIMErp])*0.0004;  //milisecs
busys= dodif32(mem1[L0_COUNTERrp], mem2[L0_COUNTERrp]);
transitions= dodif32(mem1[BUSY_COUNTERrp], mem2[BUSY_COUNTERrp]);
busys1=busys;
if(busys==0) busys1=1;
average= dead/busys1; averageInt=rounddown(average);
/*printf(
"Interval: %5.2f milsec L0s:%d Busy trans:%d Aver. BusyTime:%6.2f micsec\n",
  ms100, busys, transitions, average); */
return(averageInt);
}

int mem1ok=0;
w32 mem1[LTUNCOUNTERS];   // place for 24 LTU counters
w32 mem2[LTUNCOUNTERS];
/*------------------------------------------------------------------------------measureBusy()
Modified from cosmicfo.c 
*/
int measureBusy() {
w32 subbusy,transitions,timedif,l2, busy;
float subbusyf,timeint,frac,dead,rate,l2f,ratets,deadts,transf, busyf, dead_busy, frac_busy;
int ic;
char secs[10];
if(mem1ok==0) {
  readCounters(mem1, LTUNCOUNTERS, 0); sleep(1); mem1ok=1;
};
readCounters(mem2, LTUNCOUNTERS, 0);
timedif= dodif32(mem1[LTU_TIMErp], mem2[LTU_TIMErp]);
timeint= timedif;  // in 0.4 usecs
subbusy= dodif32(mem1[SUBBUSY_TIMERrp], mem2[SUBBUSY_TIMERrp]); 
busy= dodif32(mem1[SUBBUSY_TIMERrp], mem2[BUSY_TIMERrp]); 
transitions= dodif32(mem1[BUSY_COUNTERrp], mem2[BUSY_COUNTERrp]); 
l2 = dodif32(mem1[L2a_COUNTERrp], mem2[L2a_COUNTERrp]); 
//printf("inputRates: %d %d %d\n", timedif, busy1, busy2);
subbusyf=subbusy; busyf= busy;
l2f=l2;
transf=transitions;
if(subbusy == timedif){
 printf("Detector DEAD \n");
 return 0;
}
if(transitions == 0){
 printf("WARNING: # transitions =0, ts deadtime has no sense.\n");
 deadts=0.;
}else deadts = subbusyf*0.4/transf; 
if(l2 == 0){
 printf("WARNING: # L2a =0, l2 deadtime has no sense.\n");
 dead=0.; dead_busy=0.;
}else {dead = subbusyf*0.4/l2f; dead_busy = busyf*0.4/l2f; };
; 
frac= subbusyf/timeint;   frac_busy= busyf/timeint;   
rate= l2f/timeint/0.4*1000.;
ratets= transf/timeint/0.4*1000.;


if(transitions != l2){
 printf("WARNING: # transitions %d != # of L2a %d\n",transitions,l2);
 printf("         deadtime calculated with transitions.\n");
}

timeint=timeint*0.4;
if(timeint>1000000.) {
  timeint=timeint/1000000.; strcpy(secs,"secs    ");
}else if(timeint > 1000.){ timeint=timeint/1000.;strcpy(secs,"milisecs");}
else{strcpy(secs,"usecs");}
printf(
"Interval: %5.2f %s DeadTime Transitions: %f usecs\n\
                         Rate Transitions: %f [kHz]\n\
                         Rate L2a        : %f [kHz]\n\
                  sbusy         busy\n\
    DeadTime L2a: %10.2f      %10.2f     usecs\n\
    Fraction:     %10.2f      %10.2f     \n",
  timeint, secs, deadts, ratets, rate,
  dead, dead_busy, frac, frac_busy);
for(ic=0; ic<LTUNCOUNTERS; ic++) mem1[ic]= mem2[ic];
return 0;
}

