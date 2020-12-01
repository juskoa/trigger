/*
g++ -g -c -Wall -I$VMEBDIR/vmeblib -I/ATLAS/tdaq_drivers cordel.c
g++ cordel.o  -L$VMEBDIR/vmeblib/linux_c -lvmeb -lpthread -L/ATLAS/lib -lvme_rcc -lrcc_error -lio_rcc -lcmem_rcc -lDFDebug -lm -o cordel.exe

cordel r 1..7
       s 1-7 0-1023
register:
1: ORBMAIN     4:BCMAIN       7:BC1
2:ORB2         5:BCREF
3:ORB1         6:BC2

ps: 
0     0x0     -5120ps    minimal shift
5120  0x200       0ps
10230 0x3ff   +5110ps    maximal shift!
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>     // nanosleep
#include "vmewrap.h"

//#include "vmeblib.h"

#define CORDE_ORBMAIN 0x7fbb4
#define CORDE_RESET 0x24

int vsp=-1;
void prtHelp() {
printf("Usage:\n\
Read content of register ( 1 to 7):\n\
cordel r 1-7\n\
Set register ( 1 to 7):\n\
cordel s 1-7 0-1023\n\
Shift  register by +- N     where N is a shift in 10ps units\n\
cordel m 1-7 N\n\
\n");
}
/*shift: in 10ps units, should be in interval -150..+150 (+-1500ps)
  del: normally 7 (BC1 delay)
  rc: new value set in corde register
      0xffffffff -vme open error, bad value (shift)
      0xfffffffe -bad input (too big shift)
  origval: original value upon the corde_shift call
*/
#define corde_sleep_us 10
#define corde_ministep 5
w32 corde_shift(int del, int shift, int *origval) {  
int rc=0xffffffff; 
  int sig; w32 base,adr,lastval;
  adr= (del-1)*4 + CORDE_ORBMAIN; 
  base= vmxr32(vsp, adr); 
  *origval= base;
  if((shift <-150) || shift>150) {
      sig=0; rc=0xfffffffe;
  } else if(shift<0) {
    sig=-1;
    if((int)base < (-shift)) {
      sig=0; rc=0xfffffffe;
    };
  } else {
    sig=1;
    if((base+shift) > 1023 ) {
      sig=0; rc=0xfffffffe;
    };
  };
  if(sig!=0) {
    w32 val;
    val= base; lastval= base + shift;
    while(1) {
      val= val+sig*corde_ministep;
      if(((sig==1) && (val>lastval)) ||
         ((sig==-1) && (val<lastval))) val=lastval;
      if(val>1023) {
        printf("Error:corde_shift: cannot write %d>1023 into CORDE\n",val);
        break;
      };
      //printf("corde_shift: writing %d\n",val);
      vmxw32(vsp, adr, val);
      if(corde_sleep_us>0) usleep(corde_sleep_us);
      if((val==lastval)) break;
    };
    rc=lastval;
  };
return(rc);
}
int main(int argc, char **argv)  {
int rc, del;
w32 addr;
rc= vmxopenam(&vsp, (char *)"0x7000000", (char *)"0x7fc00", (char *)"A32");
if(rc!=0) exit(8);
if( (argc < 3) || (argc>4) ) {
  prtHelp(); exit(8);
};
if(strcmp(argv[1], "r")==0) {
  del=7;
  del= atoi(argv[2]);
  addr= (del-1)*4 + CORDE_ORBMAIN;
  printf("%d %d\n", del, vmxr32(vsp, addr));
} else if(strcmp(argv[1], "s")==0) {
  w32 val;
  del= atoi(argv[2]);
  val= atoi(argv[3]);
  addr= (del-1)*4 + CORDE_ORBMAIN;
  vmxw32(vsp, addr, val);
  printf("%d %d ok\n", del, val);
} else if(strcmp(argv[1], "m")==0) {   // move by max +-1500ps
  int origval, shift;
  del= atoi(argv[2]);
  shift= atoi(argv[3]);
  rc= corde_shift(del, shift, &origval);
  printf("%d %d\n", origval, rc);
} else {
  prtHelp(); exit(8);
};

rc= vmxclose(vsp);
//printf("vmxclose rc:%d\n", rc);
exit(0);
}
