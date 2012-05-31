/* This version of l0 class logic uses the list of active classes
 * This saves memory and time but is not very sekfexplanatory.
 * The following version will use [NCLASS][NCLUSTER] array
 */ 

#include "ssmconnection.h"
#define L0INP 24
#define NCLASS 50
#define NCLINV 44
#define NCLU 6
#define CTPDT 8 
#define SSMSTART 5 
//------------------------------------------------------
//  Hardware settings
//------------------------------------------------------
// L0 trigger inputs
int l0input[L0INP][NCLASS]; 
// L0 invertion (classes 45-NCLASS)
int l0invert[NCLASS];   // only last 6 , others = 1
// L0 function -> goes to l0 inputs
int l0function[NCLASS];
// L0 rnd trigger
int l0rndtrig[NCLASS];
// L0 scale down
int l0scldown[NCLASS];
// Cluster code
int l0clst[NCLASS];
// BC mask
int l0bcmask[NCLASS];
// p/f protection
int l0pfprot[NCLASS];
// all/rare input
int l0allrare[NCLASS];
// class mask
int l0clmask[NCLASS];
// l0 function look up tables
int l0funLT[16][2];
//--------------------------------------------------------------
w32 IntPF[Mega];
/*FGROUP DebCon-------------------------------------------------------readHW()
 * Reads Class L0 trigger definition from hardware.
*/
int readHWL0class(int board){
 int i,j;
 w32 word;
 for(i=0;i<NCLASS;i++){
  // L0 condition word
  word=vmer32(L0_CONDITION+4*i+4);
  for(j=0;j<L0INP;j++){
     l0input[j][i]=bit(word,j);
  }
  l0function[i]=bit(word,24)+2*bit(word,25);
  l0rndtrig[i]=bit(word,26)+2*bit(word,27);
  l0scldown[i]=bit(word,28)+2*bit(word,29);
  // L0 veto word
  word=vmer32(L0_VETO+4*i+4);
  l0clst[i]=bit(word,0)+2*bit(word,1)+4*bit(word,2);
  l0bcmask[i]=(word&0xf0)>>4;
  l0pfprot[i]=(word&0xf00)>>8;    
  l0allrare[i]=bit(word,12);
  word=vmer32(L0_MASK+4*i+4);
  l0clmask[i]=bit(word,0);
 }
 // L0_invert
 for(i=0;i<NCLINV;i++)for(j=0;j<L0INP;j++)l0invert[i]=0;
 for(i=NCLINV;i<NCLASS;i++){
  word=vmer32(L0_INVERT+4*i+4);
  l0invert[i]=word;
 }
 return 0;
}
/*FGROUP DebCon-------------------------------------------printHWL0class
 * Print L0 class HW settings
*/ 
void printHWL0class(int board){
 int i,j;
 printf("L0 inputs (bin): \n"); 
 for(j=0;j<L0INP;j++){
   printf("INP%2i: ",j);
   for(i=0;i<NCLASS;i++)printf("%1i",l0input[j][i]);
   printf("\n");
 }
 printf("L0 function (hex): \n");
 for(i=0;i<NCLASS;i++) printf("%1x",l0function[i]);
 printf("\n");
 printf("L0 rnd trig (hex): \n");
 for(i=0;i<NCLASS;i++) printf("%1x",l0rndtrig[i]);
 printf("\n");
 printf("L0 scaled BC (hex): \n");
 for(i=0;i<NCLASS;i++) printf("%1x",l0scldown[i]);
 printf("\n");
 printf("L0 cluster code (hex): \n");
 for(i=0;i<NCLASS;i++) printf("%1x",l0clst[i]);
 printf("\n"); 
 printf("L0 BC mask (hex): \n");
 for(i=0;i<NCLASS;i++) printf("%1x",l0bcmask[i]);
 printf("\n"); 
 printf("L0 P/F (hex): \n");
 for(i=0;i<NCLASS;i++) printf("%1x",l0pfprot[i]);
 printf("\n");
 printf("L0 all/rare (bin): \n");
 for(i=0;i<NCLASS;i++) printf("%1i",l0allrare[i]);
 printf("\n");
 printf("L0 class mask (bin): \n");
 for(i=0;i<NCLASS;i++) printf("%1i",l0clmask[i]);
 printf("\n");
 printf("L0 class invert (6hex): \n");
 for(i=NCLINV;i<NCLASS;i++) printf("%6x ",l0invert[i]);
 printf("\n");
}
/*------------------------------------------------------L0connect()
 * Testing the connection between the L0 board and other boards.
 */ 
