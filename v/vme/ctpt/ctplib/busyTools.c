#include <stdio.h>
#include <unistd.h>    /* usleep() */
#include "vmewrap.h"
#include "ctp.h"
#include "Tpartition.h"
#define NDET 24
/*---------------------------------------------------------- getBUSYtimerscounters()
Read 80 BUSY counters.
rc: !=0: OK
    ==0: error
*/
#define BYTIMERSCOUNTN 110 
int getBUSYtimerscounters(w32 *mem) {
int board=0;   //BUSY board
int bb,cix, rc=1;
w32 copyread;
bb= BSP*ctpboards[board].dial;   //this is 0x8000 for BUSY board
vmew32(bb+COPYCOUNT,DUMMYVAL); 
usleep(8); // allow 8 micsecs for copying counters to VME accessible memory
vmew32(bb+COPYCLEARADD,DUMMYVAL);
copyread= bb+COPYREAD; 
for(cix=0; cix<BYTIMERSCOUNTN; cix++) {   // 24 byin* timers, busy_time
  mem[cix]= vmer32(copyread);
};
return(rc);
}
/*FGROUP busy
Operation:
- read busy timers
- sleep 100ms
- read busy timers 
- calculate difference between 2 measurements and compare 
  with busy_timer
rc: busy pattern: [0..23] bits set to 1 correspond to Dead busy inputs
*/
w32 findDeadBusys() {
int cix;
w32 mem1[BYTIMERSCOUNTN];   // place for 24 byin* timers[0..23] +"busy_time counter"[39]
w32 mem2[BYTIMERSCOUNTN];
char ltus[200];
int rc;
w32 ms100, busys=0;
rc= getBUSYtimerscounters(mem1); usleep(100000);
rc= getBUSYtimerscounters(mem2);
ms100= dodif32(mem1[39], mem2[39]);     // busy_timer
//printf("findDeadBusys: ms100:%d\n", ms100);
for(cix=0; cix<24; cix++) {   // 24 byin* timers, busy_time
  w32 bt;
  bt= dodif32(mem1[cix], mem2[cix]);
  //printf("findDeadBusys: bt%d:%d\n", cix, bt);
  if( bt<(ms100-10) ) continue; // not DEAD if at least 4 micsecs (10*0.4)in READY
  busys= busys | (1<<cix);        
};
findLTUNAMESby(busys, ltus);
printf("busy dets:%s\n", ltus);
return(busys);
}
/*FGROUP busy
Operation:
- read busy configuraion
- read busy timers
- sleep 100ms
- read busy timers 
- calculate difference between 2 measurements and compare 
  with busy_timer
rc: busy pattern: [0..23] bits set to 1 correspond to Dead busy inputs
*/
w32 findDeadBusysRuns(int time){
int cix;
w32 mem1[BYTIMERSCOUNTN];   // place for 24 byin* timers[0..23] +"busy_time counter"[39]
w32 mem2[BYTIMERSCOUNTN];
int cls[7];
float dtfrac[NDET],dtperev[NDET],rate[NDET];
w32 clsbs;
char ltus[200];
int rc,i,j;
w32 ms100, busys=0;
for(i=0;i<NDET;i++){
  dtfrac[i]=0.;dtperev[i]=0.;
}
time=time*1000;
rc= getBUSYtimerscounters(mem1); usleep(time);
rc= getBUSYtimerscounters(mem2);
ms100= dodif32(mem1[39], mem2[39]);     // busy_timer
// input dead time per event and deat time
for(cix=0; cix<NDET; cix++) {   // 24 byin* timers, busy_time
  w32 bt,btr;
  bt= dodif32(mem1[cix], mem2[cix]);
  btr= dodif32(mem1[55+cix], mem2[55+cix]);
  dtfrac[cix]=bt*1./ms100;
  rate[cix]=btr/0.4/ms100*1.e6;
  if(btr) dtperev[cix]=bt*0.4/btr;      // in usecs
  //printf("findDeadBusys: bt%d:%d\n", cix, bt);
  if( bt<(ms100-10) ) continue; // not DEAD if at least 4 micsecs (10*0.4)in READY
  busys= busys | (1<<cix);        
};
// busy fractions
for(i=1; i<7; i++){
  cls[i]= vmer32(BUSY_CLUSTER+4*i);
  if(cls[i]){
    printf("CLUSTER %d:  measured time: %f milisecs\n",i,ms100*0.4/1000.);
    printf("Fraction DeadTime[us] Rate[Hz]  Det\n");
    for(j=0;j<NDET;j++){
      if((1<<j) & cls[i]){
        findLTUNAMESby(1<<j, ltus);
        printf(" %2.3f    %7.1f      %4.1f     %s\n",dtfrac[j],dtperev[j],rate[j],ltus);
      }
    }
   }
 }
printf("\n");
// Plot dead busys in run
for(i=1; i<7; i++){
  cls[i]= vmer32(BUSY_CLUSTER+4*i);
  clsbs=cls[i]&busys; 
  if(clsbs){
    findLTUNAMESby(clsbs, ltus);
    printf("CLUSTER %d busy dets:%s\n", i,ltus);
  }
}
return(0);
}
/*------------------------------------------------------------------------------------------*/
#define LASTBUSY1 79
int memlok=0;
w32 mem1[BYTIMERSCOUNTN];   
w32 mem2[BYTIMERSCOUNTN];
/*FGROUP busy
 Print last busy for detectors in clusters.
 I get always zero - bug pedja or me.
*/
void printLastDetectors(w32 cluster){
 w32 i,j;
 w32 cls,mask,timedif;
 float timeint;
 char dets[256];
 w32 lastbusy[NDET+1];
 for(i=0;i<NDET+1;i++)lastbusy[i]=0;
 vmew32(BUSYLAST_SELECT,cluster);
 if(memlok==0){
  getBUSYtimerscounters(mem1);sleep(1);memlok=1;
 }
 getBUSYtimerscounters(mem2);
 timedif=dodif32(mem1[39],mem2[39]);
 timeint=timedif*0.4/1000.;  // in milisecs 
  mask=0;
  cls= vmer32(BUSY_CLUSTER+4*cluster);
  for(j=0;j<NDET;j++){
   if((1<<j) & cls){
     lastbusy[j]=dodif32(mem1[LASTBUSY1+j],mem2[LASTBUSY1+j]);
     mask=mask | (1<<j);
   }
  }
 printf("mask=%i\n",mask);
 if(mask){
   printf("Number of LAST BUSYS per detector:\n");
   printf("Time interval: %f milisec\n",timeint);
   for(i=0;i<NDET;i++)if(mask & (1<<i)){
     findLTUNAMESby(1<<i,dets);
     printf("%20d %s\n",lastbusy[i],dets);
   }
   printf("\n");
 }else{
  printf("No loaded detectors in cluster %d \n",cluster);
 }
 for(i=0;i<NDET+1;i++)mem1[i]=mem2[i];
}
/*FGROUP busy
0- CTP BUSY
1-24 : detectors
25-30 : clusters
31 test cluster
*/
void busyprobe(char *det){
 int detector;
 float min,max;
 detector=findLTUdetnum(det);
 if(detector == -1){
  printf("Cannot find detector %s \n",det);
  return;
 }
 vmew32(MINIMAX_SELECT,detector);
 //vmew32(MINIMAX_CLEAR,0xff);
 sleep(1);
 max=0.4*vmer32(BUSYMAX_DATA);
 min=0.4*vmer32(BUSYMINI_DATA);
 printf("%i %s MIN:%f MAX:%f \n",detector,det,min,max);
}

