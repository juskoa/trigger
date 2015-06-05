/* ctp_proxy:
16.12.2007:
- infoLogger added (only 1 message: see generateXOD()).
  facility  "CTP"
  stream    "" or part.name if available
  system    TRG (default set in /opt/infoLogger)
- xcounters added (uses offline tag:CTP_xcounters) -not tested: 
  todo: uncomment xcountersStart/Stop
17.12.
DAQlogbook_verbose(1);      -to be removed later
- EOR actions: uses online tag CTP_runconfig
18.12.
EOD failure: does not cause ctp_proxy to go to ERRROR state. BUT e.g.
failure in load2HW leaves ctp_proxy in ERROR state -it can have serious
consequences for active partitions (if any)
18.12
prepareRunConfig/registerRunConfig
29.11. todo: goto LOAD_FAILURE  when more than 6 * (inverted) classes used
30.5.2015 TEST: when debug offline (no swtrg generation)
*/
//#define TEST
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "infolog.h"
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif

#include "vmewrap.h"
#include "shmaccess.h"
#include "vmeblib.h"
#include "lexan.h"
#include "daqlogbook.h"
#include "ctp.h"
#include "ctplib.h"
#include "ssmctp.h"
#include "Tpartition.h"
int getDAQClusterInfo(Tpartition *part, TDAQInfo *daqi);

#include "ctp_proxy.h"
//#include "../ltu/ltu.h"     from ltu.h file:
#define LTUNCOUNTERS   21
#define COPYCOUNT      0x1d4   /*dummy wr. copy counters CMD */
#define COPYCLEARADD   0x1dc   /*dummy wr. clear copy mem. add. */
#define COPYREAD       0x1e0   /*ro copy memory data */

#define FOMODE 0 

extern int quit;

// Partition HW is the final configuration which is loaded to hw.
// HW = part[0]+part[1] + .... +part[5]
Hardware HWold;

int get_fixed(char *rcfg, int *fixpos);
int lenfixpos=2;
int fixpos[53];   // run, clgroup, rel1,rel2,...,-2
static TDAQInfo daqi;

#define TAGstartcount 333
#define TAGstopcount 334
#define TAGprintruns 335
#define TAGrcfgdo 336
#define TAGrcfgdelete 337
#define TAGctprestart 338
#define TAGpcfg 339
#define TAGstartcountforced 340
#define TAGglobalcal 350
#define TAGresetclock 351
int pydimok=0;
void callback(void *tag, int *retcode){
 char command[100]; char emsg[300];
 //printf("callback: %li %i \n",*tag,*retcode);
 switch(*(int *)tag){
   case(TAGstartcount):
   case(TAGstartcountforced):
        strcpy(command,"STARTRUNCOUNT");
        break;
   case(TAGstopcount):
        strcpy(command,"STOPRUNCOUNT");
        break;
   case(TAGrcfgdo):
        strcpy(command,"CTPRCFG/RCFG");
        break;
   case(TAGrcfgdelete):
        strcpy(command,"CTPRCFG/RCFG delete");
        break;
   case(TAGctprestart):
        strcpy(command,"CTPRCFG/RCFG delete(testifpydimON)");
        break;
   case(TAGpcfg):
        strcpy(command,"CTPRCFG/RCFG pcfg");
        break;
   case(TAGglobalcal):
        strcpy(command,"CTPCALIB/DO u");
        break;
   case(TAGresetclock):
        strcpy(command,"CTPRCFG/RCFG resetclock");
        break;
   default:
        printf("callback: Unknown tag %i \n",*(int *)tag);
        return;
 }
 if(*retcode){
   if(*(int *)tag!=TAGstartcountforced) {
     sprintf(emsg, "timestamp:callback:%s successful.",command); 
     prtLog(emsg);
   };
   if(*(int *)tag==TAGctprestart) pydimok=1;
 } else {
   sprintf(emsg, "timestamp:callback:DIM command %s failed.",command); 
   prtError(emsg);
 };
}
/*
Input:
part: pointer to Tpartition
      NULL: move  all .rcfg files in RCFG directory to delmeh/
dodel: 
1: prepare file. Called from:
  - _LoadPartition(, 1)
0: delete file   called from:
  - _LoadPartition( ,0) -in case of error and if it was created
  - _StartPartition( ,0) -in case of error 
  - _StopPartition( ,0)
rc: 0: ok .rcfg prepared, (confirmed from parted)
       or all .rcfg files moved to delmeh/
    1
    2 -bad dodel
    3 rc from dic_cmnd_servcie !=1 (i.e. dim problem)
daqi: global structure seen in ctp_proxy only
*/
int prepareRunConfig(Tpartition *part, int dodel) {
/* 17.12.2007: just copy
$VMECFDIR/CFG/ctp/pardefs/partname.pcfg ->
$VMEWORKDIR/RCFG/rRUNNUMBER.rcfg
*/
int rc=0, icla, tag, rcdic; w32 ilog;
char namemode[80];
//char *environ; char runcfg[100];
char cmd[400];
char dimcom[40];
char msg[500];
if(part == NULL){ dodel=2;
 /*intError("prepareRunConfig: part=NULL");
 rc=1;return(rc);*/
} else {
  if(part->hwallocated != 0x3) {
    char emsg[300];
    sprintf(emsg, "hwallocated:%x (0x2 expected) for part:%s", 
      part->hwallocated, part->name);
    intError(emsg);
    rc=1; return(rc);
  };
  if(part->partmode[0]!='\0') {
    strcpy(namemode, part->partmode);
  } else {
    strcpy(namemode, part->name);
  };
};
if(dodel==1) {
  w32 int1lookup,int1def,int2lookup,int2def;
  tag=TAGrcfgdo;
  //printTpartition("prepareRunConfig1", part);
  sprintf(cmd,"rcfg %s %d 0x%x", namemode, part->run_number,
    part->MaskedDetectors);
  for(ilog=0;ilog<NCLUST;ilog++){
    int hwclu, ihw;
    hwclu=0;
    for(ihw=0;ihw<NCLUST;ihw++){
      if(part->ClusterTable[ihw]==0) continue;
      if(part->ClusterTable[ihw]==(ilog+1)) {hwclu=ihw+1; break;};
    };
    printf("%i -> %i hwclu:%d,",ilog,part->ClusterTable[ilog], hwclu);
    sprintf(cmd, "%s %d", cmd, hwclu);
  }; printf("\n");
  for(icla=0;icla<NCLASS;icla++){
    TKlas *klas;
    klas=part->klas[icla];
    if(klas!=NULL) {
      printTKlas(klas,icla);
      sprintf(cmd,"%s %d", cmd, klas->hwclass+1);
    } else {
      sprintf(cmd,"%s 0", cmd);
    };
  };
  // int1lookup int1def int2lookup int2def
  //int2lookupdef bug
  int1lookup= getLM0addr(L0_INTERACT1);
  int2lookup= getLM0addr(L0_INTERACT2);
  int2def= getLM0addr(L0_INTERACTSEL);
  int1def= int2def & 0x1f; int2def= int2def >> 5;
  sprintf(cmd, "%s 0x%x 0x%x 0x%x 0x%x", cmd, int1lookup, int1def, int2lookup, int2def);
  strcat(cmd,"\n");
  //printf("prepareRunConfig:%s \n",cmd);
} else if(dodel==0) {
  tag=TAGrcfgdelete;
  sprintf(cmd,"rcfgdel %s %d\n", namemode, part->run_number);
} else if(dodel==2) {
  tag=TAGrcfgdelete;
  sprintf(cmd,"rcfgdel ALL 0\n");
} else {
  char emsg[300];
  sprintf(emsg, "prepareRunConfig:dodel:%d", dodel); 
  intError(emsg);
  rc=2; return(rc);
};
sprintf(msg,"timestamp:prepareRunConfig1: %s", cmd); prtLog(msg);
if(dodel==1) {
  // new way for rcfg command:
  //
  strncpy(daqi.run1msg, cmd, MAXRUN1MSG);
  rcdic= dic_cmnd_service("CTPRCFG", (void *)&daqi, sizeof(TDAQInfo));
} else {
  strcpy(dimcom,"CTPRCFG/RCFG");
  //dic_cmnd_callback(dimcom, cmd, strlen(cmd)+1, &callback, tag);
  rcdic= dic_cmnd_service(dimcom, cmd, strlen(cmd)+1);
};
if(rcdic!=1) rc= 3;   // dim problem?
sprintf(msg,"timestamp:prepareRunConfig2:tag:%d rc %d", tag, rcdic); prtLog(msg);
return(rc);
}

/*------------------------------------------------ preparepcfg()
*/
void preparepcfg(char *partname, int runnumber, char *ACT_CONFIG) {
int rcdic;
char cmd[400];
char dimcom[40];
char msg[500];
if(strcmp(ACT_CONFIG,"NO")==0) {
  sprintf(cmd,"Ncfg %s %d\n", partname, runnumber);
} else {
  sprintf(cmd,"pcfg %s %d\n", partname, runnumber);
};
sprintf(msg,"timestamp:pcfg1: %s", cmd); prtLog(msg);
strcpy(dimcom,"CTPRCFG/RCFG");
//dic_cmnd_callback(dimcom, cmd, strlen(cmd)+1, &callback, TAGpcfg);
rcdic= dic_cmnd_service(dimcom, cmd, strlen(cmd)+1);
sprintf(msg,"timestamp:pcfg1:rc %d", rcdic); prtLog(msg);
return;
}
/*----------------------------------------------void getNAllPartitions();
*/
int getNAllPartitions() {
int i,rc=0;
for(i=0; i<MNPART; i++) {
  if(AllPartitions[i]!=NULL) 
    rc++;
}; return(rc);
}
/*----------------------------------------------void printStartedTp();
*/
void printStartedTp() {
int i;
for(i=0; i<MNPART; i++) {
  if(StartedPartitions[i]!=NULL) 
    printf("   %d: %s\n", i, StartedPartitions[i]->name);
}; printf("\n");
}
/*----------------------------------------------void printAllTp();
*/
void printAllTp() {
int i;
for(i=0; i<MNPART; i++) {
  if(AllPartitions[i]!=NULL) 
    printf("   %d: %s\n", i, AllPartitions[i]->name);
}; printf("\n");
}
/*------------------------------------------------------ checkInputs()

int checkInputs(Tpartition *part){
return(0);
} */
/*---------------------------------------------------finditem2checkRBIF()
  Find item to check in rbif.
*/
int finditem2checkRBIF(int icheck,int *item1,int *item2){
 int i1,i2; char msg[100];
 switch(icheck){  // find what to check
  case ixrnd1:
  case ixrnd2:
        i1=ixrnd1;i2=ixrnd2;break;
  case ixbc1:
  case ixbc2:
        i1=ixbc1;i2=ixbc2;break;
  case ixl0fun1:
  case ixl0fun2:
        i1=ixl0fun1;i2=ixl0fun2;break;
  /* INT FUNCTS SHOULD BE TREATED ALSO LIKE l0f ? */
  case ixintfun1:
  case ixintfun2:
        i1=ixintfun1;i2=ixintfun2;break; 
  case ixlut3132:
  case ixlut4142:
        i1=ixlut3132;i2=ixlut4142;break; 
  default:
       sprintf(msg, "finditem2checkRBIF: item %i does not exist.",icheck);
       intError(msg);
       return 1;
 } 
 *item1=i1;
 *item2=i2;
 return 0;
}
//-------------------------------------------------
int compRESVal(TRBIF *cumrbif, int i1, TRBIF *prbif, int icheck) { //rc:0 if equal
int rc,cum1,cum2,ic1,ic2;
rc= finditem2checkRBIF(i1,&cum1,&cum2); if(rc!=0) return(1);
rc= finditem2checkRBIF(icheck,&ic1,&ic2); if(rc!=0) return(1);
if( cum1 != ic1 ) {
  char msg[100];
  sprintf(msg,"compRESVal(,%d,,%d) -incorrect parameter(s)",i1,icheck);
  intError(msg); return(1);
};
rc=1;
if(cum1!=ixlut3132) {
  if(cumrbif->rbif[i1] == prbif->rbif[icheck]) rc=0;
} else {
  int indx; w8 *dst8,*src8;
  if(i1==ixlut3132) { dst8= &cumrbif->lut34[0];
  } else { dst8= &cumrbif->lut34[LEN_l0f34/2];};
  if(icheck==ixlut3132) { src8= &prbif->lut34[0];
  } else { src8= &prbif->lut34[LEN_l0f34/2];};
  rc=0;
  for(indx=0; indx<LEN_l0f34/2; indx++) {   // check all bytes in VALUE
    if(dst8[indx]!= src8[indx]) {rc=1; break;};
  };
}; return(rc);
}
/*-------------------------------------------- allocateInhw
Allocate prbif/icheck in:
 - cumrbif/ixhw
 - prbif/ixhw    
icheck,ixhw: ixrnd1,ixrnd2,...,ixlut4142
todo: also for PF and masks...
*/
void allocateInhw(TRBIF *cumrbif, int ixhw, TRBIF *prbif, int icheck ) {
char msg[300];
cumrbif->rbifuse[ixhw] = ixhw;
cumrbif->rbif[ixhw] = prbif->rbif[icheck];  //VALUE (not enough for l0f34)
if((ixhw==ixl0fun1) || (ixhw==ixl0fun2) ||
   (ixhw==ixlut3132) || (ixhw==ixlut4142)) {
  char *src, *dst, *dst2; 
  if((ixhw==ixl0fun1) || (ixhw==ixl0fun2)) {  // symb. definition only
    src= &prbif->l0intfs[(icheck-ixl0fun1)*L0INTFSMAX];
    dst= &cumrbif->l0intfs[(ixhw-ixl0fun1)*L0INTFSMAX];
    dst2= &prbif->l0intfs[(ixhw-ixl0fun1)*L0INTFSMAX];
    strcpy(dst2, src);  // ???
  } else {                     // l0f34: VALUE + symb. def
    w8 *dst8, *src8; int indx; 
    // VALUE for l0f3: lut3132[0..2047] for l0f4: lut4142[2048..4095]
    if(ixhw==ixlut3132) { dst8= &cumrbif->lut34[0];
    } else { dst8= &cumrbif->lut34[LEN_l0f34/2];};
    if(icheck==ixlut3132) { src8= &prbif->lut34[0];
    } else { src8= &prbif->lut34[LEN_l0f34/2];};
    for(indx=0; indx<LEN_l0f34/2; indx++) {   // VALUE
      dst8[indx]= src8[indx];
    };
    // symb. def:
    if(icheck==ixlut3132) src= &prbif->l0f3sym[0];
    if(icheck==ixlut4142) src= &prbif->l0f4sym[0];
    if(ixhw==ixlut3132) {dst= &cumrbif->l0f3sym[0]; dst2= &prbif->l0f3sym[0];};
    if(ixhw==ixlut4142) {dst= &cumrbif->l0f4sym[0]; dst2= &prbif->l0f4sym[0];};
    // analogy of above (l0f12)
    strcpy(dst2, src);  // ???
  };
  strcpy(dst, src);   // copy symb. definition l0f1/2/3/4
  sprintf(msg,"================allocateInhw l0f:%d in %d:%s",icheck,ixhw,dst); 
} else {
  sprintf(msg,"================allocateInhw:%d in %d:%d",icheck,ixhw,cumrbif->rbif[ixhw]); 
};
  printf("%s\n",msg);
}
/*------------------------------------------------------ checkpairRBIF()
Purpose: to check availibilty of resources for pair of items in RBIF, 
         i.e: (rnd1,rnd2),(bc1,bc2),(int1,int2), (l0f1,l0f2), (l0f3,l0f4)
icheck   -the resourse number to be checked:ixrnd1..ixintfunt, ..., ixlut3132, ixlut4142
*cumrbif -here all resources over ALL partitions are accumulated
*prbif   -only resources for ONE partition are accumulated

Called: from cumRBIF, i.e.:
LOAD_PARTITION phase:
- when resources checked (see DOC/devdbg/sharedResources)
- when adding partition to AllPartitions
START_PARTITION:
- when adding partition to StartedPartitions

Output:
prbif modified if reallocation done
rc: 1: error
    0: ok, i.e.:
       -prbif/icheck not used
       -used already: the value of icheck resource is the same (reallocation
                      done if needed)
       -not used yet: resource was allocated
*/
char *rsrcs_names[]={"rnd1","rnd2","bc1","bc2","l0f1","l0f2",
  "intfun1","intfun2","intfunt", "l0f3", "l0f4"};