int L0connect(int n, int *boards){
 int i,error,offset;
 Signal *sg,*si;
 char *pat;
 for(i=0;i<n;i++){
   while(si){
    // Find if si signal exist in generating board
    sg=findSignal(boards[0],si->signamenum,si->signame);
    if(sg){     	     
     pat=getPatfromF(sms[boards[0]].sm,sg->channel,sg->patlen);
     if(pat)offset=syncSIG2(boards[i],si->channel,pat);
     free(pat);
     if(offset<0){error++;continue;}
     error=error+compSIG(boards[0],sg->channel,boards[i],si->channel,offset,0);   
    }else {
      warnmess("L0connect",boards[i],si->signame," not in table");  
    }
    si=si->next;	 
   }
 }
 return error;
}
/*--------------------------------------------------------L0ingen()
 * Cluster generation in ingen mode
*/
int L0ingen(int n,int *boards){
 int i,offset1,offset2;
 Signal *si;
 w32 *sm= (w32 *) malloc(Mega*sizeof(w32));
 readHWL0class(boards[0]);
 //genINT();
 //calcPF();
 for(i=1;i<n;i++){
  if((si=findSignal(boards[i],25,"l0clst[1]"))){
   //genl0cluster(boards[0],si,sm);
   offset1=syncSIG1(sms[boards[i]].sm,si->channel);
   if(offset1<0) continue;
   offset2=syncSIG1(sm,0);
   compSIG3(sm,0,boards[i],si->channel,offset1-offset2,0);
  }
 }
 free(sm);
 return 0;
}
/*FGROUP DebCon---------------------------------------------------------L0outmon
 * Cluster emulation in outmon mode
*/
int L0classoutmonT(int board,int cptdt,int l0strobe,int chan){
 int i;
 w32 mask;
 List *l0active[NCLU];
 if(!sms[TEST].sm)sms[TEST].sm=IntPF;
 readHWL0class(board);
 for(i=0;i<NCLU;i++)l0active[i]= NULL;
 // Find all classes which gives this cluster,check mask
 for(i=0;i<NCLASS;i++)
    if((l0clst[i] > 0) && (!l0clmask[i]))l0active[l0clst[i]-1]=addnum(l0active[l0clst[i]-1],i);
 for(i=0;i<NCLU;i++){
  printf("CLUSTER %i classes: \n",i);
  printlist(l0active[i]);
 }
 genl0clustDT(cptdt,l0strobe,l0active,board,sms[TEST].sm,&i);
 mask=1<<chan;
 compSIG5(sms[TEST].sm,1,mask,0,SSMSTART);
 compSIG3(sms[TEST].sm,chan,1,chan,0,SSMSTART);
 sms[TEST].syncflag=1;
 sms[TEST].offset=SSMSTART;
 sms[board].offset=SSMSTART;
 // free classes
 for(i=0;i<NCLU;i++)l0active[i]=freenums(l0active[i]); 
 return 0;
}
/*---------------------------------------------------------L0outmon
 * Cluster emulation in outmon mode
*/
int L0classoutmon(int board){
 int i,dif,start;
 List *l0active[NCLU];
 w32 mask;
 if(!sms[TEST].sm)sms[TEST].sm=IntPF;
 readHWL0class(board);
 for(i=0;i<NCLU;i++)l0active[i]= NULL;
 // Find all classes which gives this cluster,check mask
 for(i=0;i<NCLASS;i++)
    if((l0clst[i] > 0) && (!l0clmask[i]))l0active[l0clst[i]-1]=addnum(l0active[l0clst[i]-1],i);
 for(i=0;i<NCLU;i++){
  printf("CLUSTER %i classes: \n",i);
  printlist(l0active[i]);
 }
 mask=0x200; // l0data
 mask=0x400000fc; //deadtime
 i=0;
 dif=1;
 while(dif && (i<=CTPDT)){
    printf("DT=%i L0s=%i:\n",i,0);
    genl0clustDT(i,0,l0active,board,sms[TEST].sm,&start);
    dif=compSIG5(sms[TEST].sm,1,mask,0,start);
    i++;
 }
 // l0strobe=1 case
 if(dif){
  printf("DT=%i L0s=%i:\n",8,1);
  genl0clustDT(8,1,l0active,board,sms[TEST].sm,&start);
  dif=compSIG5(sms[TEST].sm,1,mask,0,start);
 }
 sms[TEST].syncflag=1;
 sms[TEST].offset=start;
 sms[board].offset=start;
 // free classes
 for(i=0;i<NCLU;i++)l0active[i]=freenums(l0active[i]); 
 return 0;
}
/*---------------------------------------------------------genl0clustDT()
 * Generate L0 clusters for givan state of initial deadtime
*/ 
int genl0clustDT(int ctpbusy,int l0strobe,List *l0active[],int board,w32 *sm,int *start){
 int i,j,l0cluster,l0class,trig,trigclu;
 int smclst,sml0s,smdt,smdata;
 w32 l0data[NCLASS+1],il0data;
 List *active;
 for(i=0;i<NCLASS+1;i++)l0data[i]=0; 
 findchans(board,&smclst,&sml0s,&smdt,&smdata);
 i=SSMSTART; 
 *start=0;
 il0data=0;
 while(i<Mega){
   //L0 strobe up one BC after l0clst
   if(l0strobe){
    // comparison can start only after first l0strobe or NCLASS after 0
    if(!(*start))*start=i;
    sm[i]=wbit(sm[i],1,sml0s);
    l0strobe=0;
    //l0data
    il0data=0;
      printf("%i %i %i\n",i,il0data,smdata);
      printf("%i \n",l0data[NCLASS-il0data]);
    sm[i]=wbit(sm[i],l0data[NCLASS-il0data++],smdata);
   } else {
    sm[i]=wbit(sm[i],0,sml0s);
    //l0data
    if(il0data<NCLASS+1){
      printf("%i %i %i\n",i,il0data,smdata);
      printf("%i \n",l0data[NCLASS-il0data]);
      sm[i]=wbit(sm[i],l0data[NCLASS-il0data],smdata);
      il0data++;
    }
   }   
   if(!ctpbusy){
     for(j=0;j<NCLASS+1;j++)l0data[j]=0; 
     // Loop over clusters
     for(l0cluster=0;l0cluster<NCLU;l0cluster++){
      // OR of classes in cluster
      //loop over active triggers for this cluster	   
      trigclu=0;
      active=NULL;
      if(l0active[l0cluster])active=l0active[l0cluster]->first;
      while(active){
        l0class=active->intnum;
        active=active->next;
	//l0data[l0class]=1;
        // AND of inputs including invertion
        if(!(trig=inputsANDom(board,l0class,i-1))) continue;
        // l0 function
        // if(!(trig=trig*functionL0(i,l0class,board))) continue;
        // BUSY
        //sg=findSignal(board,l0cluster+11,"CLuBSYx");
        //if(!(trig=trig*bit(sms[board].sm[i],sg->channel))) continue;
        // p/f protection	  
        if(!getPaFu(board,l0class,i-1)) continue;
        trigclu=trigclu+trig;
	l0data[l0class]=1;
      }
      if(trigclu){
	 sm[i]=wbit(sm[i],1,l0cluster+smclst);  //l0clst
	 l0strobe=1;
         ctpbusy=CTPDT;	 
      }else{
	 sm[i]=wbit(sm[i],0,l0cluster+smclst);
     }
    }
    // Dead Time up same BC as l0clst
    if(l0strobe)sm[i]=wbit(sm[i],1,smdt); 
    else sm[i]=wbit(sm[i],0,smdt); 
   }else{
      // l0clst
      for(l0cluster=0;l0cluster<NCLU;l0cluster++)sm[i]=wbit(sm[i],0,l0cluster+smclst);
     ctpbusy--;
     if(ctpbusy) sm[i]=wbit(sm[i],1,smdt);
     else  sm[i]=wbit(sm[i],0,smdt);    
   }
   i++;
 }   
 if((*start>NCLASS) || (*start==0))*start=NCLASS;
 return 0;
}
/*---------------------------------------------------------
 *
 */
