/*BOARD ctp 0x820000 0xd000 A24
Contents: */
/*---------------------------------------------------------------- scope  */
/*---------------------------------------------------------------- INT  */
/*- ctplib ------------------------------------------------------- I2C  */
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
#include "ctpcounters.h"
#include "ctplib.h"
#include "vmeblib.h"
#include "shmaccess.h"
#include "ssmctp.h"
#define DBMAIN
#include "../ctp_proxy/Tpartition.h"

extern int quit;     // for rwvmeloop
#ifdef SSMCONNECTIONS 
void initNames();
#endif

void setseeds(long, int);
double rnlx();

Tklas Klas[50];
Tfanout FOs[6];   /* place for 6 fanouts, see getFO(), setFO() */
int ReadTemp(int ix);

/* HIDDEN Common dbghw ConfiguratioH DbgScopeCalls DebCon L012 DbgSSMBROWSERcalls */
/*HIDDEN Common L0 dbghw ConfiguratioH DbgScopeCalls DebCon L012 DebugSSMcalls DbgSSMBROWSERcalls */
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
/*FGROUP INT
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
printf("l2:%x int:%x l2-int:%x(%d) max. micsecs between reads:%d\n", l2or, intor, cd,cd, diff);
}

/*---------------------------------------------------------------- I2C */
/*FGROUP INT
Go through all boards in the CTP crate and print (ltuX, X is ECSnumber+1 from VALID.LTUS):
- I2C values  (4 voltages)
- temperature (1 value in centigrades)
*/
void i2creadall() {
//#define CERNLAB_SETUP
//#define CAVERN_SETUP
int ix, chan,branch, temp; w32 i2crd;
char stemp[8], name[8];
#ifdef CERNLAB_SETUP
printf(" CERNLAB setup:\n");
for(ix=0; ix<NCTPBOARDS; ix++) {
#endif
#ifdef CAVERN_SETUP
printf(" CAVERN setup:\n");
for(ix=0; ix<NCTPBOARDS+24; ix++) {
#endif
  if(ix<NCTPBOARDS) {
    if( notInCrate(ix) ) continue;
    strcpy(name, ctpboards[ix].name);
  } else {
    sprintf(name,"ltu%d", ix-NCTPBOARDS+1);
  }
  if(i2cgetaddr(ix, &chan, &branch)!=0) continue;
  // read temperature
#ifdef CERNLAB_SETUP
  temp= ReadTemp(ix);
  if(temp==1000) {
    strcpy(stemp,"timeout");
  } else {
    sprintf(stemp,"%7d", temp);
  };
#endif
#ifdef CAVERN_SETUP
  if(ix<NCTPBOARDS) {
    temp= ReadTemp(ix);
    if(temp==1000) {
     sprintf(stemp,"timeout");
    } else {
      sprintf(stemp,"%7d", temp);
    };
  } else {
    strcpy(stemp, "-------");
  };
#endif
  // read 4 i2c voltages (in 1 32bit word)
  i2crd= i2cread(chan, branch);
  // print
  printf("%2d %2d %8.8s: %7.7s C. Voltages:", chan, branch,name, stemp);
  vme2volt(i2crd);
};
}
/*---------------------------------------------------------------- BUSY */
/* FGROUP busy
Operation:
- read all the FO_CLUSTER,FO_TESTCLUSTER words (as stored
  in computer mem.) and set correspondingly the BUSY board
*/
void updateBusyClusts() {
int ixfo,ixltu,ltu,iclu;
w32 clusters, overlap;
w32 bsy[7];
char line[100];
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
line[0]='\0';
for(iclu=0; iclu<7; iclu++) {
  //printf("%d: %8x\n",iclu,bsy[iclu]);
  //sprintf(line, "%s%8x ", line, bsy[iclu]);
  vmew32(BUSY_CLUSTER+4*iclu, bsy[iclu]);
};
/* ALWAYS after BUSY_CLUSTER modified, update BUSY_OVERLAP: */
overlap= calcOverlap(bsy); vmew32(BUSY_OVERLAP, overlap);
//printf("%s\n",line);
//printf("BUSY clusters updated on BUSY board according to FO boards\n");
}
/*FGROUP busy
Set daqbsy, T,1,2,3,4,5,6 SET_CLUSTER word on the BUSY board
*/
void setClusters(w32 daqbsy, w32 tc,w32 c1,w32 c2,w32 c3,w32 c4,w32 c5,w32 c6) {
w32 overlap; w32 bsy[7];
int iclu;
vmew32(BUSY_DAQBUSY, daqbsy);
if(daqbsy !=0) {
  vmew32(DAQ_LED, 1);
} else {
  vmew32(DAQ_LED, 0);
};
/*VON vmew32(BUSY_CLUSTER, tc);
vmew32(BUSY_CLUSTER+4, c1);
vmew32(BUSY_CLUSTER+8, c2);
vmew32(BUSY_CLUSTER+12, c3);
vmew32(BUSY_CLUSTER+16, c4);
vmew32(BUSY_CLUSTER+20, c5);
vmew32(BUSY_CLUSTER+24, c6); */
bsy[0]= tc; bsy[1]= c1; bsy[2]= c2; bsy[3]= c3; bsy[4]= c4; 
bsy[5]= c5; bsy[6]= c6; 
for(iclu=0; iclu<7; iclu++) {
  vmew32(BUSY_CLUSTER+4*iclu, bsy[iclu]);
};
/* ALWAYS after BUSY_CLUSTER modified, update BUSY_OVERLAP: */
overlap= calcOverlap(bsy); vmew32(BUSY_OVERLAP, overlap);
}
/*FGROUP busy
Read T,1,2,3,4,5,6 SET_CLUSTER word on the BUSY board and print it as 1 line
*/
void getClusters() {
w32 overlap, overlaphw; int iclu; w32 cls[7];
char line[MAXLINE];
line[0]='\0';
for(iclu=0; iclu<7; iclu++) {
  //printf("%d: %8x\n",iclu,bsy[iclu]);
  cls[iclu]= vmer32(BUSY_CLUSTER+4*iclu);
  sprintf(line,"%s 0x%x", line, cls[iclu]);
}; 
overlap= calcOverlap(cls);
overlaphw= vmer32(BUSY_OVERLAP) &0x1fffff;
printf("%s\n", line);
//printf("%s\nBUSY_OVERLAPhw:0x%x expected:0x%x", line, overlaphw, overlap);
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
cluster: 0x44332211      11 - bits [5..0]: clusters fed through 1st connector
tcluster: 0xXCT4321
X   : bits[31..28]: toggle signal on connector 4..1
C   : bit[20]       calibration flag for sw trigger
T   : bits[19..16]: sw trigger (test class) for connector 4..1
4   : bits[15..12]: RoC for connector 4
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
//done separately in ctpcfg.py updateBusyClusts();
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
w32 l0invAC; int minAC;
if(l0AB()==0) {l0invAC=L0_INVERTac; minAC=0; } else { l0invAC=L0_INVERT; minAC=44; };
klasix= klas-1; bb= klas*4; 
if(notInCrate(1)) {
  Klas[klasix].regs[0]= 0;
  Klas[klasix].regs[1]= 0;
  Klas[klasix].regs[2]= 0;
  Klas[klasix].regs[3]= 0;
} else {
  /* not readable in A0 version, now readable: */
  Klas[klasix].regs[0]= vmer32(L0_CONDITION+bb);
  if(klas>minAC) {  /* only for inverted klasses ! */
    Klas[klasix].regs[1]= vmer32(l0invAC+bb);
  }else {
    Klas[klasix].regs[1]= 0;
  };
  mskbit1= vmer32(L0_MASK+bb)&0x1;
  if(l0AB()==0) {   //firmAC
    mskbit2= (vmer32(L0_VETO+bb)&0x1fffff) | (mskbit1<<31);
  } else {
    mskbit2= (vmer32(L0_VETO+bb)&0xffff) | (mskbit1<<16);
  };
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
i.e. 7 hexa numbers.
L0_INVERT, L1_INVERT are 0:for classes 1-44 (not valid for  >AC) or 
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
   bit31 for firmAC
2. invert,l1invert -valid only for class>=45
*/
void setClass(int klas,w32 condition, w32 invert, w32 veto, w32 scaler,
              w32 l1def, w32 l1invert, w32 l2def) {
int bb,klasix; w32 mskbit;
w32 l0invAC; int minAC;
if(l0AB()==0) {l0invAC=L0_INVERTac; minAC=0; } else { l0invAC=L0_INVERT; minAC=44; };
bb= klas*4; klasix=klas-1;
if(notInCrate(1)==0) {   // L0 board
  Klas[klasix].regs[0]= condition; vmew32(L0_CONDITION+bb, condition);
  if(klas>minAC) {  /* only for inverted klasses ! */
    Klas[klasix].regs[1]= invert; vmew32(l0invAC+bb, invert);
  };
  if(l0AB()==0) { //firmAC
    Klas[klasix].regs[2]= veto; vmew32(L0_VETO+bb, veto&0x1fffff); 
    mskbit= veto>>31; vmew32(L0_MASK+bb, mskbit);
  } else {
    Klas[klasix].regs[2]= veto; vmew32(L0_VETO+bb, veto&0xffff); 
    /* 1st L0 version (A0): vmew32(L0_MASK+bb, veto&0x10000); */
    mskbit= (veto&0x10000)>>16; vmew32(L0_MASK+bb, mskbit);
  };
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
L0_VETO      = 0xfffffff0   (cluster0) bit31:1-> class is disabled
and 0x0 in:
L0_INVERT   =0
L0_PRESCALER=0
*/
void disableClasses() {
int klas;
if(notInCrate(1)) return;
for(klas=1; klas<=50; klas++) {
  setClass(klas, 0xffffffff, 0, 0xfffffff0, 0, 0x0fffffff, 0,0x0f000fff);
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
  Klas[ix].regs[3]= vmer32(RATE_DATA) & RATE_MASK;
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
  vmew32(RATE_DATA, Klas[ix].regs[3] & RATE_MASK);
};
vmew32(RATE_MODE,0);   /* normal mode */
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
  rnd= RNLXMASK* rounddown(rnlx()); rnd= RNLXMASK&rnd;
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
/*---------------------------------------------------------------- L0-Shared */
/*FGROUP SimpleTests
To be used with CTP_hlttest (instance of PHYSICS_1 in ACT)
and mask defining 12 B-bunches. CTP_hlttest:
ALL cluster:
DEMPTY(cn=CINT1-B-NOPF-ALLNOTRD,rnd1,BCM2,cg=8)
D1DUM(cn=CINT1-AC-NOPF-ALLNOTRD,rnd2,BCM2,cg=8)
+ there is cluster ALL with TRD in readout driven by similar WU-classes

rate: required rate in hz to be set for L2a/r classes
operation:
1. calculate rate for rnd1/2 generators and set in in hw:
rnd2=(required_input_rate1*3564/12.) hz
rnd1= rnd2+1 to get different rnd1/2 setting
*/
void setRates4HLTtest(int rate) {
w32 r1,r2; float r1f,r2f;
r2f= rate*3564/12.; r2= (w32)(r2f*0x7fffffff/40./1000000);
r1f= r2f+1; r1= (w32)(r1f*0x7fffffff/40./1000000.);
vmew32(RANDOM_1, r1);
vmew32(RANDOM_2, r2);
printf("rate:%dhz: rnd1:%6.2fhz =%d   rnd2:%6.2fhz =%d was set in CTP\n",\
  rate, r1f, r1, r2f, r2);
}
/*---------------------------------------------------------------- L0-tests */
/*FGROUP inputsTools
set positive + delay on both 1EJE and 1EGA
*/
void setEJEGA(int delay) {
setEdgeDelay(2, 1, 0, delay);
setEdgeDelay(2, 2, 0, delay);
printf("1EJE,1EGA delay set to %d\n", delay);
}

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
int bcdelay;
for(bcdelay=0; bcdelay<steps; bcdelay++) {
  vmew32(BUSY_DELAY_ADD, (w32)bcdelay);
/*  vmew32(PLL_RESET, DUMMYVAL);  -doesn't work with pll_reset*/
  usleep(1000000);
};
}
/*FGROUP busy
Operation:
-Set BC_DELAY from 0..31 with step 1, 
-read BUSY_ORBIT_SELECT word (follow bit14: 0x4000 0:negBC 1:posBC)
-wait 300 usecs between
Input:
steps: number of steps 
Output: Table: delay Edgeflag
*/
void orbitscan(int steps) {
w32 bcdelay;
for(bcdelay=0; bcdelay<(w32)steps; bcdelay++) {
  w32 bos;
  vmew32(BUSY_DELAY_ADD, bcdelay);
/*  vmew32(PLL_RESET, DUMMYVAL);  -doesn't work with pll_reset*/
  usleep(300);
  bos= vmer32(BUSY_ORBIT_SELECT);
  printf("%d %d\n", bcdelay, (bos>>14)&1);
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
/*FGROUP SimpleTests 
VME read/write in the loop (for l1glitch testing)
Inputs:
address: vme address or
         0: read counters on all boards readCounters() (no I2C)
         1: readTVCounters() -I2C only
         2: do not read vme
loops:   number of loops (0: endless loop)
value:   0: read 1: write alternating 0/0xffffffff >1: write this value
mics:  sleep time in micsecs between reads
ring:    32 do not ring, 0..31 -altrenate this bit in address 
*/
void rwvmeloop(w32 address, int loops, w32 value, int mics, int ring) {
w32 data; int forever=0; int prt=0; w32 vadd; int ringbit;
//w32 altern=0xffffffff;
w32 altern=0xaaaaaaaa;
w32 ctpc[NCOUNTERS];
vadd= address; ringbit= 1<<ring;
if(loops==0) forever=1;
while(1) {
  if(address<=2) {
    w32 l2orbit;   // let's do the same VME operation as in ctdims
    if(prt==0) {printf("readCounters(buf, %d\n", NCOUNTERS); prt=1; };
    l2orbit= vmer32(L2_ORBIT_READ); 
    readCounters(ctpc, NCOUNTERS, 0, 2);
  } else if(address==1) {
    readTVCounters(&ctpc[CSTART_SPEC+3]);
  } else if(address==2) {
    ;
  } else {
    if(ring<32) {
      if(vadd & ringbit) {
        vadd= vadd & (~ringbit);
      } else {
        vadd= vadd | ringbit;
      };
    };
    if(value==0) {   // read
      if(prt==0) {printf("vme read(0x%x) ring:%d...\n",
        address, ring); prt=1; };
      data= vmer32(vadd);
    } else {
      if(prt==0) {printf("vme write(0x%x, %x) ring:%d...\n",
        address, value, ring); prt=1; };
      if(value==1) {
        vmew32(vadd, altern); altern= ~altern;
        //printf("%x\n",altern);
      } else {
        vmew32(vadd, value);
      };
    };
  };
  if(forever==1) {
   if(quit !=0) {
     printf("SIGUSR1 received, finishing loop...\n");
     break;
   } else {
     goto CONT;
   };
  };
  loops--; if(loops==0) break;
  CONT: if(mics>0) usleep(mics);
};
}
int defcounts[]={NCOUNTERS_BUSY, NCOUNTERS_L0, NCOUNTERS_L1, NCOUNTERS_L2,
  NCOUNTERS_INT, NCOUNTERS_FO, NCOUNTERS_FO, NCOUNTERS_FO, 
  NCOUNTERS_FO, NCOUNTERS_FO, NCOUNTERS_FO};

/*FGROUP SimpleTests 
read+print N counters of the board 
board (0:busy, 1:L0 2:L1, 3:L2, 4:INT, 5:FO1...)
N==0: read+print all counters (according to ctpcounters.h) of the board
*/
void printBoardCounters(int board, int N) {
int cix,counts;
w32 mem[NCOUNTERS_MAX];
if(N==0) {counts= defcounts[board];
} else { counts= N; };
printf("reading %d counters from board %d:\n", counts,board);
getCountersBoard(board, counts-1, mem, 2);
//0:proxy 1:dims 2:ctp+busytool 3:smaq 4:inputs
for(cix=0; cix<counts; cix++) {
  if(cix%5==0) {
    if(cix>0) printf("\n");
    printf("%3d:",cix);
  } else {printf(" ");};
  printf("0x%x",mem[cix]);
}; printf("\n");
}
/*FGROUP SimpleTests 
Dump choosen counters to file $VMEWORKDIR/counters.dump
*/
void dumpCounters() {
FILE *f; char fpath[100];
char fname[]="counters.dump";
int ix,cnts[]={455,461,456,462,477,481,485,153,302,448,154,303,449,
-1};
w32 mem[CSTART_SPEC];
readCounters(mem, CSTART_SPEC, 0, 2);
sprintf(fpath,"WORK/%s", fname);
f= fopen(fpath,"w");
if(f==NULL) {
  printf("%s open error \n",fpath);
  return;
};
for(ix=0; ix<CSTART_SPEC; ix++) {
  w32 val;
  if( cnts[ix]==-1 ) break;
  val= mem[cnts[ix]];
  fprintf(f,"%d %x\n", val, val);
};
fclose(f);
printf("%s written\n", fpath);
}

/*---------------------------------------------------------Flash/FPGA */

/*FGROUP SimpleTests */
void ReadTemperatures() {
int ix;
int temp2;
for(ix=0; ix<NCTPBOARDS; ix++) {
  /* if(ctpboards[ix].vmever==0) continue;    board missing */
  if(notInCrate(ix)) continue;
  //printf("ReadTemperatures:ix in the crate:%d\n",ix);
  temp2= ReadTemp(ix);
  if(temp2==1000) {
    printf("%s TEMP_STATUS.BUSY timeout:\n",ctpboards[ix].name );
    continue;
  };
  printf("%s TEMP_READ:0x%x (%d)\n", ctpboards[ix].name, temp2,temp2);
};
}
/*FGROUP SimpleTests */
void TestLEDS() {
int ix;
w32 status, va;
for(ix=0; ix<NCTPBOARDS; ix++) {
  if(notInCrate(ix)) continue;
  va= TEST_ADD+BSP*ctpboards[ix].dial;
  if(ix==0) va= va + 0x400;    // busy is special (1nn)
  status=vmer32(va); status=~status; vmew32(va, status);
};
}
/*FGROUP SimpleTests 
Wait mics microeconds (using system usleep() call) */
void micsleep(int mics) {
usleep(mics);
}
/*FGROUP SimpleTests 
Synchronise/desynchronise random generators
mask:
3 synchronise RND1 and RND2 generators on L0 board
1 desynchronise RND1 and RND2 generators on L0 board
*/
void RNDsync(int mask);

/*FGROUP SimpleTests
TL2 play. Normal value: 3952
Corresponding L2_DEALY_L1, FO_DELAY_L1CLST, L2_BCOFFSET:
3628 456 383
*/
void changeTL2(int tl2) {
w32 l2l1,fo,l2off; int ifo, tl2get;
setTimeParsDB(260, tl2, 0, 3011, 3564);
l2l1= calcL2_DELAY_L1();
fo= calcFO_DELAY_L1CLST();
l2off= calcL2_BCOFFSET();
tl2get= getTL2();
vmew32(L2_DELAY_L1, l2l1); 
ifo=5; vmew32(FO_DELAY_L1CLST+BSP*ctpboards[ifo].dial,fo);
vmew32(L2_BCOFFSET, l2off);
printf("updated TL2 L2_DELAY_L1 FO_DELAY_L1CLST L2_BCOFFSET:%d: %d %d %d\n",
  tl2get, l2l1, fo, l2off);
}

/*FGROUP Common
   rc: 0 -board ix is in the crate 
       1 -board ix is not in the crate
*/
void initmain() {
int ix;
/*ctpshmbase= (Tctpshm *)mallocShared(CTPSHMKEY, sizeof(Tctpshm), &ctpsegid);
ctpshmbase= (Tctpshm *)malloc(sizeof(Tctpshm));
validCTPINPUTs= &ctpshmbase->validCTPINPUTs[0];
validLTUs= &ctpshmbase->validLTUs[0];
*/
cshmInit(); ix= initHW(&HW);  // HW: partialy used in loadcheckctpcfg
for(ix=0; ix<6; ix++) { 
  FOs[ix].cluster= 0;
  FOs[ix].tcluster= 0;
};
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
/*int ix;
for(ix=0; ix<6; ix++) {    moved to initmain (2.2.2007)
  FOs[ix].cluster= 0;
  FOs[ix].tcluster= 0;
};*/
/* do not init in ctp expert software becasue:
1. it should be initialised by starting ctp_proxy
2. initCTP calls loadcheckctpcfg which call popenread which
   start python -> i.e. ctp.exe when started through cmdlin2
   should be alwasy started wih noboardInit option!
3. ctp.exe is called from command line interface:
   alidcsvme001:/home/.custom.rc (when booted)
*/
initCTP();
/*printf("Enabling real inputs/outputs ...\n");  */
}

