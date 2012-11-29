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
 
#define MAXLINELE 80
/* till 25.10.07: #define EPOCHSEC 725
#define epochmics 726 */
#define epochsecs CSTART_SPEC
#define epochmics CSTART_SPEC+1
//von #define L0clstT 152
#define l2orbit CSTART_SPEC+2
#define byin1 CSTART_BUSY      // from here 24 CTP busyin timers starts
#define l0timeix 13

FILE *rrdpipe;
FILE *htmlpipe;
//FILE *dbgout=NULL;
FILE *spurfile=NULL;
int csock_gcalib=-1;   // sending gcalib messages to monitor

w32 debugbusy=0;
w32 debugl2s=0;
w32 debugl0s=0;
w32 debugl2r=0;
w32 debugl1s=0;
char *hname;
w32 prevl0time=0;
int firstreading=1;
unsigned int cntsFailed=0xdeaddeed;
unsigned int cnts[NCOUNTERS];

#define N24 24
/* 5th column in cnames.sorted2: */
char *LTUORDER[]={"SPD", "SDD", "SSD", "TPC", "TRD", "TOF", "HMPID",
  "PHOS", "CPV", "PMD", "MUON_TRK", "MUON_TRG",
  "FMD", "T0", "V0", "ZDC", "ACORDE", "-", "EMCAL", "DAQ",""}; 
// by RL on TM 1.8.2012: THIS ONE USED FOR L1R CORRECTION
// 28.11.2012: corrected to be equal with DQM: (pmd, muon_trg, v0):
float l1rusecs[N24]={0, 7.0, 7.325, 6.65, 6.75, 6.705, 6.835,
  5.8, 0.0, 16.0, 14.275, 7.1,
  8.26, 6.525, 0.0, 9.2, 7.025, 0.0, 7.035, 0.0};
// by RL on TM 9.3.2012 
// both TRD values should be 55us(instead of 266.3)-see daqlog from 27.3.2012:
float l2rusecs[N24]={0, 110.5, 265.1, 306.5, 55.0, 0.0, 107.1,
 56.4, 0.0, 528.3, 412.4, 108.5,
  126.0, 2.8, 0.0, 107.1, 106.8, 0.0, 106.8, 0.0};

float l1rusecsClu[N24]={6.525, 8.525, 7.925, 8.225, 7.2, 7.771, 7.5,
  7.4, 0.0, 13.3, 14.5, 7.35,
  8.8, 7.375, 6.66, 10.3, 7.975, 0.0, 8.525, 0.0};
float l2rusecsClu[N24]={0, 112.0, 265.8, 308.1, 55.0, 6.5, 107.8,
  57.5, 0.0, 528.6, 412.6, 108.8,
  126.6, 8.2, 6.5, 108.2, 107.7, 0.0, 108.3, 0.0};

#define NCS 6   // elapsed time for BUSY, L0,1,2,FO1, FO3

/* bsy/L2s should become L2a in 2012
one of WHATBUSY strings is set in redis.bsy_screen
*/
#define WHATBUSYS 4
char *WHATBUSY[]={"bsy/L0[us]", "bsy/L2s[us]", "readout[us]", "totalbsy[%]"};
int avbsyix= 1;  // 0,1, or 2  or 3-> one of items in avbsys[]
int allreads=0;
int measnum=1;   // 1..9
Tcnt1 cs[NCS];
Tcnt1 busy[N24];   // 24 busys, the same order as in VALID.LTUS
Tcnt1 l0s[N24];   // 24 l2strobes, the same order as in VALID.LTUS
Tcnt1 l1s[N24];   // 24 l1strobes = L1 accepted
Tcnt1 l2s[N24];   // 24 l2strobes, the same order as in VALID.LTUS
Tcnt1 l2r[N24];   // 24 l2reject, the same order as in VALID.LTUS
Tcnt1 ppout[N24];   // 24 ppouts, not for each reading
Tcnt1 l2cal[N24];   // 24 ppouts, as l0s, but not for each reading
typedef struct {
  int absy[WHATBUSYS];  // avbsyl0s, avbsyl2s(till Oct. 2012) -will be l2as, avreadout
} Tavbsy;

