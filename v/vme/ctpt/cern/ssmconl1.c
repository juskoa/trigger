#include "ssmconnection.h"
static int *l1input[L1INP];
static int *l1pfprot;
static int *l1clst;
static int *l1roiv;
/*---------------------------------------------------------setlinksl1()
*/
void setlinksl1(){
 int i;
 for(i=0;i<L1INP;i++)l1input[i]=HardWare.l1input[i];
 l1pfprot=HardWare.l1pfprot;
 l1clst=HardWare.l1clst;
 l1roiv=HardWare.l1roiv;
}
/*FGROUP DebCon---------------------------------------------------------readHWL1class
*/
int readHWL1class(){
 int i,j;
 w32 word;
 if(!l1pfprot)setlinksl1();
 for(i=0;i<NCLASS;i++){
  word=vmer32(L1_DEFINITION+4*i+4);
  for(j=0;j<L1INP;j++)l1input[j][i]=bit(word,j);
  l1pfprot[i]=(word&0xf000000)>>24;
  l1clst[i]=(word&0x30000000)>>28;
  l1roiv[i]=bit(word,31);
 }
 return 0;
}
/*FGROUP DebCon--------------------------------------------------------printHWL1class
*/
void printHWL1class(){
 int i,j;
 if(!l1pfprot){
   printf("printHWL1class: HW not read! \n");
   return ;
 }
 printf("L1 inputs (bin): \n"); 
 for(j=0;j<L1INP;j++){
   printf("INP%2i: ",j);
   for(i=0;i<NCLASS;i++)printf("%1i",l1input[j][i]);
   printf("\n");
 }
}
/*---------------------------------------------------------checkModeL1()
 * mode=0x2
*/ 
int checkModeL1(int n, int *boards,w32 modecode,w32 submode){
 int ret=0;
 //printf("checkModeL0: modecode,submode %x %x \n",modecode,submode);
 switch(modecode){
	   case 0x104:  // L1 output generator mode
		if(submode == 0){
		// L1 connections		   
		Connect(n,boards);break;
                }else if(submode == 1){
                // L2 class logic test
                }else{
      		 printf("checkModeL1: unknown submode %i \n",submode);
      		 return 2;	 
		}
                break;
	   case 0x20c:  // L1 input generator
	        if(submode == 1)
                // Testing L1 class logic using only L1-L2 board
	        l1class(boards);
		break;	
	   case 0x2:  // L1 output monitor mode
		 if(submode == 1)
		 // Testing PF on L1 board
		 ret=calcPFL1a(boards[0],HardWare.ipf);
		 else if(submode == 6)
		 // Testinf all PF by rnd
		 ret=testallPFL12(boards[0]);
		break;
	   case 0x00a:  // L0 input monitor mode
		break;
	   default:
	   printf("checkModeL1: L1 modecode %x not found.\n",modecode);
           return 1;	   
   } 
 return ret;
}
/*------------------------------------------------------setBoardL1()
 *  Set HW for different submodes. For submodes see checkModeL1()
 */
int setBoardL1(int n,int *boards,w32 modecode,w32 submode){
 int ret=0;
 switch(modecode){
  case 0x104: // L1 output generator mode
	  HardWare.loop=0;
	  break;
  case 0x2:
	  if(submode == 1) HardWare.loop=0;
	  else if(submode == 6)ret= setallPF(boards[0]);
	  break;
  default:HardWare.loop=0;break;
 }
 return ret;
}
/*-------------------------------------------------------initMode()
 * Initialization of the submode for items which should be done 
 * outside the loop, if any. For submodes see checkModeL0().
 */
int initModeL1(int n,int *boards,w32 modecode,w32 submode){
 switch(modecode){
  case 0x2: // L0 output monitor mode
            break;
 }
 return 0;
}
/*--------------------------------------------------------writeBoardsL1()
*/ 
int writeBoardsL1(int n, int *boards, w32 modecode,w32 submode){
 int i;
 switch(modecode){
  case 0x104:    // Output generator mode
     if(submode == 0){
     // L1 connections	  
     for(i=1;i<11;i++)writeSPP(boards[0],0,i,"100000");
     } else if(submode == 1){
      // L2 class logic test 
      writeSPn(boards[0],0,1,0);   // zero everywhere
      // l1 strobe
      writeSPP(boards[0],0,8,"10000000000000000000000");
      // l1 data
      writeSPP(boards[0],0,9,"000000000000e1000000000");
     }else{
      printf("writeBoardsL1: unknown submode %i \n",submode);
      return 2;
     }
     break;
  case 0x20c:  // L1 input generator mode
     if(submode == 1){
     // Write to l0strobe,l0data,l0input
     writeSPn(boards[0],0,1,0);
     writeSPP(boards[0],0,3,"100000000000000000000000000000000000000000000000000000000000000");
     writeSPP(boards[0],0,4,"000000000000400000000000000000000000000000000000000000000000000");
     //writeSPP(boards[0],0,4,"e0000000000000000000000");
     // L1 1 signal
     //writeSPP(boards[0],0,8,"fffffffffffffffffffffff0000000000000000000000000000000000000000");
     //writeSPP(boards[0],0,8,"ffffff000000000000000000000000000000000000000000000000000000000");
     //writeSPP(boards[0],0,8,"00000000000000fffffffff0000000000000000000000000000000000000000");
     //writeSPP(boards[0],0,8,"00000000000000ffff000000000000000000000000000000000000000000000");
     //writeSPP(boards[0],0,8,"00000000000000ff00000000000000000000000000000000000000000000000");
     //writeSPP(boards[0],0,8,"00000000000000f000000000000000000000000000000000000000000000000");
     //                        123456789012345678901234567890123456789012345678901234567890123
     writeSPP(boards[0],0,8,"000000000000010000000000000000000000000000000000000000000000000");
       break;
     }else{
      printf("writeBoardsL1: unknown submode %i \n",submode);
      return 2;       
     }
  break; 
  default:
    printf("writeBoardsL1: L1 modecode %i not found.\n",modecode);
  return 1;  
 }
 for(i=1;i<n;i++)writeSPn(boards[i],0,1,0);   //Write all 0 to receiving boards
 if(!DEBFLG)for(i=0;i<n;i++)writeSSM(boards[i]);// Write ssm[board[i]].sm to hardware
return 0;	
}
/*------------------------------------------------------L1connect()
 * Testing the connection between the L1 board and other boards.
 * Common routine Connect() in ssmconfo.c
 */ 
