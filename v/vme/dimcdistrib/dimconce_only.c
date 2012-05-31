/* dimccounters.c -an example of dim client reading CTP/LTU counters */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include <dic.h>
#include "ctpcounters.h"
typedef unsigned int w32;
 
#define LTUNCOUNTERSall 26;
unsigned int cnts[NCOUNTERS];       // allocate maximum (i.e. for CTP)
int realcnts=NCOUNTERS;
unsigned int cntsFailed=0xdeaddeed;

typedef struct {
  int reladdr;
  w32 prevcs;
  w32 currcs;
} Tcnt1;

#define NCS 6   // elapsed time for BUSY, L0,1,2,FO1, FO3
Tcnt1 cs[NCS];

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
/*---------------------------------------------------*/ void initcntsCTP() {
/* let's take time counters from BUSY, L0,L1,L2 FO1, FO2 board,
so we can check if difference is 60 seconds (server is sending
counters every minute) */
cs[0].reladdr= CSTART_BUSY+39;
cs[1].reladdr= CSTART_L0+13; 
cs[2].reladdr= CSTART_L1+5; 
cs[3].reladdr= CSTART_L2+5;
cs[4].reladdr= CSTART_FO+0;
cs[5].reladdr= CSTART_FO+1*NCOUNTERS_FO+0;
}
/*---------------------------------------------------*/ void initcntsLTU() {
/* let's take timers: time, in_busy1, in_busy2, sbusy
   and counters:      busy_ts, l2a_strobe
*/
cs[0].reladdr= 0;
cs[1].reladdr= 1;
cs[2].reladdr= 2;
cs[3].reladdr= 3;
cs[4].reladdr= 9;
cs[5].reladdr= 19;
};
/*-----------------------*/ void gotcnts(int *tag, w32 *buffer, int *size) {
int ix;
printf("\ngotcnts tag:%d size:%d\n", *tag, *size );
if(*size != 4*realcnts) {
  printf("error in gotcnts. First word of message (if any):0x%x\n",
    buffer[0]);
  return;
};
printf(" addr    0x abs           diff[secs]\n");
for(ix=0; ix<NCS; ix++) {
  cs[ix].currcs= buffer[cs[ix].reladdr];
  printf(" %3d %8x %10.4f\n", cs[ix].reladdr, cs[ix].currcs, 
    dodif32(cs[ix].prevcs, cs[ix].currcs)/2500000.);
  cs[ix].prevcs= cs[ix].currcs;
};
printf("\n");
}

/*----------------------------------------------------------------- main() 
*/
int main(int argc, char **argv) {
int inforc;
char DETNAME[10];
char DNMON[30];
char DNGET[30];
if( argc<2 ) {
  printf("Start client with 1 parameter:\n\
dimccounters CTPDIM           (ctp counters server) or \n\
dimccounters DETECTOR_NAME    (as used by ECS/DAQ, i.e. spd, muon_trk,...)\n");
  exit(4);
};
strncpy(DETNAME, argv[1], 9);
if(strcmp(DETNAME, "CTPDIM")==0) {
  realcnts=NCOUNTERS;
  initcntsCTP();
} else {
  realcnts=LTUNCOUNTERSall;
  initcntsLTU();
};
/*
strcpy(DNMON,DETNAME); strcat(DNMON, "/MONCOUNTERS"); */
strcpy(DNGET,DETNAME); strcat(DNGET, "/GETC1");
strcpy(DNMON,DETNAME); strcat(DNMON, "/RESULT");

inforc= dic_info_service(DNMON, MONITORED, 0, 
  cnts,4*realcnts, gotcnts, 137, &cntsFailed, 4); 
printf("ONCE_ONLY version. %s service id:%d\n", DNMON, inforc);
while(1) {
  char inpline[100];
  //sleep(100);
  printf("\n\
g     -get counters immediately\n\
q     -quit:");
  fgets(inpline, 100, stdin);
  if(inpline[0]=='q') break;
  if(inpline[0]=='g') {
    int rc;
    rc= dic_cmnd_service(DNGET, cnts,0); 
    printf("RC from dic_cmnd_service:%d\n", rc);
  };
};
dic_release_service(inforc);
return(0);
} 
