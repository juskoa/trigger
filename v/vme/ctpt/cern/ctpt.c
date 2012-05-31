/*BOARD ctp 0x820000 0xd000
Contents: */
/*---------------------------------------------------------------- scope  */
/*---------------------------------------------------------------- INT  */
/*---------------------------------------------------------------- BUSY */
/*---------------------------------------------------------------- FO */
/*---------------------------------------------------------------- L0-swtrig */
/*---------------------------------------------------------------- L0-Classes */
/*---------------------------------------------------------------- L0-Shared */
/*- ctplib ------------------------------------------------------- Counters*/
/*---------------------------------------------------------------- L0-PF */
/*---------------------------------------------------------------- L0-tests */
/*---------------------------------------------------------------- busy-tests */
/*---------------------------------------------------------------- ADC-tests */
/*---------------------------------------------------------Flash/FPGA */
/*-------------------------------------------------------- all the boards */
/* history:
13.11. get/setScopeSignal() adjusted for Abis/Bbis signals
     + bug fixed: mask 0x1e0 -right mask is 0x3e0 (for B channel)
22.11.
another L0 (base: 82c000) in crate. To access this board:
- vmecrate VME2FPGA 0x82c000 (it takes ...CFG/l0/l0.rbf according
  to board identification register which is the L0
  !!! following has to be considered, when upgrading ctp ~aj->~trigger
- ctpboards ->exchanged dials for INT and L0
  (should be: l0:9 int:12)
- BICfile constant (and file itself) modified
- ctp.h addresses for L0 board modified (should be:0x9... On L0-aj is: 0xc...)
27.3.2006
- getFO/setFO modified:
  - the FO VME registers mirrored in memory (FOs)
  - automatic setting of BUSY board when FO(s) updated -see updateBusyClusts()
*/
#include <stdio.h>
#include <stdlib.h>   /* abs() */
#include <string.h>
#include <unistd.h>    /* usleep() */
#include <math.h>    /* fabs() */
#include "vmewrap.h"
#include "ctp.h"
#include "ssmctp.h"
#ifdef SSMCONNECTIONS 
void initNames();
#endif

void setseeds(int, int);
double rnlx();

Tklas Klas[50];
Tfanout FOs[6];   /* place for 6 fanouts, see getFO(), setFO() */

typedef struct {
  w32 pPF_COMMON;
  w32 pPFBLOCK_A[5];
  w32 pPFBLOCK_B[5];
  w32 pPFLUT[5];
} Tpfc;
Tpfc PF[3]; /* for L0,1,2 */ /*={ */ 
  /*
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
  */ /*
  {0,{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},
  {0,{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}},
  {0,{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}}
  }; 
  */
void initSSM();

/*HIDDEN Common */
/*FGROUP TOP GUI CTP_Classes "Classes"
The Classes definition, i.e. for each (1-50) class: 
 -enabling/disabling
 -L0,L1,L2 inputs and selectable vetos
 -cluster
 -L0 pre-scaler
*/
/*FGROUP TOP GUI CTP_Clusters "FOs/Clusters"
The Clusters definition, i.e. which detectors (LTUs) belong to each cluster.
Cluster definition consist of 'assignment of clusters to
each Fanout connector'.
*/
/*FGROUP TOP GUI Resources "Shared resources"
The Shared resources definition. They are:
- random generators(2) and BC generators(2) rates
- 3 Interaction functions (16 bits lookup tables)
- 2 L0 input functions (16 bits lookup tables)
- 2 Interaction selectors
- All/Rare events CTP option
- 5 Past/Future protection circuits for all trigger levels 
    (PF1-PF4 for physics and PF5 for Test Class)
*/
/*FGROUP TOP GUI Counters
Displays counters on all CTP boards
*/
/*FGROUP TOP GUI CheckTestClass "Test class"
Test class control.
*/
/*FGROUP TOP GUI ScopeAB "Scope Signals"
Signal selection for front panel 
A,B outputs
*/

/*FGROUP TOP GUI SSMbrowser
Browse CTP snapshot memories 
*/
/*FGROUP TOP GUI SSMcontrol
The CTP snapshot memories control
*/

/*---------------------------------------------------------------- scope  */
/*FGROUP DbgScopeCalls
rc: the number of the board (0-10) which has its ab output enabled
    -1 if there is no enabled board
Note: by running this subroutine, all boards are checked, and
      if there is more 'enabled' boards then 1, they are disabled
      (but the first enabled one)
*/
int checkScopeBoard(char ab) {
int ix, rc=-1;
w32 status,enmask=0;
if(ab=='A') enmask=0x400;
if(ab=='B') enmask=0x800;
if(enmask==0) {
  printf("internal error in checkScopeBoard\n");
  return(rc);
};
for(ix=0; ix<NCTPBOARDS; ix++) {
  if(notInCrate(ix)) continue;
  status=vmer32(SCOPE_SELECT+BSP*ctpboards[ix].dial);
  if(status & enmask) {
    if(rc==-1) {
      rc=ix;   /* this board enabled */ 
      continue;
    } else {   /* more than 1 board enabled, disable it: */
      w32 disst;
      disst= status & ~enmask;
      vmew32(SCOPE_SELECT+BSP*ctpboards[ix].dial, disst);
    };
  };
};
return(rc);
}
/*FGROUP DbgScopeCalls
Enable 1 board scope output.
rc: the number of the board (0-10) which has its ab output enabled
    -1 if there is no enabled board
Note: by running this subroutine, all boards are checked, and
      if there is more 'enabled' boards then 1, they are disabled
      (but the first enabled one)
*/
int setScopeBoard(char ab, int board) {
int ix, rc=-1;
w32 status,enmask=0;
if(ab=='A') enmask=0x400;
if(ab=='B') enmask=0x800;
if(enmask==0) {
  printf("internal error in setScopeBoard\n");
  return(rc);
};
for(ix=0; ix<NCTPBOARDS; ix++) {
  if(notInCrate(ix)) continue;
  status=vmer32(SCOPE_SELECT+BSP*ctpboards[ix].dial);
  if(ix==board) {
    status= status | enmask;
    rc=ix;
  } else {
    status= status & ~enmask;
  };
  vmew32(SCOPE_SELECT+BSP*ctpboards[ix].dial, status);
};
return(rc);
}
/*FGROUP DbgScopeCalls
rc: the number of the signal choosen for this board's ab output
    0x1000+0..31  -> Abis signals
    0x100 +0..31  -> Bbis signals
*/
int getScopeSignal(int board, char ab) {
int rc=-2;
w32 status,smask=0;
if(notInCrate(board)) {
  printf("%d not in crate\n",board);
  return(-1);
};
status=vmer32(SCOPE_SELECT+BSP*ctpboards[board].dial);
if(ab=='A') {
/*  smask=0x01f; */
  smask=0x101f;            /* 0x1000 -Abis bit */
  rc= status&smask;
}  else if(ab=='B') {
/*  smask=0x1e0; */
  smask=0x23e0;            /* 0x2000 -Bbis bit */
  rc= (status&smask) >> 5;
}  else {
  printf("internal error in getScopeSignal\n");
  return(rc);
};
/*if(rc==23) rc=-1; */
return(rc);
}
/*FGROUP DbgScopeCalls
signal: 0x1000+0..23  -> Abis signals
        0x100 +0..23  -> Bbis signals
rc: the number of the signal choosen for this board's ab output
    -1 in case of error
*/
int setScopeSignal(int board, char ab, int signal) {
int rc=-1;
w32 newstatus,status,smask=0;
if(notInCrate(board)) return(-1);
status=vmer32(SCOPE_SELECT+BSP*ctpboards[board].dial);
if(ab=='A') {
  smask=0x101f;
  newstatus= (status & ~smask) | (signal &smask);
  rc= signal&smask;
}  else if(ab=='B') {
  smask=0x23e0;
  newstatus= (status & ~smask) | ((signal<<5) &smask);
  rc= signal&(smask>>5);
}  else {
  printf("internal error in getScopeSignal\n");
  return(rc);
};
vmew32(SCOPE_SELECT+BSP*ctpboards[board].dial, newstatus);
return(rc);
}

