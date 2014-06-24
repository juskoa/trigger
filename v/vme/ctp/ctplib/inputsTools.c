/* synchronisation/alignmnet subroutines */
#include "stdio.h"
#include "vmewrap.h"
#include "ctp.h"
#include "ctplib.h"
w32 getSYNCH_ADD() {
if(l0C0()) {
  return(SYNCH_ADDr2);
} else {
  return(SYNCH_ADD);
};
}
/* 
  Find selection of input 24(ctp)->48(lm)
  Returns position of CTP input at Lm connector.
*/
int getl0selected(int input){
 int selected;
 if((input>24) || (input < 1)){
   printf("Input number out of range %i <E>\n",input);
   return 0;
 }
 selected=vmer32(BSP*ctpboards[1].dial+getSYNCH_ADD()+4*(input-1));
 return selected;
}
/*
board:0:busy (the CLK edge for input ORBIT signal) 
      1..3:L0/1/2  
input: busy: no sense,  L0,L1:1..24   L2:1-12
edge: 0:Positive 1:Negative

*/
void setEdge(int board,w32 input,w32 edge){
 int ver;
 if(edge>1){
   printf("Too big edge %i /n",edge);
   return;
 }
 ver=l0C0();
 if(ver==0){
   setEdgerun1(board,input,edge);
 }else{
   setEdgerun2(board,input,edge);
 }
}
void setEdgerun2(int board,w32 input,w32 edge){
 w32 word;
 if(board == 0){
  word=vmer32(BUSY_ORBIT_SELECT);
  word=word&0xffffefff;
  if(edge) word=word | 0x1000;
  vmew32(BUSY_ORBIT_SELECT,word);
 }else if(board == 1){
  w32 lminput=getl0selected(input);
  w32 lowhigh =  lminput > 24;
  w32 index   = (lminput-1) % 24; 
  word=vmer32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*index);
  if(lowhigh){
    word=word & 0xffffefff;
    if(edge) word |= 0x1000;
  }else{
    word=word & 0xffffffef;
    if(edge) word |= 0x10;
  }
  vmew32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*index,word);
 }else{
   word=vmer32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*(input-1));
   word=word&0xfffffeff;
   if(edge)word=word+0x100;
   vmew32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*(input-1),word);
 } 
}
void setEdgerun1(int board,w32 input,w32 edge){
 w32 word;
 if(board == 0){
  word=vmer32(BUSY_ORBIT_SELECT);
  word=word&0xffffefff;
  if(edge) word=word | 0x1000;
  vmew32(BUSY_ORBIT_SELECT,word);
 }else{
  word=vmer32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*(input-1));
  word=word&0xfffffeff;
  if(edge)word=word+0x100;
  vmew32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*(input-1),word);
 } 
}
/* Read edge/delay info from hw
Inputs:
  board: 0:busy (the CLK edge for INPUT ORBIT signal)
         1..3: L0/1/2
  input: 1..24 (for L0/1 boards) 1..12 for L2 board
Outputs:
rc: 0 -positive edge
    1 -negative edge
   >5 -error (unknown board or input)
del: meaningfull only for L0/1/2 boards.
Comment:
getedge is wrapper for getedgerun1 and getedgerun2
*/
int getedge(int board,w32 input,w32 *del){
 int ver,ret;
 ver=l0C0();
 if(ver==0){
   ret=getedgerun1(board,input,del);
 }else{
   ret=getedgerun2(board,input,del);
 }
 return ret;
}
/*
 Returns: edge 0/1
          error 2
 */