/*--------------------------------------------------------------calcPFL1a()
 * P/F testing on L1
 * Everything from single ssm
 */ 
int calcPFL1a(int board1,int ipf){
 int ret0=Mega,ret=1;
 int start,w;
 int scalea,scaleb;
 int amin,bmin;
 w32 scale;
 Signal *spf1,*sint1,*sint2,*sint3;
 int ic=0;

 //if(!sms[TEST].sm)sms[TEST].sm = (w32 *) malloc(Mega*sizeof(w32));
 if(!sms[TEST].sm)sms[TEST].sm=TestSSM;
 //spf1=findSignalS(board,0,"pf1");
 // pf1 .. pft = (69 .. 73)
 spf1=findSignal(board1,68+ipf,"pfx");
 sint1=findSignalS(board1,0,"int_a");
 sint2=findSignalS(board1,0,"int_b");
 sint3=findSignalS(board1,0,"int_d");

 if(HardWare.scaleA > HardWare.scaleB) scale=HardWare.scaleA;
 else scale=HardWare.scaleB;
 
 start=(scale+1)*2048+256; // To fill full 
 printf("calcPFL1a: start=%i \n",start);
 scaleb=0;
 while((ret) && scaleb<(HardWare.scaleB+1)){
 scalea=0;
   while((ret) && scalea<(HardWare.scaleA+1)){
     w = PFcircuit2(board1,ipf,scalea,scaleb,sint1->channel,sint2->channel,sint3->channel,0,ic);
     //w = PFcircuit1(board1,ipf,scalea,scaleb,sint1->channel,sint2->channel,sint3->channel); // old version 
     printf("scalea scaleb w %i %i %i \n",scalea,scaleb,w);
     ret=compSIG3(sms[board1].sm,spf1->channel,TEST,5*ic,0,start);
     //ret=compSIG3(sms[TEST].sm,ipf-1,board2,spf1->channel,offset,start);
     //ret=compSIG3(sms[TEST].sm,5*ic,board2,spf1->channel,offset,start);
     //ic++;
     if(ret < ret0){ amin=scalea;bmin=scaleb;ret0=ret;}
     scalea++;
   } 
   scaleb++;
 }
 sms[board1].offset=start;
 sms[TEST].offset=start;
 if(!ret) printf("testPF: SUCCES \n");
 else{
   printf("testPF: FAIL amin=%i bmin=%i \n",amin,bmin);
   //PFcircuit(board1,ipf,amin,bmin,sint1->channel,sint2->channel);
   //ret=compSIG4(sms[TEST].sm,0,board2,spf1->channel,0,start,1);
   //HardWare.loop=0;
   //printPFHW(); 
   return ret;
 }
 return ret;
}
/*---------------------------------------------------------------testallPFL12()
 * Testing all PF circuit on L0/L1 board
 * Different from L0 board since there are different signals 
 */ 
int testallPFL12(int board){
 int i,ret=0;
 for(i=1;i<6;i++)ret=ret+calcPFL1a(board,i);
 return ret;
}
/*----------------------------------------------------------------l1class()
 * Testing l1 class logic
*/
int l1class(int *boards){
 // check in l2 inmon
 Signal *sl0strobe,*sl0data;
 int i,j;
 int l1classes[NCLASS+1];
 if(!sms[TEST].sm)sms[TEST].sm=TestSSM;
 sl0strobe=findSignalS(boards[0],0,"l0strobe");
 sl0data=findSignalS(boards[0],0,"l0data");
 printf("l1class: channels l0strobe,l0data: %i %i \n",sl0strobe->channel,sl0data->channel);
 i=0;
 while(i<Mega){
 // L0 strobe
  if(bit(sms[boards[0]].sm[i],sl0strobe->channel)){
  // L0 data
   j=0;
   while(j<(NCLASS+1) && (i+j)< Mega){
    l1classes[j]=bit(sms[boards[0]].sm[i],sl0data->channel);
    // not checking for l0 strobe closer than 51
    j++;
    }
  }
  i++;
 }
 return 0;
}