/*FGROUP DbgScopeCalls
rc: word with the bits corresponding to CTP boards in VMERW-Scope mode
    0 -no board in VMERW-Scope mode
    0x7ff -all boards in VMERW-Scope mode
*/
int getVMERWScope() {
int ix;
w32 status,rc=0;
for(ix=0; ix<NCTPBOARDS; ix++) {
  if(notInCrate(ix)) continue;
  status=vmer32(TEST_ADD+BSP*ctpboards[ix].dial) & 0x2;   /* VMERW-Scope bit */
  if(status!=0) {
    rc= rc | (1<ix);
  };
};
return(rc);
}

/*FGROUP DbgScopeCalls
newv: new settings     oldv: old settings
*/
void setVMERWScope(w32 newv, w32 oldv) {
int ix,bits;
w32 msk, newm, oldm, newbit;
for(ix=0; ix<NCTPBOARDS; ix++) {
  if(notInCrate(ix)) continue;
  msk= 1<<ix; newm= newv&msk; oldm= oldv&msk;
  if( newm ^ oldm) {   /* different setting */
   if(newm != 0) newbit=0x2;
   else newbit=0;
   vmew32(TEST_ADD+BSP*ctpboards[ix].dial, newbit);
   bits++;
  };
};
if(bits!=1) {
  printf("setVMERWScope: internal error %d >1\n", bits);
};
}

/*FGROUP File GUI SaveFile Save2file
- Save configuration to WORK/.cfg configuration to file
*/
/*FGROUP File GUI LoadFile "Load file"
- Load configuration from WORK/.cfg configuration to file
Todo:
- check hw configuration with the memory-configuration
- Load (i.e. add) partition configuration (i.e. from .configuration
  file prepared by partition editor)
*/
/*FGROUP File GUI Readhw
- Load configuration from CTP boards
*/
/*FGROUP File GUI Write2hw
- Write configuration into CTP boards
*/

/* FGROUP Common
A,B: 2x5bits for Scope A,B outputs
setAB(23,23) -no output selected

void setAB(w32 A, w32 B);
*/

