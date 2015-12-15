/* readctp.c -dim client passing CTP counters to RRD
3 outputs:
1. rrd
2. html
3. rawcounters subset (all), 1 file per day, format of 1 line:
date time cnt1 ... cntlast

Has to be started in ~/CNTRRD directory. i.e.:
cd ~/CNTRRD ; $VMECFDIR/CNTRRD/linux/readctpc


13.2.2007 I2C,orbit (23 counters-> 1+11*4) added
10.7. LTU volotages added
2.12.2015 "B..." "I..." lines sent to apmonpipe (redis involved)
4.12.2015 intCTPbusy (N_CTPDET:17) now also returned in html line
          different from 'detectors': just busy time of DDL1 link in usecs given
11.12.2015 red_ mydb added
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h> 
#include <math.h> 

#ifdef CPLUSPLUS
extern "C" {
#include <dic.hxx>
}
#else
#include <dic.h>
#endif
#include "ctpcounters.h"
#include "common.h"
#include "vmeblib.h"
#include "udplib.h"
#include "hiredis.h"
 
#define MAXLINELE 80
/* till 25.10.07: #define EPOCHSEC 725
#define epochmics 726 */
#define epochsecs CSTART_SPEC
#define epochmics CSTART_SPEC+1
//von #define L0clstT 152
#define l2orbit CSTART_SPEC+2
#define byin1 CSTART_BUSY      // from here 24 CTP busyin timers starts
#define l0timeix 15   // run1:13

int ARGNORRD=0;
int ARGNOAPPMON=0;
int ARGNOHTML=0;
FILE *rrdpipe=NULL;
FILE *htmlpipe;
FILE *apmonpipe=NULL;
//FILE *dbgout=NULL;
FILE *spurfile=NULL;
int csock_gcalib=-1;   // sending gcalib messages to monitor

w32 debugbusy=0;
w32 debugl2s=0;
w32 debugl0s=0;
w32 debugl2r=0;
w32 debugl1s=0;
w32 debugl0swinp1=0;
char *hname;
w32 prevl0time=0;
int firstreading=1;
unsigned int cntsFailed=0xdeaddeed;
unsigned int cnts[NCOUNTERS];

#define N24 24
#define N48 48
#define N_CTPDET 17
/* 5th column in cnames.sorted2:  
LTUORDER: index corresponds to ECS number of the detector
"-" -not defined (i.e. LTU not connected)
""  -end of table
N_CTPDET -ECS number reserved for CTP
*/
const char *LTUORDER[]={"SPD", "SDD", "SSD", "TPC", "TRD", "TOF", "HMPID",
  "PHOS", "CPV", "PMD", "MUON_TRK", "MUON_TRG",
  "FMD", "T0", "V0", "ZDC", "ACORDE", "-", "EMCAL", "DAQ","-","AD", "-", "-",""}; 
// by RL on TM 1.8.2012: THIS ONE USED FOR L1R CORRECTION
// 28.11.2012: corrected to be equal with DQM: (pmd, muon_trg, v0):
float l1rusecs[N24]={0, 7.0, 7.325, 6.65, 6.75, 6.705, 6.835,
  5.8, 0.0, 16.0, 14.275, 7.1,
  8.26, 6.525, 0.0, 9.2, 7.025, 0.0, 7.035, 0.0,0.0,0.0};
// by RL on TM 9.3.2012 
// both TRD values should be 55us(instead of 266.3)-see daqlog from 27.3.2012:
float l2rusecs[N24]={0, 110.5, 265.1, 306.5, 55.0, 0.0, 107.1,
 56.4, 0.0, 528.3, 412.4, 108.5,
  126.0, 2.8, 0.0, 107.1, 106.8, 0.0, 106.8, 0.0,0.0,0.0};

float l1rusecsClu[N24]={6.525, 8.525, 7.925, 8.225, 7.2, 7.771, 7.5,
  7.4, 0.0, 13.3, 14.5, 7.35,
  8.8, 7.375, 6.66, 10.3, 7.975, 0.0, 8.525, 0.0,0.0,0.0};
float l2rusecsClu[N24]={0, 112.0, 265.8, 308.1, 55.0, 6.5, 107.8,
  57.5, 0.0, 528.6, 412.6, 108.8,
  126.6, 8.2, 6.5, 108.2, 107.7, 0.0, 108.3, 0.0,0.0,0.0};

#define NCS 6   // elapsed time for BUSY, L0,1,2,FO1, FO3

