#include "ssmconnection.h"
/*---------------------------------------------------------checkModeL2()
 * mode=0x2
*/ 
int checkModeL2(int n, int *boards,w32 modecode,w32 submode){
 int ret=0;
 //printf("checkModeL0: modecode,submode %x %x \n",modecode,submode);
 switch(modecode){
	   case 0x104: //outgen mode
                if(submode == 0){  
                  // L2 connections		   
		  Connect(n,boards);
                }else if(submode == 1){
                  /*Interface board test - it is not going to work:
                    - generation is not stopped when ctp_busy
                    - I cannot enable dll just after ssm start due to sw
                    I go via macro
                   */ 
                  //enable ddl
                  vmew32(0xc14c, 0xb);
                  ret=dumpIntSsm(boards[1]);
                } else {
                  printf("checkModeL2: L2 modecode submode %x %i not found.\n",modecode,submode);
                  return 1;	
                }
                break;
	   case 0x20c:  // L2 input generator
	        //L0ingen(n,boards);break;	
	   case 0x12:  // L2 PF mode
		// P/F testing all circuits by rnd
		if(submode == 6) ret=testallPFL12(boards[0]);
		break;
	   case 0x00a:  // L2 input monitor mode
		break;
	   default:
	   printf("checkModeL2: L2 modecode %x not found.\n",modecode);
           return 1;	   
   } 
 return ret;
}
/*------------------------------------------------------setBoardL1()
 *  Set HW for different submodes. For submodes see checkModeL1()
 */
int setBoardL2(int n,int *boards,w32 modecode,w32 submode){
 int ret=0;
 switch(modecode){
  case 0x104: //outgen mode
       if(submode == 1){
         // Interface board test
         // enable ctp busy on busy board
         vmew32(0x80d8, 0x0);
         // disbale ddl
         vmew32(0xc14c, 0x0);
       }
       HardWare.loop=0;
       break;
  case 0x12: // L2 p/f mode
	  if(submode == 6)ret= setallPF(boards[0]);
	  break;
  default:HardWare.loop=0;break;
 }
 return ret;
}
/*-------------------------------------------------------initMode()
 * Initialization of the submode for items which should be done 
 * outside the loop, if any. For submodes see checkModeL0().
 */
int initModeL2(int n,int *boards,w32 modecode,w32 submode){
 switch(modecode){
  case 0x2: // L0 output monitor mode
            break;
 }
 return 0;
}
/*--------------------------------------------------------writeBoardsL2()
*/ 
int writeBoardsL2(int n, int *boards, w32 modecode,w32 submode){
 int i;
 static int il0=0;
 switch(modecode){
  case 0x104:   // outgen mode
     if(submode == 0){ 
      // L0connections
      //l0clstt,l0clstt[1..6]	  
      for(i=1;i<5;i++)writeSPP(boards[0],0,i,"100000");
     } else if(submode == 1){
      // Interface board test
      // It assumes that l2 ssm is already written by hardware
     }
     break;
  case 0x20c:  // L0 input generator mode
     // Write 0 to all channels on generating board
     writeSPn(boards[0],0,1,0);
     // Signal in ith input signal
     writeSPb(boards[0],0,70,il0+8);
     il0++;
  break; 
  default:
    printf("writeBoardsL2: L2 modecode %i not found.\n",modecode);
  return 1;  
 }
 for(i=1;i<n;i++)writeSPn(boards[i],0,1,0);   //Write all 0 to receiving boards
 if(!DEBFLG)for(i=0;i<n;i++)writeSSM(boards[i]);// Write ssm[board[i]].sm to hardware
return 0;	
}

