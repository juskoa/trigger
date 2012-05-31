#include "ssmconnection.h"
/*---------------------------------------------------------checkModeBU()
 * 
*/ 
int checkModeBU(int n, int *boards,w32 modecode,w32 submode){
 int ret=0;
 //printf("checkModeL0: modecode,submode %x %x \n",modecode,submode);
 switch(modecode){
	   case 0x104:  // BUSY connections		   
		if(submode == 0 ){
		  Connect(n,boards);break;
		}
		else if(submode == 1)break;
		else {
		 printf("checkModeBU: mode 0x104 unknown modecode %i \n",submode);
		 return 1;
		}
	   default:
	   printf("checkModeBU: BUSY modecode %x not found.\n",modecode);
           return 1;	   
   } 
 return ret;
}
/*------------------------------------------------------setBoardBU()
 *  Set HW for different submodes. For submodes see checkModeBU()
 */
int setBoardBU(int n,int *boards,w32 modecode,w32 submode){
 int ret=0;
 switch(modecode){
  default:
	  //printf("setBoardBU: BUSY modecode %x not found.\n",modecode);
	  HardWare.loop=0;break;
 }
 return ret;
}
/*-------------------------------------------------------initMode()
 * Initialization of the submode for items which should be done 
 * outside the loop, if any. For submodes see checkModeL0().
 */
int initModeBU(int n,int *boards,w32 modecode,w32 submode){
 return 0;
}
/*--------------------------------------------------------writeBoardsBU()
*/ 
int writeBoardsBU(int n, int *boards, w32 modecode,w32 submode){
 int i;
 switch(modecode){
  case 0x104:
     if(submode == 0){
      // BUSY connections
      //byclstt,byclstt[1..6]	  
      for(i=1;i<8;i++)writeSPP(boards[0],0,i,"100000");
      break;
     }else if(submode == 1){
       // Generates no busy
       writeSPn(boards[0],0,1,0); 	
       break;     
     }else{
       printf("writeBoardsBU: mode 0x104 unknown modecode %x \n",submode);
       return 1;
     }
  default:
    printf("writeBoardsBU: BUSY modecode %x not found.\n",modecode);
  return 1;  
 }
 for(i=1;i<n;i++)writeSPn(boards[i],0,1,0);   //Write all 0 to receiving boards
 if(!DEBFLG)for(i=0;i<n;i++)writeSSM(boards[i]);// Write ssm[board[i]].sm to hardware
return 0;	
}

