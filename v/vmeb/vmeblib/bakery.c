/* Leslie Lamport's bakery locking algorithm 
if used in more processes, bakery should point to the memory 
shared by them */
#include <stdio.h>
#include <string.h>
//#include <unistd.h>
#include "vmeblib.h"
#include "bakery.h"

#define true 1    
#define false 0

void initBakery(Tbakery *bakery, char *name, int maxn) {
int ix, maxx;
maxx= maxn;
if(maxn>MAXNUM_THREADS) {
  maxx= MAXNUM_THREADS;
  printf("Error: initBakery: only max. %d customers allowed\n", maxx);
};
bakery-> Maxn= maxx;
for(ix=0; ix<maxx; ix++) {
  bakery->Entering[ix]= false;
  bakery->Number[ix]= 0;
};
bakery->minUsecs= 1000000000;
bakery->lastsecs= 0; bakery->lastusecs= 0;
bakery->Nlocks= 0; bakery->Nunlocks= 0;
strcpy(bakery->name, name);
}
void lockBakery(Tbakery *bakery, int icu) {
int ix,mx=0;
w32 secs,usecs, diffu;
if(icu>=bakery->Maxn) {
  printf("Error: lockBakery:%s: %d above max. customer #:%d \n",
    bakery->name, icu, bakery->Maxn-1);
  return;
};
GetMicSec(&secs, &usecs); 
if(bakery->lastsecs!=0) {
  diffu= DiffSecUsec(secs, usecs, bakery->lastsecs, bakery->lastusecs);
  if(diffu<(bakery->minUsecs)) {
    bakery->minUsecs= diffu;
  };
};
bakery->lastsecs= secs; bakery->lastusecs= usecs;
bakery->Nlocks= bakery->Nlocks+1;
//cannot be here (ctp.exe count. reading) 
//printf("lockBakery %s:%d:%ds %dus\n", bakery->name, icu, secs, usecs);
bakery->Entering[icu]= true;
//Number[icu]= 1 + max(Number[1], ..., Number[NUM_THREADS]);
for(ix=0; ix<bakery->Maxn; ix++) {
  if(bakery->Number[ix]>mx) mx=bakery->Number[ix];
}; bakery->Number[icu]= 1 + mx;
bakery->Entering[icu]= false;
for(ix=0; ix <bakery->Maxn; ix++) {
  // Wait until thread ix receives its number:
  while (bakery->Entering[ix]) { ; };
  // Wait until all threads with smaller numbers or with the same
  // number, but with higher priority, finish their work:
  //while ((Number[ix] != 0) && ((Number[ix], ix) < (Number[icu], icu))) {
  while (bakery->Number[ix] != 0) {
    //((Number[ix], ix) < (Number[icu], icu))
    if(bakery->Number[ix] < bakery->Number[icu]) continue;
    break;
    ;     /* nothing */
  }
};
}
void unlockBakery(Tbakery *bakery, int icu) {
if(icu>=bakery->Maxn) {
  printf("Error: unlockBakery: %d above max. customer #:%d \n", icu, bakery->Maxn-1);
  return;
};
bakery->Number[icu]= 0;
bakery->Nunlocks= bakery->Nunlocks+1;
}
void printBakery(Tbakery *bp) {
int i;
printf("Bakery %s:%d customers locks:%d unlocks:%d Min:%dus\n", 
  bp->name, bp->Maxn, bp->Nlocks, bp->Nunlocks, bp->minUsecs);
for(i=0;i< bp->Maxn;i++){ 
  printf("customer%d: %d %d\n", i, bp->Entering[i], bp->Number[i]);
};
}

