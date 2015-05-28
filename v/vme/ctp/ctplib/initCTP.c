#include <stdio.h>
//#include <string.h>
#include "vmewrap.h"
#include "ctp.h"
#include "ctplib.h"
#include "infolog.h"
#include "Tpartition.h"

/* FGROUP SimpleTests 
Synchronise/desynchronise random generators
8.4.2015: also on LM level
mask:
3 synchronise RND1 and RND2 generators on L0 board
1 desynchronise RND1 and RND2 generators on L0 board
other: print to stdout current status (3: sync, 1: desync mode)
*/
void RNDsync(int mask) {
w32 adr,lmver;
lmver= l0C0();
if(lmver) {
  adr= L0_ENA_CRNDlm0;
} else {
  adr= L0_ENA_CRND;
};
if((mask != 3) and (mask != 1)) {
  if(lmver>=0xc5) {
    w32 w, wlm;
    w= vmer32(adr); wlm= vmer32(LM_ENABLE_CLEAR);
    printf("L0_ENA_CRNDlm0 (3: sync, 1: desync mode):0x%x LM_ENABLE_CLEAR:0x%x\n", w, wlm);
  } else {
    w32 w;
    w= vmer32(adr);
    printf("L0_ENA_CRND (3: sync, 1: desync mode):0x%x\n", w);
  };
} else {
  if(lmver>=0xc5) {
    vmew32(adr, mask);   // L0 random gens
    vmew32(L0_CLEAR_RND, DUMMYVAL);
    vmew32(LM_ENABLE_CLEAR, mask);   // LM random gens
    vmew32(LM_CLEAR_RANDOM, DUMMYVAL);
  } else {
    vmew32(adr, mask);
    vmew32(L0_CLEAR_RND, DUMMYVAL);
  };
};
}
/*
set L0_CONDITION L0_INVERT L0_VETO (scaler-not used here) 
    L1_DEFINITION L1_INVERT L2_DEFINITION 
words for klas (1..100)
ATTENTION: 
1. bit17 (0x10000) of veto is CLASS MASK bit written into bit0 of L0_MASK
   bit31 for >AC
   LM0: L0_MASK not used. CLASS MASK bit is in: L0_VETOr2.23
2. invert,l1invert -valid only for class>=45
*/
void setClassInit(int klas,w32 condition, w32 invert, w32 veto, w32 scaler,
              w32 l1def, w32 l1invert, w32 l2def) {
int bb,klasix;
w32 l0invAC; int minAC;
l0invAC=L0_INVERTac; minAC=0;
bb= klas*4; klasix=klas-1;
if(notInCrate(1)==0) {   // L0 board
  w32 mskbit;
  printf("setClassInit:%d:%x %x %x %x\n", klas, condition,invert,veto,scaler);
  //Klas[klasix].regs[0]= condition; 
  vmew32(L0_CONDITION+bb, condition);
  if(klas>minAC) {  /* only for inverted klasses ! */
    //Klas[klasix].regs[1]= invert; 
    vmew32(l0invAC+bb, invert);
  };
  //Klas[klasix].regs[2]= veto; 
  if(l0AB()==0) {   //firm AC or higher
    if(l0C0()) {
      vmew32(L0_VETOr2+bb, veto);
    } else {
      vmew32(L0_VETO+bb, veto&0x1fffff); 
      mskbit= veto>>31; vmew32(L0_MASK+bb, mskbit);
      if(ctpboards[1].boardver>=0xaf) {  // sync. downscaling
        //printf("boardver:%x klas:%d sdg:%d\n", ctpboards[1].boardver, klas, klas-1);
        vmew32(L0_SDSCG+bb, klas-1); 
        //printf("initCTP%x+ %d <- %d\n", L0_SDSCG, bb, klas-1);
      };
    };
  } else {
    vmew32(L0_VETO+bb, veto&0xffff); 
    /* 1st L0 version (A0): vmew32(L0_MASK+bb, veto&0x10000); */
    mskbit= (veto&0x10000)>>16; vmew32(L0_MASK+bb, mskbit);
  };
  /*Klas[klasix].regs[3]= scaler;  update is done in 1 pass in rates2hwInit */
};
if(notInCrate(2)==0) {   // L1 board
  //Klas[klasix].regs[4]= l1def; 
  vmew32(L1_DEFINITION+bb, l1def);
  if(klas>=1) {  /* was 45 before 13.9.2014 (bug) */
    //Klas[klasix].regs[5]= l1invert; 
    vmew32(L1_INVERT+bb, l1invert);
  };
};
if(notInCrate(3)==0) {
  //Klas[klasix].regs[6]= l2def; 
  vmew32(L2_DEFINITION+bb, l2def);
};
}
void setClassInitLM(int klas, w32 lmcondition, w32 lminvert, w32 lmveto) {
int bb,klasix;
bb= klas*4; klasix=klas-1;
if(notInCrate(1)==0) {   // L0 board
  printf("setClassInitLM:%d:%x %x %x\n", klas, lmcondition,lminvert,lmveto);
  vmew32(LM_CONDITION+bb, lmcondition);
  vmew32(LM_INVERT+bb, lminvert);
  vmew32(LM_VETO+bb, lmveto);
};
}

