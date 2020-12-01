/*
g++ -g -c -Wall -I/local/trigger/v/vmeb/vmeblib -I/usr/local/include cordel.c
g++ cordel.o  -L/local/trigger/v/vmeb/vmeblib/linux_c -lvmeb -lpthread -L/lib/modules/daq -lvme_rcc -lrcc_error -lio_rcc -lcmem_rcc -lDFDebug -lm -o cordel.exe

cordel r 1..7
       s 1-7 0-1023
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

void prtHelp() {
printf("Usage:\n\
Read content of register ( 1 to 7):\n\
cordel r 1-7\n\
Set register ( 1 to 7):\n\
cordel s 1-7 0-1023\n");
}
int main(int argc, char **argv)  {
int rc, del,vsp;
w32 addr;
vsp=0;
rc= vmxopenam(&vsp, "0x7000000", "0x7fc00", "A32");
if(rc!=0) exit(8);
if( (argc < 3) || (argc>4) ) {
  prtHelp(); exit(8);
};
if(strcmp(argv[1], "r")==0) {
  del=7;
  del= atoi(argv[2]);
  addr= (del-1)*4 + CORDE_ORBMAIN;
  printf("%d %d\n", del, vmer32(addr));
} else if(strcmp(argv[1], "s")==0) {
  w32 val;
  del= atoi(argv[2]);
  val= atoi(argv[3]);
  addr= (del-1)*4 + CORDE_ORBMAIN;
  vmew32(addr, val);
  printf("%d %d ok\n", del, val);
} else {
  prtHelp(); exit(8);
};

rc= vmxclose(vsp);
//printf("vmxclose rc:%d\n", rc);
exit(0);
}