/*---------------------------------------------------------------- INT  */
/*FGROUP DBGint
read/print 2 counters: L2_ORBIT_READ and INT_ORBIT_READ */
void readORBIT_READs() {
w32 l2or, intor;
w32 seconds1,micseconds1, seconds2,micseconds2,diff;
int cd;
GetMicSec(&seconds1, &micseconds1);
l2or= vmer32(L2_ORBIT_READ);
intor= vmer32(INT_ORBIT_READ);
GetMicSec(&seconds2, &micseconds2);
diff=DiffSecUsec(seconds2, micseconds2, seconds1, micseconds1);
cd= l2or-intor;
printf("l2:%x int:%x l2-int:%x(%d) micsecs:%d\n", l2or, intor, cd,cd, diff);
}
#define MICSEC1 100
/* ret: 1 if timeout  0: not busy */
/*--------------------------------------------------*/ int i2cwait(int phase) {
w32 itcr; int busytime=0;
while(1) {
  itcr= vmer32(I2C_SET);
  if((itcr & 0x2000) == 0) break;
  usleep(MICSEC1);
  busytime= busytime+MICSEC1;
  if(busytime>3000) {
    printf("i2cwait: timeout (%d >  3000 micsecs) phase:%d\n",
      busytime, phase);
    return(1);
  };
};
if((itcr & 0x1000)==0x1000) {
  //printf("Err bit on found in i2cwait(%d)\n",phase);
  return 2;
};
return(0);
}
void vme2volt(w32 vme ){
float volt5,volt3_3,volt1_5,volt5b;
 volt5=(vme & 0xff)*23.725;
 volt5b=((vme & 0xff000000)>>24)*23.725;
 volt3_3=((vme & 0xff00)>>8)*23.725;
 volt1_5=((vme & 0xff0000)>>16)*12.941;
 printf("%4.0f %4.0f %4.0f %4.0f [mV]\n",volt5,volt3_3,volt1_5,volt5b);
}
/*FGROUP DBGint
read voltages from 1 board.
channel: 0-7
branch:  0-7  */
void i2cread(int channel, int branch) {
w32 cb, i2crd; //i2csetrd; int i;
cb= 0x80 | (channel<<4) | branch;
//printf("i2cread: cb=0x%x \n",cb);
//if(i2cwait(1)) goto RET; because I2C_SET remembers last error 
vmew32(I2C_SET, cb); 
//if(i2cwait(2)) goto RET; because I2C_SET remembers last error
vmew32(I2C_MUXWR, DUMMYVAL); if(i2cwait(3)) goto RET;
vmew32(I2C_MUXRD, DUMMYVAL); if(i2cwait(4)) goto RET;
  //i2csetrd= vmer32(I2C_SET); if(i2cwait(5)) goto RET;   //only for test
  //printf("   I2C_SET reading:%x\n", i2csetrd);
vmew32(I2C_ADCWR, DUMMYVAL); if(i2cwait(6)) goto RET;
//for(i=0; i<100000; i++) {
vmew32(I2C_ADCRD, DUMMYVAL); if(i2cwait(7)) goto RET;
//};
i2crd= vmer32(I2C_DATA); if(i2cwait(8)) goto RET;
printf("channel:%d branch:%d  reading:%x: ", channel, branch,i2crd);
vme2volt(i2crd);
return;
RET:
printf("channel:%d branch:%d  device not responding.\n", channel, branch);
return;
}
/*---------------------------------------------------------------- BUSY */
/*
Operation:
- read all the FO_CLUSTER,FO_TESTCLUSTER words (as stored
  in computer mem.) and set correspondingly the BUSY board
*/
void updateBusyClusts() {
int ixfo,ixltu,ltu,iclu;
w32 clusters;
w32 bsy[7];
for(iclu=0; iclu<7; iclu++) {
  bsy[iclu]=0;
};
for(ixfo=0; ixfo<6; ixfo++) {
  for(ixltu=0; ixltu<4; ixltu++) {
    ltu= ixfo*4+ixltu;   /* 0..23 */
    clusters= ((FOs[ixfo].cluster >> (8*ixltu)) & 0x3f)<<1; // 654321-
    if(FOs[ixfo].tcluster & (1<<(ixltu+16))) {
      clusters= clusters | 1;   //654321T
    };
    for(iclu=0; iclu<7; iclu++) {
      if(clusters & (1<<iclu)) {
        bsy[iclu]= bsy[iclu] | (1<<ltu);
      };
    };
  };
};
/* update BUSY board: */
for(iclu=0; iclu<7; iclu++) {
  //printf("%d: %8x\n",iclu,bsy[iclu]);
  vmew32(BUSY_CLUSTER+4*iclu, bsy[iclu]);
};
printf("BUSY clusters updated on BUSY board according to FO board\n");
}
/*FGROUP busy
Set daqbsy, T,1,2,3,4,5,6 SET_CLUSTER word on the BUSY board
*/
void setClusters(w32 daqbsy, w32 tc,w32 c1,w32 c2,w32 c3,w32 c4,w32 c5,w32 c6) {
vmew32(BUSY_DAQBUSY, daqbsy);
if(daqbsy !=0) {
  vmew32(DAQ_LED, 1);
} else {
  vmew32(DAQ_LED, 0);
};
vmew32(BUSY_CLUSTER, tc);
vmew32(BUSY_CLUSTER+4, c1);
vmew32(BUSY_CLUSTER+8, c2);
vmew32(BUSY_CLUSTER+12, c3);
vmew32(BUSY_CLUSTER+16, c4);
vmew32(BUSY_CLUSTER+20, c5);
vmew32(BUSY_CLUSTER+24, c6);
}
/*FGROUP busy
Read T,1,2,3,4,5,6 SET_CLUSTER word on the BUSY board and print it as 1 line
*/
void getClusters() {
int iclu; w32 cls[7];
char line[MAXLINE];
line[0]='\0';
for(iclu=0; iclu<7; iclu++) {
  //printf("%d: %8x\n",iclu,bsy[iclu]);
  cls[iclu]= vmer32(BUSY_CLUSTER+4*iclu);
  sprintf(line,"%s 0x%x", line, cls[iclu]);
}; printf("%s\n", line);
}
/* FGROUP busy
Set DAQ_BUSY word
void setDAQbusy(w32 daqbusy) {
w32 daqbusy;
vmew32(BUSY_DAQBUSY, daqbusy);
}*/
/*FGROUP busy
Read DAQ_BUSY word */
void getDAQbusy() {
w32 daqbusy;
daqbusy= vmer32(BUSY_DAQBUSY);
printf("0x%x\n", daqbusy);
}
/*---------------------------------------------------------------- FO */
void  setFOtcluster(int ix, w32 tcluster) {
  vmew32(FO_TESTCLUSTER+BSP*ctpboards[ix].dial, tcluster);
  FOs[ix-FO1BOARD].tcluster= tcluster;
}
/*FGROUP dbghw
ix: FO1BOARD..FO1BOARD+5   (6 FO boards)
always read FO board's CLUSTER, TEST_CLUSTER words
STDOUT:
- no output                if ix FO is not in the crate
cluster testcluster        for FO in the crate
*/
void getFO(int ix) {
w32 FOcluster,FOtestcluster;
if(notInCrate(ix)) return;
FOcluster= vmer32(FO_CLUSTER+BSP*ctpboards[ix].dial);
FOtestcluster= vmer32(FO_TESTCLUSTER+BSP*ctpboards[ix].dial);
FOs[ix-FO1BOARD].cluster= FOcluster;
FOs[ix-FO1BOARD].tcluster= FOtestcluster;
printf("0x%x 0x%x\n", FOcluster, FOtestcluster);
}
/*FGROUP dbghw
write to FO board's registers only if modified
ix: FO1BOARD..FO1BOARD+5   (i.e. 5-10, 6 FO boards)
*/
void setFO(int ix, w32 cluster, w32 tcluster) {
if(notInCrate(ix)) {
  printf("setFO error %d board not in the crate\n",ix); return;
};
//if(cluster != FOcluster) {
  vmew32(FO_CLUSTER+BSP*ctpboards[ix].dial, cluster);
  FOs[ix-FO1BOARD].cluster= cluster;
//};
//if(tcluster != FOtestcluster) {
  setFOtcluster(ix, tcluster);
//};
updateBusyClusts();
}
/*FGROUP dbghw
set cal. flag and roc on ALL the FO boards
*/
void setFOrocs(int calflag, w32 roc) {
int ix;
w32 tcluster;
roc= roc&0xf;
tcluster= roc |(roc<<4) | (roc<<8) | (roc<<12);
if(calflag !=0) tcluster =tcluster | 0x100000;
for(ix=FO1BOARD; ix<= FO1BOARD+5; ix++) {
  if(notInCrate(ix)) continue;
  setFOtcluster(ix, tcluster);
};  /* no need to call updateBusyClusts() (LTU assignments did not change) */
}

/*---------------------------------------------------------------- L0-swtrig */
/*FGROUP L0
Start SW trigger.
Input:
ssm: 1   ->start SSM recording in L0 output moinitoring mode before
           starting trigger
Operation:
- clear flags
- start SSM (optionally)
- start sw trigger
- usleep(200) -micsecs
- read L0_TCSTATUS word
Output:
L0_TCSTATUS word
ToDo: -for whic detector? 
      - ROC bits to be written to FO and INT board
*/
w32 swtrigger(int ssm) {
w32 status;
int brd=1,rc;
vmew32(L0_TCCLEAR, DUMMYVAL);
if(ssm==1) {
  rc= setomSSM(brd, 0x102); rc= startSSM1(brd); 
  setsmssw(1,"l0_outmon");
};
vmew32(L0_TCSTART, DUMMYVAL);
usleep(200);
status= vmer32(L0_TCSTATUS);
return(status);
}