/* init LM/L0 downscaling: */
void rates2hwInit() {
int ix;
vmew32(getRATE_MODE(),1);   /* vme mode */
vmew32(RATE_CLEARADD,DUMMYVAL);
for(ix=0; ix<NCLASS; ix++) {
  vmew32(RATE_DATA, 0);    // 0: from 23.6.2014  (ix<<25): before
};
vmew32(getRATE_MODE(),0);   /* normal mode */
if(l0C0()>=0xc5) {
  vmew32(LM_RATE_MODE,1);   /* vme mode */
  vmew32(LM_RATE_CLEARADD,DUMMYVAL);
  for(ix=0; ix<NCLASS; ix++) {
    vmew32(LM_RATE_DATA, 0);
  };
  vmew32(LM_RATE_MODE,0);   /* normal mode */
};
}
/*------------------------------------------------------ setEdgesDelays
10.12.2014 added: L0 switch programming according to validCTPINPUTS
*/
void setEdgesDelays(int board) {
int ix, ixinp, ninp, edge, delay;
if(board==3) {
  ninp=12;
} else if(board==2) {
  ninp=24;
} else {
  ninp=24;
  if(l0C0()) {
    //printf(" ERROR: setEdgesDelays: does not work for LM0 board!\n"); return;
    ninp=48;
    // L0 switch:
    for(ixinp=1; ixinp<=24; ixinp++) {
      int swn;
      swn= getSwnDB(ixinp);
      if(swn==-1) continue;
      setSwitch(swn, ixinp);
    };
    // LM switch:
    for(ixinp=1; ixinp<=12; ixinp++) {
      int ixvci,lminp,lmdel;
      ixvci= findSwitchInput(ixinp);   // only first 12 switch inputs
      lmdel=0; lminp=0;
      if(ixvci>=0) {   // connected
        lminp=validCTPINPUTs[ixvci].lminputnum;
        lmdel=validCTPINPUTs[ixvci].lmdelay;
        if(lminp<=0) {
          lminp=0;
        };
        if(lmdel<=0) {
          lmdel=0;
        };
      };
      setLMSwitch(validCTPINPUTs[ixvci].switchn, lminp);
      setlmdelay(ixinp, lmdel);
    };
  };
};
for(ixinp=0; ixinp<ninp; ixinp++) {
  ix= getEdgeDelayDB(board-1, ixinp+1, &edge, &delay);
  if(ix==-1) {edge=0; delay=0; }   // not configured
  if( (edge==-1) || (delay==-1) ) {edge=0; delay=0;};  //not in DB
  setEdgeDelay(board,ixinp+1,edge,delay);
};
}
/*FGROUP LM0
init CTP. Should be called only once. 
   ! Another instance of 'vmecrate ctp' calls this routine too! 
   -> i.e. use: 'vmecrate nbi ctp' for expert (ctp) sw.
   ctp_proxy, when restarted, calls this routine.
   Only 'system parameters' should be set here (i.e. timing...)
*/
void initCTP() {
int ix,rc=0;
resetPLLS();
for(ix=1; ix<=NCLASS; ix++) {
  //klas,w32 condition, w32 invert, w32 veto, w32 scaler,
  //            w32 l1def, w32 l1invert, w32 l2def
  //setClassInit(ix, 0x3fffffff, 0x0, 0x11ff0, 0, 0xffffffff, 0x0, 0xf000fff);
  if(l0C0()){
    setClassInit(ix,0xffffffff,0x0,0x009ffff0 | (ix-1)<<24, 0, 0x8fffffff, 0x0, 0xf000fff);
    setClassInitLM(ix,  0xffffffff, 0, 0x803f00 | ((ix-1)<<24));
  }else {
    setClassInit(ix,0xffffffff,0x0,0x801ffff0, 0, 0x8fffffff, 0x0, 0xf000fff);
  };
}; 
infolog_trg(LOG_INFO, (char *)"setClassInit: 1..100, also LM_VETO set to '1's");
//dbgssm("classes set");
/*
printf("omitting IR and L0F12 init to 0, on L0/LM0 board (programmed elsewhere).\n");
vmew32(getLM0addr(L0_INTERACT1), 0); vmew32(getLM0addr(L0_INTERACT2), 0);
vmew32(getLM0addr(L0_INTERACTT), 0); vmew32(getLM0addr(L0_INTERACTSEL), 0);
vmew32(getLM0addr(L0_FUNCTION1), 0); vmew32(getLM0addr(L0_FUNCTION2), 0);
*/
if(l0AB()==0) {   //firmAC
  if(l0C0()) {
    printf("omitting setL0f34c() on LM0 board.\n");
  } else {
    rc= setL0f34c(0, (char *)"0");
  };
};
ix= loadcheckctpcfg();
//dbgssm("after loadcheckctpcfg");
for(ix=0; ix<NCTPBOARDS; ix++) {
  if(notInCrate(ix)) continue;
  if(ctpboards[ix].code==FOcode) { 
    /*
    vmew32(SSMcommand+BSP*ctpboards[ix].dial, 0);
    vmew32(SSMenable+BSP*ctpboards[ix].dial, 0);
    */
    //calc vmew32(FO_DELAY_L1CLST + BSP*ctpboards[ix].dial, 412);
    //cfg vmew32(FO_DELAY_L1CLST + BSP*ctpboards[ix].dial, calcFO_DELAY_L1CLST());
    //cfg vmew32(FO_FILTER_L1    + BSP*ctpboards[ix].dial, calcFO_FILTER_L1());
  };
  if(ctpboards[ix].code==BUSYcode) {
    //cfg printf("Configuring BUSY board in 'external orbit mode'...\n"); 
    //cfg vmew32(BUSY_ORBIT_SELECT, 0);   // external orbit
/*
#ifndef CAVERN_SETUP
    printf("Configuring BUSY board in 'local orbit mode'...\n"); 
    vmew32(BUSY_ORBIT_SELECT, 3563 | 0x2000);   
#endif
*/
    //cfg vmew32(BUSY_DELAY_ADD, 0);
    vmew32(BUSY_DISB_CTP_BUSY, 1);   /* disable CTPbusy (see INT) */
    /* cfg vmew32(BUSY_CTPDEADTIME, 60);    
       longest serial message in bits -2 -2. 
       Now: (61: 1bit header +60databits) + (3: to be safe) = 64
       i.e. BUSY_CTPDEADTIME is 64 -2 -2 = 60
    
      - was 52 before 13.3. 2008
      - was 60 before 30.1.2014
      - is 60+50=110 after 30.1.2014 which corresponds 
        to real deadtime (scope) to 112BCs
       BUSY_L0L1_DEADTIME=213 corresponds to L1_DELAY_L0:160 -> to get 1 BC 
       between L1end/nearest_next_L0start on LTU output when using 
       'L0 over TTC' option
    */
    //calc vmew32(BUSY_L0L1DEADTIME, 213);  
    //cfg vmew32(BUSY_L0L1DEADTIME, calcBUSY_L0L1DEADTIME());  
  };                                 
  if(ctpboards[ix].code==L0code) { 
    int lmversion;
    //cfg vmew32(getLM0addr(L0_INTERACT1), 0);   // no interactions defined
    //cfg vmew32(getLM0addr(L0_INTERACT2), 0); //cfg vmew32(getLM0addr(L0_INTERACTT), 0);
    //cfg vmew32(getLM0addr(L0_INTERACTSEL), 0);
    //cfg vmew32(L0_BCOFFSET[r2],calcL0_BCOFFSET());
    //cfg vmew32(getLM0PFad(PF_COMMON)+BSP*ctpboards[ix].dial, calcPFisd(0)<<12);
    //setEdge(1,6,0);   // acorde single needs Positive edge
    rates2hwInit();
    infolog_trgboth(LOG_INFO, (char *)"LM+L0 class downscaling initialised to 0");
    lmversion= l0C0();
    if(lmversion) {
      // init CTP L0 switch to 1->1, 2->2,..., 24->24
      //          LM switch to 1->1... 12->12
      int lminp=1; w32 lminpadr;
      lminpadr= SYNCH_ADDr2 + BSP*ctpboards[ix].dial;
      for(lminp=1; lminp<=24; lminp++) {
        w32 val,adr;
        val= lminp<<16;   // positive edge(s), delay(s): 0
        if(lminp<=12) {
          //w32 lmw;
          //lmw= (lminp<<28); val= val | lmw;
          //lmw= val= val & 0x0fffffff;   // all 0s
          val= val & 0x0fffffff;   // all LM unconnected (0s)
        };
        adr= lminpadr+ 4*(lminp-1);
        vmew32(adr, val);
      }
      infolog_trgboth(LOG_INFO, (char *)"L0 switch set to 1-1...24-24, pos. edge. LM switch:all 0s");
      vmew32(RND1_EN_FOR_INPUTS, 0); vmew32(RND1_EN_FOR_INPUTS+4, 0);
      infolog_trgboth(LOG_INFO, (char *)"RND1 connections to switch inputs cleared");
      /*vmew32(SEL_SPARE_OUT+0xc, 1);   // 0T0C -> LM
      infolog_trgboth(LOG_INFO, (char *)"SEL_SPARE[3]) set to 1:0T0C -LM"); */
      if(lmversion>=0xc5) {
        vmew32(LM_L0_TIME, 12);
        infolog_trgboth(LOG_INFO, (char *)"LM0ver:>=0xc5 LM_L0_TIME:12 (i.e. +3=15 on LMboard, +8=20 for TRD)");
      } else {
        vmew32(SEL_SPARE_OUT+0xc, 11);
        infolog_trgboth(LOG_INFO, (char *)"LM0ver:<=0xc4 SEL_SPARE[3]) set to 11:0HCO -LM");
      };
      vmew32(SEL_SPARE_OUT+0x8, 45);
      infolog_trgboth(LOG_INFO, (char *)"SEL_SPARE[2]) set to 45:0AMU");
      /*vmew32(RND1_EN_FOR_INPUTS, 0); vmew32(RND1_EN_FOR_INPUTS+4, 0);
      infolog_trgboth(LOG_INFO, (char *)"RND1_EN_FOR_INPUTS cleared"); */
      //infolog_trgboth(LOG_INFO, (char *)"omitting LM0 CTP switch set to default 1-1 2-2...");
      /* following done in setEdgesDelays()
      setEdgeDelay(1,1,1,0);  // T0C
      ...
      setSwitch(39, 15);   // 0LSR
      */
      ddr3_reset(); printf("DDR3 reset done\n");
      vmew32(SCOPE_A_FRONT_PANEL, 14); printf("SCOPE_A_FRONT set to PLL_LOCKED_BC signal\n");
    };
    setEdgesDelays(1); printf("L0+LM CTPswitch: edges, delays set from ctpinputs.cfg\n");
    vmew32(getLM0addr(ALL_RARE_FLAG ), 1);   // 1:ALL (i.e. kill all classes with ALLRARE:0)
    vmew32(ALL_RARE_FLAG, 1);   // 1:ALL (i.e. kill all classes with ALLRARE:0)
    printf("ALL_RARE_FLAG:ALL (common for LM+L0 level)\n");
    //RNDsync(3); printf("RND1/2 synchronised\n");
    RNDsync(1); printf("RND1/2 desynchronised on both LM and L0 levels\n");
  };
  if(ctpboards[ix].code==L1code) { 
    //cfg vmew32(L1_DELAY_L0, calcL1_DELAY_L0());
    //cfg vmew32(PF_COMMON+BSP*ctpboards[ix].dial, calcPFisd(1)<<12);
    //cfg vmew32(ROIP_BUSY,1);   // no ESR flag in L1 messages
    setEdgesDelays(2);
  };
  if(ctpboards[ix].code==L2code) { 
   //calc vmew32(L2_DELAY_L1,3260);
   //cfg vmew32(L2_DELAY_L1,calcL2_DELAY_L1());
   //cfg vmew32(L2_BCOFFSET,calcL2_BCOFFSET());
   //cfg  vmew32(PF_COMMON+BSP*ctpboards[ix].dial, calcPFisd(2)<<12);
    setEdgesDelays(3);
  };
  if(ctpboards[ix].code==INTcode) { 
    // dissable scope outputs (i.e. GND):
    setScopeBoard('A', 4); setScopeBoard('B', 4);
    setScopeSignal(4,'A',5); setScopeSignal(4,'B',5);
   //cfg vmew32(INT_BCOFFSET,calcINT_BCOFFSET());
    vmew32(BUSY_DISB_CTP_BUSY, 0);   /* enable CTPbusy if INT in crate */
    /* following should be:
       0: if DAQ involved
       0xb: if DAQ not involved */
    //cfg vmew32(INT_DDL_EMU,0x0);       /* DAQ from 30.5.2008 always active */
    if(cshmGlobFlag(FLGignoreDAQRO)) {
      vmew32(INT_DDL_EMU,0xb);      /*  DAQ not active */
      infolog_trgboth(LOG_INFO, (char *)"CTP readout OFF (NODAQRO option)");
    } else {
      vmew32(INT_DDL_EMU,0x0);      /*  DAQ active  */
      infolog_trgboth(LOG_INFO, (char *)"CTP readout ON");
    };
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
/*----------------------------------------- */ int softLEDimplemented(int ix) {
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

