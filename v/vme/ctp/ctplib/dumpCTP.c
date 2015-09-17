#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    /* usleep */
#include "vmewrap.h"
#include "ctp.h"
#include "ctplib.h"

void getdatetime(char *);
w32 loadFPGA(int board);   //is in vmeblib

w32 getADC_SELECT() {
w32 adr;
if(l0C0()) {
  adr= ADC_SELECTlm0;
} else {
  adr= ADC_SELECT;
};return(adr);
}
/*
Dump vmeword settings of ctp boards.
MOst of the words but not all.
*/
void dumpL0(FILE *f){
 //w32 i;
 w32 word,offset=0x9000;
 w32 word1,word2,word3;
 w32 rates[NCLASS];
w32 l0invAC; int i, minAC;
w32 rate_mask;
if(l0C0()) {
  rate_mask= RATE_MASKr2;
} else {
  rate_mask= RATE_MASK;
};
l0invAC=L0_INVERTac; minAC=0;
/* rates: */
vmew32(LM_RATE_CLEARADD,DUMMYVAL);
for(i=0;i<NCLASS;i++){
  rates[i]= vmer32(LM_RATE_DATA)&rate_mask;
};
 word=vmer32(offset+FPGAVERSION_ADD);
 fprintf(f,"L0 BOARD= 0x%x =======================================\n", word);
 word=vmer32(offset+getADC_SELECT());
 fprintf(f,"ADC_SELECT: 0x%x\n",word);
if(l0C0()) {
 word=vmer32(L0_TCSETr2);
} else {
 word=vmer32(L0_TCSET);
};
 fprintf(f,"L0_TCSET/TC_SET: 0x%x\n",word);
 word=vmer32(L0_TCSTATUS);
 fprintf(f,"L0_TCSTATUS/TC_STATUS: 0x%x\n",word);
 fprintf(f,"CLASSES:\n");
 for(i=0;i<NCLASS;i++){
  w32 cond,veto,mask,invert; 
  cond=vmer32(L0_CONDITION+4*(i+1));
  veto=vmer32(L0_VETO+4*(i+1));
  if(l0C0()==0) {
    mask=vmer32(L0_MASK+4*(i+1));
  } else {
    mask=0xdeadbeaf;
  };
  fprintf(f,"%2i:0x%x  0x%x 0x%x 0x%x ",i+1,cond,veto,rates[i], mask);
  if((i+1)>minAC){
   invert=vmer32(l0invAC+4*(i+1));
   fprintf(f," 0x%x\n",invert);
  }else{
   fprintf(f,"\n");
  }
 }
 word1=(L0_INTERACT1);
 word2=(L0_INTERACT2);
 word3=(L0_INTERACTT);
 word=(L0_INTERACTSEL);
 fprintf(f,"INTERACT1 INTERACT2 INTERACTT INTERACTSEL: 0x%x 0x%x 0x%x 0x%x\n",word1,word2,word3,word);
 //word1=(L0_FUNCTION1);
 //word2=(L0_FUNCTION2);
 //fprintf(f,"L0FUNCTION1 L0FUNCTION2: 0x%x 0x%x \n",word1,word2);
 fprintf(f,"SYNCAL (inputs delay and edge selector)\n");
 for(i=0;i<24;i++){
  word=vmer32(offset+4*0x141+4*i);
  fprintf(f,"0x%x ",word);
  if(i==11) fprintf(f,"\n");
 } 
 fprintf(f,"\n"); 
 // pf TO BE ADDED
}
void dumpL1(FILE *f){
 int i;
 w32 word,offset=0xa000;
 fprintf(f,"L1 BOARD====================================================\n");
 word=vmer32(offset+ADC_SELECT);
 fprintf(f,"ADC_SELECT: 0x%x\n",word);
 word=vmer32(L1_TCSET);
 fprintf(f,"L1_TCSET/TC_SET: 0x%x\n",word);
 word=vmer32(L1_TCSTATUS);
 fprintf(f,"L1_TCSTATUS/TC_STATUS: 0x%x\n",word);
 // rate mode 
 //
 fprintf(f,"CLASSES:\n");
 for(i=0;i<NCLASS;i++){
  w32 definition,invert;
  definition=vmer32(L1_DEFINITION+4*(i+1));
  if(i>=0){   // was >44 (bug) before 13.9.2014
   invert=vmer32(L1_INVERT+4*(i+1));
   fprintf(f,"%2i:0x%x 0x%x",i+1,definition,invert);
  }else{
   //fprintf(f,"%2i: 0x%x  0x%x 0x%x xxxx",i+1,cond,veto,mask);
   fprintf(f,"%2i:0x%x 0x%x",i+1,definition,invert);
  }
  fprintf(f,"\n");
 }
 fprintf(f,"TIMING:\n");
 word=vmer32(L1_DELAY_L0);
 fprintf(f,"DELAY_L0: 0x%x\n",word);
 fprintf(f,"SYNCAL (inputs delay and edge selector)\n");
 for(i=0;i<24;i++){
  word=vmer32(offset+4*0x141+4*i);
  fprintf(f,"0x%x ",word);
  if(i==11) fprintf(f,"\n");
 } 
 fprintf(f,"\n"); 
 // pf TO BE ADDED
}
void dumpL2(FILE *f){
 int i;
 w32 word,offset=0xb000;
 fprintf(f,"L2 BOARD====================================================\n");
 word=vmer32(offset+ADC_SELECT);
 fprintf(f,"ADC_SELECT: 0x%x\n",word);
 word=vmer32(L2_TCSET);
 fprintf(f,"L2_TCSET/TC_SET: 0x%x\n",word);
 word=vmer32(L2_TCSTATUS);
 fprintf(f,"L2_TCSTATUS/TC_STATUS: 0x%x\n",word);
 // rate mode 
 //
 fprintf(f,"CLASSES:\n");
 for(i=0;i<NCLASS;i++){
  w32 definition;
  definition=vmer32(L1_DEFINITION+4*(i+1));
  fprintf(f,"%2i:0x%x",i+1,definition);
  fprintf(f,"\n");
 }
 fprintf(f,"TIMING:\n");
 word=vmer32(L2_DELAY_L1);
 fprintf(f,"DELAY_L1: 0x%x\n",word);
 fprintf(f,"SYNCAL (inputs delay and edge selector)\n");
 for(i=0;i<11;i++){
  word=vmer32(offset+4*0x141+4*i);
  fprintf(f,"0x%x ",word);
 } 
 fprintf(f,"\n"); 
 // pf TO BE ADDED
}
void dumpFO(int board,FILE *f){
 w32 offset,word;
 if(notInCrate(board)) return;
 offset=BSP*ctpboards[board].dial;
 fprintf(f,"FO%i==================================================== \n",board-4);
 word=vmer32(offset+FO_CLUSTER);
 fprintf(f,"CLUSTER: 0x%x\n",word);
 word=vmer32(offset+FO_TESTCLUSTER);
 fprintf(f,"FO_TESTCLUSTER/TEST_CLUSTER: 0x%x\n",word);
 word=vmer32(offset+FO_DELAY_L1CLST);
 fprintf(f,"DELAY_L1CLST: 0x%x\n",word);
 fprintf(f,"\n");
}
/*
Open files with preposition *pre ,date and extension *ext
*/
FILE *OpenWorkFile(char *pre,char *ext){
 FILE *f;
 char *environ;
 char fnpath[1024],logname[1024];
 char dt[32];
 getdatetime(dt);
 dt[10]='_';
 environ= getenv("VMEWORKDIR"); 
 strcpy(fnpath, environ);
 strcat(fnpath,"/WORK/");  
 sprintf(logname,"%s%s_%s.%s",fnpath,pre,dt,ext);
 f=fopen(logname,"w");
 if(f==NULL)printf("OPenWorkFile: cannot open file %s\n",logname);
 return f;
}
void dumpCTP(){
 int i;
 FILE *f;
 f=OpenWorkFile("HWD/f","hwd");
 if(f==NULL) return;
 dumpL0(f);
 dumpL1(f);
 dumpL2(f);
 for(i=5;i<11;i++)dumpFO(i,f);
 fclose(f);
}

