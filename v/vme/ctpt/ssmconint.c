/*  Interaction board
 *  Int board would be used in ingen mode only with DAQ
 *  Int board should be used as genereting board, i.e. on the first postion in the list *boards
 *  also in outmon mode. The reason is that outmon does not have ORBIT signal and any other
 *  position would cause error of no orbit found.
 */ 
#include "ssmconnection.h"
/*---------------------------------------------------------checkModeINT()
 *   
 */ 
int checkModeINT(int n, int *boards,w32 modecode,w32 submode){
 int ret=0;
 //printf("checkModeL0: modecode,submode %x %x \n",modecode,submode);
 switch(modecode){
	   case 0x104:   //Int outgen mode
                      ret=Connect(n,boards);
                      break;
	   case 0x33f:  // Generic none mode
		if(submode == 1){
                  ret=L2a2Interface(boards[1],boards[2],boards[0]);
		}else{
      		 printf("checkModeINT: unknown submode %i \n",submode);
      		 return 2;	 
		}
		break;
	   default:
	   printf("checkModeINT:  modecode %x not found.\n",modecode);
           return 1;	   
   } 
 return ret;
}
/*------------------------------------------------------setBoardINT()
 *  Set HW for different submodes. For submodes see checkMode()
 */
int setBoardINT(int n,int *boards,w32 modecode,w32 submode){
 int ret=0;
 switch(modecode){
  case 0x33f: // Generic none mode, nothing to do
	  HardWare.loop=0;
	  break;
  default:HardWare.loop=0;break;
 }
 return ret;
}
/*-------------------------------------------------------initMode()
 * Initialization of the submode for items which should be done 
 * outside the loop, if any. For submodes see checkMode().
 */
int initModeINT(int n,int *boards,w32 modecode,w32 submode){
 switch(modecode){
 }
 return 0;
}
/*--------------------------------------------------------writeBoards()
*/ 
int writeBoardsINT(int n, int *boards, w32 modecode,w32 submode){
 int i;
 switch(modecode){
  case 0x33f:    // Generic none mode
  break; 
  default:
    printf("writeBoardsINT: None modecode %i not found.\n",modecode);
  return 1;  
 }
 for(i=1;i<n;i++)writeSPn(boards[i],0,1,0);   //Write all 0 to receiving boards
 if(!DEBFLG)for(i=0;i<n;i++)writeSSM(boards[i]);// Write ssm[board[i]].sm to hardware
return 0;	
}