/*FGROUP L0
ret: TCSTATUS bits from L0,1,2 boards:
Bits:
4-0  L0_TCSTATUS bits
5    L1_TCSTATUS bit3 (L1ack flag)
7-6  L2_TCSTATUS bits (L2Aack and L2Rack)
*/
w32 getTCSTATUS() {
w32 status,st;
status= vmer32(L0_TCSTATUS)&0x1f;
st= (vmer32(L1_TCSTATUS)&0x8)<<2; status= status | st;
st= (vmer32(L2_TCSTATUS)&0xc)<<4; status= status | st;
return(status);
}
/*FGROUP L0
ret: TC_SET bits from L0,1,2 boards:
Bits:
18-0  L0_TC_SET bits
19   L1_TC_SET bit18 (P/F veto)
20   L2_TC_SET bit24 (P/F/veto)
*/
w32 getTCSET() {
w32 status,st;
status= vmer32(L0_TCSET)&0x7ffff;
st= (vmer32(L1_TCSET)&0x40000)<<1; status= status | st;
st= (vmer32(L2_TCSET)&0x1000000)>>4; status= status | st;
return(status);
}
/*FGROUP L0
tcset012 -extended mening (bits 18,19 -see getTCSET() )
*/
void setTCSET(w32 tcset012, w32 dets) {
w32 tcs2;
vmew32(L0_TCSET, tcset012&0x7ffff);
vmew32(L1_TCSET, (tcset012>>1)&0x40000);
tcs2= (tcset012<<4)&0x1000000;
vmew32(L2_TCSET, (tcs2| (dets&0xffffff)) );
}
/*FGROUP L0
operation: clear TC flags on all the boards
*/
void clearTC() {
vmew32(L0_TCCLEAR,DUMMYVAL);
vmew32(L1_TCCLEAR,DUMMYVAL);
vmew32(L2_TCCLEAR,DUMMYVAL);
}
/*---------------------------------------------------------------- L0-Classes */
Tklas *getpClass(int klas) {
/* read klas from HW to memory */
int bb, klasix; w32 mskbit1,mskbit2;
klasix= klas-1; bb= klas*4; 
if(notInCrate(1)) {
  Klas[klasix].regs[0]= 0;
  Klas[klasix].regs[1]= 0;
  Klas[klasix].regs[2]= 0;
  Klas[klasix].regs[3]= 0;
} else {
  /* not readable in A0 version, now readable: */
  Klas[klasix].regs[0]= vmer32(L0_CONDITION+bb);
  if(klas>=45) {  /* only for inverted klasses ! */
    Klas[klasix].regs[1]= vmer32(L0_INVERT+bb);
  }else {
    Klas[klasix].regs[1]= 0;
  };
  mskbit1= vmer32(L0_MASK+bb)&0x1;
  mskbit2= (vmer32(L0_VETO+bb)&0xffff) | (mskbit1<<16);
  Klas[klasix].regs[2]= mskbit2;
  /* scalers are updated in 1 pass (see hw2rates)
  Klas[klasix].regs[3]= 0xabeceda;  */
};
if(notInCrate(2)) {
  Klas[klasix].regs[4]= 0;
  Klas[klasix].regs[5]= 0;
} else {
  Klas[klasix].regs[4]= vmer32(L1_DEFINITION+bb);
  Klas[klasix].regs[5]= vmer32(L1_INVERT+bb);
};
if(notInCrate(3)) {
  Klas[klasix].regs[6]= 0;
} else {
  Klas[klasix].regs[6]= vmer32(L2_DEFINITION+bb);
};
return &Klas[klasix];
}
/*FGROUP L0
klas: 1-50   -class number
get L0_CONDITION L0_INVERT L0_VETO L0_PRESCALER 
    L1_DEFINITION L1_INVERT L2_DEFINITION words for klas

L0_INVERT, L1_INVERT are 0:for classes 1-44 or 
corresponding board is not in the crate
*/
void getClass(int klas) {
Tklas *klasixp;
int ix;
klasixp= getpClass(klas);
for(ix=0; ix<MAXL0REGS; ix++) {
  printf("0x%x ", klasixp->regs[ix]); 
}; printf("\n");
}

/*FGROUP L0
set L0_CONDITION L0_INVERT L0_VETO L0_PRESCALER 
    L1_DEFINITION L1_INVERT L2_DEFINITION
words for klas (1..50)
ATTENTION: 
1. bit17 (0x10000) of veto is CLASS MASK bit written into bit0 of L0_MASK
2. invert,l1invert -valid only for class>=45
*/
void setClass(int klas,w32 condition, w32 invert, w32 veto, w32 scaler,
              w32 l1def, w32 l1invert, w32 l2def) {
int bb,klasix; w32 mskbit;
bb= klas*4; klasix=klas-1;
if(notInCrate(1)==0) {   // L0 board
  Klas[klasix].regs[0]= condition; vmew32(L0_CONDITION+bb, condition);
  if(klas>=45) {  /* only for inverted klasses ! */
    Klas[klasix].regs[1]= invert; vmew32(L0_INVERT+bb, invert);
  };
  Klas[klasix].regs[2]= veto; vmew32(L0_VETO+bb, veto&0xffff); 
  /* 1st L0 version (A0): vmew32(L0_MASK+bb, veto&0x10000); */
  mskbit= (veto&0x10000)>>16; vmew32(L0_MASK+bb, mskbit);
  Klas[klasix].regs[3]= scaler; /* update is done in 1 pass in rates2hw */
};
if(notInCrate(2)==0) {   // L1 board
  Klas[klasix].regs[4]= l1def; vmew32(L1_DEFINITION+bb, l1def);
  if(klas>=45) {  /* only for inverted klasses ! */
    Klas[klasix].regs[5]= l1invert; vmew32(L1_INVERT+bb, l1invert);
  };
};
if(notInCrate(3)==0) {
  Klas[klasix].regs[6]= l2def; vmew32(L2_DEFINITION+bb, l2def);
};
}

/*FGROUP L0
disable all 50 classes, i.e.:
- set all inputs,vetos as dontcare for all 50 classes i.e.:
L0_CONDITION = 0xffffffff
L0_VETO      = 0xfffffff0   (cluster0)
and 0x0 in:
L0_INVERT   =0
L0_PRESCALER=0
*/
void disableClasses() {
int klas;
if(notInCrate(1)) return;
for(klas=1; klas<=50; klas++) {
  setClass(klas, 0xffffffff, 0, 0xfffff0, 0, 0x0fffffff, 0,0x0f000fff);
};
}

