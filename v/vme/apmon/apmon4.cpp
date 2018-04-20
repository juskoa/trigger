/*
make apmon4
g++ -Wall -g3 -I/opt/dim/dim -DCPLUSPLUS -c -o apmon4.o apmon4.cpp
g++ -lpthread apmon4.o -L/opt/dim/linux -ldim -o apmon4

g++ -Wall -I/opt/dim/dim -DCPLUSPLUS -c -o apmon4.o apmon4.cpp
g++ -lpthread apmon4.o -L/opt/dim/linux -ldim -L/home/aj/git/trigger/v/vmeb/vmeblib/linux_s -lvmeb -L/home/aj/Downloads/redis-3.0.5/deps/hiredis -lhiredis -o apmon4

*/
#include <signal.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <unistd.h>   // usleep
#include <dic.hxx>
#include <time.h>
#include "ApMon.h"

int mydbConnect();
void mydbDisconnect();
void red_get_runs(int runs[6], int dets[6]);

int sendapm= 0;
ApMon *apm;
void my_handler(int s){
  printf("Caught signal %d\n",s);
  exit(1); 
}
// ----------------------------------------------
#define MAXGRUNS 100
#define MAXDETS 23

// DIM Client
typedef unsigned int w32;
#define Datalen 20
struct Data{
 w32 epchts;
 w32 epchtu;
 float btime,avbusy,l2arate;   // 0..1(busy), us, hz
};

