#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    /* usleep() */
#include "vmewrap.h"
#include "ctp.h"
#include "ctplib.h"
#include "Tpartition.h"

/*---------------------------------------------------------------- Counters */
static w32 buf1[NCOUNTERS];
static w32 buf2[NCOUNTERS];
w32 *curprev[2]={buf1,buf2};

/* read counters to memory mem[]. 
mem: has to be mem[NCOUNTERS] -i.e. 1760 after 7.4.2015
     because L0 is read in 2 chunks (200 words at the end of big array)
NCNTStbr: # of counters to be read

accrual:== 1 -return accruals
        != 1 -return current values
customer: 
0: ctpproxy
1: ctpdims
2: ctp.exe + busyTools +ctpt
3: smaq
4: inputs

Counters are placed in mem in following order:
L0 300                 (check in ctp/ctpcounters.h)
L1  
L2  
FO1 
...
FO6
BUSY
INT
SPEC 49 words
L0 200 lmclassB1..100 lmclassA1..100

If board is not present, corresponding word is left unchanged
*/
void readCounters(w32 *mem, int NCNTStbr, int accrual, int customer) {
int cix, memshift, b123, bb, NCNTS;
w32 copyread;
w32 *bufp;
int countsread=0;
/*if(NCOUNTERS>NCNTStbr) {
  printf("ERROR: readCounters a buffer at %d words, only %d available.\n", 
    NCOUNTERS,NCNTStbr); return; 
}; */
//printBakery(&ctpshmbase->ccread);
lockBakery(&ctpshmbase->ccread, customer);
//printBakery(&ctpshmbase->ccread);
bufp= curprev[0]; curprev[0]= curprev[1]; curprev[1]= bufp;
for(b123=0; b123<NCTPBOARDS; b123++) {   /* COPY */
  if(notInCrate(b123)) continue;
  bb= BSP*ctpboards[b123].dial;
  vmew32(bb+COPYCOUNT,DUMMYVAL); 
};
usleep(30); // allow 12 (8 in run1) micsecs for copying counters to VME accessible memory
for(b123=0; b123<NCTPBOARDS; b123++) {   /* READ */
  if(notInCrate(b123)) continue;    //comment for dbg mode
  bb= BSP*ctpboards[b123].dial;
  vmew32(bb+COPYCLEARADD,DUMMYVAL);
  copyread= bb+COPYREAD; 
  NCNTS=ctpboards[b123].numcnts;
  memshift=ctpboards[b123].memshift;
  /*printf("readCounters: %d in, board:%s base:%x shift:%d NCNTS:%d\n", 
    b123, ctpboards[b123].name,bb, memshift, NCNTS);*/
  if(accrual==1) {
    for(cix=memshift; cix<NCNTS+memshift; cix++) {
      w32 cur,prev,dif; int cixc;
      if((b123==1) && ( cix>=memshift+NCOUNTERS_L0)) {
        // L0 in 2 chunks (300 and 200 words)
        cixc= cix + (CSTART_L0200 - CSTART_L0 - NCOUNTERS_L0);
      } else {
        cixc= cix;
      };
      curprev[1][cixc]= vmer32(copyread);
      cur= curprev[1][cixc]; prev= curprev[0][cixc];
      if(cur >= prev) {
        dif= cur-prev;
      } else {
        dif= (0xffffffff - prev) + cur +1;
      };
      if(cixc < NCNTStbr) {
        mem[cixc]= dif;
      };
    };
  } else {
    //char* environ;
    //environ=getenv("VMEWORKDIR");
    //char file[1024];
    //strcpy(file,environ);
    //strcat(file,"/WORK/counters.log");
    //FILE *ff = fopen(file,"a"); 
    for(cix=memshift; cix<NCNTS+memshift; cix++) {
      int cixc;
      if((b123==1) && ( cix>=memshift+NCOUNTERS_L0)) {
        cixc= cix + (CSTART_L0200 - CSTART_L0 - NCOUNTERS_L0);
      } else {
        cixc= cix;
      };
      if(cixc < NCNTStbr) {
        mem[cixc]= vmer32(copyread);
        //mem[cixc]= cixc;   //dbg
        //fprintf(ff,"cust:%i %i %ui \n",customer,cixc,mem[cixc]);
      } else {
        vmer32(copyread);   // all counters MUST be read
        //printf("readCounters: attempt to write too far (%d)...\n", cixc);
      };
      //countsread++; if(countsread>NCNTStbr) break;
    };
    //fclose(ff);
/*    printf("readCounters: %d..%d\n", memshift, NCNTS+memshift-1); */
  };
  countsread= countsread+ NCNTS;
  if(countsread>NCNTStbr) break;
};
unlockBakery(&ctpshmbase->ccread, customer);
/*printf("cnts 13 165:%d %d\n", mem[13], mem[165]); */
}
/* in .h FGROUP L012
I: board: 0(busy),1(L0),2(L1), 3(L2), 4(FO1),...,10(INT)
   reladr: from 0...reladr  (i.e. 3 means reading first 4 counters)
   customer: 2 (for ctp exp. sw)
*/
w32 getCounter(int board, int reladr, int customer) {
int bb,cix,nbc; w32 copyread;
w32 mem[NCOUNTERS_MAX];
lockBakery(&ctpshmbase->ccread, customer);
bb= BSP*ctpboards[board].dial;
nbc= ctpboards[board].numcnts;
vmew32(bb+COPYCOUNT,DUMMYVAL); 
usleep(30); // allow 8 micsecs for copying counters to VME accessible memory
vmew32(bb+COPYCLEARADD,DUMMYVAL);
copyread= bb+COPYREAD; 
//for(cix=0; cix<=reladr; cix++) {   // seems not working (cannot catch it) 
for(cix=0; cix<nbc; cix++) {
  mem[cix]= vmer32(copyread);
};
unlockBakery(&ctpshmbase->ccread, customer);
return(mem[reladr]);
}
/* FGROUP L012
I: board: 0(busy),1(L0),2(L1), 3(L2), 4(INT), 5(FO1), 6(FO2),...
   reladr: Max. rel address to be read. from 0...
   mem: memory[MAXCOUNTERS]
*/
void getCountersBoard(int board, int reladr,w32 *mem, int customer) {
int bb,cix; w32 copyread; //w32 ignore;
lockBakery(&ctpshmbase->ccread, customer);
bb= BSP*ctpboards[board].dial;
vmew32(bb+COPYCOUNT,DUMMYVAL); 
usleep(30); // allow 8 micsecs for copying counters to VME accessible memory
vmew32(bb+COPYCLEARADD,DUMMYVAL);
copyread= bb+COPYREAD; 
/* 25.3.2014: ALL counters have to be readout! */
for(cix=0; cix<ctpboards[board].numcnts; cix++) {
  if(cix<=reladr) {
    mem[cix]= vmer32(copyread);
  } else {
    /*ignore=*/ vmer32(copyread);
  };
};
unlockBakery(&ctpshmbase->ccread, customer);
}
/*FGROUP L012
Print NCNTS counters to stdout (1 per line)
!!! getCounters(), readCounters() params are the same in
ltulib/ltuCounters.c and ctplib/readCounters.c !!!
NCNTS: number of counters to be printed
if accrual==1, than print accruals
*/
void getCounters(int NCNTS, int accrual, int customer) {
int cix;
//w32 buffer[CSTART_SPEC];
w32 buffer[NCOUNTERS];
//noreadCounters(curprev[1], accrual);
readCounters(buffer, NCOUNTERS, accrual, customer);
for(cix=0; cix<NCNTS; cix++) {
  //printf("0x%x\n",curprev[1][cix]);
  printf("0x%x\n",buffer[cix]);
};
//};
}
/*FGROUP L012
Clear all counters
*/
void clearCounters(int customer) {
int bb,b123;
lockBakery(&ctpshmbase->ccread, customer);
for(b123=0; b123<NCTPBOARDS; b123++) {
  if(notInCrate(b123)) continue;
  bb= BSP*ctpboards[b123].dial;
  if((b123==1) && (l0C0())) {
    vmew32(CLEARCOUNTER_lm0,1); 
  } else {
    vmew32(bb+CLEARCOUNTER,1); 
  };
};
usleep(30);
for(b123=0; b123<NCTPBOARDS; b123++) {
  if(notInCrate(b123)) continue;
  bb= BSP*ctpboards[b123].dial;
  if((b123==1) && (l0C0())) {
    vmew32(CLEARCOUNTER_lm0,0); 
  } else {
    vmew32(bb+CLEARCOUNTER,0);
  };
};
unlockBakery(&ctpshmbase->ccread, customer);
printf("Counters cleared.\n");
}

