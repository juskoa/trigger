/* Leslie Lamport's bakery locking algorithm 
If used in more processes, bakery object should point to the memory 
shared by them.
Testing: uncomment dic.hxx include and main() at the end.
g++ -I/opt/dim/dim -lpthread -L/opt/dim/linux -ldim bakery.c linux_c/timeroutines.o -o /tmp/bakerytest.exe 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/dim/linux

30.11.2015 usleep(bakery->lockus) added in lockBakery()
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>   // usleep
#include "vmeblib.h"
#include "bakery.h"

//#include <dic.hxx>

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
bakery->lockus= 0;
if(strcmp(name,"ccread")==0) bakery->lockus= 500;
if(strcmp(name,"ssmcr")==0) bakery->lockus= 500000;
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
    if(bakery->Number[ix] < bakery->Number[icu]) {
      // if(bakery->lockus > 0) { usleep(bakery->lockus); }; 
      usleep(bakery->lockus); // better, (not 100%cpu for lockus==0)
      continue;
    };
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
printf("Bakery %s:%d:%dus locks:%d unlocks:%d Min:%dus\n", 
  bp->name, bp->Maxn, bp->lockus, bp->Nlocks, bp->Nunlocks, bp->minUsecs);
for(i=0;i< bp->Maxn;i++){ 
  printf("customer%d: %d %d\n", i, bp->Entering[i], bp->Number[i]);
};
}
/* rc: 0: not locked, 1: ok resource locked */
int lockBakeryTimeout(Tbakery *bakery, int icu, int timeout_secs) {
int ix,mx=0;
w32 secs,usecs, timeout_usecs;
if(icu>=bakery->Maxn) {
  printf("Error: lockBakery:%s: %d above max. customer #:%d \n",
    bakery->name, icu, bakery->Maxn-1);
  return(0);
}; timeout_usecs= timeout_secs*1000000;
GetMicSec(&secs, &usecs); 
if(bakery->lastsecs!=0) {
  w32 diffu;
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
    if(bakery->Number[ix] < bakery->Number[icu]) {
      w32 secs2,usecs2, diffu;
      // if(bakery->lockus > 0) { usleep(bakery->lockus); }; 
      usleep(bakery->lockus); // better, (not 100%cpu for lockus==0)
      GetMicSec(&secs2, &usecs2); 
      diffu= DiffSecUsec(secs2, usecs2, secs, usecs);
      if(diffu>= timeout_usecs) { // timeout
        bakery->Number[icu]= 0;
        return(0);
      };
      continue;
    };
    break;
  }
};
return(1);
}
/*
#define Ncustomers 5
Tbakery ccread[Ncustomers];

void printText(char *text) {
int it=0;
while(text[it]!='\0') {
  printf("%c", text[it]); usleep(100);
  it++;
}; //printf("\n");
};
void Thread(void *tag) {
int ii,cycles=0;
ii= *(int *)tag;
while (true) {
  char message[100];
  //sprintf(message,"h%de%dr%de thread%d",cycles,cycles,ii,ii);
  sprintf(message,"-%d:%d-", ii, cycles);
  lockBakery(ccread,ii); // The critical section goes here...
  printf("L%d",ii);
  printText(message); usleep(1000000);
  printf("U%d\n",ii);
  unlockBakery(ccread, ii); // non-critical section...
  //usleep(100);
  cycles++; if(cycles>=20) break;
};
}

int main() {
int Thread1=0;
int Thread2=1;

// see ctp_proxy/ctp_main.c -for all bakeries allocation
initBakery(ccread, "counters",Ncustomers);
dim_start_thread(Thread, (void *)&Thread1);
dim_start_thread(Thread, (void *)&Thread2);
printf("stopping...\n");
sleep(15);
}
*/
