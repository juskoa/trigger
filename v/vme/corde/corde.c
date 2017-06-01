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
int step=100; int ministep=50; int sleep_us=0;
char *dnames[8]={ "none", "ORBMAIN", "ORB2", "ORB1", "BCMAIN",
  "BCREF", "BC2", "BC1"};

w32 getreg(w32 regad) {
w32 val;
//val=vmer32(regad) & 0x3ff;
val=vmer32(regad);
return(val);
}

/*FGROUP
Define 1 step. Step is time delay (in ps), set in CORDE board
by increasing/decreasing delay in ministeps by successive VME writes.
The time between 2 successive writes is set by sleep_us.
step_ps: step in ps
ministep_ps: ministep delay in ps 
sleep_us: in microsecs. Meaningfull values: 0, >20
sign: 0 - positive, 1 - negative
*/
void set_step(int step_ps, int ministep_ps, int sleep_us, int sign) {
int st,minist;
st= step_ps/10; minist= ministep_ps/10;
if((ministep_ps > step_ps) ||
   (ministep_ps<0) ||
   (step_ps<0)
  ) 
{
  //printf("ministep has to be >= step, both have to be >=0\n");
  printf("ministep has to be <= step\n");
} else {
  if(ministep_ps != (minist*10)) {
    printf("Using ministep:%d\n",minist*10);
  };
  step= st*10;
  ministep= minist*10;
  if(sign){
   step=-step;
   ministep=-ministep;
  }
};
printf("step:%dps ministep:%dps\n", step, ministep);
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
/* baseval: current delay set in register delreg
*/
int step_up(w32 regad, w32 baseval) {
int val,lastval;
val=baseval; lastval= baseval+step/10;
while(1) {
  val= val+ministep/10;
  if(val > 1023) val=1023;
  if(val < 0) val=0;
  vmew32(regad, (w32) val);
  if(sleep_us>0) usleep(sleep_us);
  if((val>=lastval) || (val>=1023) || (val<=0) ) break;
};
return(val);
}

/*FGROUP
Increase successively delay by defined step/ministep (set by set_step). 
See step_up  -applied until 1023 or 0
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
void up(int delay, int ms) {
w32 regad;
int newval;
if((delay<1)||(delay>7)){printf("Bad delay (1..7 allowed)\n");return;};
regad=(delay-1)*4 + ORBMAIN;
newval=getreg(regad);
printf("%s is %dps.\n", dnames[delay], newval*10); 
printf("Ramping up by %d/%d ps (%dms between steps, %dus between ministeps)\n", step, ministep, ms, sleep_us);
while(1) {
  newval=step_up(regad,(w32) newval);
  if(newval >= 1023) break;
  if(newval <= 0) break;
  usleep(ms*1000);
};
printf("%s set to %dps\n", dnames[delay], newval*10);
}

void initmain() {
step=100; ministep=20; sleep_us=0;
printf("step:%dps ministep:%dps sleep_us:%d us\n", step, ministep, sleep_us);
}
void boardInit() {
printf("boardInit called...\n");
}
void endmain() {
printf("endmain called...\n");
}