/* bsy/L2s should become L2a in 2012
one of WHATBUSY strings is set in redis.bsy_screen
*/
#define WHATBUSYS 4
const char *WHATBUSY[]={"bsy/L0[us]", "bsy/L2s[us]", "readout[us]", "totalbsy[%]"};
int avbsyix= 2;  // 0,1, or 2  or 3-> one of items in avbsys[]
w32 allreads=0;
int measnum=1;   // 1..9
Tcnt1 cs[NCS];
Tcnt1 busy[N24];   // 24 busys, the same order as in VALID.LTUS
Tcnt1 l0s[N24];   // 24 l2strobes, the same order as in VALID.LTUS
Tcnt1 l1s[N24];   // 24 l1strobes = L1 accepted
Tcnt1 l2s[N24];   // 24 l2strobes, the same order as in VALID.LTUS
Tcnt1 l2r[N24];   // 24 l2reject, the same order as in VALID.LTUS
Tcnt1 ppout[N24];   // 24 ppouts, not for each reading
Tcnt1 l2cal[N24];   // 24 ppouts, as l0s, but not for each reading
Tcnt1 intCTPbusy[1];
Tcnt1 l0swinps[N48];   // l0inp1..48
typedef struct {
  int absy[WHATBUSYS];  // avbsyl0s, avbsyl2s(till Oct. 2012) -will be l2as, avreadout
  int runn;             // 0: not in global, >0: in global runn, [N_CTP].runn is always 0
} Tavbsy;

Tavbsy avbusys[N24];      // usecs, -1: not connected   >999000: dead

#define MAXcalibDets 5
// TOF MUON_TRG T0 ZDC EMCAL
int calibDets[MAXcalibDets]={5,11,13,15,17};

char spurfilename[80]="xx";
char spurline[NCOUNTERS*16];
int spurcnts[]={150, 152, 153, -1};
char rrdpibu[30000];
int rrdpibuready=0;   // 1: ready, write it out

/* print:
  <table class="bsyTable">
  <tr> <th>Detector</th><th>avl1rbsy</th><th>avl2rbsy</th></tr>
  <tr> <td>SPD</td><td>0</td><td>0</td> </tr>
  ...
  </table>
---------------------------------------------*/ void prtL12Table(int ixfrom, int ixto, FILE *of) {
int ix;
fprintf(of,"<table class=\"bsyTable\">\n\
  <tr> <th>Detector</th><th>avl1rbsy</th><th>avl2rbsy</th></tr>\n");
for(ix=ixfrom; ix<=ixto; ix++) {
  fprintf(of,"<tr> <td>%s</td><td>%.3f</td><td>%.3f</td> </tr>\n",
    LTUORDER[ix], l1rusecs[ix], l2rusecs[ix]);
};
fprintf(of,"</table>\n");
}
/*---------------------------------------------*/ void prtTables() {
FILE *of;
char fname[40];
sprintf(fname,"htmls/l12rtimes.html");
of= fopen(fname, "w");
if(of==NULL) {
  printf("Cannot open %s\n", fname);
  return;
} else {
  printf("File %s opened\n", fname);
};
fprintf(of,"<table cellspacing=\"30px\"><tr>\n");
fprintf(of,"<td valign=\"top\">\n");
prtL12Table(0, 9, of);
fprintf(of,"</td>\n<td valign=\"top\">\n");
prtL12Table(10, 19, of);
fprintf(of,"</td>\n</table>\n");
fclose(of);
}
/*---------------------------------------------*/ void gotsignal(int signum) {
char msg[100];
// SIGUSR1:  // kill -s USR1 pid or pkill -SIGUSR1 readctpc
signal(signum, gotsignal); siginterrupt(signum, 0);
sprintf(msg, "got signal:%d", signum); printf("%s\n", msg);
if(signum==SIGUSR1) {   // signum: 10
  avbsyix++; if(avbsyix>=WHATBUSYS) avbsyix=0; 
  printf("busy calculation:%d (0: b/L0 1: b/L2s 2: readout= corrected b/L2a,3:totbsy)\n",
    avbsyix); fflush(stdout);
  //prtTables();
};
if(signum==SIGUSR2) { prtTables(); };  // signum:12 
if(signum==SIGPIPE) { 
   printf("SIGPIPE received...\n"); // when html (pipe) down
};
fflush(stdout);
}

