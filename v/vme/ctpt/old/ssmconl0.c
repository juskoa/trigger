#include "ssmconnection.h"
#define L0INP 24
//------------------------------------------------------
//  Hardware settings
//------------------------------------------------------
// L0 trigger inputs
static int l0input[L0INP][50]; 
// L0 invertion (classes 45-50)
static int l0invert[L0INP][50];   // only last 6 , others = 1
// L0 function -> goes to l0 inputs
static int l0function[50];
// L0 rnd trigger
static int l0rndtrig[50];
// L0 scale down
static int l0scldown[50];
// Cluster code
static int l0clst[50];
// BC mask
static int l0bcmask[4][50];
// p/f protection
static int l0pfprot[4][50];
// all/rare input
static int l0allrare[50];
// class mask
static int l0clmask[50];
// l0 function look up tables
static int l0funLT[16][2];
//--------------------------------------------------------------
//    Interaction and p/f protection
//
// Interaction 1/2 look up tables
static int INT1[16],INT2[16];
// P/F settings
static int THa1[5],THa2[5],THb1[5],THb2[5],dTa[5],dTb[5];
// PreScale factors
static int psfA[5],psfB[5];
//Look up tables
static int outP[8][5],INTa[4],INTb[4],delayedINTLT[4];
static int delayedINT;
// Working file
// 1..4 - pf1..pf4
// 10   - int1
// 11   - int2
// 12   - inta
// 13   - intb
static int IntPF[Mega];
/*--------------------------------------------------------readHW()
*/
int readHW(int board){
 int j;
 w32 word,i;
 w32 L0_condition=0x100;        // Pedja convention
 w32 L0_veto=0x140;
 w32 L0_invert=0x1c0;
 L0_condition=4*L0_condition;   // Anton convention
 L0_veto=4*L0_veto;
 L0_invert=4*L0_invert;
 for(i=0;i<50;i++){
  // L0 condition word
  word=vmer32(L0_condition+i+BSP*ctpboards[board].dial);
  for(j=0;j<L0INP;j++){
     l0input[j][i]=bit(word,j);//26=24+2+...
  }
  l0function[i]=bit(word,24)+2*bit(word,25);
  l0rndtrig[i]=bit(word,26)+2*bit(word,27);
  l0scldown[i]=bit(word,28)+2*bit(word,29);
  // L0 veto word
  word=vmer32(L0_veto+i+BSP*ctpboards[board].dial);
  l0clst[i]=bit(word,0)+2*bit(word,1)+4*bit(word,2);
  for(j=0;j<4;j++){
    l0bcmask[j][i]=bit(word,4+j);
    l0pfprot[j][i]=bit(word,8+j);    
  }
  l0allrare[i]=bit(word,12);
  l0clmask[i]=bit(word,16);
 }
 // L0_invert
 for(i=0;i<44;i++)for(j=0;j<L0INP;j++)l0invert[j][i]=0;
 for(i=44;i<50;i++){
  word=vmer32(L0_invert+i+BSP*ctpboards[board].dial);
  for(j=0;j<L0INP;j++)l0invert[j][i]=bit(word,j);
 }
 return 0;
}
/*---------------------------------------------------------readHWPF()
 * Read INT/PF settings
 */
int readHWINTPF(){
 return 0;
}
/*---------------------------------------------------------checkModeL0()
*/ 
int checkModeL0(int n, int *boards,w32 modecode,w32 submode){
 int ret=0;
 switch(modecode){
	   case 0x204:  // L0 connections		   
		L0connect(n,boards);break;
	   case 0x20c:  // L0 input generator
	        L0ingen(n,boards);break;	
	   case 0x2:  // L0 output monitor mode
		if(submode == 1) ret=testDelayedINT(boards[0]);
		else if(submode == 2) ret=testPF(boards[0]);
		else if(submode == 3) ret=testPF(boards[0]);
		break;
	   default:
	   printf("checkModeL0: L0 modecode %x not found.\n",modecode);
           return 1;	   
   } 
 return ret;
}
/*------------------------------------------------------setBoardL0()
 */