int findchans(int board,int *smclst,int *sml0s,int *smdt,int *smdata){
 /*
 Signal *sig;
 sig=findSignalS(board,0,"l0clst1");
 *smclst=sig->channel;
 sig=findSignalS(board,0,"l0strobe");
 *sml0s=sig->channel;
 sig=findSignalS(board,0,"deadtime");
 *smdt=sig->channel;
 */
 *smclst=2;*sml0s=8;*smdt=30;*smdata=9;
 printf("findchans: l0clst1=%i; l0strobe=%i; deadtime=%i \n",*smclst,*sml0s,*smdt);
 return 0;
}
/*---------------------------------------------------------getPaFu()
 * P/F protection for fiven class 'l0class'
 * P/F read from real data.
 */ 
int getPaFu(int board,int l0class,int ibc){
 int pf,i,pafu;
 pf=l0pfprot[l0class];
 pafu=1;
 for(i=0;i<4;i++){
  if(!(pf & (1<<i)))pafu=pafu*bit(sms[board].sm[ibc],14+i);
 }
 //printf("%i %i %i %i \n",ibc,pf,l0class,pafu);
 return pafu;
}
/*---------------------------------------------------------inputANDom()
 * inputs AND in output mon mode
 * input works like (selection | input)
 */
int inputsANDom(int board,int l0class,int ism){
 int trig,i;
 trig=1;
 // Loop over input signals
 for(i=0;i<L0INP;i++)if(!l0input[i][l0class]) return 0;
 for(i=0;i<2;i++){ 
  if(!(l0scldown[l0class] & (1<<i))) trig = trig*bit(sms[board].sm[ism],23+i);
  if(!(l0rndtrig[l0class] & (1<<i))) trig = trig*bit(sms[board].sm[ism],25+i);
 }
 //if(l0class==1)printf("trig: %i %i \n",ism,trig);
 return trig;
}
/*----------------------------------------------------------
 * for ingen mode
*/
int inputsAND(int board0,int l0class,int ism){
 int k,trig=1;
 Signal *sg;
 // Loop over input signals	    	    
 for(k=0;k<L0INP;k++){
   if(l0input[k][l0class]){	     
     // AND of input signals , inclusing invertion	    
     sg=findSignal(board0,k+44,"l0k");
     //trig=trig*(bit(sms[board0].sm[ism],sg->channel)-l0invert[k][l0class]);
   }
 }
 return trig;
}
/*--------------------------------------------------------functionL0()
 * calculate L0 function  of first 4 input signals 
 * wrt to look up table
 * Inputs: board0 - generating board
 *         ism    - position in sm memory
 *         if0    - number of l0 function - there are two of them
 */ 
