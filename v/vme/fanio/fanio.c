/*BOARD fanio 0xc00000 0x200 */
#include <stdio.h>
#include <unistd.h>
/*
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
*/
#include "vmewrap.h"
#include "fanio.h"

extern int quit;
//extern char BoardBaseAddress[];
/*FGROUP TOP 
Input: 
channel: number 0-23   or    24: set the dealy for all the channels
delay: delay in ns, value for delay register willl be 
       calculated: value= delay/12.5 
Operation:
- set Bit [2] of CONTROL_REG (ClockPhaseEnabled)  to 1 !
- set channel(s)
*/
void setdelay(int channel, int delay) {
w32 del,cr;
del= delay/12.5;
cr= vmer32(CONTROL_REG) | 0x2; vmew32(CONTROL_REG, cr);
if(channel==24) {
  int channex;
  for(channex=0; channex<24; channex++) {
    vmew32(DELAY0+(channex<<4), del);
  };
  printf("All channels set to %d ns = %x\n", delay, del);
} else {
  vmew32(DELAY0+(channel<<4), del);
  printf("Channel %d is set to %d ns = 0x%x\n", channel, delay, del);
};
}

/*FGROUP TOP
FANOUT scan.
Input: Common fanout input is connected to L0 source.
Operation:
measure number of L0 triggers for 4 phases
1st measurement:
scan()
phase: 0 L0s: 176075
phase: 1 L0s: 176111
phase: 2 L0s: 176111
phase: 3 L0s: 176111
scan()
phase: 0 L0s: 176068
phase: 1 L0s: 176111
phase: 2 L0s: 176117
phase: 3 L0s: 176105

clock delayed by 3 ns:
phase: 0 L0s: 139155
phase: 1 L0s: 176111
phase: 2 L0s: 177135
phase: 3 L0s: 176111
scan()
phase: 0 L0s: 171364
phase: 1 L0s: 176147
phase: 2 L0s: 176075
phase: 3 L0s: 176111
*/
void scan() {
int ix;
for(ix=0; ix<4; ix++) {     // for 4 clock phases
  w32 l0s, phase;
  phase=ix;
  vmew32(CONTROL_REG, (phase | 0x4) );
  vmew32(VME_RESET, DUMMY );
  usleep(1000000);
  l0s= vmer32(L0_COUNTER);
  printf("phase: %d L0s: %d\n", phase, l0s);
};
}

void initmain() {   /* called once, at the very beginning */
}
void endmain() {   /* called once, at the very end */
}
void boardInit() {   /* called once, after initmain, if -noboardInit 
                        was not present in Command line */
vmew32(CONTROL_REG,0);   // disbale TEST BIT !
}
/*ENDOFCF
*/

