/* swtrigger.c
General aproach when programming sw trigger:
1. setswtrig()
2. startswtrig()  - as many times as necessary
or
GenSwtrg(N,...)   - set and generate N triggers

DAQbusy doesn't have to be applied (i.e. setswtrig(), startswtrig
can be invoked togethether with physics triggers being active)
*/
#include <unistd.h>
#include <stdio.h>
#include "vmewrap.h"
#include "ctp.h"
#include "Tpartition.h"
#define TIMEOUT 100      /* <10 with mysleep(10) */
/*---------------------------------------------------------------CountTime()
  Count time from the last function call.
*/
w32 CountTime(){
 static w32 seconds=0;
 static w32 micsec=0;
 w32 sec,mic,time;
 GetMicSec(&sec,&mic);
 time=(sec-seconds)*1000000+(mic-micsec);
 seconds=sec;
 micsec=mic;
 return time;
}
/*FGROUP DebCon
mysleep: delta is in micsecs
*/
void mysleep(w32 delta){
 w32 seconds,micsec,sec,mic,time;
 GetMicSec(&seconds,&micsec);
 time=0;
 while(time<delta){
   GetMicSec(&sec,&mic);
   time=(sec-seconds)*1000000+(mic-micsec);
 }
}

/*FGROUP DebCon---------------------------------------------clearflags()
 * Clear flags on all levels
*/
void clearflags(){
 vmew32(L0_TCCLEAR,0x0);
 vmew32(L1_TCCLEAR,0x0);
 vmew32(L2_TCCLEAR,0x0);
}
char TRIGTYPE;
/*FGROUP DebCon---------------------------------------------------setswtrig
 This routine should be called to set trigger 
 before the repetitive call of startswtrig().
 (or there is GenSwtrg() combining both)
 TODO:
 - BCmask is not used. Should be given as symbolic name
   (similarly pf protection) in one if input parameters
 - p/f is disabled. If set, we should wait at least 'L0 Protection interval'
   before generating sw trigger (see ctp_pf.doc)
Inputs:
-------
 trigtype: 'a' - asynchronous
           's' - synchronous, noncalibration
           'c' - callibration
 roc: -> INT,FO* boards
 BC : bunch crossing for the case of 'c' and 's'
 ctprodets: 24 bits representing CTP readout (L2 board). I.e.
            bits 23..0 corresponds to detectors 24..1 with
            DAQ numbering (i.e. daqdet# in VALID.LTUS file)
            It is used for programming:
            - L2 board
            - BUSY board 
            - FO boards
Operation:
----------
Set:
 L0: bc, bcmask is OFF now, flags: syn/asyn  cal, PF:off 
 L1: PF:off
 L2: PF:off, list of detectors- ctprodets
BUSY: BUSY_CLUSTER_TEST word
FOs: FO_TEST_CLUSTER words:
INT: INTtcset (roc + CIT flag in case of Cal. trigger)

RC: 0: if successfully set
*/
int setswtrig(char trigtype, int roc, w32 BC, w32 ctprodets){
w32 word, INTtcset, busyclusterT, overlap, bsysc[NCLUST+1];
int i, idet, ifo, iconnector;
w32 testclust[NFO],rocs[NFO];

if(BC>3563){
  printf("setswtrig: BC>3563 %i \n",BC);
  return 1;
};
INTtcset= roc<<1;   // INT board
// L0 board   p/f, masks off
//     P/F      BCM4   BCM3    BCM2    BCM1
word=(1<<18)+(1<<17)+(1<<16)+(1<<15)+(1<<14);
switch(trigtype){
  case 'a':
       word=word+0;
       if(BC !=0) {   // mask (4bits, 1:use mask 0: do not use mask)
         word= word & (~(BC&0xf));
       };
       //printf("setswtrig: asynchr trigger 0x%x \n",word);
       break;
  case 's':
       word=word+(1<<12)+BC;
       //printf("setswtrig: synchr trigger 0x%x \n",word);
       break;
  case 'c':
       word=word+(1<<12)+(1<<13)+BC;
       INTtcset= INTtcset | 1;
       //printf("setswtrig: calib trigger 0x%x \n",word);
       break;
  default:
       printf("setswtrig: unknown type of trigger %c \n",trigtype);
       return 1;
}; TRIGTYPE= trigtype;
vmew32(L0_TCSET,word);        // L0 board -p/f prot. off
word=(1<<18);
vmew32(L1_TCSET,word);        // L1 board p/f prot. off
word=(1<<24)+ctprodets;
vmew32(L2_TCSET,word);        // L2 board p/f off
vmew32(INT_TCSET,INTtcset);   // INT board
/* set BUSY_CLUSTER word (bits 23..0 for detectors 24..1 connected to BUSY)
*/
busyclusterT= findBUSYinputs(ctprodets);
vmew32(BUSY_CLUSTER, busyclusterT);
/* we should update BUSY_OVERLAP word (at least bits corresponding
to combinations 1T 2T 3T 4T 5T 6T should be updated: */
bsysc[0]= busyclusterT;
for(i=1;i<NCLUST+1;i++){
  bsysc[i]=vmer32(BUSY_CLUSTER+i*4);
};
overlap= calcOverlap(bsysc); vmew32(BUSY_OVERLAP, overlap);
if(DBGswtrg) {
  printf("setswtrig: BUSY/SET_CLUSTER: 0x%x BUSY_OVERLAP:0x%x ctprodets:0x%x\n", 
    busyclusterT, overlap, ctprodets);
};
/* set corresponding FO boards: */
for(i=0;i<NFO;i++){testclust[i]=0;rocs[i]=0;}
for(idet=0;idet<NDETEC;idet++){
  if((ctprodets & (1<<idet))!=0) {
    if(Detector2Connector(idet,&ifo,&iconnector)) continue;  //not connected
    testclust[ifo]=testclust[ifo] +(1<<(16+iconnector));  //TestCluster
    if(trigtype=='c') {    // cal. trigger, set CALFLAG
      testclust[ifo]=testclust[ifo] | 0x100000 ;
    };
    rocs[ifo]=rocs[ifo]+(roc<<(4*iconnector));  
    if(DBGswtrg) {printf(
      "setswtrig ifo=%i icon=%i testcl=0x%x roc=0x%x dets:0x%x\n",
        ifo,iconnector,testclust[ifo],rocs[ifo], ctprodets);
    };
  };
};
for(ifo=0;ifo<NFO;ifo++){   // set all FOs always
  if((notInCrate(ifo+FO1BOARD)==0)) {
    w32 vmeaddr;
    vmeaddr= FO_TESTCLUSTER+BSP*(ifo+1);
    if(DBGswtrg) printf("setswtrig FO:%d Waddr: 0x%x data: 0x%x\n",
      ifo, vmeaddr, rocs[ifo] | testclust[ifo]);
    vmew32(vmeaddr, rocs[ifo] | testclust[ifo]);
  }
};
return 0;
}
/*---------------------------------------------------------getlxackn
*/
w32 getl0ackn(){
 return (vmer32(L0_TCSTATUS)&0x8)/0x8;
}
w32 getl1ackn(){
 return vmer32(L1_TCSTATUS)/0x8;
}
w32 getl2ackn(){
 return vmer32(L2_TCSTATUS);
}
w32 getPPrqst(){
 return (vmer32(L0_TCSTATUS)&0x2)/0x2;
}
w32 getL0rqst(){
 return (vmer32(L0_TCSTATUS)&0x4)/0x4;
}
/*---------------------------------------------------------startswtrig
Operation:
 * - start sw trig. Should be called as many times as necessary after
     setswtrig
 * - follows acknowledgement on L0,L1,L2 levels
 * - clear flags (in any case (error or success))
 * return 0 = fail
          1 = killed at l0
          2 = killed at l1
          3 = l2r
          4 = l2a OK, generated
          5 = PP timeout
          6 = L0 timeout
          7 = L2a/r timeout (>120micsec)
          8 = bad request (neither of: c s a)
*/
char reason[8][40]={"Fail", "killed at L0", "killed at L1", "L2r", "OK",
  "PP timeout", "L0 timeout", "L2a/r timeout"};
