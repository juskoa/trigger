#include <stdio.h>
#include <stdlib.h>   //malloc
#include "string.h"
#include "vmewrapdefs.h"
#include "shmaccess.h"
//#include "bakery.h" is in Tpartition.h
#include "vmeblib.h"
//#include "ctp.h"
#include "Tpartition.h"

/*-------------------------------------------------------*/ void cshmInit() {
/* invoked from: 
altri1: ctp_proxy/ctp_proxy.c test.c main_shm.c
alitri: pydim/server.c 
*/
int i; Tpartitionshm *shmpart;
// 1 of the following 2 lines:
ctpshmbase= (Tctpshm *)mallocShared(CTPSHMKEY, sizeof(Tctpshm), &ctpsegid);
//ctpshmbase= (Tctpshm *)malloc( sizeof(Tctpshm)); ctpshmbase->datetime[0]='\0';
if(ctpshmbase->datetime[0]=='\0') {
  //printf("cshmInit: initializing ctp shm\n");
  getdatetime(ctpshmbase->datetime);
  shmpart= &ctpshmbase->startedParts[0];
  for(i=0;i<NDETEC;i++){ ctpshmbase->validLTUs[i].name[0]= '\0'; };
  for(i=0;i<NCTPINPUTS;i++){ ctpshmbase->validCTPINPUTs[i].name[0]= '\0'; };
  for(i=0;i<MNPART;i++){ shmpart[i].name[0]= '\0'; };
  ctpshmbase->GlobalFlags=0;
  ctpshmbase->active_cg=0;
  initBakery(&ctpshmbase->swtriggers, "swtriggers", 4);
  initBakery(&ctpshmbase->ccread, "ccread", 5);
};
if(validCTPINPUTs==NULL) {
  validCTPINPUTs= &ctpshmbase->validCTPINPUTs[0];
  validLTUs= &ctpshmbase->validLTUs[0];
};
}
void cshmDetach() {   // client should relese memory usage by calling this
detachShared((void *)ctpshmbase);
}
/*------------------------------------------------- */ void cshmClear() {
int i; Tpartitionshm *shmpart;
shmpart= &ctpshmbase->startedParts[0];
for(i=0;i<MNPART;i++){ 
  int ii;
  shmpart[i].name[0]= '\0'; 
  shmpart[i].run_number = 0;
  shmpart[i].paused = 0;
  for(ii=0; ii<NDETEC; ii++) {
    shmpart[i].Detector2Clust[ii]= 0x0;
  };
  //part->cshmpart= NULL; done in initTpartition()
};
}
/*------------------------------------------------- GlobalFlags: */
int cshmGlobFlag(w32 flag) {
if( ctpshmbase->GlobalFlags & flag) return(1);
return(0);
}
void cshmSetGlobFlag(w32 flag) {
ctpshmbase->GlobalFlags= ctpshmbase->GlobalFlags | flag;
}
void cshmClearGlobFlag(w32 flag) {
ctpshmbase->GlobalFlags= ctpshmbase->GlobalFlags & (~flag);
}
void setglobalflag(int argc,char **argv,char *flagName,int flag) {
char onoff[8];
if(isArg(argc, argv, flagName)) {
  cshmSetGlobFlag(flag); strcpy(onoff,"ON");
} else {
  cshmClearGlobFlag(flag); strcpy(onoff,"OFF");
};
printf("%s:%s\n", onoff, flagName);
}   
int cshmBM() {
return((ctpshmbase->GlobalFlags & FLGBMmask)>>8);
}
void cshmSetBM(w32 newbm) {
ctpshmbase->GlobalFlags= ctpshmbase->GlobalFlags & (~FLGBMmask);
ctpshmbase->GlobalFlags= ctpshmbase->GlobalFlags | ((newbm<<8) & FLGBMmask);
}
/*---------------------------------------------------cshmGlobalDets()
rc: mask of all detectors in all global runs (paused partitions EXCLUDED)
*/
int cshmGlobalDets() {
int i,gdets=0; Tpartitionshm *shmpart;
shmpart= &ctpshmbase->startedParts[0];
for(i=0;i<MNPART;i++){ 
  int id;
  if(shmpart[i].name[0] == '\0') continue;
  if(shmpart[i].paused == 1) continue;   // paused partition
  for(id=0;id<NDETEC;id++){
    w32 logclu;
    logclu= shmpart[i].Detector2Clust[id];
    if(logclu==0) continue;
    gdets= gdets | (1<<id);
  };
}; return(gdets);
}
/*----------------------------------------------------- cshmPrint()
*/
void cshmPrint() {
int i; Tpartitionshm *shmpart;
shmpart= &ctpshmbase->startedParts[0];
printf("VALID.LTUS:\n");
for(i=0;i<NDETEC;i++){ 
  if(ctpshmbase->validLTUs[i].name[0] != '\0'){
    printf("%d:%s fo:%d:%d %d %s\n", i, ctpshmbase->validLTUs[i].name,
      ctpshmbase->validLTUs[i].fo, ctpshmbase->validLTUs[i].foc,
      ctpshmbase->validLTUs[i].busyinp, ctpshmbase->validLTUs[i].ltubasea);
  };
};
printf("GlobalFlags:%x active_cg:%d beammode:%d\n", ctpshmbase->GlobalFlags,
  ctpshmbase->active_cg, cshmBM());
printBakery(&ctpshmbase->swtriggers);
printBakery(&ctpshmbase->ccread);
/*printf("VALID.CTPINPUTS: name Level/1..24 edge delay\n");
for(i=0;i<NCTPINPUTS;i++){ 
  if(ctpshmbase->validCTPINPUTs[i].name[0] != '\0'){
    printf("%8s %d/%2d %d %2d\n", ctpshmbase->validCTPINPUTs[i].name,
      ctpshmbase->validCTPINPUTs[i].level, 
      ctpshmbase->validCTPINPUTs[i].inputnum,
      ctpshmbase->validCTPINPUTs[i].edge,
      ctpshmbase->validCTPINPUTs[i].delay);
  };
};*/
printf("Partitions:\n");
for(i=0;i<MNPART;i++){ 
  if(shmpart[i].name[0] != '\0'){
    printf("%s/%d paused:%d ", shmpart[i].name, shmpart[i].run_number, 
      shmpart[i].paused);
    printDetector2Clust(&shmpart[i].Detector2Clust[0]);
  };
};
printf("Detectors in global runs:%x\n", cshmGlobalDets());
printf("l0f1..4 lm1..4:\n");
for(i=0;i<8;i++){ 
  char lnam[4]; int ixl;
  if(i<4) {strcpy(lnam, "l0f"); ixl= i+1;
  } else {strcpy(lnam, "lmf"); ixl= i-3;}
  printf("%s%d: %s\n", lnam,ixl, &ctpshmbase->lut88[(i)*LUT8_LEN]);
};
printf("intl0f1..3 intlm1..3:\n");
for(i=0;i<6;i++){ 
  char lnam[4]; int ixl;
  if(i<3) {strcpy(lnam, "intl0f"); ixl= i+1;
  } else {strcpy(lnam, "intlmf"); ixl= i-2;}
  printf("%s%d: %s\n", lnam,ixl, &ctpshmbase->intlut88[(i)*LUT8_LEN]);
};

}

