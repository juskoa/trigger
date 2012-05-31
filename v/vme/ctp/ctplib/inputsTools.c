/* synchronisation/alignmnet subroutines */
#include "stdio.h"
#include "vmewrap.h"
#include "ctp.h"
/*
board:0:busy (the CLK edge for input ORBIT signal) 
      1..3:L0/1/2  
input: busy: no sense,  L0,L1:1..24   L2:1-12
edge: 0:Positive 1:Negative

*/
void setEdge(int board,w32 input,w32 edge){
 w32 word;
 if(edge>1){
   printf("Too big edge %i /n",edge);
   return;
 }
 if(board == 0){
  word=vmer32(BUSY_ORBIT_SELECT);
  word=word&0xffffefff;
  if(edge) word=word | 0x1000;
  vmew32(BUSY_ORBIT_SELECT,word);
 }else{
  word=vmer32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1));
  word=word&0xfffffeff;
  if(edge)word=word+0x100;
  vmew32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1),word);
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
*/
int getedge(int board,w32 input,w32 *del){
 w32 edge=0;
 if(board == 0){
   edge=(vmer32(BUSY_ORBIT_SELECT)&0x1000)>>12;
 }else if((board == 1) || (board == 2)){
  if((input>24) || (input < 1)){
   printf("Input number out of range %i <E>\n",input);
   return 4;
  }
  edge=vmer32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1));
  *del=(edge&0xf);
  edge=(edge&0x100)==0x100;
 }else if(board == 3){   
  if((input>12) || (input < 1)){
   printf("Input number out of range %i <E>\n",input);
   return 5;
  }
  edge=vmer32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1));
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
w32 synch;
synch=vmer32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1));
synch= (synch&0xfffffef0) | ((edge & 0x1)<<8) | (delay&0xf);
vmew32(BSP*ctpboards[board].dial+SYNCH_ADD+4*(input-1),synch);
//printf("setEdgeDelay2: %d i:%d: %d %d=0x%x\n", board, input,edge,delay, synch);
}
/*------------------------------------------------ printEdgeDelay()
 print Edge/Delay on stdout */
void printEdgeDelay(int board){
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



