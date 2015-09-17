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
/*---------------------------------------------------------------- LM0-tests */
/*---------------------------------------------------------------- L0-tests */
/*---------------------------------------------------------------- busy-tests */
/*---------------------------------------------------------------- ADC-tests */
/*---------------------------------------------------------------- DDR3 */
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
/*von #ifdef SSMCONNECTIONS 
void initNames();
#endif*/

void setseeds(long, int);
double rnlx();

Tklas Klas[NCLASS];
Tfanout FOs[6];   /* place for 6 fanouts, see getFO(), setFO() */
int ReadTemp(int ix);

/* HIDDEN Common dbghw ConfiguratioH DbgScopeCalls DebCon L012 DbgSSMBROWSERcalls */
/*HIDDEN Common L0 dbghw ConfiguratioH DbgScopeCalls DebCon DebugSSMcalls DbgSSMBROWSERcalls */
/* HIDDEN Common L0 dbghw ConfiguratioH DbgScopeCalls DebCon DebugSSMcalls */
/*FGROUP TOP GUI CTP_Classes "Classes"
The Classes definition, i.e. for each (1-NCLASS) class: 
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

/*FGROUP LM0 */
void initCTP();
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
w32 overlap,adr; w32 bsy[7];
int iclu;
vmew32(BUSY_DAQBUSY, daqbsy);
if(l0C0()) {
  adr= DAQ_LEDlm0;
} else {
  adr= DAQ_LED;
};
if(daqbsy !=0) {
  vmew32(adr, 1);
} else {
  vmew32(adr, 0);
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
  setsmssw(1,(char *)"l0_outmon");
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
w32 status,st,l0_tcset;
if(l0C0()) { l0_tcset= L0_TCSETr2; } else { l0_tcset= L0_TCSET; };
status= vmer32(l0_tcset)&0x7ffff;
st= (vmer32(L1_TCSET)&0x40000)<<1; status= status | st;
st= (vmer32(L2_TCSET)&0x1000000)>>4; status= status | st;
return(status);
}
/*FGROUP L0
tcset012 -extended mening (bits 18,19 -see getTCSET() )
*/
void setTCSET(w32 tcset012, w32 dets) {
w32 tcs2,l0_tcset;
if(l0C0()) { l0_tcset= L0_TCSETr2; } else { l0_tcset= L0_TCSET; };
vmew32(l0_tcset, tcset012&0x7ffff);
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
//if(l0AB()==0) {l0invAC=L0_INVERTac; minAC=0; } else { l0invAC=L0_INVERT; minAC=44; };
l0invAC=L0_INVERTac; minAC=0;
klasix= klas-1; bb= klas*4; 
if(notInCrate(1)) {
  Klas[klasix].regs[0]= 0;
  Klas[klasix].regs[1]= 0;
  Klas[klasix].regs[2]= 0;
  Klas[klasix].regs[3]= 0;
  Klas[klasix].regs[7]= 0; Klas[klasix].regs[8]= 0; Klas[klasix].regs[9]= 0; Klas[klasix].regs[10]= 0;
} else {
  /* not readable in A0 version, now readable: */
  Klas[klasix].regs[0]= vmer32(L0_CONDITION+bb);
  if(klas>minAC) {  /* only for inverted klasses ! */
    Klas[klasix].regs[1]= vmer32(l0invAC+bb);
  }else {
    Klas[klasix].regs[1]= 0;
  };
  if(l0C0()==0) {
    mskbit1= vmer32(L0_MASK+bb)&0x1;
    if(l0AB()==0) {   //firmAC
      mskbit2= (vmer32(L0_VETO+bb)&0x1fffff) | (mskbit1<<31);
    } else {
      mskbit2= (vmer32(L0_VETO+bb)&0xffff) | (mskbit1<<16);
    };
  } else {
    mskbit2= vmer32(L0_VETOr2+bb);
  };
  Klas[klasix].regs[2]= mskbit2;
  /* scalers are updated in 1 pass (see hw2rates)
  Klas[klasix].regs[3,10]= 0xabeceda;  */
  Klas[klasix].regs[7]= vmer32(LM_CONDITION+bb);
  Klas[klasix].regs[8]= vmer32(LM_INVERT+bb);
  Klas[klasix].regs[9]= vmer32(LM_VETO+bb);
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
klas: 1-NCLASS   -class number
get L0_CONDITION L0_INVERT L0_VETO L0_PRESCALER 
    L1_DEFINITION L1_INVERT L2_DEFINITION
    LM_CONDITION LM_INVERT LM_VETO LM_RATE_DATA
i.e. 7+4 hexa numbers.
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
words for klas (1..NCLASS)
ATTENTION: 
1. bit17 (0x10000) of veto is CLASS MASK bit written into bit0 of L0_MASK
   bit31 for firmAC
   LM0: bit23 (as in hw)
2. invert,l1invert -valid only for class>=45
*/
void setClass(int klas,w32 condition, w32 invert, w32 veto, w32 scaler,
              w32 l1def, w32 l1invert, w32 l2def) {
int bb,klasix; w32 mskbit;
w32 l0invAC; int minAC;
l0invAC=L0_INVERTac; minAC=0;
bb= klas*4; klasix=klas-1;
if(notInCrate(1)==0) {   // L0 board
  Klas[klasix].regs[0]= condition; vmew32(L0_CONDITION+bb, condition);
  if(klas>minAC) {  /* only for inverted klasses ! */
    Klas[klasix].regs[1]= invert; vmew32(l0invAC+bb, invert);
  };
  if(l0AB()==0) { //firmAC
    if(l0C0()) {
      Klas[klasix].regs[2]= veto; vmew32(L0_VETOr2+bb, veto);
    } else {
      Klas[klasix].regs[2]= veto; vmew32(L0_VETO+bb, veto&0x1fffff); 
      mskbit= veto>>31; vmew32(L0_MASK+bb, mskbit);
    };
  } else {
    Klas[klasix].regs[2]= veto; vmew32(L0_VETO+bb, veto&0xffff); 
    /* 1st L0 version (A0): vmew32(L0_MASK+bb, veto&0x10000); */
    mskbit= (veto&0x10000)>>16; vmew32(L0_MASK+bb, mskbit);
  };
  Klas[klasix].regs[3]= scaler; /* update is done in 1 pass in rates2hw */
};
if(notInCrate(2)==0) {   // L1 board
  Klas[klasix].regs[4]= l1def; vmew32(L1_DEFINITION+bb, l1def);
  if(klas>=1) {  /* was 45 till 13.9.2014 (bug) */
    Klas[klasix].regs[5]= l1invert; vmew32(L1_INVERT+bb, l1invert);
  };
};
if(notInCrate(3)==0) {
  Klas[klasix].regs[6]= l2def; vmew32(L2_DEFINITION+bb, l2def);
};
}

/*FGROUP L0
set LM_CONDITION LM_INVERT LM_VETO
*/
void setClassLM(int klas,w32 lmcondition, w32 lminvert, w32 lmveto, w32 lmscaler) {
int bb,klasix;
bb= klas*4; klasix=klas-1;
if(notInCrate(1)==0) {   // L0 board
  Klas[klasix].regs[7]= lmcondition;
  Klas[klasix].regs[8]= lminvert; 
  Klas[klasix].regs[9]= lmveto; 
  Klas[klasix].regs[10]= lmscaler; // update in 1 pass in rates2hw
};
setClassInitLM(klas, lmcondition, lminvert, lmveto);
}
/*FGROUP L0
disable all NCLASS classes, i.e.:
- set all inputs,vetos as dontcare for all NCLASS classes i.e.:
LM+L0_CONDITION = 0xffffffff
L0_VETO      = 0xfffffff0 (cluster0) bit31:1-> class is disabled
L0_VETOr2    = 0xXX9ffff0 (cluster0) DSCG: XX (low 7 bits)
LM_VETO      = 0xXX803f00 DSCG: XX, class disabled,PF1-4,ALll/rare,LMdead
and 0x0 in:
LM+L0_INVERT   =0
LM+L0_PRESCALER=0

*/
void disableClasses() {
int klas; w32 l0veto=0xfffffff0;
if(notInCrate(1)) return;
if(l0C0()) l0veto=0x009ffff0;
for(klas=1; klas<=NCLASS; klas++) {
  setClass(klas, 0xffffffff, 0, l0veto | ((klas-1)<<24), 0, 0x0fffffff, 0,0x0f000fff);
  setClassLM(klas,  0xffffffff, 0, 0x803f00 | ((klas-1)<<24), 0);
};
}

/*FGROUP L0
read all rates (scalers) and seeds from hw to Klas structure
*/
void hw2rates() {
int ix;
w32 rate_mask;
if(l0C0()) {
  rate_mask= RATE_MASKr2;
} else {
  rate_mask= RATE_MASK;
};
vmew32(LM_RATE_CLEARADD,DUMMYVAL);
for(ix=0; ix<NCLASS; ix++) {
  Klas[ix].regs[3]= vmer32(LM_RATE_RND_OFFSET) & rate_mask;
};
vmew32(LM_RATE_CLEARADD,DUMMYVAL);
for(ix=0; ix<NCLASS; ix++) {
  Klas[ix].regs[10]= vmer32(LM_RATE_DATA) & RATE_MASKr2;
};
}
/*FGROUP L0
write rates and seeds from Klas structure to hw
*/
void rates2hw() {
int ix;
w32 rate_mask;
if(l0C0()) {
  rate_mask= RATE_MASKr2;
} else {
  rate_mask= RATE_MASK;
};
vmew32(LM_RATE_CLEARADD,DUMMYVAL);
for(ix=0; ix<NCLASS; ix++) {
  vmew32(LM_RATE_RND_OFFSET, Klas[ix].regs[3]);
};
// reset
vmew32(LM_RATE_CLEARADD,DUMMYVAL);
for(ix=0; ix<NCLASS; ix++) {
  vmew32(LM_RATE_DATA, Klas[ix].regs[10] & RATE_MASKr2);
};
vmew32(LM_RATE_RND_RESET,DUMMYVAL);
}

/*FGROUP SimpleTests
*/
void printBC_STATUSes() {
int ix;
printf("   board code ser# base     vmeV boardV BCstatus\n");
for(ix=0; ix<NCTPBOARDS; ix++) {
  w32 bcst; int adshift;
  char errnote[80]="";
  if(notInCrate(ix)) continue;
  adshift=BSP*ctpboards[ix].dial;
  bcst= vmer32(adshift+BC_STATUS)&0x7;
  printf("%2d:%5s 0x%x %4d 0x82%1x000 0x%x 0x%x   %x     %s\n",
    ix, ctpboards[ix].name,ctpboards[ix].code, ctpboards[ix].serial, 
    ctpboards[ix].dial, ctpboards[ix].vmever, ctpboards[ix].boardver,bcst,
    errnote);
};
}
/*FGROUP SimpleTests
what: 0: set LM_RATE_DATA  (100 words, 21 bits)
      1: set MASK_DATA  (3564 words, 12 bits)
      2: set LM_RATE_RND_OFFSET  (100 words, 21 bits)
value: to be written
Notes:
RATE_DATA: rnd: 21 bits     busy: 0x2000000 | 25bits (1 step: 10us)
MASK_DATA: 0xfff -disbable all bits
write 1.. 100/3564 words
*/
void setrates(int what, w32 value) {
int ix;
w32 rate_mask,vmemode,clearad,datad;
int MAXIX;
if(what==0) {              // RATE_DATA
  vmemode=0;
  rate_mask=0x1fffff;
  MAXIX=NCLASS;
  clearad= LM_RATE_CLEARADD;
  datad= LM_RATE_DATA;
} else if(what==1) {              // MASK_DATA
  if(l0C0()) {
    vmemode= MASK_MODEr2;
  } else {
    vmemode= MASK_MODE;
  };
  rate_mask= 0xfff;
  MAXIX=ORBITLENGTH;
  clearad= MASK_CLEARADD;
  datad= MASK_DATA;
} else if(what==2){                   // LM_RATE_DATA
  //vmemode= LM_RATE_MODE;
  vmemode=0;
  rate_mask= 0x1fffff;
  MAXIX=NCLASS;
  clearad= LM_RATE_CLEARADD;
  datad= LM_RATE_RND_OFFSET;
} else{
  printf("Select what \n");
  return ;
};
if(vmemode)vmew32(vmemode,1);   /* vme mode */
if(what<10) {
  vmew32(clearad,DUMMYVAL);
  printf("writing 0x%x 1..%d ...\n",value, MAXIX);
  for(ix=0; ix<MAXIX; ix++) {
    vmew32(datad, value);
  };
};
if(vmemode)vmew32(vmemode,0);   /* normal mode */
}
/*FGROUP SimpleTests
what: 0: test LM_RATE_DATA  (100 words, 25 bits)
      1: test MASK_DATA  (3564 words, 12 bits)
      2: test LM_RATE_RND_OFFSET
     10: just read LM_RATE_DATA
     11: just read MASK_DATA
     12: just read LM_RATE_RND_OFFSET
write 1.. 100/3564,read back and print if not as expected
*/
void testrates(int what) {
int ix;
w32 rate_mask,vmemode,clearad,datad;
int MAXIX, okn;
if((what%2)==0) {              // LM_RATE_DATA
  vmemode=0;
  if(l0C0()) {
    rate_mask= RATE_MASKr2;
  } else {
    rate_mask= RATE_MASK;
  };
  MAXIX=NCLASS;
  clearad= LM_RATE_CLEARADD;
  if((what%10)==0)datad= LM_RATE_DATA; else datad= LM_RATE_RND_OFFSET;
} else {                   // MASK_DATA
  if(l0C0()) {
    vmemode= MASK_MODEr2;
  } else {
    vmemode= MASK_MODE;
  };
  rate_mask= 0xfff;
  MAXIX=ORBITLENGTH;
  clearad= MASK_CLEARADD;
  datad= MASK_DATA;
};
if(vmemode)vmew32(vmemode,1);   /* vme mode */
if(what<10) {
  vmew32(clearad,DUMMYVAL);
  printf("writing 1..%d ...\n",MAXIX);
  for(ix=0; ix<MAXIX; ix++) {
    vmew32(datad, (ix+1) & rate_mask);
  };
};
//read back
if(what<10)printf("reading..., printing out (errors only)...\n");
vmew32(clearad,DUMMYVAL); okn=0;
for(ix=0; ix<MAXIX; ix++) {
  w32 da;
  da= vmer32(datad) & rate_mask;
  if(what<10) {
    if (da!= ((ix+1) & rate_mask)) {
      printf("%2d: %d expected:%d=0x%x\n", ix+1, da, ((ix+1) & rate_mask),
        ((ix+1) & rate_mask));
    } else {
      okn++;
    };
  } else {
      printf("%2d: %d=0x%x\n", ix+1, da, da);
  };
}; 
if(vmemode)vmew32(vmemode,0);   /* normal mode */
if(what==10) {
  //vmew32(lmvmemode,0);   /* normal mode */
};
if(what<10) {
  printf("tested words:%d, ok: %d words\n", MAXIX, okn);
};
}

#define RNLXMASK 0xffffffff
/*FGROUP L0
check spy memory (256 words from 0x9400 on L0 board )
Operation: 
- write a, a+1 a+2 ... 
- read back and check
- write 0s to all the 256 words
*/
void clearSPY(int board) {
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
vmew32((RANDOM_1), r1);
vmew32((RANDOM_2), r2);
printf("rate:%dhz: rnd1:%6.2fhz =%d   rnd2:%6.2fhz =%d was set in CTP\n",\
  rate, r1f, r1, r2f, r2);
}
/*---------------------------------------------------------------- LM0-tests */
/*FGROUP LM0 
Input: lut: 1..8 corresponding to L0F1..4 LMF1..4
            0: load all 8 LUTs with the same LUT
expr: a&c|b|~h   -i.e. using letters a b c d e f g h for first 8 L0, resp. LM inputs
      0          - put 0s
      1          - put 1s
Out:
rc: 0 ok
*/
int loadLUT8(int lut, char *expr) {
int rc; char cmd[100];
#define LUTMAXLEN 300   // longer (can return error message)
char lookupt[LUTMAXLEN];
if( (strcmp(expr,"0")!=0) &&  (strcmp(expr,"1")!=0) ) {
  //sprintf(cmd, "python $VMEBDIR/trigdb.py log2tab '%s'", expr);
  sprintf(cmd, "python $VMEBDIR/txtproc.py '%s'", expr);
  lookupt[0]= '\0'; rc= popenread(cmd, lookupt, LUTMAXLEN);
  if((strncmp(lookupt,"0x", 2)!=0) || (rc != EXIT_SUCCESS) ) {
    printf("ERROR in %s definition: popenread rc:%d\n",expr, rc);
    printf("%s\n", lookupt);
    return(8);
  };
} else {
  strcpy(lookupt, expr);
};
if(lookupt[strlen(lookupt)-1]=='\n') {
  lookupt[strlen(lookupt)]='\0';
};
if(strlen(lookupt)==67) {
  lookupt[strlen(lookupt)-1]='\0';
};
rc= setLUT(lut, lookupt);
printf("LUT %d loaded with %s rc:%d\n",lut, lookupt, rc);
return(rc);
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
   if(l0C0()) {
    w32 word;
    sadr= bb+SYNCH_ADDr2+4*ix1;
    word= vmer32(sadr);
    // keep edge, do it for first 24 only
    word= (word & 0xfffffff0) | sync;
    vmew32(sadr, word);
   } else {
    sadr= bb+SYNCH_ADD+4*ix1;
    vmew32(sadr, sync);
   };
  };
/*  rc= startSSM1(brd); */
  usleep(1000000);
};
}