int checkpairRBIF(int icheck,TRBIF *cumrbif,TRBIF *prbif){
int i1,i2,ixhw,iused; char msg[100];
ixhw=prbif->rbifuse[icheck];   // position in hw where saved 
if(ixhw == notused) {
  //printf("checkpairRBIF: hwallocated icheck:%d ixhw:%d=notused\n", icheck, ixhw);
  return 0;  // not used in part
};
if(ixhw != nothwal){           // allocated in hw already:
  //check if the same value, if not check other one whether it is available or the same
  if(DBGcumRBIF) {
    printf("checkpairRBIF: hwallocated icheck:%d ixhw:%d\n", icheck, ixhw);
  };
  if(cumrbif->rbifuse[ixhw] == notused){ // add to cum
    allocateInhw(cumrbif, ixhw, prbif, icheck);
    // todo: we should check, if other one is not the same -if yes, reallocation
    //       should be done. Now, it may happen, 2 resources are allocated with the
    //       same value, i.e. when new partition can be refused for lack of resources
    return 0;
  } else {   // used already, check if values equal
    if(cumrbif->rbif[ixhw] != prbif->rbif[icheck]){
      sprintf(msg, "checkpairRBIF: %x rbif already allocated.",ixhw);
      intError(msg);return 1;
    }else return 0;      
  };
};
// Here only 'not yet in hw resources', i.e.new partition:
if(finditem2checkRBIF(icheck,&i1,&i2)) return 1;
iused=(cumrbif->rbifuse[i1]==notused)+((cumrbif->rbifuse[i2]==notused)<<1);
if(DBGcumRBIF) {
  printf("checkpairRBIF: NOThwallocated icheck:%d ixhw:%d i1/2:%d %d iused:%d\n", 
    icheck, ixhw, i1,i2, iused);
};
/* used:
   3: none used
   2: i1 only already used
   1: i2 only already used
   0: both used already, check if the one already taken can be used (i.e. equal value):
Modifications for new l0f34:
- complex check/copy for value (2048 bytes) todo (23.1.2012: done only for case 3!)
*/
switch(iused){
  case 3:  // none used, allocate prbif/icheck in cumrbif/i1
    allocateInhw(cumrbif, i1, prbif, icheck);
    prbif->rbifuse[icheck]=i1;   // possible reallocation
    return 0;
  case 2: // i1 already used
    if(compRESVal(cumrbif, i1, prbif, icheck)==0){ 
      //new resource equal to the existing one, just use it:
      prbif->rbifuse[icheck]=i1; 
      if(DBGcumRBIF) {
        printf("equal resource %d alllocated in %d\n", icheck, i1);
      };
    }else{  // use available i2
      allocateInhw(cumrbif, i2, prbif, icheck);
      prbif->rbifuse[icheck]=i2;
      if(DBGcumRBIF) {
        printf("resource %d alllocated in free %d\n", icheck, i2);
      };
    };
    return 0;
  case 1: // i2 already used
    if(compRESVal(cumrbif, i2, prbif, icheck)==0){ 
      //new resource equal to the existing one
      prbif->rbifuse[icheck]=i2; 
    }else{  // store in free i1
      allocateInhw(cumrbif, i1, prbif, icheck);
      prbif->rbifuse[icheck]=i1;
    };
    return 0;
  case 0: // resource not available, check if the one taken can be used (equal value):
    if(compRESVal(cumrbif, i1, prbif, icheck)==0){ 
      prbif->rbifuse[icheck]=i1;
      return 0;
    } else if(compRESVal(cumrbif, i2, prbif, icheck)==0){ 
      prbif->rbifuse[icheck]=i2;
      return 0;
    }else{
     //printf("checkpairRBIF: resource %i not available\n",icheck);
     char errmsg[300];
     sprintf(errmsg, "Shared resource %s(%i) not available\n", 
       rsrcs_names[icheck], icheck);
     infolog_trgboth(LOG_FATAL, errmsg);
     return 2;
    }
  default:
     sprintf(msg, "checkpairRBIF: iused=%i",iused);
     intError(msg); return 1;
 }
}
/*
bcmi: 0..11 -bit number of the mask (corresponds to bcm1..12) to be checked
      if it is equal
rc: 1 ok, the same masks
*/
int checkBCMaskBits(TRBIF *cumrbif, TRBIF *prbif, int bcmi) {
int bit, ix, rccode=1;
bit=1<<bcmi;
for(ix=0; ix<ORBITLENGTH; ix++) {
  int bcm, bcm2;
  bcm= prbif->BCMASK[ix]; bcm2= cumrbif->BCMASK[ix];
  if((bcm & bit) != (bcm2 & bit)) { rccode=0; break; };
};
return(rccode);
}
/*delme char int12hex(int m) {
char c;
m=m&0xf;
if(m<9) {
  c= '0'+ m;
} else {
  c= 'a'+ (m-10);
}; return(c);
} */
/*
bcmi: 0..11 -bit number of the mask (corresponds to bcm1..12)
*/
void copyBCMaskBits(TRBIF *cumrbif, TRBIF *prbif, int bcmi) {
int bit,ix,bcmaskn, flag=0;
if(l0AB()==0) {bcmaskn=BCMASKN;} else {bcmaskn=4;};
for(ix=0; ix<bcmaskn; ix++) {
  //printf("copyBCMaskBits:BCMASKuse[%d]:%d\n", ix, cumrbif->BCMASKuse[ix]);
  if( cumrbif->BCMASKuse[ix] != 0 ) {
    flag=1; break;
  };
};
bit=1<<bcmi;
for(ix=0; ix<ORBITLENGTH; ix++) {
  int bcm, bcm2; w16 newc;
  if(flag==0) {   // cumrbif->BCMASK not initialised yet
    newc= prbif->BCMASK[ix];
  } else {
    bcm= prbif->BCMASK[ix]; 
    bcm2= cumrbif->BCMASK[ix];
    bcm2= (bcm2&(~bit)) | (bcm&bit);
    newc= bcm2;
  };
  cumrbif->BCMASK[ix]= newc;
  //if(ix<10) printf("copyBCMaskBits:%d:%c\n", ix, newc);
}; //cumrbif->BCMASK[ORBITLENGTH]= '\0';
ix= cumrbif->BCMASKuse[bcmi];
if( ix!=0 ) {
  char emsg[300];
  sprintf(emsg,"Internal error:copyBCMaskBits: mask %d should be 0, but is:%d",
    bcmi, ix);
  infolog_trgboth(LOG_ERROR, emsg);
};
cumrbif->BCMASKuse[bcmi]= bcmi+1;   // allocate usage of this mask
}
/*-------------------------------------------------------------------
Check BCmasks resource.
Note: BCmask can be 'reused' by other partition if it is the same
(i.e. the same pattern and the same mask (we do not reshuffle
1..12 bcmask number!).
Operation:
for each bit in prbif:  // check if the same, or just copy if new: cumrbif<-prbif
  if another partition(in cumrbif) is using it
    check (3654 bits) whether the same
    if IS THE SAME:
      continue
    else:
      error (simple case)
  else
    allocate this bit for this partition
Todo better: i.e. with reshuffling the BCmasks -e.g.
             if *prbif uses mask2 and the same is already
             used by some partition as mask3, reassign
             the usage (modify) in all classes of *prbif
rc: 0: ok   1: mask cannot be loaded -another partition is using it
*/
int checkBCMASKS(TRBIF *cumrbif,TRBIF *prbif){
int bcmi,bcmaskn; int rcode=0;
//w8 bcmsused;
if(DBGmask) {
  prtLog(".................................checkBCMASKS...");
};
if(l0AB()==0) {bcmaskn=BCMASKN;} else {bcmaskn=4;};
for(bcmi=0; bcmi<bcmaskn; bcmi++) {
  w8 bcm;
  bcm= prbif->BCMASKuse[bcmi];
  if( bcm == 0 ) continue;
  if( cumrbif->BCMASKuse[bcmi] != 0 ) {// this +another partition is using it
    char emsg[300];
    //check (3654 bits) whether the same
    if( checkBCMaskBits(cumrbif, prbif, bcmi) ) {
      sprintf(emsg,"mask %d already used and reused...",bcmi+1); prtLog(emsg);
      continue;    // the mask already used is the same we want to use
    } else {
      sprintf(emsg,"mask %d already used by another partition",bcmi+1);
      infolog_trgboth(LOG_ERROR, emsg); rcode=1; break;
    };
  } else { // not used yet, this partition is using it
    char emsg[300];
    //allocate this bit for this partition
    copyBCMaskBits(cumrbif, prbif, bcmi); // allocates BCMASKuse[bcmi] word too
    if(DBGmask) {
      sprintf(emsg,"checkBCMASKS: mask %d allocated now",bcmi+1);
      prtLog(emsg);
    };
  };
};
return(rcode);
}
/* similar logic as for masks above */
int checkPFS(TRBIF *cumrbif,TRBIF *prbif){
int bcmi; int rcode=0;
//w8 bcmsused;
if(DBGpfs) {
  prtLog(".................................checkPFS...");
};
for(bcmi=0; bcmi<4; bcmi++) {
  w8 bcm;
  bcm= prbif->PFuse[bcmi];
  if( bcm == 0 ) continue;
  rcode= copycheckPF(cumrbif, prbif, bcmi);
  if(rcode !=0) {
    char emsg[100];
    sprintf(emsg,"PF%d already used by another partition",bcmi+1);
    infolog_trgboth(LOG_ERROR, emsg); rcode=1; break;
  };
};
return(rcode);
}