int setBoardL0(int n,int *boards,w32 modecode,w32 submode){
 switch(modecode){
  case 0x2: // L0 output monitor mode delayed int check
	  if(submode == 1)settestDelayedINT(boards[0]);
	  else if(submode == 2)settestPF(boards[0]);
	  else if(submode == 3)HardWare.loop=0;
	  else HardWare.loop=0;
	  break;
  default:HardWare.loop=0;break;
 }
 return 0;
}
/*------------------------------------------------------L0connect()
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
*/
int L0ingen(int n,int *boards){
 int i,offset1,offset2;
 Signal *si;
 w32 *sm= (w32 *) malloc(Mega*sizeof(w32));
 readHW(boards[0]);
 genINT();
 genPF();
 for(i=1;i<n;i++){
  if((si=findSignal(boards[i],25,"CluL02=l0clst[1]"))){
   genl0cluster(boards[0],si,sm);
   offset1=syncSIG1(sms[boards[i]].sm,si->channel);
   if(offset1<0) continue;
   offset2=syncSIG1(sm,0);
   compSIG3(sm,0,boards[i],si->channel,offset1-offset2,0);
  }
 }
 free(sm);
 return 0;
}
/*------------------------------------------------------------l0clst()
 * Emulate cluster signal generation
*/ 
int genl0cluster(int board0, Signal *si,w32 *sm){
 int i,j=0,trig,l0class,l0cluster;
 int l0active[50]; // list of trigger classes in this cluster
 Signal *sg;
 l0cluster=si->signamenum-24;
 for(i=0;i<50;i++)l0active[i]=-1;
 // Find all classes which gives this cluster
 for(i=0;i<50;i++)if(l0clst[i] == l0cluster)l0active[j++]=i;
 if(l0active[0]>=0){
   //loop over sm	 
   for(i=0;i<Mega;i++){
    sm[i]=0;
    // OR of classes in cluster
    //loop over active triggers for this cluster	   
    j=0;
    while((l0class=l0active[j++])>=0){
     // AND of inputs including invertion
     if(!(trig=inputsAND(i,l0class,board0))) continue;
     // l0 function
     if(!(trig=trig*functionL0(i,l0class,board0))) continue;
     // BUSY
     sg=findSignal(board0,l0cluster+11,"CLuBSYx");
     if(!(trig=trig*bit(sms[board0].sm[i],sg->channel))) continue;
     // p/f protection 
     trig=trig*getPaFu(i,l0class);	  
     sm[i]=sm[i]+trig;
    }
    if(sm[i])sm[i]=1;
   } 
 }
return 0;
}
/*----------------------------------------------------------
 *
*/
int inputsAND(int ism,int l0class,int board0){
 int k,trig=1;
 Signal *sg;
 // Loop over input signals	    	    
 for(k=0;k<L0INP;k++){
   if(l0input[k][l0class]){	     
     // AND of input signals , inclusing invertion	    
     sg=findSignal(board0,k+44,"l0k");
     trig=trig*(bit(sms[board0].sm[ism],sg->channel)-l0invert[k][l0class]);
   }
 }
 return trig;
}
/*--------------------------------------------------------getpf()
 * calculate p/f for l0class at ism position in sm memory
*/
int getPaFu(int ism,int l0class){
 int i,pf=1;
 // Loop over pf/ protection circuits
 for(i=0;i<5;i++)if(l0pfprot[i][l0class])pf=pf*bit(IntPF[ism],i);
 return pf;
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
/*--------------------------------------------------------genPF()
 * Generate P/F vetos in file IntPF
*/
int genPF(int board0){
 // How does the initial state of SM influences pf ?
 // Musim zacat generovat signal, az ked si budem isty, ze uz prijimam 
 int i,j,num;
 int intcounta=0,intcountb=0,imema=0,imemb=0,nint;
 int PA1,PA2,PB1,PB2,P1,P2;
 int dualmemA[5][256]; 
 int dualmemB[5][256]; 
 for(i=0;i<5;i++){
    for(j=0;j<256;j++)dualmemA[i][j]=0;
    for(j=0;j<256;j++)dualmemB[i][j]=0;
 }
 for(i=delayedINT;i<Mega;i++){
  if(bit(IntPF[i],12))intcounta++;
  if(bit(IntPF[i],13))intcountb++;
    for(j=0;j<5;j++){
      PA1=0;PA2=0;	    
      if(i % psfA[j]){
        nint=dualmemA[imema]-dualmemA[(imema-dTa[j])%256];
        dualmemA[j][(++imema)%256]=intcounta;
      }
      if(nint>THa1[j])PA1=1;
      if(nint>THa2[j])PA2=1;
       
      PB1=0;PB2=0;	    
      if(i % psfB[j]){
	nint=dualmemB[imemb]-dualmemB[(imemb-dTb[j])%256];
	dualmemB[j][(++imemb)%256]=intcountb;
      }
      if(nint>THb1[j])PB1=1;
      if(nint>THb2[j])PB2=1;

      P1=PA1 || PB1;
      P2=PA2 || PB2;

      num=P1+2*P2+4*bit(IntPF[i-delayedINT],14);
      IntPF[i]=IntPF[i]+(1<<j)*outP[num][j];
       
    }
 }
 return 1;
}
/*--------------------------------------------------------genINT()
 * Generate interaction signal
*/
int genINT(int board0){
 int i,j,num,int1,int2;
 Signal *sg;
 for(j=0;j<Mega;j++){
   num=0;
   for(i=0;i<4;i++){
     sg=findSignal(board0,i+44,"l0i");
     num=num+(1<<i)*bit(sms[board0].sm[j],sg->channel);    
   }
   int1=INT1[num];int2=INT2[num];
   // INT1,INT2
   IntPF[j]=IntPF[j]+ int1*(1<<10);   
   IntPF[j]=IntPF[j]+ int2*(1<<11); 
   // INTa,INTb
   IntPF[j]=IntPF[j]+ INTa[int1+2*int2]*(1<<12);
   IntPF[j]=IntPF[j]+ INTb[int1+2*int2]*(1<<13);
   // DelayedINT
   IntPF[j]=IntPF[j]+ delayedINTLT[int1+2*int2]*(1<<14);
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
/*-----------------------------------------------------------testDelayedINT()
 * Seeting HW for testDelayedINT
*/ 
int settestDelayedINT(int board){
 w32 bb,intsel,pfcommon;
 static w32 int12=3,lut=0x0,delay=10;  
 //HardWare.loop=0;
 
 printf("Hardware: delay= %i lut=%x \n",delay,lut); 
 bb=BSP*ctpboards[board].dial;
 // BC scale
 vmew32(SCALED_1,0xa);
 // Int1/2/T look up tables
 //vmew32(L0_INTERACT1,0x0);
 // Int 1/2 selection
 // 0   1   2   3      4    5   6   7     8    9
 // LUT BC1 BC2 RND1   RND2 LUT BC1 BC2   RND1 RND2
 HardWare.int1=bit(int12,0);
 HardWare.int2=bit(int12,1); 
 intsel = (1<<1)*HardWare.int1+(1<<6)*HardWare.int2;
 
 vmew32(L0_INTERACTSEL,intsel);
 // Set P look up table to select only DelayedINT
 vmew32(bb+PFBLOCK_A+(HardWare.ipf*3-1)*4,0xf0);        
 //pfcommon=0x400+(i<<12); 
 pfcommon=(lut<<8) + (delay<<12);          //delay
 vmew32(bb+PF_COMMON,pfcommon);
 HardWare.delayedINTlut=lut;
 HardWare.delayINT=delay;
 //delay=(delay+7) % (4*1024);
 lut=(lut+1); // % 16;
 if(lut > 16) HardWare.loop=0;
 return 1;
}
/*------------------------------------------------------------testDelayedINT()
 * Testing the Delayed INT look up table.
*/
int testDelayedINT(int board){
 int ism,ii,ret=0;
 int itest;
 Signal *spf1,*sint1,*sint2;
 itest=NCTPBOARDS+4;
 if(!sms[itest].sm)sms[itest].sm = (w32 *) malloc(Mega*sizeof(w32));

 //spf1=findSignalS(board,0,"pf1");
 //printf("pf1 num=%i \n",spf1->signamenum);
 spf1=findSignal(board,68+HardWare.ipf,"pfx");
 sint1=findSignalS(board,0,"Int1");
 sint2=findSignalS(board,0,"Int2");

 for(ism=0;ism<HardWare.delayINT+1;ism++)sms[itest].sm[ism]=0;;
 for(ism=HardWare.delayINT+1;ism<Mega;ism++){
  // Look-up table: DelayedINT
  ii=bit(sms[board].sm[ism-HardWare.delayINT-1],sint1->channel)+
     bit(sms[board].sm[ism-HardWare.delayINT-1],sint2->channel)*2;
  ii=1<<ii;
  sms[itest].sm[ism]=((HardWare.delayedINTlut&ii)==ii);
 }
 ii=syncSIG1(sms[itest].sm,0);
 ret=compSIG(itest,0,board,spf1->channel,0,ii);
 return ret;
}
/*--------------------------------------------------------------settestPF()
 * Setting HardWare for testPF
*/
int settestPF(int board){
 int ret=0;
 w32 bb,intsel,pfcommon,pfba,pflut;
 static w32 ii=0x2; 
 
 bb=BSP*ctpboards[board].dial;
 // Set Bc1
 vmew32(SCALED_1,HardWare.sbc1);
 // Set Rnd1 max=0x7fffffff
 vmew32(RANDOM_1,0x7fffffff/HardWare.sbc1);
 vmew32(RANDOM_2,0x7fffffff/(HardWare.sbc1+1));
 // Set Int1 and Int2 to BC1 or RND1
 //intsel = (1<<1)+(1<<6);     // SBC only
 //intsel=(1<<3) + (1<<9);     // RND only
 intsel=(1<<3) + (1<<6);       // Mixed
 vmew32(L0_INTERACTSEL,intsel);
 // Set  INTa   INTb        INTd
 pfcommon=HardWare.luta + (HardWare.lutb<<4)+ (HardWare.delayedINTlut<<8);
 vmew32(bb+PF_COMMON,pfcommon);vmew32(bb+PF_COMMON,pfcommon);
 // Set P look up table (PFLUT word) to select only INTa 
 //HardWare.lut12D=ii;
 pflut=HardWare.lut12D+(HardWare.scaleA<<8)+(HardWare.scaleB<<13);
 vmew32(bb+PFBLOCK_A+(HardWare.ipf*3-1)*4,pflut);    
 // Set thresholds A
 pfba=HardWare.THa1+(HardWare.THa2<<6)+(HardWare.deltaTa<<12);
 pfba=pfba+(HardWare.delayA<<20)+(HardWare.nodelayAf<<31);
 vmew32(bb+PFBLOCK_A+(HardWare.ipf-1)*4,pfba);
 // Set thresholds B
 //pfba=(63)+(63<<6)+(0x1)*(1<<12);
 pfba=HardWare.THb1+(HardWare.THb2<<6)+(HardWare.deltaTb<<12);
 //pfba=pfba+(HardWare.delayB<<20)+(HardWare.nodelayBf<<31); 
 vmew32(bb+PFBLOCK_A+(HardWare.ipf+1-1)*4,pfba);
 printf("ii=%x %i\n",ii,HardWare.loop); 
 //if(ii>=0xff)
 HardWare.loop=0;
 //ii++;
 return ret;
}
/*--------------------------------------------------------------testPF()
 * Testing branch a of PF
 * There are two modes in PF protectoion:
 * Nodelay flag = 0 -> testPFnodel0()
 * Nodelay flag = 1 -> testPFnodel1()
 * At L0 board there is only mode 0 available
 */
int testPF(int board){
 w32 scale;
 if(HardWare.scaleA > HardWare.scaleB) scale=HardWare.scaleA;
 else scale=HardWare.scaleB;
 return testPFnodel1(board,scale);
}
/*--------------------------------------------------------------testPFnodel1()
 * Nodelay flag=1 -> no delay available
 * pf signal imediately follows inta 
 * used on L0 board
 */ 
int testPFnodel1(int board,w32 scale){
 static FILE *ff=NULL;
 int ism,ismascale,iconf,ismbscale;
 int itest,idel,ret=1;
 Signal *spf1,*sint1,*sint2;
 w32 dpma[256],dpmb[256],deldpm[10];
 w32 inta,intb,counta,countb,intd;
 w32 pa1,pa2,pb1,pb2,p1,p2,pp;

 //if(!ff)ff=fopen("work.txt","w");
 itest=NCTPBOARDS+4;
 if(!sms[itest].sm)sms[itest].sm = (w32 *) malloc(Mega*sizeof(w32));

 for(ism=0;ism<256;ism++){dpma[ism]=0;dpmb[ism]=0;}
 //spf1=findSignalS(board,0,"pf1");
 //printf("pf1 num=%i \n",spf1->signamenum);
 spf1=findSignal(board,68+HardWare.ipf,"pfx");
 sint1=findSignalS(board,0,"Int1");
 sint2=findSignalS(board,0,"Int2");
 
 iconf=0;
 while(ret && iconf<(scale+1))
 {
 ism=0;
 inta=0;intb=0;intd=0; 
 ismascale=0;ismbscale=0;
 counta=0;countb=0;
 while(ism < Mega){
 
  // Write pf to ssm 	 
  sms[itest].sm[ism]=pp;
  
  // Block a 
  if((counta-dpma[(ismascale-HardWare.deltaTa-1)%256]) > HardWare.THa1)pa1=1;
  else pa1=0;
  if((counta-dpma[(ismascale-HardWare.deltaTa-1)%256]) > HardWare.THa2)pa2=1;
  else pa2=0;  
  if(!((ism+iconf)%(HardWare.scaleA+1))){
    dpma[ismascale%256]=counta;    
    ismascale++;
  }
  // Block b 
  //sms[itest].sm[ism]=pa1;
  if((countb-dpmb[(ismbscale-HardWare.deltaTb-1)%256]) > HardWare.THb1)pb1=1;
  else pb1=0;
  if((countb-dpmb[(ismbscale-HardWare.deltaTb-1)%256]) > HardWare.THb2)pb2=1;
  else pb2=0;  
  if(!((ism+iconf)%(HardWare.scaleB+1))){
    dpmb[ismbscale%256]=countb;    
    ismbscale++;  
  }
  // OR for P1 and P2
  //if((pb1+pb2))printf("ism,pa1,pb1,pa2,pb2 %i %i %i %i %i\n",ism,pa1,pb1,pa2,pb2);
  p1 = pa1 || pb1;
  p2 = pa2 || pb2;	  
  //pp = p1;
  pp=lookup8(p1,p2,deldpm[(ism-1)%10],HardWare.lut12D);
  //printf("p1,p2,,intd,pp %i %i %i %i\n",p1,p2,intd,pp);
  // Look-up table P1,P2,Delayed INT
  
  // BC clock
  ism++;
  // Look-up table INTa
  inta=lookup4(bit(sms[board].sm[ism],sint1->channel),
	       bit(sms[board].sm[ism],sint2->channel),
	       HardWare.luta);
  counta=counta+inta;
  // Look-up table INTb
  intb=lookup4(bit(sms[board].sm[ism],sint1->channel),
	       bit(sms[board].sm[ism],sint2->channel),
	       HardWare.lutb);
  countb=countb+intb;
  // Look-up table Dealyed INT
  // (For the L0 case delay=0, for L1 and L2 boards delay has to be added!)
  intd=lookup4(bit(sms[board].sm[ism],sint1->channel),
	       bit(sms[board].sm[ism],sint2->channel),
	       HardWare.delayedINTlut);
  deldpm[(ism)%10]=intd;
 }
 idel=(scale+1)*256;
 ret=compSIG3(sms[itest].sm,0,board,spf1->channel,0,idel);
 iconf++;
 } 
 if(!ret) printf("testPF: SUCCES \n");
 else printf("testPF: FAIL \n");
 /*fprintf(ff,"%x %i \n",HardWare.lut12D,ret);
 if(HardWare.lut12D>=0xff){
	 fclose(ff);
	 exit(1);
 }*/
 sms[board].offset=idel;
 sms[itest].offset=idel;
 return ret;
}
/*--------------------------------------------------------------testPFnodel0()
 * nodelayflag =0
 * delays allowed - will be used at L1 and L2 boards
 */ 
int testPFnodel0(int board,w32 scale,w32 delay){
 int ism,ismscale,iconf,ret=1;
 int itest,idel;
 Signal *spf1,*sint1,*sint2;
 w32 dpm[256];
 w32 inta,counta=0,pf;
 
 itest=NCTPBOARDS+4;
 if(!sms[itest].sm)sms[itest].sm = (w32 *) malloc(Mega*sizeof(w32));

 //spf1=findSignalS(board,0,"pf1");
 //printf("pf1 num=%i \n",spf1->signamenum);
 spf1=findSignal(board,68+HardWare.ipf,"pfx");
 sint1=findSignalS(board,0,"Int1");
 sint2=findSignalS(board,0,"Int2");
 
 iconf=0;
 while(ret && iconf<(scale+1))
 {
 ism=0;
 inta=0; 
 ismscale=0;
 pf=0;
 idel=1+delay*(scale+1);
 while((ism+idel) < Mega){  
  sms[itest].sm[ism+idel]=pf;
  if(!((ism+iconf)%(scale+1))){
    dpm[ismscale%256]=counta;    
    if((counta-dpm[(ismscale-HardWare.deltaTa-1)%256]) > HardWare.THa1)pf=1;
    else pf=0;
    ismscale++;
  }
  ism++;
  // Look-up table INTa
  inta=lookup4(bit(sms[board].sm[ism],sint1->channel),
	       bit(sms[board].sm[ism],sint2->channel),
	       HardWare.luta);
  counta=counta+inta;
 }
 //     fill the dpm         +  delay
 idel=256*(scale+1)+  delay*(scale+1)+1;
 ret=compSIG3(sms[itest].sm,0,board,spf1->channel,0,idel);
 iconf++;
 } 
 if(!ret) printf("testPF: SUCCES \n");
 else printf("testPF: FAIL \n");
 sms[board].offset=idel;
 sms[itest].offset=idel;
 //pat=getPatfromF(sms[board].sm,spf1->channel,20);
 //pat=getPatfromF(sms[itest].sm,0,20);
 return ret;
}
/*--------------------------------------------------------------testPF()
 * Testing branch a of PF
 * 
int testPFnodel1(int board){
 int ism,ii,ret=0;
 int itest;
 Signal *spf1,*sint1,*sint2;
 w32 dpm[256];
 w32 inta,counta,pf;
 
 itest=NCTPBOARDS+4;
 if(!sms[itest].sm)sms[itest].sm = (w32 *) malloc(Mega*sizeof(w32));

 //spf1=findSignalS(board,0,"pf1");
 //printf("pf1 num=%i \n",spf1->signamenum);
 spf1=findSignal(board,68+HardWare.ipf,"pfx");
 sint1=findSignalS(board,0,"Int1");
 sint2=findSignalS(board,0,"Int2");
 
 ism=0;
 inta=0; 
 counta=0;
 pf=0;
 while(ism < Mega){
    // Write to dpm at old address
    dpm[ism%256]=counta;      
    // Write to ssm at old address    
    sms[itest].sm[ism]=pf;    
    //read dpm at old address and calculate new pf
    if((counta-dpm[(ism-HardWare.deltaTa-1)%256]) > HardWare.THa1)pf=1;
    else pf=0;
    // increase bc and address
    ism++;
    // Look-up table: DelayedINT
    ii=bit(sms[board].sm[ism],sint1->channel)+
       bit(sms[board].sm[ism],sint2->channel)*2;
    ii=1<<ii;
    // update inta and counta
    inta=((HardWare.luta&ii)==ii);
    counta=counta+inta;    
 }
 ret=compSIG3(sms[itest].sm,0,board,spf1->channel,0,256+2); 
 sms[board].offset=256+2;
 sms[itest].offset=256+2;
 return ret;
}
*/
