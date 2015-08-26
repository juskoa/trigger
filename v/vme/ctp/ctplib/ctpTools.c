#include <stdio.h>
#include "vmewrap.h"
#include "vmeblib.h"
#include "ctp.h"
#include "ctplib.h"
void DAQonoff(int daqon) {
w32 ddlemu;
if((daqon==0) ||(daqon==0xb)) {
  vmew32(INT_DDL_EMU, daqon);
} else {
  ddlemu= vmer32(INT_DDL_EMU);
  printf("INT_DDL_EMU(low 4 bits):%x (0: daq active, 0xb: daq off)\n", ddlemu&0xf);
  printf("INT_DDL_EMU(all bits):%x \n", ddlemu);
  printf("6:DDL fiLF, 5:DDL fiBEN, 4:DDLfiDIR, 3:EnableDDLemulation \n\
2..0 -emulated 6..4 signals \n");
};
}
// classn:1..100  mskbit: 1: disable 0: enable
void setClaMask(int classn, int mskbit) {
w32 mskCLAMASK=0x80000000; int bb;
bb= 4*classn;
if(l0C0()) {
  w32 val; 
  mskCLAMASK= 0x800000;   // and use L0_VETOr2
  val= vmer32(L0_VETOr2+bb); 
  val= val & (~mskCLAMASK); val= val | (mskbit<<23);
  vmew32(L0_VETOr2+bb, val);
} else {
  vmew32(L0_MASK+bb, mskbit);
}
}
w32 getRATE_MODE() {
w32 adr;
if(l0C0()) {
  adr= RATE_MODElm0;
} else {
  adr= RATE_MODE;
};return(adr);
}
w32 getLM0PFad(w32 addr) {
if((addr>=PF_COMMON) && (addr<=PFLUT)) {
  if(l0C0()) {
    return(addr-L0LM0PFDIFF);
  } else {
    return(addr);
  };
} else {
  printf("getLM0PFad: internal error for addr:0x%x\n", addr);
  return(0);
}
}
/* reg: 1..8
1..4: get address of L0F1..4 (LUT table for 8inputs L0F implemented from LM0 c606
5..8: get address of LMF1..4
*/
w32 getLM0_F8ad(int reg) {
w32 rc;
if( (reg<1) || (reg>8)) {
  printf("getLM0F8ad: internal error for addr:0x%x\n", reg);
  return(0);
};
switch(reg) { 
  case 1: rc= 0x9214; break;   //L0_F
  case 2: rc= 0x9218; break;
  case 3: rc= 0x923c; break;
  case 4: rc= 0x9240; break;
  case 5: rc= 0x9274; break;   // LM_F
  case 6: rc= 0x9278; break;
  case 7: rc= 0x927c; break;
  case 8: rc= 0x9300; break;
  default:
    printf("getLM0F8ad: internal error for addr:0x%x\n", reg);
    rc=0;
};
return(rc);
}
/*
 * inter=1 LMINT1
 * inter=2 LMINT2
 * inter=3 LMINTT
 * inter=4 L0INT1
 * inter=5 L0INT2
 * inter=6 L0INTT
*/
w32 getLM0_INTad(int inter)
{
 w32 rc;
 if( (inter<1) || (inter>6)) {
  printf("getLM0_INTad: internal error for inter:0x%x\n", inter);
  return(0);
 };
 switch(inter){
  case 1: rc=0x9204; break;
  case 2: rc=0x9208; break;
  case 3: rc=0x920c; break;
  case 4: rc=0x93c4; break;
  case 5: rc=0x93c8; break;
  case 6: rc=0x93cc; break;
 }
 return rc;
}
/* 
 * inter 1..8  LM_PF_INT_SEL_1..8
 * inter 9..12 L0_PF_INT_SEL_1..4
*/ 
w32 getLM0_PFINTad(int inter)
{
 w32 rc;
 if( (inter<1) || (inter>12)) {
  printf("getLM0_PFINTad: internal error for inter:0x%x\n", inter);
  return(0);
 };
 if(inter<9) rc=0x9000+(0x58+inter-1)*4;
 else rc=0x9000+(0xed+inter-9)*4;
 return rc;
}
/* 
 * inter 1..8  LM_PF_BLOCK_1..8
 * inter 9..12 L0_PF_BLOCK_1..4
*/ 
w32 getLM0_PFBLKad(int inter)
{
 w32 rc;
 if( (inter<1) || (inter>12)) {
  printf("getLM0_PFBLKad: internal error for inter:0x%x\n", inter);
  return(0);
 };
 if(inter<9) rc=0x9000+(0xc5+inter-1)*4;
 else rc=0x9000+(0xe9+inter-9)*4;
 return rc;
}


 
