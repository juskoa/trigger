#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vmewrap.h"
#include "ctp.h"
#include "ctplib.h"

typedef struct {
  w32 pPF_COMMON;
  w32 pPFBLOCK_A[5];
  w32 pPFBLOCK_B[5];
  w32 pPFLUT[5];
} Tpfc;
static Tpfc PF[3]; /* for L0,1,2 */ /*={ */ 
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


/*---------------------------------------------------------------- L0-PF */
void getPF(int ix) {
int bb,ixa; w32 val;
if(notInCrate(ix)) return;
bb= BSP*ctpboards[ix].dial;
ixa=ix-1;
if(ix==1) {
  val= vmer32(getLM0PFad(PF_COMMON)+bb); 
} else {
  val= vmer32(PF_COMMON+bb); 
};
PF[ixa].pPF_COMMON= val;
printf("0x%x\n", PF[ixa].pPF_COMMON);
}
void getprtPF(int ix) {
int bb,ixa; w32 val,INTa,INTb,dINT,delay;
if(notInCrate(ix)) return;
bb= BSP*ctpboards[ix].dial;
ixa=ix-1;
if(ix==1) {
  val= vmer32(getLM0PFad(PF_COMMON)+bb); 
} else {
  val= vmer32(PF_COMMON+bb); 
};
PF[ixa].pPF_COMMON= val;
//printf("0x%x\n", PF[ixa].pPF_COMMON);
INTa= val & 0xf; INTb= (val>>4) & 0xf;
dINT= (val>>8) & 0xf;
delay= (val>>12) & 0xfff;
printf("L%d:%x:INTa/b/delayed 0x: %x %x %x delay:%d\n", ixa, val, INTa, INTb,dINT,delay);
}
//------------------------------------------------------------------------------------
void getPFc(int ix, int circ) {
int bb,ixa, ixc, i;
w32 adr0, vals[3];
char os[80]="";

if(notInCrate(ix)) return;
bb= BSP*ctpboards[ix].dial;
ixa=ix-1; ixc=circ-1;
if(ix==1) {
  adr0= getLM0PFad(PFBLOCK_A)+bb+(ixc*12);
} else {
  adr0= PFBLOCK_A+bb+(ixc*12);
};
  /* instead get from memory for dbg: 
  vals[0]=PF[ixa].pPFBLOCK_A[ixc];
  vals[1]=PF[ixa].pPFBLOCK_B[ixc];
  vals[2]=PF[ixa].pPFLUT[ixc]; */
for(i=0; i<3; i++) {
  w32 val;
  val= vmer32(adr0); vals[i]= val;
  sprintf(os, "%s0x%x ", os, vals[i]);
  adr0= adr0+4;
};
PF[ixa].pPFBLOCK_A[ixc]= vals[0];
PF[ixa].pPFBLOCK_B[ixc]= vals[1];
PF[ixa].pPFLUT[ixc]= vals[2];
printf("%s\n", os);
}
//----------------------------------------------------------------------------------------------
void getprtPFc(int ix, int circ) {
int bb,ixa, ixc, i;
w32 adr0, vals[3];
char os[120]="";

if(notInCrate(ix)) return;
bb= BSP*ctpboards[ix].dial;
ixa=ix-1; ixc=circ-1;
if(ix==1) {
  adr0= getLM0PFad(PFBLOCK_A)+bb+(ixc*12);
} else {
  adr0= PFBLOCK_A+bb+(ixc*12);
};
  /* instead get from memory for dbg: 
  vals[0]=PF[ixa].pPFBLOCK_A[ixc];
  vals[1]=PF[ixa].pPFBLOCK_B[ixc];
  vals[2]=PF[ixa].pPFLUT[ixc]; */
for(i=0; i<3; i++) {
  vals[i]= vmer32(adr0); 
  adr0= adr0+4;
};
sprintf(os,"L%d.%d:%x %x %x\n", ixa, circ, vals[0], vals[1], vals[2]);
for(i=0; i<3; i++) {
  w32 val; char block;
  val= vals[i];
  //sprintf(os, "%s0x%x ", os, vals[i]);
  if(i==0) { block='A';};
  if(i==1) { block='B';};
  if(i<2) {
    w32 th1,th2,dT,delay,dflag;
    th1= val & 0x3f;
    th2= (val>>6) & 0x3f;
    dT= (val>>12) & 0xff;
    delay= (val>>20) & 0x3ff;
    dflag= val>>31;
    sprintf(os, "%s%c:th1/2:%d %d dT:%d del/f:%d/%d ", os,block,th1,th2,dT,delay,dflag);
  } else {
    w32 plut, sdA, sdB;
    plut= val & 0xff; sdA= (val>>8) & 0x1f; sdB= (val>>13) & 0x1f;
    sprintf(os, "%sP:0x%x %d %d ", os,plut,sdA, sdB);
  };
};
PF[ixa].pPFBLOCK_A[ixc]= vals[0];
PF[ixa].pPFBLOCK_B[ixc]= vals[1];
PF[ixa].pPFLUT[ixc]= vals[2];
printf("%s\n", os);
}
//---------------------------------------------------------------------------------------------
void setPF(int ix, w32 pfc) {   // ix:1..3 (L0.. L2)
int bb,ixa;
if(ix==1) { printf("setPF: PF_COMMON <- 0x%x, ignored.\n", pfc); return;};
if(notInCrate(ix)) return;
bb= BSP*ctpboards[ix].dial;
ixa=ix-1;
/*if(pfc != PF[ixa].pPF_COMMON ) { */
/*  printf("POZOR\n"); */
  PF[ixa].pPF_COMMON= pfc;
  if(ix==1) {
    vmew32(getLM0PFad(PF_COMMON)+bb, pfc);
  } else {
    vmew32(PF_COMMON+bb, pfc);
  };
/*}; */
}
//----------------------------------------------------------------------------------------------
void setPFc(int ix, int circ, w32 A, w32 B, w32 LUT) { //ix:1..3 circ:1..4
int bb,ixa, ixc, i;
w32 adr0, vals[3];
w32 *vala[3];

if(ix==1) { printf("setPFc: PF_BLOCK_A/B/LUT <- 0x%x 0x%x 0x%x, ignored.\n", A, B, LUT); return;};
if(notInCrate(ix)) return;
bb= BSP*ctpboards[ix].dial;
ixa=ix-1; ixc=circ-1;
if(ix==1) {
  adr0= getLM0PFad(PFBLOCK_A)+bb+(ixc*12);
} else {
  adr0= PFBLOCK_A+bb+(ixc*12);
};
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
//-----------------------------------------------------------------------------------
void printPFwc(int deltat) {
int rc, ix;
w32 ablt12[12];
rc= calcPFablut(deltat, ablt12);
if(rc!=0) ablt12[0]=0xffffffff;
for(ix=0; ix<12; ix++) {
  printf("0x%x ", ablt12[ix]);
}; printf("\n");
/*
for(ix=1; ix++;ix<=3) {   set it on L0/1/2 boards
  w32 pfcom,pfcb;
  bb= BSP*ctpboards[ix].dial; pfcb= bb + (pfcircuit-1)*0xc;
  vmew32(PFBLOCK_A + pfcb, ablt12[0]);
  vmew32(PFBLOCK_B + pfcb, ablt12[1]);
  vmew32(PFLUT     + pfcb, ablt12[2]);
  pfcom= vmer32(PFCOMMON + bb ) & 0xff000fff
  vmew32(PF_COMMON    +bb, pfcom | (ablt12[3]<<12));
};
return(rc);
*/
}

/*FGROUP PF 
 * ----------------------------------------------------------------------------------------------
*/
void ReadPF(int circuit) {
int i,mi,ma;
/* Read PF_COMMON */
getprtPF(1); getprtPF(2); getprtPF(3);
/* Read PF_BLOCK and PF_LUT */
if(circuit==0) {
  mi=1; ma=5;
} else {
  mi=circuit; ma=circuit;
};
for (i=mi; i<=ma; i++) {
  getprtPFc(1,i);
  getprtPFc(2,i);
  getprtPFc(3,i);
};
}
//------------------------------------------------------------------------------------------------
void ReadPF2str2(int circuit, char *line) {
int bb,ixa, ixc, ix;
w32 adr0, val, vals[3];
char os[160]="";

for(ix=1; ix<=3; ix++) {
  int i;
  if(notInCrate(ix)) return;
  bb= BSP*ctpboards[ix].dial;
  ixa=ix-1; ixc=circuit-1; 
  if(ix==1) {
    adr0= getLM0PFad(PFBLOCK_A)+bb+(ixc*12);
  } else {
    adr0= PFBLOCK_A+bb+(ixc*12);
  };
  for(i=0; i<3; i++) {
    vals[i]= vmer32(adr0); 
    adr0= adr0+4;
  };
  //sprintf(os,"%sL%d.%d:%x %x %x\n",os,ixa,circuit,vals[0],vals[1],vals[2]);
  sprintf(os,"%s0x%x 0x%x 0x%x ",os,vals[0],vals[1],vals[2]);
}; 
// common
for(ix=1; ix<=3; ix++) {
  bb= BSP*ctpboards[ix].dial;
  if(ix==1) {
    val= vmer32(getLM0PFad(PF_COMMON)+bb);
  } else {
    val= vmer32(PF_COMMON+bb);
  };
  sprintf(os,"%s0x%x ", os, val);
};
strcpy(line, os);
}
/*FGROUP PF
 * -------------------------------------------------------------------------------
*/
void ReadPF2str(int circuit) {
char line[160];
ReadPF2str2(circuit, line);
printf("%s\n", line);
};

void decintAB(int intab, char *outs) {
if(intab==0xa) {
  strcpy(outs,"IR1");
} else if(intab==0xc) {
  strcpy(outs,"IR2");
} else {
  sprintf(outs,"0x%1x", intab);
};
}
void decPFC(w32 pfc, char *intA, char *intB, char *delLT, char *delSD){
decintAB(pfc&0xf, intA);
decintAB((pfc>>4)&0xf, intB);
sprintf(delLT,"0x%1x", (pfc>>8)&0xf);
sprintf(delSD,"%4d", (pfc>>12)&0xfff);
}

int dT2ns(int dT, int sd_factor){
int width;
if(dT==0) {
  width= 257;
} else {
  width= dT+1;
};
return(width*(sd_factor+1));
};
int del2ns(int delay, int sd_factor) {
return((delay+1)*(sd_factor+1));
}
//----------------------------------------------------------------------------
void  decPFBL(int level, char AB, int pfb, int pflut, char *line){
int dT, dtns, dsf, delbcs,delay; char ABL[4]; char pflutstr[8];
dT= (pfb>>12) & 0xff;
delay= (pfb>>20) & 0x3ff;
if(AB=='A') {
  sprintf(ABL, "%1dA", level);
  dsf= (pflut>>8)&0x1f;
  sprintf(pflutstr,"0x%2x", pflut&0xff);
} else {
  sprintf(ABL, " B");
  dsf= (pflut>>13)&0x1f;
  pflutstr[0]='\0';
};
dtns= dT2ns(dT, dsf);
delbcs= del2ns(delay, dsf);
//printf("    th1 th2 dT -> [BCs]  del->[BCs] f dsf PLUT       PFBLOCK+PFLUT\n");
sprintf(line,"%s:  %2d  %2d %3d    %4d %4d   %4d %1d  %2d %s",
  ABL, pfb & 0x3f, (pfb>>6) & 0x3f, dT, dtns,
  delay, delbcs, (pfb>>31)&0x1, dsf, 
  pflutstr);
};

/*FGROUP PF 
 *--------------------------------------------------------------------------------
 * */
void DecodePF(char *pfstr) {
int nwds,ix;
w32 pfbA[3], pfbB[3], pflut[3];
w32 pfc[3];
char intA[4], intB[4], delLT[4], delSD[5];
char pfstrdef[160];
char line[160];
//printf("PF definition:\n%s\n",pfstr);
if((strcmp(pfstr,"1")>=0) && (strcmp(pfstr,"4")<=0)) {
  ReadPF2str2(atoi(pfstr), pfstrdef);
} else {
  strcpy(pfstrdef, pfstr);
};
nwds= sscanf(pfstrdef,
  "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
  &pfbA[0], &pfbB[0], &pflut[0],
  &pfbA[1], &pfbB[1], &pflut[1],
  &pfbA[2], &pfbB[2], &pflut[2],
  &pfc[0], &pfc[1], &pfc[2]);
printf(
  "PF def,%d words:\n0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
  nwds,
  pfbA[0], pfbB[0], pflut[0],
  pfbA[1], pfbB[1], pflut[1],
  pfbA[2], pfbB[2], pflut[2],
  pfc[0], pfc[1], pfc[2]);
printf("    intA intB delLT delSD                            PF_COMMON\n");
for(ix=0; ix<3; ix++) {
  decPFC(pfc[ix], intA, intB, delLT, delSD);
  printf("L%1d: %s  %s  %s    %s\n", ix, intA, intB, delLT, delSD);
};
printf("    th1 th2 dT -> [BCs]  del->[BCs] f dsf PLUT       PFBLOCK+PFLUT\n");
for(ix=0; ix<3; ix++) {
  decPFBL(ix, 'A', pfbA[ix], pflut[ix], line);
  printf("%s\n", line);
  decPFBL(ix, 'B', pfbB[ix], pflut[ix], line);
  printf("%s\n", line);
};
}
/* FGROUP PF
 * ---------------------------------------------------------------------
*/
void WritePFcommon(w32 INTa,w32 INTb,w32 Delayed_INT) {

/* This subroutine converts user input for Past-Future protection into
PF_COMMON word*/

w32 PFcommon,Delay_Delayed_INT;
int TL1,TL2;
w32 InteractionA, InteractionB, Delayed_Interaction;

/* This is for preserving some of the original values, it is used for some checks in following code*/
/*InteractionA = INTa;*/
InteractionA = INTa;
InteractionB = INTb;
Delayed_Interaction = Delayed_INT;

/* PF on L0 BOARD ------------------------------------- */
INTa = INTa << 28;
INTb = INTb << 28;
Delayed_INT = Delayed_INT << 28;

INTa = INTa >> 28;
INTb = INTb >> 24;
Delayed_INT = Delayed_INT >> 20;

PFcommon = INTa | INTb | Delayed_INT;
setPF(1,PFcommon);

/* PF on L1 BOARD ----------------------------------------- */

TL1 = getTL1(); /*find out how to read it from timmings.c!*/

Delay_Delayed_INT = TL1 - 3;

Delay_Delayed_INT = Delay_Delayed_INT << 20;
Delay_Delayed_INT = Delay_Delayed_INT >> 8;

PFcommon = INTa | INTb | Delayed_INT | Delay_Delayed_INT;
setPF(2,PFcommon);

/* PF on L2 BOARD ----------------------------------------- */

TL2 = getTL2(); /* find out how to read it from timmings.c! */

Delay_Delayed_INT = TL2 - 3;

/* Check for Delayed_INT value already done in L1 part*/

Delay_Delayed_INT = Delay_Delayed_INT << 20;
Delay_Delayed_INT = Delay_Delayed_INT >> 8;

PFcommon = INTa | INTb | Delayed_INT | Delay_Delayed_INT;
setPF(3,PFcommon);

}

/* FGROUP PF
 * --------------------------------------------------------------------------
*/
void WritePF(w32 icircuit,w32 THa1,w32 THa2,w32 THb1,w32 THb2,int dTa,int dTb,w32 P_signal) {

printf("icircuit=%d THa1=%d dTa=%d\n",icircuit,THa1,dTa);
/* This subroutine converts user input for Past-Future protection into
three VME control words: PFBLOCK_A, PFBLOCK_B, PFLUT */

w32 clk_factorA,clk_periodA,clk_factorB,clk_periodB;
w32 PF_intervalA,PF_intervalB,PF_delayA,PF_delayB;
w32 PFblock_A,PFblock_B,PFlut,No_Delay;
int TL1,TL2;
w32 THa1L0,THa2L0,THb1L0,THb2L0;

/* PF on L0 BOARD ------------------------------------- */

/* Thresholds on L0 should always be smaller by 1 than on L1 and L2
because interaction in question for L0 is not taken into N(dT) */

if (THa1==0) THa1L0 = 0; else THa1L0 = THa1 - 1;
if (THa2==0) THa2L0 = 0; else THa2L0 = THa2 - 1;
if (THb1==0) THb1L0 = 0; else THb1L0 = THb1 - 1;
if (THb2==0) THb2L0 = 0; else THb2L0 = THb2 - 1;

/* Setting Block A */
clk_factorA = dTa/256;
clk_periodA = clk_factorA + 1;
PF_intervalA = (dTa/clk_periodA) - 1;

THa1L0 = THa1L0 << 26;    /* 6 bits, 0 to 63 */
THa2L0 = THa2L0 << 26;    /* 6 bits, 0 to 63 */
PF_intervalA = PF_intervalA << 24;   /* 8 bits, 0 to 255 */

THa1L0 = THa1L0 >> 26;
THa2L0 = THa2L0 >> 20;
PF_intervalA = PF_intervalA >> 12;

PFblock_A = THa1L0 | THa2L0 | PF_intervalA;

/* Setting Block B */
clk_factorB = dTb/256;
clk_periodB = clk_factorB + 1;
PF_intervalB = (dTb/clk_periodB) - 1;

THb1L0 = THb1L0 << 26;    /* 6 bits, 0 to 63 */
THb2L0 = THb2L0 << 26;    /* 6 bits, 0 to 63 */
PF_intervalB = PF_intervalB << 24;   /* 8 bits, 0 to 255 */

THb1L0 = THb1L0 >> 26;
THb2L0 = THb2L0 >> 20;
PF_intervalB = PF_intervalB >> 12;

PFblock_B = THb1L0 | THb2L0 | PF_intervalB;

P_signal = P_signal << 24;    /* 8 bits, 0 to 255 */
clk_factorA = clk_factorA << 27;   /* 5 bits, 0 to 31 */
clk_factorB = clk_factorB << 27;   /* 5 bits, 0 to 31 */

P_signal = P_signal >> 24;
clk_factorA = clk_factorA >> 19;
clk_factorB = clk_factorB >> 14;

PFlut = P_signal | clk_factorA | clk_factorB;

setPFc(1,icircuit,PFblock_A,PFblock_B,PFlut);

/* PF on L1 BOARD ----------------------------------------- */

THa1 = THa1 << 26;    /* 6 bits, 0 to 63 */
THa2 = THa2 << 26;    /* 6 bits, 0 to 63 */

THa1 = THa1 >> 26;
THa2 = THa2 >> 20;

THb1 = THb1 << 26;    /* 6 bits, 0 to 63 */
THb2 = THb2 << 26;    /* 6 bits, 0 to 63 */

THb1 = THb1 >> 26;
THb2 = THb2 >> 20;

TL1 = getTL1(); /*find out how to read it from timmings.c!*/

if (dTa < TL1)
{
   clk_factorA = 2*dTa/256;
   clk_periodA = clk_factorA + 1;
   PF_intervalA = (2*dTa/clk_periodA) - 1;
   PF_delayA = ((dTa+TL1)/clk_periodA) - (PF_intervalA + 1) - 1;
   No_Delay = 0;
} else {
   clk_factorA = (dTa+TL1)/256;
   clk_periodA = clk_factorA + 1;
   PF_intervalA = ((dTa+TL1)/clk_periodA) - 1;
   PF_delayA = 0;
   No_Delay = 1;
}

PF_intervalA = PF_intervalA << 24;
PF_delayA = PF_delayA << 21;
No_Delay = No_Delay << 31;

PF_intervalA = PF_intervalA >> 12;
PF_delayA = PF_delayA >> 1;
No_Delay = No_Delay >> 0;

PFblock_A = THa1 | THa2 | PF_intervalA | PF_delayA | No_Delay;

if (dTb < TL1)
{
   clk_factorB = 2*dTb/256;
   clk_periodB = clk_factorB + 1;
   PF_intervalB = (2*dTb/clk_periodB) - 1;
   PF_delayB = ((dTb+TL1)/clk_periodB) - (PF_intervalB + 1) - 1;
   No_Delay = 0;
} else {
   clk_factorB = (dTb+TL1)/256;
   clk_periodB = clk_factorB + 1;
   PF_intervalB = ((dTb+TL1)/clk_periodB) - 1;
   PF_delayB = 0;
   No_Delay = 1;
}

PF_intervalB = PF_intervalB << 24;
PF_delayB = PF_delayB << 21;
No_Delay = No_Delay << 31;

PF_intervalB = PF_intervalB >> 12;
PF_delayB = PF_delayB >> 1;
No_Delay = No_Delay >> 0;

PFblock_B = THb1 | THb2 | PF_intervalB | PF_delayB | No_Delay;

P_signal = P_signal << 24;    /* 8 bits, 0 to 255 */
clk_factorA = clk_factorA << 27;   /* 5 bits, 0 to 31 */
clk_factorB = clk_factorB << 27;   /* 5 bits, 0 to 31 */

P_signal = P_signal >> 24;
clk_factorA = clk_factorA >> 19;
clk_factorB = clk_factorB >> 14;

PFlut = P_signal | clk_factorA | clk_factorB;

setPFc(2,icircuit,PFblock_A,PFblock_B,PFlut);

/* PF on L2 BOARD ----------------------------------------- */

TL2 = getTL2(); /* find out how to read it from timmings.c! */

if (dTa < TL2)
{
   clk_factorA = 2*dTa/256;
   clk_periodA = clk_factorA + 1;
   PF_intervalA = (2*dTa/clk_periodA) - 1;
   PF_delayA = ((dTa+TL2)/clk_periodA) - (PF_intervalA + 1) - 1;
   printf("0x%x \n", PF_delayA);
   No_Delay = 0;
} else {
   if (dTa > TL2) printf("Note:Prot.int. dTa %d bigger than TL2:%d!\n",dTa,TL2);
   clk_factorA = (dTa+TL2)/256;
   clk_periodA = clk_factorA + 1;
   PF_intervalA = ((dTa+TL2)/clk_periodA) - 1;
   PF_delayA = 0;
   No_Delay = 1;
}

PF_intervalA = PF_intervalA << 24;
PF_delayA = PF_delayA << 21;
No_Delay = No_Delay << 31;

PF_intervalA = PF_intervalA >> 12;
PF_delayA = PF_delayA >> 1;
No_Delay = No_Delay >> 0;

printf("0x%x \n", PF_delayA);
PFblock_A = THa1 | THa2 | PF_intervalA | PF_delayA | No_Delay;

if (dTb < TL2)
{
   clk_factorB = 2*dTb/256;
   clk_periodB = clk_factorB + 1;
   PF_intervalB = (2*dTb/clk_periodB) - 1;
   PF_delayB = ((dTb+TL2)/clk_periodB) - (PF_intervalB + 1) - 1;
   No_Delay = 0;
} else {
   if (dTb > TL2) printf("Note:Prot.int. dTb %d bigger than TL2:%d!\n",dTb,TL2);
   clk_factorB = (dTb+TL2)/256;
   clk_periodB = clk_factorB + 1;
   PF_intervalB = ((dTb+TL2)/clk_periodB) - 1;
   PF_delayB = 0;
   No_Delay = 1;
}

PF_intervalB = PF_intervalB << 24;
PF_delayB = PF_delayB << 21;
No_Delay = No_Delay << 31;

PF_intervalB = PF_intervalB >> 12;
PF_delayB = PF_delayB >> 1;
No_Delay = No_Delay >> 0;

PFblock_B = THb1 | THb2 | PF_intervalB | PF_delayB | No_Delay;

P_signal = P_signal << 24;    /* 8 bits, 0 to 255 */
clk_factorA = clk_factorA << 27;   /* 5 bits, 0 to 31 */
clk_factorB = clk_factorB << 27;   /* 5 bits, 0 to 31 */

P_signal = P_signal >> 24;
clk_factorA = clk_factorA >> 19;
clk_factorB = clk_factorB >> 14;

PFlut = P_signal | clk_factorA | clk_factorB;

setPFc(3,icircuit,PFblock_A,PFblock_B,PFlut);

}

/*FGROUP PF
*/
void WritePFuser(w32 icircuit,w32 THa1,w32 dTa) 
{

//ReadPF();
getprtPF(1); getprtPF(2); getprtPF(3);
WritePFcommon(0xa,0x0,0x0);
//WritePF(w32 icircuit,w32 THa1,w32 THa2,w32 THb1,w32 THb2,int dTa,int dTb,w32 P_signal)
WritePF(icircuit,THa1,(w32)63,(w32)63,(w32)63,dTa,(w32)1,(w32)0x02);
//new:WritePF(icircuit,THa1,(w32)63,(w32)63,(w32)63,dTa,(w32)62,(w32)0x02);
getprtPFc(1,icircuit);
getprtPFc(2,icircuit);
getprtPFc(3,icircuit);
}
///////////////////////////////////////////////////////////////////////
/* Seetings for one blobk
 Input:
        Th1,TH2 - thresholds
        dTa - time interval in BC
        plut - lut for PF signal
 Output:
        PFBLOCK word
        downscale factor for clock
*/
int calcPFBlock(w32 TH1,w32 TH2,int dTa,w32 plut,w32* PFblock,w32* clk_factor)
{
 int ret=0;
 w32 clk_period,No_Delay;
 w32 PF_interval,PF_delay;

 *clk_factor = dTa/256;
 if(*clk_factor>31){
  printf("Error: calcPFBlock clock scale down factor > 31; %i \n",*clk_factor);
  ret++;
 }
 clk_period = *clk_factor + 1;
 if(clk_period==1)PF_interval=dTa ; else PF_interval = (dTa/clk_period) - 1;
 if(PF_interval>255 || PF_interval<0){
  printf("Error: calcPFBlock PF_interval in wrong range; %i \n",PF_interval);  
  ret++;
 }else if(PF_interval==0){
  printf("Warning: calcPFBlock PF_interval=0 \n");
 }
 printf("clk factor=%i clk period=%i \n",*clk_factor,clk_period);
 printf("TH1= %i TH2=%i PF_int=%i \n",TH1,TH2,PF_interval);

 TH1 = TH1 << 0;    /* 6 bits, 0 to 63 */
 TH2 = TH2 << 6;    /* 6 bits, 0 to 63 */
 PF_interval = PF_interval << 12;   /* 8 bits, 0 to 255 */

 PF_delay =0 << 20; // So far only killing up to level
 No_Delay=1 << 31;

 *PFblock = TH1 | TH2 | PF_interval | PF_delay | No_Delay;
 printf("PFBlockA=0x%x \n",*PFblock);
 return ret; 
}
/*
Setting of level ilevel={0,1,2} circuit icircui of pf
This started from Marek's code.
*/
int setPFL(w32 ilevel,int ipf,w32 THa1,w32 THa2,w32 THb1,w32 THb2,int dTa,int dTb,int P_signal)
{
 int ret=0;
 w32 clk_factorA,clk_factorB;
 w32 PFblock_A,PFblock_B,PFlut;

 printf("LEVEL %i\n",ilevel);
 /* Setting Block A */
 printf("Block A------------\n");
 ret += calcPFBlock(THa1,THa2,dTa,P_signal,&PFblock_A,&clk_factorA);

 /* Setting Block B */
 printf("Block B------------\n");
 ret += calcPFBlock(THb1,THb2,dTb,P_signal,&PFblock_B,&clk_factorB);
 /* Setting PFlut  */
 P_signal = P_signal << 0;    /* 8 bits, 0 to 255 */
 clk_factorA = clk_factorA << 8;   /* 5 bits, 0 to 31 */
 clk_factorB = clk_factorB << 13;   /* 5 bits, 0 to 31 */

 PFlut = P_signal | clk_factorA | clk_factorB;
 printf("PFlut=0x%x \n",PFlut);
 setPFc(ilevel+1,ipf,PFblock_A,PFblock_B,PFlut);
 return ret;
}
/*FGROUP PF
*/
int WritePFuserII(w32 Ncoll,w32 dT1,w32 dT2,w32 ipf,w32 plut)
{
 w32 TL1=getTL1();
 w32 TL2L1=getTL2()-TL1;
 // set dT1 for L0 level
 setPFL(0,ipf,Ncoll,0,0,0,dT1,0,plut);   
 //
 if(dT2 < TL1){
   printf("Not ready for dT2 < TL1");
   return 1;
 }else{
   // set L1
   setPFL(1,ipf,Ncoll+1,0,0,0,TL1+1,0,plut);   
 }
 if(dT2 < TL2L1){
   printf("Not ready for dT2 < TL1L2");
   return 1;
 }else{
  // set L2
  setPFL(2,ipf,Ncoll+1,0,0,0,TL2L1+1,0,plut);   
 }
 return 0;
}
 
//==============================================================================================
// Run2 development. Philosophy is to simplify to maximum:
// - use only one branch of PF
// - inta/intb = int1/int2
// - plut = always P1 
/*
*/
/*---------------------------------------------------------------------------
 *  Just set PFCOMMON
 */ 
void setPFCOMMON(w32 ix,w32 INTalut,w32 INTblut,w32 DINTlut,w32 delayDINT)
{
 if(notInCrate(ix)) return;
 w32 bb= BSP*ctpboards[ix].dial;
 if(ix==1) {
    printf("setPFCOMMON: net ready for L0\n");
    return;
 } else {
    w32 pfc=INTalut+(INTblut<<4)+(DINTlut<<8)+(delayDINT<<12);
    vmew32(PF_COMMON+bb, pfc);
 }; 
}
void setPFLUT(w32 ix,w32 ipf,w32 PLUT,w32 scaleA,w32 scaleB)
{
 if(notInCrate(ix)) return;
 w32 bb= BSP*ctpboards[ix].dial;
 if(ix==1) {
    printf("setPFLUT: net ready for L0\n");
    return;
 } else {
    w32 pfl=PLUT+(scaleA<<8)+(scaleB<<13);
    vmew32(PFLUT+bb+4*(ipf-1), pfl);
 };  
}
void setPFBLOCK(w32 ix,w32 ipf,w32 Th, w32 dT,w32 delay,w32 delayflag)
{
 if(notInCrate(ix)) return;
 w32 bb= BSP*ctpboards[ix].dial;
 if(ix==1) {
    printf("setPFBLOCK: net ready for L0 \n");
    return;
 } else {
    w32 pfb=Th+(dT<<12)+(delay<<20)+(delayflag<<31);
    vmew32(PFBLOCK_A+bb+4*(ipf-1), pfb);
 };  
}
/*FGROUP PF
 * Ncol1 -number of collisions in time window dT1 before trigger interaction
 * Ncol2 -number of collisions in time window dT2 after trigger interaction
 * inter - 1=INT1, 2=INT2
 */
int setPFUser(int ipf,w32 Ncol1, w32 dT1,w32 Ncol2,w32 dT2, w32 inter)
{
 w32 TL1=getTL1();
 w32 TL2L1=getTL2()-TL1;
 w32 INTa=0;
 w32 dT=0,delay=0,scale=0,dflag=0;
 if(inter==1)INTa=0xa; else INTa=0xc;
 w32 plut=0xaa; // always P1
 // L0 level
 printf("NO setting for L0 yet \n");
 // L1 level
 if(dT2>TL1){
   dflag=1;
   delay=0;
   scale=1;    
   dT=TL1;
 }else{
   dflag=0;
   delay=TL1/2-1;
   scale=1;
   dT=dT2;
 }
 printf("setPFuser: L1L INTa: 0x%x plut: 0x%x scale: %i dT:%i delay:%i \n",INTa,plut,scale,dT,delay);
 setPFCOMMON(2,INTa,0,0,0);
 setPFLUT(2,ipf,plut,scale,0);
 setPFBLOCK(2,ipf,Ncol1-1,dT,delay,dflag);
 // L2 level
 
 return 0;
}
/*FGROUP PF
 ibl=1..8 LM / 9..12 L0
 scale - 5 bits
 dT - 8 bits
 delay - 9 bits
 */ 
void setLML0PFblock(int ibl,w32 scale,w32 dT,w32 Ncol,w32 delay,w32 delflag)
{
 w32 addr=getLM0_PFBLKad(ibl);
 w32 word=scale+(dT<<5)+(Ncol<<14)+(delay<<22)+(delflag<<31);
 vmew32(addr,word);
}
/*FGROUP PF
 int1 = 0-selected /1=deselected  
 int2 = 0/1
 bcmask 12 bit mask
*/
void setLML0PFINTSEL(w32 ibl,w32 int1,w32 int2,w32 bcmask)
{
 w32 addr=getLM0_PFINTad(ibl);
 w32 word=int1+(int2<<1)+(bcmask<<2);
} 
/*FGROUP PF
 ibl=1..8 LM / 9..12 L0
 scale - 5 bits
 dT - 8 bits
 delay - 9 bits
 int1 = 0/1 
 int2 = 0/1
 bcmask 12 bit mask 
 */ 
void setLML0PF(int ibl,w32 scale,w32 dT,w32 Ncol,w32 delay,w32 delflag,w32 int1,w32 int2,w32 bcmask)
{
 w32 addr=getLM0_PFBLKad(ibl);
 w32 word=scale+(dT<<5)+(Ncol<<14)+(delay<<22)+(delflag<<31);
 vmew32(addr,word);
 setPFINTSEL(ibl,int1,int2,bcmask);
}