/*FGROUP L0
read all rates (scalers) from hw to Klas structure
*/
void hw2rates() {
int ix;
vmew32(RATE_MODE,1);   /* vme mode */
vmew32(RATE_CLEARADD,DUMMYVAL);
for(ix=0; ix<50; ix++) {
  Klas[ix].regs[3]= vmer32(RATE_DATA)&0x1fffff;
};
vmew32(RATE_MODE,0);   /* normal mode */
/*printf("hw2rates.\n"); */
}
/*FGROUP L0
write all rates (scalers) from Klas structure to hw
*/
void rates2hw() {
int ix;
vmew32(RATE_MODE,1);   /* vme mode */
vmew32(RATE_CLEARADD,DUMMYVAL);
for(ix=0; ix<50; ix++) {
  vmew32(RATE_DATA, Klas[ix].regs[3]);
};
vmew32(RATE_MODE,0);   /* normal mode */
}

/*---------------------------------------------------------------- L0-Shared */
/*FGROUP L0
get rnd1 rnd2 bcsc1 bcsd2 int1 int2 intt L0fun1 L0fun2 INTSEL1 INTSEL2 allrare
*/
void getShared() {
w32 w;
if(notInCrate(1)) return;
w= vmer32(RANDOM_1); printf("0x%x\n", w);
w= vmer32(RANDOM_2); printf("0x%x\n", w);
w= vmer32(SCALED_1); printf("0x%x\n", w);
w= vmer32(SCALED_2); printf("0x%x\n", w);
w= vmer32(L0_INTERACT1); printf("0x%x\n", w);
w= vmer32(L0_INTERACT2); printf("0x%x\n", w);
w= vmer32(L0_INTERACTT); printf("0x%x\n", w);
w= vmer32(L0_FUNCTION1); printf("0x%x\n", w);
w= vmer32(L0_FUNCTION2); printf("0x%x\n", w);
w= vmer32(L0_INTERACTSEL); 
printf("0x%x\n", w&0x1f); printf("0x%x\n", ((w>>5)&0x1f));
w= vmer32(ALL_RARE_FLAG); printf("0x%x\n", w&0x1);
}

/*FGROUP L0
set rnd1 rnd2 bcsc1 bcsd2 int1 int2 intt L0fun1 L0fun2
*/
void setShared(w32 r1,w32 r2,w32 bs1,w32 bs2,
               w32 int1,w32 int2,w32 intt,w32 l0fun1,w32 l0fun2) {
if(notInCrate(1)) return;
vmew32(RANDOM_1, r1);
vmew32(RANDOM_2, r2);
vmew32(SCALED_1, bs1);
vmew32(SCALED_2, bs2);
vmew32(L0_INTERACT1, int1);
vmew32(L0_INTERACT2, int2);
vmew32(L0_INTERACTT, intt);
vmew32(L0_FUNCTION1, l0fun1);
vmew32(L0_FUNCTION2, l0fun2);
}
/*FGROUP L0
set INTERACTSEL ALL_RARE_FLAG
*/
void setShared2(w32 intsel, w32 allrare) {
if(notInCrate(1)) return;
vmew32(L0_INTERACTSEL, intsel);
vmew32(ALL_RARE_FLAG , allrare);
}
/*FGROUP L0
check spy memory (256 words from 0x9400 on L0 board )
Operation: 
- write a, a+1 a+2 ... 
- read back and check
- write 0s to all the 256 words
*/
void clearSPY(int board) {
#define RNLXMASK 0xffffffff
int bb, ix; w32 rnd, data;
bb= BSP*ctpboards[board].dial;
setseeds(7,3);
for(ix=0; ix<256; ix++) {
  rnd= RNLXMASK*rnlx(); rnd= RNLXMASK&rnd;
  vmew32(SPY_MEMORY+bb, rnd);
  data= vmer32(SPY_MEMORY+bb);
  if(rnd != data) {
    printf("clearSPY error: got:0x%x expected: 0x%x\n",data,rnd);
  };
};
for(ix=0; ix<256; ix++) {
  vmew32(SPY_MEMORY+bb, 0);
};
printf("256 words of spy memory on %s tested and cleared\n", ctpboards[board].name);
}
/*FGROUP L0
read BC masks from HW and print out 3564 4bits words
*/
void getBCmasks() {
int ix; char m4[ORBITLENGTH+1];
vmew32(MASK_MODE,1);   /* vme mode */
vmew32(MASK_CLEARADD,DUMMYVAL);
for(ix=0; ix<ORBITLENGTH; ix++) {
  w32 c;
  c=vmer32(MASK_DATA)&0xf;
  if(c>=10)
    m4[ix]= c-10+'a';
  else
    m4[ix]= c+'0';
  /*  printf("%x",vmer32(MASK_DATA)&0xf); */
};
m4[ORBITLENGTH]='\0';
vmew32(MASK_MODE,0);   /* normal mode */
printf("%s\n",m4);
}
/*FGROUP L0
set BC masks in HW from input line containing 3564 hexa-chars.
*/
void setBCmasks() {
w32 val;
int ix,strl; char m4[ORBITLENGTH+3];
fgets(m4,ORBITLENGTH+2,stdin);
strl= strlen(m4)-1;
if(strl!= ORBITLENGTH) {
  printf("Bad input for setBCmask length:%d\n",strl);
/*  return; */
  if(strl>ORBITLENGTH) strl=ORBITLENGTH;
  printf("anyhow, the %d bytes will be written to hw\n",strl);
};
vmew32(MASK_MODE,1);   /* vme mode */
vmew32(MASK_CLEARADD,DUMMYVAL);
for(ix=0; ix<strl; ix++) {
  int c;
  c=m4[ix];
  if((c>='a') && (c<='f'))
    val= c - 'a' +10;
  else
    val= c-'0';
  /*  if(ix<80) printf("%x",val); */
  vmew32(MASK_DATA,val);
  /*  printf("%x",vmer32(MASK_DATA)&0xf); */
};
vmew32(MASK_MODE,0);   /* normal mode */
printf("written to hw:%d\n",strl);
}

/*FGROUP L0
set/read/check ntimes
words: if 0 than checj whole BCmask memory (3564)
*/
void checkBCmasks(int ntimes, int words) {
w32 val;
int ix,strl, cycles; //char m4[ORBITLENGTH+3];
if(words==0)
  strl=ORBITLENGTH;
else
  strl=words;
for(cycles=0; cycles<ntimes; cycles++) {
  vmew32(MASK_MODE,1);   /* vme mode */
  vmew32(MASK_CLEARADD,DUMMYVAL);
  for(ix=0; ix<strl; ix++) {
    val=ix&0xf;
    vmew32(MASK_DATA,val);
    /*  printf("%x",vmer32(MASK_DATA)&0xf); */
  };
  vmew32(MASK_MODE,0);   /* normal mode */
  /*  printf("written to hw:%d\n",strl); */
  /* read and check; */
  vmew32(MASK_MODE,1);   /* vme mode */
  vmew32(MASK_CLEARADD,DUMMYVAL);
  for(ix=0; ix<strl; ix++) {
    w32 valr;
    val=ix&0xf;
    valr= vmer32(MASK_DATA)&0xf;
    if(val != valr) {
      printf("Error at %d exp:%x got:%x\n",ix,val,valr);
    };
  };
  vmew32(MASK_MODE,0);   /* normal mode */
  printf("read/checked:%d\n",strl);
};
}