int functionL0(int ism,int l0class,int board0){
 int i,num=0;
 Signal *sg;
 if(l0function[l0class] == 3) return 1;
 for(i=0;i<4;i++){
  sg=findSignal(board0,i+44,"l0i");
  num=num+(1<<i)*bit(sms[board0].sm[ism],sg->channel);
 }
 if(l0function[l0class] == 0)return (l0funLT[num][0])*l0funLT[num][1];
 if(l0function[l0class] == 1)return l0funLT[num][0];
 if(l0function[l0class] == 2)return l0funLT[num][1];
 return -1;
}
//--------------------------------------------------------------------
//
// Steering routines
// 
/*---------------------------------------------------------checkModeL0()
 * mode=0x2
 * submode = 1: testing cluster generation on outmon
 * submode = 2: testing P/F. Input of HW settings from python standard 
 *              interface.
 * submode = 3: testing P/F. None input of HW. HW settings read from HW.
 * submode = 4: testing P/F in loop. HW settings set by ranlux. Stops when
 *              error detected.
 * submode = 5: testing P/F in loop. Like submode=4, but before writing 
 *              HW settings to HW , the emulation calculates result and 
 *              throw it away, if trivial. (Trivial is all 0 or all 1.)  
 * submode = 6: testing all 5 P/F circuits;  
*/ 
int checkModeL0(int n, int *boards,w32 modecode,w32 submode){
 int ret=0;
 //printf("checkModeL0: modecode,submode %x %x \n",modecode,submode);
 switch(modecode){
	   case 0x204:  // L0 connections		   
		L0connect(n,boards);break;
	   case 0x20c:  // L0 input generator
	        L0ingen(n,boards);break;	
	   case 0x2:  // L0 output monitor mode
		if(submode == 1) ret = L0classoutmon(boards[0]);
		else if(submode == 2) ret=calcPF(boards[0],HardWare.ipf);
		else if(submode == 3) ret=calcPF(boards[0],HardWare.ipf);
		else if(submode == 4) ret=calcPF(boards[0],HardWare.ipf);
		else if(submode == 5) ret=calcPF(boards[0],HardWare.ipf);
		else if(submode == 6) ret=testallPF(boards[0]);
		break;
	   default:
	   printf("checkModeL0: L0 modecode %x not found.\n",modecode);
           return 1;	   
   } 
 return ret;
}
/*------------------------------------------------------setBoardL0()
 *  Set HW for different submodes. For submodes see checkModeL0()
 */
