/* dimccounters.c -an example of dim client reading CTP/LTU counters */
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
 
#define LTUNCOUNTERSall 26
#define cntsNCOUNTERS (2*NCOUNTERS) // allocate more (i.e. if CTPcnts update)
unsigned int cnts[cntsNCOUNTERS];
int realcnts=NCOUNTERS;
int cntsprinted=0;
unsigned int cntsFailed=0xdeaddeed;
float cntsFailedLum= -1.0;

typedef struct {
  int reladdr;
  w32 prevcs;
  w32 currcs;
  char name[24];
} Tcnt1;

#define NCS 10   // elapsed time for BUSY, L0,1,2,FO1, FO3
Tcnt1 cs[NCS];
#define NCSv 24   // volts (4 in 1 word)
Tcnt1 csv[NCSv];

#define MAXNLUMCNTS 50   // max. # of fixed counters for DCS
unsigned int LUMCNTS[MAXNLUMCNTS];

unsigned int BEAMMODE;   //service value

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
int ix;
/* let's take time counters from BUSY, L0,L1,L2 FO1, FO2 board,
so we can check if difference is 60 seconds (server is sending
counters every minute) */
cs[0].reladdr= CSTART_BUSY+43; strcpy(cs[0].name, "bytime");
/*
cs[1].reladdr= CSTART_L0+13; strcpy(cs[1].name, "l0time");
cs[2].reladdr= CSTART_L1+5; strcpy(cs[2].name, "l1time");
cs[3].reladdr= CSTART_L2+5;strcpy(cs[3].name, "l2time");
cs[4].reladdr= CSTART_FO+0;strcpy(cs[4].name, "fo1time");
*/
ix=1;cs[ix].reladdr= CSTART_SPEC+13; strcpy(cs[ix].name, "fo1temp");
ix=2;cs[ix].reladdr= CSTART_SPEC+14; strcpy(cs[ix].name, "fo1volts");
ix=3;cs[ix].reladdr= CSTART_SPEC+15; strcpy(cs[ix].name, "fo2temp");
ix=4;cs[ix].reladdr= CSTART_SPEC+16; strcpy(cs[ix].name, "fo2volts");
cs[5].reladdr= CSTART_FO+1*NCOUNTERS_FO+0;strcpy(cs[5].name, "fo2time");
cs[6].reladdr= CSTART_SPEC; strcpy(cs[6].name, "epochsecs");
cs[7].reladdr= CSTART_SPEC+1; strcpy(cs[7].name, "epochmics");
cs[8].reladdr= CSTART_TSGROUP; strcpy(cs[8].name, "spare895TSGROUP");
cntsprinted=9;
for(ix=0; ix<24; ix++) {
  char ltuname[24];
  sprintf(ltuname,"ltu%2.0d",ix+1);
  csv[ix].reladdr= CSTART_LTUVOLTS+ix; strcpy(csv[ix].name, ltuname);
};
}
/*---------------------------------------------------*/ void initcntsLTU() {
/* let's take timers: time, in_busy1, in_busy2, sbusy
   and counters:      busy_ts, l2a_strobe
*/
cs[0].reladdr= 0; strcpy(cs[0].name, "time");
cs[1].reladdr= 1; strcpy(cs[1].name, "in_busy1");
cs[2].reladdr= 2; strcpy(cs[2].name, "in_busy2");
cs[3].reladdr= 3; strcpy(cs[3].name, "sbusy");
cs[4].reladdr= 9; strcpy(cs[4].name, "busy_ts");
cs[5].reladdr= 19; strcpy(cs[5].name, "l2a_strobe");
cs[6].reladdr= 24; strcpy(cs[6].name, "epochsecs");
cs[7].reladdr= 25; strcpy(cs[7].name, "epochmics");
cntsprinted=8;
};
/*------------------------*/ void printVoltages(w32 *buffer) {
int ix;
printf(" addr name \t 0xABS(volts)\n");
for(ix=0; ix<NCSv; ix++) {
  //w32 dif32;
  csv[ix].currcs= buffer[csv[ix].reladdr];
  //dif32= dodif32(csv[ix].prevcs, csv[ix].currcs);
  printf(" %3d %s\t %8x\n", 
    csv[ix].reladdr, csv[ix].name, csv[ix].currcs);
  csv[ix].prevcs= csv[ix].currcs;
};
printf("\n");
}
/*-----------------------------------------*/ void printCounts(w32 *buffer) {
int ix;
printf(" addr name \t\t 0xABS     diff        diff[secs]\n");
for(ix=0; ix<cntsprinted; ix++) {
  w32 dif32;
  cs[ix].currcs= buffer[cs[ix].reladdr];
  dif32= dodif32(cs[ix].prevcs, cs[ix].currcs);
  printf(" %3d %s\t\t %8x %d \t %10.4f\n", 
    cs[ix].reladdr, cs[ix].name, cs[ix].currcs, 
    dif32, dif32/2500000.);
  cs[ix].prevcs= cs[ix].currcs;
};
printf("\n");
}
/*-----------------------*/ void gotcnts(void *tag, void *buffer, int *size) {
printf("\ngotcnts tag:%d size:%d (%d words)\n", *(int *)tag, *size, *size/4 );
if(*size != 4*realcnts) {
  printf("error in gotcnts. got:%d, expected:%d words\n\
First word of message (if any):0x%x\n",
    *size/4, realcnts, *(w32 *)buffer);
};
printCounts((w32 *)buffer);
//printVoltages((w32 *)buffer);
}
/*---------------------*/ void gotlumcnts(void *tag, void *buffer, int *size) {
int ix;
printf("\ngotlumcnts tag:%d size:%d(%d words)\n", *(int *)tag, *size, *size/4);
/*if(*size != 4*realcnts) {
  printf("error in gotcnts. got:%d, expected:%d words\n\
First word of message (if any):0x%x\n",
    *size/4, realcnts, *(w32 *)buffer);
};*/
for(ix=0; ix<*size/4; ix++) {
  float value;
  value= ((float *)buffer)[ix];
  //if(value!=0) printf("%3d:%d=0x%8x\n", ix, value,value);
  printf("%3d:%f\n", ix, value);
};
}
/*---------------------*/ void gotbeammode(void *tag, void *buffer, int *size) {
w32 bm;
bm= *(w32 *)buffer;
printf("\ngotbeammode tag:%d size:%d bytes bm:%d(0x%x)\n", 
  *(int *)tag, *size, bm, bm);
}