/*---------------------------------------------------------------- L0-PF */
/*FGROUP L0
get PF parameters for 1 board (L0, L1, or L2 -> ix= 1, 2 or 3)
*/
void getPF(int ix) {
int bb,ixa;
if(notInCrate(ix)) return;
bb= BSP*ctpboards[ix].dial;
ixa=ix-1;
PF[ixa].pPF_COMMON= vmer32(PF_COMMON+bb);
printf("0x%x\n", PF[ixa].pPF_COMMON);
}

/*FGROUP L0
get PF parameters for 1 circuit 
I:
L0, L1, or L2 -> ix= 1, 2 or 3
circ -> 1..5
O: on stdout: 3 hexadecimal numbers: PFBLOCK_A, PFBLOCK_B, PFLUT
*/
void getPFc(int ix, int circ) {
int bb,ixa, ixc, i;
w32 adr0, vals[3];
char os[80]="";

if(notInCrate(ix)) return;
bb= BSP*ctpboards[ix].dial;
ixa=ix-1; ixc=circ-1; adr0= PFBLOCK_A+bb+(ixc*12);
  /* instead get from memory for dbg: 
  vals[0]=PF[ixa].pPFBLOCK_A[ixc];
  vals[1]=PF[ixa].pPFBLOCK_B[ixc];
  vals[2]=PF[ixa].pPFLUT[ixc]; */
for(i=0; i<3; i++) {
  vals[i]= vmer32(adr0);
  sprintf(os, "%s0x%x ", os, vals[i]);
  adr0= adr0+4;
};
PF[ixa].pPFBLOCK_A[ixc]= vals[0];
PF[ixa].pPFBLOCK_B[ixc]= vals[1];
PF[ixa].pPFLUT[ixc]= vals[2];
printf("%s\n", os);
}

/*FGROUP L0
set PF parameters for 1 board (L0, L1, or L2 -> ix= 1, 2 or 3)
pfc: PF_COMMON word
*/
void setPF(int ix, w32 pfc) {
int bb,ixa;
if(notInCrate(ix)) return;
bb= BSP*ctpboards[ix].dial;
ixa=ix-1;
/*if(pfc != PF[ixa].pPF_COMMON ) { */
/*  printf("POZOR\n"); */
  PF[ixa].pPF_COMMON= pfc;
  vmew32(PF_COMMON+bb, pfc);
/*}; */
}

/*FGROUP L0
set PF parameters for 1 circuit 
I:
L0, L1, or L2 -> ix= 1, 2 or 3
circ -> 1..5
A, B, LUT -3 words to be written
*/
void setPFc(int ix, int circ, w32 A, w32 B, w32 LUT) {
int bb,ixa, ixc, i;
w32 adr0, vals[3];
w32 *vala[3];

if(notInCrate(ix)) return;
bb= BSP*ctpboards[ix].dial;
ixa=ix-1; ixc=circ-1; adr0= PFBLOCK_A+bb+(ixc*12);
vals[0]= A; vals[1]= B; vals[2]= LUT;
vala[0]= PF[ixa].pPFBLOCK_A;
vala[1]= PF[ixa].pPFBLOCK_B; 
vala[2]= PF[ixa].pPFLUT;
for(i=0; i<3; i++) {
/*  if( vals[i] != vala[i][ixc]) { 
    printf("POZOR:setPFc: circuit: %d,%d i:%d old:%x new: %x\n",
      ixa,ixc,i,vala[i][ixc],vals[i]); */
    vala[i][ixc]= vals[i];
    vmew32(adr0, vals[i]);
/*  }; */
  adr0= adr0+4;
};
}

/*---------------------------------------------------------------- L0-tests */
/*FGROUP L0
1. Prepare 24 input signals in SSM, start it in continuous input generator mode.
2. set step by step (keep 1 sec delay between settings) all 24 SYNCH_ADD
   from 0 to 15
*/
void testSYNCH() {
int bb,sync,ix1,brd,rc;
w32 sadr;
brd=1;
bb= BSP*ctpboards[brd].dial;
rc= readSSM(brd);   /* allocate memory */
for(ix1=0; ix1<Mega; ix1++) {
  if(ix1 % 8 ==0) {         
    sms[brd].sm[ix1]=0xfffffd02;   /* 24 L0 inputs + BYCLSTT */
  } else {
    sms[brd].sm[ix1]=0;
  };
};
rc= writeSSM(brd);
rc= setomSSM(brd, 0x20d); rc= startSSM1(brd); 
/*rc= setomSSM(brd, 0x20c); */
for(sync=0; sync<=15; sync++) {
  for(ix1=0; ix1<24; ix1++) {
    sadr= bb+SYNCH_ADD+4*ix1;
    vmew32(sadr, sync);
  };
/*  rc= startSSM1(brd); */
  usleep(1000000);
};
}

/*---------------------------------------------------------------- busy-tests */
/*FGROUP busy
Set BC_DELAY from 0..31 with step 1, wait 1 sec between
steps: number of steps (0..34)
*/
void testBCDELAY(int steps) {
w32 bcdelay;
for(bcdelay=0; bcdelay<steps; bcdelay++) {
  vmew32(BUSY_DELAY_ADD, bcdelay);
/*  vmew32(PLL_RESET, DUMMYVAL);  -doesn't work with pll_reset*/
  usleep(1000000);
};
}
/*FGROUP busy
set Orbit on backplane to BC/2 signal.
bit8: 1 ->set toggling     0->disable toggling
Necessary for delay measurement on l0/l1/l2 board.
*/
void setOrbitBChalf(int bit8) {
w32 eo;
eo=vmer32(BUSY_ORBIT_SELECT);
if(bit8) {
  vmew32(BUSY_ORBIT_SELECT, eo | 0x3000);
} else {
  vmew32(BUSY_ORBIT_SELECT, (eo&0xfff) | 0x2000);
};
/* this was valid with busy-test FPGA (before 1st BUSY-FPGA)
eo=vmer32(BUSY_ENABLE_OUT);
if(bit8) {
  vmew32(BUSY_ENABLE_OUT, eo | 0x100);
} else {
  vmew32(BUSY_ENABLE_OUT, eo & 0xffffeff);
};
*/
}

