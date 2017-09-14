#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "infolog.h"
#include "vmewrap.h"
#include "ctp.h"
#include "ctplib.h"
#include "daqlogbook.h"
#include "Tpartition.h"
#include "lexan.h"

char TRD_TECH[24]="";
/*  SDGS operations
 line: "SDG name 0x23"
*/
int SDGadd(char *line, char *pname) {
int ix,ixf=-1;
char name[MAXSDGNAME]; w32 dsf;
enum Ttokentype token; char value[80];
// line-> name dsf
ix= 4; token=nxtoken(line, value, &ix);
if((token==tSYMNAME) && (strlen(value)<=(MAXSDGNAME-1)) ) {
  strcpy(name, value);
  token=nxtoken(line, value, &ix);
  if(token==tHEXNUM) {
    dsf= hex2int(&value[2]);
  } else {
    char emsg[300];
    sprintf(emsg,"Hexadecimal number expected in 3rd column:%s",line);
    infolog_trgboth(LOG_ERROR, emsg);
    return(1);
  };
} else {
  char emsg[300];
  sprintf(emsg,"Symbolic name (max. %d chars) expected in 2nd column:%s",
    (MAXSDGNAME-1), line);
  infolog_trgboth(LOG_ERROR, emsg);
  return(1);
};
for(ix=0; ix<NCLASS; ix++) {
  if(strcmp(SDGS[ix].name,"")==0) {
    ixf= ix ; break;
  };
};
if(ixf>=0) {
  strcpy(SDGS[ixf].name, name);
  strcpy(SDGS[ixf].pname, pname);
  SDGS[ixf].l0pr= dsf;
  SDGS[ixf].firstclass= 0;
  if(ixf>=NSDGS) NSDGS= ixf+1;
  printf("SDGadd: %s %s 0x%x\n", name, pname, dsf);
  return(0);
}; 
infolog_trgboth(LOG_ERROR, "too many SDG definitions in all partitions");
return(1);
}
int SDGfind(char *name, char *pname) {
int ix,ixf=-1;
for(ix=0; ix<NSDGS; ix++) {
  //printf("SDGfind: name %s %s \n",SDGS[ix].name,name);
  //printf("SDGfind: pname %s %s \n",SDGS[ix].pname,pname);
  if((strcmp(SDGS[ix].name,name)==0) &&
     (strcmp(SDGS[ix].pname,pname)==0)) {
    ixf= ix ; break;
  };
};return(ixf);
}
void SDGinit(){
int ix;
for(ix=0; ix<NCLASS; ix++) {
  SDGS[ix].name[0]='\0';
  SDGS[ix].pname[0]='\0';
  SDGS[ix].l0pr= 0;
  SDGS[ix].firstclass= 0;
}; NSDGS=0;
}
void SDGclean(char *pname){
int ix,newNSDGS=NSDGS;
for(ix=0; ix<NSDGS; ix++) {
  if(strcmp(SDGS[ix].pname, pname)==0) {
    SDGS[ix].name[0]='\0';
    if(ix==(newNSDGS-1)) newNSDGS--;
  };
}; NSDGS= newNSDGS;
}

/*------------------------------------------------------------ copyPF
 * obsolete
void copyPF(TPastFut *newpf, TPastFut *pf) {
int ix;
for(ix=0; ix< ixMaxpfdefs; ix++) {
  //newpf->pfdefs[ix]= pf->pfdefs[ix]
  strcpy(newpf->name,pf->name);
  newpf->bcmask=pf->bcmask;
  newpf->PeriodBefore=pf->PeriodBefore;
  newpf->PeriodAfter=pf->PeriodAfter;
  newpf->NintBefore=pf->NintBefore;
  newpf->NintAfter=pf->NintAfter; 
  newpf->OffBefore=pf->OffBefore;
  newpf->OffAfter=pf->OffAfter; 
  //pfdefs[ixMaxpfdefs];
};
}
int checkPF(TPastFut *newpf, TPastFut *pf) {
int ix;
for(ix=0; ix< ixMaxpfdefs; ix++) {
  //if(newpf->pfdefs[ix]!= pf->pfdefs[ix] ) return 1;
}; return 0;   // ok
}
*/
/*
void copyPFC(TPastFutCommon *newpfc, TPastFutCommon *pfc) {
int ix;
for(ix=0; ix< ixMaxpfdefsCommon; ix++) {
  newpfc->pfdefsCommon[ix]= pfc->pfdefsCommon[ix]; 
};
}
int checkPFC(TPastFutCommon *newpfc, TPastFutCommon *pfc) {
int ix;
for(ix=0; ix< ixMaxpfdefsCommon; ix++) {
  if(newpfc->pfdefsCommon[ix]!= pfc->pfdefsCommon[ix]) return 1; 
}; return 0;
}
*/
/*--------------------------------------------------------- copycheckPF
Obsolete
Check or copy:
  rbif->pf[ixpf] and rbif->pfCommon  -> rbifnew
rc:0 ok
*/
/*int copycheckPF(TRBIF *rbifnew, TRBIF *rbif, int ixpf) {
int rc=0;
if(rbifnew->PFuse[ixpf]>0) {  // check 
  rc= checkPF(&rbifnew->pf[ixpf], &rbif->pf[ixpf]);
} else {                     // copy (not used yet)
  copyPF(&rbifnew->pf[ixpf], &rbif->pf[ixpf]);
  rbifnew->PFuse[ixpf]= 1;
  if(rbifnew->PFCuse>0) {     //check
    rc= checkPFC(&rbifnew->pfCommon, &rbif->pfCommon);
  } else {                   // copy
    copyPFC(&rbifnew->pfCommon, &rbif->pfCommon);
    rbifnew->PFCuse= 1;
  };
};
return rc;
}
*/
/*------------------------------------------------------------cleanTRBIF()
If leaveint1!=0: do not touch int1/2/t definitions
*/
void cleanTRBIF(TRBIF *rbif, int leaveint){
int ix,i,j;
//char bcmask[ORBITLENGTH];
if(rbif != NULL){
  for(i=0; i<ixrbifdim; i++) {
    if( (i>=ixintfun1) && (i<=ixintfunt) && (leaveint!=0)) continue;
    rbif->rbif[i]=0;
    rbif->rbifuse[i]=notused;
  };
  for(ix=0; ix<L0FINTN*L0INTFSMAX; ix=ix+L0INTFSMAX) {      
    if( (ix>=ixintfun1) && (ix<=ixintfunt) && (leaveint!=0)) continue;
    rbif->l0intfs[ix]= '\0';
  };
  if(l0C0()>=0xc606) {
    for(ix=0; ix<8; ix++) {
      strcpy(&rbif->lut8[ix*LUT8_LEN],"0x0000000000000000000000000000000000000000000000000000000000000000");
    };
  };
  for(j=0;j<12;j++){
   rbif->BCMASKuse[j]=0; 
   //for(i=0;i<ORBITLENGTH;i++)rbif->BCMASK[i][j]=0;
  }
  //strcpy(rbif->BCMASK,"");
  for(j=0;j<ORBITLENGTH;j++){ rbif->BCMASK[j]= 0; };
  // PF:
  for(int jj=0;jj<NPF;jj++){
     cleanTPastFut(&rbif->pf[jj]);
     rbif->PFuse[jj]=0; 
  }
  //for(j=0;j<5;j++){
   //rbif->PFuse[j]=0; 
    //for(int jj=0;jj<ixMaxpfdefs;jj++){
    //  rbif->pf[j].pfdefs[jj]= 0; 
    //};
  //}; 
  //rbif->PFCuse=0;
  //for(int jj=0;jj<ixMaxpfdefsCommon;jj++) rbif->pfCommon.pfdefsCommon[jj]=0; 
  
 };
 
}
/*-----------------*/
TRBIF *allocTRBIF() {
TRBIF *rc;
rc = (TRBIF *) malloc(sizeof(TRBIF));
if(rc == NULL){
  printf("RBIF2Partition: not enough memory \n");
  return NULL;
};
cleanTRBIF(rc,0);
return(rc);
}
/*------------------------------------------------------printTRBIF()
*/
void printTRBIF(TRBIF *rbif){
int ix;
if(rbif==NULL){
 printf("printTRBIF: rbif==NULL\n");
 return;
}
printf("RBIFuse: ");
for(ix=0; ix<ixrbifdim; ix++) printf(" %d", rbif->rbifuse[ix]);
printf("\n");
printf("  RBIF "); 
printf("rnd1:0x%x rnd2:0x%x ",rbif->rbif[ixrnd1],rbif->rbif[ixrnd2]);
printf("bc1:0x%x bc2:0x%x ",rbif->rbif[ixbc1],rbif->rbif[ixbc2]);
if(l0C0()>=0xc606) {
  printf("l0f1:0x%x l0f2:0x%x l0f3:0x%x l0f4:0x%x\n",
    rbif->rbif[ixl0fun1],rbif->rbif[ixl0fun2],
    rbif->rbif[ixl0fun3],rbif->rbif[ixl0fun4]
  );
  for(ix=ixl0fun1; ix<ixintfun1; ix++) {
    printf("lut%d:%s\n", ix-ixl0fun1+1, &rbif->lut8[(ix-ixl0fun1)*LUT8_LEN]);
  };
} else {
  printf("l0f1:0x%x l0f2:0x%x ",rbif->rbif[ixl0fun1],rbif->rbif[ixl0fun2]);
  //printf("  intf1,2,t:0x%x 0x%x 0x%x\n",rbif->rbif[ixintfun1], rbif->rbif[ixintfun2], rbif->rbif[ixintfunt]);
};
for(ix=ixl0fun1; ix<=ixintfunt; ix++) {
  char t12; char l0int[4];
  if(ix<ixintfun1) { 
    strcpy(l0int,"l0f");
    t12='1' + ix - ixl0fun1;
  } else {  strcpy(l0int,"int"); };
  if( ix==ixintfun1 ) { t12='1';};
  if( ix==ixintfun2) { t12='2';};
  if( ix==ixintfunt) { t12='t';};
  //printf("RBIF:%d %s\n",ix,l0int);
  if(rbif->rbifuse[ix]!=notused){
    printf("  %s%c rbifuse:%d %s:\n", l0int, t12,
      rbif->rbifuse[ix], &rbif->l0intfs[(ix-ixl0fun1)*L0INTFSMAX]);
  }; 
};printf("\n");
/* not used: printTRBIF_34(rbif, ixlut3132); printTRBIF_34(rbif, ixlut4142);*/
//printf("INTSEL sel,allrare:0x%x 0x%x \n",rbif->intsel,rbif->rare);
//printf("INTSEL sel:0x%x \n",rbif->intsel);
//printf("BCMASKs len=%i:\n%s\n",strlen(rbif->BCMASK),rbif->BCMASK);
printf("  BCMASKS+PF use:");
for(ix=0;ix<12;ix++)printf("%2i",rbif->BCMASKuse[ix]);
printf("  PF:");
for(ix=0;ix<NPF;ix++)printf("%2i",rbif->PFuse[ix]);
printf("\n");
for(ix=0;ix<NPF;ix++){printf("%i ",ix); printTPastFut(&rbif->pf[ix]);}
//printf("  PFC:%2i",rbif->PFCuse);
printf("\n");
}
/*------------------------------------------------------copyTRBIF()
*/
void copyTRBIF(TRBIF *dst, TRBIF *src) {
int jx;
for(jx=0; jx<ixrbifdim; jx++) {
  dst->rbif[jx]= src->rbif[jx];
  dst->rbifuse[jx]= src->rbifuse[jx];
};
dst->intsel= src->intsel;
//dst->rare= src->rare;
for(jx=0; jx<L0FINTN*L0INTFSMAX; jx++) {    // l0f1/2/3/4 int* symb. definitions
  dst->l0intfs[jx]= src->l0intfs[jx];
};
for(jx=0; jx<8*LUT8_LEN; jx++) {    // l0f1/2/3/4 definitions
  dst->lut8[jx]= src->lut8[jx];
};
//strcpy(dst->BCMASK,src->BCMASK);
for(jx=0;jx<ORBITLENGTH;jx++){
  dst->BCMASK[jx]= src->BCMASK[jx];
};
for(jx=0;jx<12;jx++){
  dst->BCMASKuse[jx]= src->BCMASKuse[jx];
};
for(jx=0;jx<NPF;jx++){
  dst->PFuse[jx]= src->PFuse[jx];
  copyTPastFut(&dst->pf[jx],&src->pf[jx]);
}; 
//dst->PFCuse= src->PFCuse;
//if(dst->PFCuse>0) {
//    copyPFC(&dst->pfCommon, &src->pfCommon);
//};
}
/*---------------------------------------------------------cleanTFO()
*/
void cleanTFO(TFO *fo){
 if(fo != NULL){
  fo->cluster=0;
  fo->test_cluster=0;
 }
}
/*---------------------------------------------------------copyTFO()
*/
void copyTFO(TFO *to,TFO *from){
 to->cluster=from->cluster;
 to->test_cluster=from->test_cluster;
}
/*---------------------------------------------------------cleanTINPSCTP
 */
