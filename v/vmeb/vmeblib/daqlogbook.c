/* daqlogbook.c
stdout: has to start with 'INFO ' or 'ERROR ' -routines here
are called from vme/pydim/server.c which is popened from pydimserver.py
11.12.2015
red_ added -update detectors in globruns
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexan.h"
#include "vmeblib.h"
#include "vmewrap.h"
#include "daqlogbook.h"
/* from Tpartition.h:
#define NCLUST 6
struct TDAQInfo
*/

#ifdef DAQLOGBOOK
#ifdef CPLUSPLUS
extern "C" {
#include "DAQlogbook.h"
/*von int DAQlogbook_update_LTUConfig(unsigned int run, const char *detector,
  unsigned int LTUFineDelay1, unsigned int LTUFineDelay2, 
  unsigned int LTUBCDelayAdd); */
}
#else
#include "DAQlogbook.h"
#endif
#endif
#define CTPLTUECSN 17

/*
rc: 0 -ok
1     -not opened not compiled with DAQLOGBOOK  (lab)
2     -not opened, daq logbook error in lab
-1    -not opened, run should be stopped (in the pit)
*/
int daqlogbook_open() {
char *vmesite;
int rco;
char DAQlogbookDB[120]="";
/* only if we need verbose output:
DAQlogbook_verbose(1);     */
vmesite=getenv("VMESITE");
if(vmesite !=NULL) {
  if(strcmp(vmesite,"ALICE")==0) {
    //DAQ_DB_LOGBOOK=trigger:trigger123@10.161.36.8/LOGBOOK
    //strcpy(DAQlogbookDB, "daq:daq@aldaqdb/LOGBOOK");   // was till 12.9.2010
    //strcpy(DAQlogbookDB, "trigger:trigger123@10.161.36.8/LOGBOOK");   // run1
    strcpy(DAQlogbookDB, "logbooktrg:ts0G9ce2@aldaqdb/LOGBOOK");
  } else if(strcmp(vmesite,"SERVER")==0) {
    strcpy(DAQlogbookDB, "daq:daq@pcald30/LOGBOOK_CTP");
    //strcpy(DAQlogbookDB, "daq:daq@137.138.143.230/LOGBOOK_CTP");   //pcald30's ip
    //strcpy(DAQlogbookDB, "daq:daq@128.141.139.225/LOGBOOK");  //slc6 (Franco)
  } else {
    strcpy(DAQlogbookDB, "");
  };
};
//printf("INFO Opening DAQlogbook:%s", DAQlogbookDB);
#ifdef DAQLOGBOOK
rco= DAQlogbook_open(DAQlogbookDB);
if(rco==-1) {
  printf("ERROR DAQlogbook_open %s failed\n",DAQlogbookDB);
  if(strcmp(vmesite,"ALICE")!=0) {
    rco=2;
  };
}else{
  printf("INFO DAQlogbook %s opened succesfuly.\n",DAQlogbookDB);
}
#else
printf("INFO daqlogbook_open: no attemt for DAQlogbook:  DAQLOGBOOK undefined");
rco=1;
#endif
return(rco);
}
int daqlogbook_close() {
int rc;
#ifdef DAQLOGBOOK
rc= DAQlogbook_close();
#else
rc=0;
#endif
if(rc==-1) {
  char em[]= "DAQlogbook_close failed"; prtError(em);
};
return(rc);
}