int startswtrig(){
 w32 flag;
 int ret,i;
//w32 itime=0,time[20]; 
//time[itime++]=CountTime();
vmew32(L0_TCSTART,0);
if(TRIGTYPE == 'c'){ i=0;
  while(getPPrqst() && i<TIMEOUT)i++;
  if(i>=TIMEOUT){
    ret=5; goto RETERR;
  };
};
if(TRIGTYPE == 's' || TRIGTYPE == 'c'){
  usleep(180);   // for SYNC we should wait for sure 2 orbits
  i=0;
  while(getL0rqst() && i<TIMEOUT)i++;
  if(i>=TIMEOUT){
    ret=6; goto RETERR;
  } 
} else if( TRIGTYPE != 'a'){
  ret=8; goto RETERR;
};
ret=3;
 //time[itime++]=CountTime();
 //printf("l0ackn: %i \n",getl0ackn());
i=0; while(!getl0ackn() && (i<TIMEOUT))i++;
 //time[itime++]=CountTime();
if(i>=TIMEOUT){
  ret = 1; goto RETERR;
};
 //mysleep(10); // wait for L1 trigger  
 //printf("l1ackn: %i \n",getl1ackn());
i=0; while(!getl1ackn() && i<TIMEOUT)i++;
 //time[itime++]=CountTime();
if(i>=TIMEOUT){
  ret=2; goto RETERR;
};
 //usleep(120);
flag=getl2ackn();
i=0;
 //while((flag != 8) && (flag != 4) && i<TIMEOUT){
while(((flag&0xc) == 0) && i<TIMEOUT) {   //wait L2a/L2r ACK
  flag=getl2ackn();
  mysleep(10);
  i++;
};
//if(DBGswtrg) printf("  l2ackn:0x%x loops(10us sleep) %i \n",flag, i);
if(i >= TIMEOUT){
  printf("startswtrig: Timeout at l2ackn. \n");
  ret = 7; goto RET;
};
if(flag == 4){         // L2r ack
  ret= 3; goto RETERR;
}else if(flag == 8){   // L2a ack
  ret= 4; goto RET;
}else{
  printf("startswtrig: FAIL, flag=%i %i I should never be here.\n",flag,i);
  return 0; // there should be l2a or l2r 
}
 // Time
 /* time[itime++]=CountTime();
 printf("startswtrig: TIMES ");
 for(i=0;i<itime;i++)printf(" %i ",time[i]);
 printf("ret= %i \n",ret);
 */ 
RETERR:
printf("startswtrig: %d %s\n", ret, reason[ret] );
RET:  clearflags(); 
return(ret);
}