void cleanTINPSCTP(TINPSCTP *inpsctp){
 if(inpsctp){
   inpsctp->rnd1enabled1=0;
   inpsctp->rnd1enabled2=0;
 }else{
   printf("Warning: cleanTINPSCTP: attempt to clean nonexisitng strucuture. \n");
 }
}
void copyTINPSCTP(TINPSCTP *to,TINPSCTP *from)
{
  if(from && to){
    to->rnd1enabled1=from->rnd1enabled1;
    to->rnd1enabled2=from->rnd1enabled2;
  }else{
   printf("Warning: copyTINPSCTP: one of structures does not exists %p %p\n",from,to);
  }
}
void printTINPSCTP(TINPSCTP *t)
{
 if(t){
    printf("TINPSCTP: 0x%x 0x%x \n",t->rnd1enabled1,t->rnd1enabled2);
 }else{
    printf("TINPSCTP: does not exist. \n");
 }
}
/*---------------------------------------------------------cleanTBUSY()
*/
void cleanTBUSY(TBUSY *busy){
 int i;
 for(i=0;i<NCLUST+1;i++)busy->set_cluster[i]=0;
}
/*-------------------------------------------------------copyTBUSY()
*/
void copyTBUSY(TBUSY *to,TBUSY *from){
 int i;
 for(i=0;i<NCLUST+1;i++)to->set_cluster[i]=from->set_cluster[i];
}
/*-------------------------------------------------------cleanTPastFut()
*/
void cleanTPastFut(TPastFut *pf){
 if(pf == NULL) return ;
  //for(int i=0;i<ixMaxpfdefs;i++)pf->pfdefs[i]=0;
 pf->bcmask=0;
 pf->inter=0;
 pf->PeriodBefore=0;
 pf->PeriodAfter=0;
 pf->NintBefore=0;
 pf->NintAfter=0;
 pf->OffBefore=0;
 pf->OffAfter=0;
 for(int i=0;i<8;i++)pf->lmpf[i]=0;
 for(int i=0;i<4;i++){
   pf->l0pf[i]=0;
 }
 strcpy(pf->name,"");
}
void printTPastFut(TPastFut *pf)
{
 printf("TPastFut: %s\n",pf->name);
 printf("bcmask: 0x%x\n",pf->bcmask);
 printf("inter: %i\n",pf->inter);
 printf("PeriodBefore: %i PeriodAfter %i NintBefore %i NintAfter %i ",pf->PeriodBefore,pf->PeriodAfter,pf->NintBefore,pf->NintAfter);
 printf("OffBefore %i OffAfter %i \n",pf->OffBefore,pf->OffAfter);
 printf("lm circuits usage: ");
 for(int i=0;i<8;i++)printf("%i ",pf->lmpf[i]);
 printf("\n"); 
 printf("l0 circuits usage: ");
 for(int i=0;i<4;i++)printf("%i ipf use: %i ",i,pf->l0pf[i]);
 printf("\n");
}
/*-------------------------------------------------------copyTPastFut()
*/
void copyTPastFut(TPastFut *to,TPastFut *fr)
{
 to->bcmask=fr->bcmask;
 to->inter=fr->inter;
 to->PeriodBefore=fr->PeriodBefore;
 to->PeriodAfter=fr->PeriodAfter;
 to->NintBefore=fr->NintBefore;
 to->NintAfter=fr->NintAfter;
 to->OffBefore=fr->OffBefore;
 to->OffAfter=fr->OffAfter;
 strcpy(to->name,fr->name);
 for(int i=0;i<8;i++)to->lmpf[i]=fr->lmpf[i];
 for(int i=0;i<4;i++){
    to->l0pf[i]=fr->l0pf[i];
 }
}
/*------------------------------------------------------copyTKlas()
*/
void copyTKlas(TKlas *toklas,TKlas *fromklas){
 toklas->l0inputs=fromklas->l0inputs;
 toklas->l0inverted=fromklas->l0inverted;
 toklas->l0vetos=fromklas->l0vetos;
 toklas->scaler=fromklas->scaler;
 toklas->l1definition=fromklas->l1definition;
 toklas->l1inverted=fromklas->l1inverted;
 toklas->l2definition=fromklas->l2definition;
 toklas->classgroup=fromklas->classgroup;
 toklas->hwclass=fromklas->hwclass;
 toklas->partname=fromklas->partname;
 toklas->lmcondition= fromklas->lmcondition;
 toklas->lminverted= fromklas->lminverted;
 toklas->lmvetos= fromklas->lmvetos;
 toklas->lmscaler= fromklas->lmscaler;
 toklas->sdg= fromklas->sdg;
 toklas->pf = fromklas->pf;
}
/*---------------------------------------------------- findHWCluster()
Input: part, pcluster:1-6.
return: hw cluster (1-6)
        0: intError (should be found, but was not)
*/
w32 findHWCluster(Tpartition *part, w32 pcluster) {
w32 icl,icluster;
for(icl=0; icl<NCLUST; icl++) {
  if(part->ClusterTable[icl]==pcluster) {
    icluster= icl+1; return(icluster);
  };
};
if((part->hwallocated&0x1)==0x1) {// this part. has already its clus allocated
  char emsg[ERRMSGL];
  sprintf(emsg, "findHWCluster: pcluster %d not found", pcluster);
  intError(emsg);
  printf("ClusterTable (cluster numbers used in .pcfg file):");
  for(icl=0; icl<NCLUST; icl++) {
    printf(" %d",part->ClusterTable[icl]);
  }; printf("\n");
} else {
  char emsg[ERRMSGL];
  sprintf(emsg, "findHWCluster:partname:%s hwallocated:0x%x pcluster:%d", 
    part->name, part->hwallocated, pcluster);
  /* this should not be error: ClusterTable is filled ONLY
     in clusterPart2HW() AFTER the findHWCluster() call !
     Strange : the problem (Error:) seems to started
  from 13.11.2009. I.e. it seems this part of the code was note
  reached before.
  prtError(emsg); */
  prtLog(emsg);
};
return(0);
}
/*-----------------------------------------------------------------------
  translate from bit in l0condition to the position in rbif array
called from:
checkRBIFown
modL0input[f14]
checkRES
*/
int l0condition2rbif(int bit,int *ix){
 int ret=0;
 switch(bit){
  case 24: *ix=ixl0fun1; break;
  case 25: *ix=ixl0fun2; break;
  case 26: *ix=ixl0fun3; break;
  case 27: *ix=ixl0fun4; break;
  case 28: *ix=ixrnd1; break;
  case 29: *ix=ixrnd2; break;
  case 30: *ix=ixbc1; break;
  case 31: *ix=ixbc2; break;
  default: printf("Internal error:l0condition2rbif: wrong bit 0x%x\n",bit);
           ret=1; break;
 };
return ret;
}
/*-----------------------------------------------------------------------
  Modifies pair of l0inp resources. 
  bit= 24,26,28 or 30  (also for AC L0 firmware)
  Resources in part->rbif : symbolic (partition) names
               part->rbifuse: hardware allocation of symbolic (partition) res
               HW->rbif :   hardware resources
               Klas shoud have HW.rbif[part->rbifuse[ix]] resource for ix 
*/
int modL0input(w32 *l0inp,Tpartition *part,int bit){
 TRBIF *rbif;
 w32 conf; int ix;
 if(l0condition2rbif(bit,&ix)) return 1;
 rbif=part->rbif;
 conf=(*l0inp & (0x3<<bit)) >> bit;
 //if(DBGcumRBIF)printf("modL0input-------%s--------->%i %i\n",part->name,conf,bit);
 switch(conf){
  case 3:  // nothing selected
       //to do : check consistency with rbifuse
       break;;
  case 1:  // bc2 rnd2 or l0fun2 selected (0:active)
       ix=ix+1;
       if(rbif->rbifuse[ix] == (w32)ix) return 0;  // same position
       *l0inp=(*l0inp & (~(0x3<<bit))) + (1<<(bit+1));      // swap
       break;;
  case 2:  // bc1,rnd1 l0fun1 selected
       if(rbif->rbifuse[ix] == (w32)ix) return 0;  // same position
       *l0inp=(*l0inp & (~(0x3<<bit))) + (1<<(bit));      // swap
       break;
  case 0: // both are sleceted, nothing to do
          // freq are swaped in part and hardware but does not matter
       break;
 }
 return 0; 
}
/* bit: 24 -i.e. called for fy >=c606 when l0f1..4 available

 * */
int modL0inputf14(w32 *l0inp,Tpartition *part,int bit){
 TRBIF *rbif;
 w32 conf, pconf; int ix,nf;
 if(l0condition2rbif(bit,&ix)) return 1;
 rbif=part->rbif;
 conf=(*l0inp & (0xf<<bit)) >> bit;   // class bits: 0: selected
pconf=0;
for(nf=ixl0fun1; nf<=ixl0fun4; nf++) {
  if(rbif->rbifuse[nf] != notused) pconf= pconf | ( 1<<(nf-ixl0fun1));
}; 
pconf= ~pconf; // no used: -> 0
// pconf: rbif 
 //if(DBGcumRBIF)printf("modL0input-------%s--------->%i %i\n",part->name,conf,bit);
if(conf==0) {
  // all selected, nothing to do, really?
  // freq are swaped in part and hardware but does not matter
  ;
} else if(conf==0xf) {
  // nothing selected
  //to do : check consistency with rbifuse
  ;
} else {
  int ix; w32 hwconf;
  /* conf: l0f4..1 0:active -to be reshuffled according to allocation in hw
     hwconf: new, considering reallocation
  */
  hwconf=0xf;   // l0f* unused
  for(ix=0; ix<4; ix++) {
    w32 realix;
    realix= rbif->rbifuse[ix+ixl0fun1];
    if((conf & (1<<ix)) == 0) {  //selected
      if(realix == (w32)(ix+ixl0fun1)) { ;  // same position
      } else {   // 0-> realix bit
        ;
      };
      hwconf= hwconf & (~( 1<<(realix-ixl0fun1)));
    } else {   // not selected
      ; //leave 1 (unused, not selected)
    }
  };
  printf("modL0inputf14: l0f bits in l0cond: 0x%x -> 0x%x rbif:0x%x\n", conf, hwconf, pconf);
  *l0inp=(*l0inp & (~(0xf<<bit))) | (hwconf<<bit);
};
return 0; 
}
/*-----------------------------------------------------------------------
Needed for clusters and rbif update:
- cluster number is in hw 3times (l0vet, l1def l2def)
- 
Called: only from addClasses2HW() 2 times:
        - when looping over old partitions
        - when looping over new partitions
*/
void copymodTKlas(TKlas *toklas,TKlas *fromklas, Tpartition *part){
w32 l0veto,l1def,l2def,l0inp;
w32 pcluster,hwclust;
copyTKlas(toklas,fromklas);
l0veto= toklas->l0vetos;
l0inp = toklas->l0inputs;
l1def = toklas->l1definition;
l2def = toklas->l2definition;
//clusters
pcluster = (l0veto) & 0x7;   //partition cluster name
hwclust= findHWCluster(part, pcluster);
//printf("b l0veto=0x%x l1def= 0x%x l2def=0x%x\n",l0veto,l1def,l2def);
l0veto= (l0veto & 0xfffffff8) + hwclust;
l1def=  (l1def & 0x8fffffff) + (hwclust<<28);
l2def=  (l2def & 0x8fffffff) + (hwclust<<28);
// RBIF
modL0input(&l0inp,part,28);  //rnd1
modL0input(&l0inp,part,30);  //bc1
if(l0C0()<=0xc605) {
  modL0input(&l0inp,part,24);  //l0f1..2
} else {
  modL0inputf14(&l0inp,part,24);  //l0f1..4
};
toklas->l0inputs= l0inp;
toklas->l0vetos= l0veto;      
toklas->l1definition= l1def;      
toklas->l2definition= l2def;     
}
/*---------------------------------------- getCLAMASK() */
w32 getCLAMASK() {
if(l0C0()) { 
  //return(0x800000);
  //return(0x800000);
  //return(0x9ffff0); 
  return(0xf9ffff0); //PF
} else {return(0x80000000);};
}
/*----------------------------------------------------------cleanTKlas()
*/
void cleanTKlas(TKlas *klas){
if(klas != NULL){
  klas->l0inputs=0xffffffff;   //0;
  klas->l0inverted=0;
  klas->lmcondition=0xffffffff;
  klas->lminverted=0;
  klas->l0vetos=getCLAMASK();// disable klas by default
  //klas->lmvetos= 0x803f00; //class mask, PF1..4, all, LM deadtime ONLY 0xffffff valid!
  klas->lmvetos= 0x83ff00; //class mask, PF1..8
  klas->scaler=0;
  klas->lmscaler=0;
  klas->l1definition=0x8fffffff; //0;
  klas->l1inverted=0;
  klas->l2definition=0x0f000fff;  //0;
  klas->hwclass=0;
  klas->classgroup=0;   // always IN
  klas->partname=NULL;
  klas->sdg=-1;
  klas->pf=0xf;
};
}

