/*
  The first versions of this code are also in eprex4:~rl/alice/align
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    /* usleep */
#include "vmewrap.h"
#include "ctp.h"
#include "ssmctp.h"
#include "bakery.h"
#define NUMOFCHAN 32
// 24 = inps, 32 = channels in L0 outmon
#define NUMOFL0INPS (24+32)
#define NUMOFH0INPS (32)
#define NUMOFINPS (24+32+24+12)
int syncSSM(int n, int *boards);
int startSSMnor(int n,int *board);
int condstopSSM(int board, int cntpos, int maxloops, int sleepafter, int customer);
//int condstopSSM(int board,int counter,int maxloops,int sleepafter);
/*  
    All code is written optimising the speed not the elegance or readability.
    The motivation is that we may need many ssms to get result.
*/
/*--------------------------------------------------------
Structure for zero suppressed ssm memory.
One structure is one channel of ssm. It remembers postions corrected by offset.
NMAX 50000 should correspond about 10 normal ssms for 200 kHz int rate.
*/
#define BUFSIZE 50000
typedef struct SSM0CHAN{
 //char name[32];
 w32 *bctime;
 w32 size;
 w32 last;    
 //int input; this is known from index of INPUTS 
 int ssmid;
 struct SSM0CHAN *next;
}SSM0CHAN;
typedef struct RAREFLAG{
int board,input,mode;
}RAREFLAG;
// GLOBAL variables
int IDread;  //id if ssm
SSM0CHAN *INPUTS[NUMOFINPS];
int corefun[NUMOFINPS];
RAREFLAG RareFlag;
/*FGROUP ALItools
*/
void setRareFlag(int board,int input,int mode){
 RareFlag.board=board;
 RareFlag.input=input;
 RareFlag.mode=mode;
}
/*FGROUP ALItools
*/
void initAignment(){
 int i;
 for(i=0;i<NUMOFINPS;i++)INPUTS[i]=NULL;
 IDread=0;
}
/*FGROUP ALItools
*/
void printINPUTS(){
 SSM0CHAN *inp;
 int i,iok;
 for(i=0;i<NUMOFINPS;i++){
  inp=INPUTS[i];
  while(inp != NULL){
   if(i<NUMOFL0INPS)iok=i+1;
   else if(i<(NUMOFL0INPS+24))iok=i-NUMOFL0INPS+1;
   else iok=i-NUMOFL0INPS-24+1;
   //printf("Input= %i %i %i, size=%i ID= %i \n",iok,i,inp->input,inp->size,inp->ssmid);
   printf("Input= %i %i, size=%i last=%i ID= %i \n",iok,i,inp->size,inp->last,inp->ssmid);
   inp=inp->next;
  }
 }
}
/*FGROUP ALItools
*/
void resetINPUTS(){
 int i;
 SSM0CHAN *inp1,*inp2;
 for(i=0;i<NUMOFINPS;i++){
  inp1=INPUTS[i];
  while(inp1 != NULL){
   inp2=inp1->next;
   free(inp1->bctime);
   free(inp1);
   inp1=inp2;
  }
  INPUTS[i]=NULL;
 }
 IDread=0;
}
//--------------------------------------------
void add2INPUTS(SSM0CHAN *Input,int input){
 if(INPUTS[input]==NULL){
   INPUTS[input]=Input;
   Input->next=NULL;
 }
 else{
  SSM0CHAN *i;
  i=INPUTS[input];
  Input->next=i;
  INPUTS[input]=Input;
 } 
}
/*-------------------------------------------------------------------
Adds chan0 to chanM to INPUTS
- level : position of input in INPUTS starts at level
- offset: for synchronisation of signals from different boards
*/
int add2SSM0(w32 *sm,int chan0,int chanM,int level,w32 offset){
 int i,input, j;
 w32 buffer[32][BUFSIZE];
 w32 size[32],last[32];  
 for(i=0;i<32;i++){
   size[i]=0;
   last[i]=Mega;
 }
 // Zero supress ssm to buffer
 for(i=3;i<Mega;i++){
  for(j=chan0;j<chanM;j++){
   input=j-chan0;
   if(size[input]<BUFSIZE){
     if(sm[i]&(1<<j)){
       buffer[input][size[input]]=i;
       size[input]=size[input]+1;
     }
   }else{
    if(last[input]==Mega){
     printf("add2SSM0 WARNING: input %i level %i overflow buffer at %i\n",input,level,i);
     last[input]=i;
    }
   }
  }
 }
 // move buffer to ssm zero supressed
 for(i=chan0;i<chanM;i++){
  //printf("size i %i %i %i\n",size[i-chan0],i,i-chan0);
  if(size[i-chan0]>0){
    SSM0CHAN *input0;
    w32 *bctime;
    input0 = (SSM0CHAN *)malloc(sizeof(SSM0CHAN));
    if(input0 == NULL) goto RET1;
    bctime = (w32 *)malloc(size[i-chan0]*sizeof(w32));
    if(bctime == NULL) goto RET2;
    for(j=0;(w32)j<size[i-chan0];j++)bctime[j]=buffer[i-chan0][j]+offset; // +OR-!!!
    input=i-chan0+level;
    input0->size=size[i-chan0];
    input0->last=last[i-chan0];
    //input0->input=input; 
    input0->ssmid=IDread;
    input0->bctime=bctime;
    add2INPUTS(input0,input);
  }
 }
 return 0;
 RET1:
 printf("add2SSM0: not enough memory \n");
 return 1;
 RET2:
 printf("add2SSM0: not enough memory \n");
 return 2;
}
/*--------------------------------------------------------------------------------------
*/
void board2pars(int board,int *chan0,int *chanM,int *level){
  if(board == 1){
   *chan0=8;*chanM=32;
   *level=0;
 }else if(board == 2){
   *chan0=8;*chanM=32;
   *level=NUMOFL0INPS;
 }else{
   *chan0=6;*chanM=18;
   *level=NUMOFL0INPS+24;
 }
}
/*----------------------------------------------------------wbit()
 * writes bit=(0 or 1) in word num at position channel.
 */ 
