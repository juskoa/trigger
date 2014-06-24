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
w32 getLM0addr(w32 addr) {
if((addr>=L0_INTERACT1) && (addr<=ALL_RARE_FLAG)) {
  if(l0C0()) {
    return(addr-L0LM0DIFF);
  } else {
    return(addr);
  };
} else {
  printf("getLM0addr: internal error for addr:0x%x\n", addr);
  return(0);
}
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

