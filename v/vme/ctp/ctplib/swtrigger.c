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
#include <stdlib.h>   // getenv
#include <stdio.h>
#include <string.h>
#include "vmewrap.h"
#include "vmeblib.h"
#include "lexan.h"
#include "ctp.h"
#include "Tpartition.h"
#define TIMEOUT 100      /* <10 with mysleep(10) */
#define DBGswtrg2 1
#define DBGswtrg3 1
#define DBGswtrg4 0

int l0C0();

w32 ifoglob[NFO];
/* read CALIBRATION_BC from ltuproxy. Return -1 if more detectors
involved or unknow detector
*/
int getCALIBBC2(w32 ctprodets) {
char detname[20]; int rc;
bit2name(ctprodets, detname);
// ask ltuproxy for CALIBRATION_BC:
if(detname[0]!='\0') {
  rc= getCALIBBC(detname);
} else {
  rc=-1;
};
return(rc);
}
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
int LMSTART=0;   // 0: L0 start   1:LM start
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
 BC : BCmask for trigtype 'a'
      bunch crossing for the trigtype 'c' or 's'
         0xfff: retrieve CALIBRATION_BC from ltuproxy
 ctprodets: 24 bits representing CTP readout (L2 board). I.e.
            bits 23..0 corresponds to detectors 23..0 with
            DAQ numbering (i.e. daqdet# in VALID.LTUS file)
            It is used for programming:
            - L2 board
            - BUSY board 
            - FO boards
            - for decision if trigger starts from LM level (i.e. TRD in)
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
    1: bad parameter(s): BC>3563 or bad trigtype
*/
int setswtrig(char trigtype, int roc, w32 BC, w32 ctprodets){
w32 daqonoff, word, INTtcset, busyclusterT, overlap, bsysc[NCLUST+1];
int rc=0, i, idet, ifo, iconnector;
w32 testclust[NFO],rocs[NFO];
#define TRDECSM 0x10
if( (((BC>3563) && (BC!=0xfff)) && (trigtype !='a')) 
  ){
  printf("Error: setswtrig: BC>3563 %i \n",BC);
  rc=1; goto RET;
};
if(ctprodets & TRDECSM) { LMSTART=1; } else { LMSTART=0;};
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
       {
       //word=word+(1<<12)+BC;
       int mybc=BC-vmer32(L0_BCOFFSETr2);
       if(mybc<0) BC=mybc+3564; else BC=mybc; 
       word=word+(1<<12)+BC;  // from 12.11.2015 (c707)
       //printf("setswtrig: synchr trigger 0x%x \n",word);
       break;
       }
  case 'c':
    if(BC==0xfff) { 
      BC= getCALIBBC2(ctprodets);
    };
    {
    //BC= BC - 97;   // CALIB_BC: 3011 ORBIT_BC: 91
    //BC= BC - 2;   // CALIB_BC: 3011 ORBIT_BC: 3560
    int mybc = BC - 2 - vmer32(L0_BCOFFSETr2); // from 12.11.2015 (c707) 
    /* BC-2: 
     software trigger in 3011 gives BCID: 3008
     calib.   trigger in 3011 gives BCID: 3006
     We do not know why we program BC-2 for cal. trigger (but it was programmed
     this way before c707)
     Before c707: SOD (sync trigger), generated in BC 1750 was seen in DAQ in BC
     1762 -this is very likely for TRD-run, when BCID is higher by 15, i.e.
     1750 + 15 -3 = 1762).
    */
    if(mybc<0) BC=mybc+3564; else BC=mybc;
    BC=BC%3564; 
    word=word+(1<<12)+(1<<13)+BC;
    INTtcset= INTtcset | 1;
    //printf("setswtrig: calib trigger 0x%x \n",word);
    }
    break;
  default:
    printf("Error: setswtrig: unknown type of trigger %c \n",trigtype);
    rc=1; goto RET;
}; TRIGTYPE= trigtype;
// L0 board -p/f prot. off
if(l0C0()) { 
  if(LMSTART != 0) word= word | 0x80000;
  vmew32(L0_TCSETr2,word); 
} else { 
  vmew32(L0_TCSET,word); 
};
word=(1<<18);
vmew32(L1_TCSET,word);        // L1 board p/f prot. off
word=(1<<24)+ctprodets;
  daqonoff= vmer32(INT_DDL_EMU) &0xf;
  if((daqonoff==0) || (daqonoff==0x3b)) { 
    // ctp readout active or emulated, set TRIGGER bit 
    //the bit has to be set for CALIBRATION events too!
    // if not set, EVB complains (run 67492)
    word= word  | (1<<CTPLTUECSN);   // from Monday 23rd bit17 back
    //word=word ;        // temporary suppress bit17 till Monday 23rd
    //printf("setswtrig: bit17 not set\n");
    //};
  };
vmew32(L2_TCSET,word);        // L2 board p/f off
if(DBGswtrg4) printf("setswtrig:L2_TCSET set to:%x\n", word);
vmew32(INT_TCSET,INTtcset);   // INT board
/* set BUSY_CLUSTER word (bits 23..0 for detectors 24..1 connected to BUSY)
*/
busyclusterT= findBUSYinputs(ctprodets);   //no bit17!
vmew32(BUSY_CLUSTER, busyclusterT);
/* we should update BUSY_OVERLAP word (at least bits corresponding
to combinations 1T 2T 3T 4T 5T 6T should be updated: */
bsysc[0]= busyclusterT;
for(i=1;i<NCLUST+1;i++){
  bsysc[i]=vmer32(BUSY_CLUSTER+i*4);
};
overlap= calcOverlap(bsysc); vmew32(BUSY_OVERLAP, overlap);
if(DBGswtrg4) {
  printf("setswtrig: BUSY/SET_CLUSTER: 0x%x BUSY_OVERLAP:0x%x ctprodets:0x%x\n", 
    busyclusterT, overlap, ctprodets);
};
/* set corresponding FO boards: */
for(i=0;i<NFO;i++){testclust[i]=0;rocs[i]=0;}
for(idet=0;idet<NDETEC;idet++){
  if((ctprodets & (1<<idet))!=0) {   // no bit17!
    if(Detector2Connector(idet,&ifo,&iconnector)) continue;  //not connected
    testclust[ifo]=testclust[ifo] +(1<<(16+iconnector));  //TestCluster
    if(trigtype=='c') {    // cal. trigger, set CALFLAG
      testclust[ifo]=testclust[ifo] | 0x100000 ;
    };
    rocs[ifo]=rocs[ifo]+(roc<<(4*iconnector));  
    if(DBGswtrg4) {printf(
      "setswtrig ifo=%i icon=%i testcl=0x%x roc=0x%x BC:%d dets:0x%x\n",
        ifo,iconnector,testclust[ifo],rocs[ifo], BC, ctprodets);
    };
  };
};
for(ifo=0;ifo<NFO;ifo++){   // set all FOs always
  //printf("FO:%d\n",ifo);
  if((notInCrate(ifo+FO1BOARD)==0)) {
    w32 vmeaddr;
    vmeaddr= FO_TESTCLUSTER+BSP*(ifo+1);
    ifoglob[ifo]= testclust[ifo] | rocs[ifo];
    if((DBGswtrg4==1)&&(testclust[ifo]!=0)) printf("setswtrig FO:%d Waddr: 0x%x data: 0x%x\n",
      ifo, vmeaddr, rocs[ifo] | testclust[ifo]);
    vmew32(vmeaddr, rocs[ifo] | testclust[ifo]);
  }
};
RET:
return(rc);
}
void vmew32f(w32 adr, w32 data) {
printf("vmew32f:%8x = %x\n", adr, data);
}