w32 getSDSGr2(int cls) {
w32 adr, adrlm; //, rc;
adr= L0_VETOr2+4*cls;
adrlm= LM_VETO+4*cls;
// rc: 0xMM00   MM: LM-SDSCG   00:L0-SDSCG
return( ((vmer32(adrlm)>>16)&0x7f00) | ((vmer32(adr)>>24)&0x7f ));
}
void putSDSGr2(int cls, int grp) {
w32 adr, adrlm;
adr= L0_VETOr2+4*cls;
adrlm= LM_VETO+4*cls;
vmew32(adr, (vmer32(adr)&0x80ffffff) | (grp<<24)) ;
vmew32(adr, (vmer32(adrlm)&0x80ffffff) | (grp<<24)) ;
return;
}
/*FGROUP SimpleTests
clas:
0:         print L0_SDSCG+4,8,...  ('group' par. has no sense in this case,
           but some number has to be provided). Format: class:SDSCG
           In case L0_SDSCG != LM_SDSCG, both groups are printed in format
           class:LM_SDSCG:L0_SDSCG           
1,2,3,...: class number of class to be set in group 'group'
951: set all classes to 'group' (LM+L0)
952: set all classes to init state (no sync downscaling), i.e. 0,1,2,...,49,0,0...
group: Set class' group to group (meaningfull only for classes 1..100)
       Should be : 1,2,3,...,50
LM0: set in 2 places: L0_VETOSr2[30..24] + LM_VETO[30..24]
*/
void printsetSDSCG(int clas, int group) {
if(clas==0) {
  int ixc;
  if(l0C0()) {
    for(ixc=1; ixc<=NCLASS; ixc++) {
      w32 val, lmval;
      val= getSDSGr2(ixc); lmval= (val&0x7f00)>>8;  val= val&0x7f; 
      if(lmval==val) {
        printf("%2d:%2d ", ixc, val);
      } else {
        printf("%2d:%2d:%2d ", ixc, lmval, val);
      };
      if((ixc%10)==0) printf("\n");
    };
  } else {
    for(ixc=1; ixc<=NCLASS; ixc++) {
      w32 adr,val;
      adr= L0_SDSCG + ixc*4; val= vmer32(adr);
      printf("%2d:%2d ", ixc, val);
      if((ixc%10)==0) printf("\n");
    };
  }
} else if(clas==951) {
  printf("setting SDSCG for all classes to %d...\n",group);
  int ixc;
  if(l0C0()) {
    for(ixc=1; ixc<=NCLASS; ixc++) {
      putSDSGr2(ixc, group);
    };
  } else {
    for(ixc=1; ixc<=NCLASS; ixc++) {
      w32 adr;
      adr= L0_SDSCG + ixc*4; vmew32(adr, group);
    };
  };
} else if(clas==952) {
  printf("setting SDSCG for all classes to default: 0,1,...49, 0,0,\n");
  int ixc;
  if(l0C0()) {
    for(ixc=1; ixc<=NCLASS; ixc++) {
      if(ixc>=51) {
        putSDSGr2(ixc, ixc-1);
      } else {
        putSDSGr2(ixc, 0);
      };
    };
  } else {
    for(ixc=1; ixc<=NCLASS; ixc++) {
      w32 adr;
      adr= L0_SDSCG + ixc*4; vmew32(adr, ixc-1);
    };
  };
} else if(clas>NCLASS) {
  printf("clas: 0..100 allowed\n");
} else {
  if((group<0) || (group>49)) {
    printf("group: 0..49 allowed (0: allowed but should not be used)\n");
  } else {
    w32 adr;
    if(group==0) {
      printf("Warning: group 0 allowed but should not be used!\n");
    };
    if(l0C0()) {
      putSDSGr2(clas, group);
    } else {
      adr= L0_SDSCG + clas*4; vmew32(adr, group);
    };
  };
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
w32 data; int forever=0; int prt=0; w32 vadd; int ringbit, loopsdone=0;
//w32 altern=0xffffffff;
w32 altern=0xaaaaaaaa;
w32 ctpc[NCOUNTERS];
vadd= address; ringbit= 1<<ring;
if(loops==0) forever=1;
printf("Break loop: kilme %d from cmdline, starting vme loop...\n",getpid());
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
  }; loopsdone++;
  if(quit !=0) {
    printf("SIGUSR1 received, finishing loop...\n"); quit=0;
    break;
  };
  if(forever==0) {
    loops--; if(loops==0) break;
  };
  if(mics>0) usleep(mics);
};
printf("vme loops:%d\n", loopsdone);
}
int defcounts[]={NCOUNTERS_BUSY, NCOUNTERS_L0+NCOUNT200_L0, 
  NCOUNTERS_L1, NCOUNTERS_L2,
  NCOUNTERS_INT, NCOUNTERS_FO, NCOUNTERS_FO, NCOUNTERS_FO, 
  NCOUNTERS_FO, NCOUNTERS_FO, NCOUNTERS_FO};