/*------------------------------------------------------ cumRBIF()
Operation: Take partition and add its RBIF to cumrbif
rc: 0: ok
    1: cannot cumulate, error message printed, cumrbif not changed
    todo: intsel,bcmask
Called from:
addRBIF2HW
checkRBIF 
*/
int cumRBIF(Tpartition *part, TRBIF *cumrbif) {
TRBIF rbifloc;
if(DBGcumRBIF) {
  printf("cumRBIF: part:%s\n",part->name);
};
copyTRBIF(&rbifloc, cumrbif);
if(checkpairRBIF(ixrnd1,&rbifloc,part->rbif))goto ERR1;
if(checkpairRBIF(ixrnd2,&rbifloc,part->rbif))goto ERR1;
if(checkpairRBIF(ixbc1,&rbifloc,part->rbif))goto ERR1;
if(checkpairRBIF(ixbc2,&rbifloc,part->rbif))goto ERR1;

//printf("\n------------- cumRBIFl0fun1B:\n");
//printTRBIF(part->rbif); printTRBIF(&rbifloc); 
if(checkpairRBIF(ixl0fun1,&rbifloc,part->rbif))goto ERR1;
//printf("cumRBIFl0fun1A:\n");
//printTRBIF(part->rbif); printTRBIF(&rbifloc);
if(checkpairRBIF(ixl0fun2,&rbifloc,part->rbif))goto ERR1;
//printf("cumRBIFl0fun2A:\n");
//printTRBIF(part->rbif); printTRBIF(&rbifloc);
/* intfun1/2 not used in partitions, commented out from jan2012:
if(checkpairRBIF(ixintfun1,&rbifloc,part->rbif))goto ERR1;
if(checkpairRBIF(ixintfun2,&rbifloc,part->rbif))goto ERR1; */

if(checkpairRBIF(ixlut3132,&rbifloc,part->rbif))goto ERR1;
if(checkpairRBIF(ixlut4142,&rbifloc,part->rbif))goto ERR1;
if(checkBCMASKS(&rbifloc,part->rbif))goto ERR1;
if(checkPFS(&rbifloc,part->rbif))goto ERR1;
copyTRBIF(cumrbif, &rbifloc); // cumrbif -> HW.rbif in addRBIF2HW()
return(0);
ERR1:
//prtError("cumRBIF error");
return(1);
}
/*------------------------------------------------------checkRBIF()
  Purpose: to check if any free RBIF
  Parameters: 
       input:*part pointer to partition which is examined
       output:?
  Globals: ?
  Return: error code = 0 if ok
                      = 1 RBIF are not free
                      = 2 AllPartitions over resources (intError)
  Called by: checkResources only
  Comment: 
   - go through AllPartitions classes and find used resources
   - at last go through new partition classes and check if *part can be 
     added to AllPartitions
*/
int checkRBIF(Tpartition *part, Tpartition *parray[]){
int ixp,flag;
TRBIF myrbif; 
Tpartition *part1;
flag=0;
if(parray==StartedPartitions)flag=1;
if(DBGcumRBIF) {
  printf("checkRBIF:startedPartitions:%d\n", flag);
};
cleanTRBIF(&myrbif,0);
// go through existing partitions:
// todo: BCMASK test to be added, at the moment new partition rewrites everything
for(ixp=0;ixp<MNPART;ixp++){
  if((part1=parray[ixp])==NULL) continue;
  if(cumRBIF(part1, &myrbif)) {   //int error
    infolog_trgboth(LOG_FATAL, "Internal error: Active partitions' resources over available resources");
    return 2; 
  };
};
// now check if new part. can be added:
if(cumRBIF(part, &myrbif)) {
  char msg[ERRMSGL];
  if(flag) {
    sprintf(msg,"Internal error: %s cannot be loaded due to lack of resources",part->name);
    infolog_trgboth(LOG_ERROR, msg);
  } else {
    sprintf(msg,"Error: %s cannot be loaded due to lack of resources",part->name);
    infolog_trgboth(LOG_ERROR, msg);
  };
  return(1);
};
//copyTRBIF(HW.rbif,&myrbif);  // was commented till 2.6.2010
return 0;
}
/*------------------------------------------------------checkClusters()
  Purpose: to check if any free clusters
  Parameters: 
        input: *part: pointer to partition which is examined
  Globals: 
        input AllPartitions[];
  Return: error code = 0 if ok
                       1 not enough clusters
                       2 any other error
  Called by: checkResources()
*/
int checkClusters(Tpartition *part){
 int i,nclust,nPart;
 nofclustTpartition(part,&nclust);
 for(i=0;i<MNPART;i++){
  if(AllPartitions[i]){
   nofclustTpartition(AllPartitions[i],&nPart);
   nclust=nclust+nPart;
  }
 }
 printf("checkClusters: # of clusters %i\n",nclust);
 if(nclust>6){
  char errmsg[200];
  sprintf(errmsg,"CheckClusters: not enough clusters: %i\n",nclust);
  infolog_trgboth(LOG_FATAL, errmsg);
  return 1;
 }
 return 0;
}
/*------------------------------------------------------checkDetectors()
 Purpose: check if detectors of new partition are amongs detectors
          of existing partitions
 Parameters: 
       input: *part: pointer to partition which is examined
 Globals: 
       input: AllPartitions[]
 Return: 0 if ok
         1 detector of *part is already in at least 1 partition
 Called by: CheckResources() 
 Comment: stop at first error
*/
int checkDetectors(Tpartition *part){
 int i,j,flag;
Tpartition *loadedp;
 for(i=0;i<MNPART;i++){
  loadedp= AllPartitions[i];
  if(loadedp == NULL) continue;
   for(j=0;j<NDETEC;j++){
    if(loadedp->Detector2Clust[j] == 0) continue;
    if(checkdetTpartition(part,j,&flag)) return 2;
    if(flag){
      char emsg[ERRMSGL];
      sprintf(emsg, 
        "%s not loaded. Detector %s(%i) already used in partition %s\n",
        part->name, validLTUs[j].name,j, loadedp->name);
      infolog_trgboth(LOG_FATAL, emsg);
      return 1;
    }
   }
  }
 return 0;
}
/*------------------------------------------------------checkTS() 
rc: pointer to Timesharing partition
*/
Tpartition *checkTS() {
int ip; Tpartition *apart;
for(ip=0;ip<MNPART;ip++){           //loop over partitions
  if(AllPartitions[ip] == NULL) continue;
  apart=AllPartitions[ip]; 
  //if(apart == part) continue;   // do not check myself
  if(apart->nclassgroups>0) { 
    return(apart);
  };
}; return(NULL);
}
/*------------------------------------------------------checkResources()
  Purpose: check conflict of resources of new *part against partitions
           saved in AllPartitions[]
  Parameter: 
        input: *part: pointer to partition which is examined
  Globals: 
        input: AllPartitions[]
  Return: 0 if ok
          1 if not enough resources 
          2 any other error
  Called by: ctp_StartPartition() 
  Comment: Checked items: 
     -are detectors from new *part already in existing partitions ?
     -are the free clusters ?
     -are there free classes ?
     -are there free RBIF ?
     -are rhere free PastFut ?
     - ?     
*/
int checkResources(Tpartition *part){
int ret=0; Tpartition *apart;
  //printf("checkResources: checking ... \n");
  // Detectors in different partitions
if(part->nclassgroups>0) { //this partition is 'timed'
  apart= checkTS();
  if(apart != NULL) {
    char errmsg[300];
    sprintf(errmsg, "'Timed class group(s)' already used in %s.", 
      apart->name);
    infolog_trgboth(LOG_FATAL, errmsg);
    ret=1; return(ret);
  };
};
  if((ret=checkDetectors(part)))return ret;
  //Cluster resources
  if((ret=checkClusters(part))) return ret;
  // RBIF resources 
  if((ret=checkRBIF(part,AllPartitions))) return ret;
  // PF resources
  //if((ret=checkInputs(part))) return ret;
  // PF resources

  return 0;
}
/*-----------------------------------------------------addRBIF2HW()
Merge all partitions' shared resources to HW
- first do 'dry run' (HW not changed)
- if OK change HW 
RET: 0: merge OK     1:not merged, HW not changed
*/
int addRBIF2HW(Tpartition *parray[]) {
int ip;
Tpartition *part;
TRBIF rbifloc;   // for check before HW is changed
copyTRBIF(&rbifloc, HW.rbif);
for(ip=0;ip<MNPART;ip++){           //loop over partitions
  if(parray[ip] == NULL) continue;
  //printf("Partition %i \n",ip);
  part=parray[ip];
  if(cumRBIF(part, &rbifloc)) goto ERR1;
};
copyTRBIF(HW.rbif, &rbifloc);
return(0);
ERR1:return(1);
}
/*---------------------------------------------------- findfreeHWCluster()
Loop over all partitions' ClusterTables
return: 1-6
        0 - no free cluster
*/
w32 findfreeHWCluster() {
int ip,icl;
w32 ret;
int fc[NCLUST]={0,0,0,0,0,0};   // part.# 1-6 which uses this hwcluster:
Tpartition *part;
for(ip=0;ip<MNPART;ip++){           //loop over partitions
  if(AllPartitions[ip] == NULL) continue;
  part=AllPartitions[ip]; 
  if(DBGaf2HW) printf("findfreeHWCluster: Partition %i \n",ip);
  //bad idea if((part->hwallocated)&0x1==0) continue;  // new partition
  // better to count with  new partition (it may have already some cluster allocated)
  for(icl=0; icl<NCLUST; icl++) {
    if(part->ClusterTable[icl]!=0) {  // hw cluster icl allocated for part (!=0)
      if( fc[icl] != 0) {             // and in the same time 
        if( fc[icl] != (ip+1) ) {
          intError("findfreeHWCluster:");
        };
      } else {
        fc[icl]= ip+1;   // icl kept by ip
      };
    };
  };
};
ret=0;
if(DBGaf2HW) printf("  partitions keeping clusters 1-6:\n");
for(icl=0; icl<NCLUST; icl++) {
  if( fc[icl]==0 ) {ret= icl+1;   // free cluster
    break;
  } else {
    if(DBGaf2HW) 
      printf("  HWcluster(1-6) %d kept by part. %d\n", icl+1, fc[icl]);
  };
};
if(DBGaf2HW) printf(  "  Returning:%d\n", ret);
return(ret);
}
/*---------------------------------------------------- rNNNN.rcfg */
void getruncfgname(w32 run, char *runcfg) {
char *environ;
environ= getenv("VMEWORKDIR"); strcpy(runcfg, environ);
sprintf(runcfg, "%s/WORK/RCFG/r%d.rcfg", runcfg, run);
}
/*---------------------------------------------------updateDAQClusters()
rc: 0: ok   >0 stop the run
2:  indets= getInputDets(HW.klas[hwclass]);   error
3: cannot update trigger config - .rcfg file not found
4: DAQlogbook_update_cluster() failed
5: cannot update trigger config in daqlogbook
6: cannot update trigger inputs in daqlogbook
8: DAQlogbook open unsuccessful
9: DAQlogbook update unsuccessful
10 internal error
Goal:
1.
for(iclu=0; iclu<6 iclu++) {
  rc=DAQlogbook_update_cluster(partit->run_number, iclu+1, 
    masks[iclu], partit->name, inpmasks[iclu], classmasks[iclu]);
    detectors                  inp.detetectors classes
}
*/
int updateDAQClusters(Tpartition *partit) {
//int rc; //idet, iclu, iclass, rc;
//int rco;    // 0: DAQlogbook opened    -1: not opened/do not update it
int rcdaqlog=0;    // rc from updateDAQClusters(). 0: ok  >1 stop the run
char emsg[ERRMSGL];
/* only if we need verbose output:
DAQlogbook_verbose(1);     */
prtProfTime("get inps2daq");
{
  int len;
  char *mem;
  FILE *ifile;
  char name[80];
  //--------------------- WORK/RCFG/rNNN.rcfg file -> DAQlogbook
  /*Operation:
    malloc,read,update DAQlogbook, free, remove
  */
  //sprintf(name,"WORK/RCFG/r%d.rcfg", partit->run_number);
  //getruncfgname(partit->run_number, name);
  sprintf(name, "/tmp/r%d.rcfg", partit->run_number);
  len= detectfile(name, 8);    // wait max. 8 (was 25 before LS1) secs for file
  // len=0;
  /*sprintf(emsg, "updateDAQClusters: Run: %d file:%s len:%d",
    partit->run_number, name, len); prtLog(emsg); */
  /*if(len<=0) {
    printf("%s not found, trying /tmp/r%d.rcfg\n",name, partit->run_number);
  }; */
  if(len>0) {
    ifile= fopen(name,"r");
    if(ifile==NULL) {
      sprintf(emsg, "updateDAQClusters: File: %s not opened\n",name);
      infolog_trgboth(LOG_FATAL, emsg);
      rcdaqlog=3;
    } else {
      int rl;
      mem= (char *)malloc(len+1);
      mem[0]='\0';
      rl=fread((void *)mem, 1, len, ifile); mem[rl]='\0';
      if(rl != len) {
        sprintf(emsg, "updateDAQClusters: File: %s read error\n",name); 
        infolog_trgboth(LOG_FATAL, emsg);
      };  
      prtProfTime("got rcfg");
      /* todo: This is the only reason we need .rcfg file in ctpproxy? */
      if(strcmp(partit->name,"PHYSICS_1")==0) {
        int fixl;
        fixl= get_fixed(mem, &fixpos[2]);
        fixpos[0]= partit->run_number;
        lenfixpos= fixl+2;
      };
      fclose(ifile); free(mem); 
      rl= remove(name);
      /*sprintf(emsg, "updateDAQClusters: Run: %d file:%s\nDAQlogbook_update_triggerConfig() rc:%d\n",
        partit->run_number, name, rc);
      prtLog(emsg); */
      if(rl != 0) {
        sprintf(emsg, "updateDAQClusters: File: %s not removed\n",name); 
        prtError(emsg);
      };
    };
  } else {
    sprintf(emsg, "updateDAQClusters: Partition:%s file %s not found.\n", partit->name, name);
    prtWarning(emsg);   // was prtError
    //infolog_trgboth(LOG_FATAL, emsg);
    rcdaqlog=3;
  };
};
//-------------------------------- close
/*
if(rco==0) {
  if(cshmGlobFlag(FLGignoreDAQLOGBOOK)) { rc=0;
  } else {
    rc= daqlogbook_close();
  };
  if(rc==-1) {
    infolog_trgboth(LOG_ERROR, "DAQlogbook_close failed");
  };
};*/
return(rcdaqlog);
}
/*---------------------------------------------------clusterPart2HW()
  Purpose: to convert clustercodes in partition frame to
           cluster codes in Hardware frame
  Parameters: 
        input: pclustcodes - in *part structure  0..5
               1 detector can belong to more clusters!
        output: hclustcode - clustercode in HW structure
  Globals: none
  Returns: error code = 0 if ok
                        1 if error
  Comment: assignment of clusters in HW 
*/
w32 clusterPart2HW(w32 pclustercodes,Tpartition *part,w32 *hclustcodes){
 w32 pclust;
 int i,j;
 *hclustcodes=0;
 if(DBGaf2HW) printf("clusterPart2HW:pclustercodes:%x\n", pclustercodes);
 for(i=0;i<NCLUST;i++){
   // find clusters in pclustcode             
   if((pclustercodes) & (1<<i))pclust=i+1;  //pclust cannot be 0
   else continue;
   if(DBGaf2HW) printf("  clusterPart2HW:part:%s pclust=%i\n",
       part->name, pclust);
   // Convert pcluster name ->> hw cluster
   j=findHWCluster(part, pclust);
   if(j==0) {
     if(part->hwallocated&0x1) {
        char emsg[ERRMSGL];
        sprintf(emsg, "hw cluster not found for part-cluster %d of 'old' partition %s", pclust, part->name);
       intError(emsg);
     };
     j=findfreeHWCluster();
   } else {
   };
   if(j==0) {
     prtError("free cluster not found"); 
     return(1);
   };
   j--;   // now: j is hwcluster -1
   /* ----?
   j=0;                              
   while((pclust != part->ClusterTable[j]) && (j<NCLUST))j++;
   if(j>=NCLUST){
    if(DBGaf2HW) printf("clusterPart2HW: incompatible clusters in TKlas and clusters(FO).\
j:%d .But this was already checked in readDatabaseTpartition , so some bug.\n",j);
    return 1;
   }
   */
   *hclustcodes=*hclustcodes+(1<<j); 
   // allocate j+1 hwcluster for partition part with pclust:
   part->ClusterTable[j]= pclust;
   if(DBGaf2HW) printf("  clusterPart2HW:j:%d =%d *hclustcodes:%x\n",
     j,pclust, *hclustcodes);
 }; return 0;
}
/*-----------------------------------------------------addFO2HW()
 Purpose: add FO words to HW
 Globals: input AllPartitions[]
          output HW
 Operation:
 - for part in AllPartitions:
     for det in 'Detectors in part':
       set HW.fo[ifo] (according to det)
       set HW.busy if valid BUSY input (found in validLTUs table) 
     set part->hwallocated[0x1] -clusters for this part allocated in HW

 Returns: error code =0 if ok

     //THIS GOS to addFO2hw: new part, so we are creating ClusterTable:
     part->ClusterTable[icluster]=pcluster;
      icluster=hwcluster;   
      hwcluster++;         //todo: here we should find free hw cluster!
*/
int addFO2HW(Tpartition *parray[]){
int ip,idet,ifo,iconnector;
w32 pclustercodes,hclustercodes;
Tpartition *part;
for(ip=0;ip<MNPART;ip++){         // loop over partitions
 if(parray[ip] == NULL) continue;
 part=parray[ip];
 if(DBGaf2HW) printf("addFO2HW: part:%s\n",part->name);
 for(idet=0;idet<NDETEC;idet++){    //loop over detectors in partition
   int bsyinp, ihwclu;
   if((pclustercodes = part->Detector2Clust[idet])==0) continue;
   // partition cluster name ->> hardware cluster code 
   // (and allocate this hw cluster if necessary):
   if(clusterPart2HW(pclustercodes,part,&hclustercodes)) return 1;
   if(DBGaf2HW) 
     printf("  addFO2HW: idet:%d pcc=0x%x hcc=0x%x\n",
       idet,pclustercodes,hclustercodes);
   if(Detector2Connector(idet,&ifo,&iconnector)) return 1;
   if((part->MaskedDetectors & (1<<idet))==0) {
     /* we should never get here (detector removed already in applyMask): */
     char emsg[ERRMSGL];
     sprintf(emsg,"addFO2HW: det:%d masked(excluded), why checked? \n",idet);
     intError(emsg);
     continue;
   }; 
   for(ihwclu=0; ihwclu<NCLUST; ihwclu++) {
     w32 hclustercode;
     hclustercode= 1<<ihwclu;
     if( hclustercode & hclustercodes) {
       HW.fo[ifo].cluster= HW.fo[ifo].cluster | (hclustercode<<(8*iconnector));
       // set HW.busy for connected detectors:
       if((bsyinp=validLTUs[idet].busyinp)) {  //BUSY line connected too:
         HW.busy.set_cluster[ihwclu+1]=    // 0: test 1..6 hw clusts
           HW.busy.set_cluster[ihwclu+1] | (1<<(bsyinp-1));
         if(DBGbusy) printf(
           "addFO2HW: det:%d bsyinp:%d cluster:0x%x/%d set_cluster:0x%x\n", 
           idet, bsyinp, hclustercode,ihwclu+1, 
           HW.busy.set_cluster[ihwclu+1]);
       };
       if(DBGaf2HW) {
         printf("  addFO2HW: det: %i fo/foc:%d/%d hclustcode=0x%x \n",
           idet,ifo,iconnector,hclustercode);
       };
     };
   };
 };
 if(DBGaf2HW) printf("  addFO2HW: hwallocated:%x\n",
     part->hwallocated);
 if(part->hwallocated==0) {
   part->hwallocated= 0x1;
   if(DBGaf2HW)printf("  new part:-bit 'clusters allocated' set to 0x1\n");
 } else if(part->hwallocated==0x3) {;
   if(DBGaf2HW)printf("  old part:-bit 'clusters allocated' was already set\n");
 } else {
   intError("hwallocated !=0,3");
 };
}
return 0;
}
/*-----------------------------------------------------addClasses2HW()
  Purpose: add classes to HW
  Parameters: none
  Globals: input AllPartitions[]: there are 2 types of partitions:
           - new (not assigned hw)
           - partitions, which already have assigned clusters/classes
           output HW
  Returns : error code : 0:ok >0: error
  Operation:
  Comment: assignment of partition cluster to hw cluster is arbitrary
           I assume: partition cluster names are numbers 1..6 as in
           part->l0veto word. Partition does not know to which cluster
           in HW it goes if loaded 1st time.
           Loaded partition keeps its classes/clusters when another
           partition is loaded (i.e. the same hw is used for partition,
           from Start to End of run.
           Asignment to hardware clusters is from 1 as the partition
           clusters of new partitions are coming.
           The partition delete (stop the run) releases its clusters,
           so they can be used for another partition.
           Table is stored in part->ClusterTable[]        
*/  
int addClasses2HW(Tpartition *parray[]){
 int i,ip,ips,pclass,hwclass,rcode=0;
 Tpartition *part;
 TKlas *klas;
 // merge classes from partitions *parray[]
if(DBGac2HW) {printf("addClasses2HW: ALl partititons:\n"); printAllTp(); };
 // First loop over 'old' partitions (to keep the same hw for running partitions):
 for(ip=0;ip<MNPART;ip++){           //loop over old partitions
   char oldclasses[1200]="";   // 1200> 11*100
   if(parray[ip] == NULL) continue;
   part=parray[ip]; if((part->hwallocated&0x2)==0) continue;
   if(DBGac2HW) printf("  old Partition %i hwallocated:%x \n",
       ip,part->hwallocated);
   pclass=0;                         // loop over classes in partition
   oldclasses[0]='\0';
   for(pclass=0; pclass<NCLASS; pclass++) {
     if((klas=part->klas[pclass]) == NULL) continue;
     if(DBGac2HW) printf(" hwallocated. pclass: %i \n",pclass);
     hwclass= part->klas[pclass]->hwclass;  // 0..49/99
     if(hwclass >= NCLASS) {
       char errmsg[400];
       sprintf(errmsg, "addClasses2HW: intError: hwclass>= 50/100"); rcode=1;
       infolog_trgboth(LOG_FATAL, errmsg); goto ERRRET;
     };
     /* If partition cluster (pcluster) is different 
     from hw cluster correct it in HW.klas[]->l0vetos and 
     l1/2defininition words: */
     //pcluster = (klas->l0vetos) & 0x7;   //partition cluster name
     //icluster= findHWCluster(part, pcluster);
     copymodTKlas(HW.klas[hwclass],klas, part);
     HW.klas[hwclass]->hwclass = 1;      
     sprintf(oldclasses,"%s %d:%d:%d", 
       oldclasses, pclass, hwclass,HW.klas[hwclass]->classgroup);
   };
   printf("addClasses2HW old:%s %d:%s\n", part->name, part->run_number, oldclasses);
   if(DBGac2HW) printf("  addClasses2HW: hwallocated not changed, it is:%x\n",
     part->hwallocated);
 };
 // Now loop over 'new' partitions (to allocate the rest of resources)
 for(ip=0;ip<MNPART;ip++){           //loop over new partitions
   char newclasses[1200]="";
   if(parray[ip] == NULL) continue;
   part=parray[ip]; if((part->hwallocated&0x2)==0x2) continue;
   if(DBGac2HW) printf("  new Partition: %i \n",ip);
   // loop over classes in partition
   newclasses[0]='\0';
   for(pclass=0; pclass<NCLASS; pclass++) {
     char errmsg[400];
     w32 l0inv, l1inv, l2inv; int startn;
     if((klas=part->klas[pclass]) == NULL) continue;
     l0inv= part->klas[pclass]->l0inverted;
     l1inv= part->klas[pclass]->l1inverted;
     l2inv= part->klas[pclass]->l2definition & 0xfff000;
     if( (l1inv !=0) || (l2inv !=0) )         { startn= 44;
     } else if( (l0inv != 0) && (l0AB()!=0) ) { startn= 44;
     } else                                   { startn=0; };
     for(hwclass=startn; hwclass<NCLASS; hwclass++) {
       //printf("loop hw=%i p=%i %i\n",hwclass,pclass,HW.klas[hwclass]->hwclass);
       if(HW.klas[hwclass]->hwclass==0) goto OKHWCL1;   // free hw found
     };
     sprintf(errmsg, "addClasses2HW: free class not found (searched from:%d)",startn);rcode=1;
     infolog_trgboth(LOG_FATAL, errmsg); goto ERRRET;
     OKHWCL1:
     part->klas[pclass]->hwclass= hwclass;  // remember which is used by pclass
     if(DBGac2HW)printf("  addClasses2HW: hwclass set (reserved)for log.class %d\n", hwclass);
     copymodTKlas(HW.klas[hwclass],klas, part);
     HW.klas[hwclass]->hwclass = 1;         // reserve it and
     sprintf(newclasses,"%s %d:%d:%d", 
       newclasses, pclass, hwclass, HW.klas[hwclass]->classgroup);
     //printTKlas(HW.klas[hwclass],hwclass);
   };
   printf("addClasses2HW new:%s %d:%s\n", part->name, part->run_number, newclasses);
   if(DBGac2HW) printf("  addClasses2HW: hwallocated before set to 0x3:%x\n",
     part->hwallocated);
   part->hwallocated= 0x3;
   printHardware(&HW, "addClasses2HW");
   if(DBGac2HW) {
     printf("  addCLasses2HW: ClusterTable for new part:%d:", ip);
     for(i=0;i<NCLUST;i++)printf("0x%x ",part->ClusterTable[i]);
     printf("\n"); 
   };
 }; // over all new partitions
// find first class for each SDG group:
for(ips=0;ips<NSDGS;ips++){                   //loop over all SDGS
  int firstc;
  char errmsg[150];
  if(SDGS[ips].name[0]=='\0') continue;     // empty entry
  firstc= 51;
  for(ip=0;ip<MNPART;ip++){           //find our partition
    part= parray[ip];
    if(part == NULL) continue;
    if(strcmp(part->name, SDGS[ips].pname)==0) goto OKPART;
  };
  sprintf(errmsg,"addClasses2HW:SDGS with illegal partition %s",
    SDGS[ips].pname);
  infolog_trgboth(LOG_ERROR, errmsg);
  rcode=2; goto ERRRET;
  OKPART:
  for(pclass=0; pclass<NCLASS; pclass++) { // all classes
    klas=part->klas[pclass];
    if(klas == NULL) continue;
    if(klas->sdg == -1) continue;
    if(klas->sdg == ips) {
      if(klas->hwclass < firstc) firstc= klas->hwclass+1;   // 0..49-> 1..50/100
    };
  };
  if(firstc<51) {
    SDGS[ips].firstclass= firstc;
  };
  printf("addClasses2HW:SDGS[%d]:%s %s: 1st clas:%d\n", ips,
    SDGS[ips].name, SDGS[ips].pname, SDGS[ips].firstclass);  
};
// put results from SDGS in HW.sdgs
for(ip=0;ip<MNPART;ip++){ 
  part= parray[ip];
  if(part == NULL) continue;
  for(pclass=0; pclass<NCLASS; pclass++) { // all classes
    klas=part->klas[pclass];
    if(klas == NULL) continue;
    if(klas->sdg != -1) {
      int hwclass;
      hwclass= klas->hwclass;  // 0..49/99
      HW.sdgs[hwclass]= SDGS[klas->sdg].firstclass - 1;
      //HW.lmsdgs[hwclass]= SDGS[klas->sdg].firstclass - 1;  todo
      printf("addClasses2HW:hwclass0..49/99:sdgs[%d]:%d\n", 
        hwclass, HW.sdgs[hwclass]);
    };
  };
};
ERRRET:
return(rcode);
}
/*----------------------------------------------------addPartitions2HW()
  Purpose: ro unite (merge,join) parray[] to HW structure
  Patameters: none
  Globals: input: parray[]
           output: HW
  Returns: 0 if ok; 
           1 if error
  Comment: ClusterTable has to be cleared on every call
*/
int addPartitions2HW(Tpartition *parray[]){
 cleanHardware(&HW, 1);
 if(addRBIF2HW(parray)) return 1;
 if(addFO2HW(parray)) return 1; // must be before addClasses2HW (to alloc clusters)
 if(addClasses2HW(parray)) return 1;
 // addBUSY -not necessary -done in addFO2HW   dont forget mask -is considered in addFO2HW
 //addPastFut
 /* 2 phases (above): clusters (in addFO2HW) and classes (in addClasses2HW)
 for(ip=0;ip<MNPART;ip++){           //mark 'hw allocated'
   if(parray[ip] == NULL) continue;
   parray[ip]->hwallocated=1;
 };*/
 return 0;
}

