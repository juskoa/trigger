#include <stdio.h>
#include <unistd.h>    /* usleep() */
#include "vmewrap.h"
#include "ctp.h"


/*---------------------------------------------------------------- Counters */
w32 buf1[CSTART_SPEC];    // these are not necessary if we do not need accruals
w32 buf2[CSTART_SPEC];
w32 *curprev[2]={buf1,buf2};

/* read counters to memory mem[]. 
NCNTStbr: # of counters to be read
accrual:== 1 -return accruals
        != 1 -return current values
Counters are placed in mem in following order:
L0  160 counters
L1  160 ...
L2  134
FO1  34
...
FO6  34
BUSY 160
INT  19

If board is not present, corresponding word is left unchanged
*/
void readCounters(w32 *mem, int NCNTStbr, int accrual) {
int cix, b123, bb, NCNTS;
w32 copyread, memshift;
w32 *bufp;
int countsread=0;
bufp= curprev[0]; curprev[0]= curprev[1]; curprev[1]= bufp;
for(b123=0; b123<NCTPBOARDS; b123++) {   /* COPY */
  if(notInCrate(b123)) continue;
  bb= BSP*ctpboards[b123].dial;
  vmew32(bb+COPYCOUNT,DUMMYVAL); 
};
usleep(8); // allow 8 micsecs for copying counters to VME accessible memory
for(b123=0; b123<NCTPBOARDS; b123++) {   /* READ */
  if(notInCrate(b123)) continue;
  bb= BSP*ctpboards[b123].dial;
  vmew32(bb+COPYCLEARADD,DUMMYVAL);
  copyread= bb+COPYREAD; 
  NCNTS=ctpboards[b123].numcnts;
  memshift=ctpboards[b123].memshift;
  /*printf("readCounters: %d in, board:%s base:%x shift:%d NCNTS:%d\n", 
    b123, ctpboards[b123].name,bb, memshift, NCNTS); */
  if(accrual==1) {
    for(cix=memshift; cix<NCNTS+memshift; cix++) {
      w32 cur,prev,dif;
      curprev[1][cix]= vmer32(copyread);
      cur= curprev[1][cix]; prev= curprev[0][cix];
      if(cur >= prev) {
        dif= cur-prev;
      } else {
        dif= (0xffffffff - prev) + cur +1;
      };
      mem[cix]= dif;
      countsread++; if(countsread>NCNTStbr) break;
    };
  } else {
    for(cix=memshift; cix<NCNTS+memshift; cix++) {
      mem[cix]= vmer32(copyread);
      countsread++; if(countsread>NCNTStbr) break;
    };
/*    printf("readCounters: %d..%d\n", memshift, NCNTS+memshift-1); */
  };
  if(countsread>NCNTStbr) break;
};
/*printf("cnts 13 165:%d %d\n", mem[13], mem[165]); */
}
/*FGROUP L012
I: board: 0(busy),1(L0),2(L1), 3(L2), 4(FO1),...
   reladr: from 0...
*/
w32 getCounter(int board, int reladr) {
int bb,cix; w32 copyread;
w32 mem[NCOUNTERS_MAX];
bb= BSP*ctpboards[board].dial;
vmew32(bb+COPYCOUNT,DUMMYVAL); 
usleep(8); // allow 8 micsecs for copying counters to VME accessible memory
vmew32(bb+COPYCLEARADD,DUMMYVAL);
copyread= bb+COPYREAD; 
for(cix=0; cix<=reladr; cix++) {
  mem[cix]= vmer32(copyread);
};
return(mem[reladr]);
}
/*FGROUP L012
I: board: 0(busy),1(L0),2(L1), 3(L2), 4(FO1),...
   reladr: Max. rel address to be read. from 0...
   mem: memory[MAXCOUNTERS]
*/
void getCountersBoard(int board, int reladr,w32 *mem) {
int bb,cix; w32 copyread;
bb= BSP*ctpboards[board].dial;
vmew32(bb+COPYCOUNT,DUMMYVAL); 
usleep(8); // allow 8 micsecs for copying counters to VME accessible memory
vmew32(bb+COPYCLEARADD,DUMMYVAL);
copyread= bb+COPYREAD; 
for(cix=0; cix<=reladr; cix++) {
  mem[cix]= vmer32(copyread);
};
}
/*FGROUP L012
Print all counters to stdout (1 per line)
!!! getCounters(), readCounters() params are the same in
ltulib/ltuCounters.c and ctplib/readCounters.c !!!
NCNTS: number of counters to be read + 1 (rel. position of last counter)
if accrual==1, than print accruals
*/
void getCounters(int NCNTS, int accrual) {
int cix;
//w32 buffer[CSTART_SPEC];
w32 buffer[NCNTS+1];
//noreadCounters(curprev[1], accrual);
readCounters(buffer, NCNTS, accrual);
/*if(accrual==1) {
for(cix=0; cix<CSTART_SPEC; cix++) {
  w32 cur,prev,dif;
  cur= curprev[1][cix]; prev= curprev[0][cix];
  if(cur >= prev) {
    dif= cur-prev;
  } else {
    dif= (0xffffffff - prev) + cur +1;
  };
  printf("0x%x\n",dif);
};
}else{*/
for(cix=0; cix<NCNTS; cix++) {
  //printf("0x%x\n",curprev[1][cix]);
  printf("0x%x\n",buffer[cix]);
};
//};
}
/*FGROUP L012
Clear all counters
*/
void clearCounters() {
int bb,b123;
for(b123=0; b123<NCTPBOARDS; b123++) {
  if(notInCrate(b123)) continue;
  bb= BSP*ctpboards[b123].dial;
  vmew32(bb+CLEARCOUNTER,1); 
};
usleep(4);
for(b123=0; b123<NCTPBOARDS; b123++) {
  if(notInCrate(b123)) continue;
  bb= BSP*ctpboards[b123].dial;
  vmew32(bb+CLEARCOUNTER,0);
};
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
};
for(ix=NCTPBOARDS; ix<NCTPBOARDS+24; ix++) {
  mem[ix]= 0;
  if(i2cgetaddr(ix, &chan, &branch)!=0) continue;
  mem[ix]= i2cread(chan, branch); // 4 i2c voltages (in one 32bit word)
  //vme2volt(i2crd);
};
}