int wbit(w32 num, w32 bit,int channel){
 if(bit)return (1<<channel) | num;
 else return ~(1<<channel) & num;
}
/*-------------------------------------------------------------------------
see cnmaes2.sorted
*/
#define L0START (CSTART_L0+66)
#define L1START (CSTART_L1+6)
#define L2START (CSTART_L2+6)
int findcounter(int board,int mode,int input){
 if(board == 1){
  if(mode == 0){
   return (L0START-1+input);
  }else{
   // outmon for L0
   if(input<8 && input>0){
    return (152-1+input);
   }else if(input == 8) return 98;
    else if(input == 9) return 98;  // trigger data with strobe
    else if(input == 10) return 151;  
    else if(input < 14) return 92+input-11;  
    else if(input < 18) return 8+input-14;  
    else if(input == 18) return 7;  
    else if(input == 29) return 150;
    else return 0;
  }
 }else if(board == 2){
  return (L1START-1+input);
 }else if(RareFlag.board == 3){
  return (L2START-1+input);
 }else{
  printf("findcounter error: uknown board %i \n",board);
  return 0;
 }
}
/*FGROUP ALItools
------------------------------------------------------------------------------
Record and read 1SSM
*/
void take1SSM(int board,int ntimes){
 int i,chan0,chanM,level,rare,counter;
 rare = (board == RareFlag.board) && (RareFlag.mode == 0);
 board2pars(board,&chan0,&chanM,&level);
 if(rare){
   counter=findcounter(RareFlag.board,RareFlag.mode,RareFlag.input);
   if(counter == 0){
    printf("take1SSM: Unknown input: %i \n",RareFlag.input);
    return;
   }
   setomSSM(board,0xb);
 }
 else setomSSM(board,0xa); //before after
 printf("take1SSM: rare,board,counter: %i %i %i\n",rare,board,counter);
 for(i=0;i<ntimes;i++){   
  startSSM1(board);
  usleep(30000);
  if(rare){
    printf("take1SSM:  2 rare,board,counter: %i %i %i ",rare,board,counter);
    condstopSSM(board,counter,100,10000,ccread_inputs);
    printf(" finished succesfuly \n");
  }else{
    stopSSM(board);
  }
  readSSM(board);
  IDread++;
  add2SSM0(sms[board].sm,chan0,chanM,level,0);
 }
}
/*FGROUP ALItools
*/
void filter(w32 *sm,int chan){
 int i;
 w32 bitp,bitn,mask;
 printf("Filtering channel %i \n",chan);
 mask=(1<<chan);
 bitp=(sm[0] & mask) == mask;
 for(i=1;i<(Mega);i++){
  bitn=(sm[i] & mask) == mask;
       if((bitp==0) && (bitn==0)) sm[i-1]=wbit(sm[i-1],0,chan);
  else if((bitp==0) && (bitn==1)) sm[i-1]=wbit(sm[i-1],0,chan);
  else if((bitp==1) && (bitn==0)) sm[i-1]=wbit(sm[i-1],1,chan);
  else if((bitp==1) && (bitn==1)) sm[i-1]=wbit(sm[i-1],0,chan);
  else printf("filter error /n");
  bitp=bitn;
 }
}
/*FGROUP ALItools
----------------------------------------------------------------------------
Record and read 1SSM
*/
void takerbSSM(int ntimes){
 int i,chan0=0,chanM=31,level=24;
 int rare,counter;
 int board=1;
 rare = (board == RareFlag.board) && (RareFlag.mode == 1);
 if(rare){
   counter=findcounter(RareFlag.board,RareFlag.mode,RareFlag.input);
   if(counter == 0){
    printf("take1SSM: Unknown input: %i \n",RareFlag.input);
    return;
   }
   setomSSM(board,0x3);
 }else setomSSM(board,0x2);
 for(i=0;i<ntimes;i++){   
  startSSM1(board);
  usleep(30000);
  if(rare){
    condstopSSM(board,counter,5000,13000,ccread_inputs);
  }else{
   stopSSM(board);
  }
  readSSM(board);
  IDread++;
  //filter(sms[board].sm,25);
  add2SSM0(sms[board].sm,chan0,chanM,level,0);
 }
}
/*FGROUP ALItools
---------------------------------------------------------------------------------------
Record and read and synchronise 2 SSMs
*/
int take2SSM(int board1,int board2,int ntimes){
 int boards[2];
 int i,dontcare;
 int rare,counter;
 int chan01,chanM1,level1;
 int chan02,chanM2,level2;
 w32 offset;

 rare = (board1 == RareFlag.board) || (board2 == RareFlag.board);
 if(board2 == RareFlag.board){  // board1 assumed to have flag
   int board = board2;
   board2=board1;
   board1=board;
 }
 printf("take2SSM: rare= %i\n",rare);
  if(rare){
   counter=findcounter(RareFlag.board,RareFlag.mode,RareFlag.input);
   if(counter == 0){
    printf("take2SSM: Unknown input: %i \n",RareFlag.input);
    return 1;
   }
   setomSSM(board1,0xb);
   setomSSM(board2,0xb);
 }else{
   setomSSM(board1,0xa);
   setomSSM(board2,0xa);
 }

 board2pars(board1,&chan01,&chanM1,&level1);
 board2pars(board2,&chan02,&chanM2,&level2);
 boards[0]=board1;
 boards[1]=board2;
 
 // board2 starts first
 for(i=0;i<ntimes;i++){
   if(startSSMnor(2,boards)){
    printf("startSSM error: SSMs have not started inside one ORBIT. Sync not possible \n");
   }
   usleep(40000);
  if(rare){
    condstopSSM(board1,counter,100,10000,4);
    stopSSM(board2);
  }else{
    stopSSM(board1);
    stopSSM(board2);
  }
   readSSM(board1);
   readSSM(board2);
   IDread++;
   dontcare= syncSSM(2,boards);
   printf("offsets: %i %i\n",sms[board1].offset,sms[board2].offset);
   offset=sms[1].offset-sms[2].offset;
   add2SSM0(sms[board1].sm,chan01,chanM1,level1,0);   
   add2SSM0(sms[board2].sm,chan02,chanM2,level2,offset);   
 }
 return 0;
}
/*FGROUP ALItools
To be written. We do not need it untill any L2 inputs exits
*/
int take3SSM(int ntimes){
 int i,dontcare;
 int boards[3];
 int offset1,offset2;
 boards[0]=1;
 boards[1]=2;
 boards[2]=3;
 setomSSM(1,0xa);
 setomSSM(2,0xa);
 setomSSM(3,0xa);
 for(i=0;i<ntimes;i++){
   if(startSSMnor(3,boards)){
    printf("startSSM error: SSMs have not started inside one ORBIT. Sync not possible \n");
   }
 }
 usleep(40000);
 readSSM(1);
 readSSM(2);
 readSSM(3);
 IDread++;
 dontcare= syncSSM(3,boards);
 printf("offsets: %i %i %i\n",sms[1].offset,sms[2].offset,sms[3].offset);
 offset1=sms[1].offset-sms[2].offset;
 offset2=sms[1].offset-sms[3].offset;
 add2SSM0(sms[1].sm,8,31,0,0);   
 add2SSM0(sms[2].sm,8,31,1,offset1);   
 add2SSM0(sms[3].sm,6,17,2,offset2);   
 return 0;
}
/*
*/
void correl(w32 *b1,w32 *b2,w32 size1,w32 size2,int cordist,int delta,w32 *corfun){
 w32 i,j;
 for(i=0;i<size1;i++){
   for(j=0;j<size2;j++){
     if(abs(b2[j]-b1[i]-cordist) < delta) corfun[b2[j]-b1[i]-cordist+delta]++;
  }
 }
}
/*FGROUP ALItools
  delta=0 -> autocor fun
  delta=1 -> -1,0,1
*/
void croscor1(int input1,int input2,int cordist,int delta,int dir){
 SSM0CHAN *in1,*in2,*in;
 w32 corfun[2*delta+1];
 int i;
 if(dir == 0)cordist=-cordist;
 for(i=0;i<(2*delta+1);i++)corfun[i]=0;
 in1=INPUTS[input1];
 in=INPUTS[input2];
 while(in1 != NULL){
  in2=in;
  while((in2 != NULL) && (in1->ssmid != in2->ssmid))in2=in2->next;
  if(in2 != NULL){
   correl(in1->bctime,in2->bctime,in1->size,in2->size,cordist,delta,corfun);
   in=in2;
  }
  in1=in1->next;
 }
 printf("Croscor: %i %i \n",input1,input2);
 for(i=0;i<2*delta+1;i++)printf("<%i> <%i> ",i-delta+cordist,corfun[i] );
 //for(i=0;i<2*delta+1;i++)printf("%i ",corfun[i] );
 printf("<input %i %i> <end>\n",input1,input2);
}
/*FGROUP ALItools
Autocorel one input
cordist>0
*/ 
void autocor(int input,int cordist,w32 delta){
 SSM0CHAN *inp;
 int autofun[delta];
 w32 last,size,i,j;
 w32 *bc;
 for(i=0;i<delta;i++)autofun[i]=0;
 inp=INPUTS[input];
 while(inp != NULL){
  bc=inp->bctime;
  size=inp->size;
  last=inp->last;
  for(i=0;i<(size);i++){
   j=i;
   if((last-bc[i])<delta) continue;
   while(((bc[j]-bc[i])<(w32)cordist) && (j<size))j++; 
   if(j==size) continue;   
   while(((bc[j]-bc[i]-cordist)<delta) && (j<size)){
     autofun[bc[j]-bc[i]-cordist]++;
     j++;
   }
  }
  inp=inp->next;
 }
 printf("Autocor: ");
 for(i=0;i<delta;i++)printf("<%i> <%i> ",i+cordist,autofun[i]);
 printf(" <input %i> ",input);
 printf("<end>\n");
}
/*FGROUP ALItools
Steering routone for correlation:
 type =0 : autocorrelation = noise, bckg
 type =1 : crosscorrelation = alignment
 h0chans = channels in ssm in l0 output mode
*/
void Correl(int type,int l0inputs,int l1inputs,int l2inputs,int h0chans,int cordist,int delta,int dir){
 int i,j,nactive;
 int activeinputs[NUMOFINPS];
 for(i=0;i<NUMOFINPS;i++)activeinputs[i]=0;
 nactive=0;
 for(i=0;i<(NUMOFL0INPS-NUMOFH0INPS);i++){
  if(l0inputs & (1<<i)){
    activeinputs[nactive]=i;
    nactive++;
  }
 }
 for(i=0;i<NUMOFH0INPS;i++){
  if(h0chans & (1<<i)){
    activeinputs[nactive]=i+NUMOFL0INPS-NUMOFH0INPS;
    nactive++;
  }
 }
 for(i=0;i<24;i++){
  if(l1inputs & (1<<i)){
    activeinputs[nactive]=i+NUMOFL0INPS;
    nactive++;
  }
 }
 for(i=0;i<12;i++){
  if(l2inputs & (1<<i)){
   activeinputs[nactive]=i+NUMOFL0INPS+24;
   nactive++;
  }
 }
 if(type==1){
   for(i=0;i<nactive;i++){
    for(j=i;j<nactive;j++){
     if(i != j)croscor1(activeinputs[i],activeinputs[j],cordist,delta,dir);
    }
   }
  }else{
   for(i=0;i<nactive;i++)autocor(activeinputs[i],cordist,delta);
  }
}
//--------------------------------------------------------------------------
// startssm normal order, only CTP boards
//-------------------------------------------------------------------------
int startSSMnor(int n,int *boards){
 int i,board,rc=0;
 w32 boardoffset, seconds1,micseconds1, seconds2,micseconds2,diff;
 w32 status;
 GetMicSec(&seconds1, &micseconds1);
 for(i=0;i<n;i++){
  board= boards[i];
  boardoffset=BSP*ctpboards[board].dial; 
  vmew32(SSMaddress+boardoffset,0);
  /*    micwait(30); */
  status= vmer32(SSMstatus+boardoffset);
  vmew32(SSMstart+boardoffset, DUMMYVAL);
 };
 GetMicSec(&seconds2, &micseconds2);
 if((diff=DiffSecUsec(seconds2, micseconds2, seconds1, micseconds1))>80) {
  rc=10;
  printf("startSSM: diff[usecs]=%d SSMstatus before start:%x\n",diff, status);
 };
 return(rc);
}