class Detector : public DimInfo {
private:
  int lastepchsecs, n5;   // send out after 5 secs
  int nminr;   // Number of Measurements In Run   (1/s measurements)
  float average_ecs;
  Data *data; //float *dd;
  int detecs;
  char dimname[20];
  char sdname[20];
  int limit;
  int logactive;   // works with nminr, i.e. printing always 1st 6 measurements
  int received;
  char run_number[10];
  void printParameters(char *infostr, char *runnstr, char *detname, 
    int nParameters, char **paramNames, int *valueTypes, char **paramValues,
    long timestamp){
    char line[300];
    if((logactive==0) && (nminr>=6)){
      if(nminr==6) {
        printf("stoplog6 %d %s %s\n", timestamp, runnstr, detname);
      };
     return;
    };
    sprintf(line, "%s %d %s %s:%d ", infostr, timestamp,
    runnstr, detname, nParameters);
    for(int ix=0; ix<nParameters; ix++) {
      int val,typ;
      val= *(int *)paramValues[ix];
      typ= valueTypes[ix];
      sprintf(line, "%s %s:%d:%d", line, paramNames[ix], typ, val);
    }; strcat(line,"\n");
    printf(line); fflush(stdout);
  };
  void infoHandler() {
    long timestamp;
    int nParameters= 2;
    int sendecs, dsize; int avbusy=0;
    const char *paramNames[2]= {"busyLimit", "busyTime"};
    char *paramValues[2]= {(char *)&limit, (char *)&avbusy};
    //char *paramValuesECS[2]= {(char *)&limit, (char *)&average_ecs}; bad with private item
    int valueTypes[2]= { XDR_INT32, XDR_INT32 };
    //int valueTypes[2]= { 2, 2 };
    //paramValues[0]= (char *)limit;
    data= (Data *)getData();
    dsize= getSize();
    avbusy= data->avbusy;
    //if(strcmp(run_number,"0")==0) {   // Simulate a high busy time when not in a run
    //  avbusy= 10000;
    //};
    timestamp = data->epchts;   // time(NULL);
    /*printf("Data %s:%d: %d/%d %.4f %.4f %.4f\n", dimname, dsize,
      data->epchts,data->epchtu,data->btime,data->avbusy,data->l2arate);
    */
    // CTP only if no global run
    // CTP always in global run
    // apply average over 5 secs for data sent to ECS status display:
    received++;
    if((timestamp%1000)==0) {
      printf("received %s:%d at:%d\n", sdname, received, timestamp); fflush(stdout);
      received= 0;
    };
    if(strcmp(run_number,"0")!=0) {
      char istr[40];
      nminr= nminr+1;
      //if(nminr<=1) return;   // ignore 1st mesurement
      n5= n5+1; average_ecs= average_ecs + avbusy;
      if(lastepchsecs == 0) lastepchsecs= data->epchts;   // wait 5 secs after 1st measurement
      if(lastepchsecs+5 <= data->epchts) {
        sendecs= 1;   // send out A: last measured (nothing came) or B: average of last 1..5 values:
        lastepchsecs= data->epchts;
        average_ecs= average_ecs/n5;
      } else {
        sendecs= 0;
        sprintf(istr, "avrg n:%d", n5);
        printParameters(istr, run_number, sdname, 
          nParameters, (char **)paramNames, 
          valueTypes, paramValues, timestamp);
      };
      try {   // this det in run
        if( sendapm==1 ) {
          if( sendecs==1 ) {
            avbusy= average_ecs;
            sprintf(istr, "SEND n:%d", n5);
            n5= 0; average_ecs= 0;
            apm -> sendTimedParameters(run_number, sdname,
              nParameters, (char **)paramNames, 
              valueTypes, paramValues, timestamp);
            printParameters(istr, run_number, sdname, 
              nParameters, (char **)paramNames, 
              valueTypes, paramValues, timestamp);
          };
        } else {
          if( sendecs==1 ) {
            sprintf(istr, "nae  n:%d", n5);
            avbusy= average_ecs;
            n5= 0; average_ecs= 0;
          } else {
            sprintf(istr, "nane n:%d", n5);
          };
          printParameters(istr, run_number, sdname, 
            nParameters, (char **)paramNames, 
            valueTypes, paramValues, timestamp);
        };
      //} catch(runtime_error &e) {
      } catch(int e) {
        printf("Send operation failed: %d\n", e);
        //fprintf(stderr, "Send operation failed: %s\n", e.what());
        //exit(-1); 
      };
    };
    //usleep(2100000); //nebavi
    //tili.Update(t, name, *dd);
  };
public :
  //Detector(const char *dim_name, int detn, const char *sd_name, int lim): DimInfo(dim_name,-1) { dd = new float; 
  Detector(const char *dim_name, int detn, const char *sd_name, int lim): DimInfo(dim_name, (void *)&data, Datalen) {
    data = new Data;
    strcpy(dimname, dim_name); strcpy(run_number, "0");
    detecs= detn; 
    lastepchsecs= 0; n5= 0; average_ecs= 0; received= 0; logactive=0; nminr= 0;
    sprintf(sdname,"DET(%s)", sd_name); limit= lim;
  };
  void updateRunn(int runn) {
    sprintf(run_number, "%d", runn);
    printf("updateRunn: %s %s\n", sdname, run_number);
  };
  void resetRunn(char *runs) {
    if(strcmp(run_number,runs)==0) {
      strcpy(run_number, "0"); nminr= 0;
      printf("resetRunn: %s %s -> 0\n", sdname, runs);
    };
  };
  void printlogs(int logs) {
    if(logs==1) {
      logactive= 1;
    } else {
      logactive= 0;
    };
  };
}; 
class Detector *alldets[MAXDETS]; 
class Gruns : public DimInfo {
private:
  char *msg;
  void infoHandler() {
    int n, runn; unsigned int tstamp;
    msg= getString();
/*
s tstamp n dets -run n started with detectors dets(hex pattern without 0x, e.g.: bc9)
c tstamp 0      -clear all
c tstamp n      -run n stopped
L               -log
N               -no log /s
*/
    printf("GRUNS msg:%s.\n",msg); fflush(stdout);
    if(msg[0]=='s') {   // s tstamp n dets_hexpat
      unsigned int detpat;
      n= sscanf(&msg[2], "%d %d %x", &tstamp, &runn, &detpat);
      if(n!=3) {
        printf("ERROR: bad msg: %s\n", msg);
      } else {
        for(int ixd=0; ixd<MAXDETS; ixd++) {
          if(alldets[ixd]==NULL) continue;
          if( detpat & (1<<ixd) ) {
            alldets[ixd]-> updateRunn(runn);
          };
        };
      };
    } else if(msg[0]=='c') {   // c tstamp 0   c tstamp n
      n= sscanf(&msg[2], "%d %d", &tstamp, &runn);
      if(n!=2) {
        printf("ERROR: bad msg: %s\n", msg);
      } else {
        if(runn==0) {   // c tstamp 0
          for(int ixd=0; ixd<MAXDETS; ixd++) {
            if(alldets[ixd]==NULL) continue;
            alldets[ixd]-> updateRunn(0);
          };
        } else {
          char runs[20];
          sprintf(runs,"%d", runn);
          for(int ixd=0; ixd<MAXDETS; ixd++) {
            if(alldets[ixd]==NULL) continue;
            alldets[ixd]-> resetRunn(runs);
          };
        };
      };
    } else if(msg[0]=='L') {
      for(int ixd=0; ixd<MAXDETS; ixd++) {
        if(alldets[ixd]==NULL) continue;
        alldets[ixd]-> printlogs(1);
      };
    } else if(msg[0]=='N') {
      for(int ixd=0; ixd<MAXDETS; ixd++) {
        if(alldets[ixd]==NULL) continue;
        alldets[ixd]-> printlogs(0);
      };
    } else {
      printf("ERROR: bad msg: %s\n", msg);
    };
  }
public :
  Gruns(): DimInfo("CTPRCFG/GRUNS",-1) {
    int runs[6], dets[6];
    msg = new char[MAXGRUNS]; strcpy(msg, "empty");
    red_get_runs(runs, dets);
    for(int ixr=0; ixr<6; ixr++) {   // all runs
      if(runs[ixr]==0) break;   // list of runs finished by 0
      //sprintf(run_number,"%d", runs[ixr]);
      for(int ixd=0; ixd<MAXDETS; ixd++) {
        if(alldets[ixd]==NULL) continue;
        if( dets[ixr] & (1<<ixd) ) {
          alldets[ixd]-> updateRunn(runs[ixr]);
        };
      };
    };
  };
};

