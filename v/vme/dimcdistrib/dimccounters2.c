/* dimccounters2.c -an example of dim client reading more CTP counters */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif

#include "ctpcounters.h"
typedef unsigned int w32;
 
unsigned int cnts[NCOUNTERS];
unsigned int cntsFailed=0xdeaddeed;

#define NCS 3  // 3 values per board: elapsed time, temperature, voltages
#define NBOARDS 11 //BUSY, L0,1,2,INT,FO1-FO6
typedef struct {
  int reladdr;  // in array coming from CTPDIM server
  w32 prevcs;   // previous value
  w32 currcs;   // current value
} Tcnt1;

Tcnt1 orbit;
typedef struct {
  char name[8];
  Tcnt1 cs[NCS];
} Tboard;

Tboard boards[NBOARDS];

/*------------------------------------------------------------- dodif32()
Substract 2 32 bits values (representing counters)
*/
w32 dodif32(w32 before, w32 now) {
w32 dif;
if(now >= before) dif= now-before;
else dif= now+ (0xffffffff-before) +1;
//if(DBGcnts) printf("dodif32:%d\n", dif);
return(dif);
}
/*------------------------------------------------------------- dodif24()
Substract 2 24 bits values (representing orbit)
*/
w32 dodif24(w32 before, w32 now) {
w32 dif;
if(now >= before) dif= now-before;
else dif= (0x1000000-before) + now;
//if(DBGcnts) printf("dodif24:%d\n", dif);
return(dif);
}
/*--------------------------------*/ void vme2volt(w32 vme, char *mv4str) {
float volt5,volt3_3,volt1_5,volt5b;
 volt5=(vme & 0xff)*23.725;
 volt5b=((vme & 0xff000000)>>24)*23.725;
 volt3_3=((vme & 0xff00)>>8)*23.725;
 volt1_5=((vme & 0xff0000)>>16)*12.941;
 sprintf(mv4str, "%4.0f %4.0f %4.0f %4.0f",volt5,volt3_3,volt1_5,volt5b);
}
/*---------------------------------------------------*/ void initcntsstr() {
int ix;
orbit.reladdr= CSTART_SPEC+2;  // Orbit
strcpy(boards[0].name, "busy"); boards[0].cs[0].reladdr= CSTART_BUSY+39;
strcpy(boards[1].name, "l0"  ); boards[1].cs[0].reladdr= CSTART_L0+13; 
strcpy(boards[2].name, "l1"  ); boards[2].cs[0].reladdr= CSTART_L1+5; 
strcpy(boards[3].name, "l2"  ); boards[3].cs[0].reladdr= CSTART_L2+5;
strcpy(boards[4].name, "int" ); boards[4].cs[0].reladdr= CSTART_INT+2;
strcpy(boards[5].name, "fo1" ); boards[5].cs[0].reladdr= CSTART_FO+0; // 6 FOs
strcpy(boards[6].name, "fo2" ); boards[6].cs[0].reladdr= CSTART_FO+1*NCOUNTERS_FO+0;
strcpy(boards[7].name, "fo3" ); boards[7].cs[0].reladdr= CSTART_FO+2*NCOUNTERS_FO+0;
strcpy(boards[8].name, "fo4" ); boards[8].cs[0].reladdr= CSTART_FO+3*NCOUNTERS_FO+0;
strcpy(boards[9].name, "fo5" ); boards[9].cs[0].reladdr= CSTART_FO+4*NCOUNTERS_FO+0;
strcpy(boards[10].name, "fo6" ); boards[10].cs[0].reladdr= CSTART_FO+5*NCOUNTERS_FO+0;
for(ix=0; ix<11; ix++) {
  boards[ix].cs[1].reladdr= CSTART_SPEC+3+2*ix;  // temperaure
  boards[ix].cs[2].reladdr= CSTART_SPEC+3+2*ix+1;  // volts
};
}
/*-----------------------*/ void gotcnts(void *tag, void *buffervoid, int *size) {
int ixb,ix; w32 *buffer= (w32 *) buffervoid;
printf("gotcnts tag:%d size:%d\n", *(int *)tag, *size );
if(*size != 4*NCOUNTERS) {
  printf("error in gotcnts. First word of message (if any):0x%x\n",
    buffer[0]);
  return;
};
orbit.currcs= buffer[orbit.reladdr];
printf("Orbit: %d Orbit-previousOrbit:%d\n", orbit.currcs, 
  dodif24(orbit.prevcs, orbit.currcs));
printf("board ixE ixT ixV  elapsed/diff[s]      temp mV1  mV2  mV3  mV4\n");
for(ixb=0; ixb<NBOARDS; ixb++) {
  char milivolts4[24];
  for(ix=0; ix<NCS; ix++) {
    boards[ixb].cs[ix].currcs= buffer[boards[ixb].cs[ix].reladdr];
  };
  printf("%4s  ", boards[ixb].name);
  vme2volt(boards[ixb].cs[2].currcs, milivolts4);
  printf("%3d %3d %3d %8x/%10.4f %5d %s\n", 
    boards[ixb].cs[0].reladdr, boards[ixb].cs[1].reladdr, 
    boards[ixb].cs[2].reladdr,
    boards[ixb].cs[0].currcs, 
    dodif32(boards[ixb].cs[0].prevcs, boards[ixb].cs[0].currcs)/2500000.,
    boards[ixb].cs[1].currcs, milivolts4);
}; printf("\n");
orbit.prevcs= orbit.currcs;
for(ixb=0; ixb<NBOARDS; ixb++) {
  for(ix=0; ix<NCS; ix++) {
    boards[ixb].cs[ix].prevcs= boards[ixb].cs[ix].currcs;
  };
};
}

int main(int argc, char **argv) {
int inforc;
inforc= dic_info_service("CTPDIM/MONCOUNTERS", MONITORED, 0, cnts,4*(NCOUNTERS),
  gotcnts, 137, &cntsFailed, 4); 
printf("CTPDIM/MONCOUNTERS service id:%d\n", inforc);
initcntsstr();
while(1) {
  sleep(100);
};
dic_release_service(inforc);
return(0);
} 