/*---------------------------------------------------------------- ADC-tests */
#define ADCstable 10
#define ADCtimeout 10000
/* Reads ADC checking for busy and timeout.  */
int readadc()
{
 int i=0;
 int value, brd,bb;
 brd=1;
 bb= BSP*ctpboards[brd].dial;
 vmew32(bb+ADC_START,0x0);
 while( (i<ADCtimeout) && ((vmer32(bb+ADC_DATA)&0x100) == 0x100))i++;
 if( i>=ADCtimeout) value=-1;else
  value = (vmer32(bb+ADC_DATA)&0xff);
 /*printf("adc= %i\n",value);*/
 return value; 
}
/*
Reads ADC using readadc and checking that two subsequent values are the same.
*/
int readadc_s()
{
 int value0,value1,i=0;
 value0=readadc();
 if(value0 == -1) return -1;
 while( ( (value1=readadc()) != value0) && (i<ADCstable) ){
     if(value1 == -1) return -2;
     value0=value1;
     i++;
    }
 /*printf("i=%i value=%i \n",i,value0);*/
 if(i>=ADCstable) return -3;
 return value1;
}
#define klength 8
#define plength 8
void readctpadc(float *k, float *p, float *t) {
#define MAXLILE 600
FILE *f;
char cmd[100];
char line[MAXLILE];
strcpy(cmd,"CFG/busy/"); strcat(cmd,"ctpadc.cfg");
f= fopen(cmd,"r");
if(f==NULL) {
  printf("%s -missing\n",cmd);
  return;
};
fgets(line, MAXLILE, f); //printf("%s\n",line);
/* k0-k3, k4-k7 -> k,q for NE,SE,SW,NW. k8=width, k9=Xnorth k10=Xsouth */
sscanf(line, "%f %f %f %f %f %f %f %f", 
  &k[0],&k[1],&k[2],&k[3], &k[4], &k[5], &k[6], &k[7]);
//printf("%f %f %f %f %f\n", k[0],k[1],k[2],k[3], k[4]);
fgets(line, MAXLILE, f); //printf("%s\n",line);
/* k0-k3, k4-k7 -> k,q for NE,SE,SW,NW. k8=width, k9=Xnorth k10=Xsouth */
sscanf(line, "%f %f %f %f %f %f %f %f", 
  &p[0],&p[1],&p[2],&p[3], &p[4], &p[5], &p[6], &p[7]);
fclose(f);
strcpy(cmd,"CFG/busy/"); strcat(cmd,"busydel");
f= fopen(cmd,"r");
if(f==NULL) {
  printf("%s -missing\n",cmd);
  return;
};
fgets(line, MAXLILE, f); //printf("%s\n",line);
sscanf(line, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f",
  &t[0],&t[1],&t[2],&t[3], &t[4], &t[5], &t[6], &t[7],
  &t[8],&t[9],&t[10],&t[11], &t[12], &t[13], &t[14], &t[15],
  &t[16],&t[17],&t[18],&t[19], &t[20], &t[21], &t[22], &t[23],
  &t[24],&t[25],&t[26],&t[27], &t[28], &t[29], &t[30], &t[31]
  );
fclose(f);
/* {
int ix;
for(ix=0; ix<32; ix++) {
  printf("%d: %f\n",ix, t[ix]);
};}; */
}
/* FGROUP L0
Operation: -toto pravdepodobne blbost (nehladame idealny
  delay, ale 'oneskorenie vst. signalu') -vid findNS() -hlada
  ns podla zmeraneho ADC a fdf() -vola findNS() pre test
- read W Kne Kse Ksw Knw from WORK/ctpadc.cfg
- set ticks on BUSY board
- measure ADC 
- compute delay
*/
int fastDelayFind(int ticks) {
w32 adc;
int ix;
float w,kaverage, dtres, k[klength], dt[4], p[plength],t2ns[32];
readctpadc(k,p,t2ns);
kaverage= 0.;
/*
for(ix=0; ix<4; ix++) {
  kp[ix]= k[ix];
  if(kp[ix]<0.) kp[ix]= -kp[ix];
  kaverage= kaverage+kp[ix];
kaverage= kaverage/4;
printf("width:%6.3f ticks,  average k:%6.3f\n",w,kaverage); 
*/
vmew32(BUSY_DELAY_ADD, ticks);
adc = readadc_s();
dtres=0.0;
for(ix=0; ix<4; ix++) {
  dt[ix]=-1000000.;
  if(adc > 63) {   //north
    if(ix==1 || ix==2) continue;
    dt[ix]= ticks - adc/k[ix];
    if(dt[ix] < 0) dt[ix]= dt[ix]+ w;
    if(dt[ix] >31) dt[ix]= dt[ix]- w;
  }else {
    if(ix==0 || ix==3) continue;
    dt[ix]= ticks - adc/k[ix];
    if(dt[ix] < 0) dt[ix]= dt[ix]+ w;
    if(dt[ix] >31) dt[ix]= dt[ix]- w;
  };
};
printf("adc:%d Xs:\t",adc);
for(ix=0; ix<4; ix++) {
  if(dt[ix]!=-1000000.) printf("%d: %6.3f (%6.3fns)", ix,dt[ix], dt[ix]*25/w);
}; printf("\n");
return(100);
}
float t2ns0=0.0;
/*FGROUP L0
*/
void findNS() {
w32 adc,adc2; int busyorig,del1,del2,ix, qu;
float width, del1ns;
float kq[klength];   /* k0-k3, q0-q3 */
float rdlu[plength]; /* right, down,left,top: X0 Y0 - X3 Y3 */
float t2ns[32];      /* ticks(0..31)->ns(0.. cca 29.40) */
float dif,dt;
float delayed;
readctpadc(kq, rdlu, t2ns);
width= rdlu[0] - rdlu[4];   // Xright -Xleft

/* toggling signal should be on input */
vmew32(ADC_SELECT, 0x100);   // choose input
/* Do 2 measurements with:
1. range of ADCs should be 8-60 or 66-120
2. delay between them 1 tick
*/
del1=0; del1ns=0.0;
del2= 1;             // shift in ticks for 2nd measurement
busyorig=vmer32(BUSY_DELAY_ADD); 
ix=0;
while(1) {   //max. 3 attempts
  if(busyorig==31) {
    /* we can't measure with 31 -next DELAY setting would be 32 */
    del1=del1-1; del2=del2-1;
  };
  vmew32(BUSY_DELAY_ADD, busyorig+del1);
  adc = readadc_s();
  if(((adc>8) && (adc<60)) || ((adc>66) && (adc<120))) {
    /*if((adc>26) || (adc>93)){
      del2=-del2;
    }blbina */
    vmew32(BUSY_DELAY_ADD, busyorig+del2);
    adc2 = readadc_s();
    /* adc2 should be different just by abs(k*1tick)= cca abs(1.5*5)
       (2nd measurement is always 1 tick more or 1 tick less) */
    if(abs(adc2-adc) < 1.5*5) {
      if(((adc2>8) && (adc2<60)) || ((adc2>66) && (adc2<120))) {
        vmew32(BUSY_DELAY_ADD, busyorig);
        break;
      } else {
        printf("adc2:%d",adc2);
      };
    } else {
      printf("flip for adc2, once more...\n");
      goto IX1;
    };
  };
  printf("adc1:%d",adc);
  if(busyorig < (25-del2)) {
    del1=del1+6; del2=del2+6;
  }else {
    del1=del1-6; del2=del2-6;
  };
  del1ns=t2ns[busyorig+del1]-t2ns[busyorig];
  printf(" -next measurement with del1,del2 ticks:%d,%d\n", del1,del2);
  IX1: ix=ix+1;
  if(ix>3) {
    vmew32(BUSY_DELAY_ADD, busyorig);
    printf("cannot find right delay\n"); return; 
  };
};
if(adc > 64) {   //north
  if(adc2>adc) { //west Q3
    qu=3;
    dt= (adc- kq[3+4])/kq[3] - del1ns;
    delayed= dt- rdlu[6];
  } else {   //east Q0
    qu=0;
    dt= (adc- kq[0+4])/kq[0] - del1ns;
    delayed= dt- rdlu[6];
  };
} else if(adc < 62) {   //south
  if(adc2>adc) { //east Q1
    qu=1;
    dt= (adc- kq[1+4])/kq[1] - del1ns;
    delayed= dt- rdlu[2];
  } else {   //west Q2
    qu=2;
    dt= (adc- kq[2+4])/kq[2] - del1ns;
    delayed= dt- rdlu[2];
  };
} else {
  printf("findNS internal error\n");
};
// do shift to compare with calibrated values:
if(busyorig==0) t2ns0= delayed;    /* has to be 1st (called from fdf() ) */
delayed= delayed-t2ns0;
if(busyorig==0) {
  dif=0.0;
} else {
  dif=delayed+25-t2ns[busyorig];
};
printf("%d: ADC: %d q:%d: Xns:%6.2f delay:%6.2f +25ns:%6.2f cal:%6.2f d:%6.2f\n", 
    busyorig, adc, qu,dt, delayed, delayed+25, t2ns[busyorig], dif);
}
/*FGROUP L0
*/
void fdf() {
int ix;
for(ix=0; ix<32; ix++) {
  vmew32(BUSY_DELAY_ADD, ix);
  findNS();
};
}
/*---------------------------------------------------------Flash/FPGA */