/*-------------------------------------------*/ int isDetector(char *ln) {
int ix;
for(ix=0; ix<N24; ix++) {
  if( LTUORDER[ix][0]=='\0') return(-1);
  if( strcmp(ln, LTUORDER[ix])==0 ) {
    return(ix);
  };
};
return(-1);
}
int isfosignal(char *cname, char *isc) {
int lng,ifoc;
lng= strlen(isc); ifoc= lng+3;   // "fo[1-6]" + cname + [1-4]
if((strncmp(cname,"fo",2)==0) && 
   (strncmp(&cname[3],isc,lng)==0) && 
   ( (cname[2]>='1') && (cname[2]<='6') ) &&
   ( (cname[ifoc]>='1') && (cname[ifoc]<='4') ) ) {
  return(1);
} else {
  return(0);
};
}
/*-------------------------------------------------- shiftcnt()
Operation:
cntstr[ix].prevcs= currcs;
cntstr[ix].currcs= buff32[cntstr[ix].reladdr];
*/
void shiftcnt(Tcnt1 *cntstr, int ix, w32 *bufw32) {
int rad;
//w32 *bufw32= (w32 *)buffer;
rad= cntstr[ix].reladdr;
// debug: change bufw32 if busy according to avbsyix:
if(strcmp(hname, "alidcscom835")!=0) {
  if(cntstr==busy) {
    bufw32[rad]= debugbusy;
    if(ix==0) {
      // +400us or 800us or 1200us
      debugbusy= debugbusy+ 1000000*60;  // 1000000*0.4us/s
      debugl2s= debugl2s+ 100*60;   // 100hz
      debugl0s= debugl0s+ 900*60;   // 900hz
      debugl2r= debugl2r+ 20*60;    // 20hz
      debugl1s= debugl1s+ 500*60;   // 500hz
    };
  } else if(cntstr==intCTPbusy) {
    //printf("CTPbusy:%d:0x%x\n", rad, bufw32[rad]);
    //bufw32[rad]= bufw32[rad]+ allreads*16;   // CTP DDL1 busy time
    ;
  } else if(cntstr==l2s) {
    bufw32[rad]= debugl2s;
  } else if(cntstr==l0s) {
    bufw32[rad]= debugl0s;
  } else if(cntstr==l2r) {
    bufw32[rad]= debugl2r;
  } else if(cntstr==l1s) {
    bufw32[rad]= debugl1s;
  } else if(cntstr==l0swinps) {
    if(ix==0) {
      int ix48;
      debugl0swinp1= debugl0swinp1+ 10*60*allreads;   // +10hz/min
      for(ix48=0; ix48<N48; ix48++) {
        bufw32[rad+ix48]= debugl0swinp1 + ix*100*60;
      };
    } else {
      bufw32[rad]= bufw32[rad] + (allreads%10)*ix*10*60;   // +0,..., 4700 hz for 1..48 inps
    };
    if(ix<=3) {
      printf("   ix:%d bufw32[%d]:%d", ix, rad, bufw32[rad]); 
      if(ix==3) printf("\n");
    };
  } else {
    printf("ERROR internal in shiftcnt\n");
  };
};
cntstr[ix].prevcs= cntstr[ix].currcs; cntstr[ix].currcs= bufw32[rad];  
// let's put the same previous reading with 1st reading (i.e. results in rate 0, not a big peek)
if(allreads==0) {
  cntstr[ix].prevcs= cntstr[ix].currcs;
};
}
/*--------------------------------------------------- checktrigs() */
w32 checktrigs(w32 trigsdif) {
if(trigsdif<0) {
  printf("ERROR in gotcnts: trigsdif:%d\n", trigsdif);
  fflush(stdout);
  trigsdif=1;
} else if(trigsdif==0)trigsdif=1;
return(trigsdif);
}
/*-------------------------------------------------*/ void initbusyl0s() {
/* find addresses of following counters in cnames.sorted2 file:
BUSY:
byinX xxx T SDD            X:1..24
FO: 
foXl0outY xxx foX C SDD    X:1..6  Y:1..4     old (busy calculated with L0s)
foXl0stroY xxx foX C SDD    X:1..6  Y:1..4

foXppoutY xxx foX C SDD

L0:
l0inp1..48
*/
int ix;
int   l0inp1addr=0;    // l0inpXadr= l0inp1addr + (X-1) for X:1..48
FILE *cnames;
char *cfdir; char cnamesname[100];
Tsorted pl;

char line[MAXLINELE];
for(ix=0; ix<N24; ix++) {
  busy[ix].reladdr=-1;      // not found in cnames.sorted2
  l0s[ix].reladdr=-1;
  l1s[ix].reladdr=-1;
  l2s[ix].reladdr=-1;
  l2r[ix].reladdr=-1;
  ppout[ix].reladdr=-1;
  l2cal[ix].reladdr=-1;
};
cfdir= getenv("VMECFDIR");
strcpy(cnamesname, cfdir); strcat(cnamesname, "/dimcdistrib/cnames.sorted2");
cnames= fopen(cnamesname, "r");
while(1) {
  int rc,isdet; char *rcp;
  rcp=fgets(line, MAXLINELE, cnames);
  if(rcp==NULL) break;
  rc=parseLine(line, &pl);
  /*printf("%s %d %s %s %c rc:%d\n",pl.cname,pl.addr,pl.board,pl.ltuname,
    pl.type, rc);*/
  if(rc!=0) {
    printf("parseLine rc: %d line:%s\n", rc, line);
    break;
  };
  if(strcmp(pl.cname, "l0inp1")==0) {
     l0inp1addr= pl.addr;
     continue;
  };
  if(strcmp(pl.cname, "intCTPbusy")==0) {
     intCTPbusy[0].reladdr= pl.addr;
     continue;
  };
  //if(pl.addr>3) break;
  // look for: byin1..byin24:
  isdet= isDetector(pl.ltuname);
  if(isdet==-1) continue;
  if((strncmp(pl.cname,"byin",4)==0) && 
     ( (pl.cname[4]>='1') &&(pl.cname[4]<='9'))) {
     busy[isdet].reladdr= pl.addr;
     continue;
  };
  if(isfosignal(pl.cname, (char *)"l0out")) {
     l0s[isdet].reladdr= pl.addr;
     continue;
  };
  if(isfosignal(pl.cname, (char *)"l1out")) {
     l1s[isdet].reladdr= pl.addr;
     continue;
  };
  if(isfosignal(pl.cname, (char *)"l2stro")) {
     l2s[isdet].reladdr= pl.addr;
     l2cal[isdet].reladdr= pl.addr;
     continue;
  };
  if(isfosignal(pl.cname, (char *)"l2rout")) {
     l2r[isdet].reladdr= pl.addr;
     continue;
  };
  if(isfosignal(pl.cname, (char *)"ppout")) {
     ppout[isdet].reladdr= pl.addr;
     continue;
  };
};
fclose(cnames);
for(ix=0; ix<N24; ix++) {
  if(LTUORDER[ix][0]=='\0') break;
  printf("%d:%s: %d\t%d\t%d\t%7.3f\n", ix,
    LTUORDER[ix], busy[ix].reladdr, l2s[ix].reladdr, ppout[ix].reladdr,
    l1rusecsClu[ix] );
};
printf("l0inp1addr:%d\n", l0inp1addr);
for(ix=0; ix<48; ix++) {
  l0swinps[ix].reladdr= l0inp1addr + ix;
  l0swinps[ix].currcs= 0;
};
}
/*------*/float calc_rate(Tcnt1 *counter, float caltime_usecs) {
w32 l2dif; float rate;
l2dif= dodif32(counter->prevcs, counter->currcs);
if(l2dif==0) {
  rate= 0.;
} else {
  rate= 1000000.0*l2dif/caltime_usecs;
};
if(rate<0.001) rate= 0.;
return(rate);
}
/*------  float calc_rate_old(Tcnt1 *counter, w32 *bufw32, float caltime) {
w32 l2dif; int rad; float rate;
rad= counter->reladdr;
if(rad != -1) { 
  l2dif= dodif32(counter->prevcs, bufw32[rad]);
  counter->prevcs= bufw32[rad];
  if(l2dif==0) {
    rate= 0.;
  } else {
    rate= l2dif/caltime;
    if(rate<0.001) rate= 0.;
  };
} else {
  rate= 0.;
};
return(rate);
} */
#define do1streading() \
  caltime= -1.; \
  /* store prevl0time, prev_l2, prev_ppout for 5dets */ \
  prevl0time= bufw32[l0timeix]; \
  for(ix=0; ix<N24; ix++) { \
    int rad; \
    rad= l2cal[ix].reladdr; \
    if(rad != -1) { l2cal[ix].prevcs= bufw32[rad]; }; \
    rad= ppout[ix].reladdr; \
    if(rad != -1) { ppout[ix].prevcs= bufw32[rad]; }; \
  }; \

