/* Usage:
1. for testing correctness of these routines, uncomment
   lines with main() and compile:
   gcc timingpars.c -o timingpars.exe
2. Compilation only for libctp.a: see makefile
   getTimeParsDB()  -to be modified for use with DB
   calc*            -calculate particular register(s) value(s)
                     to be loaded into CTP
 */
#include <stdio.h>
//#include <unistd.h>   /* usleep */
//#include <time.h>   
//#include <sys/time.h>   /* gettimeofday */

typedef unsigned int w32;

static int ORBITLENGTH=3564;
static int TL1=260;                 // was 225 before 24.8.2008
/* see ltu/ltulib/ltuinit.c: L1_DEALY is TL1-1= 224
from 31.8.2007. 225 corresponds to LTU.L1_DELAY=TL1-1=224
from 4.6.2008 (LTUvi): 225 LTU.L1_DELAY= TL1-2= 223 which should give real 224bcs

LTU.L2_DELAY should be TL2
*/
//static int TL2=3472;  till 11.6.2008
static int TL2=3952;
static int TBCL0=0;   // L0_BCOFFSET from allignment measurement

/*--------------------------------*/ int getTimeParsDB() {
/* change defaults:
TL0: time of last L0 input synchronised with BC from interaction
TL1: positive edge of L1 signal on the output of CTP relative to TL0
TL2: positive edge of L2 signal on the output of CTP relative to TL0
TBCL0: alignement measurement. This value is used for calculation of:
       L0_BCOFFSET, L2_BCOFFSET, INT_BCOFFSET registers.
ORBITLENGTH
by values from DB */
return(0);
}