//-----------------------------------------------------------
//  Partitions Manipulations
//-----------------------------------------------------------
/*---------------------------------------------------getPartitions()
 Purpose: get pointer to partition with name in AllPartitions{}
 Parameters: input name - partition name
             parray:   AllPartitions or StartedPartitions
 Returns: pointer to partition with name
*/
Tpartition *getPartitions(char *name, Tpartition *parray[]){
 int i;
 Tpartition *part;
 if(name == NULL){
  printf("getPartitions error: name=NULL \n");
  return NULL;
 }
 for(i=0;i<MNPART;i++){
  if((part=parray[i])){
   if(strcmp(part->name,name) == 0) return part;
  }
 }
 return NULL;
}
/*------------------------*/ int getPartitionsN(Tpartition *parray[]){
int i, rc=0;
for(i=0;i<MNPART;i++){
  if(parray[i]) rc++; 
};
return rc;
}
/*------------------------------------------------deletePartitions()
  Purpose: remove partition from both lists: AllPartitions[] StartedPartitions[]
  part: input partition to be deleted
  Returns:
  1: deleted from AllPartitions[]
  3: deleted from both
  0: ERROR: partition is not present in any of 2 lists
  2: ERROR: partition deleted from StartedPartitions list only
  4: ERROR: part==NULL
*/
int deletePartitions(Tpartition *part) {
int i, rc=0; char pname[40];
if(part==NULL){
 intError("deletePartitions error: partition=NULL"); return 4;
};
strcpy(pname, part->name);
i=0; while((part != AllPartitions[i]) && (i<MNPART))i++;
if(i>=MNPART) {rc= 0;  // not in All
} else { // IN all
  if((part->positionInAllPartitions!=-1) &&   // never started
    (part->positionInAllPartitions!=i)) {
    char msg[300];
    sprintf(msg, 
      "deletePartitions:%s positionInAllPartitions:%d but it is at:%d .",
      pname, part->positionInAllPartitions, i); 
    intError(msg);
  };
  AllPartitions[i]=deleteTpartition(part);
  rc=1;
};
i=0; while((part != StartedPartitions[i]) && (i<MNPART))i++;
if(i>=MNPART) { // not in Started
  rc= rc;  //deleted only from All (1) or not found in any (0)
} else { 
  if(rc==1) {   // IN started
    StartedPartitions[i]=NULL;   // freed already in All
    rc=3;
  } else {  // found only in Started
    char msg[200];
    sprintf(msg, 
      "deletePartitions error. Partition %s only in StartedPartitions.",pname); 
    intError(msg);
    StartedPartitions[i]=deleteTpartition(part);
    rc=2;
  };
};
if((rc==0) || (rc==2)) {
  char msg[200];
  sprintf(msg, "deletePartitions error %d for partition %s", rc,pname); 
  intError(msg);
};
return rc;
}
/*------------------------------------------------ctp_StopAllPartitions()
used in case: CTRL C
*/
void ctp_StopAllPartitions() {
int i; Tpartition *part; char pname[40];
for(i=0;i<MNPART;i++){
  if((part=AllPartitions[i])){
   //int rc1;
   char msg[200];
   strcpy(pname, part->name);
   infolog_SetStream(pname, part->run_number);
   sprintf(msg, "Stop forced for partition %s", pname); 
   infolog_trgboth(LOG_FATAL, msg);
   ctp_StopPartition(pname);
   //rc1= deletePartitions(Tpartition *part);
  };
};
infolog_SetStream("",0);
return;
}
/*----------------------------------------------------------addPartitions()
 Purpose: add partition to AllPartitions[]
 Parameters: input part
 Globals: AllPartitions[]
 Returns: error code: 0=ok
*/
int addPartitions(Tpartition *part){
 int i;
 for(i=0;i<MNPART;i++){
   if(AllPartitions[i] == NULL){
    AllPartitions[i] = part;
    part->positionInAllPartitions=i;
    return 0;
   }
  }
  printf("addPArtitions sw error: all AllPartitions[] != NULL ?\n");
  return 1; 
}
int addStartedPartitions(Tpartition *part) {
 int i;
 for(i=0;i<MNPART;i++){
   if(AllPartitions[i] == part){
      goto OK;
   };
 };
 intError("addStartPartitions: partition not found in AllPartitions.");
 return(1);
 OK:
 // check if already started: done before calling addStartedPartitions()
 for(i=0;i<MNPART;i++){
   if(StartedPartitions[i] == NULL){
    StartedPartitions[i] = part;
    return 0;
   }
  }
  intError("CheckResources sw error: all StartedPartitions[] != NULL ?");
  return 1; 
}