/* return 46 words. 
1st part (22 words):
-------------------
2 words: (temperature,voltages) or 0,0 if missing board.
Temperature: in centigrades
Voltages:
for each of the following boards: busy l0 l1 l2 int fo1-6 
2nd part (24 words):
-------------------
Voltages for ltu1-24
*/
void readTVCounters(w32 *mem) {
int ix, chan,branch;
// w32 i2crd;
for(ix=0; ix<NCTPBOARDS; ix++) {
  if( (i2cgetaddr(ix, &chan, &branch)!=0) ||
      notInCrate(ix) ) {
    mem[2*ix]= 0; mem[2*ix+1]= 0;  continue;
  };
  mem[2*ix]= ReadTemp(ix);  // read temperature
  mem[2*ix+1]= i2cread(chan, branch); // 4 i2c voltages (in one 32bit word)
  //dbg mem[2*ix]= 0xaaaa;
  //dbg mem[2*ix+1]= 0xbbbb;// 4 i2c voltages (in one 32bit word)
};
for(ix=NCTPBOARDS; ix<NCTPBOARDS+24; ix++) {  //bug fixed 15.12.2010
  mem[ix+(2-1)*NCTPBOARDS]= 0;
  if(i2cgetaddr(ix, &chan, &branch)!=0) continue;
  mem[ix+(2-1)*NCTPBOARDS]= i2cread(chan, branch); // 4 i2c voltages (in one 32bit word)
  //dbg mem[ix+(2-1)*NCTPBOARDS]= ix+(2-1)*NCTPBOARDS;
  //vme2volt(i2crd);
};
}