/*FGROUP SimpleTests 
*/
void clearAllCounters() {
clearCounters(2);
}
/*FGROUP SimpleTests 
read+print N counters of the board from FROM counter (counting from 0). 
board (0:busy, 1:L0 2:L1, 3:L2, 4:INT, 5:FO1...)
N==0: read+print all counters (according to ctpcounters.h) of the board
*/
void printBoardCounters(int board, int FROM, int N) {
int cix,counts,cixmod=0; w32 usecs, s1,s2,us1,us2;
w32 mem[NCOUNTERS_MAX];
if(N==0) {
  counts= defcounts[board];
} else if((FROM+N)>defcounts[board] ) {
  counts= defcounts[board];
} else { 
  counts= FROM+N; 
};   // counts: rel. adres of counters which should not be read
if(FROM > counts) FROM=0;
printf("reading counters %d - %d from board %d", FROM, counts-1,board);
// always starting from first counter:
GetMicSec(&s1,&us1);
getCountersBoard(board, counts-1, mem, 2);
GetMicSec(&s2,&us2);
usecs= DiffSecUsec(s2, us2, s1, us1);
printf("getCountersBoard took %d us\n", usecs);
//0:proxy 1:dims 2:ctp+busytool 3:smaq 4:inputs
for(cix=FROM; cix<counts; cix++) {
  if((cixmod%5)==0) {
    //if(cixmod>0) printf("\n");
    printf("\n%3d:",cix);
  } else {printf(" ");};
  printf("%8x",mem[cix]);
  cixmod++;
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
  if((ix==1) && (l0C0())) {
    va= TEST_ADDr2;
  } else {
    va= TEST_ADD+BSP*ctpboards[ix].dial;
  };
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
Synchronise/desynchronise 4 (2xLM 2xL0) random generators
mask:
3 synchronise RND1 and RND2 generators on L0 board
1 desynchronise RND1 and RND2 generators on L0 board
other: show current setting
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

/*---------------------------------------------------------------- DDR3 */
#define MEGA 1024*1024
w32 seqdata[16];
/*FGROUP DDR3 
Read DDR3  16 words (i.e. 1 block) from DDR3.
blockad: 0,1,2,...  corresponds to ddr3 addrees 0,16,32,... in 32bits words
       block length: 512 bits
*/
void ddr3_r16test(int blockad) {
int rc,ix, nwords;
nwords=16;
vmew32(DDR3_CONF_REG1, blockad*8);  
vmew32(DDR3_CONF_REG2, 1);
rc= ddr3_rddone(); if(rc!=0) return;
for(ix=0; ix<nwords; ix++) {
  if((ix%16)==0) {
    //vmew32(DDR3_CONF_REG1, 0);
    //vmew32(DDR3_CONF_REG2, nwords);
    printf("reading 16 words starting by DDR3_BUFF_DATA ...\n");
  };
  printf("%3d: 0x%x\n", ix, vmer32(DDR3_BUFF_DATA+ix*4));
};
}
/*FGROUP DDR3 
Write 16 words (i.e. 1 block) to DDR3 from blockad.
block: 0,1,2,...  corresponds to ddr3 addreses 0,16,32,... in 32bits words
       block length: 512 bits
*/
void ddr3_w16test(int blockad) {
int rc,ix; w32 val;
setseeds(7,3);
for(ix=0; ix<16; ix++) {
  int ix2;
  val= ix; for(ix2=0; ix2<3; ix2++) { val= (val<<4) | (ix); };
  seqdata[ix]= val;
};
vmew32(DDR3_CONF_REG3, blockad*8);   // from this block, (in 64bit words),
vmew32(DDR3_CONF_REG4, 1);       // number of blocks (512 bits)
for(ix=0; ix<16; ix++) {
  //int ix2; float fval;
  // 0..f:
  //val= ix; for(ix2=0; ix2<7; ix2++) { val= (val<<4) | (ix); };
  // random vals:
  // 0x00 at the end of val:
  //fval= RNLXMASK* rnlx(); val= rounddown(fval);
  // 0xff at the end of val:
  val= RNLXMASK* rnlx();
  val= (blockad<<16) | seqdata[ix];
  vmew32(DDR3_BUFF_DATA+ix*4, val);
  printf("%3d written 0x%x\n", ix, val);
  //if((ix%16)==0) {
};
/* following 2 lines here does not work:
vmew32(DDR3_CONF_REG3, blockad);   // from this block
vmew32(DDR3_CONF_REG4, 1);       // number of blocks
block0 write finishes:
ddr3_w16test( 0) 
 Error: status: 0xec000000
*/
rc= ddr3_wrdone();
}
w32 *bigarray;

/*FGROUP DDR3 
write,read,compare
ddr3_ad: 0, 16, 32,...   in words (1 word= 32 bits). If not,
         it will be rounded down
nws: number of words (1word: 32 bits). n*16, if not, it will be
     rounded up for allocation, but test done for nws words
     only

pattern (todo): 
0: all 0s
1: all 1 (i.e. each word set to 0xffffffff
2: random
3: each block (16 words = 64  bytes) set this way:

Notes: SSM is 2GB, max. allocated memory:
i.e. ddr3 chunks:
           ddr3_ad, nws
           ------------
0..   1MB  0, 0x40000
1..   2MB  0x40000, 0x40000    i.e. 2nd 1 MB chunk tested
0..  16MB  0, 0x400000         4.4 secs writing, 5.7secs reading
0..  64MB  0, 0x1000000        17 secs           23 secs 
0.. 256MB  0, 0x4000000
0.. 512MB  0, 0x8000000
For SSM we use 64MB way (23 secs)
*/
void ddr3_wr_test(w32 ddr3_ad, int nws) {
int rc, nws2, nerr=0, ix; w32 ddr3_adcor;
w32 tsec1,usec1,tsec2,usec2, usecs;
ddr3_adcor= ddr3_ad/16; ddr3_adcor= ddr3_adcor*16;
nws2= (nws-1)/16; nws2= (nws2+1)*16;
bigarray= (w32 *)malloc(nws2*4);   // allocate n*16
if(bigarray==NULL) {
  printf("0x%x (%d) not allocated\n", nws2, nws2);
  return;
};
setseeds(7,3);
for(ix=0; ix<nws2; ix++) {
  w32 val;
  val= RNLXMASK* rnlx();
  bigarray[ix]= val;
};
printf("0x%x (%d) 32bits words allocated + initialised, writing %d words to ddr3 from %x (%d)....\n",
 nws2, nws2, nws, ddr3_adcor, ddr3_adcor);
GetMicSec(&tsec1, &usec1);
//ddr3_reset();   reset: destroying memory content!
rc= ddr3_write(ddr3_adcor, bigarray, nws);  
if(rc!=0) {
  return;
};
GetMicSec(&tsec2, &usec2); usecs= DiffSecUsec(tsec2,usec2,tsec1,usec1);
printf("writing time: %d usecs\n", usecs);
printf("reading %d words...\n", nws);

GetMicSec(&tsec1, &usec1);
rc= ddr3_read(ddr3_adcor, bigarray, nws);
GetMicSec(&tsec2, &usec2); usecs= DiffSecUsec(tsec2,usec2,tsec1,usec1);
printf("reading time: %d usecs\n", usecs);
if(rc!=0) {
  return;
};
printf("testing %d words...\n", nws);
setseeds(7,3);
for(ix=0; ix<nws; ix++) {   // should be nws
  w32 val;
  val= RNLXMASK* rnlx();
  if(ix<5) {
    if(ix==0) {
      printf("first words, just to see rnd values generation...\n");
    };
    printf("%d: 0x%x\n", ix, val);
  };
  if(bigarray[ix] != val) {
    if(nerr<10) {
      printf("Error at %d: read:%x expected:0x%x\n", ix, bigarray[ix], val);
    };
    nerr++;
  };
};
printf("errors:%d, releasing memory...\n", nerr);
free(bigarray);
}
w32 ssm1[MEGA]; w32 ssm2[MEGA];
/*FGROUP DDR3 
Show ssm1, ssm2
from: 0..1024*1024-1
lines: number of words to stdout (equal lines printed only once)
mask: mask to be applied to second (16. word LM0 input mon)
      eg. 0xf  -be sensitive only to 4 least significant bits
 * */
void ddr3_ssmshow(int from, int lines, w32 mask) {
int ix; char line[90]="", line2[90]="", prevline[90]="abc";
if(from>=MEGA) {
  printf("Too far. Last address is: %d\n", MEGA-1);
  return;
};
for(ix=from; ix<= from+lines;  ix++) {
  w32 wrd;
  if(ix>=MEGA) break;
  wrd= ssm2[ix] & mask; 
  //sprintf(line, "%8x %8x", ssm1[ix], ssm2[ix]);
  sprintf(line, "%8x",wrd);
  sprintf(line2, "%8x %8x", wrd, ssm2[ix]);
  if( strcmp(line,prevline)!=0) {
    printf("%6d: %s\n", ix, line2);
    strcpy(prevline, line);
  };
};
}
/*FGROUP DDR3 
*/
int ddr3_dump(char *fname){
FILE *dump; int i,retcode=0;
int allbitn=0, ix=0; int bits[32];
int lowix=8; int highix=31;
for(ix=lowix; ix<=highix; ix++) bits[ix]=0;

dump= fopen(fname,"w");
if(dump==NULL) {
  printf("cannot open file %s\n", fname);
  retcode=0; goto RET;
};
for(i=0; i<MEGA; i++) {
  w32 d;
  d= ssm2[i];
  for(ix=lowix; ix<=highix; ix++) {
    w32 msk;
    msk= 1<<ix;
    if(d & msk) bits[ix]++;
  };
  fwrite(&d, sizeof(w32), 1, dump);
};
fclose(dump);
for(ix=lowix; ix<=highix; ix++) allbitn= allbitn+bits[ix];
printf("%s dumped. bits:%d\n", fname, allbitn);
retcode= allbitn;
RET: return(retcode);
}
/*FGROUP DDR3 
l0inppos: position in cnames.sorted2 (must be lm0 board. l0inp1:119)
idn:      name (example: t0c):
          idn.log, idn_N.dump files created
waitsecs: roughly in secs to wait for event
maxevents: stop after acquisition of maxevents

operation:
- start ssm
- stop ssm on condition (l0inp change)
- ddr3 read+dump
- write to log
- check quit condition
- loop again from start ssm

Note: use kilme pid  to raise quit condition
*/
int ddr3_daq(int l0inp, char *idn, int waitsecs, int maxevents){
FILE *logf; int loops=0,rc=0,rcssm=0,waitloops;
char dati[30], logname[80]; w32 ssmrad;
waitloops=1000*waitsecs;
sprintf(logname,"WORK/%s_.log",idn);
logf= fopen(logname,"w");
ssmrad= BSP*ctpboards[1].dial+SSMaddress;
while(1) {
  w32 stopadr; char fname[40];
  SSMS: printf("---> ssmstart(1)\n");
  ddr3_ssmstart(1);
  while(1) {
    rcssm= condstopSSM(1, l0inp, waitloops,10, 2); 
    printf("condstopSSM rc:%d loops:%d\n", rcssm, loops);
    loops++;
    if(loops>= maxevents) break;
    if(rcssm==0) break;   // data
    if(rcssm==10) continue;   // data not found try again
    if(rcssm==1) goto SSMS;   // not started
    break;    // vme error
  };
  if(rcssm==0) {
    int rcssmr;
    stopadr= vmer32(ssmrad);
    rcssmr= ddr3_ssmread(ssm1, ssm2);
    if(rcssmr==0) {
      getdatetime(dati);
      sprintf(fname,"WORK/%s_%d.dmp", idn, loops);
      rc= ddr3_dump(fname);
      fprintf(logf, "%s 0x%x %s %d\n", dati, stopadr, fname, rc);
    };
  } else {
    printf("condstopSSM rc:%d\n", rcssm);
  };
  if(loops>= maxevents) {
    if(rcssm==10) stopSSM(1);
    break;
  };
  if(quit !=0) {
    fprintf(logf, "SIGUSR1 received, finishing loop...\n"); quit=0;
    break;
  };
  fflush(logf);
};
fclose(logf);
return(rc);
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
printf("initmain()...\n");
cshmInit(); ix= initHW(&HW);  // HW: partialy used in loadcheckctpcfg
for(ix=0; ix<6; ix++) { 
  FOs[ix].cluster= 0;
  FOs[ix].tcluster= 0;
};
/* check/configure all the CTP boards: */
checkCTP();
initSSM();
/* von #ifdef SSMCONNECTIONS 
initNames();
#endif */
gettableSSM(0);
}
void endmain() {
}
void boardInit() {
printf("boardInit()...\n");
/*int ix;
for(ix=0; ix<6; ix++) {    moved to initmain (2.2.2007)
  FOs[ix].cluster= 0;
  FOs[ix].tcluster= 0;
};*/
/* do not init in ctp expert software because:
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

