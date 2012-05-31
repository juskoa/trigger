#include <stdio.h>
#include "vmewrap.h"
#include "ctp.h"
#include "ssmctp.h"

/*----------------------------------------------------------------scan()
    Finds first 1 and then checks if they follow Orbit.
    If fails on the first orbit, try next one (only next one)
    since first orbit can be inetrupted.
*/
int scan(int board, int *offset)
{
 int bit,i=0,i0,flag=0;
 //int start=3;  // This is necessary due to wrong ssm at beginning
 w32 OrbitMask;
 if(!sms[board].sm){
  printf("scan: Board %i sm = NULL\n",board);
  return 3;
 }
 OrbitMask=1<<sms[board].orbit; // This is different from ssmconnection
 //printf("Orbitmask %i %i \n",board,OrbitMask);
 //i=start;
CONT:
 //Find first 1
 while( ((i<Mega) && (sms[board].sm[i]&OrbitMask) == 0))i++;
 if(i == Mega){
   printf("scan error: Board: %i No Orbit found. \n",board);
   return 1;   
 }
 *offset=i;
 while(i<Mega){
      i0=i;
      while((i-i0)<40 && i<Mega){
        bit = (sms[board].sm[i]&OrbitMask) == OrbitMask; 
        if(!bit){
	 if(flag) goto ERR1;
	 // one more chance since first orbit can be broken
	 flag=1;
         goto CONT;	 
	}
        i++;	
      }      
      while((i-i0)<Orbit && i<Mega){
        bit = (sms[board].sm[i]&OrbitMask) == OrbitMask;
        if(bit) goto ERR2;
        i++;	
      }
 }
 //printf("%i \n",flag);
 return 0;
ERR1:
 printf("scan: 0 found in first 40 bits of ORBIT %i.\n",i);
 return 1;
ERR2:
 printf("scan: 1 found outside ORBIT %i.\n",i);
 return 2;
}
/*------------------------------------------------------ syncSSM()
Synchronise SSMs (pointers to their contents are in sms[i].sm)
Input:
n - number of SMS to be synced (minimal 1)
... -numbers of SSMs to be synced.
ORDER is IMPORTANT !
First SSM in list -> first SSM started -> largest offset.
Assumption: all SSM start during one orbit.
(Solution without this assumption may exists if SSM multiple of Orbit).
syncSSM checks if orbit channel is found (!= 33).
If orbit channel is not found, syncssm does not synchronise that board.
*/
int syncSSM(int n, int *boards) {
 int i,j,ret,offset,offsets[NSSMBOARDS];
 
 for(i=0; i<n; i++){                       // loop over ssms
  offsets[i]=-1;
  if(sms[boards[i]].orbit != 33){
    if((ret=scan(boards[i],&offset))){
      printf("syncSSM error: WRONG ORBIT board=%i ret=%i \n",boards[i],ret);
      ret=1;
      goto RET;
    }else{
      printf("syncSSM: Board %i ORBIT correct, offset= %i\n",boards[i],offset);
      offsets[i]=offset;
      for(j=0;j<i;j++){
         if(offsets[j] < offset) offsets[j] = offsets[j]+Orbit;
      }
    }
   }
 }
 for(i=0;i<n;i++){
    sms[boards[i]].syncflag=SYNCFLAG;
    sms[boards[i]].offset=offsets[i];
 }
 ret=0;
 RET:
 return ret; 
}