//--------------------------------------------------------------------------
//  Hardware operations
//--------------------------------------------------------------------------- 
// following structure should be probably part of global HW structure
// called like: hwcontrol (i.e. used for CTP control (not partition config))
struct daqbusys {
  w32 activebusys; // valid (i.e. in BUSY_DAQBUSY) outside of 'GLOBAL BUSY'
                   // clusters mask 1:cluster is paused
                   // MUST always be set according to paused_dets/_dts
  int global;      // 1: global busy ON. activebusys keeps info about
                   // active busys), BUSY_DAQBUSY is set to 0x3f
                   // 0: global busy OFF. activebusys is copy of BUSY_DAQBUSY
                   // Used only: Start/Stop part 
  int paused_dets; // detectors in PAUSED state for all parts
  int paused_dts;  // detectors in short PAUSED state (during SYNC) -MUST NOT
                   // overlap with paused_dets
  int clust_dts;   // clusters in short PAUSED state (during SYNC) -MUST NOT
                   // overlap with activebusys
} DAQBUSY={0,0,0,0,0};   // Initialised in ctp_Initproxy

//--------------------------------------------------------setALLDAQBusy()
void setALLDAQBusy(){
 if(DAQBUSY.global==1) {
   intError("setALLDAQBusy: setALLDAQBusy called 2 times. No acion taken");
 } else {
   cshmSetGlobFlag(FLGignoreGCALIB);
   DAQBUSY.activebusys=vmer32(BUSY_DAQBUSY)&0x3f;
   vmew32(BUSY_DAQBUSY,0x3f); usleep(110);  //the must (L2 pipeline flush)
   DAQBUSY.global=1;
   if(DBGbusy) printf("setALLDAQBusy: old BUSY_DAQBUSY:0x%x \n",DAQBUSY.activebusys);
 };
}
//--------------------------------------------------------unsetALLDAQBusy()
void unsetALLDAQBusy(){
 if(DAQBUSY.global!=1) {
   intError("unsetALLDAQBusy: setALLDAQBusy was not called before. No action taken");
 } else {
   vmew32(BUSY_DAQBUSY,DAQBUSY.activebusys);
   cshmClearGlobFlag(FLGignoreGCALIB);   // enable calibration
   DAQBUSY.global=0;
   if(DBGbusy) printf("unsetALLDAQBusy: old BUSY_DAQBUSY:0x%x \n",DAQBUSY.activebusys);
 };
}
//--------------------------------------------------------setPartDAQBusy()
/* Set partition daq busy after alldaq busy. 
detectors: 
- only clusters, detectors are included in, to be paused 
  (bit pattern already checked for correctness, i.e. not checked here)
- 0: pause all clusters of part
 -1: detto, + store info about 'fast' paused_dts/clust_dts
Note:
*/
int setPartDAQBusy(Tpartition *part, int detectors) {
w32 newbusy, clustbusy;
/* we do not need bakery-lock yet, clustbusy returned in following line
   is 'static' -i.e. always the same, depends only on part. config */
 if(DAQBUSY.global==1) {
   intError("setPartDAQBusy: called when GLOBAL busy ON. No acion taken");
 } else {
   int p_detectors; 
   //bakery_lock
   if(detectors==0) {
     clustbusy=getBusyMaskPartition(part, 0);
     p_detectors= getPartDetectors(part);
   } else if(detectors==-1) {
     int d_dts; w32 c_dts;
     clustbusy=getBusyMaskPartition(part, 0);
     p_detectors= getPartDetectors(part);
     d_dts= p_detectors &  DAQBUSY.paused_dets;
     if(d_dts!= p_detectors) {  
       // some dets still need to be paused
       if(DAQBUSY.paused_dts!=0) {
         prtError("setPartDAQBusy:DAQBUSY.paused_dts should be 0");
       };
       DAQBUSY.paused_dts= (~d_dts) & p_detectors;   // 1: these need to be paused/released
     };
     c_dts= clustbusy &  DAQBUSY.activebusys;
     if(c_dts!= clustbusy) {  
       DAQBUSY.clust_dts= (~c_dts) & clustbusy;   // 1: these need to be released
     };
     if(DBGbusy) {printf("setPartDAQBusy:paused_dts/clust_dts:0x%x 0x%x\n",
       DAQBUSY.paused_dts,DAQBUSY.clust_dts); 
     };
   } else {
     clustbusy=getBusyMaskPartition(part, detectors);
     p_detectors= detectors;
   };
   newbusy=clustbusy | DAQBUSY.activebusys;
   vmew32(BUSY_DAQBUSY,newbusy); usleep(110);  //the must (L2 pipeline flush)
   DAQBUSY.activebusys=newbusy;
   DAQBUSY.paused_dets= DAQBUSY.paused_dets | p_detectors;
   //bakery_unlock
   if(DBGbusy) printf("setPartDAQBusy: clustbusy:0x%x BUSY_DAQBUSY=0x%x \n",clustbusy,newbusy); 
 };
 return 0;
}
//-------------------------------------------------------------unsetPartDAQBusy()
/*
detectors: 
- make ready all clusters but clusters containing detectors 
- == 0: enable all clusters(i.e. all detectors) of part
  ==-1: release only those marked '1' in DAQBUSY.paused_dts/clust_dts
  =! 0: detectors defines the clusters to be left 'paused'

operation:
ALL= DAQBUSY.activebusys   -all clusters paused
     DAQBUSY.paused_dets   -all detectors paused
1. find clustpatt for 'release all detectors in partition' case (DPap)
        detpatt                                                 (DPap_dets)
2. find clustpatt for 'leave detectors paused' case (clust)
        detpatt                                     (pdets=detectors 
                                                          =DPap_dets if 0)
3. new set of busy clusters: ALL & (~DPap) | ((~clust) & DPap)
                             outside         inside partition
              paused_dets:   paused_dets | ( ~DPap_dets | pdets)
   i.e.
   ~DPap:  '0' for all part. clusters, i.e. we want to make them active at SOR
   clust:  '1' for all part. clusters
   ~DPap & clust:   '0': go active  '1': stay off 
   ALL & ~DPap  :  other ctp clusters, not in part (0:active 1:off)
   ~clust & DPap: in clusters mask to be paused (0:active, 1:off)
rc: always 0
*/
int unsetPartDAQBusy(Tpartition *part, int detectors){
w32 clust;
if(part==NULL) {
  prtWarning("unsetPartDAQBusy: NULL partition..."); return(0);
};
if(DAQBUSY.global==1) {
  w32 dbab;
  clust=getBusyMaskPartition(part,detectors);
  if(part-> remseconds != -1) {
    if(part->cshmpart->paused==0) {
      intError("unsetPartDAQBusy: called when GLOBAL busy ON. No acion taken");
    } else {
      prtError("unsetPartDAQBusy: called when GLOBAL busy ON,paused partition. No acion taken");
    };
  };
  dbab= DAQBUSY.activebusys;
  DAQBUSY.activebusys= DAQBUSY.activebusys & (~clust); 
  if(DBGmask) {
    printf("unsetPartDAQBusy: called when GLOBAL busy ON. Daqbusy.activebusys:0x%x and ~0x%x = 0x%x\n",
      dbab, clust, DAQBUSY.activebusys);
  };
} else {
  w32 dbab, newdbab, DPap; int origdets, DPap_dets, pdets;
  clust=getBusyMaskPartition(part,detectors);
  DPap= getBusyMaskPartition(part,0);   // part clusters (1:in 0:out)
  DPap_dets= getPartDetectors(part);
  //bakery_lock
  if(detectors==0) {
    pdets= DPap_dets;   // 1: for those to be released (made active)
    // clust -the same for clusters
  } else if(detectors==-1) {
    pdets= DAQBUSY.paused_dts;
    clust= (DAQBUSY.clust_dts) & DPap;
    DAQBUSY.paused_dts=0; DAQBUSY.clust_dts=0;
  } else {
    pdets= (~detectors) & DPap_dets;
    // now clust: 1: for those to be left busy
    clust= (~clust) & DPap;
    // now clust: 1: for those to be released
  };
  dbab= DAQBUSY.activebusys;
  newdbab= (DAQBUSY.activebusys & (~DPap)) | ((~clust) & DPap);
  //       outsiders                         insiders
  //old clust = DAQBUSY.activebusys & (~clust); 
  //old vmew32(BUSY_DAQBUSY,clust); DAQBUSY.activebusys= clust;
  vmew32(BUSY_DAQBUSY,newdbab); DAQBUSY.activebusys= newdbab;
  origdets= DAQBUSY.paused_dets;
  DAQBUSY.paused_dets= (DAQBUSY.paused_dets & (~DPap_dets)) | ((~pdets) & DPap_dets);
  //bakery_unlock
  if(DBGbusy) {
    printf("unsetPartDAQBusy clsts _dts:0x%x org/all/leavebusy:0x%x 0x%x 0x%x ->0x%x\n",
      DAQBUSY.clust_dts, dbab, DPap, clust, newdbab); 
    printf("unsetPartDAQBusy  dets _dts:0x%x org/all/leavebusy:0x%x 0x%x 0x%x ->0x%x\n",
      DAQBUSY.paused_dts, origdets, DPap_dets, pdets, DAQBUSY.paused_dets); 
  };
};
return 0;
}
/*------------------------------------------------ readLTUcntsInCraDIM()
read LTU counters if they ar in the same crate (to be thrown away
as soon as we have DIM server for that).
*/
void readLTUcntsInCraDIM(int idet, w32 *mem) {
int cix, vsp;
    //ltuc= vmxr32(validLTUs[idet].ltuvsp, L0_COUNTER+4*ix);
if((validLTUs[idet].ltubasea[0]!='\0') && 
   (strncmp(validLTUs[idet].ltubasea,"0x",2)==0)) {   // LTU in CTP crate
  vsp= validLTUs[idet].ltuvsp;
  vmxw32(vsp, COPYCOUNT,DUMMYVAL);
  usleep(8); // allow 8 micsecs for copying counters to VME accessible memory
  vmxw32(vsp, COPYCLEARADD,DUMMYVAL);
  for(cix=0; cix<LTUNCOUNTERS; cix++) {
    mem[cix]= vmxr32(vsp, COPYREAD);
  };
} else {   //LTUDIM
  char emsg[ERRMSGL];
  sprintf(emsg,"LTUDIM counters not supported yet\n");
  intError(emsg);
};
return;
}

