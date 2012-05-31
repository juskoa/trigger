/* dimcltus.c -an example of dim client reading LTU counters from 2 DIM servers */
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
Tcnt1 cs[24][NCS];

int Ndets=0;
int inforcs[24];
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
/*---------------------------------------------------*/ void initcntsCTP(int ix) {
/* let's take time counters from BUSY, L0,L1,L2 FO1, FO2 board,
so we can check if difference is 60 seconds (server is sending
counters every minute) */
cs[ix][0].reladdr= CSTART_BUSY+39;
cs[ix][1].reladdr= CSTART_L0+13; 
cs[ix][2].reladdr= CSTART_L1+5; 
cs[ix][3].reladdr= CSTART_L2+5;
cs[ix][4].reladdr= CSTART_FO+0;
cs[ix][5].reladdr= CSTART_FO+1*NCOUNTERS_FO+0;
}
/*---------------------------------------------------*/ void initcntsLTU(int ix) {
/* let's take timers: time, in_busy1, in_busy2, sbusy
   and counters:      busy_ts, l2a_strobe
*/
cs[ix][0].reladdr= 0;
cs[ix][1].reladdr= 1;
cs[ix][2].reladdr= 2;
cs[ix][3].reladdr= 3;
cs[ix][4].reladdr= 9;
cs[ix][5].reladdr= 19;
};
/*-----------------------*/ void gotcnts(int *tag, w32 *buffer, int *size) {
int ix, tagm1;
printf("\ngotcnts tag:%d size:%d (%d words)\n", *tag, *size, *size/4 );
if(*size != 4*realcnts) {
  printf("error in gotcnts. got:%d, expected:%d words\n\
First word of message (if any):0x%x\n",
    *size/4, realcnts, buffer[0]);
  return;
}; tagm1= *tag-1;
printf(" addr    0x abs           diff[secs]\n");
for(ix=0; ix<NCS; ix++) {
  cs[tagm1][ix].currcs= buffer[cs[tagm1][ix].reladdr];
  printf(" %3d %8x %10.4f\n", cs[tagm1][ix].reladdr, cs[tagm1][ix].currcs, 
    dodif32(cs[tagm1][ix].prevcs, cs[tagm1][ix].currcs)/2500000.);
  cs[tagm1][ix].prevcs= cs[tagm1][ix].currcs;
};
printf("\n");
}

/*----------------------------------------------------------------- main() 
*/
int main(int argc, char **argv) {
int inforc, ixdet;
char DETNAME[10];
char DNMON[30];
char DNGET[30];
if( argc<2 ) {
  printf("Start client with al least 1 parameter:\n\
dimccounters det1 det2 ...    (as used by ECS/DAQ, i.e. spd, muon_trk,...)\n");
  exit(4);
};
ixdet=1;
while (ixdet<argc) {
  strncpy(DETNAME, argv[ixdet], 9);
  if(strcmp(DETNAME, "CTPDIM")==0) {
    realcnts=NCOUNTERS;
    initcntsCTP(ixdet-1);
  } else {
    realcnts=LTUNCOUNTERSall;
    initcntsLTU(ixdet-1);
  };
  strcpy(DNMON,DETNAME); strcat(DNMON, "/MONCOUNTERS");
  if(ixdet==1) {
    strcpy(DNGET,DETNAME); strcat(DNGET, "/GETCOUNTERS");
  };
  inforc= dic_info_service(DNMON, MONITORED, 0, 
    cnts,4*realcnts, gotcnts, ixdet, &cntsFailed, 4); 
  inforcs[ixdet-1]=inforc;
  printf("%s service id:%d\n", DNMON, inforc);
  ixdet++;
}; Ndets=ixdet-1;
while(1) {
  char inpline[100];
  //sleep(100);
  printf("\n\
g     -get counters immediately from first LTU\n\
q     -quit:");
  fgets(inpline, 100, stdin);
  if(inpline[0]=='q') break;
  if(inpline[0]=='g') {
    int rc;
    rc= dic_cmnd_service(DNGET, cnts,0);
    printf("RC from dic_cmnd_service:%d\n", rc);
  };
};
for(ixdet=0; ixdet<Ndets; ixdet++ ) {
  dic_release_service(inforcs[ixdet]);
};
return(0);
} 