int setBoardL0(int n,int *boards,w32 modecode,w32 submode){
 int ret=0;
 switch(modecode){
  case 0x2: // L0 output monitor mode delayed int check
	  if(submode == 2)ret=settestPF(boards[0],HardWare.ipf);
	  else if(submode == 3)HardWare.loop=0;
	  else if(submode == 4)ret=setrndPF(boards[0],HardWare.ipf);
	  else if(submode == 5)ret=setrndPF2(boards[0]);
	  else if(submode == 6)ret=setallPF(boards[0]);
	  else HardWare.loop=0;
	  break;
  default:HardWare.loop=0;break;
 }
 return ret;
}
/*-------------------------------------------------------initMode()
 * Initialization of the submode for items which should be done 
 * outside the loop, if any. For submodes see checkModeL0().
 */
int initModeL0(int n,int *boards,w32 modecode,w32 submode){
 switch(modecode){
  case 0x2: // L0 output monitor mode
 }
 return 0;
}

/*--------------------------------------------------------writeBoardsL0()
*/ 
int writeBoardsL0(int n, int *boards, w32 modecode){
 int i;
 static int il0=0;
 switch(modecode){
  case 0x204: // L0connections
     //l0clstt,l0clstt[1..6]	  
     for(i=1;i<8;i++)writeSPP(boards[0],0,i,"100000");
     //pp 
     writeSPP(boards[0],0,10,"100000");
     break;
  case 0x20c:  // L0 input generator mode
     // Write 0 to all channels on generating board
     writeSPn(boards[0],0,1,0);
     // Signal in ith input signal
     writeSPb(boards[0],0,70,il0+8);
     il0++;
  break; 
  default:
    printf("writeBoardsL0: L0 modecode %i not found.\n",modecode);
  return 1;  
 }
 for(i=1;i<n;i++)writeSPn(boards[i],0,1,0);   //Write all 0 to receiving boards
 if(!DEBFLG)for(i=0;i<n;i++)writeSSM(boards[i]);// Write ssm[board[i]].sm to hardware
return 0;	
}
//***********************************************************************
//  L0 Output Monitoring mode:
//  - branch of testing using only inputs which board can generate iself,
//    i.e. BC downscale, rnd trigger
//***********************************************************************
/*--------------------------------------------------------------settestPF()
 * Setting HardWare for caclPF
*/
int settestPF(int board,int ipf){
 int ret=0;
 //static w32 ii=0x2; 
 
 // Set Bc1
 //vmew32(SCALED_1,HardWare.sbc1);
 // Set Rnd1 max=0x7fffffff
 //vmew32(RANDOM_1,0x7fffffff/HardWare.sbc1);
 //vmew32(RANDOM_2,0x7fffffff/(HardWare.sbc1+1));
 // Set Int1 and Int2 to BC1 or RND1
 //intsel = (1<<1)+(1<<6);     // SBC only
 //intsel=(1<<3) + (1<<9);     // RND only
 //intsel=(1<<3) + (1<<6);       // Mixed
 //vmew32(L0_INTERACTSEL,intsel);
 // Set  INTa   INTb        INTd
 setPFCOMMON(board);
 // Set Block A 
 setPFBLOCKA(board,ipf);   
 // Set Block B
 setPFBLOCKB(board,ipf);   
 //Set PFLUT
 setPFLUT(board,ipf);
 //printf("ii=%x %i\n",ii,HardWare.loop); 
 //if(ii>=0xff)
 //HardWare.loop=0;
 //ii++;
 return ret;
}
/*--------------------------------------------------------------setrndPF()
 * Setting HardWare for random testing 
*/
int setrndPF(int board,int ipf){
 int ret=0;
 static int ii=0;
 
 // Set Bc1
 //vmew32(SCALED_1,HardWare.sbc1);
 // Set Rnd1 max=0x7fffffff
 //vmew32(RANDOM_1,0x7fffffff/HardWare.sbc1);
 //vmew32(RANDOM_2,0x7fffffff/(HardWare.sbc1+1));
 // Set Int1 and Int2 to BC1 or RND1
 //intsel = (1<<1)+(1<<6);     // SBC only
 //intsel=(1<<3) + (1<<9);     // RND only
 //intsel=(1<<3) + (1<<6);       // Mixed
 //vmew32(L0_INTERACTSEL,intsel);
 // Set  INTa   INTb        INTd
 HardWare.luta=0xf*rnlx();
 HardWare.lutb=0xf*rnlx();
 HardWare.delayedINTlut=0xf*rnlx();
 setPFCOMMON(board);
 // Set thresholds A
 HardWare.THa1=64*rnlx();
 HardWare.THa2=64*rnlx();
 HardWare.deltaTa=1+255*rnlx();
 HardWare.delayA=0;
 HardWare.nodelayAf=1;
 setPFBLOCKA(board,ipf);
 // Set thresholds B
 HardWare.THb1=64*rnlx();//HardWare.THa1;
 HardWare.THb2=64*rnlx();//HardWare.THa2;
 HardWare.deltaTb=1+255*rnlx();//HardWare.deltaTa;
 HardWare.delayB=0;
 HardWare.nodelayBf=1;
 setPFBLOCKB(board,ipf);
 // Set P look up table (PFLUT word) to select only INTa 
 HardWare.lut12D=0xff*rnlx();
 HardWare.scaleA=32*rnlx();
 HardWare.scaleB=32*rnlx();
 setPFLUT(board,ipf);  
 printf(" %i  \n",++ii);
 //printPFHW(); 
 //if(ii>=3)HardWare.loop=0;
 //ii++;
 return ret;
}
/*--------------------------------------------------------------setrndPF2()
 * Setting HardWare for random testing 
 * changing settings until it finds nontrivial configuration
*/
int setrndPF2(int board){
 static int isuc=0,iall=0;
 setrndPF(board,1);
 while( abs(PFcircuit(board,1,0,0,11,12)-Mega/2) > (Mega/2-3)){
	 setrndPF(board,1);iall++;
 }
 isuc++;iall++;
 printPFHW(); 
 printf("all=%i succ=%i \n",iall,isuc);
return 0;
}
/*--------------------------------------------------------------setallPF
 */