FILE *openrrd() {
FILE *rcp;
rcp= popen("/usr/bin/rrdtool -", "w");
//rcp= popen("cat - >logs/rrdcat", "w");
if(rcp==NULL) {
  printf("Cannot open /usr/bin/rrdtool\n");
} else {
  printf("rrdpipe opened.\n");
}; return(rcp);
}
FILE *openpipew(char *cmd) {
FILE *rcp;
rcp= popen(cmd, "w");
if(rcp==NULL) {
  printf("Cannot open %s\n", cmd);
} else {
  setlinebuf(rcp);
  printf("%s opened.\n", cmd);
}; return(rcp);
}
/*---------------------------------------------------- update_runns()
O: update avbusys[0..23].runn from redis database
*/
void update_runns() {
int ixr, runs[6], dets[6];
for(ixr=0; ixr<N24; ixr++) { avbusys[ixr].runn= 0; };
red_get_runs(runs,dets);
for(ixr=0; ixr<6; ixr++) {   // all runs
  int id, det;
  if(runs[ixr]==0) break;
  det= dets[ixr];
  for(id=0; id<N24; id++) {
    if( (1<<id) & det ) {
      int oldrun;
      oldrun= avbusys[id].runn;
      if((oldrun !=0) && (oldrun != runs[ixr])) {
        printf("ERROR: det:%d already in run:%d. New run:%d\n",
          id, oldrun, runs[ixr]);
      };
      avbusys[id].runn= runs[ixr];
    };
  };
};
}
/*---------------------------------------------------- db_getrunn(detn)
rc: 0 if detector not in global run
   >0 run number where detn is included
*/
int db_getrn(int detn) {
int runn;
if(strcmp(LTUORDER[detn], "-")==0) {runn=0;   // not connected
} else if(strcmp(LTUORDER[detn], "")==0) {runn=0;   // end of table (detn=24) should never happen!
//} else if((detn%4)==0) {runn=0;
//} else {runn=222222;
} else {
  runn= avbusys[detn].runn;
};
return(runn);
}
#define MAX_APMONB 452
#define MAX_APMONI 652
/*---------------------------------------------------- update_apmonlineB
Operation:
if detN inside a run:   (use redis db)
  - update line
CTP always present with runn:0
 * */
void update_apmonlineB(char *apmonline, int detN, int avbusy) {
int runn=0 ; char detpart[24];
if(detN==N_CTPDET) {
  runn=0;
} else {
  runn= db_getrn(detN);
};
if((runn!=0) || (detN==N_CTPDET)) {
  sprintf(detpart," %d %d %d", detN, runn, avbusy);
  if((strlen(detpart)+strlen(apmonline)) < MAX_APMONB) {
    strcat(apmonline, detpart);
  } else {
    printf("ERROR: too long apmonlineB:'%s'+'%s'\n", apmonline, detpart);
  };
};
}