/*----------------------------------------------------------------- main() 
*/
int main(int argc, char **argv) {
int inforc,infolum,infobm;
char DETNAME[10];
char DNMON[30]; char DNLUM[30]; char DNGET[30]; char DNPRINT[30];
char DNBEAMMODE[30];
if( argc<2 ) {
  printf("Start client with 1 parameter:\n\
dimccounters CTPDIM           (ctp counters server) or \n\
dimccounters DETECTOR_NAME    (as used by ECS/DAQ, SMALL CAPS! i.e. spd, muon_trk,...)\n");
  exit(4);
};
strncpy(DETNAME, argv[1], 9);
if((strcmp(DETNAME, "CTPDIM")==0) ||
   (strcmp(DETNAME, "CTPDIMt")==0)) {
  realcnts=NCOUNTERS;
  initcntsCTP();
} else {
  realcnts=LTUNCOUNTERSall;
  initcntsLTU();
};
strcpy(DNMON,DETNAME); strcat(DNMON, "/MONCOUNTERS");
strcpy(DNLUM,DETNAME); strcat(DNLUM, "/MONLUMCNTS");
strcpy(DNGET,DETNAME); strcat(DNGET, "/GETCOUNTERS");
strcpy(DNPRINT,DETNAME); strcat(DNPRINT, "/PRINTRUNS");
strcpy(DNBEAMMODE,DETNAME); strcat(DNBEAMMODE, "/BEAMMODE");

inforc= dic_info_service(DNMON, MONITORED, 0, 
  cnts,4*cntsNCOUNTERS, gotcnts, 137, &cntsFailed, 4); 
if(strcmp(DETNAME,"CTPDIM")==0) {
  infolum= dic_info_service(DNLUM, MONITORED, 0, 
    LUMCNTS,4*MAXNLUMCNTS, gotlumcnts, 138, &cntsFailedLum, 4); 
  infobm= dic_info_service(DNBEAMMODE, MONITORED, 0, 
    &BEAMMODE,4, gotbeammode, 138, &cntsFailed, 4); 
  printf("services id %s:%d %s:%d %s:%d\n", DNMON, inforc, 
    DNLUM, infolum, DNBEAMMODE, infobm);
} else {
  printf("services id %s:%d\n", DNMON, inforc);
};
while(1) {
  char inpline[100];
  //sleep(100);
  printf("\n\
g     -get counters immediately\n\
p     -CTPDIM/PRINTRUNS (see dims.log)\n\
q     -quit:");
  fgets(inpline, 100, stdin);
  if(inpline[0]=='q') break;
  if(inpline[0]=='g') {
    int rc;
    rc= dic_cmnd_service(DNGET, cnts,0);
    printf("RC from dic_cmnd_service:%d (OK:1)\n", rc);
  } else if(inpline[0]=='p') {
    int rc;
    rc= dic_cmnd_service(DNPRINT, (void *)"abc",4);
    printf("RC from dic_cmnd_service(%s):%d (OK:1)\n", DNPRINT,rc);
  };
};
dic_release_service(inforc);
dic_release_service(infolum);
return(0);
} 
