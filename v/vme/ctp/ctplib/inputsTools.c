/* synchronisation/alignmnet subroutines */
#include "stdio.h"
#include "vmewrap.h"
#include "ctp.h"
#include "ctplib.h"

// 0x1f for 1..24, 0x1f<<8 for 25..48
#define DELAYBITS 0x1f 
/*
board:0:busy (the CLK edge for input ORBIT signal) 
      1..3:L0/1/2  
input: busy: no sense,  L0,L1:1..24   L2:1-12
                        LM: 1..48
edge: 0:Positive 1:Negative
      Note: common for LM/L0 inputs (i.e. 48 values for LM0)
*/
void setEdge(int board,w32 input,w32 edge) {
 w32 word;
 if(board == 0){
  word=vmer32(BUSY_ORBIT_SELECT);
  word=word&0xffffefff;
  if(edge) word=word | 0x1000;
  vmew32(BUSY_ORBIT_SELECT,word);
 }else{
   if((board==1) && (l0C0()!=0)) {
     int inp24; w32 ms2, msk;
     if(input>24) { inp24= input-24; } else { inp24= input; };
     word=vmer32(BSP*ctpboards[board].dial+SYNCH_ADDr2+4*(inp24-1));
     if(input>24) {
       //msk= 0xffffefff; ms2= 0x00001000;
       msk= 0xffff7fff; ms2= 0x00008000;
     } else {
       //msk= 0xffffffef; ms2= 0x00000010;
       msk= 0xffffff7f; ms2= 0x00000080;
     };
     word= word & msk;
     if(edge) word= word | ms2;
     vmew32(BSP*ctpboards[board].dial+SYNCH_ADDr2+4*(inp24-1),word);
   } else {
     word=vmer32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1));
     word=word & 0xfffffeff;
     if(edge) word=word | 0x100;
     vmew32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1),word);
   };
 } 
}
/* Read edge/delay info from hw
Inputs:
  board: 0:busy (the CLK edge for INPUT ORBIT signal)
         1..3: L0/1/2
  input: 1..24 (for L0/1 boards) 1..12 for L2 board
         1..48 for LM0 board
Outputs:
rc: 0 -positive edge
    1 -negative edge
   >3 -error (unknown board or input)
del: meaningfull only for L0/LM0/1/2 boards.
Comment:
edge is common for LM/L0 iputs
LMdelay: see getlmdelay/setlmdelay
*/
int getedge(int board,w32 input,w32 *del){
 w32 edge=0;
 if(board == 0){
   edge=(vmer32(BUSY_ORBIT_SELECT)&0x1000)>>12;
 }else if(board == 2){
   if((input>24) || (input < 1)){
     printf("L1 Input number out of range %i <E>\n",input);
     return 4;
   };
   edge=vmer32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1));
   *del=(edge&0xf);
   edge=(edge&0x100)==0x100;
 } else if(board==1) {   // L0 or LM0 boad
   if(l0C0()!=0) {   // LM0 board
     int inp24;
     if((input>48) || (input < 1)){
       printf("LM0 Input number out of range %i <E>\n",input);
       return 4;
     };
     if(input>24) { inp24= input-24; } else { inp24= input; };
     edge=vmer32(BSP*ctpboards[board].dial+SYNCH_ADDr2+4*(inp24-1));
     if(input>24) edge= edge>>8;
     //edge= edge &0x1f; *del=(edge&0xf); edge= (edge&0x10) >> 4; till 12.2.2015
     edge= edge & (0x80 | DELAYBITS); *del=(edge&DELAYBITS); edge= (edge&0x80) >> 7;
   } else {          // L0
     if((input>24) || (input < 1)){
       printf("L0 Input number out of range %i <E>\n",input);
       return 4;
     };
     edge=vmer32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1));
     *del=(edge&0xf);
     edge=(edge&0x100)==0x100;
   };
 }else if(board == 3){   
   if((input>12) || (input < 1)){
     printf("L2 Input number out of range %i <E>\n",input);
     return 5;
   };
   edge=vmer32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1));
   *del=(edge&0xf);
   edge=(edge&0x100)==0x100;
 }else{
  printf("Unknown board %i <E>\n",board);
  return 6;
 }
 return edge;
}
/*------------------------------------------------ getlmdelay()
swin: LM switch input 1..12
rc: LM delay 0..7 bcs
*/
int getlmdelay(w32 swin) {
w32 lmdel;
if(swin>12) {
  printf("ERROR getlmdelay: swin:%d\n", swin); return(0);
};
lmdel=vmer32(BSP*ctpboards[1].dial+SYNCH_ADDr2+4*(swin-1));
return((lmdel&0x7000000)>>24);
}
/*------------------------------------------------ setlmdelay()
swin: LM switch input 1..12
delay: LM delay 0..7 bcs
*/
void setlmdelay(w32 swin, int delay) {
w32 lmdel;
if(swin>12) {
  printf("ERROR setlmdelay: swin:%d\n", swin); return;
};
lmdel=vmer32(BSP*ctpboards[1].dial+SYNCH_ADDr2+4*(swin-1));
lmdel= (lmdel&0xf8ffffff) | (delay & 0x7);
vmew32(BSP*ctpboards[1].dial+SYNCH_ADDr2+4*(swin-1), lmdel);
return;
}
/*------------------------------------------------ setEdgeDelay()
set Edge/Delay 
Inputs:
board: 1:L0 2:L1 3:L2
input: 1..24, 1..12 for L2, 1..48 for LM0
edge:  0:positive 1:negative
delay: 0..15, LM0: 0..31 */
void setEdgeDelay(int board, int input, int edge, int delay){
w32 synch;
if((board==1) && (l0C0()!=0)) {
  int inp24; w32 ed,msk;
  if(input>24) { inp24= input-24; } else { inp24= input; };
  synch=vmer32(BSP*ctpboards[board].dial+SYNCH_ADDr2+4*(inp24-1));
  //ed= ((edge & 0x1)<<4) | (delay&0xf); till 12.2.2015
  ed= ((edge & 0x1)<<7) | (delay&DELAYBITS);
  if(input>24) {
    //msk= 0xffffe0ff;
    msk= 0xffff60ff;   // should use DELAYBITS
    ed= ed<<8;
  } else {
    //msk= 0xffffffe0;
    msk= 0xffffff60;
  };
  synch= (msk & synch) | ed;
  vmew32(BSP*ctpboards[board].dial+SYNCH_ADDr2+4*(inp24-1), synch);
} else {
  synch=vmer32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1));
  synch= (synch&0xfffffef0) | ((edge & 0x1)<<8) | (delay&0xf);
  vmew32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1),synch);
};
//printf("setEdgeDelay: %d i:%d: %d %d=0x%x\n", board, input,edge,delay, synch);
}
/* print LM/L0 dealys in case of L0 (board;1)
*/
void printEdgeDelay(int board) {
w32 delay; int edge,ix,maxix;
if(board==0) {
  edge= getedge(board, 0, &delay);
  printf("BUSY board, clock edge for orbit:%d\n", edge);
} else {
  if(board==1 || board==2) {
    maxix=24;
    if((board==1) && (l0C0()!=0)) {
      maxix=48;
      printf("Input Edge delay LMdelay\n");
    } else {
      printf("Input Edge delay\n");
    };
  } else {
    maxix=12;
    printf("Input Edge delay\n");
  };
  printf("Input Edge delay\n");
  for(ix=1; ix<=maxix; ix++) {
    edge= getedge(board, ix, &delay);
    if((ix<=12) && (maxix==48)) {
      int lmdelay;
      lmdelay= getlmdelay(ix);
      printf("%2d     %d     %d   %d\n", ix, edge, delay, lmdelay);
    } else {
      printf("%2d     %d     %d\n", ix, edge, delay);
    };
  };
};
}
/* i48 -> i24  Set L0 CTP switch only */
void setSwitch(int i48, int i24) {
w32 synch;
synch=vmer32(BSP*ctpboards[1].dial+SYNCH_ADDr2+4*(i24-1));
synch= (synch & 0xffc0ffff) | ((i48 & 0x3f)<<16);
vmew32(BSP*ctpboards[1].dial+SYNCH_ADDr2+4*(i24-1), synch);
}
/* i12 -> i12  Set LM CTP switch only */
void setLMSwitch(int in12, int out12) {
w32 synch;
if((in12>12) || (out12>12)) {
  printf("ERROR setLMdSwitch: in12:%d out12:%d\n", in12, out12); return;
};
synch=vmer32(BSP*ctpboards[1].dial+SYNCH_ADDr2+4*(out12-1));
synch= (synch & 0x0fffffff) | ((in12 & 0xf)<<28);
vmew32(BSP*ctpboards[1].dial+SYNCH_ADDr2+4*(out12-1), synch);
}
/* Print info about L0 + LM CTP switches */
void printSwitch() {
int input;
printf("Only connected LM/L0 inputs (1..24) shown:\n");
for(input=1; input<=24; input++) {
  w32 synch, fedby, lmfedby; 
  synch=vmer32(BSP*ctpboards[1].dial+SYNCH_ADDr2+4*(input-1));
  fedby= (synch & 0x003f0000) >> 16;   // fedby:0 - L0 not connected
  lmfedby= (synch & 0xf0000000) >> 28;   // lmfedby:0 - LM not connected
  if((fedby!=0) || (lmfedby!=0)) {
    if(input<=12) {
      printf("%d <- %d lm fed by:%d\n", input, fedby, lmfedby );
    } else {
      printf("%d <- %d lm fed by:%d -should be 0\n", input, fedby, lmfedby );
    };
  };
};
}