/*-----------------------*/ void gotcnts(void *tag, void *buffer, int *size) {
int ix, rcapmon; //,ixx;   
int ndaso;
w32 timedelta;
float timesecs; //, caltime;
unsigned int itimesecs;
char dat[20]; char dmyhms[20];
// readout[us] 30.11.2015 15:43:52 minute 1000000 ...   39+24*8= 231
#define MAX_HTMLLINE 300
char htmlline[MAX_HTMLLINE];     // was 1000 before 30.11.2015
// B epochT     det runN  avbusy
// B 1448880367 10 245865 1000000...   maxlen: 13+24*18= 445
char apmonlineB[MAX_APMONB];
// I epochT     inp1 ... inp48   (float: 12.3f)
// I 1448880367 40000000.222 ...       maxlen: 13+48*13= 637
char apmonlineI[MAX_APMONI];
w32 *bufw32= (w32 *)buffer;
//printf("gotcnts tag:%d+1 size:%d rrdpibuready:%d\n", *(int *)tag, *size, rrdpibuready );
//*(int *)tag= *(int *)tag  +1;
if(*size != 4*NCOUNTERS) {
  printf("ERROR in gotcnts. Size:%d First word(if any):0x%x\n",
    *size, *bufw32);
  return;
};
timesecs= bufw32[epochsecs]+ bufw32[epochmics]/1000000.;
itimesecs= nearbyintf(timesecs);
getdatetime(dmyhms);
printf("%s %17.6f: %d counters\n", dmyhms, timesecs, *size/4); fflush(stdout);
timedelta= dodif32(prevl0time, bufw32[l0timeix]);  // in 0.4micsecs
prevl0time= bufw32[l0timeix];
measnum++; if(measnum>=10) measnum=1;
// printf("intCTPbusy:%d:0x%x\n", intCTPbusy[0].reladdr, bufw32[intCTPbusy[0].reladdr]);
/*------------------------------------------------------------ rrd */
if(rrdpibuready==0) {
  //fprintf(rrdpipe, "update rrd/ctpcounters.rrd ");
  sprintf(rrdpibu, "update rrd/ctpcounters.rrd ");
  //fprintf(dbgout, "update rrd/ctpcounters.rrd ");
  //fprintf(rrdpipe, "%u:", bufw32[epochsecs]);
  sprintf(rrdpibu, "%s%u:", rrdpibu, bufw32[epochsecs]); ndaso=0; // epochsecs
  //fprintf(dbgout, "%u:", bufw32[epochsecs]);
  for(ix=0; ix<=(NCOUNTERS-1); ix++) {
    int ixspec;
    if((( (ix>=CSTART_SPEC+4) && (ix<=CSTART_SPEC+4+20) ) 
       && (((ix-CSTART_SPEC) %2)==0)) || 
       ((ix>CSTART_SPEC+4+20) && (ix<=CSTART_SPEC+4+20+24))   // 24xLTU voltages
      ) {   // CTP-voltage || LTU voltage
      int volts[4];
      vme2volt(bufw32[ix], volts);
      for(ixspec=0; ixspec<4; ixspec++) {
        //printf("ix:%d ixspec:%d NCOUNTERS:%d\n", ix, ixspec,NCOUNTERS);
        // from 16.5.2015 counters end is later...
        //fprintf(rrdpipe, "%u:", volts[ixspec]);
        sprintf(rrdpibu, "%s%u:", rrdpibu, volts[ixspec]); ndaso++;
      };
      continue;
    };
    if( ix==(NCOUNTERS-1) ) { // from 16.5.2015 we need NL here
      //fprintf(rrdpipe, "%u: \n", bufw32[ix]);
      sprintf(rrdpibu, "%s%u: \n", rrdpibu, bufw32[ix]); ndaso++;
      printf("rrdpibu length:%d num. of data sources:%d\n",(int)strlen(rrdpibu),ndaso);
      if(ndaso < 1865) {
        printf("skipping rrd write (<1865)...\n");
      } else {
        if(ARGNORRD==0) {
           rrdpibuready=1;
           //rcr= fprintf(rrdpipe, "%s", rrdpibu);  moved to main
        } else {
          printf("skipping fprintf(rrdpipe..., -norrd\n");
        };
      };
      ndaso=0;
      break;
    } else {
      //fprintf(rrdpipe, "%u:", bufw32[ix]);
      sprintf(rrdpibu, "%s%u:", rrdpibu, bufw32[ix]); ndaso++;
    };
  }; 
} else {
  printf("rrd update skipped.\n");
};
epoch2date(bufw32[epochsecs], dat);
printf("epoch: %s\n", dat);
printf("busytemp:%d l0temp:%d\n", bufw32[1514], bufw32[1516]);

/*------------------------------------------------------------ html */
sprintf(htmlline, "%s %s minute ", WHATBUSY[avbsyix], dat);
sprintf(apmonlineB, "B %u ", itimesecs); sprintf(apmonlineI, "I %u ", itimesecs);
update_runns();
for(ix=0; ix<N24; ix++) {
  int rad,notdefined;
  notdefined=0;
  rad= busy[ix].reladdr;
  //printf("ix:%d rad:%d\n", ix, rad);
  if(rad != -1) {
    if((strcmp(hname, "alidcscom835")!=0) && (ix<2)) {
      printf("Warning: ================================ arranging dbg values in non-p2 setup!...\n");
    };
    shiftcnt(busy, ix, bufw32);
    shiftcnt(l0s, ix, bufw32);
    shiftcnt(l1s, ix, bufw32);
    shiftcnt(l2s, ix, bufw32);
    shiftcnt(l2r, ix, bufw32);
    //shiftcnt(ppout, ix, bufw32); treated separately
    //shiftcnt(l2cal, ix, bufw32);
  } else {
    if(ix==N_CTPDET) {
      shiftcnt(intCTPbusy, 0, bufw32);
    };
    notdefined=1;
  };
  if(allreads==0) goto RTRN;  // we need 2 readings at least
  if(notdefined==1) {
    if(ix==N_CTPDET) {
      int avbusy;
      //printf("prev curr:0x%x 0x%x\n", intCTPbusy[0].prevcs, intCTPbusy[0].currcs);
      avbusy= (int)round( dodif32(intCTPbusy[0].prevcs, intCTPbusy[0].currcs)*0.4);
      sprintf(htmlline, "%s %d", htmlline, avbusy);   // just busy in usecs for CTP
      update_apmonlineB(apmonlineB, ix, avbusy);
    } else {
      sprintf(htmlline, "%s -", htmlline);
    };
  } else {
    int cix;
    for(cix=0; cix<WHATBUSYS; cix++) {
      int avbusy; w32 trigsdif, l2rsdif; float totbusy;
      totbusy= dodif32(busy[ix].prevcs, busy[ix].currcs)*0.4;
      //printf("dbg1: %d %d %d %f\n", cix, busy[ix].prevcs, busy[ix].currcs, totbusy);
      //dbg totbusy= timedelta*0.4*(measnum/10.); 
      if(cix==0) {   // bsy/L0
        trigsdif= dodif32(l0s[ix].prevcs, l0s[ix].currcs);
        trigsdif= checktrigs(trigsdif);
        avbusy= (int)round(totbusy/trigsdif);
        if(ix==0) {
          printf("cix:%d:totb:%f trgs:%d avb:%d\n", 
            cix, totbusy, trigsdif, avbusy);
        };
      } else if(cix==1) {   // bsy/L2s
        trigsdif= dodif32(l2s[ix].prevcs, l2s[ix].currcs);
        /* should be: 
        l2rsdif= dodif32(l2r[ix].prevcs, l2r[ix].currcs);
        trigsdif= trigsdif - l2rsdif;   // bsy/L2a */
        trigsdif= checktrigs(trigsdif);
        avbusy= (int)round(totbusy/trigsdif);
        if(ix==0) {
          printf("cix:%d:totb:%f trgs:%d avb:%d\n", 
            cix, totbusy, trigsdif, avbusy);
        };
      } else if(cix==2) {   // readout/L2a [us]
        float busy_L2r, busy_L1r; w32 cl0s, cl1s, l2rs, l1rs;
        trigsdif= dodif32(l2s[ix].prevcs, l2s[ix].currcs);
        l2rsdif= dodif32(l2r[ix].prevcs, l2r[ix].currcs);
        trigsdif= trigsdif - l2rsdif;
        trigsdif= checktrigs(trigsdif);
        cl0s= dodif32(l0s[ix].prevcs, l0s[ix].currcs);
        cl1s= dodif32(l1s[ix].prevcs, l1s[ix].currcs);
        l1rs= cl0s - cl1s;
        busy_L1r= l1rusecs[ix]*l1rs;
        l2rs= dodif32(l2r[ix].prevcs, l2r[ix].currcs);
        busy_L2r= l2rusecs[ix]*l2rs;
        avbusy= (int)round((totbusy-busy_L1r-busy_L2r)/trigsdif);
        if(ix==0) {
          printf("cix:%d %d=%s:totb:%f L1rb:%f L2rb:%f trgs:%d avb:%d\n", 
            cix,ix,LTUORDER[ix], totbusy, busy_L1r, busy_L2r,trigsdif,avbusy);
        };
        if(avbusy<0) avbusy=0;
      } else if(cix==3) {    // totbusy [%]
        avbusy= (int)round(100*totbusy/(timedelta*0.4));
      };
      //avbusy=100*ix;
      avbusys[ix].absy[cix]= avbusy;
      if(cix==avbsyix) {
        if(avbusy>999900) {
          sprintf(htmlline, "%s dead", htmlline);
        } else {
          //sprintf(htmlline, "%s %d", htmlline, 100*ix);
          sprintf(htmlline, "%s %d", htmlline, avbusy);
        };
        update_apmonlineB(apmonlineB, ix, avbusy);
      };
    };
  };
}; strcat(htmlline,"\n");
strcat(apmonlineB,"\n");
printf("%u=%s: l2orbit:%u busytemp:%u busyvolts:%x debugbusy:%u\n", 
  bufw32[epochsecs], dat, bufw32[l2orbit], bufw32[CSTART_SPEC+3],
  bufw32[CSTART_SPEC+4], debugbusy); 
if(bufw32[epochsecs]==0) {
  printf("ERROR in gotcnts: bad time\n");
  fflush(stdout);
  return;
} else {
  /*int inforc;
  inforc= ftell(htmlpipe); printf("ftell:%d\n", inforc); always -1 */
  if(ARGNOHTML==0) {
    fwrite(htmlline, strlen(htmlline), 1, htmlpipe);   //fprintf(htmlpipe, htmlline);
    printf("%s", htmlline);
  };
};

/*------------------------------------------------------------ apmon 
*/
for(ix=0; ix<N48; ix++) {
  float trigrate; //w32 trigsdif;
  char strigrate[16];
  shiftcnt(l0swinps, ix, bufw32);
  /* trigsdif= dodif32(l0swinps[ix].prevcs, l0swinps[ix].currcs);
  trigrate= round(trigsdif/(timedelta*0.4)); */
  trigrate= calc_rate(&l0swinps[ix], 0.4*timedelta);
  sprintf(strigrate, " %12.3f", trigrate);
  strcat(apmonlineI, strigrate);
}; strcat(apmonlineI,"\n");
if(ARGNOAPPMON==0) {
  rcapmon= fprintf(apmonpipe, "%s", apmonlineB);
  if((w32)rcapmon != strlen(apmonlineB) )  {
    printf("ERROR: apmonB fprintf rc:%d != line_len:%u\n", rcapmon, (int)strlen(apmonlineB));
  } else {
    printf("INFO apmonB pipe ok:%s", apmonlineB);
  };
  rcapmon= fprintf(apmonpipe, "%s", apmonlineI);
  if((w32)rcapmon != strlen(apmonlineI) )  {
    printf("ERROR: apmonI fprintf rc:%d != line_len:%u\n", rcapmon, (int)strlen(apmonlineI));
  } else {
    printf("INFO apmonI pipe ok:%s", apmonlineI);
  };
};
/*------------------------------------------------------------ gcalib 
for LTUs: TOF MUON_TRG T0 ZDC EMCAL. Attention: MUON_TRG cal. rate; 1/33secs
Correct way should be: go through gcalib.cfg and find expected rates...
goal: send udp message if measured interval is > 1 minute:
ppout date time TOF: L2arate PPrate MUON_TRG: L2arate PPrate ...
 */
/*
if(firstreading==1) {
  do1streading()
  firstreading=0;
} else {
  if(timedelta> (59*2500000)) {   // enough time for rates (>1min)
    int rcudpsend;
    caltime= timedelta/2500000.;   // in secs
    //printf("timedelta:%d caltime:%f.3\n",timedelta, caltime);
    // calculate for 5 dets, send udp message, suggested format: 
    //gcal date time det l2rate ppoutrate
    char udpm[300];
    sprintf(udpm,"gcalib %s", dat);
    for(ix=0; ix<N24; ix++) {
      float l2rate, ppoutrate;
      if((ix==5) || (ix==11) || (ix==13) || (ix==15) || (ix==18) ) {
        // calibrated detector
        l2rate= calc_rate_old(&l2cal[ix], bufw32, caltime);
        sprintf(udpm,"%s %s %.3f ", udpm, LTUORDER[ix], l2rate);
        ppoutrate= calc_rate_old(&ppout[ix], bufw32, caltime);
        sprintf(udpm,"%s %.3f ", udpm, ppoutrate);
      };
    };
    if(csock_gcalib!=-1) {
      rcudpsend= udpsend(csock_gcalib, (unsigned char *)udpm, strlen(udpm)+1);
    };
    //printf("%s\n",udpm);
    //printf("rcudpsend:%d chars sent\n", rcudpsend);
    // gcal 18.08.2011 09:25:49 TOF 92.900  4.903  MUON_TRG 88.031  0.033  T0 0.000  0.000  ZDC 0.000  0.000  EMCAL 0.000  0.000
  } else {
    do1streading()
  };
};
*/
fflush(stdout);

/*---------------------------------------------------------- L1spurious: */
if(strncmp(&spurfilename[0], &dat[0], 2) != 0) {        //every day
//if(strncmp(&spurfilename[10], &dat[11], 2) != 0)     every hour
  char fname[100];
  if(spurfile != NULL) {
    fclose(spurfile); spurfile= NULL;
  };
  //create new spurfile name
  strncpy(spurfilename, dat, 10); 
  spurfilename[10]='\0';
   //spurfilename[10]= dat[11];   every hour
   //spurfilename[11]= dat[12];
   //spurfilename[12]='\0';
  sprintf(spurfilename,"%s.rawcnts", spurfilename); 
  sprintf(fname,"rawcnts/%s", spurfilename);
  spurfile= fopen(fname, "a");
  if(spurfile==NULL) {
    printf("Cannot open %s\n", fname);
  } else {
    printf("File %s opened\n", fname);
  };
};
if(spurfile) {
  //int ixx;
  sprintf(spurline, "%s", dat); //ixx=0;
  for(ix=0; ix<=(NCOUNTERS-1); ix++) {
    /*if(ix == spurcnts[ixx]) {
      sprintf(spurline, "%s %d", spurline, bufw32[ix]);
      ixx++;
      if(spurcnts[ixx]==-1) break;
    };*/
    sprintf(spurline, "%s %x", spurline, bufw32[ix]);
  };fprintf(spurfile,"%s\n", spurline);
  //printf("spurlinelen:%d %s\n", strlen(spurline), spurline);
  printf("spurlinelen:%d\n", (int)strlen(spurline));
};
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
RTRN:
allreads++; return;
}