Tavbsy avbusys[N24];      // usecs, -1: not connected   >999000: dead

#define MAXcalibDets 5
// TOF MUON_TRG T0 ZDC EMCAL
int calibDets[MAXcalibDets]={5,11,13,15,17};

char spurfilename[80]="xx";
char spurline[8000];
int spurcnts[]={150, 152, 153, -1};

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
void shiftcnt(Tcnt1 *cntstr, int ix, w32 *bufw32) {
int rad;
//w32 *bufw32= (w32 *)buffer;
rad= cntstr[ix].reladdr;
// debug: change bufw32 if busy according to avbsyix:
if(strcmp(hname, "alidcscom188")!=0) {
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
  } else if(cntstr==l2s) {
    bufw32[rad]= debugl2s;
  } else if(cntstr==l0s) {
    bufw32[rad]= debugl0s;
  } else if(cntstr==l2r) {
    bufw32[rad]= debugl2r;
  } else if(cntstr==l1s) {
    bufw32[rad]= debugl1s;
  } else {
    printf("error internal in shiftcnt\n");
  };
};
cntstr[ix].prevcs= cntstr[ix].currcs; cntstr[ix].currcs= bufw32[rad];  
}
w32 checktrigs(w32 trigsdif) {
if(trigsdif<0) {
  printf("error in gotcnts: trigsdif:%d\n", trigsdif);
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
*/
int ix;
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
  //if(pl.addr>3) break;
  // look for: byin1..byin24:
  isdet= isDetector(pl.ltuname);
  if(isdet==-1) continue;
  if((strncmp(pl.cname,"byin",4)==0) && 
     ( (pl.cname[4]>='1') &&(pl.cname[4]<='9'))) {
     busy[isdet].reladdr= pl.addr;
     continue;
  };
  if(isfosignal(pl.cname, "l0out")) {
     l0s[isdet].reladdr= pl.addr;
     continue;
  };
  if(isfosignal(pl.cname, "l1out")) {
     l1s[isdet].reladdr= pl.addr;
     continue;
  };
  if(isfosignal(pl.cname, "l2stro")) {
     l2s[isdet].reladdr= pl.addr;
     l2cal[isdet].reladdr= pl.addr;
     continue;
  };
  if(isfosignal(pl.cname, "l2rout")) {
     l2r[isdet].reladdr= pl.addr;
     continue;
  };
  if(isfosignal(pl.cname, "ppout")) {
     ppout[isdet].reladdr= pl.addr;
     continue;
  };
};
fclose(cnames);
for(ix=0; ix<N24; ix++) {
  if(LTUORDER[ix][0]=='\0') break;
  printf("%s: %d\t%d\t%d\t%7.3f\n",
    LTUORDER[ix], busy[ix].reladdr, l2s[ix].reladdr, ppout[ix].reladdr,
    l1rusecsClu[ix] );
};
}
/*------*/float calc_rate(Tcnt1 *counter, w32 *bufw32, float caltime) {
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
}
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

