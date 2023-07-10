/*BOARD corde 0x7000000 0x7fc00 A32  */
/* base: 0xML00000   M=MSB L=LSB */
#include <string.h>
#include <stdio.h>
#include <unistd.h>   //usleep
#include "vmewrap.h"
#include <stdlib.h>
//#include "vmeblib.h"

/*REGSTART32 */
#define RESET 0x24
#define ORBMAIN 0x7fbb4
/* 0: ORBMAIN +4:ORB2 +8:ORB1 
+0xC:BCMAIN +0x10:BCREF +0x14:BC2 +0x18:BC1
delays: 
from  -5120ps  0x0
0         0ps  0x200
to    +5110ps  0x3ff
*/

/*REGEND */

extern int quit;
int shift=100; int ministep=20; int sleep_us=0;
const char *dnames[8]={ "none", "ORBMAIN", "ORB2", "ORB1", "BCMAIN",
  "BCREF", "BC2", "BC1"};

w32 getreg(w32 regad) {
w32 val;
//val=vmer32(regad) & 0x3ff;
val=vmer32(regad);
return(val);
}

/*FGROUP
Define shift (in ps) in CORDE board set later by shift_up/shift_down.
The delay is increased/decreased in ministeps by successive VME writes.
The time between 2 successive writes is set by sleep_us.
shift_ps: shift in ps
ministep_ps: ministep delay in ps 
sleep_us: in microsecs. Meaningfull values: 0, >20
*/
void set_shift(int shift_ps, int ministep_ps, int sleepus) {
int st,minist;
st= shift_ps/10; minist= ministep_ps/10;
sleep_us= sleepus;
if((ministep_ps > shift_ps) ||
   (ministep_ps<0) ||
   (shift_ps<0)
  ) 
{
  //printf("ministep has to be >= step, both have to be >=0\n");
  printf("ministep has to be <= shift\n");
} else {
  if(ministep_ps != (minist*10)) {
    printf("Using ministep:%d\n",minist*10);
  };
  shift= st*10;
  ministep= minist*10;
};
printf("shift:%dps ministep:%dps sleep_us:%d\n", shift, ministep, sleep_us);
}
/*FGROUP
reset Corde board: writing 1 and 0 to RESET register.
*/
void reset() {
vmew32(RESET,0);
vmew32(RESET,1); vmew32(RESET,0);
}
/*FGROUP
read all delay registers.
*/
void readall() {
int ix;
printf("ministep: %d shift: %d sleep:us:%d\n", ministep, shift, sleep_us);
for(ix=1; ix<=7; ix++) {
  w32 regad; w32 val;
  regad=(ix-1)*4 + ORBMAIN; val= getreg(regad);
  printf("%8s: %x =%6d  *10ps\n", dnames[ix], val, val);
};
}
/*FGROUP
set delay to ps
delay: register to be changed
1: ORBMAIN     4:BCMAIN       7:BC1
2:ORB2         5:BCREF
3:ORB1         6:BC2

ps: 
0     0x0     -5120ps    minimal shift
5120  0x200       0ps
10230 0x3ff   +5110ps    maximal shift!

An example from 2013 pA run (after pb-p changed back to p-p):
shifter reported too big shift: -1.5ns

BC1 (7) changed from 8010ps (0x321) -> 9600ps
i.e. +1590ns
after ~ 1-2 minutes, clock shift reached +0.1ns
*/
void set(int delay, int ps) {
w32 regad, val;
if((delay<1)||(delay>7)){printf("Bad delay (1..7 allowed)\n");return;};
regad=(delay-1)*4 + ORBMAIN; val= ps/10;
vmew32(regad, val);
printf("%s set to %d(0x%x) (read back:%x)\n", 
  dnames[delay], val,val, getreg(regad));
}
/*FGROUP
regix: 5 (REF -debug) or 7: BC1 
The clock is shifted up by 'shift_ps' value set in set_shift
*/
int shift_up(int regix) {
int val,lastval; w32 regad, baseval;
regad= (regix-1)*4 + ORBMAIN;
baseval= vmer32(regad);
val=baseval; lastval= baseval+shift/10;
while(1) {
  val= val+ministep/10;
  if(val > 1023) val=1023;
  if(val < 0) val=0;
  vmew32(regad, (w32) val);
  //val=getreg(regad);
  if(sleep_us>0) usleep(sleep_us);
  if((val>=lastval) || (val>=1023) || (val<=0) ) break;
};
return(val);
}
/*FGROUP
regix: 5 (REF -debug) or 7: BC1 
The clock is shifted down by 'shift_ps' value set in set_shift
*/
int shift_down(int regix) {
int val,lastval; w32 regad, baseval;
regad=(regix-1)*4 + ORBMAIN;
baseval= vmer32(regad);
val=baseval; lastval= baseval-shift/10;
while(1) {
  val= val-ministep/10;
  if(val > 1023) val=1023;
  if(val < 0) val=0;
  vmew32(regad, (w32) val);
  if(sleep_us>0) usleep(sleep_us);
  if((val<=lastval) || (val<=ministep/10) || (val<=0) ) break;
};
return(val);
}

/*FGROUP
Increase successively delay by defined shift/ministep (set by set_shift). 
See shift_up/shift_down  -applied until 1023 or 0
Allow ms miliseconds between steps.
delay: register to be changed
1:ORBMAIN 
2:ORB2 
3:ORB1 
4:BCMAIN 
5:BCREF 
6:BC2 
7:BC1   <- this one (CORDE_DELREG) used for ALICE clock shift before rf2ttc board
*/
void up(int delayreg, int ms) {
w32 regad;
int newval;
if((delayreg<1)||(delayreg>7)){printf("Bad delayreg (1..7 allowed)\n");return;};
regad=(delayreg-1)*4 + ORBMAIN;
newval=getreg(regad);
printf("%s is %dps.\n", dnames[delayreg], newval*10); 
printf("Ramping up by %d/%d ps (%dms between steps, %dus between ministeps)\n", shift, ministep, ms, sleep_us);
while(1) {
  newval= shift_up(regad);   // increase by shift
  if(newval >= 1023) break;
  if(newval <= 0) break;
  usleep(ms*1000);
};
printf("%s set to %dps\n", dnames[delayreg], newval*10);
}

void initmain() {
shift=100; ministep=20; sleep_us=0;
printf("shift:%dps ministep:%dps sleep_us:%d us\n", shift, ministep, sleep_us);
}
void boardInit() {
printf("boardInit called...\n");
}
void endmain() {
printf("endmain called...\n");
}