/*------------------------------------------------ readLTUcnts()
xse: 'S' -just store in validLTUs
     'E' -read/subtract the value in validLTUs[], store in validLTUs[]
*/
void readLTUcnts(Tpartition *partit, char xse) {
int idet;
// Find out active partition detectors
for(idet=0;idet<NDETEC;idet++){
  int rc=1, ix; w32 ltucnts[LTUNCOUNTERS]; 
  if(partit->Detector2Clust[idet] ==0) continue;
  //printf("readLTUcnts: %d ltubase:%s:\n", idet, validLTUs[idet].ltubasea);
  if(validLTUs[idet].ltuvsp==-1) continue;  // don't try any more
  if(validLTUs[idet].ltuvsp==-2) {   // open it
    validLTUs[idet].ltuvsp=-1;
    if(validLTUs[idet].ltubasea[0]=='\0') {
      return; // LTU not present
    } else if(strncmp(validLTUs[idet].ltubasea,"0x",2)!=0) {
      return; // not support yet to get LTU cnts over DIM
      //rc= dic_open(validLTUs[idet].ltubasea);
      //rc=0;
    } else {
      rc= vmxopen(&validLTUs[idet].ltuvsp, validLTUs[idet].ltubasea, "0x800");
    };
    if(rc!=0) {
      validLTUs[idet].ltuvsp= -1; //don't try to open more
      return;
    } else {
      printf("readLTUcnts: LTU %s vsp %d opened\n",
        validLTUs[idet].name, validLTUs[idet].ltuvsp);
    };
  };
  //readLTUcntsInCrate(validLTUs[idet].ltuvsp, ltucnts);
  readLTUcntsInCraDIM(idet, ltucnts);
  for(ix=16; ix<16+5; ix++) {  // cnts: L0 L1 L1s L2a
    w32 ltuc;
    //ltuc= vmxr32(validLTUs[idet].ltuvsp, L0_COUNTER+4*ix);
    ltuc= ltucnts[ix];
    //printf(" %d", ltuc);
    if(ix==3) {   // L2a
      if(xse=='S') validLTUs[idet].ltul2asod= ltuc;
      else {
        validLTUs[idet].ltul2a= dodif32(validLTUs[idet].ltul2asod, ltuc);
      };
    };
  };
};
}
/*--------------------------------------------------- readCTPcnts()
xse: 'S' -just store in validLTUs
     'E' -read/subtract the value in validLTUs[], store in validLTUs[]
*/
#define newreadCTPcnts
void readCTPcnts(Tpartition *part, char xse) {
int idet,reladr;
#ifdef newreadCTPcnts
//w32 mem[CSTART_SPEC];
w32 mem[NCOUNTERS];
printBakery(&ctpshmbase->ccread);
usleep(100000);
//readCounters(mem, CSTART_SPEC, 0, 0);
readCounters(mem, NCOUNTERS, 0, 0);
printf("bakery after readCounters:\n");
usleep(100000);
printBakery(&ctpshmbase->ccread);
for(idet=0;idet<NDETEC;idet++){
  if(part->Detector2Clust[idet] ==0) continue;
  reladr= CSTART_FO + NCOUNTERS_FO*(validLTUs[idet].fo-1) +
    validLTUs[idet].foc -1; 
  if(xse=='S') {
    validLTUs[idet].ctpl0outsod= mem[SODEODfol0out1+reladr];
    validLTUs[idet].ctpl1outsod= mem[SODEODfol1out1+reladr];
    validLTUs[idet].ctpl2strosod= mem[SODEODfol2stro1+reladr];
  };
  if(xse=='E') {
    validLTUs[idet].ctpl0out= dodif32(validLTUs[idet].ctpl0outsod, 
      mem[SODEODfol0out1+reladr]);
    validLTUs[idet].ctpl1out= dodif32(validLTUs[idet].ctpl1outsod, 
      mem[SODEODfol1out1+reladr]);
    validLTUs[idet].ctpl2stro= dodif32(validLTUs[idet].ctpl2strosod, 
      mem[SODEODfol2stro1+reladr]);
  };
};
#else
int bb,cix; w32 copyread;
int board;
w32 mem[NCOUNTERS_MAX];
//board=5; reladr=31;   // FO1, L2strobe on 2nd connector
reladr=30;
for(idet=0;idet<NDETEC;idet++){
  if(part->Detector2Clust[idet] ==0) continue;
  board= validLTUs[idet].fo -1 + 5;   // busy,L0 1 2 INT FO1...
  reladr= 30+ validLTUs[idet].foc -1; //L2strobe (last in the sequence)
  bb= BSP*ctpboards[board].dial;
  if(notInCrate(board) == 0){
    vmew32(bb+COPYCOUNT,DUMMYVAL); 
    usleep(8); // allow 8 micsecs for copying counters to VME accessible memory
    vmew32(bb+COPYCLEARADD,DUMMYVAL);  
    copyread= bb+COPYREAD; 
    for(cix=0; cix<=reladr; cix++)mem[cix]= vmer32(copyread);
  }else{
    for(cix=0; cix<=reladr; cix++)mem[cix]= 0;
  }
  if(xse=='S') {
    validLTUs[idet].ctpl2strosod= mem[reladr];
  };
  if(xse=='E') {
    validLTUs[idet].ctpl2stro= dodif32(validLTUs[idet].ctpl2strosod,
      mem[reladr]);
  };
};
#endif
return;
}
/*------------------------------------------------------ readALLcnts()
xse: 'S' -just store in validLTUs
     'E' -read/subtract the value in validLTUs[], store in validLTUs[]
     'P' -like 'E' but, print the counters in addition
*/
void readALLcnts(Tpartition *part, char xse) {
char xse2='0';
int idet;
if(xse=='P') {
  xse='E'; xse2='P';
};
readCTPcnts(part, xse);
readLTUcnts(part, xse);
if(xse2=='P') {
  char msg[3000], msgmism[200]="" ;
  //sprintf(msg, "detector         CTPL2a     LTUL2a\n");
  sprintf(msg, "detector           CTPL0      CTPL1     CTPL2a  run:%d\n",
    part->run_number);
  for(idet=0;idet<NDETEC;idet++){
    w32 cl0, cl1, cl2;
    if(part->Detector2Clust[idet] ==0) continue;
    cl0= validLTUs[idet].ctpl0out;
    cl1= validLTUs[idet].ctpl1out;
    cl2= validLTUs[idet].ctpl2stro;
    //sprintf(msg,"%s %12s %10d %10d\n", msg, validLTUs[idet].name, validLTUs[idet].ctpl2stro, validLTUs[idet].ltul2a);
    sprintf(msg,"%s %12s %10u %10u %10u \n", msg, validLTUs[idet].name, 
      cl0, cl1, cl2);
    if( (cl0 < cl1) || (cl1 < cl2) ) {   // was cla>cl2 till 20.1.2015
      sprintf(msgmism, "%s %s", msgmism, validLTUs[idet].name);
    };
  };
  infolog_trgboth(LOG_INFO, msg);
  if( msgmism[0]!='\0') {
    sprintf(msg, "L0/L1/L2 mismatch for %s", msgmism);
    infolog_trgboth(LOG_ERROR, msg);
  };
};
}
/*------------------------------------------------ generateXOD()
  Purpose: to generate SOD/EOD/SYNC
  Parameters: input: part
                     x = 'S' for SOD, 'E' for EOD 'Y' for SYNC
  Partition's clusters/detectors are assumed to be paused (do not
  have to, but with high physics rate, it can happen sw. trigger won't go through).

  Return: 0 if succes, errorReason not touched
          1 if fails, errorReason filled
  Comment: 
    -set the ROC on all the FOs feeding detectors in partition
    -set the list of detectors for software trigger: L2_TCSET
    -send the software trigger
  Calls: GenSwtrg()
  Called by: ctp_Start/Stop/Sync Partition();
*/
int generateXOD(Tpartition *part,char x, char *errorReason, w32 *orbitn){
int i,ifo, idet, iattempt, iconnector, ret=0;
w32 xod,detectors=0;
w32 busyclusterT;
w32 testclust[NFO],roc[NFO];   // 20112006 von
char SEY[8];
char emsg[ERRMSGL];
 if(part == NULL){
  sprintf(emsg,"generateXOD error: part=NULL"); intError(emsg);
  strncpy(errorReason, emsg,ERRMSGL);
  return 1;
 }
 if(x == 'S') {xod=0xe; strcpy(SEY,"SOD");
  readALLcnts(part, 'S');
 } else if(x == 'E') {xod =0xf; strcpy(SEY,"EOD");
 } else if(x == 'Y') {xod =0xd; strcpy(SEY,"SYNC");
 } else{
  sprintf(emsg,"generateXOD error: wrong x: %c \n",x);
  strncpy(errorReason, emsg,ERRMSGL);
  intError(emsg);
  return 1;
 };
 /* Find out partition detectors, and program BUSY inputs:
    20112006: it is enough to set detectors variable which is
    than used to program BUSY board in GenSwtrg.
 */
 busyclusterT= 0;   // see too: findBUSYinputs(detectors);
 for(idet=0;idet<NDETEC;idet++){
   int busyinp;
   if((part->Detector2Clust[idet])==0) continue;  //no cluster associated
   detectors=detectors+(1<<idet);
   if((busyinp=validLTUs[idet].busyinp)) {  //BUSY line connected
     busyclusterT= busyclusterT | (1<<(busyinp-1));
   };
 };
 //20112006 von vmew32(BUSY_CLUSTER, busyclusterT);
 if(DBGbusy) {
   printf("genXOD: BUSY/SET_CLUSTER: 0x%x L2_TCSET:0x%x\n", 
     busyclusterT, detectors);
 };
 /* ROC on FO should be set (FO_TESTCLUSTER)
    20112006 von
 */
 for(i=0;i<NFO;i++){testclust[i]=0;roc[i]=0;}
 for(i=0;i<NDETEC;i++){
   if(part->Detector2Clust[i]){
     if(Detector2Connector(i,&ifo,&iconnector)){
       sprintf(emsg,"Detector2Connector() int error");
       strncpy(errorReason, emsg,ERRMSGL);
       return 1;
     };
     testclust[ifo]=testclust[ifo] +(1<<(16+iconnector));
     roc[ifo]=roc[ifo]+(xod<<(4*iconnector));  
     if(DBGswtrg) printf(
       "genXOD:%s: ifo=%i icon=%i testcl=0x%x roc=0x%x dets:0x%x\n",
         SEY,ifo,iconnector,testclust[ifo],roc[ifo], detectors);
    }
 }
 for(ifo=0;ifo<NFO;ifo++){   // set all FOs always
   //if((notInCrate(ifo+FO1BOARD)==0)) {
     if(DBGswtrg) printf("genXOD:%s: FO:%d Waddr: 0x%x data: 0x%x\n",
       SEY,ifo,FO_TESTCLUSTER+BSP*(ifo+1),roc[ifo] | testclust[ifo]);
     //20112006 von vmew32(FO_TESTCLUSTER+BSP*ifo,roc[ifo] | testclust[ifo]);
   //}
 }
 //software trigger with default setings
#define MAX_XOD_ATTEMPTS 2
iattempt=0;/*
if(x=='E') {
   setomSSM(5,0x202);   //26 ms, FO L2 monitor mode
   startSSM1(5);
};*/
/* SYN: because we want to have it in the middle of the orbit to set
 correctly orbit counter on INT board (synchronisation with L2) 
 The waiting time for 'SOD/EOD success' should match the waiting
 time in STD_ALONE mode -see ltu_proxy/ltu_utils.c sodeod().   
*/
if(strcmp(&part->name[strlen(part->name)-2],"_U")!=0) {
#ifndef TEST   
  while((GenSwtrg(1,'s', xod, 1750, detectors, 0, orbitn) == 0) && (iattempt<MAX_XOD_ATTEMPTS)){
   iattempt++;
   usleep(100000);
 };
#else
 prtLog("generateXOD; TEST mode: no attempt to generate.");
#endif
 if(DBGpriv) {iattempt=0; infolog_trg(LOG_WARNING, "DBGpriv, ignore sw trigger failure"); };
 if(iattempt>=MAX_XOD_ATTEMPTS){
   w32 deadbusys;
   char emsg[ERRMSGL];
   char ltunames[200];
   //sprintf(emsg,"      generate%cOD error: cannot generate sw trigger\n",x);prtError(emsg);
   deadbusys= findDeadBusys(detectors) & busyclusterT;
   findLTUNAMESby(deadbusys, detectors, ltunames);
   deadbusys= vmer32(INT_DISB_CTP_BUSY)&3;
   if(deadbusys !=0) { strcat(ltunames," CTP"); };
   sprintf(emsg, "%s cannot be sent because of dead detectors (run:%d):%s", 
     SEY, part->run_number, ltunames);
   infolog_trgboth(LOG_FATAL, emsg);
   // following emsg must start with 'detectorBusy' followed by
   // list of detectors separated by spaces, last detector
   // is followed by comma. Reason: errorReason is specially processed in ECS
   sprintf(emsg, "detectorBusy %s, %s cannot be sent", ltunames, SEY);
   strncpy(errorReason, emsg,ERRMSGL);
   ret=1;
 }else {
  printf("%s event ok at %d attempt.\n", SEY, iattempt);
  /*
  if(DBGswtrg) {
    for(ifo=0;ifo<NFO;ifo++){
      if((notInCrate(ifo+FO1BOARD)==0)) {
        w32 tcread;
        tcread= vmer32(FO_TESTCLUSTER+BSP*(ifo+1));
        printf("generateXOD:%c: FO:%d Raddr: 0x%x data: 0x%x\n",
          x,ifo,FO_TESTCLUSTER+BSP*(ifo+1),tcread);
      }
    }; 
  }; */
 };
} else {
  char emsg[ERRMSGL];
  sprintf(emsg, "%cOD generation suppressed (part. name: ..._U).\n", x);
  strncpy(errorReason, emsg,ERRMSGL);
  infolog_trgboth(LOG_FATAL, emsg);
  ret=2;
};
if(x == 'E') {
  readALLcnts(part, 'P');
};
 /* clean tc cluster word -no need (it is enough to set befor use)
 for(ifo=0;ifo<NFO;ifo++){
   if((notInCrate(ifo+FO1BOARD)==0)) 
     vmew32(FO_TESTCLUSTER+BSP*(ifo+1),0x0);
 }*/
 return ret;
}
/*-----------------------------------------------------------generateXODSSM()
 Purpose: Generates XOD from FO ssm in outgen mode.
          Trick to use exactly same sequence as in standalone mode
 Parameters: input: x='S' for SOD and x='E' for EOD
 Globals: reads ssm memory dumps from disk which contains SOD/EOD from ltu
 Returne: error code: 0=ok

int generateXODSSM(char x){
 int ret;
 stopSSM(5);
 if(x == 'S') ret=readSSMDump(5,"/home/alice/rl/v/vme/WORK/SSMsodDAQ.dump");
 else ret=readSSMDump(5,"/home/alice/rl/v/vme/WORK/SSMeodDAQ.dump");
 printf("Generating %cOD in FO mode\n",x);
 writeSSM(5);
 setomSSM(5,0x104);   //outgen 1 pass
 startSSM1(5);
 sleep(1);
 stopSSM(5);
 return ret;
}*/
/*----------------------------------------------------- resetclock */
void resetclock() {
char msg[80], cmd[40], dimcom[40];
//tag=TAGrcfgdelete;
sprintf(cmd,"resetclock\n");
sprintf(msg,"timestamp:resetclock:"); prtLog(msg);
strcpy(dimcom,"CTPRCFG/RCFG");
/*rcdic=*/ dic_cmnd_service(dimcom, cmd, strlen(cmd)+1);
}
/*----------------------------------------------------- gcalibUpdate */
void gcalibUpdate() {
//    res= pydim.dic_cmnd_service("CTPCALIB/DO", arg, "C")
//dic_cmnd_service("CTPCALIB/DO", (void *)"u", 2);
dic_cmnd_callback("CTPCALIB/DO", (void *)"u", 2,&callback,TAGglobalcal);
};
/*----------------------------------------------------- xcounters */
void xcountersStart(w32 run, w32 clgroup) {
char com[100],msg[254];
int size; int tag; int runcg[2];
//int xrc; char xpid[20]="";
//                       printf("xcountersStart removed\n"); return;
/* check xcountersdaq active:
moved to DOrcfg in pydim (xcounters is running there)
xrc= popenread((char *)"ps --no-headers -C xcountersdaq -o pid=", xpid, 20);
if(xpid[0]=='\0') {
  infolog_trgboth(LOG_FATAL, "xcounters problem, stop all global runs, call CTP expert");
  quit=1;
};*/
if(run != 0) { // do not print log for 'forced counters reading'
  /*sprintf(msg,"Run %i: starting xcounters. clgroup:%d xpid:%s popen rc:%d",
    run, clgroup, xpid, xrc);  */
  sprintf(msg,"Run %i: starting xcounters. clgroup:%d", run, clgroup);
  prtLog(msg);
  tag=TAGstartcount;
} else {
 tag=TAGstartcountforced;
};
strcpy(com,"CTPDIM/STARTRUNCOUNTER");
/* following action:
 - starts xcounters
 - moves .rcfg file to WORK/RCFG/delme/ directory
 That's why group owner of RCFG is fes on alidcscom026:
 drwxrwxr-x  4 trigger fes      4096 Mar  9 16:00 RCFG
 drwxrwxr-x  3 trigger fes   20480 Apr  2 08:20 delme
*/
if((run!=0) && ((w32)fixpos[0]==run)) {  // not forced, PHYSICS_1
  fixpos[1]= clgroup;
  size= lenfixpos*sizeof(int);   
  dic_cmnd_callback(com,&fixpos[0], size,&callback,tag);
} else {
  size=sizeof(runcg);
  runcg[0]=run; runcg[1]=clgroup;
  dic_cmnd_callback(com,&runcg[0], size,&callback,tag);
};
}
void xcountersStop(w32 run) {
 char com[100],msg[100];
 int size;
 //                       printf("xcountersStop removed\n"); return;
 sprintf(msg,"Run %i: stopping xcounters.",run);
 strcpy(com,"CTPDIM/STOPRUNCOUNTER");
 prtLog(msg);
 size=sizeof(run);
 dic_cmnd_callback(com,&run, size,&callback,TAGstopcount);
 /* ??? Registration of the closed file should be 
 on DCS FES with offline tag CTP_xcounters. 
 See v/vme/dimcoff/dimccounters.c -the executable of this
 DIM client is placed in tri@alidcscom026 from where it is started.
 This client should invoke (after closing the xcounters file)
 dcsFES_putData.sh to register it on DCS FES */
}