Gruns *gruns;
int main(int argc, char **argv) {
char *vmesite, *apmondir; char filename[40]="";
int rc, ecsn;
struct sigaction sigIntHandler;
sigIntHandler.sa_handler = my_handler;
sigemptyset(&sigIntHandler.sa_mask);
sigIntHandler.sa_flags = 0;
sigaction(SIGINT, &sigIntHandler, NULL);   // CTRL C

vmesite= getenv("VMESITE");
apmondir= getenv("APMON");
if(strcmp(vmesite,"ALICE")==0) {
  sprintf(filename, "%s/examples/apmonConfig_alice.conf", apmondir);
} else if(strcmp(vmesite,"SERVER")==0) {
  sprintf(filename, "%s/examples/apmonConfig_lab.conf", apmondir);
} else {
  strcpy(filename, "");
};
if(filename[0] == '\0') {
  apm= NULL;
  return 8;
} else {
  //ApMon *apm = new ApMon(filename);
  apm = new ApMon(filename);
};
printf("args:");
for(int ix=0; ix<argc; ix++) {
  if(ix==0) continue;
  if(strcmp(argv[ix], "apm")==0) {
    printf(" apm"); sendapm=1;
  } else {
    printf(" ?%s", argv[ix]);
  };
}; printf("\n");
printf("INFO ApMon instantiated.\n"); fflush(stdout);

for(int ix=0; ix<MAXDETS; ix++) {
  alldets[ix]= NULL;
};
ecsn=0 ; alldets[ecsn]= new Detector("spd/MONBUSY", ecsn, "SPD", 0);
ecsn=1 ; alldets[ecsn]= new Detector("sdd/MONBUSY", ecsn, "SDD", 1100);
ecsn=2 ; alldets[ecsn]= new Detector("ssd/MONBUSY", ecsn, "SSD", 310);
ecsn=3 ; alldets[ecsn]= new Detector("tpc/MONBUSY", ecsn, "TPC", 1300);
ecsn=4 ; alldets[ecsn]= new Detector("trd/MONBUSY", ecsn, "TRD", 600);
ecsn=5 ; alldets[ecsn]= new Detector("tof/MONBUSY", ecsn, "TOF", 0);

ecsn=6 ; alldets[ecsn]= new Detector("hmpid/MONBUSY", ecsn, "HMP", 320);

ecsn=7 ; alldets[ecsn]= new Detector("phos/MONBUSY", ecsn, "PHS", 200);
ecsn=8 ; alldets[ecsn]= new Detector("cpv/MONBUSY", ecsn, "CPV", 300);
ecsn=9 ; alldets[ecsn]= new Detector("pmd/MONBUSY", ecsn, "PMD", 600);
ecsn=10; alldets[ecsn]= new Detector("muon_trk/MONBUSY", ecsn, "MCH", 600);
ecsn=11; alldets[ecsn]= new Detector("muon_trg/MONBUSY", ecsn, "MTR", 200);

ecsn=12; alldets[ecsn]= new Detector("fmd/MONBUSY", ecsn, "FMD", 300);

ecsn=13; alldets[ecsn]= new Detector("t0/MONBUSY", ecsn, "T00", 0);
ecsn=14; alldets[ecsn]= new Detector("v0/MONBUSY", ecsn, "V00", 0);
ecsn=15; alldets[ecsn]= new Detector("zdc/MONBUSY", ecsn, "ZDC", 200);
ecsn=16; alldets[ecsn]= new Detector("acorde/MONBUSY", ecsn, "ACO", 200);
//ecsn=17; alldets[ecsn]= new Detector("trg/MONBUSY", ecsn, "TRI", 0);
ecsn=18; alldets[ecsn]= new Detector("emcal/MONBUSY", ecsn, "EMC", 300);
ecsn=19; alldets[ecsn]= new Detector("daq/MONBUSY", ecsn, "TST", 200);
ecsn=21; alldets[ecsn]= new Detector("ad/MONBUSY", ecsn, "AD0", 10);

rc= mydbConnect(); printf("mydbConnect rc:%d\n", rc);
gruns= new Gruns();
printf("mydbDisconnect...\n"); mydbDisconnect();
while(1) pause();
return 0;
}