/*-----------------------*/ void gotcnts(void *tag, void *buffer, int *size) {
int ix; //,ixx;
w32 timedelta;
float timesecs, caltime;
char dat[20];
char htmlline[1000];
w32 *bufw32= (w32 *)buffer;
//printf("gotcnts tag:%d size:%d\n", *(int *)tag, *size );
if(*size != 4*NCOUNTERS) {
  printf("error in gotcnts. Size:%d First word(if any):0x%x\n",
    *size, *bufw32);
  return;
};
timesecs= bufw32[epochsecs]+ bufw32[epochmics]/1000000.;
printf("%17.6f: %d counters\n", timesecs, *size/4); fflush(stdout);
timedelta= dodif32(prevl0time, bufw32[l0timeix]);  // in 0.4micsecs
prevl0time= bufw32[l0timeix];
measnum++; if(measnum>=10) measnum=1;

/*------------------------------------------------------------ rrd */
fprintf(rrdpipe, "update rrd/ctpcounters.rrd ");
//fprintf(dbgout, "update rrd/ctpcounters.rrd ");
fprintf(rrdpipe, "%u:", bufw32[epochsecs]);
//fprintf(dbgout, "%u:", bufw32[epochsecs]);
for(ix=0; ix<=(NCOUNTERS-1); ix++) {
  int ixspec;
  if((( (ix>=CSTART_SPEC+4) && (ix<=CSTART_SPEC+4+20) ) 
     && (((ix-CSTART_SPEC) %2)==0)) || 
     (ix>CSTART_SPEC+4+20)
    ) {   // CTP-voltage || LTU voltage
    int volts[4];
    vme2volt(bufw32[ix], volts);
    for(ixspec=0; ixspec<4; ixspec++) {
      //printf("ix:%d ixspec:%d NCOUNTERS:%d\n", ix, ixspec,NCOUNTERS);
      if((ix==(NCOUNTERS-1)) && (ixspec==3)) {
        fprintf(rrdpipe, "%u \n", volts[ixspec]);
        //fprintf(dbgout, "%u \n", volts[ixspec]);
        fflush(rrdpipe);   // has to be here!
        //fflush(dbgout);   // has to be here!
      } else {
        fprintf(rrdpipe, "%u:", volts[ixspec]);
        //fprintf(dbgout, "%u:", volts[ixspec]);
      };
    };
  } else {
    fprintf(rrdpipe, "%u:", bufw32[ix]);
    /*if((ix>869) && (ix<890)) {
      fprintf(dbgout, "%d=%u:", ix, bufw32[ix]); fflush(dbgout);
    };fprintf(dbgout,"\n");*/
  };
}; 
//fprintf(rrdpipe, "%u \n", bufw32[NCOUNTERS-1]); fflush(rrdpipe); see above
epoch2date(bufw32[epochsecs], dat);

/*------------------------------------------------------------ html */
sprintf(htmlline, "%s %s minute ", WHATBUSY[avbsyix], dat);
for(ix=0; ix<N24; ix++) {
  int rad,notdefined;
  notdefined=0;
  rad= busy[ix].reladdr;
  if(rad != -1) {
    shiftcnt(busy, ix, bufw32);
    shiftcnt(l0s, ix, bufw32);
    shiftcnt(l1s, ix, bufw32);
    shiftcnt(l2s, ix, bufw32);
    shiftcnt(l2r, ix, bufw32);
    //shiftcnt(ppout, ix, bufw32); treated separately
    //shiftcnt(l2cal, ix, bufw32);
  } else {
    notdefined=1;
  };
  if(allreads==0) goto RTRN;  // we need 2 readings at least
  if(notdefined==1) {
    sprintf(htmlline, "%s -", htmlline);
  } else {
    int cix;
    for(cix=0; cix<WHATBUSYS; cix++) {
      int avbusy; w32 trigsdif, l2rsdif; float totbusy;
      totbusy= dodif32(busy[ix].prevcs, busy[ix].currcs)*0.4;
      //dbg totbusy= timedelta*0.4*(measnum/10.); 
      if(cix==0) {
        trigsdif= dodif32(l0s[ix].prevcs, l0s[ix].currcs);
        trigsdif= checktrigs(trigsdif);
        avbusy= (int)round(totbusy/trigsdif);
        if(ix==0) {
          printf("cix:%d:totb:%f trgs:%d avb:%d\n", 
            cix, totbusy, trigsdif, avbusy);
        };
      } else if(cix==1) {
        trigsdif= dodif32(l2s[ix].prevcs, l2s[ix].currcs);  // bsy/L2s
        /* should be: 
        l2rsdif= dodif32(l2r[ix].prevcs, l2r[ix].currcs);
        trigsdif= trigsdif - l2rsdif;   // bsy/L2a */
        trigsdif= checktrigs(trigsdif);
        avbusy= (int)round(totbusy/trigsdif);
        if(ix==0) {
          printf("cix:%d:totb:%f trgs:%d avb:%d\n", 
            cix, totbusy, trigsdif, avbusy);
        };
      } else if(cix==2) {
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
        //if(ix==0) {
          printf("cix:%d %d=%s:totb:%f L1rb:%f L2rb:%f trgs:%d avb:%d\n", 
            cix,ix,LTUORDER[ix], totbusy, busy_L1r, busy_L2r,trigsdif,avbusy);
        //};
      } else if(cix==3) {    // totbusy
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
      };
    };
  };
}; strcat(htmlline,"\n");
printf("%u=%s: l2orbit:%u busytemp:%u busyvolts:%x debugbusy:%u\n", 
  bufw32[epochsecs], dat, bufw32[l2orbit], bufw32[CSTART_SPEC+3],
  bufw32[CSTART_SPEC+4], debugbusy); 
if(bufw32[epochsecs]==0) {
  printf("error in gotcnts: bad time\n");
  fflush(stdout);
  return;
} else {
  /*int inforc;
  inforc= ftell(htmlpipe); printf("ftell:%d\n", inforc); always -1 */
  fprintf(htmlpipe, htmlline);
  //printf(htmlline);
};
/*------------------------------------------------------------ gcalib 
for LTUs: TOF MUON_TRG T0 ZDC EMCAL. Attention: MUON_TRG cal. rate; 1/33secs
Correct way should be: go through gcalib.cfg and find expected rates...
goal: send udp message if measured interval is > 1 minute:
ppout date time TOF: L2arate PPrate MUON_TRG: L2arate PPrate ...
 */
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
        l2rate= calc_rate(&l2cal[ix], bufw32, caltime);
        sprintf(udpm,"%s %s %.3f ", udpm, LTUORDER[ix], l2rate);
        ppoutrate= calc_rate(&ppout[ix], bufw32, caltime);
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
  int ixx;
  sprintf(spurline, "%s", dat); ixx=0;
  for(ix=0; ix<=(NCOUNTERS-1); ix++) {
    /*if(ix == spurcnts[ixx]) {
      sprintf(spurline, "%s %d", spurline, bufw32[ix]);
      ixx++;
      if(spurcnts[ixx]==-1) break;
    };*/
    sprintf(spurline, "%s %x", spurline, bufw32[ix]);
  };fprintf(spurfile,"%s\n", spurline);
  //printf("%s\n", spurline);
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

/*------------------------------*/ int main(int argc, char **argv) {
int inforc;
hname= getenv("HOSTNAME");
//setbuf(stdout, NULL);   nebavi
initbusyl0s();
//return(0);
rrdpipe= popen("/usr/bin/rrdtool -", "w");
if(rrdpipe==NULL) {
  printf("Cannot open /usr/bin/rrdtool -\n");
  exit(8);
};
//htmlpipe= popen("python ./htmlCtpBusys.py stdin >logs/htmlCtpBusys.log", "w");
//htmlpipe= popen("./htmlCtpBusys.py stdin", "w");
printf("%s rrdpipe opened, opening /tmp/htmlfifo... Is htmlCtpBusy daemon running?\n", hname);
htmlpipe= fopen("/tmp/htmlfifo", "w");    // mkfifo /tmp/htmlfifo
// waiting on above open until htmlCtpBusy is not started
if(htmlpipe==NULL) {
  printf("Cannot open /tmp/htmlfifo \n");
  exit(8);
};
setlinebuf(htmlpipe);
signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
signal(SIGUSR2, gotsignal); siginterrupt(SIGUSR2, 0);

csock_gcalib= udpopens("localhost", 9931);
if(csock_gcalib==-1) {printf("udpopens error\n"); /* exit(8);*/ };

//inforc= ftell(htmlpipe); printf("ftell:%d\n", inforc); always -1
//dbgout= fopen("logs/dbgout.log", "w");
inforc= dic_info_service("CTPDIM/MONCOUNTERS", MONITORED, 0, 
  cnts,4*(NCOUNTERS), gotcnts, 137, &cntsFailed, 4); 
//printf("CTPDIM/MONCOUNTERS service id:%d\n", inforc);
while(1) {
  sleep(100);
};
pclose(rrdpipe); pclose(htmlpipe);
//fclose(dbgout);
dic_release_service(inforc);
//udpclose(csock_gcalib);
return(0);
} 

