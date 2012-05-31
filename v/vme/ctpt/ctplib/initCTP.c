#include <stdio.h>
//#include <string.h>
#include "vmewrap.h"
#include "ctp.h"
w32 calcFO_DELAY_L1CLST();
w32 calcFO_FILTER_L1();
w32 calcBUSY_L0L1DEADTIME();
w32 calcPFisd(int level);
w32 calcL1_DELAY_L0();
w32 calcL2_DELAY_L1();
w32 calcL0_BCOFFSET();
w32 calcL2_BCOFFSET();
w32 calcINT_BCOFFSET();

/*
set L0_CONDITION L0_INVERT L0_VETO L0_PRESCALER 
    L1_DEFINITION L1_INVERT L2_DEFINITION
words for klas (1..50)
ATTENTION: 
1. bit17 (0x10000) of veto is CLASS MASK bit written into bit0 of L0_MASK
2. invert,l1invert -valid only for class>=45
*/
void setClassInit(int klas,w32 condition, w32 invert, w32 veto, w32 scaler,
              w32 l1def, w32 l1invert, w32 l2def) {
int bb,klasix; w32 mskbit;
bb= klas*4; klasix=klas-1;
if(notInCrate(1)==0) {   // L0 board
  //Klas[klasix].regs[0]= condition; 
  vmew32(L0_CONDITION+bb, condition);
  if(klas>=45) {  /* only for inverted klasses ! */
    //Klas[klasix].regs[1]= invert; 
    vmew32(L0_INVERT+bb, invert);
  };
  //Klas[klasix].regs[2]= veto; 
  vmew32(L0_VETO+bb, veto&0xffff); 
  /* 1st L0 version (A0): vmew32(L0_MASK+bb, veto&0x10000); */
  mskbit= (veto&0x10000)>>16; vmew32(L0_MASK+bb, mskbit);
  /*Klas[klasix].regs[3]= scaler;  update is done in 1 pass in rates2hwInit */
};
if(notInCrate(2)==0) {   // L1 board
  //Klas[klasix].regs[4]= l1def; 
  vmew32(L1_DEFINITION+bb, l1def);
  if(klas>=45) {  /* only for inverted klasses ! */
    //Klas[klasix].regs[5]= l1invert; 
    vmew32(L1_INVERT+bb, l1invert);
  };
};
if(notInCrate(3)==0) {
  //Klas[klasix].regs[6]= l2def; 
  vmew32(L2_DEFINITION+bb, l2def);
};
}
void rates2hwInit() {
int ix;
vmew32(RATE_MODE,1);   /* vme mode */
vmew32(RATE_CLEARADD,DUMMYVAL);
for(ix=0; ix<50; ix++) {
  vmew32(RATE_DATA, 0);
};
vmew32(RATE_MODE,0);   /* normal mode */
}
/* init CTP. Should be called only once. 
   ! Another instance of vmecrate ctp calls this routine too! 
   -> i.e. use: 'vmecrate nbi ctp'.
   ctp_proxy, when restarted, calls this routine.
   Only 'system parameters' should be set here (i.e. timing...)
*/
void initCTP() {
int ix;
for(ix=1; ix<=50; ix++) {
  setClassInit(ix, 0x3fffffff, 0x0, 0x11ff0, 0, 0xffffffff, 0x0, 0xf000fff);
}; rates2hwInit();
for(ix=0; ix<NCTPBOARDS; ix++) {
  if(notInCrate(ix)) continue;
  if(ctpboards[ix].code==FOcode) { 
    /*
    vmew32(SSMcommand+BSP*ctpboards[ix].dial, 0);
    vmew32(SSMenable+BSP*ctpboards[ix].dial, 0);
    */
    //calc vmew32(FO_DELAY_L1CLST + BSP*ctpboards[ix].dial, 412);
    vmew32(FO_DELAY_L1CLST + BSP*ctpboards[ix].dial, calcFO_DELAY_L1CLST());
    vmew32(FO_FILTER_L1    + BSP*ctpboards[ix].dial, calcFO_FILTER_L1());
  };
  if(ctpboards[ix].code==BUSYcode) {
#ifdef CAVERN_SETUP
    printf("Configuring BUSY board in 'external orbit mode'...\n"); 
    vmew32(BUSY_ORBIT_SELECT, 0);   // external orbit
#else
    printf("Configuring BUSY board in 'local orbit mode'...\n"); 
    vmew32(BUSY_ORBIT_SELECT, 3563 | 0x2000);   
#endif
    vmew32(BUSY_DISB_CTP_BUSY, 1);   /* disable CTPbusy (see INT) */
    /* longest serial message in bits -2 -2. 
       Now: (61: 1bit header +60databits) + (3: to be safe) = 64
       i.e. BUSY_CTPDEADTIME is 64 -2 -2 = 60
    */
    vmew32(BUSY_CTPDEADTIME, 60);    /* was 52 before 13.3. 2008 */
    /* 213 corresponds to L1_DELAY_L0:160 -> to get 1 BC between 
       L1end/nearest_next_L0start on LTU output when using 'L0 over TTC'
       option */
    //calc vmew32(BUSY_L0L1DEADTIME, 213);  
    vmew32(BUSY_L0L1DEADTIME, calcBUSY_L0L1DEADTIME());  
  };                                 
  if(ctpboards[ix].code==L0code) { 
    vmew32(L0_INTERACT1, 0);   // no interactions defined
    vmew32(L0_INTERACT2, 0);
    vmew32(L0_INTERACTT, 0);
    vmew32(L0_INTERACTSEL, 0);
    vmew32(L0_BCOFFSET,calcL0_BCOFFSET());
    vmew32(PF_COMMON+BSP*ctpboards[ix].dial, calcPFisd(0)<<12);
    //setEdge(1,6,1);   // acorde single needs Negative edge
    setEdge(1,6,0);   // acorde single needs Positive edge
  };
  if(ctpboards[ix].code==L1code) { 
    vmew32(L1_DELAY_L0, calcL1_DELAY_L0());
    vmew32(PF_COMMON+BSP*ctpboards[ix].dial, calcPFisd(1)<<12);
    vmew32(ROIP_BUSY,1);   // no ESR flag in L1 messages
  };
  if(ctpboards[ix].code==L2code) { 
   //calc vmew32(L2_DELAY_L1,3260);
   vmew32(L2_DELAY_L1,calcL2_DELAY_L1());
   vmew32(L2_BCOFFSET,calcL2_BCOFFSET());
    vmew32(PF_COMMON+BSP*ctpboards[ix].dial, calcPFisd(2)<<12);
  };
  if(ctpboards[ix].code==INTcode) { 
    // dissable scope outputs (i.e. GND):
    setScopeBoard('A', 4); setScopeBoard('B', 4);
    setScopeSignal(4,'A',5); setScopeSignal(4,'B',5);
   vmew32(INT_BCOFFSET,calcINT_BCOFFSET());
    vmew32(BUSY_DISB_CTP_BUSY, 0);   /* enable CTPbusy if INT in crate */
    /* following should be:
       0: if DAQ involved
       0xb: if DAQ not involved */
#ifdef CAVERN_SETUP
    vmew32(INT_DDL_EMU,0x0);       /* DAQ from 30.5.2008 always active */
#else
    vmew32(INT_DDL_EMU,0xb);      /*  DAQ not active if not in cavern */
    printf("CTP readout OFF\n");
#endif
  };
};
}
/*---------------------------------------------------  int nnis1nn(ix) { */
/* ix: board number (0-10).
rc: 1 if board:
- is already converted to addresses:
  '1nn' (pedja's notation), i.e. 0xcc -> 0x4cc 
- SOFT_LED is valid for this board
These boards are converted:
busy fpga >= A3

if((ix==0) && (ctpboards[ix].boardver >= 0xa3) ) {
  return(1);
} else {
  return(0);
};
}*/
/*----------------------------------------- */ int softLEDimplemented(ix) {
/* ix: board number (0-10).
rc: 1 if Software LED implemented on the board:
These boards are converted:
busy fpga >= A3
l0 fpga >= A5
All the boards support SOFTLED (from 'CTPinputs readout implemented'
*/
if(((ix==0) && (ctpboards[ix].boardver >= 0xa3)) ||
   ((ix==1) && (ctpboards[ix].boardver >= 0xa5)) ||
   ((ix==2) && (ctpboards[ix].boardver >= 0xa2)) ||   // l1
   ((ix==3) && (ctpboards[ix].boardver >= 0xa4)) ||   // l2
   ((ix==4) && (ctpboards[ix].boardver >= 0xa5)) ||   // int
   (((ix>=5) && (ix<=10)) && (ctpboards[ix].boardver >= 0xa9))   //fo 
  ) {
  return(1);
} else {
  return(0);
};
}

