#include <stdio.h>
//#include <stdlib.h>
#include "vmewrap.h"
#include "ltu.h"
/*FGROUP ConfiguratioH
Enable/Disable BUSY inputs by setting BUSY_ENABLE bits:
binputs  BUSY1    BUSY2
0        disabled disabled 
1        enabled  disabled 
2        disabled enabled  
3        enabled  enabled  

BUSY_STATUS bits: 
0 BUSY1 enabled
1 BUSY2 enabled
2,3 L1,L2 FIFO status
4 SW BUSY status
5,6 BUSY1,BUSY2 input
7 BUSY output
*/
void setBUSY(w32 binputs) {
vmew32(BUSY_ENABLE, binputs);
/*printf("BUSY_STATUS:0x%x\n",vmer32(BUSY_STATUS)); */
}
/*
A,B: 2x5bits for Scope A,B outputs
setAB(23,23) -no output selected
*/
void setAB(w32 A, w32 B) {
vmew32(SCOPE_SELECT, (B<<5) | A);
}