/*--------------------------------*/ int getTL1() {
return(TL1);
}
/*--------------------------------*/ int getTL2() {
return(TL2);
}
/*--------------------------------*/ w32 calcL0_BCOFFSET() {
/* L0_BCOFFSET register, L0 board */
return(TBCL0);
}
/*--------------------------------*/ w32 calcL1_DELAY_L0() {
/* L1_DELAY_L0 register, L1 board */
return(TL1-55);   // changed 4.6.2008 to -55
}
/*--------------------------------*/ w32 calcL2_DELAY_L1() {
/* L2_DELAY_L1 register, L2 board */
//return(TL2-TL1-56);    // changed 4.6.2008 to -56
return(TL2-TL1-64);    // to be changed (24.8.2008) to -64
}
/*--------------------------------*/ w32 calcBUSY_L0L1DEADTIME() {
/* BUSY_L0L1DEADTIME register, BUSY board */
//return(TL1-1);
return(TL1);      //changed 1.8.2008
}
/*--------------------------------*/ w32 calcFO_DELAY_L1CLST() {
/* FO_DELAY_L1CLST register, FO boards */
//return((TL2-TL1-38)/8.+1.);
return((TL2-TL1-38)/8);    // changed 1.8.2008
}
/*--------------------------------*/ w32 calcFO_FILTER_L1(){
return ((TL1-1)+(1<<12));}
/*--------------------------------*/ w32 calcL2_BCOFFSET() {
/* L2_BCOFFSET register, L1 board */
//return((TBCL0+TL2+1)%ORBITLENGTH);
return((TBCL0+TL2-5)%ORBITLENGTH); //26.8.2008
}
/*--------------------------------*/ w32 calcINT_BCOFFSET() {
/* INT_BCOFFSET register, INT board p.6 */
return((TBCL0+3)%ORBITLENGTH);       // 26.8.2008
//return((TBCL0+2)%ORBITLENGTH);       // 20.8.2008
//return((TBCL0+TL2+1)%ORBITLENGTH); // 1.8.2008
}
/*--------------------*/ w32 calcPFisd(int level) {
w32 rc;
if(level==0) {
  rc= 0;
} else if(level==1) {
  rc= TL1-3;   // not used (see calcPFisd() )
} else if(level==2) {
  rc= TL2-3;
} else {
rc=0xffffffff;
}
return(rc);
}
/*--------------------*/ void clcPFablut12(int T12, int dt, w32 *ablut) {
/* I: T12: =TL1 for L1 calculation
           =TL2 for L2 calculation
      dt: DeltaT (half of the PF protection interval) in BC
   O: ablut[4-7] or ablut[8-11] (in accordance with T12)
*/
int ix12; char c12;
w32 pfinterval, CLKfactor, CLKperiod, pfdelay;
if(T12==TL1) {ix12=4; c12='1'; 
  ablut[ix12+3]= calcPFisd(1)<<12;   // not used (see calcPFisd() )
};
if(T12==TL2) {ix12=8; c12='2'; 
  ablut[ix12+3]= calcPFisd(2)<<12;
};
if(dt<T12) {
  CLKfactor= 2*dt/256.; CLKperiod= CLKfactor+1;
  //printf("CLKfactorL%c:%d\n", c12, CLKfactor);
  ablut[ix12+2]= (CLKfactor<<13) | (CLKfactor<<8);
  pfinterval= 2.0*dt/CLKperiod; pfinterval= pfinterval - 1;
  pfdelay= 1.0*(dt + T12)/CLKperiod; pfdelay= pfdelay - pfinterval-2;
  ablut[ix12]= (pfinterval<<12) | (pfdelay<<20);
  ablut[ix12+1]= (pfinterval<<12) | (pfdelay<<20);
  // has to be the same for any dt! ablut[ix12+3]= (dt-3)<<12;
} else {
  CLKfactor= (dt+T12)/256.; CLKperiod= CLKfactor+1;
  //printf("CLKfactorL%c:%d\n", c12, CLKfactor);
  ablut[ix12+2]= (CLKfactor<<13) | (CLKfactor<<8);
  pfinterval= 1.0*(dt+T12)/CLKperiod; pfinterval= pfinterval - 1;
  pfdelay= 0;
  ablut[ix12]= (pfinterval<<12) | (pfdelay<<20) | 0x80000000;
  ablut[ix12+1]= (pfinterval<<12) | (pfdelay<<20) | 0x80000000;
};
}
/*--------------------------------*/ int calcPFablut(int dt, w32 *ablut) {
/* I: dt: DeltaT (half of the PF protection interval) in BC
   O: rc=0
      ablut: aL0,bL0,lutL0,delintL0,...,aL2,bL2,lutL2,delintL2
         L0: 0 1 2 3
         L1: 4 5 6 7 
         L2: 8 9 10 11
             I.e.: 3x4=12 words to be written to:
PFBLOCK_A + L012base + (pfcircuit-1)*0xc 
  abFlag[31] abDelay[30:20] DTa[19:12] TH2[11:6] TH1[5:0]
PFBLOCK_B + L012base + (pfcircuit-1)*0xc
  abFlag[31] abDelay[30:20] DTb[19:12] TH2[11:6] TH1[5:0]
PFLUT     + L012base + (pfcircuit-1)*0xc
  Bsc[17:13] Asc[12:8] Psignal[7:0]
PF_COMMON + L012base    
  bits [11:0] -> bits[23..12]          INT signal delay PFL.0
         where:
         L012base -is base addres of ?0/1/2 board
         pfcircuit: 1,2,3,4,5 (for PF circuits 1,2,3,4,T)
      rc=1: Error printed on stdout.
Example how to initialise pfT circuit for +-888BC past future interval:
w32 ablt12[12];
rc= calcPFablut(888, ablt12);
pfcircuit=5; 
for(ix=1; ix++;ix<=3) {   set it on L0/1/2 boards
  w32 pfcom,pfcb;
  bb= BSP*ctpboards[ix].dial; pfcb= bb + (pfcircuit-1)*0xc;
  vmew32(PFBLOCK_A + pfcb, ablt12[(ix-1)*4+0]);
  vmew32(PFBLOCK_B + pfcb, ablt12[(ix-1)*4+1]);
  vmew32(PFLUT     + pfcb, ablt12[(ix-1)*4+2]);
  !use calcPFisd() instead of following lines! (used in CTPinit.c)
  //pfcom= vmer32(PFCOMMON + bb ) & 0xff000fff
  //vmew32(PF_COMMON    +bb, pfcom | (ablt12[(ix-1)*4+3]<<12));
}
 */
int rc=0;
w32 pfinterval, CLKfactor, CLKperiod;
/* L0 board: */
if(dt>TL2) {
  //printf("PFdt(%d) > TL2(%d)\n", dt, TL2); 
  rc=1;
} else {
  CLKfactor= dt/256.; CLKperiod= CLKfactor+1;
  //printf("CLKfactorL0:%d\n", CLKfactor);
  ablut[2]= (CLKfactor<<13) | (CLKfactor<<8);
  pfinterval= 1.0*dt/CLKperiod; pfinterval= (pfinterval-1)&0xff;
  ablut[0]= pfinterval<<12;
  ablut[1]= pfinterval<<12;
  ablut[3]= 0;   // not necessary for L0
  clcPFablut12(TL1, dt, ablut);
  clcPFablut12(TL2, dt, ablut);
};
return(rc);
}