int getl0edgedel(int input,w32*del){
 w32 edge=0;
 w32 lminput=getl0selected(input);
 int lowhigh =  lminput > 24;
 int index   = (lminput-1) % 24; 
 edge=vmer32(BSP*ctpboards[1].dial+getSYNCH_ADD()+4*index);
 if(lowhigh){
   edge=(edge&0x1f00)>>8;
 }else{
   edge=edge&0x1f;
 }
 *del=(edge&0xf);
 edge=(edge&0x10)==0x10;
 return edge; 
}
int getedgerun2(int board,w32 input,w32 *del){
 w32 edge=0;
 if(board == 0){
   edge=(vmer32(BUSY_ORBIT_SELECT)&0x1000)>>12;
 }else if(board == 1){
  if((input>24) || (input < 1)){
   printf("Input number out of range %i <E>\n",input);
   return 4;
  }
   edge=getl0edgedel(input,del);
  }else if(board==2){
  if((input>24) || (input < 1)){
   printf("Input number out of range %i <E>\n",input);
   return 4;
  }
  edge=vmer32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*(input-1));
  *del=(edge&0xf);
  edge=(edge&0x100)==0x100;
 }else if(board==1){
  if((input>12) || (input < 1)){
   printf("Input number out of range %i <E>\n",input);
   return 5;
  }
  edge=vmer32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*(input-1));
  *del=(edge&0xf);
  edge=(edge&0x100)==0x100;
 }else{
  printf("Unknown board %i <E>\n",board);
  return 6;
 }
 return edge;
}
int getedgerun1(int board,w32 input,w32 *del){
 w32 edge=0;
 if(board == 0){
   edge=(vmer32(BUSY_ORBIT_SELECT)&0x1000)>>12;
 }else if((board == 1) || (board == 2)){
  if((input>24) || (input < 1)){
   printf("Input number out of range %i <E>\n",input);
   return 4;
  }
  edge=vmer32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*(input-1));
  *del=(edge&0xf);
  edge=(edge&0x100)==0x100;
 }else if(board == 3){   
  if((input>12) || (input < 1)){
   printf("Input number out of range %i <E>\n",input);
   return 5;
  }
  edge=vmer32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*(input-1));
  *del=(edge&0xf);
  edge=(edge&0x100)==0x100;
 }else{
  printf("Unknown board %i <E>\n",board);
  return 6;
 }
 return edge;
}
/*------------------------------------------------ setEdgeDelay()
set Edge/Delay 
Inputs:
board: 1:L0 2:L1 3:L2
input: 1..24 (1..12 for L2)
edge:  0:positive 1:negative
delay: 0..15*/
void setEdgeDelay(int board, int input, int edge, int delay){
 int ver;
 ver=l0C0();
 if(ver==0){
   setEdgeDelayrun1(board,input,edge,delay);
 }else{
   setEdgeDelayrun2(board,input,edge,delay);
 }
}
void setEdgeDelayrun2(int board, int input, int edge, int delay){
 w32 synch;
 if(board == 1){
  w32 lminput=getl0selected(input);
  w32 lowhigh =  lminput > 24;
  w32 index   = (lminput-1) % 24; 
  synch=vmer32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*index);
  if(lowhigh){
    synch=synch & 0xffffe0ff;
    synch |= (edge<<12) | (delay<<8);
  }else{
    synch=synch & 0xffffffe0;
    synch |= (edge<<4) | delay;
  }
  vmew32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*index,synch);
 }else{
   synch=vmer32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*(input-1));
   synch= (synch&0xfffffef0) | ((edge & 0x1)<<8) | (delay&0xf);
   vmew32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*(input-1),synch);
 }
}
void setEdgeDelayrun1(int board, int input, int edge, int delay){
 w32 synch;
 synch=vmer32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*(input-1));
 synch= (synch&0xfffffef0) | ((edge & 0x1)<<8) | (delay&0xf);
 vmew32(BSP*ctpboards[board].dial+getSYNCH_ADD()+4*(input-1),synch);
 //printf("setEdgeDelay2: %d i:%d: %d %d=0x%x\n", board, input,edge,delay, synch);
}
/*------------------------------------------------ printEdgeDelay()
 print Edge/Delay on stdout 
 It seem run1=run2 print.
*/
void printEdgeDelay(int board){
 int ver;
 ver=l0C0();
 if(ver==0){
   printEdgeDelayrun1(board);
 }else{
   printEdgeDelayrun1(board);
 }
}
void printEdgeDelayrun1(int board){
w32 delay; int edge,ix,maxix;
if(board==0) {
  edge= getedge(board, 0, &delay);
  printf("BUSY board, clock edge for orbit:%d\n", edge);
} else {
  if(board==1 || board==2) {
    maxix=24;
  } else {
    maxix=12;
  };
    printf("Input Edge delay\n");
  for(ix=1; ix<=maxix; ix++) {
    edge= getedge(board, ix, &delay);
    printf("%2d     %d     %d\n", ix, edge, delay);
  };
};
}



