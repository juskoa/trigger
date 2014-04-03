#include <stdio.h>
#include <string.h>
#include "vmewrap.h"
#include "ltu.h"
int main(int argc, char **argv) {
int rccret;
char BaseAddress[12]="0x811000";
char SpaceLength[]="0x800";
if( argc<2 ) {
  printf("Base address required, i.e. when LTU dial set to 0:\n\
printcounters 0x810000 \n\
prints abs. values of %d LTU counters to stdout. \n\
Counter names are in $VMECFDIR/dimcdistrib/ltunames.sorted2 .\n\
\n\
This should not be started in parallel with ltuproxy\n", LTUNCOUNTERS);
  return(4);
};
strcpy(BaseAddress, argv[1]);
rccret= vmeopen(BaseAddress,SpaceLength);
if(rccret!=0) {
  printf("vmeopen rc:%d\n", rccret); return(8);
};
getCounters(LTUNCOUNTERS, 0, 0);
rccret= vmeclose();
return(0);
}
