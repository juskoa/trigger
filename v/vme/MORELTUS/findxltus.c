/* print list of base addresses of the LTUs found in the crate
 * cd v/vme/MORELTUS
 * make findxltus.make
 * */
#include "vmewrap.h"
#include "../ltu/ltu.h"

#define linux
#if defined(linux) || defined(AIX)
  #include <stdio.h>
  /* #include <stdlib.h> */
  #include <unistd.h>
  #include <fcntl.h>
  #include <signal.h>
#endif
char BoardSpaceLength[40];
char BoardBaseAddress[40];

/*
#define MINBASETX "0x810000"
#define MINBASE    0x810000
#define BASEDIFF   0x001000
#define BASEDIF8  "0x008000"
#define BASEDIF16 "0x010000"   should be OK with VMECCT/pciBusSpace 123567 */

int quit=0;
int buserr=0;

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
  /*printf("got SIGBUS signal:%d\n", signum); */
  buserr=1;
  break;
default:
  printf("got unknown signal:%d\n", signum);
};
}

int main(int argn, char **argv) {
int ix, rc, mindial, maxdial;
char base[40], blength[40];

if(argn < 3) {
  printf("Usage: finxltu.exe mindial maxdial    (decimal numbers) \n");
  printf("       using       0       2                            \n");
  mindial=0; maxdial=2;
} else {
  mindial= atoi(argv[1]);
  maxdial= atoi(argv[2]);
};
	
signal(SIGBUS, gotsignal); siginterrupt(SIGBUS, 0);
printf("BASE vmever ltuver serial\n");
for(ix=mindial; ix<=maxdial; ix++) {   /* 3 ltus 1,2,3: 1..3 */
  w32 code, vmever, ltuver, serial;
  sprintf(base,"0x81%1x000", ix);
  sprintf(base,"0x816000", ix);
  sprintf(blength,"0x300");
/*  printf("base:%s, blength:%s\n",base, blength); continue; */
  rc= vmxopen(&ix, base, blength);
  if(rc) {
    printf("Error opening ltu %d\n",ix);
  } else {
    code= 0xff&vmxr32(ix, CODE_ADD);
    if(buserr==1) continue;
    if (code== 0x56) {
      vmever= 0xff&vmxr32(ix, VERSION_ADD);
      ltuver= 0xff&vmxr32(ix, LTUVERSION_ADD);
      serial= 0xff&vmxr32(ix, SERIAL_NUMBER);
      printf("base:%s %2x %2x %2x\n",
        base,vmever, ltuver, serial);
    } else {
      /*printf("ltu code 0x56 expected, but read: %x\n", code); */
    };
  };
};
for(ix=mindial; ix<=maxdial; ix++) {   /* 3 ltus 1,2,3 */
  rc=vmxclose(ix);
  printf("vmxclose(%d) rc:%d\n",ix,rc);
};
}

