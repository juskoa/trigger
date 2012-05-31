/* print list of base addresses of the LTUs found in the crate
 * cd v/vme/MORELTUS
 * make -f findltus.make
 * */
#include "vmewrap.h"
#include "../ltu/ltu.h"

#include <stdio.h>
#include <stdlib.h>  //atoi
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

char BoardSpaceLength[40];
char BoardBaseAddress[40];

#define MINBASETX "0x810000"
#define MINBASE    0x810000
#define BASEDIFF   0x001000
#define BASEDIF8  "0x008000"
#define BASEDIF16 "0x010000"   /* should be OK with VMECCT/pciBusSpace 123567 */

int quit=0;

void vmeish();
void gotsignal(int signum) {
switch(signum) {
case SIGUSR1:
  signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
  printf("got SIGUSR1 signal:%d\n", signum);
  quit=signum;
  break;
case SIGBUS:
/*  vmeish(); */
  printf("got SIGBUS signal:%d\n", signum);
  break;
default:
  printf("got unknown signal:%d\n", signum);
};
}

int main(int argn, char **argv) {
int ix, rc;
w32 curdif=0;
signal(SIGBUS, gotsignal); siginterrupt(SIGBUS, 0);
rc= vmeopen(MINBASETX, BASEDIF16);
printf("BASE vmever ltuver serial\n");
for(ix=0; ix<16; ix++) {   /* max. 16 ltus in crate */
  w32 code, vmever, ltuver, serial;
  code= 0xff&vmer32(CODE_ADD+curdif);
  if (code= 0x56) {
    vmever= 0xff&vmer32(VERSION_ADD+curdif);
    ltuver= 0xff&vmer32(LTUVERSION_ADD+curdif);
    serial= 0xff&vmer32(SERIAL_NUMBER+curdif);
    printf("0x%x %2x %2x %2x\n",
      MINBASE+curdif,vmever, ltuver, serial);
  };
  curdif= curdif + BASEDIFF;
};
rc=vmeclose();
}