/*-----------------------------*/    void cshmAddPartition(Tpartition *part) {
int i; Tpartitionshm *shmpart;
//shmpart= ctpshmbase->startedParts;
shmpart= &ctpshmbase->startedParts[0];
for(i=0;i<MNPART;i++){
  if(shmpart[i].name[0] == '\0'){
    shmpart[i].run_number = part->run_number;
    shmpart[i].paused = 0;
    copyDetector2Clust(&shmpart[i].Detector2Clust[0], &part->Detector2Clust[0]);
    strcpy(shmpart[i].name, part->name);
    part->cshmpart= &ctpshmbase->startedParts[i];
    return;
  };
 };
}
/*--------------------------------------*/ void cshmDelPartition(char *pname) {
int i; Tpartitionshm *shmpart;
shmpart= &ctpshmbase->startedParts[0];
for(i=0;i<MNPART;i++){
  if(strcmp(pname, shmpart[i].name) == 0){
    shmpart[i].name[0]= '\0';
  };
};
}
/*------------------------------*/ void cshmPausePartition(Tpartition *p) {
p->cshmpart->paused=1;
}
/*------------------------------*/ void cshmResumePartition(Tpartition *p) {
p->cshmpart->paused=0;
}
/*------------------------------*/ w32 cshmQueryPartition(Tpartition *p) {
return(p->cshmpart->paused);
}

/*------------------------------ lut8; hw copy */
int cshmsetLUT(int lutn, char *lutt) {
//printf("cshmsetLUT: %d %x\n", lutn, lutt);
strcpy(&ctpshmbase->lut88[(lutn-1)*LUT8_LEN], lutt);
return(0);
}
/*------------------------------ lut8; hw copy 
in: lutn: 0..7 (corresponds to l0f1..4 lm1..4) */
int cshmgetLUT(int lutn, char *lutt) {
strcpy(lutt, &ctpshmbase->lut88[(lutn-1)*LUT8_LEN]);
// nemoz lebo py printf("cshmgetLUT: %d %x\n", lutn, lutt);
return(0);
}


/*------------------------------ intlut8; hw copy */
int cshmsetintLUT(int lutn, char *lutt) {
//printf("cshmsetintLUT: %d %x\n", lutn, lutt);
strcpy(&ctpshmbase->intlut88[(lutn-1)*LUT8_LEN], lutt);
return(0);
}
/*------------------------------ intlut8; hw copy 
in: lutn: 0..5 (corresponds to l0f1..4 lm1..4) */
int cshmgetintLUT(int lutn, char *lutt) {
strcpy(lutt, &ctpshmbase->intlut88[(lutn-1)*LUT8_LEN]);
// nemoz lebo py printf("cshmgetLUT: %d %x\n", lutn, lutt);
return(0);
}