/*-------------------------------------------------------- all the boards */
/*FGROUP SimpleTests */
void ReadTemperatures() {
int ix,i;
w32 temp2, status;
for(ix=0; ix<NCTPBOARDS; ix++) {
  /* if(ctpboards[ix].vmever==0) continue;    board missing */
  if(notInCrate(ix)) continue;
  vmew32(TEMP_START+BSP*ctpboards[ix].dial, DUMMYVAL);
  for(i=0; i<3; i++) {
    usleep(300);
    status=vmer32(TEMP_STATUS+BSP*ctpboards[ix].dial);
    if( (status & 0x1) == 0) goto TEMPOK;
  };
  printf("%s TEMP_STATUS.BUSY timeout:\n",ctpboards[ix].name );
  continue;
  TEMPOK:
  temp2=vmer32(TEMP_READ+BSP*ctpboards[ix].dial)&0xff;
  printf("%s TEMP_READ:0x%x (%d)\n",
    ctpboards[ix].name, temp2,temp2);
};
}
/*FGROUP SimpleTests */
void TestLEDS() {
int ix;
w32 status;
for(ix=0; ix<NCTPBOARDS; ix++) {
  if(notInCrate(ix)) continue;
  status=vmer32(TEST_ADD+BSP*ctpboards[ix].dial);
  status=~status;
  vmew32(TEST_ADD+BSP*ctpboards[ix].dial, status);
};
}
/*FGROUP SimpleTests 
Wait mics microeconds (using system usleep() call) */
void micsleep(int mics) {
usleep(mics);
}

/*FGROUP Common
   rc: 0 -board ix is in the crate 
       1 -board ix is not in the crate
*/
void initmain() {
/* check/configure all the CTP boards: */
checkCTP();
initSSM();
#ifdef SSMCONNECTIONS 
initNames();
#endif
}
void endmain() {
}
void boardInit() {
int ix;
for(ix=0; ix<6; ix++) {
  FOs[ix].cluster= 0;
  FOs[ix].tcluster= 0;
};
/*printf("Enabling real inputs/outputs ...\n");  */
for(ix=0; ix<NCTPBOARDS; ix++) {
  if(notInCrate(ix)) continue;
  if(ctpboards[ix].code==FOcode) { 
    /*
    vmew32(SSMcommand+BSP*ctpboards[ix].dial, 0);
    vmew32(SSMenable+BSP*ctpboards[ix].dial, 0);
    */
    vmew32(FO_DELAY_L1CLST + BSP*ctpboards[ix].dial, 412);
  };
  if(ctpboards[ix].code==BUSYcode) {
    printf("Configuring BUSY board in 'local orbit mode'...\n"); 
    //vmew32(BUSY_LOCAL_ORBIT, 3563 | 0x1000);   old (0xf6 version)
    vmew32(BUSY_ORBIT_SELECT, 3563 | 0x2000);   
    //vmew32(BUSY_DISB_CTP_BUSY, 1);   /* disable CTPbusy */
    vmew32(BUSY_DISB_CTP_BUSY, 0);   /* enable CTPbusy if INT in crate */
    vmew32(BUSY_CTPDEADTIME, 52);   
    vmew32(BUSY_L0L1DEADTIME, 202);
  };
  if(ctpboards[ix].code==L1code) { 
    vmew32(L1_DELAY_L0,244);
  };
  if(ctpboards[ix].code==L2code) { 
   vmew32(L2_DELAY_L1,3260);
  };
  if(ctpboards[ix].code==INTcode) { 
    /* following should be:
       0: if DAQ involved
       0xb: if DAQ not involved */
    vmew32(INT_DDL_EMU,0x0);      /* DAQ */
    /*vmew32(INT_DDL_EMU,0xb);       no DAQ */
  };
};
}