/*-----------------------------------------------------printTKlas()
*/
void printTKlas(TKlas *klas,int i){
  printf("CLA.%2i 0x%x ",i,klas->l0inputs);
  printf("0x%x 0x%x ",klas->l0inverted,klas->l0vetos);
  printf("0x%x 0x%x ",klas->scaler,klas->l1definition);
  printf("0x%x 0x%x hwcl:%d ",klas->l1inverted,
    klas->l2definition, klas->hwclass);
  printf("cg:%d 0x%x 0x%x 0x%x 0x%x %i pf:0x%x %s\n",klas->classgroup,
    klas->lmcondition, klas->lminverted, klas->lmvetos, klas->lmscaler, klas->sdg,klas->pf,klas->partname);
}
/*------------------------------------------------------checkCluV0TKlas()
*/
int checkCluV0TKlas(TKlas *klas){
 int ret=0;
 w32 l0mask=0x7,l1mask=0x70000000,l2mask=0x70000000;
 if(((klas->l0vetos) & l0mask) == 0){
     printf("checkCluV0TKlas: class has zero cluster at L0\n ");
     ret++; 
  }
 if(((klas->l1definition) & l1mask) == 0){
    printf("checkCluV0TKlas: class has zero cluster at L1\n ");
    ret++; 
 }
 if(((klas->l2definition) & l2mask) == 0){
     printf("checkCluV0TKlas: class has zero cluster at L2\n ");
     ret++; 
 }
 return ret;
}
/*-------------------------------------------------------checkCluVL012TKlas()
*/
int checkCluVL012TKlas(TKlas *klas){
  w32 l0mask=0x7,l1mask=0x70000000,l2mask=0x70000000;
  w32 l0,l1,l2;
  l0=klas->l0vetos & l0mask;
  l1=(klas->l1definition & l1mask)>>28;
  l2=(klas->l2definition & l2mask)>>28;
  if(l0 != l1){
    printf("checkCluVL012TKlas: L0 and L1 clusters different: %i %i \n",l0,l1);
    return 1;
  }
  if(l0 != l2){
    printf("checkCluVL012TKlas: L0 and L2 clusters different: %i %i \n",l0,l2);
    return 1;
  }
  return 0;
}
/*---------------------------------------------------initTpartition()
*/
void initTpartition(Tpartition *part,char *name){
 int i;
 //setnameTpartition(part,name);
 strcpy(part->name, name); 
strcpy(part->partmode, partmode);  // partmode:for rcfg
 part->inpsctp=0;
 for(i=0;i<NCLASS;i++)part->klas[i]=NULL;
 for(i=0;i<NCLUST;i++){
   part->ClusterTable[i]=0;
 }
 for(i=0;i<NDETEC;i++){
   part->Detector2Clust[i]=0;
 }
 part->rbif=NULL;
 part->hwallocated=0;
 part->MaskedDetectors=0;
 part->run_number=0;
 part->positionInAllPartitions=-1;
 part->remseconds= -1;
 for(i=0;i<MAXCLASSGROUPS;i++){
   part->classgroups[i]=0;  // allways active 
   part->totalsecs[i]=0;
   part->totalmics[i]=0;
 };
 //part->classgroups[0]=0; is always 0 
 /* see cfg2part.c ParseFile
 part->classgroups[1]=1;  
 part->classgroups[2]=2; 
 part->classgroups[3]=3;
 part->classgroups[4]=4;
 part->classgroups[5]=5; 
 part->classgroups[6]=6;
 part->classgroups[7]=7; 
 part->classgroups[8]=8; 
 part->classgroups[9]=9; */
 part->active_cg= 0;
 part->cshmpart= NULL;
}
/*--------------------------------------------------deleteTpartition()
 rbif has no pinters to free.
*/
Tpartition *deleteTpartition(Tpartition *part){
 int i,np=0;
 for(i=0;i<NCLASS;i++){
   if(part->klas[i]!= NULL) {
     free(part->klas[i]); part->klas[i]=NULL;
     np++;
   };
 };
 if(part->inpsctp) delete (part->inpsctp);
 if(part->rbif) free(part->rbif);
 SDGclean(part->name);
 //printf("deleteTpartition: not null classes:%d\n",np);
 //free(part->rbif); //free P/F
 //if(part->fixed_cnts!=NULL) free(part->fixed_cnts);
 free(part);
 return NULL;
}
/*----------------------------------------------------setnameTpartition()
int setnameTpartition(Tpartition *part, char *name){
 int len;
 len=strlen(name);
 part->name = (char *) malloc(len*sizeof(char));
 if(part->name == NULL) return 1;
 strcpy(part->name,name);
 return 0;
}
*/
/*-----------------------------------------------------printTpartition()
*/
void printTpartition(char *headtxt, Tpartition *part){
int i;
char clgroups[400];
if(part == NULL){
  printf("printTpartition: partition does not exist (NULL)\n");
  return;
}
if(part->name == NULL)
  printf("printTpartition name=NULL RunNum=%i\n",part->run_number);
else printf("%s ------ %s --- part name: %s RunNum:%i hwallocated:%d\n",
  part->name,headtxt, part->name, part->run_number, part->hwallocated);
printTINPSCTP(part->inpsctp);
printTRBIF(part->rbif);
strcpy(clgroups, "");
printf("classgroups:%d:%s\n",part->nclassgroups, clgroups);
for(i=0;i<MAXCLASSGROUPS;i++){
  if(part->classgroups[i]!=0) {
    //sprintf(clgroups," %s %d:%d s", clgroups, i, part->classgroups[i]);
    printf("%d:%d \n", i,part->classgroups[i]);
  };  // active only given time 
}; //printf("class groups:%s\n", clgroups);
printf("CLASSES:\n");
for(i=0;i<NCLASS;i++){
  TKlas *klas;
  if((klas=part->klas[i])) {
    printTKlas(klas,i);
  } else {
    continue;
  };
};
 printf("ClusterTable:");
 for(i=0;i<NCLUST;i++)printf("0x%x ",part->ClusterTable[i]);
 printf("\n");
 printf("CLUSTERS:");
 for(i=0;i<NCLUST;i++){
  if(part->ClusterTable[i])
  printf(" HW%i->P%i ",i,part->ClusterTable[i]);
 }
 printf("\n");
 printf("Detectors:");
 /*for(i=0;i<NDETEC;i++) {    ? von
   int dn, bn;
   dn= part->Detector2Clust[i]; bn= validLTUs[dn].busyinp;
   printf("0x%x:%d ", dn, bn);
 }; printf("\n");*/
printDetector2Clust(part->Detector2Clust);
printf("Masked detectors: 0x%x \n",part->MaskedDetectors);
/* print classes, including fed detectors:
for(i=0;i<NCLASS;i++){
  TKlas *klas; int cluster, clustermask, ixdet;char txdets[100];
  if((klas=part->klas[i])) {
    printTKlas(klas,i);
  } else {
    continue;
  };
  //else break;   // classes are always allocated 1,2,3,...
  // the above is not when mask is applied !!!
  cluster= klas->l0vetos & 0x7;
  clustermask= 1<<(cluster-1);
  txdets[0]='\0';
  printf("dbg_printTpar: clu:%d mask:%d...\n", cluster, clustermask);
  for(ixdet=0; ixdet<NDETEC; ixdet++) {
    int clsts;
    //printDetsInCluster(part, cluster);
    clsts= part->Detector2Clust[ixdet];   // log. clusters ixdet is in
    if(clsts & clustermask) {
      sprintf(txdets, "%s %d", txdets, ixdet);
    };
  };
  printf("    fed dets: %s\n", txdets);
};*/
if(part->name != NULL){
printf("---------> End of printing: part name: %s RunNum:%i\n",part->name,part->run_number);
}
}
/*------------------------------------------------------ getIDl0f
Goal: find
   - L0 ctp inputs (1..24) defining l0f
   - whether l0f is 'pure LM' function (i.e. ONLY LM inputs in its definition)
I: l0fn: 0..3  l0f number
   rbifs: partition's (in case when called from checkmodLM) or HW's rbif
rc: trigger detectors pattern. -1 in case of error (l0f not allowed input )
    l0finputs: is updated by l0 inputs used in this function
               e.g.: 0xc: when L0 ctp inputs 3,4 are used in l0f definition
    pure lm: 1: yes (only LM inputs)  0:no (also non-LM inputs used) 
*/
//int getIDl0f(Tpartition *part, int l0fn, w32 *l0finputs, int *purelm) {
int getIDl0f(TRBIF *rbifs, int l0fn, w32 *l0finputs, int *purelm) {
char *l0ftxt; int maxinpallowed,ixn, rcinpdets=0;
char currone[24];
char emsg[400];
//char emsg2[400];
*purelm=1;
if(l0C0()>=0xc606) {
  maxinpallowed=8;
} else {
  maxinpallowed=4;
};
sprintf(emsg, "getIDl0f:\n");
for(ixn=0; ixn<maxinpallowed/2; ixn++) {
  currone[0]='\0';
  if(ixn==l0fn) {
    strcpy(currone, " <---");
  };
  sprintf(emsg,"%sl0f%d:%s%s\n", emsg,
    ixn+1, &rbifs->l0intfs[ixn*L0INTFSMAX], currone);
}; prtLog(emsg);
//l0ftxt= &(part->rbif->l0intfs[l0fn*L0INTFSMAX]);
l0ftxt= &rbifs->l0intfs[l0fn*L0INTFSMAX];
emsg[0]='\0'; ixn=0;
while(1) {
  int rc,vcip; char name[30];
  rc= getNextFunName(l0ftxt+ixn, name);
  if(rc==-1) { break; }; // no meaningfull name found
  //ixn: points just after name in line string (' ', '\0', ':', '\n')
  //name: contains next name.
  if(DBGgetInputDets) printf("getIDl0f:name:%s\n", name);
  vcip= findInputName(name);
  if(vcip >=0) {
    if(validCTPINPUTs[vcip].level==0) {
      int inn;
      inn= validCTPINPUTs[vcip].inputnum;
      if(inn>maxinpallowed) {
        sprintf(emsg,"getIDl0f:l0Fun uses %s not connected to L0/1..%d", name, maxinpallowed);
      } else {
        int inpdet;
        if(validCTPINPUTs[vcip].lminputnum<=0) *purelm=0;
        *l0finputs= *l0finputs | (1<<(inn-1));
        inpdet= validCTPINPUTs[vcip].detector;
        rcinpdets= rcinpdets | (1<<inpdet);
        if(DBGgetInputDets) 
          printf("getIDl0f:inn:%d inpdet:%d\n", inn, inpdet);
      };
    } else {
      sprintf(emsg,"getIDl0f:l0Fun %s used but not L0 input", name);
    };
  } else {
    sprintf(emsg,"getIDl0f: L0 input %s used in l0f not found", name);
  };
  if(emsg[0]!='\0') {
    infolog_trgboth(LOG_FATAL, emsg); rcinpdets=-1;
    break;
  };
  ixn=rc+ixn;
}; 
return(rcinpdets);
}
/*-------------------------------------------------- checkmodLMPF */
#define LMPFMASK 0x3fc00
#define L0L0PFMASK 0xf0
#define L0LMPFMASK 0xf000000
int checkmodLMPF(Tpartition *part){
 //TRBIF* rbif=HW.rbif;
 TRBIF* rbif=part->rbif;
 for(int icla=0;icla<NCLASS;icla++){
  TKlas *klas; int cluster, clustermask, ixdet;char txdets[100];
  if((klas=part->klas[icla])) {
    printTKlas(klas,icla);
  } else {
    continue;
  };
  // PF class has to have l0 veto != 0xf
  w32 pfmask=(klas->l0vetos&0xf0)>>4; 
  if(pfmask == 0xf) continue;
  //printf("checkmodLMPF: PF class found %i pf=%s \n",icla,klas->pfname);
  printf("checkmodLMPF: PF class found %i pf=%x \n",icla,pfmask);
  // Find PF in partition
  w32 pfs[NPF];
  for(int ipf=0;ipf<NPF;ipf++){
   pfs[ipf]=0;
   bool pfipf=(1<<ipf)&(~pfmask);
   if(pfipf){
     pfs[ipf]=1;
     if(rbif->PFuse[ipf]==0){ 
       printf("checkmodLMPF: internal error PF ipf=%i pfmask=0x%x not found in HW \n",ipf,pfmask);
       return 1;
     }
   }
  }
  // check if TRD class
  cluster= klas->l0vetos & 0x7;
  clustermask= 1<<(cluster-1);
  txdets[0]='\0';
  ixdet= 4 ; 
  int clsts= part->Detector2Clust[ixdet];   // log. clusters ixdet is in
  printf("checkmodLMPF: cluster:%d  clsts:0x%x\n", cluster, clsts);
  if(clsts & clustermask) {
    //LM classes
    printf("checkmodLMPF:LM class %i: lmveto before 0x%x l0veto before 0x%x\n",icla,klas->lmvetos,klas->l0vetos);
    // lmvetoes 3rd pf out of 3
    w32 lmv=klas->lmvetos;
    w32 mask=0;
    for(int ipf=0;ipf<NPF;ipf++)if(pfs[ipf])mask+=1<<(ipf+4+10);  //ok
    lmv=lmv&(~mask);
    klas->lmvetos=lmv;
    // l0vetoes 1st pf out of 3
    w32 l0v=klas->l0vetos;
    l0v=l0v|0xf0;
    mask=0;
    for(int ipf=0;ipf<NPF;ipf++)if(pfs[ipf])mask+=1<<(ipf+24);   
    l0v=l0v&(~mask);
    klas->l0vetos=l0v; 
    printf("checkmodLMPF:LM class %i: lmveto after 0x%x l0veto after 0x%x\n",icla,klas->lmvetos,klas->l0vetos);
  }else{
    //nonLM classes
    printf("checkmodLMPF:nonLM class %i: lmveto before 0x%x l0veto before 0x%x\n",icla,klas->lmvetos,klas->l0vetos);
    // lmvetoes nothing
    // l0vetoes 1st and 2nd pfs out of 3
    w32 l0v=klas->l0vetos;
    l0v=l0v|0xf0;
    w32 mask=0;
    for(int ipf=0;ipf<NPF;ipf++)if(pfs[ipf])mask+=(1<<(ipf+24)) + (1<<(ipf+4));     //ok
    l0v=l0v&(~mask);
    klas->l0vetos=l0v; 
    printf("checkmodLMPF:nonLM class %i: lmveto after 0x%x l0veto after 0x%x\n",icla,klas->lmvetos,klas->l0vetos);
  }
 }
 return 0;
}
/*-------------------------------------------------- checkmodLM */
int checkmodLM(Tpartition *part){
#define L0RNDBCMASK 0xf0000000
#define L0FUNSMASK  0x0f000000
#define LMRNDBCMASK 0x000f0000
#define LMFUNSMASK  0x0000f000
int icla, retcode=0;
printf("checkmodLM0: part. hwallocated:%d\n", part->hwallocated);
for(icla=0;icla<NCLASS;icla++){
  TKlas *klas; int cluster, clustermask, ixdet;char txdets[100];
  if((klas=part->klas[icla])) {
    printTKlas(klas,icla);
  } else {
    continue;
  };
  //else break;   // classes are always allocated 1,2,3,...
  // the above is not valid after mask is applied !!!
  cluster= klas->l0vetos & 0x7;
  clustermask= 1<<(cluster-1);
  txdets[0]='\0';
  //printf("dbg_printTpar: clu:%d mask:%d...\n", cluster, clustermask);
  //for(ixdet=0; ixdet<NDETEC; ixdet++) {
  ixdet= 4 ; { // TRD ECS#
    int clsts;
    //printDetsInCluster(part, cluster);
    clsts= part->Detector2Clust[ixdet];   // log. clusters ixdet is in
    printf("checkmodLM: cluster:%d  clsts:0x%x\n", cluster, clsts);
    if(clsts & clustermask) {
      w32 Ngens,Nfunsmsk,inpmsk; int ixvci, Nlm, Ngenslm, Nfuns;
      sprintf(txdets, "%s %d", txdets, ixdet);
      // class feeding TRD:
      // by default, disable all LMFUN* LMRND/BC:
      printf("checkmodLM1:clas:%d l0inputs:0x%x lmcondition:0x%x\n", icla+1, klas->l0inputs,
        klas->lmcondition);
      klas->lmcondition= klas->lmcondition | (LMRNDBCMASK | LMFUNSMASK);
      printf("checkmodLM2:clas:%d l0inputs:0x%x lmcondition:0x%x\n", icla+1, klas->l0inputs,
        klas->lmcondition);
      Ngenslm=0;
      Ngens= (~(klas->l0inputs & L0RNDBCMASK))>>28;   // e.g: 0xf
      if(Ngens) {   //===================  RND/BC used in this class
        // use LM copy of RND/BC (i.e. effectively allow only 4 generators not 8):
        // instead of L0 generators:
        klas->lmcondition= klas->lmcondition & (~(Ngens<<16));  // use at LM
        klas->l0inputs= klas->l0inputs | (Ngens<<28);   //do not use at L0
        Ngenslm= Ngens;
      };
      printf("checkmodLM3:clas:%d l0inputs:0x%x lmcondition:0x%x\n", icla+1, klas->l0inputs,
        klas->lmcondition);
      // check LM functions
      // pure LM-function: use it only on LM level (i.e. remove from L0 for TRD classes)
      // else: use it ONLY at L0 level
      //
      Nfuns=0; Nfunsmsk= (~(klas->l0inputs & L0FUNSMASK))>>24;   // e.g: 0xf
      if(Nfunsmsk) {   //===================  LMFUN used in this class
        /*von  use LM copy of L0F1/2
        klas->lmcondition= klas->lmcondition & (~(0xf<<12));  // use at LM, first 0s
        klas->lmcondition= klas->lmcondition | (~(Nfuns<<12));  // use at LM
        klas->l0inputs= klas->l0inputs | ((Nfuns)<<24);   //do not use at L0
        Nfunslm= Nfuns; */
        int ixl0fn,purelm;
        klas->lmcondition= klas->lmcondition | (0xf<<12);  // init to 'not used'
        for(ixl0fn=0; ixl0fn<4; ixl0fn++) {
          w32 msk0, l0finputs; int inpdets;
          msk0= 1<<ixl0fn;
          if((msk0 & Nfunsmsk)==0) continue;
          Nfuns++;
          inpdets= getIDl0f(part->rbif, ixl0fn, &l0finputs, &purelm);
          if(inpdets<0) { retcode=1; break;};   // internal error (no det found for used input)
          if(purelm==1) {
            klas->lmcondition= klas->lmcondition & (~(msk0<<12));  // use at LM
            klas->l0inputs= klas->l0inputs | (msk0<<24);   //do not use at L0
          } else {
            ; // leave it at L0-level
          };
        };
      };
      printf("checkmodLM3a:clas:%d l0inputs:0x%x lmcondition:0x%x\n", icla+1, klas->l0inputs,
        klas->lmcondition);
      //================================= check if at least 1 LM input!
      Nlm=0;
      for(ixvci=0; ixvci<NCTPINPUTS; ixvci++) {
        w32 actinps; int lminpn;
        actinps= ~(klas->l0inputs & 0x00ffffff);   // 1: active input, 0: not used
        if(validCTPINPUTs[ixvci].level!=0) continue;
        if(validCTPINPUTs[ixvci].inputnum>24) {
          printf("ERROR (bad validCTPINPUTs) in checkloadLM: %d %s \n",
            ixvci, validCTPINPUTs[ixvci].name);
          retcode= 1; break;
        };
        lminpn= validCTPINPUTs[ixvci].lminputnum;
        if((validCTPINPUTs[ixvci].switchn!=0) &&
           (validCTPINPUTs[ixvci].inputnum!=0) && 
           (lminpn>0)) {    // LM input
          w32 inpmsk;
          printf("checkmodLM3b: LM swin:%d inputnum:%d lminputnum:%d\n",
            validCTPINPUTs[ixvci].switchn, validCTPINPUTs[ixvci].inputnum,
            validCTPINPUTs[ixvci].lminputnum);
          inpmsk= 1<<(validCTPINPUTs[ixvci].inputnum-1);
          if(inpmsk & actinps) {
            // L0level: do not use it
            // LMlevel: set lmcondition considering LM switch
            printf("checkmodLM3c:inp:%d l0inputs:0x%x lmcondition:0x%x\n", validCTPINPUTs[ixvci].inputnum-1, 
              klas->l0inputs, klas->lmcondition);
            Nlm++;
            klas->l0inputs= klas->l0inputs | inpmsk;
            klas->lmcondition= klas->lmcondition & (~(1<<(lminpn-1)));
          };
        };
      };
      //================== downscaling
      // Downscaling all set at LM level
      // nothing to do: lmscalers are asigned at cfg2part and loaded in load2HW
      //klas->lmscaler= klas->scaler; klas->scaler=0;
      // also switch off lmgroup at lm level - not necessary because scalers=0 ?
      printf("checkmodLM4:clas:%d Nlm/Ngens/Nfuns:%d/%d/%d l0inputs:0x%x lmcondition:0x%x l0/lm scaling:0x%x 0x%x\n", 
        icla+1, Nlm, Ngenslm, Nfuns, klas->l0inputs, klas->lmcondition, klas->lmscaler, klas->scaler);
      if((Nlm==0) && (Ngenslm==0) && (Nfuns==0) ) {
        char msg[200];
        sprintf(msg, "no LM input for TRD (i.e. 40mhz at LM level) in logical class (i.e. .pcfg CLA.%d)",icla+1);
        //infolog_trgboth(LOG_INFO, msg);  goesonly toctpproxy log from 29.5.2017
        printf("INFO %s\n",msg);
      };
      // copy L0-allrare flag (bit20) to LM_VETO (bit9):
      inpmsk= ((klas->l0vetos & 0x100000) >> 20) << 9;
      klas->lmvetos= (klas->lmvetos & (~0x200)) | inpmsk;
      // set LM_DEADTIME for LM class:
      klas->lmvetos= klas->lmvetos & (~0x800100);  // LM clmask+deadtime
      // LM-L0 BUSY flag for LM class to 1  (init to 0 (for L0-classes) in cleanTKlas):
      klas->l0vetos= klas->l0vetos | 0x200000;  // LM-L0 BUSY
      strcpy(TRD_TECH, part->name);  // see ctp_StopPartition
    };
  };
  //printf("checkmodLM5: ECS numbers of fed dets: %s\n", txdets);
};
return(retcode);
}
/*-------------------------------------------------------------- checmodLM TECHNICAL (old)
Check if there are classes feeding TRD. Do these modifications for them:
- check rnd1 bit in L0_CONDITION word for this class
  if ON and 'downscaling 0' and 'no other input definition in this class':
    - remove RND1 from L0_CONDITION AND ADD l0inp22
    - connect swinp32/11 to RND1 generator
  else
    nothing, but make sure RND1 connection to swinp32/11 is disabled at EOR

Consequence: it is nonsense to start TRD cluster with
more classes mixing rnd1 usage.
*/
int checkmodLMold(Tpartition *part){
#define RND1MASK 0x10000000
//#define SWIN32MSK (1<<(32-25))
#define SWIN11MSK (1<<(11-1))
int i, retcode=0;
for(i=0;i<NCLASS;i++){
  TKlas *klas; int cluster, clustermask, ixdet;char txdets[100]; char msg[200];
  if((klas=part->klas[i])) {
    printTKlas(klas,i);
  } else {
    continue;
  };
  //else break;   // classes are always allocated 1,2,3,...
  // the above is not when mask is applied !!!
  cluster= klas->l0vetos & 0x7;
  clustermask= 1<<(cluster-1);
  txdets[0]='\0';
  //printf("dbg_printTpar: clu:%d mask:%d...\n", cluster, clustermask);
  //for(ixdet=0; ixdet<NDETEC; ixdet++) {
  ixdet= 4 ; { // TRD
    int clsts;
    //printDetsInCluster(part, cluster);
    clsts= part->Detector2Clust[ixdet];   // log. clusters ixdet is in
    sprintf(msg, "checkmodLM: cluster:%d  clsts:0x%x", cluster, clsts);
    infolog_trg(LOG_INFO, msg);
    if(clsts & clustermask) {
      sprintf(txdets, "%s %d", txdets, ixdet);
      // class feeding TRD:
      if(((klas->l0inputs & RND1MASK) == 0) &&  // RND1 used in this class
         (klas->scaler==0 ) &&                  // downscaling 0
         (klas->l0inputs & 0xe0ffffff)==0xe0ffffff)  {  // no other input
        w32 rndw1, rndw2, rndw3, ninps; int ixdb, ctpin;
        ninps= klas->l0inputs | RND1MASK;  // do not use it at L0
        /*ninps= ninps & (~0x200000);  // AND USE l0inp22:0HCO (was l0inp5) !
          instead, better is to find where 0HCO is: */
        ixdb= findInputName("0HCO");
        if(ixdb==-1) {
          infolog_trgboth(LOG_FATAL, "0HCO not found for technical with TRD + rnd1.");
          // we need to know the input where to connect rnd1!
          retcode=1;
        } else {
          ctpin= validCTPINPUTs[ixdb].inputnum;
          if(ctpin==0) {
            infolog_trgboth(LOG_FATAL, "0HCO not connected for technical with TRD.");
            // it has to be connected ( to a 1-24 ctp input becasue we need to control a class)
            retcode=1;
          } else if((ctpin<1) || (ctpin>24)) {
            sprintf(msg, "0HCO not found. Internal error: ixdb/ctpin:%d/%d)", ixdb,ctpin);
            infolog_trgboth(LOG_FATAL, msg);
            retcode=1;
          } else {
            ninps= ninps & ( ~( 1<< (ctpin-1)));   // to be connected to RND1
          };
        };
        if(retcode!=0) return(retcode);
        klas->l0inputs= ninps;
        //printf("checkmodLM:%d l0inputs:0x%x RND1_EN_FOR_INPUTS:0x%x 0x%x->0x%x \n", 
        //  i, ninps, rndw1, rndw2, rndw3);
        rndw1= vmer32(RND1_EN_FOR_INPUTS);
        rndw2= vmer32(RND1_EN_FOR_INPUTS+4);
        rndw3= rndw1 | (SWIN11MSK);
        vmew32(RND1_EN_FOR_INPUTS, rndw3);
        printf("checkmodLM:%d ctpin:%d l0inputs:0x%x RND1_EN_FOR_INPUTS:0x%x 0x%x->0x%x 0x%x\n", 
          i, ctpin, ninps, rndw1, rndw2, rndw3, rndw2);
        strcpy(TRD_TECH, part->name);  // see ctp_StopPartition
      };
    };
  };
  printf("    fed dets: %s\n", txdets);
};
return(retcode);
}
/*----------------------------------------------------------- checkRES 
I:
bit,cls:   class+bit to be checked. bit: 24..31 (was 24..29 before AC)
part:  checked partition
rbifnew: new TRBIF structure
return:  0:OK  1: int error
called: only from applyMask to update shared resources for one partition
  after throwing out detectors. 
*/
int checkRES(int bit, Tpartition *part, TKlas *cls, TRBIF *rbifnew) {
int ixres;  // 0.. resource number ixrnd1,2, ixbc1,...,ixl0fun2,..,ixlut4142
int retrc=0;
w32 mask;   //32 bit mask. 1 bit set -according to position of resource in L0_CONDITION
mask=1<<bit;
//printf("checkRES:bit:%d\n", bit); fflush(stdout);
if(l0condition2rbif(bit,&ixres)) return(1);
if((mask & (~cls->l0inputs))) {   // resource used by a class
  rbifnew->rbifuse[ixres]=nothwal;       // allocate ixres resource
  if(part->rbif->rbifuse[ixres] == notused) {
    char emsg[ERRMSGL];
    sprintf(emsg, 
      "checkRES: resource %d used after mask applied, but not before?",
      ixres);
    intError(emsg); retrc=1;
  } else {
    // use value known from 'unmasked' partition
    rbifnew->rbifuse[ixres]= part->rbif->rbifuse[ixres];  //from 15.2.2011 !
    rbifnew->rbif[ixres]= part->rbif->rbif[ixres];
    if((ixres>=ixl0fun1) && (ixres<=ixl0fun4)) { // l0f1234
      unsigned int maxlen;
      char *srcstr, *dststr;
      // copy symbolic definition from: rbif/ixres to: rbifnew/ixres
      if(l0C0() < 0xc606) {
        if(ixres>ixl0fun2) {
          infolog_trgboth(LOG_ERROR, "l0f>2 in checRES() for lm0<0xc606");
          return(1);
        };
      };
      maxlen= L0INTFSMAX;
      srcstr= &part->rbif->l0intfs[(ixres-ixl0fun1)*L0INTFSMAX];
      dststr= &rbifnew->l0intfs[(ixres-ixl0fun1)*L0INTFSMAX];
      printf("checkRES:rbif/use:%d/%d srclen:%d ixres:%d\n", 
        part->rbif->rbif[ixres],
        part->rbif->rbifuse[ixres],
        (int)strlen(srcstr), ixres); fflush(stdout);
      printf("checkRES:str:%s:\n", srcstr); fflush(stdout);
      strncpy(dststr, srcstr, maxlen);   // sym. def
      if( strlen(srcstr)>=maxlen ) {
        char emsg[200]; 
        //rbifnew->l0intfs[(ixres-ixl0fun1)*L0INTFSMAX + L0INTFSMAX-1]='\0';
        dststr[maxlen-1]='\0';
        sprintf(emsg, "checkRES:rbif/use:%d/%d srclen:%d \n", 
        part->rbif->rbif[ixres], part->rbif->rbifuse[ixres],
        (int)strlen(srcstr));
        infolog_trgboth(LOG_ERROR, emsg);
      };
      maxlen= L0INTFSMAX;
      srcstr= &part->rbif->lut8[(ixres-ixl0fun1)*LUT8_LEN];
      dststr= &rbifnew->lut8[(ixres-ixl0fun1)*LUT8_LEN];
      strcpy(dststr, srcstr);
    };
    //  &part->rbif->l0intfs[(ixres-ixl0fun1)*L0INTFSMAX]);
  };
}; return(retrc);
}

