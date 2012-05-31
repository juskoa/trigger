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