/*int registerRunConfig(w32 run) {
// Offline tag: CTP_runconfig

int rc=0;
//char cmd[400];
char runcfg[100];
getruncfgname(run, runcfg);
** see tri@alidcscom026:dcsFES_putData.sh 
Offline tag for .rcfg file: CTP_runconfig
dcsFES_putData.sh RUN# DET FILEID INPUTFILE_full_path
Example:
~tri/dcsFES_putData.sh 2 TRI CTP_runconfig $VMEWORKDIR/WORK/RCFG/r2.rcfg
Last idea:
dcsFES_putData.sh will not be called from here. dcsFES_putData.sh
should be modified (run number is known in this script, and
this is only parameter we need) to copy and delete .rcfg file
just before xcounters file at the time of xcountersStart() 
**
printf("registerRunConfig NOACTION: run:%d %s\n", run, runcfg);
return(rc);
}*/
//--------------------------------------------------------------------
// Routines for smi:
//--------------------------------------------------------------------
// Initialise
//w8 *mallocShared(w32 shmkey, int size, int *segid);
extern char TRD_TECH[];   /* partition name in case it is TRD partition
                             see checkmodLM() */
int ctp_Initproxy(){
int sp,rc; char *environ;
char msg[300], cmd[100], dimcom[40], alipath[120];
#define MAXALIGNMENTLEN 4000
char alignment[MAXALIGNMENTLEN];
TRD_TECH[0]='\0';
rc= vmeopen("0x820000", "0xd000");
if(rc!=0) {
  printf("vmeopen CTP vme:%d\n", rc); exit(8);
};
printf("ctp_proxy ver: 06.04.2015\n");
xcountersStop(0);           // clear list of active runs
SDGinit();
checkCTP();   /* check which boards are in the crate - ctpboards */
cshmClear();
readTables(); // onlly in ctp_proxy and pydim/server.c
if(initHW(&HWold)) return 1; // initialise and clean HWold structure
if(initHW(&HW)) return 1;   // initialise and clean HW structure
initCTP();    /* has to be after initHW(&HW). Init system pars in CTP boards + INT* in HW */

/* alignment info has to be created AFTER ctp.cfg reading: */
getctp_alignment(NULL, alignment, MAXALIGNMENTLEN, 0);
if(alignment=='\0') {
  infolog_trgboth(LOG_FATAL, "Alignment info for DAQlogbook is empty");
} else {;
  environ= getenv("VMEWORKDIR");sprintf(alipath, "%s/WORK/alignment2daq",environ);
  sp= writedbfile(alipath, alignment);
  if(sp==0) {
    infolog_trgboth(LOG_FATAL, "Alignment info not written");
  };
};
// 
 // to do: read run database WORK/RCFG
 //DAQBUSY.activebusys=vmer32(BUSY_DAQBUSY)&0x3f;
 // but now:
 load2HW(&HW, "");
 /* initialise hardware ('not configurable' part i.e.: busy, TC_CLEAR):
 */
DAQBUSY.global=0; DAQBUSY.paused_dets=0xffffff;
DAQBUSY.paused_dts=0; DAQBUSY.clust_dts=0;  // 'fast' pause for SYNC
DAQBUSY.activebusys=0x3f; 
vmew32(BUSY_DAQBUSY, DAQBUSY.activebusys);  // was 0x0 before LS1!
clearflags();
printf("ctp_Initproxy: cleaning CTP hw (all classes \
disabled, all BUSYs cleaned, swtrg flags cleaned.\n");

/* check DIM server is running: */
if(cshmGlobFlag(FLGignoreDAQLOGBOOK)) {
  sprintf(cmd,"rcfgdel ignoreDAQLOGBOOK\n");
} else {
  sprintf(cmd,"rcfgdel useDAQLOGBOOK\n");
};
sprintf(msg,"timestamp:TAGctprestart: %s", cmd); prtLog(msg);
strcpy(dimcom,"CTPRCFG/RCFG");
rc= dic_cmnd_callback(dimcom, cmd, strlen(cmd)+1, callback, TAGctprestart);
printf("rc from \"CTPRCFG/RCFG rcfgdel ignore/useDAQLOGBOOK\":%d (1 is OK,but check callback)\n", rc);
usleep(1000000);
prepareRunConfig(NULL,2);  // mv all lurking .rcfg to delmeh/
usleep(1000000);
if(pydimok==1) {rc=0; } else {rc=1;};

#ifdef TEST
rc=0; printf("TEST mode, rc forced to 0\n");
#endif
return(rc);
}
int ctp_Endproxy() {
int rc=0, rc1=0,idet, vsp;
printf("ctp_Endproxy: Forcing all partitions to stop...\n");
ctp_StopAllPartitions();
printf("ctp_Endproxy: closing VME...\n");
for(idet=0;idet<NDETEC;idet++){
  if((vsp= validLTUs[idet].ltuvsp)>0) {
    rc1= vmxclose(vsp);
    printf("vmxclose LTU %s vme:%d\n", validLTUs[idet].name, rc);
  };
}
rc=vmeclose();
if(rc!=0) {
  printf("vmeclose CTP vme:%d\n", rc);
};
return(rc+rc1);
}
/*---------------------------------------------ctp_PausePartition()
 * Standard Pause
*/
int ctp_PausePartition(char *name, int detectors){
Tpartition *part; int ix, rc=0;
infolog_SetStream(name,-1);
part=getPartitions(name, StartedPartitions);   //only Started can be paused
if(part == NULL) { rc=1; goto ERR; };
infolog_SetStream(name, part->run_number);
for(ix=0; ix<NDETEC; ix++) {   // let's check first
  if((detectors & (1<<ix))==0) continue;
  if(part->Detector2Clust[ix]==0) {   // if belongs to this partition
    Tdetector *ltup; char emsg[300];
    ltup= findLTUdaqdet(ix);
    sprintf(emsg,"attempt to pause the detector %s not included in %s",
      ltup->name,name);
    infolog_trgboth(LOG_ERROR, emsg);
    rc=2; goto ERR;
  };
  if((DAQBUSY.paused_dets & (1<<ix))==1) {   // if paused already
    Tdetector *ltup; char emsg[300];
    ltup= findLTUdaqdet(ix);
    sprintf(emsg,"pausing the detector %s already paused in partition %s",
      ltup->name,name);
    infolog_trgboth(LOG_WARNING, emsg);
  };
};
//paused_dets= DAQBUSY.paused_dets | detectors; <-done in setPartDAQBusy
cshmPausePartition(part);   // just flag 'paused' (no info about clusters)
setPartDAQBusy(part, detectors);
if(part->nclassgroups  > 0 ) {
  part->remseconds= stopTimer(part, 255);   // TS group 255 after reading
};
usleep(200); // temporary: from 24.11.2011 15:15 readALLcnts(part, 'P');
//printf("\n ctp_PausePartition: SUCCES \n");
ERR:
infolog_SetStream("", 0);
return(rc);
}
/*----------------------------------------------------------------
 Pause when in FO outgen mode
int ctp_PausePartition(char *name,char *mask){
 setALLDAQBusy();
 stopSSM(5);
 return 0;
}
*/
/*---------------------------------------------ctp_SyncPartition()
before LS1: allowed only when whole partition paused
after LS1: allowed at any time for all detectors (paused/unpaused) of partition
*/
int ctp_SyncPartition(char *name, char *errorReason, w32 *orbitn) {
Tpartition *part; int rc=0; int src;
char emsg[300];
infolog_SetStream(name,-1);
part=getPartitions(name, StartedPartitions);   //only Started can be synced
if(part == NULL) return 1;
infolog_SetStream(name, part->run_number);
/* von if(cshmQueryPartition(part)!=1) {
  sprintf(emsg,"SYNC not sent (partition not PAUSED)");
  infolog_trgboth(LOG_ERROR, emsg);
  strncpy(errorReason, emsg,ERRMSGL); rc=2;
} else {   */
//bakery lock  -should be combined with detectors== -1 (i.e. not done in setPartDAQbusy, but here)
setPartDAQBusy(part, -1);
src=generateXOD(part,'Y', errorReason, orbitn);
unsetPartDAQBusy(part, -1);
//bakery unlock
  if(src==0) {
    //src= generateXOD(part,'S', emsg ); bug -was here till 11.7.2012 !!!
    sprintf(emsg,"SYNC sent, orbitn:%u.", *orbitn); 
    infolog_trgboth(LOG_INFO, emsg);
  } else {
    sprintf(emsg,"SYNC not sent. generateXOD() rc:%d orbitn:%u", src, *orbitn); 
    infolog_trgboth(LOG_ERROR, emsg);
    rc=3;
  };
//};
//usleep(200); // temporary: from 24.11.2011 15:15 readALLcnts(part, 'P');
//printf("\n ctp_PausePartition: SUCCES \n");
infolog_SetStream("", 0);
return(rc);
}
/*---------------------------------------------ctp_ResumePartition()
 * Standard resume
*/
int ctp_ResumePartition(char *name, int detectors){
Tpartition *part; int ix,rc=0;
infolog_SetStream(name, -1);
part=getPartitions(name, StartedPartitions);
if(part == NULL) { rc=1; goto ERR; };
infolog_SetStream(name, part->run_number);
for(ix=0; ix<NDETEC; ix++) {   // let's check first
  if((detectors & (1<<ix))==0) continue;
  if(part->Detector2Clust[ix]==0) {   // if belongs to this partition
    Tdetector *ltup; char emsg[300];
    ltup= findLTUdaqdet(ix);
    sprintf(emsg,"attempt to leave detector %s paused not included in %s",
      ltup->name,name);
    infolog_trgboth(LOG_ERROR, emsg);
    rc=2; goto ERR;
  };
  if((DAQBUSY.paused_dets & (1<<ix))==0) {   // if paused already
    Tdetector *ltup; char emsg[300];
    ltup= findLTUdaqdet(ix);
    sprintf(emsg,"attempt to leave active detector %s paused. Partition: %s",
      ltup->name,name);
    infolog_trgboth(LOG_ERROR, emsg);
    rc=3; goto ERR;
  };
};
if(part->nclassgroups  > 0 ) {
  startTimer(part, part->remseconds, part->active_cg); part->remseconds=-1;
};
unsetPartDAQBusy(part, detectors); 
cshmResumePartition(part); gcalibUpdate();
//printf("\n ctp_ResumePartition: SUCCES \n");
ERR:
infolog_SetStream("", 0);
return(rc);
}
/*------------------------------------------------ctp_StopPartition()
Stop the trigger.
Operation:
Loaded partition: only remove from AllPArtitions
Started: remove from All/Started Partitions and reload HW

rc: 0: OK, EOD generated, partition unloded
    2: problem when  unloading from HW
*/
int ctp_StopPartition(char *name){
 int ret, rc=0;
 w32 run_number, orbitn;
 Tpartition *part, *tspart;
 char tsname[MAXNAMELENGTH]="";
 char emsg[ERRMSGL]="";
 infolog_SetStream(name,0);
 part=getPartitions(name, StartedPartitions); 
 if(part == NULL) { // only loaded (not in HW)
   part=getPartitions(name, AllPartitions); 
   if(part == NULL) { rc= 1; 
     sprintf(emsg,"Attempt to stop partition %s -not loaded before",name);
     goto RETSTOP_badsyntax; //nothing to do,partition never existed
   } else {
     infolog_SetStream(name, part->run_number);
     prepareRunConfig(part,0);
   };
   ret= deletePartitions(part); part=NULL;
   if(ret != 1){
     sprintf(emsg,"deletePartition %s inconsitent: %d",name,ret);
     goto RETSTOP_badsyntax; //nothing to do,partition never existed
   }
   // normally, .rcfg file is deleted by starting xcountersStart in
   // ctp_StartPartition. We should delete it here, because we did not
   // come to the point where partition is started
   goto RET;
 } else {
   infolog_SetStream(name, part->run_number);
 };
 // started partition: delete in All/Started + reload HW
 run_number= part->run_number;
setPartDAQBusy(part, 0);   //von setALLDAQBusy();  
usleep(150);   /* 100-> 150 4.9.2012  L2time is 105.2us */
                /* to be sure CTP is quiet when reading counters at the EOR */
 xcountersStop(run_number);
 if(part->nclassgroups  > 0 ) {
   ret= stopTimer(part,0xfffffffe);  // do not read counters
 };
/*usleep(2000);  to keep trigger rate <1kHz for DDG*/
usleep(1000000);  /* asked by DAQ -see mail/daq from 12.12.2009 */
cshmDelPartition(part->name);
// xcountersStop(run_number);   moved up
//von unsetPartDAQBusy(part, 0);    // in case it was 'paused'
if(generateXOD(part,'E', emsg, &orbitn )) {
  infolog_trgboth(LOG_INFO, emsg);
  sprintf(emsg,"EOD failure for partition %s.",name); 
  printf("%s\n",emsg); emsg[0]='\0';
  //goto RETSTOPunset;  anyhow, we have to relese hw
};
prepareRunConfig(part,0);
/* not used withfull LM implementation...
if(strcmp(part->name, TRD_TECH)==0) {
  // disconnect RND1, connected becasue of TRD in techn. partition
  vmew32(RND1_EN_FOR_INPUTS, 0);
  vmew32(RND1_EN_FOR_INPUTS+4, 0);
  TRD_TECH[0]='\0';
  prtLog("RND1_EN_FOR_INPUTS and TRD_TECH cleared");
};*/
ret=deletePartitions(part); tspart= checkTS();
if(tspart!=NULL) {
  strcpy(tsname, tspart->name);
} else {
  //if there is no TS partition, set active_cg to 0:
  ctpshmbase->active_cg=0;
};
 if(ret!=3) {   // was not in both list
   rc=1;
   sprintf(emsg,"ERROR. rc:%d from deletePartitions(%s)",ret, name);
 };
 if((ret=addPartitions2HW(StartedPartitions))){ rc=ret;  // to check !!
   sprintf(emsg, 
   "addPartition error when stopping partition %s.", 
   name);
 } else {
   if(load2HW(&HW, tsname)) {rc=2;
     sprintf(emsg,"load2HW failure for partition %s.",name);
   };
 };
 //registerRunConfig(run_number);
//RETSTOPunset:
//von unsetALLDAQBusy();
 //printf("\n ctp_StopPartition: SUCCES \n");
RETSTOP_badsyntax:
if(emsg[0]!='\0') printf("%s\n",emsg);
RET:
if(quit==1) {
  if(getNAllPartitions()==0) {
    sprintf(emsg,"ctp_proxy stopping (no active partitions)");
    quit=10;
  } else {
    sprintf(emsg,"ctp_proxy still waiting before exit (there are still active partitions)");
  };
  infolog_trgboth(LOG_INFO, emsg);
}
infolog_SetStream("",0);
return rc;
}
#define MAXMSG 1000
/*---------------------------------------------ctp_LoadPartition()
*/
int ctp_LoadPartition(char *name,char *mask, int run_number, 
    char *ACT_CONFIG, char *errorReason) {
int rc=0;
Tpartition *part;
char msg[MAXMSG];
errorReason[0]='\0';
part=getPartitions(name, AllPartitions); 
if(part==NULL) { 
  rc= ctp_InitPartition(name,mask,run_number,ACT_CONFIG, errorReason);
} else {
  infolog_SetStream(name,0);
  part=getPartitions(name, StartedPartitions); 
  if(part==NULL) { 
    sprintf(msg, "%s already loaded (earlier by INIT_PARTITION)",name);
    infolog_trgboth(LOG_INFO, msg); 
  } else {
    sprintf(msg, "Attempt to load %s, which is already started",name);
    infolog_trgboth(LOG_ERROR, msg); 
    strncpy(errorReason, msg,ERRMSGL); rc=5;
  };
  infolog_SetStream("",0);
};
return rc;
}
/*---------------------------------------------ctp_InitPartition()
Timeout when loading partition is 20 seconds (including
the switch of all LTUs global->stdalone).

 Operation: 
 0. copy existing HW to hwold
 1. read database, check resources
 2. if OK then: add it to Partitions and create new hw file
    else go to 7.
 3. pause ALL clusters
 4. generate SOD
 5. load HW
 6. resume all clusters
    end
 7. copy hwold to HW
    end
 Return : error flag= 0 ok
                      1 not enough resources
                      2 any other error
 Input: partition name, mask
 mask: decimal or hex (0x..) number, 24 bits
       1: for valid detectors, 0: not valid
       "" (empty string): do not apply mask
 Notes about mask:
 - the mask bits corresponds to detector table given in VALID.LTUS
 - masking is done by programming FO and BUSY board:
   - 0 -> corresponding FO 8-bit word
   - BUSY input is ignored from this detector
   STATIC option(current): masking allowed only at the time of partition start.
   DYNAMIC option (not foreseen from DAQ point of view): 
   Only if DAQ is capable to mask detectors during run. 
   It sems, CTP is capable to do it (i.e. another
   commands for paused (perhaps even running) partitions: 
   APPLY_MASK (and possibly UNMASK) should be prepared
   In this case, masked detectors should keep resources all the time,
   regardless wheter they are masked or not.

rc: if !=0, errorReason set
1 : cannot create .pcfg file
2: pcfg cannot be processed
3: applyMask problem 
4: addPartitions() problem
5: checkmodLM (TRD in) partition problem
*/
int ctp_InitPartition(char *name,char *mask, int run_number, 
    char *ACT_CONFIG, char *errorReason) {
int ret=0, rc=0;
char name2[80];
char msg[MAXMSG];
Tpartition *part;
/* 
way of masking: mask is applied in memory directly
    after part. definition is read in
 */
errorReason[0]='\0';
/* removed 3.6.2015 (i.e. was here at the end of run1)
if((getPartitionsN(AllPartitions)==0) && strcmp(name,"PHYSICS_1")==0) {
  resetclock();
}; */
infolog_SetStream(name,0);
part=getPartitions(name, AllPartitions); 
if(part!=NULL) { 
  sprintf(msg, "Attempt to Init %s, which is already loaded or started",name);
  infolog_trgboth(LOG_ERROR, msg); 
  strncpy(errorReason, msg,ERRMSGL); rc=5; goto RET2;
};
infolog_SetStream(name, run_number);
//------------------------------------------- prepare fresh .pcfg file:
if( partmode[0] == '\0'){strcpy(name2, name);}else{ strcpy(name2, partmode); };
sprintf(msg,"rm -f /tmp/%s.pcfg", name2); ret=system(msg);
prtProfTime("get pcfg");
preparepcfg(name2, run_number, ACT_CONFIG);
sprintf(msg, "/tmp/%s.pcfg", name2);
ret= detectfile(msg, 3);  // wait max. 3 (was 39 at the end of run1) secs for file
sprintf(msg,"timestamp:pcfg2: name2:%s name:%s...", name2, name); prtLog(msg);
if(ret<=0) {
  sprintf(msg, "Wrong partition definition (cannot create .pcfg file) ret:%d",ret);
  infolog_trgboth(LOG_FATAL, msg); strncpy(errorReason, msg,ERRMSGL); rc=1; goto RET2;
};
if(partmode[0] != '\0') {
  sprintf(msg,"mv /tmp/%s.pcfg /tmp/%s.pcfg", name2, name);
  ret= system(msg); 
  if(ret!=0) {
    sprintf(msg, "cannot create .pcfg file (mv).Mode:%s ", partmode);
    infolog_trgboth(LOG_FATAL, msg); strncpy(errorReason, msg,ERRMSGL); rc=1; goto RET2;
  };
};
checkPCFG(name, msg, MAXMSG); //printf("checkPCFG ret:%s.\n",msg);
if(msg[0]!='\0') {
  infolog_trgboth(LOG_FATAL, msg); 
  strncpy(errorReason, "partition definition incorrect",ERRMSGL); 
  rc=1; goto RET2;
};
prtProfTime("got pcfg");
copyHardware(&HWold,&HW);   // HWold <- HW
/* From now: seems HW is touched ONLY in addPartitions2HW !
i.e. we restore HW in case of:
- not error found
- addPrtitions2HW error
*/
sprintf(msg,"timestamp:reading partition %s %d", name, run_number); prtLog(msg);

part=readDatabase2Tpartition(name); 
if(part == NULL) {  
  strncpy(errorReason, "Cannot read partition definition file (.pcfg)", ERRMSGL); 
  rc=2;goto RET2; };
part->run_number= run_number;
// convention mask="" - mask is not used: applymask creates mask
//                      acoording to partition
printTpartition("Before mask applied", part);
if(applyMask(part, mask)) { 
  strncpy(errorReason, "Are the readout detectors a subset of detectors allowed in partition being started?", ERRMSGL);
  rc=3; goto RET2; };
if(DBGparts) { printTpartition("After mask applied", part); };
sprintf(msg,"timestamp:mask applied %s %d", name, run_number); prtLog(msg);
if((ret=checkResources(part))) {
   strncpy(errorReason, "Not enough CTP resources for this partition", ERRMSGL);
   rc=ret; ret=deletePartitions(part); part=NULL;
   goto RET2; };
//printTpartition("After checkResources", part);
ret= checkmodLM(part);   // not good idea (better: in START_PARTITION)
if(ret!=0) {
  rc=5; ret=deletePartitions(part); part=NULL;
  goto RET2; };
// If resources available, continue and add part to Partitions[]
// From now on, no checks necessary (all checks already done)
if(addPartitions(part)) { 
  strncpy(errorReason, "Cannot add partition", ERRMSGL);
  rc=4; ret=deletePartitions(part); part=NULL;
  prtError("addPartitions error."); goto RET2; };
if(DBGparts) {
  printf("Partitions after adding partition:%s\n",part->name);
  printAllTp();
};
sprintf(msg,"timestamp:partition merged: %s %d", name, run_number); prtLog(msg);

if((ret=addPartitions2HW(AllPartitions))){ //just check if enough resources
  printf("addPartitions2HW error: %i \n", ret);   
  strncpy(errorReason, "Cannot load partition", ERRMSGL);
  rc=ret; deletePartitions(part); part=NULL;
  copyHardware(&HW,&HWold); // discard 'addPartitions2HW(AllPartitions)' actions:
  goto RET2;
};
if(DBGparts) { 
  printTpartition("After addPartitions2HW:", part); 
  printTRBIF(HW.rbif);
};

sprintf(msg,"timestamp:partition inHW: %s %d", name, run_number); prtLog(msg);
//we already know HW configuration (allocation of physics resources):
rc= getDAQClusterInfo(part, &daqi);
daqi.daqonoff= vmer32(INT_DDL_EMU) &0xf;
prtProfTime("get rcfg");
prepareRunConfig(part,1); 
//has to be here (2 secs for pydimserver to prepare .rcfg file
// 27.1.2012: following usleep commented:
//usleep(2000000);        // 1sec is not enough, 2 seems enough before SOD
rc= updateDAQClusters(part);
sprintf(msg, "timestamp:rc:%d from updateDAQClusters()\n", rc); prtLog(msg);
/*partition is 'loaded' */
if(rc!=0) {
 strncpy(errorReason, "updateDAQClusters() problem", ERRMSGL);
 prepareRunConfig(part,0);
 deletePartitions(part); part=NULL;
};
//printHardware(&HW,"ctp_InitPartition");
copyHardware(&HW,&HWold); // discard 'addPartitions2HW(AllPartitions)' actions:
RET2:
infolog_SetStream("",0);
sprintf(msg, "timestamp:ctp_InitPartition finished %s %d", name, run_number);
prtLog(msg);
return rc;
}
/*---------------------------------------------ctp_StartPartition()
Return : 0 ok
         1 partition was not loaded before calling StartPartition()
         2 partition already started
         3 SOD not sent (busy detector(s)
         4 load HW problem
         5 CTP readout enabled, but DDL link not ready
*/
int ctp_StartPartition(char *name, char *errorReason) {
int ret,rc=0, clgroup; w32 intddlemu, orbitn;
Tpartition *part, *tspart;
char tsname[MAXNAMELENGTH]="";
char emsg[ERRMSGL];
infolog_SetStream(name,0); errorReason[0]='\0';
part=getPartitions(name, StartedPartitions); 
if(part!=NULL) { 
  rc=2; sprintf(emsg, "Partition %s already started.", name);
  strncpy(errorReason, emsg, ERRMSGL);
  infolog_trgboth(LOG_FATAL, emsg);
  goto RET;
};
part=getPartitions(name, AllPartitions); 
if(part==NULL) { 
  rc=1; sprintf(emsg, "Partition %s not loaded.", name);
  strncpy(errorReason, emsg, ERRMSGL);
  infolog_trgboth(LOG_FATAL, emsg);
  goto RET; };
infolog_SetStream(name, part->run_number);

copyHardware(&HWold,&HW);
addStartedPartitions(part); 

tspart= checkTS();if((tspart!=NULL) && (tspart!=part)) {
  // there is already another TS partition
  strcpy(tsname, tspart->name);
};
addPartitions2HW(StartedPartitions);
 
/* todo: it is sufficient to call setALLDAQBusy just before load2HW()
   i.e. generateXOD() can be called 'on the fly'
*/
//von setALLDAQBusy(); 
clearflags();
intddlemu= vmer32(INT_DDL_EMU);
if((intddlemu&0xf)==0) {   //check DDL if DAQ is active
  if((intddlemu&0x70)!=0x30) {
/* note 17.11.2014:
when switch off/on the crate, or disconnect/connect DDL,
fiBEN bit (0x10) goes to 0 and cannot be set to 1 from our side
by enabling DAQ readout. It gets set only when CTP server at other
side of DDL is restarted (tested today with Sylvain in lab).
Fix: 
CTPserver will go down automatically when DDL is not ready
(Sylavain is going to modify it this way). When ctp reconnects DDL,
CTPserver needs to be restarted, which set fiBEN bit ON
*/
    char msg[200];
    sprintf(msg,"Run %d: CTP DDL link full or in bad state before sending SOD.\
 INT_DDL_EMU:0x%x expected: 0x30",
      part->run_number, intddlemu);
    infolog_trgboth(LOG_FATAL, msg);
    //von prtLog(msg);
    strncpy(errorReason, msg, ERRMSGL);
    rc= 5; goto UNSETRETddl;
  };  
};
setPartDAQBusy(part, 0);   // added 13.2 2015  (seems we did not pause at start!)
usleep(200);   // be sure, CTP is quiet when reading counters at SOR
if(generateXOD(part,'S', errorReason, &orbitn)) {
  // SOD was not delivered (busy)
  //infolog_trgboth(LOG_ERROR, "SOD not sent (busy)");
  rc= 3; goto UNSETRET;
  /*if((ret=addPartitions2HW())){ 
    printf("addPartitions error: %i \n", ret);
    goto UNSETRET;}; */
};
//usleep(1100000);   // maybe more for rorc initialisation
if(load2HW(&HW, tsname)){
  strncpy(errorReason, "ctpproxy: internal error found in load2HW()", ERRMSGL);
  rc= 4; goto UNSETRET;
}
cshmAddPartition(part);
usleep(2000000);   // at least 2secs for: DIM transmission + read counters
disableNEVERACTIVE(part);
clgroup= nextclassgroup(part);
if(clgroup > 0) {
  int ix; char msg[200];
  strcpy(msg,"TIMESHARING:");
  for(ix=0; ix<MAXCLASSGROUPS; ix++) {
    sprintf(msg,"%s %d", msg, clg_defaults[ix]);
  }; 
  infolog_trgboth(LOG_INFO, msg);
  enableclassgroup(part, clgroup); startTimer(part, 0, 0xfffffffe);
} else {
  clgroup= 0xffffffff; // if this part not using TS, do not update shm
};
xcountersStart(part->run_number, clgroup);
//UNSETRETadb: 
unsetPartDAQBusy(part, 0);   //von unsetALLDAQBusy();
gcalibUpdate();
//printf("\n ctp_StartPartitions: SUCCESS \n");
RET:
infolog_SetStream("",0);
return rc;
UNSETRETddl:
  prepareRunConfig(part,0);
  copyHardware(&HW,&HWold);
  deletePartitions(part);part=NULL;
  /*goto UNSETRETadb;
  no gcalib/busys -they were not updated anyhow (UNSETRET) */
  goto RET;
UNSETRET:
  prepareRunConfig(part,0);
  copyHardware(&HW,&HWold);
  unsetPartDAQBusy(part, 0);   //von unsetALLDAQBusy();
  deletePartitions(part); part=NULL;
  gcalibUpdate();
  goto RET;  //UNSETRETadb;
}