void prtPFblock(char *ab, w32 *pf) {
/* Input: ab: "A0"   ..    "B2" */
int ixc, ix;
if(ab[0]=='A') ixc=0;
if(ab[0]=='B') ixc=1;
if(ab[1]=='0') ix= ixc+0;
if(ab[1]=='1') ix= ixc+4;
if(ab[1]=='2') ix= ixc+8;
//printf("BLOCK_%c: TH1:%d=0x%x TH2:%d=0x%x DT:%d=0x%x Delay:%d=0x%x DF:%d\n",
printf("BLOCK_%c: TH1:%d TH2:%d Prot_Int:%d Delay:%d NDFlag:%d\n",
ab[0], pf[ix]&0x3f, (pf[ix]>>6)&0x3f, (pf[ix]>>12)&0xff, (pf[ix]>>20)&0x7ff,
(pf[ix]>>31)&0x1);
}

void prtPFlutcom(char ab, w32 *pf) {
int ix;
if(ab=='0') ix= 2;
if(ab=='1') ix= 6;
if(ab=='2') ix= 10;
//printf("Asd:%d=0x%x Bsd:%d=0x%x INTsigdel:%d=0x%x\n",
printf("ClkAscaled-down:%d ClkBscaled-down:%d Delayed INT signal delay:%d\n",
(pf[ix]>>8)&0x1f, (pf[ix]>>13)&0x1f, (pf[ix+1]>>12)&0xfff);
}
/*
int main(int argn, char **argv) {
int PFdt=440; w32 reg;
w32 pf[12];
if(argn<=1) {
  printf("Usage: exe PF_DeltaT        (half of PF interval in BCs)\n\n");
} else {
  PFdt=atoi(argv[1]);
};
printf("TL1:%d   TL2:%d   TBCL0 (L0 BC_offset):%d   PFdt:%d\n\n", 
TL1,TL2,TBCL0,PFdt);
reg= calcL1_DELAY_L0(); 
printf("L1_DELAY_L0:      %6d = 0x%8x\n", reg, reg);
reg= calcL2_DELAY_L1(); 
printf("L2_DELAY_L1:      %6d = 0x%8x\n", reg, reg);
reg= calcBUSY_L0L1DEADTIME(); 
printf("BUSY_L0L1DEADTIME:%6d = 0x%8x\n", reg, reg);
reg= calcFO_DELAY_L1CLST(); 
printf("FO_DELAY_L1CLST:  %6d = 0x%8x\n", reg, reg);
reg= calcL2_BCOFFSET(); 
printf("L2_BCOFFSET:      %6d = 0x%8x\n", reg, reg);
reg= calcINT_BCOFFSET(); 
printf("INT_BCOFFSET:      %6d = 0x%8x\n", reg, reg);
calcPFablut(PFdt, pf);
printf("L0:\n"); 
prtPFblock("A0", pf); prtPFblock("B0", pf); prtPFlutcom('0', pf);
printf("L1:\n"); 
prtPFblock("A1", pf); prtPFblock("B1", pf); prtPFlutcom('1', pf);
printf("L2:\n"); 
prtPFblock("A2", pf); prtPFblock("B2", pf); prtPFlutcom('2', pf);
}
*/