/*FGROUP DebCon   --------------------------------------------- GenSwtrg
Generate n software trigger sequences
Operation:
 -setswtrig()
 -while(n) startswtrig()

Parameters: see setswtrig()
RC: number of L2a successfully generated
*/
int  GenSwtrg(int n,char trigtype, int roc, w32 BC,w32 detectors){
 int flag,itr=0;
 int l0=0,l1=0,l2a=0,l2r=0;
 w32 status;
 status=vmer32(L0_TCSTATUS);
 if((status&0x10)==0x10){
  printf(" GenSwtrg: TC busy. L0 TC_STATUS:0x%x\n", status);
  return 0;
 }
 setswtrig(trigtype,roc,BC,detectors);
 while(((itr<n) && ((flag=startswtrig(trigtype))))){
      if(flag == 1)l0++;
      else if(flag == 2)l1++;
      else if(flag == 3)l2r++;
      else if(flag == 4)l2a++;
      else {
        printf(" GenSwtrg: unexpected flag %i \n",flag);
        return 0;
      }
      itr++;
      //usleep(60000000);
 };
 if(DBGswtrg) {
   printf(" GenSwtrg: %i %c-triggers generated for detectors:0x%x.\n",
     itr,trigtype, detectors);
   printf("l0,l1,l2r,l2a: %i %i %i %i \n",l0,l1,l2r,l2a);
 };
 TRIGTYPE='.';
 // return i;
 return l2a;
}
