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
#define l0time 13

FILE *rrdpipe;
FILE *htmlpipe;
//FILE *dbgout=NULL;
FILE *spurfile=NULL;
int sock;

w32 prevl0time=0;
int firstreading=1;
unsigned int cntsFailed=0xdeaddeed;
unsigned int cnts[NCOUNTERS];

#define N24 24
char *LTUORDER[]={"SPD", "SDD", "SSD", "TPC", "TRD", "TOF", "HMPID",
  "PHOS", "CPV", "PMD", "MUON_TRK", "MUON_TRG",
  "FMD", "T0", "V0", "ZDC", "ACORDE", "-", "EMCAL", "DAQ",""}; 

#define NCS 6   // elapsed time for BUSY, L0,1,2,FO1, FO3

int allreads=0;
Tcnt1 cs[NCS];
Tcnt1 busy[N24];   // 24 busys, the same order as in VALID.LTUS
Tcnt1 l0s[N24];   // 24 l2strobess, the same order as in VALID.LTUS

Tcnt1 ppout[N24];   // 24 ppouts, not for each reading
Tcnt1 l2cal[N24];   // 24 ppouts, as l0s, but not for each reading
int avbusys[N24];      // usecs, -1: not connected   >999000: dead

#define MAXcalibDets 5
// TOF MUON_TRG T0 ZDC EMCAL
int calibDets[MAXcalibDets]={5,11,13,15,17};

char spurfilename[80]="xx";
char spurline[8000];
int spurcnts[]={150, 152, 153, -1};

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
  l0s[ix].reladdr=-1;      // not found in cnames.sorted2
  ppout[ix].reladdr=-1;      // not found in cnames.sorted2
  l2cal[ix].reladdr=-1;      // not found in cnames.sorted2
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
  isdet= isDetector(pl.ltuname);
  if((strncmp(pl.cname,"byin",4)==0) && 
     ( (pl.cname[4]>='1') &&(pl.cname[4]<='9') ) &&
     (isdet!=-1)) {
     busy[isdet].reladdr= pl.addr;
     continue;
  };
  //(strncmp(&pl.cname[3],"l0out",5)==0) && , define Yix 8
#define Yix 9
  if((strncmp(pl.cname,"fo",2)==0) && 
     (strncmp(&pl.cname[3],"l2stro",6)==0) && 
     ( (pl.cname[2]>='1') && (pl.cname[2]<='6') ) &&
     ( (pl.cname[Yix]>='1') && (pl.cname[Yix]<='4') ) &&
     (isdet!=-1)) {
     l0s[isdet].reladdr= pl.addr;
     l2cal[isdet].reladdr= pl.addr;
     continue;
  };
  if((strncmp(pl.cname,"fo",2)==0) && 
     (strncmp(&pl.cname[3],"ppout",5)==0) && 
     ( (pl.cname[2]>='1') && (pl.cname[2]<='6') ) &&
     ( (pl.cname[8]>='1') && (pl.cname[8]<='4') ) &&
     (isdet!=-1)) {
     ppout[isdet].reladdr= pl.addr;
     continue;
  };
};
fclose(cnames);
for(ix=0; ix<N24; ix++) {
  if(LTUORDER[ix][0]=='\0') break;
  printf("%s: %d\t%d\t%d\n",
    LTUORDER[ix], busy[ix].reladdr, l0s[ix].reladdr, ppout[ix].reladdr );
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
  prevl0time= bufw32[l0time]; \
  for(ix=0; ix<N24; ix++) { \
    int rad; \
    rad= l2cal[ix].reladdr; \
    if(rad != -1) { l2cal[ix].prevcs= bufw32[rad]; }; \
    rad= ppout[ix].reladdr; \
    if(rad != -1) { ppout[ix].prevcs= bufw32[rad]; }; \
  }; \

/*-----------------------*/ void gotcnts(void *tag, void *buffer, int *size) {
int ix; //,ixx;
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
    //fprintf(dbgout, "%u:", bufw32[ix]);
  };
}; 
//fprintf(rrdpipe, "%u \n", bufw32[NCOUNTERS-1]); fflush(rrdpipe); see above
epoch2date(bufw32[epochsecs], dat);

/*------------------------------------------------------------ html */
sprintf(htmlline, "busyL0 %s minute ", dat);
for(ix=0; ix<N24; ix++) {
  int rad,notdefined; w32 l0dif;
  notdefined=0;
  rad= busy[ix].reladdr;
  if(rad != -1) {
    busy[ix].prevcs= busy[ix].currcs; busy[ix].currcs= bufw32[rad];  
  } else {
    notdefined=1;
  };
  rad= l0s[ix].reladdr;
  if(rad != -1) {
    l0s[ix].prevcs= l0s[ix].currcs; l0s[ix].currcs= bufw32[rad];  
  } else {
    notdefined=1;
  };
  if(allreads==0) goto RTRN;
  if(notdefined==1) {
    sprintf(htmlline, "%s -", htmlline);
  } else {
    l0dif= dodif32(l0s[ix].prevcs, l0s[ix].currcs); if(l0dif==0)l0dif=1;
    //avbusys[ix]=100*ix;
    avbusys[ix]= round(dodif32(busy[ix].prevcs, busy[ix].currcs)*0.4/l0dif);
    if(avbusys[ix]>999900) {
      sprintf(htmlline, "%s dead", htmlline);
    } else {
      //sprintf(htmlline, "%s %d", htmlline, 100*ix);
      sprintf(htmlline, "%s %d", htmlline, avbusys[ix]);
    };
  };
}; strcat(htmlline,"\n");
printf("%u=%s: l2orbit:%u busytemp:%u busyvolts:%x \n", 
  bufw32[epochsecs], dat, bufw32[l2orbit], bufw32[CSTART_SPEC+3],
  bufw32[CSTART_SPEC+4]); 
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
  w32 timedelta;
  timedelta= dodif32(prevl0time, bufw32[l0time]);  // in 0.4micsecs
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
    prevl0time= bufw32[l0time];
    rcudpsend= udpsend(sock, (unsigned char *)udpm, strlen(udpm)+1);
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
printf("rrdpipe opened, opening /tmp/htmlfifo... Is htmlCtpBusy daeomn running?\n");
htmlpipe= fopen("/tmp/htmlfifo", "w");    // mkfifo /tmp/htmlfifo
if(htmlpipe==NULL) {
  printf("Cannot open /tmp/htmlfifo \n");
  exit(8);
};
setlinebuf(htmlpipe);
sock= udpopens("localhost", 9931);
if(sock==-1) {printf("udpopens error\n");  /*exit(8);*/ };
//inforc= ftell(htmlpipe); printf("ftell:%d\n", inforc); always -1
//dbgout= fopen("dbgout", "w");
inforc= dic_info_service("CTPDIM/MONCOUNTERS", MONITORED, 0, 
  cnts,4*(NCOUNTERS), gotcnts, 137, &cntsFailed, 4); 
//printf("CTPDIM/MONCOUNTERS service id:%d\n", inforc);
while(1) {
  sleep(100);
};
pclose(rrdpipe); pclose(htmlpipe);
//fclose(dbgout);
dic_release_service(inforc);
udpclose(sock);
return(0);
} 