int setallPF(int board){
 int i;
 setrndPF(board,1);
 for(i=2;i<6;i++)settestPF(board,i);
 return 0;
}
int testallPF(int board){
 int i,ret=0;
 for(i=1;i<6;i++)ret=ret+calcPF(board,i);
 return ret;
}
/*--------------------------------------------------------------caclPF()
 * Test of FULL P/F.
 */
int calcPFall(int board){
 int i,ret=0;
 for(i=1;i<6;i++){
  getPFHW(board,i);
  ret=ret+calcPF(board,i);
 }
 return ret;
}
/*--------------------------------------------------------------calcPFnodel1()
 * Nodelay flag=1 -> no delay available
 * pf signal imediately follows inta 
 * used on L0 board
 */ 
int calcPF(int board,int ipf){
 int ret0=Mega,ret=1;
 int start,w;
 int scalea,scaleb;
 int amin,bmin;
 w32 scale;
 Signal *spf1,*sint1,*sint2;

 //if(!sms[TEST].sm)sms[TEST].sm = (w32 *) malloc(Mega*sizeof(w32));
 if(!sms[TEST].sm)sms[TEST].sm=IntPF;
 //spf1=findSignalS(board,0,"pf1");
 spf1=findSignal(board,68+ipf,"pfx");
 sint1=findSignalS(board,0,"Int1");
 sint2=findSignalS(board,0,"Int2");

 if(HardWare.scaleA > HardWare.scaleB) scale=HardWare.scaleA;
 else scale=HardWare.scaleB;
 
 start=(scale+1)*256+256;
 scaleb=0;
 while((ret) && scaleb<(HardWare.scaleB+1)){
 scalea=0;
   while((ret) && scalea<(HardWare.scaleA+1)){
     w = PFcircuit(board,ipf,scalea,scaleb,sint1->channel,sint2->channel);
     printf("%i %i %i",scalea,scaleb,w);
     //ret=compSIG4(sms[TEST].sm,0,board,spf1->channel,0,start,0);
     ret=compSIG3(sms[TEST].sm,ipf-1,board,spf1->channel,0,start);
     if(ret < ret0){ amin=scalea;bmin=scaleb;ret0=ret;}
     scalea++;
   } 
   scaleb++;
 }
 if(!ret) printf("testPF: SUCCES \n");
 else{
   printf("testPF: FAIL amin=%i bmin=%i \n",amin,bmin);
   PFcircuit(board,ipf,amin,bmin,sint1->channel,sint2->channel);
   ret=compSIG4(sms[TEST].sm,0,board,spf1->channel,0,start,1);
   HardWare.loop=0;
   printPFHW(); 
   return ret;
 }
 //sms[board].offset=start;
 //sms[TEST].offset=start;
 return ret;
}
/* 
 * There are two modes in PF protectoion:
 * Nodelay flag = 0 -> calcPFnodel0()
 * Nodelay flag = 1 -> calcPFnodel1()
 * At L0 board there is only mode 0 available
*/