/* called from pydim/server.c */
//int daqlogbook_update_triggerClassName(unsigned int runN, unsigned char classN, char *value) {
int daqlogbook_update_triggerClassName(unsigned int run, unsigned char classId, const char *className, unsigned int classGroupId, float classGroupTime,const char *downscaling, const char **aliases)
{
int rc;
#ifdef DAQLOGBOOK
  //rc= DAQlogbook_update_triggerClassName(runN, classN, value);
  rc= DAQlogbook_update_triggerClassName(run, classId, className, 
classGroupId, classGroupTime, downscaling, aliases);
#else
  printf("INFO DAQlogbook_update_triggerClassName() not called");
  rc=0;
#endif
return(rc);
}
//int daqlogbook_update_triggerClassCounter(unsigned int runN, unsigned char classN, unsigned int L2count) {
int daqlogbook_update_triggerClassCounter(unsigned int run, unsigned char classId,  unsigned long long LMbCount, unsigned long long LMaCount, unsigned long long L0bCount, unsigned long long L0aCount, unsigned long L1bCount, unsigned long L1aCount, unsigned long L2bCount, unsigned long L2aCount, float ctpDuration)
{
int rc;
#ifdef DAQLOGBOOK
  //rc= DAQlogbook_update_triggerClassCounter(runN, classN, L2count);
  //rc=DAQlogbook_update_triggerClassCounter(run, classId, L0bCount, L0aCount, L1bCount, L1aCount, L2bCount, L2aCount, ctpDuration);
  rc=DAQlogbook_update_triggerClassCounter(run, classId, LMbCount, LMaCount, L0bCount, L0aCount, L1bCount, L1aCount, L2bCount, L2aCount, ctpDuration);
#else
  printf("INFO DAQlogbook_update_triggerClassCounter() not called");
  rc=0;
#endif
return(rc);
}
int daqlogbook_update_triggerGlobalCounter(unsigned int run, unsigned long L2aCount, float ctpDuration)
{
int rc;
#ifdef DAQLOGBOOK
  rc=DAQlogbook_update_triggerGlobalCounter(run, L2aCount, ctpDuration);
#else
  printf("INFO DAQlogbook_update_triggerGlobalCounter() not called");
  rc=0;
#endif
return(rc);
}
int daqlogbook_update_triggerDetectorCounter(unsigned int run, const char *detector, unsigned long L2aCount)
{
int rc;
#ifdef DAQLOGBOOK
 rc=DAQlogbook_update_triggerDetectorCounter(run, detector, L2aCount);
#else
  printf("INFO DAQlogbook_update_triggerDetectorCounter() not called");
  rc=0;
#endif
return(rc);
}
int daqlogbook_update_triggerClusterCounter(unsigned int run, unsigned int cluster, unsigned long L2aCount)
{
int rc;
#ifdef DAQLOGBOOK
 rc=DAQlogbook_update_triggerClusterCounter(run, cluster, L2aCount);
#else
  printf("INFO DAQlogbook_update_triggerClusterCounter() not called");
  rc=0;
#endif
return(rc);
}
int daqlogbook_insert_triggerInput(unsigned int run, unsigned int inputId, const char *inputName, unsigned int inputLevel)
{
int rc;
#ifdef DAQLOGBOOK
 rc= DAQlogbook_insert_triggerInput(run, inputId, inputName, inputLevel);
#else
  printf("INFO DAQlogbook_update_triggerInput() not called");
  rc=0;
#endif
return(rc);
}
int daqlogbook_update_triggerInputCounter(unsigned int run, unsigned int inputId, unsigned int inputLevel, unsigned long long inputCount)
{
int rc;
#ifdef DAQLOGBOOK
 rc= DAQlogbook_update_triggerInputCounter(run, inputId, inputLevel, inputCount);
#else
  printf("INFO DAQlogbook_update_triggerInputCounter() not called");
  rc=0;
#endif
return(rc);
}
int daqlogbook_add_comment(int runN,char *title,char *msg) {
int rc;
#ifdef DAQLOGBOOK
rc= DAQlogbook_add_comment(runN,title,msg);
if(rc!=0) {
  printf("INFO DAQlogbook_add_comment(0,%s,%s) rc:%d",title,msg,rc);
};
#else
  printf("INFO DAQlogbook_add_comment(0,%s,%s) not called",title,msg);
  rc=0;
#endif
return(rc);
}
/*==================
Description: This function should be called at SOR to insert the LTU
parameters needed by the offline. The detector parameter should be the
detector NAME (e.g. MUON_TRG), not the detector code (same as for the
DAQlogbook_update_triggerDetectorCounter function you already use).
*/
int daqlogbook_update_LTUConfig(unsigned int run, const char *detector,
  unsigned int LTUFineDelay1, unsigned int LTUFineDelay2, 
  unsigned int LTUBCDelayAdd) {
int rc;
#ifdef DAQLOGBOOK
rc= DAQlogbook_update_LTUConfig(run, detector, LTUFineDelay1, 
  LTUFineDelay2, LTUBCDelayAdd);
if(rc!=0) {
  printf("INFO DAQlogbook_update_LTUConfig(%d,%s,%d,%d,%d) rc:%d",run,detector,
    LTUFineDelay1, LTUFineDelay2, LTUBCDelayAdd, rc);
};
#else
printf("INFO DAQlogbook_update_LTUConfig(%d,%s,...) not called",
  run,detector);
rc=0;
#endif
return(rc);
}
#define ORBITL 3564
#define ORBITwb 3564/32+1
void storebcbit(int bc, unsigned int *uint32) {
int wix, bix;
wix= bc/32; bix= bc-32*wix; 
uint32[wix]= uint32[wix] | (1<<bix);
}
/*----------------------------------------------------------------*/
int daqlogbook_update_triggerConfig(int runn, char *mem, char *alignment, unsigned int inputDetectorMask) {
int rc;
#ifdef DAQLOGBOOK
rc= DAQlogbook_update_triggerConfig(runn, mem, alignment);
//following call can appear ONLY after previous one!
if(rc==0) {
  rc= DAQlogbook_update_triggerFilteredInputs(runn, inputDetectorMask);
} else {
  printf("ERROR daqlogbook_update_triggerConfig error:%d",rc);
};
#else
printf("INFO DAQlogbook_update_triggerConfig(%d,...) not called", runn);
rc=0;
#endif
return(rc);
}
/*
------------------Interface to DAQlogbook_insert_triggerCollisionSchedule()
*/
int daqlogbook_update_cs(unsigned int runn, char *cs_string) {
int rc;
int ix=0,slen;
unsigned long ACBEI[5]; // # of bits sets
unsigned long ACBEItr[5]; // # of words for DB (do not count 0s at the end)
char csName[200], daqlog_csName[205];
unsigned int beamA[ORBITwb], beamC[ORBITwb], colliding[ORBITwb], empty[ORBITwb],ignored[ORBITwb];

for(rc=0; rc<5; rc++) { ACBEI[rc]=0; ACBEItr[rc]=0; };
for(rc=0; rc<ORBITwb; rc++) {beamA[rc]=0; beamC[rc]=0; 
  colliding[rc]=0; empty[rc]=0; ignored[rc]=0;
};
rc= getNextLine(&cs_string[ix]); slen=rc-1;
if(slen > 0) {
  int loops;
  char year[8], datetime[20];
  strncpy(csName, cs_string, slen); csName[slen]='\0'; ix= ix+rc;
  getdatetime(datetime); // dmyhms:  string[20] dd.mm.yyyy hh:mm:ss */
  strncpy(year, &datetime[6], 4); year[4]='\0'; sprintf(daqlog_csName, "%s_%s", year, csName);
  //printf("INFO csName:%s slen:%d\n",csName, slen);
  for(loops=0; loops<3*ORBITL; loops++ ) {
    char *abce; int bc; char line[80];
    rc= getNextLine(&cs_string[ix]); slen=rc-1;
    //printf("INFO line:%s slen:%d\n",&cs_string[ix], slen);
    if(rc==0) break;   // EOF reached
    strncpy(line, &cs_string[ix], slen); line[slen]='\0'; ix= ix+rc;
    if(line[0]=='\n') continue;
    if(line[0]=='#') continue;
    abce= strpbrk(line, "ABCE");
    if(abce==NULL) { 
      printf("ERROR bad line in cs:len:%d:%s\n", slen, line);
      continue;
    };
    rc= sscanf(line,"%d", &bc);
    if(rc<1) { rc=-3; break; };   // bad format
    if(*abce == 'A') {storebcbit(bc, beamA); ACBEI[0]++;};
    if(*abce == 'C') {storebcbit(bc, beamC); ACBEI[1]++;};
    if(*abce == 'B') {storebcbit(bc, colliding); ACBEI[2]++;};
    if(*abce == 'E') {storebcbit(bc, empty); ACBEI[3]++;};
    abce= strchr(line, '*');
    if(abce != NULL) {storebcbit(bc, ignored); ACBEI[4]++;};
  };
  if(rc==0) {
    /* For DAQ we need 'number of truncated bytes' -
       i.e. last non-zero word is in: */
    //for(rc=ORBITwb-1; rc<=0; rc--)
    for(rc=0; rc<ORBITwb; rc++) {
      if(beamA[rc]!=0) ACBEItr[0]= rc;
      if(beamC[rc]!=0) ACBEItr[1]= rc;
      if(colliding[rc]!=0) ACBEItr[2]= rc;
      if(empty[rc]!=0) ACBEItr[3]= rc;
      if(ignored[rc]!=0) ACBEItr[4]= rc;
    };
    for(rc=0; rc<5; rc++) { ACBEItr[rc]= (ACBEItr[rc]+1)*4; }; // length in bytes
#ifdef DAQLOGBOOK
    rc= DAQlogbook_insert_triggerCollisionSchedule(runn, daqlog_csName, 
      beamA,ACBEItr[0], beamC,ACBEItr[1], colliding,ACBEItr[2], 
      empty,ACBEItr[3], ignored,ACBEItr[4]);
#else
    printf("INFO DAQlogbook_insert_triggerCollisionSchedule(%d,%s,...) not called", runn,daqlog_csName);
    rc=0;
#endif
    /* above OK */
    // printf("INFO daqlogbook_update_cs skipped\n");   //INVER
    printf("INFO number of bits ACBEI:%ld %ld %ld %ld %ld\n",
      ACBEI[0], ACBEI[1], ACBEI[2], ACBEI[3], ACBEI[4]);
    printf("INFO daqlogbook_update_cs %s ACBEI:%ld %ld %ld %ld %ld rc:%d\n",
      daqlog_csName, ACBEItr[0], ACBEItr[1], ACBEItr[2], ACBEItr[3], ACBEItr[4], rc);
  } else {
    printf("ERROR daqlogbook_update_cs error:%d",rc);
  };
} else {
  rc=-1;
};
if(rc!=0) {
  printf("INFO DAQlogbook_insert_triggerCollisionSchedule(%d,%s,...) rc:%d",
    runn,daqlog_csName, rc);
};
return(rc);
}
/*
Interface to DAQlogbook_insert_ACTConfig()
*/
int daqlogbook_update_ACTConfig(unsigned int runn, char *item, char *iname, char *iver) {
int rc;
#ifdef DAQLOGBOOK
printf("INFO daqlogbook_update_ACTConfig(%d part:%s instance:%s ver:%s\n",
  runn, item, iname, iver);
rc= DAQlogbook_insert_ACTConfig(runn, item, iname, iver);   // OK:
//    printf("INFO daqlogbook_update_cs skipped\n");   //INVER
#else
printf("INFO DAQlogbook_insert_ACTConfig(%d,...) not called", runn);
rc=0;
#endif
return(rc);
}
/* update clusters info in DAQlogbook (6 clusters)
   update redis db: hash gruns_dets runn:0xdetpattern
*/
int daqlogbook_update_clusters(unsigned int runn, char *pname,
  TDAQInfo *daqi, 
  unsigned int ignoredaqlog) {    // on vme available in shm
  //unsigned int effiout)          // inp. dets effectively filtered out 
int iclu,rc; w32 detpattern=0;
printf("INFO daqlogbook_update_clusters: pname:%s runn:%d\n", pname, runn);
for(iclu=0;iclu<NCLUST;iclu++) {
  if(daqi->masks[iclu]==0) continue;
  if(daqi->daqonoff==0) { // ctp readout active, set TRIGGER bit17 
    daqi->masks[iclu]= daqi->masks[iclu] | (1<<CTPLTUECSN);
  };
  detpattern= detpattern | daqi->masks[iclu];   // all dets in given run
  //printf("INFO daqlogbook_update_clusters: cluster:%d det/inp/class0-63/class64 mask:0x:%x %x %llx %llx effiout:0x%x\n", 
  printf("INFO daqlogbook_update_clusters: cluster:%d det/inp/class0-63/class64 mask:0x:%x %x %llx %llx\n", 
    iclu+1, daqi->masks[iclu], 
    daqi->inpmasks[iclu], daqi->classmasks00_063[iclu],
    daqi->classmasks64_100[iclu]); //, effiout);
#ifdef DAQLOGBOOK
  if(ignoredaqlog!=0) { rc=0;
    printf("INFO DAQlogbook_update_cluster(%d,...) not called(ignore daq)\n", runn);
  } else { 
    logbook_triggerClassMask_t classmask;
    classmask[0]=daqi->classmasks00_063[iclu];
    classmask[1]=daqi->classmasks64_100[iclu];
    rc=DAQlogbook_update_cluster(runn, iclu+1, daqi->masks[iclu], 
      pname, daqi->inpmasks[iclu], classmask);
      //pname, daqi->inpmasks[iclu], classmask, effiout);
      // not used here -updated only once for whole partition in DAQlogbook_update_triggerFilteredInputs
    if(rc!=0) {
      printf("ERROR DAQlogbook_update_cluster failed. rc:%d", rc);
      break;
    };
  };
#else
printf("INFO DAQlogbook_update_cluster(%d,...) not called\n", runn);
rc=0;
#endif
};
red_update_detsinrun(runn, detpattern);
return(rc);
}
/* not needed -called directly in uupdate_triggerConfig
int daqlogbook_update_triggerFilteredInputs(unsigned int runn, unsigned int inputDetectorMask) {
int iclu,rc;
#ifdef DAQLOGBOOK
// can be called after DAQlogbook_update_triggerConfig() !
rc= DAQlogbook_update_triggerFilteredInputs(runn, inputDetectorMask) {
#else
printf("INFO DAQlogbook_update_daqlogbook_update_triggerFilteredInputs(%d,...) not called\n", runn);
rc=0;
#endif
return(rc);
}*/
void printTDAQInfo(TDAQInfo *tdaq)
{
 printf("TDAQInfo: daqoonoff= %i \n",tdaq->daqonoff);
 printf("Clusters detector masks: \n");
 for(int i=0;i<NCLUST;i++)printf("%i=0x%x ",i,tdaq->masks[i]);
 printf("\n");
 printf("Input detector masks: \n");
 for(int i=0;i<NCLUST;i++)printf("%i=0x%x ",i,tdaq->inpmasks[i]);
 printf("\n");
 printf("Classmasks: \n");
 for(int i=0;i<NCLUST;i++)printf("%i=0x%llx 0x%llx \n",i,tdaq->classmasks00_063[i],tdaq->classmasks64_100[i]);
 printf("\n");
 printf("run1msg: %s \n",tdaq->run1msg);
}