/*----------------------------------------------int applyMask();
apply mask to partition.
strdetmask: string. "0xhexa" or "decimal" or ""-> 0
Operation:
if strdetmask =="": only create mask according to partition detectors
- create local clu2det table from Detector2Clust
- fill in: part->MaskedDetectors=detmask;
- go through Detector2Clust, removing masked detectors from
  both (Detector2Clust and clu2det)
  possible error: attempt to mask detector not belonging to this partition
  if some cluster becomes empty:
    - release classes feeding this cluster
- release shared resources:
  - for CLASS in all classes:
    - check resources used by CLASS
- if all clusters empty: return 1

rc: 0: ok, mask applied
    1: applying mask led to empty partition
    >1: error (part. cannot be loaded)
    WARNING message if cluster became empty
    ERROR message in some cases
*/
int applyMask(Tpartition *part, char *strdetmask) {
int id, iclu, icla, nclu, retrc=0, isr, bcmscopied=0;
int upperbit, bcmfrom, bcmto;
w32 detmask, realmask=0;
TRBIF rbifnew;         // resources after detectors thrown out
w32 clu2det[NCLUST];   // bits 23..0 valid. 1:det is IN
cleanTRBIF(&rbifnew,0);
// convert decimal or hexa number to w32:
for(iclu=0;iclu<NCLUST;iclu++)clu2det[iclu]=0;
if(DBGmask) { printf("applyMask: Detector2Clust: det:0xclusts: "); };
for(id=0;id<NDETEC;id++) {
  int iclus;
  iclus=part->Detector2Clust[id];
  if(DBGmask) { printf("%d:%x ",id, iclus); };
  if(iclus) {
    // iclus:1..0x3f (0x3f: id is in all clusters)
    for(iclu=0;iclu<NCLUST;iclu++) {
      if( iclus & (1<<iclu)) {
        clu2det[iclu]= clu2det[iclu]|(1<<id);
      };
    };
    realmask= realmask | (1<<id);   //det. is at least in 1 cluster
  };
}; // realmask: valid detectors (i.e. at least in 1 cluster)
if(strcmp(strdetmask,"")==0) {  // mask not applied mask=""
 detmask=realmask;
}else{ 
  if(gethexdec(strdetmask, &detmask)) return(2);
}
if(DBGmask) printf("\n");
// detmask: '1' for valid detectors
if((detmask & realmask)!= detmask) {    // (| would be for 1=active)
  w32 baddets; int i  ;
  char emsg[ERRMSGL]; char baddetstr[ERRMSGL];
  char detmasks[ERRMSGL], realmasks[ERRMSGL];
  // only detectors defined for partition can be choosen to be valid:
  baddets= detmask & (~realmask);
  baddetstr[0]='\0';
  for(i=0; i<24; i++) {
    if((baddets & (1<<i))!=0) {
      Tdetector *vls; char dtname[20];
      vls= findLTUdaqdet(i);
      if(vls!=NULL) {strcpy(dtname, vls->name);} else {
        sprintf(dtname,"ECS#:%d",i);
      };
      strcat(baddetstr, dtname); strcat(baddetstr," ");
    };
  };
  sprintf(emsg, "applyMask: Detectors: %s not allowed in partition %s (whole cluster(s) thrown out due to applied filter on CTP inputs...).",
    baddetstr, part->name);
  prtError(emsg); infolog_trg(LOG_FATAL, emsg);
  i= bit2name(detmask,detmasks); i= bit2name(realmask,realmasks);
  sprintf(emsg, "Required detectors:%s (0x%x), but only available dets:%s (0x%x).",
    detmasks, detmask, realmasks, realmask );
  prtError(emsg); infolog_trg(LOG_FATAL, emsg);
  return(2);
};
part->MaskedDetectors=detmask;
if(DBGmask) {   // only warning (for ECS):
  printf("applyMask: mask: 0x%x realmask:0x%x\n",detmask, realmask);
  printf("           clu2det:");
  for(iclu=0;iclu<NCLUST;iclu++) printf(" 0x%x", clu2det[iclu]);
  printf("\n");
};
for(id=0;id<NDETEC;id++) {
  int iclus;
  if((detmask & (1<<id))!=0) continue;   // id is not masked
  iclus=part->Detector2Clust[id];
  if(iclus) {
    part->Detector2Clust[id]= 0;   // this det is not used
    for(iclu=0;iclu<NCLUST;iclu++) {  // it can belong to more clusters!
      if( iclus & (1<<iclu)) {
        clu2det[iclu]= clu2det[iclu] & ~(1<<id);
        if(clu2det[iclu]==0) {   // Cluster iclu became empty, check classes
          char emsg[ERRMSGL];
          for(icla=0 ; icla<NCLASS  ; icla++){
            if(part->klas[icla] == NULL) continue;
            //printTKlas(part->klas[icla],icla);
            int clust;
            clust=part->klas[icla]->l0vetos & 0x7;
            // shared resources released later...
            if(clust == iclu+1) {   //release this class
              if(DBGmask)printf("releasing class %i \n",icla);
              free(part->klas[icla]); part->klas[icla]=NULL;
            };
          };
          sprintf(emsg, "applyMask: Cluster iclu:%d became empty\n", iclu);
          infolog_trgboth(LOG_WARNING, emsg);
        };
      };
    };
  } else {
    /*char emsg[ERRMSGL];
    sprintf(emsg, "applyMask(added 11.12.2009): detector %d does not belong to part. %s",
      id, part->name);
    prtError(emsg);*/
    continue;   // checking next masked detectors
  };
};
// release shared resources (i.e. allocate by going over classes left):
  upperbit= L0CONBITLac;
  bcmfrom=8; bcmto=19;
for(icla=0 ; (icla<NCLASS) ; icla++){
  TKlas *cls;
  int bit;
  cls= part->klas[icla]; if(cls == NULL) continue;
  if(DBGmask)printf("checking class%i l0inputs:0x%x ivetos:0x%x \n",
    icla,cls->l0inputs, cls->l0vetos);
  for(bit=L0CONBIT0;bit<=upperbit;bit++) { // l0f1/2 rnd1/2 bc1/2 l0f3/4
    //printf("applyMask:bit:%d\n", bit);
    retrc=retrc+checkRES(bit, part, cls, &rbifnew);    
    if(retrc>0) break;
  };
  // check id rnd1 in for INPRND1
  if(part->inpsctp){
    retrc += checkRND1(part->rbif,&rbifnew);
  }
  //
  if(retrc>0) break;
  for(bit=bcmfrom;bit<=bcmto;bit++) {     // BCM1..4 or BCM1..12
    w32 mask;
    mask=1<<bit;
    if((mask & (~cls->l0vetos))) {   // BCM[bit-7] used
      rbifnew.BCMASKuse[bit-bcmfrom]= bit-(bcmfrom-1);
      //printf("applyMask:enabling bit:%d <- %d\n", bit-bcmfrom,bit-(bcmfrom-1));
      if(bcmscopied==0) {// following copy needs to be done only once.
        int bc;
        bcmscopied=1; 
        //strcpy(rbifnew.BCMASK,part->rbif->BCMASK); 16bits instead of 4 before:
        for(bc=0; bc<ORBITLENGTH; bc++) {
          rbifnew.BCMASK[bc]= part->rbif->BCMASK[bc];
        };
      };
    };
  };
  // 
  for(bit=4;bit<=7;bit++) {         // PF1..4 
    w32 mask; int ixpf;
    mask=1<<bit; ixpf= bit-4;
    if((mask & (~cls->l0vetos))) {
      //retrc= copycheckPF(&rbifnew, part->rbif, ixpf);
      copyTPastFut(&rbifnew.pf[ixpf], &part->rbif->pf[ixpf]);
      rbifnew.PFuse[ixpf]=1;
      //if(retrc>0) break;
    };
  };
  if(retrc>0) break;
  
};
/*was till 11.12.2009  -now: see the loop above
BC masks fast fix: if  BCMASK in pcfg, always go to hw
if(part->rbif->BCMASKuse[0]){
 int i;
 for(i=0;i<4;i++)rbifnew.BCMASKuse[i]=1;
 strcpy(rbifnew.BCMASK,part->rbif->BCMASK);
}*/

if(retrc>0) {
  printf("Resource usage left according to old, unmasked partition\n");
  printf("rsrc(0..    useold    usenew\n");
  for(isr=0; isr< ixrbifdim; isr++) {   // over shared rsrcs
    printf("%d    %d     %d\n", isr, part->rbif->rbifuse[isr],
      rbifnew.rbifuse[isr]);
  }; printf("\n"); fflush(stdout);
} else {
  copyTRBIF(part->rbif, &rbifnew);
};
nclu=0;
for(iclu=0;iclu<NCLUST;iclu++){
  if(clu2det[iclu]!=0) nclu++;
};
if(DBGmask) 
  printf("applyMask: %d active clusters in %s:\n",nclu, part->name);
if(nclu==0) {
  retrc=1;
};
return(retrc);
}
/*
 * check if rbifnew has rnd1.
 * if yes ok.
 * if no check if rbif has end.
 * if yes copy to rfbifnew
 * if no error
*/
int checkRND1(TRBIF* rbif,TRBIF* rbifnew)
{
 printf("rbifuse %i \n",rbifnew->rbifuse[ixrnd1]);
 if(rbifnew->rbifuse[ixrnd1] != 51) return 0;
 if(rbif->rbifuse[ixrnd1]==61){
   rbifnew->rbif[ixrnd1]=rbif->rbif[ixrnd1];
   rbifnew->rbifuse[ixrnd1]=61;
   printf("checkRND1: rnd1 added to resources \n");
   return 0;
 } 
 printf("Error: chcekRND1: RND1 not defined for INPRND1 \n");
 return 1;
}
/*----------------------------------------------int nofclustTpartition();
  calculate # of clusters in this partition.
*/
int nofclustTpartition(Tpartition *part,int *nofclust){
 int i,j,clusters[NCLUST];
 w32 clustercode;
 if(part == NULL){
  printf("nofclustTpartition: partition does not exist (NULL)\n");
  return 1;
 }
 for(i=0;i<NCLUST;i++)clusters[i]=0;
 for(i=0; i<NDETEC ;i++ ){
   if((clustercode=part->Detector2Clust[i])){
     for(j=0;j<NCLUST;j++){
       if(clustercode & (1<<j))clusters[j]=clusters[j]+1;
     }
   }
 }
 *nofclust=0;
 for(i=0;i<NCLUST;i++)if(clusters[i])*nofclust=*nofclust+1;
 return 0; 
}
/*----------------------------------------------int nofdetecTpartition();
  calculate # of detectors in this partition.
*/
int nofdetecTpartition(Tpartition *part,int *nofdet){
 int i;
 if(part == NULL){
  printf("nofdetecTpartition: partition does not exist (NULL)\n");
  return 1;
 }
 *nofdet=0;
 for(i=0; i<NDETEC ;i++ ){
   if(part->Detector2Clust[i])*nofdet=*nofdet+1;
 }
 return 0; 
}
/*--------------------------------------------------int checkdetTpartition()
 Checks if detector idet is in partition *part
*/
int checkdetTpartition(Tpartition *part,int idet,int *flag){
 int i;
  if(part == NULL){
  printf("checkdetTpartition: partition does not exist (NULL)\n");
  return 1;
 }
 *flag=0;
 for(i=0;i<NDETEC;i++){
   if(part->Detector2Clust[i] != 0){
     if(idet == i){
       *flag=1;
        break;
     }
   }
 }
 return 0;
}
/*---------------------------------------------------int checkClustl012Tpartition()
  Checks if classes at level L0,L1,L2 are compatible
*/
int checkClustV0Tpartition(Tpartition *part){
 int i,ret=0;
 for(i=0;i<NCLASS;i++){
  if(part->klas[i] == NULL) continue;
  if(checkCluV0TKlas(part->klas[i])){
   printf("Class %i wrong cluster \n",i+1);
   ret++;
  }
 }
 return ret;
}
/*---------------------------------------------------int checkClustl012Tpartition()
  Checks if classes at level L0,L1,L2 are compatible
*/
int checkClustVl012Tpartition(Tpartition *part){
 int i,ret=0;
 for(i=0;i<NCLASS;i++){
  if(part->klas[i] == NULL) continue;
  if(checkCluVL012TKlas(part->klas[i])){
   printf("Class %i wrong cluster \n",i+1);
   ret++;
  }
 }
 return ret;
}
/*---------------------------------------------------int checkTClustVDClustTpartition()
  Check if Clusters from triggers are compatible with clusters
  from detectors
*/
int checkTClustVDClustTpartition(Tpartition *part){
 int Cluster[NCLUST];
 int i,j,clust;
 //Klas clusters
 for(i=0;i<NCLUST;i++)Cluster[i]=0;
 for( i=0 ; i<NCLASS ; i++){
  if(part->klas[i] == NULL) continue;
  clust=part->klas[i]->l0vetos & 0x7;
  if((clust == 0) || (clust == 7)){
   printf("checkTClustVDClustTpartition: class %i cluster=%i \n ",i+1,clust);
   return 1;
  }
  Cluster[clust-1]=Cluster[clust-1]+1;
 }
 // Detector2Clust clusters (from FO)
 for(i=0;i<NDETEC;i++){
  for(j=0;j<NCLUST;j++){
   if( (part->Detector2Clust[i]) & (1<<j)){
     if(Cluster[j] == 0){
      printf("checkTClustVDClustTpartition: uncompatible clusters: cluster %i in detector %i not found in classes\n",j+1,i);
      return 1;
     }
   }
  }
 }
 return 0;
}
/*-------------------------------------------------*/ int getPartDetectors(Tpartition *part) {
int idet, rc=0, rc_exp=0;
for(idet=0;idet<NDETEC;idet++){
  int pclu, pclust, hwclust;
  pclust= part->Detector2Clust[idet]; // det. can belong to more clusters!
  if(pclust ==0) continue;
  // idet is in pclust, find HWclust:
  rc_exp= rc_exp | (1<<idet);   // should be enough, but chek for possible int. error
  for(pclu=0; pclu<NCLUST; pclu++) {
    if(pclust & (1<<pclu)) {
      //printf("updateDAQClusters:findHWc:%s pclust:%x pclu:%x\n", part->name, pclust, pclu);
      hwclust= findHWCluster(part, pclu+1);
      if(hwclust>0) {
        rc= rc | (1<<idet);
      };
    };
  };
};
if(rc!=rc_exp) {
  char errmsg[120];
  sprintf(errmsg,"getPartDetectors: rc:0x%x expected:0x%x", rc, rc_exp);
  intError(errmsg);
};
return(rc);
}
/*---------------------------------------------------------getBusyMaskPartition()
 Purpose: calculate cluster busy mask for partition part (for pause/resume)
 Parameters: input: part, 
   detectors: 0: return all clusters in partition
            !=0: consider only clusters with these detectors
 Returns: cluster mask, i.e. 5..0 bits, 1: cluster in part
*/
w32 getBusyMaskPartition(Tpartition *part, int detectors){
w32 clust=0, exp_clust=0;
int i, locdetectors; int idet;
if(detectors==0) {
  locdetectors= 0xffffff;
  for(i=0;i<NCLUST;i++){ // old way (all clusters)
    if(part->ClusterTable[i]) exp_clust=exp_clust | (1<<i);
  };
} else {
/* instead of all clusters, find out only those defined in detectors pattern
   detector pattern not checked for correctness here.
*/
  locdetectors= detectors;
};
//printf("getBusyMaskPartition: detectors:0x%x %s\n", detectors, part->name);
for(idet=0;idet<NDETEC;idet++){
  int pclu, pclust, hwclust;
  if((locdetectors & (1<<idet)) == 0 ) continue;
  pclust= part->Detector2Clust[idet]; // det. can belong to more clusters!
  if(pclust ==0) continue;
  for(pclu=0; pclu<NCLUST; pclu++) { // idet is in pclust, find HWclust:
    if(pclust & (1<<pclu)) {
      //printf("updateDAQClusters:findHWc:%s pclust:%x pclu:%x\n", part->name, pclust, pclu);
      hwclust= findHWCluster(part, pclu+1);
      //printf("getBusyMaskPartition:idet:%2d pclust:%d pclu:%d hwclust:%d\n", idet, pclust, pclu,hwclust);
      if(hwclust>0) {
        clust= clust | (1<<(hwclust-1));
      } else {
        printf("ERROR internal hwclust:0 for pclu(0..5):%d\n", pclu);
      };
    };
  };
};
if(detectors==0) {   // check if available (for possible bug in ctpproxy code...)
  if(exp_clust != clust) {
    char errmsg[120];
    sprintf(errmsg,"getBusyMaskPartition: %s dets:0x%x clust:0x%x exp:0x%x\n",
      part->name, detectors, clust, exp_clust);
    intError(errmsg);
  };
};
if(DBGbusy) {
  printf("getBusyMaskPartition: %s dets:0x%x clust:0x%x exp:0x%x\n",
    part->name, detectors, clust, exp_clust);
};
//printf("getBusyMaskPartition: part:%s 0x%x \n", part->name, clust);
return clust;
}
/*------------------------------------------------------- prtalignment1()
I: level:0..2, inp:1..24, af, leng
   err:0 don't issue error if not connected
       1       issue error if not connected
 
O: af updated if level/inp is configured
   rc: 0: ok
      -1: input not connected -error message issued
      -2: short af array (af emptied by af<-'\0')
*/
int prtalignment1(int level, int inp, char *af, int leng, int err) {
int rc;
rc= findInput(level, inp);
if(rc>-1) {
  char line[80];
  int edge,delay,dmin,dmax;
  char edg; 
  edge= validCTPINPUTs[rc].edge;
  delay= validCTPINPUTs[rc].delay;
  dmin= validCTPINPUTs[rc].deltamin;
  dmax= validCTPINPUTs[rc].deltamax;
  if(edge==0) edg='P' ;else edg='N';
  sprintf(line,"%s %c %d %d %d\n", validCTPINPUTs[rc].name, 
    edg, delay, dmin, dmax); 
  if((strlen(af) + strlen(line)) >= (unsigned int)leng) {
    infolog_trgboth(LOG_FATAL, "getctp_alignment: short char array reserved for output");
    af[0]='\0'; rc=-2; 
  } else {
    sprintf(af,"%s%s", af,line);
    rc=0;
  };
} else {
  if(err==1) {
    char msg[200];
    sprintf(msg, "getctp_alignment: L%d input %d not connected", level, inp);
    infolog_trgboth(LOG_FATAL, msg); rc=-1;
  } else {
    rc=0;
  };
};
return(rc);
}
/*---------------------------------------------------getctp_alignment()
Input: leng: length of af (maximum)
       af: pointer to char array
       partit: partition (only inputs used in this partition are relevant)
               NULL -print complete alignment table (all inputs)
Output: af=='' if error (short af)
        af: alignment info in format:
L012 L1 L2
name PN delay

where: name: input name (i.e. 0SCP, 1ZDC...)
PN  P (positive) N (negative)
delay 0-15 in BCs
*/
void getctp_alignment(Tpartition *partit, char *af, int leng, w32 l0finputs) {
int tl1, tl2;
tl1= getTL1(); tl2= getTL2(); sprintf(af, "L012 %d %d\n", tl1, tl2);
if(partit==NULL) {
  int level, ninp, inp;
  for(level=0; level<=2; level++ ) {
    if(level==2) { ninp=12; } else { ninp=24; };
    for(inp=1; inp<=ninp; inp++ ) {
      int rc;
      rc= prtalignment1(level, inp, af, leng, 0);
      if(rc < -1) return;
    };
  };
} else {   // look for relevant inputs only
  int iclass;
  w32 doneinputs[3];
  for(iclass=0; iclass<3; iclass++) {doneinputs[iclass]=0;};
  for(iclass=0; iclass<NCLASS; iclass++) {
    int hwclassn,ixlevel; TKlas *klpo, *klas;
    if((klas=partit->klas[iclass]) == NULL) continue;
    hwclassn= klas->hwclass;  // 0..NCLASS-1
    klpo= HW.klas[hwclassn];
    // *klpo should be checked with *klas (internal consistency)
    for(ixlevel=0; ixlevel<3; ixlevel++) {
      int ix;
      for(ix=0; ix<26; ix++) {   // 0..23 L0 inputs, 24,25 L0functions
        w32 ins012; w32 bit; w32 ins012bit;
        if( (ixlevel!=0) && (ix>23) ) break;
        if( ixlevel==2) { if(ix>11) break; ins012= klpo->l2definition; };
        if( ixlevel==1) { ins012= klpo->l1definition; };
        if( ixlevel==0) { ins012= klpo->l0inputs; };
        bit=(1<<ix);
        ins012bit= ins012 & bit;  //1:input not used 0:input is used
        if( ins012bit == 0) {  //1:input not used 0:input is used
          if(doneinputs[ixlevel]&bit) {
            continue;   // already printed out
          };
          if(( ix>23) && (ixlevel==0) ) {   // L0fun
            char emsg[200];
            if(l0finputs==0) {
              sprintf(emsg,"getctp_alignment:l0Fun %d used but l0finputs is 0",
              ix-24+1);
              infolog_trgboth(LOG_FATAL, emsg);
            };
            // go through all inputs defined by this l0f:
          } else {
            // ixlevel, ix
            int rc;
            rc= prtalignment1(ixlevel, ix+1, af, leng, 1);
            if(rc < -1) return;
          };
          doneinputs[ixlevel]= doneinputs[ixlevel] | bit;  // register it was already printed out
        };
      };
    };
  };
  // add l0finputs (from getInputDets):
  {int ix;
  printf("applying l0fs in ctp_alignment:0x%x\n", l0finputs);
  for(ix=0; ix<4; ix++) {
    w32 bit; int rc;
    bit=(1<<ix);
    if(l0finputs & bit) {
      if(doneinputs[0]&bit) continue;   // already printed out
      rc= prtalignment1(0, ix+1, af, leng, 1);
      if(rc < -1) return;
    };
  };
  };
};
}
//------------------------------------------------------------------------
// Hardware
//------------------------------------------------------------------------
/*----------------------------------------------------------initHW()
  Purpose: to initialise HW structure
  Parameters: none
  Globals: output HW
  Returns: error code: 0=ok
  Comment:
*/
int initHW(Hardware *HW){
 int i;
 for(i=0;i<NCLASS;i++){
  HW->klas[i] = (TKlas *) malloc(sizeof(TKlas));
  if(HW->klas[i] == NULL){
   printf("initHW error: not enough memory \n");
   return 1;
  }
  strcpy(HW->name,"CTP");
 }
 HW->rbif= allocTRBIF();
 HW->inpsctp = new TINPSCTP;
 cleanHardware(HW, 0);
 return 0;
}
/*-------------------------------------------------------printHardware()
*/
void printHardware(Hardware *hwpart, char *dbgtext){
 int i; w32 mskCLAMASK;
 if(hwpart == NULL){
  printf("printHardware: partition does not exist (NULL)\n");
  return;
 }
 if(hwpart->name == NULL)printf("%s Hardware name=NULL \n", dbgtext);
 else printf("%s Hardware name: %s -----------\n", dbgtext,hwpart->name);
 printTINPSCTP(hwpart->inpsctp);
 printTRBIF(hwpart->rbif); mskCLAMASK=getCLAMASK();
 for(i=0;i<NCLASS;i++){
  TKlas *klas;
  if((klas=hwpart->klas[i])) {
    if(((klas->l0vetos & mskCLAMASK)==0) ||    // class mask is 0
       (klas->hwclass!=0) ) printTKlas(klas,i);
  };
 }
 for(i=0;i<NFO;i++){
  int j,det;
  w32 clust;
  printf("FO.%i 0x%x ",i+1,clust=hwpart->fo[i].cluster);
  for(j=0;j<4*8;j++){
   if(clust & (1<<j)){
     Connector2Detector(i+1,(j/8)+1,&det);
     //printf("Cluster %i  det=%i %s",(j%8),det,validLTUs[det].name);
     printf("%s.%i ",validLTUs[det].name,(j%8)+1);
   }
  }
  printf("\n");
 }
 //printf("SDGS:\n");
 //for(int i=0;i<NCLASS;i++){
 // printf("%4i 0x%6x 0x%6x |",i,hwpart->sdgs[i],hwpart->lmsdgs[i]);
 // if((i+1)%5 == 0)printf("\n");
 //} 
}
/*----------------------------------------------------cleanHardware()
Clean HW structure in memory, before it is filled by
data from combined partitions.
initHW allocates memory;
*/
void cleanHardware(Hardware *hw, int leaveint){
 int i;
 //hw->name="EMPTY";
 for(i=0;i<NCLASS;i++)cleanTKlas(hw->klas[i]);
 cleanTINPSCTP(hw->inpsctp);
 cleanTRBIF(hw->rbif, leaveint);   // also PF
 for(i=0;i<NFO;i++){
  cleanTFO(&hw->fo[i]);
 }
 cleanTBUSY(&hw->busy);
for(i=0; i<NCLASS; i++) {hw->sdgs[i]=i; hw->lmsdgs[i]=i;}
}
/*-----------------------------------------------------copyHardware()
  All memory allocated by initHW
*/
void copyHardware(Hardware *to,Hardware *from){
 int i;
 strcpy(to->name, from->name);
 for(i=0;i<NCLASS;i++) copyTKlas(to->klas[i],from->klas[i]);
 copyTRBIF(to->rbif,from->rbif);
 for(i=0;i<NFO;i++) to->fo[i]=from->fo[i];
 copyTBUSY(&(to->busy),&(from->busy));
for(i=0;i<NCLASS;i++) {
  to->sdgs[i]= from->sdgs[i]; to->lmsdgs[i]= from->lmsdgs[i];
};
}
/*----------------------------------------------------------load2HW()
  Purpose: load HW to hw
  Parameters: *hw, *tsname -the name of other TimeSharing partition
  Operation:   
    -program CTP hardware (ALL classes,clusters, FO,...) according to hw,BUT:
    -program only classes allocated for partitions
     not using dynamic class grouping (TIMESHARING).
     Problem: the way how to exclude 'currently
     inluded' partition has to be found -i.e. we WANT to program
     even TIMESHARING partition when loaded FIRST time
     tsname: "" no TSpart present, program all classes
           !="" check part->klas[i]->partname
  Returns: error code: 0:ok
*/
int load2HW(Hardware *hw, char *tsname){
w32 isp,bb, overlap,flag,bcmaskn;
TKlas *klas;
TRBIF *rbif;
w32 l0invAC, minAC;
w32 rate_mask;
//int parthwclasses[NCLASS]; // 0:can be reloaded 1: the TIMESHARING class
char skipped[200]="";
printf("load2HW nams %s %i exit %i \n",tsname,tsname[0],strlen(tsname));
//if(strlen(tsname) != 0)exit(1);
if(l0C0()) {
  rate_mask= RATE_MASKr2;
} else {
  rate_mask= RATE_MASK;
};

l0invAC=L0_INVERTac; minAC=0;
// find out TIMESHARING classes, using StartedPartitions:
// seems this serves to nothing...
//for(i=0;i<NCLASS;i++) parthwclasses[i]=0;
for(isp=0;isp<MNPART;isp++){
  Tpartition *part;
  if(StartedPartitions[isp] == NULL) continue;
  part= StartedPartitions[isp];
  if(part->nclassgroups == 0) continue;  // can be reprogrammed in HW
  printf("load2HW: %s %d part->hwallocated:%d hwclass:", 
    part->name, part->run_number, part->hwallocated);
  for(int i=0;i<NCLASS;i++){
    int hwc;
    if(part->klas[i]!=NULL) {
      hwc= part->klas[i]->hwclass;
      printf("%d ",hwc);
      //if() parthwclasses[i]=1;
    } else {
      printf(". ");
    };
  }; printf("\n");
};
 //------------------------------------------- INPSCTP
 if(hw->inpsctp){
   vmew32(RND1_EN_FOR_INPUTS,hw->inpsctp->rnd1enabled1);
   vmew32(RND1_EN_FOR_INPUTS+4,hw->inpsctp->rnd1enabled2);
   printf("load2HW: RND1_EN_FOR_INPUTS set: 0x%x 0x%x \n",hw->inpsctp->rnd1enabled1,hw->inpsctp->rnd1enabled2);
 }
 //------------------------------------------- RBIF
 rbif=hw->rbif;
 printTRBIF(rbif);
 vmew32((RANDOM_1), rbif->rbif[ixrnd1]);
 vmew32((RANDOM_2), rbif->rbif[ixrnd2]);
 vmew32((SCALED_1), rbif->rbif[ixbc1]);
 vmew32((SCALED_2), rbif->rbif[ixbc2]);
 printf("load2HW: 4 L0 gens -> 4 LM gens...\n");
 vmew32(LM_RANDOM_1, rbif->rbif[ixrnd1]);
 vmew32(LM_RANDOM_2, rbif->rbif[ixrnd2]);
 vmew32(LM_SCALED_1, rbif->rbif[ixbc1]);
 vmew32(LM_SCALED_2, rbif->rbif[ixbc2]);
if(l0C0()>=0xc606) {
  int ixf; // 4 8-inputs luts:
  for(ixf=0; ixf<4; ixf++) {
    setShared4(ixf+1, &rbif->lut8[ixf*LUT8_LEN]);
  };
  for(ixf=0; ixf<8; ixf++) {
    printf("load2HW set lut8[%d] %s\n", ixf+1, &rbif->lut8[ixf*LUT8_LEN]);
  };
} else {
 // 2 4-inputs luts:
 //vmew32((L0_FUNCTION1), rbif->rbif[ixl0fun1]);
 //vmew32((L0_FUNCTION2), rbif->rbif[ixl0fun2]);
 // following works if the same signals connected to first 4 inputs on L0 vs LM levels
 //vmew32(LM_FUNCTION1, rbif->rbif[ixl0fun1]);
 //vmew32(LM_FUNCTION1+4, rbif->rbif[ixl0fun2]);
 //vmew32(LM_FUNCTION1+8, 0);    //LMF3/4 <- 0
 //vmew32(LM_FUNCTION1+12, 0);
 printf("Error: Unexpected firmware version\n");
 return 1;
};
//------------------------------------------- L0f34 + BCmasks
//todo:
flag=0;
if(l0AB()==0) {
  bcmaskn=BCMASKN;
  for(int i=ixlut3132;i<=ixlut4142;i++)if(rbif->rbifuse[i]!=notused){
    printf("load2HW: l0f%d: allocated in %d\n", i-ixlut3132+3,rbif->rbifuse[i]);
    flag++;
  };
  printf("load2HW: l0f34 not available on LM0 board\n"); /*
  if(flag) {
    int rc;
    char m4[LEN_l0f34+1];
    combine34(rbif->lut34, m4);
    rc=setL0f34c(0, m4);
    if(rc!=0) {
      printf("load2HW:load l0f34 rc:%d\n", rc);
    };
  }; */
} else {bcmaskn=4;};

flag=0;
for(w32 i=0;i<bcmaskn;i++)if(rbif->BCMASKuse[i])flag++;
if(flag)loadBCmasks(rbif->BCMASK);
//------------------------------------------- PF
// new PF
for(int i=0;i<NPF;i++){
 if(rbif->PFuse[i] != 0){
   TPastFut *pf=&rbif->pf[i];
   int ret=loadPF2HW(pf);
   if(ret) return ret;
 }
}

//------------------------------------------- classes
skipped[0]='\0';
/*strcpy(skipped,"NO CLASS POGRAMMING!"); rwclasses(); if(NCLASS==12345678) {
printf("load2HW:only 6 classes,no L0vetos  written\n");
for(i=0;i<6;i++) */
for(w32 i=0;i<NCLASS;i++) {
  w32 mskbit; int skip=0;
  if(hw->klas[i] == NULL){
   char msg[200];
   sprintf(msg,"load2HW sw error: unexpected HW.klas[%i]=NULL \n",i);
   intError(msg);
   return 1;
  }
  //usleep(100000);
  klas=hw->klas[i];
  bb=4*(i+1);
  //printTKlas(klas, i); this is misleadind as classes are chneged later
  vmew32(L0_CONDITION+bb,klas->l0inputs);
  if(i>=minAC)vmew32(l0invAC+bb,klas->l0inverted);
  if(l0AB()==0) {   //firmAC or >C0
    if(l0C0()) {
      w32 lmm, l0m, lmcond, l0vets, lmvets;
      //l0vets= (klas->l0vetos & 0x00ffffff) | ((hw->sdgs[i])<<24);
      l0vets= (klas->l0vetos);
      // keep it disabled on L0 level, we cannot change cluster + enable in 1 write
      l0vets= l0vets | 0x800000;   
      vmew32(L0_VETOr2+bb,  l0vets);
      lmcond= klas->lmcondition;
      l0m= (klas->l0vetos & 0xfff00)>>8;
      lmm= (klas->lmcondition & 0xfff00000) >> 20;   // 
      if(l0m != lmm) {
        lmcond= lmcond & 0x000fffff;
        lmcond= lmcond | (l0m<<20);
        printf("load2HW: cl:%d BCmsks lmcond:0x%x corrected to l0veto:0x%x \n", i+1,lmm, l0m);
      };
      vmew32(LM_CONDITION+bb, lmcond);
      vmew32(LM_INVERT+bb,klas->lminverted);
      //lmvets= (klas->lmvetos & 0x80ffffff) | ((hw->lmsdgs[i])<<24);
      lmvets= (klas->lmvetos);
      vmew32(LM_VETO+bb,lmvets);
      //printf("load2HW:%3d l0c+v: 0x%x 0x%x lmc+v: 0x%x 0x%x\n", i+1, klas->l0inputs, l0vets, lmcond, lmvets);
    } else {
      vmew32(L0_VETO+bb,(klas->l0vetos)&0x1fffff);
    };
  } else {
    vmew32(L0_VETO+bb,(klas->l0vetos)&0xffff);
  };
  vmew32(L1_DEFINITION+bb, klas->l1definition);
  if(i>=1)vmew32(L1_INVERT+bb, klas->l1inverted);   // was 44 (bug) before 13.9.2014
  vmew32(L2_DEFINITION+bb, klas->l2definition);
  // check part. name (do not reprogram TSpartitions) + classgroup
  if(tsname[0]=='\0') {skip=0;               // no TS or TS just being started
  } else if(klas->classgroup == 0) { skip=0; // this class is always in
  } else if(strcmp(tsname, klas->partname) == 0) {skip=1; 
  } else { skip=0; };
  if(skip==1) {
    sprintf(skipped,"%s %d", skipped, i);
  } else {
    if(l0AB()==0) {  //firmAC and 
      if(l0C0()==0) {  // L0 (not LM0) board
        mskbit= (klas->l0vetos)>>31; vmew32(L0_MASK+bb, mskbit);
      };
      // no need for LM0 (done above)
    } else {
     mskbit= ((klas->l0vetos)&0x10000)>>16; vmew32(L0_MASK+bb, mskbit);
    };
  };
};
if(skipped[0]!='\0') printf("load2HW:skipped classes:%s\n", skipped);
if(l0C0()==0) {
for(int j=0;j<NCLASS;j++){
  vmew32(L0_SDSCG+(j+1)*4, hw->sdgs[j]);
};
} else {
 ; //see L0-VETOs
};
 //--------------------------------------------- L0 downscalers
 //vmew32(getRATE_MODE(),1);   /* vme mode */
 //vmew32(RATE_CLEARADD,DUMMYVAL);
 //for(i=0; i<NCLASS; i++) {
   /* 23.6.2014: no reason to set 0..49 in bits 30..25,
      although see note in ctp.h at RATE_MASK). From now, put 0 above bit 25
   vmew32(RATE_DATA, (i<<25) | (hw->klas[i]->scaler & RATE_MASK)); */
   //vmew32(RATE_DATA, (hw->klas[i]->scaler & rate_mask));
 //};
 //vmew32(getRATE_MODE(),0);   /* normal mode */
 //--------------------------------------------- LM downscalers
 //vmew32(LM_RATE_MODE,1);   /* vme mode */
 vmew32(LM_RATE_CLEARADD,DUMMYVAL);
 for(int i=0; i<NCLASS; i++) {
   /* 23.6.2014: no reason to set 0..49 in bits 30..25,
      although see note in ctp.h at RATE_MASK). From now, put 0 above bit 25
   vmew32(RATE_DATA, (i<<25) | (hw->klas[i]->scaler & RATE_MASK)); */
   vmew32(LM_RATE_DATA, (hw->klas[i]->lmscaler & rate_mask));
 };
 //vmew32(LM_RATE_MODE,0);   /* normal mode */
 // ------lsfr seeds - seed=sdg : this can be changed 
 vmew32(LM_RATE_CLEARADD,DUMMYVAL);
 for(int i=0; i<NCLASS; i++) {
   vmew32(LM_RATE_RND_OFFSET, (hw->sdgs[i]));
 };
 vmew32(LM_RATE_RND_RESET, 0);
 printf("SDGS:\n");
 for(int i=0;i<NCLASS;i++){
  printf("%4i 0x%6x 0x%6x |",i,hw->sdgs[i],hw->lmsdgs[i]);
  if((i+1)%5 == 0)printf("\n");
 } 
 //--------------------------------------------- FOs
 for(int i=0; i<NFO; i++){
   if((notInCrate(i+FO1BOARD)==0)) {
     w32 vmeaddr, data;
     vmeaddr= FO_CLUSTER+BSP*(i+1); data= hw->fo[i].cluster; 
     printf("load2HW: FO%d addres:0x%x data:0x%x \n", i+1, vmeaddr,data);
     vmew32(vmeaddr, data);
   };
 }
 //--------------------------------------------- BUSYs
if(DBGbusy) printf("load2HW. SET_CLUSTER T 1..6:");
for(int i=0;i<NCLUST+1;i++){
  if(DBGbusy) {
     printf("0x%x ",hw->busy.set_cluster[i]);
  };
  vmew32(BUSY_CLUSTER+i*4,hw->busy.set_cluster[i]);
}; 
overlap= calcOverlap(hw->busy.set_cluster);
vmew32(BUSY_OVERLAP, overlap);
if(DBGbusy)printf("BUSY_OVERLAP:0x%x\n", overlap);
// finally enable all allowed classes on L0 level:
//printf("Finally allow all allowed classes:\n"); removed 30.5.2017
for(int i=0;i<NCLASS;i++) {
  w32 l0vets;
  klas=hw->klas[i];
  bb=4*(i+1);
  //l0vets= (klas->l0vetos & 0x00ffffff) | ((hw->sdgs[i])<<24);
  l0vets= klas->l0vetos;
  printTKlas(klas, i);
  //if((l0vets & 0x800000)==0) {
  vmew32(L0_VETOr2+bb,  l0vets);
  //};
  //printf(" load2HW: enab/dis:%d 0x%x\n", i+1, l0vets);
};
return 0;
}
/*----------------------------------------------------------readHW()
  Purpose: read Hardware to hw
  Parameters: *hw 
  Returns: error code: 0=ok
  Comment: - inverse of load2HW.
           - to be used for detection of on fly  configuration changes
*/
int readHW(Hardware *hw){
 w32 i,bb, overlap,overlaphw;
 TKlas *klas;
 TRBIF *rbif;
w32 l0invAC, minAC;
//if(l0AB()==0) {l0invAC=L0_INVERTac; minAC=0; } else { l0invAC=L0_INVERT; minAC=44;};
l0invAC=L0_INVERTac; minAC=0;
 //------------------------------------------- RBIF
 rbif=hw->rbif;
 rbif->rbif[ixrnd1]=vmer32((RANDOM_1));
 rbif->rbif[ixrnd2]=vmer32((RANDOM_2));
 rbif->rbif[ixbc1]=vmer32((SCALED_1));
 rbif->rbif[ixbc2]=vmer32((SCALED_2));
if(l0C0()>=0xc606) {
   // here should be lut8 ?
   printf("L0F LMF not read yet neither l0f+lmf...\n");
} else {
 //rbif->rbif[ixl0fun1]=vmer32((L0_FUNCTION1));
 //rbif->rbif[ixl0fun2]=vmer32((L0_FUNCTION2));
 //printf("LMF not read yet...\n");
 printf("Error: Unexpected firmware version.\n");
 return 1;
};
 //------------------------------------------- classes
 for(i=0;i<NCLASS;i++){
    w32 l0vetos;
    if(hw->klas[i] == NULL){
     char msg[200];
     sprintf(msg,"readHW sw error: unexpected hw.klas[%i]=NULL \n",i);
     intError(msg);
     return 1;
    }
    klas=hw->klas[i];
    bb=4*(i+1);
    //printTKlas(klas, i);
    // L0
    klas->l0inputs=vmer32(L0_CONDITION+bb);
    if(i>=minAC)klas->l0inverted=vmer32(l0invAC+bb);
    else klas->l0inverted=0;
    if(l0C0()) {
      l0vetos=vmer32(L0_VETOr2+bb);
    } else {
      w32 mskbit;
      l0vetos=vmer32(L0_VETO+bb);
      mskbit=vmer32(L0_MASK+bb);
      l0vetos= (l0vetos&0x1fffff) | ((mskbit&0x1)<<31);
    };
    klas->l0vetos=l0vetos;
    //L0 scaler done separately to keep vme access low
    //L1
    klas->l1definition=vmer32(L1_DEFINITION+bb);
    if(i>=1)klas->l1inverted=vmer32(L1_INVERT+bb);   // was 44 (bug) before 13.9.2014
    else klas->l1inverted=0;
    //L2
    klas->l2definition=vmer32(L2_DEFINITION+bb);
 }
 //--------------------------------------------- L0 downscalers
 //vmew32(getRATE_MODE(),1);   /* vme mode */
 //vmew32(RATE_CLEARADD,DUMMYVAL);
 //for(i=0; i<NCLASS; i++)hw->klas[i]->scaler=vmer32(RATE_DATA);
 //vmew32(getRATE_MODE(),0);   /* normal mode */
 //--------------------------------------------- FOs
 for(i=0; i<NFO; i++){
   if((notInCrate(i+FO1BOARD)==0)) {
     w32 vmeaddr;
     vmeaddr= FO_CLUSTER+BSP*(i+1); 
     //printf("readHW: FO%d addres:0x%x data:0x%x \n", i+1, vmeaddr,data);
     hw->fo[i].cluster=vmer32(vmeaddr);
   };
 }
 //--------------------------------------------- BUSYs
if(DBGbusy) printf("readHW. SET_CLUSTER T 1..6:");
for(i=0;i<NCLUST+1;i++){
  if(DBGbusy) {
     printf("0x%x ",hw->busy.set_cluster[i]);
  };
  hw->busy.set_cluster[i]=vmer32(BUSY_CLUSTER+i*4);
}; 
overlap= calcOverlap(hw->busy.set_cluster);
overlaphw= vmer32(BUSY_OVERLAP) &0x1fffff;
if(DBGbusy){
  char msg[200];
  sprintf(msg, "BUSY_OVERLAPhw:0x%x expected:0x%x\n", overlaphw, overlap);
  if(overlap != overlaphw) intError(msg);
};
return 0;
}
/*----------------------------------------------------------compHardware
*/
void compHardware(Hardware *hw1,Hardware *hw2){
 int i;
 TRBIF *rb1,*rb2;
 rb1=hw1->rbif;
 rb2=hw2->rbif;
 for(i=0;i<ixrbifdim;i++)
  if(rb1->rbif[i] != rb2->rbif[i]){
   printf("RBIF change detected ixres=%i \n",i);
  }
}
