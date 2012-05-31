/* readltu.c -dim client passing LTU counters to RRD
todo: LTUS[] table is static. Should be read from database
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h> 

#ifdef CPLUSPLUS
extern "C" {
#include <dic.hxx>
} 
#else
#include <dic.h>
#endif
#include "common.h"
#include "vmeblib.h"
 
#define LTUNCOUNTERSall 26

/* till 25.10.07: #define EPOCHSEC 725
#define epochmics 726 */
#define temp 23
#define epochsecs 24
#define epochmics 25
#define l2a_strobe 19

#define MAXTEMPLATEL 20
#define NLTUS 19
char *LTUS[]={"spd", "sdd", "ssd", "tpc", "trd", "tof", "phos",
  "cpv", "hmpid", "muon_trk", "muon_trg", "pmd",
  "fmd", "t0", "v0", "zdc", "acorde", "emcal", "daq"}; 
//define NLTUS 2 
//char *LTUS[]={"hmpid", "muon_trk"};

/* this should be loaded from ltunames.sorted 
char *ltunames[]={"time","in_busy1","in_busy2","sbusy","busy","l1nf","l2nf",
"bcerror", "start_all","busy_ts", "l1nf_ts","l2nf_ts","any_error",
"bcerror_ts","mstrobe_ts","prepulse","l0","l1_only","l1_strobe",
"l2a_strobe", "l2r_strobe","spare1","spare2","temp",
"epochsecs","epochmics"};
*/

FILE *rrdpipe;
FILE *dbgout=NULL;

unsigned int cntsFailed=0xdeaddeed;
unsigned int cnts[LTUNCOUNTERSall];

typedef struct {
  int inforc;
  w32 timeInSecs;   // 0: first pass 1: after first pass (not used anyhow)
  char ntemplate[MAXTEMPLATEL];   // ltu name UPPER case
} Tinforcs;

Tinforcs inforcs[NLTUS];
int Nupdates=0;  // # of updates received. max: NLTUS

/*----------------------------------*/ void checkServers(w32 now) {
/* now: current time
Operation:
- restart the servers, which did not updated in last 90 seconds
*/
int ix; char msg[500]="";
for(ix=0; ix< NLTUS; ix++) {
  if( inforcs[ix].timeInSecs< 2) {
    inforcs[ix].timeInSecs= now; 
  } else { ;
    w32 timedif;
    timedif= dodif32(inforcs[ix].timeInSecs,now);  //<60 if all servers ON
    //printf("checkServers: timedif:%d\n",timedif);
    if( timedif>66) {
      int rc=11; char cmd[80];
      if(msg[0]=='\0') {
        char dat[20];
        epoch2date(now, dat);
        sprintf(msg,"Restart at %s(%d):", dat, now);
      };
      sprintf(cmd,"lturestart.bash %s", LTUS[ix]);
      //rc= system(cmd);
      sprintf(msg, "%s %s:%d:%d", msg, inforcs[ix].ntemplate, timedif, rc);
    };
  };
}; strcat(msg,"\n");
if(strcmp(msg,"\n")!=0) {
  printf(msg); fflush(stdout);
};
}
/*-----------------------*/ void gotcnts(void *tag, void *buffer, int *size) {
int ix, ixtag;
w32 timesecs,timemics;
char dat[20];
w32 *bufw32=(w32 *)buffer;
//printf("gotcnts tag:%d size:%d\n", *(int *)tag, *size ); fflush(stdout);
if(*size != 4*LTUNCOUNTERSall) {
  printf("error in gotcnts. Tag:%d Size:%d First word(if any):0x%x\n",
    *(int *)tag, *size, bufw32[0]);
  fflush(stdout);
  return;
};
ixtag=*(int *)tag;
//timesecs= bufw32[epochsecs]+ bufw32[epochmics]/1000000.; 
timesecs= bufw32[epochsecs]; timemics= bufw32[epochmics];
inforcs[ixtag].timeInSecs= timesecs;
Nupdates++;
if(Nupdates>=NLTUS) {Nupdates=0; checkServers(timesecs); };
//printf("gotcnts:%s:%d:%d/%d\n",inforcs[ixtag].ntemplate, ixtag, timesecs, timemics);

fprintf(rrdpipe, "update rrd/%scounters.rrd ", inforcs[ixtag].ntemplate);
//fprintf(dbgout, "update rrd/%scounters.rrd ", inforcs[ixtag].ntemplate);

fprintf(rrdpipe, "%u:", bufw32[epochsecs]);
//fprintf(rrdpipe, "N:");
//fprintf(dbgout, "%u:", bufw32[epochsecs]);
//fprintf(dbgout, "N:");
for(ix=0; ix<(LTUNCOUNTERSall-1); ix++) {
  fprintf(rrdpipe, "%u:", bufw32[ix]);
//  fprintf(dbgout, "%u:", bufw32[ix]);
}; 
fprintf(rrdpipe, "%u \n", bufw32[LTUNCOUNTERSall-1]); fflush(rrdpipe);
//fprintf(dbgout, "%u \n", bufw32[LTUNCOUNTERSall-1]); fflush(dbgout);
epoch2date(bufw32[epochsecs], dat);
/* printf("%u=%s: %d=%s: l2a_strobe:%u \n", 
  bufw32[epochsecs], dat, ixtag, LTUS[ixtag], bufw32[l2a_strobe]);
fflush(stdout); */
/*
printf(" addr    0x abs           diff\n");
for(ix=0; ix<NCS; ix++) {
  cs[ix].currcs= bufw32[cs[ix].reladdr];
  printf(" %3d %8x %10.4f\n", cs[ix].reladdr, cs[ix].currcs, 
    dodif32(cs[ix].prevcs, cs[ix].currcs)/2500000.);
  cs[ix].prevcs= cs[ix].currcs;
};
printf("\n");
*/
}

/*---------------------------------------*/ int main(int argc, char **argv) {
int inforc, ixtag, ixdet;
char service[40], ltuUPPER[40];
//setbuf(stdout, NULL);   nebavi
//dbgout= fopen("dbgout", "w");
rrdpipe= popen("/usr/bin/rrdtool - >ltu_rrdtool.log", "w");
if(rrdpipe==NULL) {
  printf("Cannot open /usr/bin/rrdtool -\n");
  exit(8);
};
for(ixtag=0; ixtag< NLTUS; ixtag++) {
};
for(ixtag=0; ixtag< NLTUS; ixtag++) {
  inforcs[ixtag].timeInSecs= 0;
  strcpy(ltuUPPER, LTUS[ixtag]); UPPER(ltuUPPER);
  sprintf(inforcs[ixtag].ntemplate, "%s", ltuUPPER);
  sprintf(service, "%s/MONCOUNTERS", LTUS[ixtag]);    // small caps
  inforc= dic_info_service(service, MONITORED, 0, 
    cnts,4*(LTUNCOUNTERSall), gotcnts, ixtag, &cntsFailed, 4); 
  inforcs[ixtag].inforc= inforc;
  printf("%s=%d id:%d %s\n", service, ixtag, inforc, inforcs[ixtag].ntemplate);
};
while(1) {
  sleep(100);
};
//fclose(dbgout);
pclose(rrdpipe);
for(ixdet=0; ixdet<NLTUS; ixdet++ ) {
  dic_release_service(inforcs[ixdet].inforc);
};
return(0);
} 