/*---------------------------------------------------------getlxackn
*/
w32 getlMackn(){
return (vmer32(L0_TCSTATUS)&0x80)/0x80;
}
w32 getl0ackn(){
return (vmer32(L0_TCSTATUS)&0x8)/0x8;
/*w32 rc;
if(l0C0()) {
 rc= vmer32(L0_TCSTATUSr2)&0x8)/0x8;
} else {
 rc= vmer32(L0_TCSTATUS)&0x8)/0x8;
}; return(rc); */
}
w32 getl1ackn(){
 return (vmer32(L1_TCSTATUS)&0x8)/0x8;
}
w32 getl2ackn(){
 return vmer32(L2_TCSTATUS);
}

w32 getPPrqst(){
return (vmer32(L0_TCSTATUS)&0x2)/0x2;
/*w32 rc;
if(l0C0()) {
 rc= vmer32(L0_TCSTATUSr2)&0x2)/0x2;
} else {
 rc= vmer32(L0_TCSTATUS)&0x2)/0x2;
}; return(rc); */
}
w32 getLMrqst(){
return (vmer32(L0_TCSTATUS)&0x40)/0x40;
}
w32 getL0rqst(){
return (vmer32(L0_TCSTATUS)&0x4)/0x4;
/* w32 rc;
if(l0C0()) {
 rc= vmer32(L0_TCSTATUSr2)&0x4)/0x4;
} else {
 rc= vmer32(L0_TCSTATUS)&0x4)/0x4;
}; return(rc); */
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
          9 = LM request timeout
         10 = killed at LM
*/
char reason[9][40]={"Fail", "killed at L0", "killed at L1", "L2r", "OK",
  "PP timeout", "L0 timeout", "L2a/r timeout","bad TRIGTYPE (!= c s a)"};
int startswtrig(w32 *orbitn){
 w32 flag;
 int ret,i;
//w32 itime=0,time[20]; 
//time[itime++]=CountTime();
*orbitn= vmer32(L2_ORBIT_READ); vmew32(L0_TCSTART,0); 
if(TRIGTYPE == 'c'){ i=0;
  while(getPPrqst() && i<TIMEOUT)i++;
  if(i>=TIMEOUT){
    ret=5; goto RETERR;
  };
};
if(TRIGTYPE == 's' || TRIGTYPE == 'c'){
  usleep(180);   // for SYNC we should wait for sure 2 orbits
  //usleep(380); printf("startswtrig:380us\n");
} else {
  if( TRIGTYPE != 'a'){ ret=8; goto RETERR; };
};
if(LMSTART!=0) {
  i=0; while(getLMrqst() && i<TIMEOUT)i++;
  if(i>=TIMEOUT){ ret=9; goto RETERR; };
  i=0; while(!getlMackn() && (i<TIMEOUT))i++;
  if(i>=TIMEOUT){ 
    /*w32 l0status;
    l0status=vmer32(L0_TCSTATUS);*/
    ret=10; goto RETERR;
  };
};
i=0; while(getL0rqst() && i<TIMEOUT)i++;
if(i>=TIMEOUT){ ret=6; goto RETERR; };
 //time[itime++]=CountTime();
 //printf("l0ackn: %i \n",getl0ackn());
i=0; while(!getl0ackn() && (i<TIMEOUT))i++;
 //time[itime++]=CountTime();
if(i>=TIMEOUT){
  /* w32 l0status;
  l0status=vmer32(L0_TCSTATUS);
  printf("startswtrig: L0 TC_STATUS:0x%x\n", l0status);*/
  ret= 1; goto RETERR;
};
 //mysleep(10); // wait for L1 trigger  
 //printf("l1ackn: %i \n",getl1ackn());
i=0; while(!getl1ackn() && i<TIMEOUT)i++;
 //time[itime++]=CountTime();
if(i>=TIMEOUT){
  ret= 2; goto RETERR;
};
 //usleep(120);
flag=getl2ackn(); i=0;
 //while((flag != 8) && (flag != 4) && i<TIMEOUT){
while(((flag&0xc) == 0) && i<TIMEOUT) {   //wait L2a/L2r ACK
  flag=getl2ackn();
  mysleep(10);
  i++;
};
//if(DBGswtrg4) printf("  l2ackn:0x%x loops(10us sleep) %i \n",flag, i);
if(i >= TIMEOUT){
  printf("startswtrig: Timeout at l2ackn. \n");
  ret= 7; goto RET;
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
if(DBGswtrg4) printf("startswtrig: %d %s\n", ret, reason[ret] );
RET:  clearflags(); 
return(ret);
}
int msgpres=0;
/* ------------------------------------------------------------ GenSwtrg 
orbitn: orbit number read just before the first trigger generation, i.e.
        it is always <= of ORBITID of the sw. trigger event
RC: number of L2a successfully generated, or
    12345678: cal. triggers stopped becasue det. is not in global run
*/
int  GenSwtrg(int ntriggers,char trigtype, int roc, w32 BC,w32 detectors, 
              int customer, w32 *orbitn ){
int flag,itr=0;
int l0=0,l1=0,l2a=0,l2r=0, lm=0;
w32 status, orbitnloc;
if(l0C0()) {
  status=vmer32(L0_TCSTATUS);
} else {
  status=vmer32(L0_TCSTATUS);
};
if((status&0x10)==0x10){
  printf(" GenSwtrg: TC busy. L0 TC_STATUS:0x%x\n", status);
  return 0;
}
if(detectors & 0x20000) {
  printf("GenSwtrg: CTP (ECS number 17) cannot be sw triggered, bit17 removed\n");
  detectors= detectors & (~0x20000);
};
if(trigtype=='c') {
  status= cshmGlobalDets();
  //if(strcmp("ALICE", getenv("VMESITE"))==0) 
  if(strcmp("ALICE", "ALICE")==0) {
    if((status & detectors)!=detectors) {
      printf("GenSwtrg: calibrated dets:%x but dets in global run(s):%x\n", 
        detectors,status);
      return 12345678;   //magic used in ctp/testclass.py
    };
  } else {
    status= detectors;
    if(msgpres==0) {
      printf("Presence of dets in glob. run not checked!!!\n");
      msgpres++;
    };
  };
};
lockBakery(&ctpshmbase->swtriggers, customer);
if( setswtrig(trigtype,roc,BC,detectors)!=0) {
  l2a=0; goto RELEASERET; //return 0;
};
#ifdef SIMVME
l2a++ ; goto RELEASERET;
#endif
while(((itr<ntriggers) && ((flag=startswtrig(&orbitnloc))))){
  if(itr==0) *orbitn= orbitnloc;
  if(flag == 1)l0++;
  else if(flag == 2)l1++;
  else if(flag == 3)l2r++;
  else if(flag == 4)l2a++;
  else if(flag ==10)lm++;
  else {
    printf(" GenSwtrg: unexpected flag %i\n",flag);
    goto RELEASERET; //return l2a;
  }
  itr++;
  //usleep(60000000);
};
if(DBGswtrg3==1) {
int ifo;
for(ifo=0;ifo<NFO;ifo++){   // set all FOs always
  //printf("FO:%d\n",ifo);
  if((notInCrate(ifo+FO1BOARD)==0)) {
    w32 vmeaddr,val;
    vmeaddr= FO_TESTCLUSTER+BSP*(ifo+1);
    val=vmer32(vmeaddr);
    //if((DBGswtrg3==1)&&(val!=0)) printf("setswtrig FOr:%d Raddr: 0x%x data: 0x%x\n",
    if(val!=ifoglob[ifo]) printf("ERROR setswtrig FOr:%d Raddr: 0x%x dataw r: 0x%x 0x%x\n",
      ifo, vmeaddr, ifoglob[ifo], val);
  }
};
};
RELEASERET:
unlockBakery(&ctpshmbase->swtriggers, customer);
if(DBGswtrg4) {
  printf(" GenSwtrg: %i %c-triggers generated for detectors:0x%x.\n",
    itr,trigtype, detectors);
  printf("lm, l0,l1,l2r,l2a: %i %i %i %i \n",l0,l1,l2r,l2a);
};
TRIGTYPE='.';
// return i;
return l2a;
}
/* see ctplib.h */
int  GenSwtrg_op(int ntriggers,char trigtype, int roc, w32 BC,w32 detectors) {
w32 orbitn;
return(GenSwtrg(ntriggers, trigtype, roc, BC, detectors, 2, &orbitn ));
}
/* called only in case of problem with gcalib. Idea: print
out (vmew32f()) all vme access */
/*von
int GenSwtrg2(int ntriggers,char trigtype, int roc, w32 BC,w32 detectors, int customer ){
int itr=0;
int l0=0,l1=0,l2a=0,l2r=0;
w32 status;
status=vmer32(L0_TCSTATUS);
if((status&0x10)==0x10){
  printf(" GenSwtrg: TC busy. L0 TC_STATUS:0x%x\n", status);
  return 0;
}
if(detectors & 0x20000) {
  printf("GenSwtrg: CTP (ECS number 17) cannot be sw triggered, bit17 removed\n");
  detectors= detectors & (~0x20000);
};
if(trigtype=='c') {
  status= cshmGlobalDets();
  if((status & detectors)!=detectors) {
    printf("GenSwtrg: calibrated dets:%x but dets in global run(s):%x\n", 
      detectors,status);
    return 12345678;   //magic used in ctp/testclass.py
  };
};
lockBakery(&ctpshmbase->swtriggers, customer);
if( setswtrig2(trigtype,roc,BC,detectors)!=0) {
  l2a=0; goto RELEASERET; //return 0;
};
RELEASERET:
unlockBakery(&ctpshmbase->swtriggers, customer);
if(DBGswtrg2) {
  printf(" GenSwtrg: %i %c-triggers generated for detectors:0x%x.\n",
    itr,trigtype, detectors);
  printf("l0,l1,l2r,l2a: %i %i %i %i \n",l0,l1,l2r,l2a);
};
TRIGTYPE='.';
// return i;
return l2a;
}
*/
