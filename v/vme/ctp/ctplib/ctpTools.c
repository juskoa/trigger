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