/*------------------------------*/ int main(int argn, char **argv) {
int inforc, rc, ix, tag=137;
for(ix=0; ix<argn; ix++) {
  //printf("arg%d:%s: ",i, argv[i]); 
  if(ix==0) continue;
  printf(" %s", argv[ix]);
  if(strcmp(argv[ix], "-norrd") == 0){
    ARGNORRD=1;
    continue;
  } else if(strcmp(argv[ix], "-noapmon") == 0){
    ARGNOAPPMON=1;
    continue;
  } else if(strcmp(argv[ix], "-nohtml") == 0){
    ARGNOHTML=1;
    continue;
  };
}; printf("\n");
rc= mydbConnect();
hname= getenv("HOSTNAME");
//setbuf(stdout, NULL);   nebavi
initbusyl0s();
//return(0);
if(ARGNORRD==1) {
  printf("skipping openrrd(..., -norrd\n");
} else {
  rrdpipe= openrrd();
  if(rrdpipe==NULL) {
    printf("Cannot open /usr/bin/rrdtool -\n");
    exit(8);
  };
  //nebavi asi htmlpipe= popen("python ./htmlCtpBusys.py stdin >logs/htmlCtpBusys.log", "w");
  //? htmlpipe= popen("./htmlCtpBusys.py stdin", "w");
  printf("%s rrdpipe OPENED. Opening /tmp/htmlfifo (will wait for htmlCtpBusy daemon running)...\n", hname);
};
rrdpibuready=0;
if(ARGNOHTML==0) {
  htmlpipe= fopen("/tmp/htmlfifo", "w");    // mkfifo /tmp/htmlfifo
  // waiting on the above open until htmlCtpBusy is not started
  if(htmlpipe==NULL) {
    printf("Cannot open /tmp/htmlfifo \n");
    exit(8);
  };
  printf("/tmp/htmlfifo opened, i.e. htmlCtpBusy daemon is running.setlinebuf()...\n");
  setlinebuf(htmlpipe);
} else {
  printf("INFO -nohtml, html not updated\n");
};
if(ARGNOAPPMON==1) {
  printf("skipping apmon(..., -noapmon\n");
} else {
  // started in ~/CNTRRD pwd, i.e. apmonConfig.conf in ~/CNTRRD/
  apmonpipe= openpipew((char *)"/home/alice/trigger/Downloads/ApMon_cpp-2.2.8/examples/example_3 >logs/apmon.log 2>&1");
};
signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
signal(SIGUSR2, gotsignal); siginterrupt(SIGUSR2, 0);
//signal(SIGPIPE, SIG_IGN);
signal(SIGPIPE, gotsignal); siginterrupt(SIGPIPE, 0);

csock_gcalib= udpopens((char *)"localhost", 9931);
if(csock_gcalib==-1) {printf("ERROR in udpopens\n"); /* exit(8);*/ };
printf("gcalib monitoring skipped (csock_gcalib:%d)...\n", csock_gcalib);

//inforc= ftell(htmlpipe); printf("ftell:%d\n", inforc); always -1
//dbgout= fopen("logs/dbgout.log", "w");
inforc= dic_info_service((char *)"CTPDIM/MONCOUNTERS", MONITORED, 0, 
  cnts,4*(NCOUNTERS), gotcnts, tag, &cntsFailed, 4); 
//printf("CTPDIM/MONCOUNTERS service id:%d\n", inforc);
while(1) {
  //sleep(100);
  //printf("dtq_sleep ...\n");
  dtq_sleep(2);
  //printf("dtq_slept %d %d\n",rrdpibuready, tag); fflush(stdout);
  if(rrdpibuready==1) {
    int rcr;
    rcr= fprintf(rrdpipe, "%s", rrdpibu);
    if((w32)rcr != strlen(rrdpibu) )  {
      printf("rrd rc:%d (1 record lost), reopening rrdpipe\n", rcr);
      rrdpipe= openrrd();
      if(rrdpipe==NULL) { exit(8); };
    } else {
      ; //printf("rrdpipe ok.\n");
    };
    rrdpibuready=0;
  };
};
if(rrdpipe) pclose(rrdpipe); 
if(ARGNOHTML==0) {
  ; //pclose(htmlpipe);
};
//fclose(dbgout);
mydbDisconnect();
dic_release_service(inforc);
//udpclose(csock_gcalib);
return(0);
} 

