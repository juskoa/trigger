#include "ssmconnection.h"
#define NCLINV 44
#define NCLU 6
#define NCOUNTERs 160
#define CTPDT 8 
#define SSMSTART 5 
//------------------------------------------------------
//  Hardware settings
//------------------------------------------------------
// L0 trigger inputs
static int *l0input[L0INP]; 
// L0 invertion (classes 45-NCLASS)
static int *l0invert;   // only last 6 , others = 1
// L0 function -> goes to l0 inputs
static int *l0function;
// L0 rnd trigger
static int *l0rndtrig;
// L0 scale down
static int *l0scldown;
// Cluster code
static int *l0clst;
// BC mask
static int *l0bcmask;
// p/f protection
static int *l0pfprot;
// all/rare input
static int *l0allrare;
// class mask
static int *l0clmask;
// l0 function look up tables
static int l0funLT[16][2];
// counters
static unsigned long int l0counter[160];
//rate
static w32 l0rate[NCLASS];
//--------------------------------------------------------------
/*-------------------------------------------------------------------setlinks()
 *  Set the links between the global variable HardWare and local variables
 *  for L0 classes hardware.
 */
void setlinksl0(){
 int i;
 for(i=0;i<L0INP;i++)l0input[i] = HardWare.l0input[i];
 l0function=HardWare.l0function;
 l0invert=HardWare.l0invert;
 l0rndtrig=HardWare.l0rndtrig;
 l0scldown=HardWare.l0scldown;
 l0bcmask=HardWare.l0bcmask;
 l0pfprot=HardWare.l0pfprot;
 l0allrare=HardWare.l0allrare;
 l0clmask=HardWare.l0clmask;
 l0clst=HardWare.l0clst;
}
/*FGROUP DebCon-------------------------------------------------readL0count()
 *  1= BUSY
 *  0= ok
 *  redaHW =1 : reads Counters
 *          0 : reads only memory
 */
int readL0count(int board,int readHW){
 static int flag=1;
 int i;
 if(flag){
  //flag=0;
  for(i=0;i<NCOUNTERs;i++)l0counter[i]=0;
 }
 if(readHW){
   vmew32(COPYCOUNT,0x0);
   usleep(8000);
   if(vmer32(COPYBUSY)){
   printf("readL0count: copybusy after 8000.");
   return 1;
   }
 }
 vmew32(COPYCLEARADD,0x0);
 for(i=0;i<NCOUNTERs;i++)l0counter[i]=vmer32(COPYREAD)-l0counter[i];
 return 0;
}
/*FGROUP DebCon------------------------------------------------printL0count()
 */
void printL0count(int board){
 int i,j,nw;
 nw=5;
 printf("L0 counters:\n");
 for(i=0;i<(NCOUNTERs/nw);i++){
  printf("%i - %i:",i*nw,nw*(i+1)-1);
  for(j=0;j<nw;j++)printf(" %lu",l0counter[j+i*nw]);
  printf("\n");
 }
}
/*FGROUP DebCon------------------------------------------readHWrate()
 * Reads down sacle factors for every class
 */
void readHWrate(int board){
 int i;
 vmew32(RATE_MODE,1);
 vmew32(RATE_CLEARADD,0x0);
 for(i=0;i<NCLASS;i++)l0rate[i]=vmer32(RATE_DATA);
 vmew32(RATE_MODE,0);
}
/*FGROUP DebCon------------------------------------------printHWrate()
 * Prints presacling factors.
 */ 
void printHWrate(int boards){
 int i;
 printf("Pre scaling factors (hex): \n");
 for(i=0;i<NCLASS;i++){
  printf("%x ",l0rate[i]);
  if(!((i+1) % 10))printf("\n");
 }
 printf("\n");
}
/*FGROUP DebCon------------------------------------------setRate()
 *  Set rate for class i. 1st class: i=0; 
 *  Writes to cpu memory.
 */
void setRate(int class,w32 rate){
 l0rate[class]=rate;
}
/*FGROUP DebCon------------------------------------------writeHWrate()
 * Writes pre scaling factors to board
 */
void writeHWrate(int board){
 int i;
 vmew32(RATE_MODE,1);
 vmew32(RATE_CLEARADD,0x0);
 for(i=0;i<NCLASS;i++)vmew32(RATE_DATA,l0rate[i]);
 vmew32(RATE_MODE,0);
}
/*-------------------------------------------------------writeHWL0class()
 * Reads Class L0 trigger definition from hardware.
 * not finished
*/
int writeHWL0class(int board){
 int i;
 w32 word;
 for(i=0;i<NCLASS;i++){
  // L0 condition word
  word=65*rnlx();
  word=word<<24;
  vmew32(L0_CONDITION+4*i+4,word);
  // L0 veto word
  word=17*rnlx();
  word=(word<<4)+7*rnlx();
  vmew32(L0_VETO+4*i+4,word);
  if(rnlx()>0.5)word=1;else word=0;
  vmew32(L0_MASK+4*i+4,word);
 }
 // L0_invert
 //for(i=NCLINV;i<NCLASS;i++){
 // l0invert[i]=word;
 // word=vmer32(L0_INVERT+4*i+4);
 //}
 return 0;
}
/*FGROUP DebCon-------------------------------------------------------readHW()
 * Reads Class L0 trigger definition from hardware.
*/
int readHWL0class(int board){
 int i,j;
 w32 word;
 if(!l0clmask)setlinksl0();
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
  l0pfprot[i]=(word&0xf0)>>4;
  l0bcmask[i]=(word&0xf00)>>8;    
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
 if(!l0clmask){
   printf("printHWL0class: HW not read! \n");
   return ;
 }
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
/*FGROUP DebCon------------------------------------------------------testRate()
 */
int testRate(int board,int overflow,int rate,int downscale){
 lint maxc,max,maxd;
 lint time,l0b28,l0a28,l28rate;
 lint Time,expl0b,expl0a,exprate;
 double ds,ratio1,ratio2,ratio3;
 maxc=0x100000000;
 max=((1<<21)-1);
 maxd=max*50;
 readL0count(board,0); 
 ds= ((double)(downscale))/max;
 time=l0counter[13];
 l28rate=l0counter[14];             //fast
 l0b28=l0counter[43];               //fast
 l0a28=l0counter[127];              //slow
 printf("time,l28rate,l28before,l28after: %llu %llu %llu %llu\n",time,l28rate,l0b28,l0a28);
/*
 rest1=0;
 rest2=0;
 expl0b=0;
 expl0a=0;
 exprate=0;
 for(i=0;i<overflow;i++){
    expl0b=(expl0b+(16*maxc/rate)) % (maxc);
    rest1=rest1+((16*maxc) % rate);
    
    expl0a=(expl0a + (16*maxc/maxd)*50*(max-downscale)) % (maxc);
    rest2=rest2+((16*maxc) % maxd);
    
    exprate= (exprate+ (16*maxc/maxd)*50*(max-downscale)) % (maxc);
    rest3=rest3 + ((16*maxc) % maxd);
 }
 expl0b= (expl0b + ((rest1+16*time)/rate)) % maxc;
 expl0a= (expl0a + rest2+16*time) % maxc;
 exprate = exprate + 
*/
 Time = 16*overflow*maxc+16*time; 
 expl0b=(Time/rate) % maxc;
 expl0a=((lint)((double)(Time)*(1.-ds)/rate)) % maxc;
 exprate=((lint)((double)(Time)*ds));
 exprate=(exprate/16) % maxc;
 
 ratio1=((double)(expl0b))/l0b28; 
 ratio2=((double)(expl0a))/l0a28; 
 ratio3=((double)(exprate))/l28rate; 
 printf("Expected number of l0b,l0a, rate:%llu %llu %llu\n",expl0b,expl0a,exprate);
 printf("RATIOS: l0b/l0be l0a/l0ae rate/ratee: %f %f %f\n",ratio1,ratio2,ratio3);
 
 // Tu treba testovat velky overflow
 //
 expl0a=((double)(Time)*(1.-ds)/rate) / maxc;
 expl0a= expl0a*maxc+ l0a28;
 expl0b=Time/rate/maxc;
 expl0b=expl0b*maxc+l0b28;
 exprate=((lint)((double)(Time)*ds));
 exprate=(exprate/16) / maxc;
 exprate=exprate*maxc+l28rate;
 ratio1=((double)(expl0a))/expl0b; 
 ratio2=((double)(exprate))/Time*16; 
 printf("RATIOS II: l0ac/l0bc ratec/Time: %g %g \n",ratio1,ratio2);
 printf("RATIOS II: l0ac/l0bc ratec/Time: %g %g \n",1.-ds,ds);
 
 return 0;
}
/*------------------------------------------------------L0connect()
 * Testing the connection between the L0 board and other boards. 
int L0connect(int n, int *boards){
 int i,error=0,offset;
 Signal *sg,*si;
 int ii=0;
 char *pat;
 for(i=1;i<n;i++){
   si=sms[boards[i]].signal->first;  
   while(si && ii++<33){
    // Find if si signal exist in generating board
    sg=findSignal(boards[0],si->signamenum,si->signame);
    if(sg){     	     
     //  Try to find pattern
     pat=getPatfromF(sms[boards[0]].sm,sg->channel,sg->patlen);
     printf("pat=%s \n",pat);
     if(pat)offset=syncSIG2(boards[i],si->channel,pat);
     free(pat);
     
     // Assumes : test pattern starts with 1 !!!
     //offset=syncSIG1(sms[boards[i]].sm,si->channel);
     if(offset<0){error++;si=si->next;continue;}
     error=error+compSIG(boards[0],sg->channel,boards[i],si->channel,offset,0); 
     //error=error+compSIG1(boards[0],sg->channel,boards[i],si->channel,offset,0); 
    }else {
      //warnmess("L0connect",boards[i],si->signame," not in table");  
    }
    //printf("si= %s \n",si->signame);    
    si=si->next;
   }
 }
 return error;
}
*/
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
 * What is this ?
*/
int L0classoutmonT(int board,int cptdt,int l0strobe,int chan){
 int i;
 w32 mask;
 int l0classes[NCLU][NCLASS];
 if(!sms[TEST].sm)sms[TEST].sm=TestSSM;
 readHWL0class(board);
 findactivel0(l0classes);
 mask=0x200; // l0data
 mask=0x400000fc; //deadtime
 genl0clustDT(cptdt,l0strobe,l0classes,board,sms[TEST].sm,&i);
 mask=1<<chan;
 compSIG5(sms[TEST].sm,1,mask,0,SSMSTART);
 compSIG3(sms[TEST].sm,chan,1,chan,0,SSMSTART);
 sms[TEST].syncflag=1;
 sms[TEST].offset=SSMSTART;
 sms[board].offset=SSMSTART;
 return 0;
}
/*---------------------------------------------------------L0outmon
 * Cluster emulation in outmon mode
*/
int L0classoutmon(int board){
 int dif,start,dt;
 int l0classes[NCLU][NCLASS];
 w32 mask;
 if(!sms[TEST].sm)sms[TEST].sm=TestSSM;
 readHWL0class(board);
 findactivel0(l0classes);
 mask=0x200; // l0data
 mask=0x400000fc; //clusts+deadtime
 mask=0x400003fc; // clusts+l0strobe+l0data+dt
 dif=1;
 dt=0;
 while(dif && (dt <= CTPDT)){
    printf("DT=%i L0s=%i:\n",dt,0);
    genl0clustDT(dt,0,l0classes,board,sms[TEST].sm,&start);
    dif=compSIG5(sms[TEST].sm,1,mask,0,start);
    dt++;
 }
 // l0strobe=1 case
 if(dif){
  printf("DT=%i L0s=%i:\n",8,1);
  genl0clustDT(8,1,l0classes,board,sms[TEST].sm,&start);
  dif=compSIG5(sms[TEST].sm,1,mask,0,start);
 }
 sms[TEST].syncflag=1;
 sms[TEST].offset=start;
 sms[board].offset=start;
 return 0;
}
/*---------------------------------------------------------findactivel0
 * Find active l0 classes which may contribute to trigger
 */
int findactivel0(int l0classes[][NCLASS]){
 List *l0active[NCLU];
 int i,j;
 for(i=0;i<NCLU;i++)l0active[i]=NULL;
 for(i=0;i<NCLU;i++)for(j=0;j<NCLASS;j++)l0classes[i][j]=0;
 // Find all classes which gives this cluster,check mask
 for(i=0;i<NCLASS;i++)
    if((l0clst[i] > 0) && (!l0clmask[i])){
      l0classes[l0clst[i]-1][i]=1;
      l0active[l0clst[i]-1]=addnum(l0active[l0clst[i]-1],i); // Only for output
    }
 for(i=0;i<NCLU;i++){
  printf("CLUSTER %i classes: \n",i);
  printlist(l0active[i]);
 }
 // free classes
 for(i=0;i<NCLU;i++)l0active[i]=freenums(l0active[i]); 
 return 0;
}
/*---------------------------------------------------------genl0clustDT()
 * Generate L0 clusters for given state of initial deadtime
 * l0data: 50 bit long ; 0-49 classes + 50th test class
*/ 
int genl0clustDT(int ctpbusy,int l0strobe,int l0classes[][NCLASS],int board,w32 *sm,int *start){
 int ism,l0cluster,l0class,trig,trigclu;
 int smclst,sml0s,smdt,smdata;
 w32 l0data[NCLASS+1],l0dataS[NCLASS+1],il0data;
 int l0cla2clu[50];  
 findchans(board,&smclst,&sml0s,&smdt,&smdata);
 
 ism=SSMSTART; 
 *start=0;
 il0data=NCLASS+1; 
 l0data[NCLASS]=0;   //test class
 while(ism<Mega){
   //L0 strobe up one BC after l0clst
   if(l0strobe){
    // comparison can start only after first l0strobe or NCLASS after 0
    if(!(*start))*start=ism;
    sm[ism]=wbit(sm[ism],1,sml0s);
    l0strobe=0;
    //l0data
    il0data=0;
    //printf("%i %i %i\n",ism,il0data,smdata);
    //printf("%i \n",l0data[NCLASS-il0data]);
    for(l0class=0;l0class<NCLASS+1;l0class++)l0dataS[l0class]=l0data[l0class];
    sm[ism]=wbit(sm[ism],l0dataS[NCLASS-il0data++],smdata);
   } else {
    sm[ism]=wbit(sm[ism],0,sml0s);
    //l0data
    if(il0data<NCLASS+1){
      //printf("%i %i %i\n",ism,il0data,smdata);
      //printf("%i \n",l0data[NCLASS-il0data]);
      sm[ism]=wbit(sm[ism],l0dataS[NCLASS-il0data],smdata);
      il0data++;
    }else sm[ism]=wbit(sm[ism],0,smdata); 
   }   
   if(!ctpbusy){
     for(l0class=0;l0class<NCLASS;l0class++)l0data[l0class]=0;
     for(l0cluster=0;l0cluster<NCLU;l0cluster++){ 
       // Loop over classes	   
       trigclu=0;
       for(l0class=0;l0class<NCLASS;l0class++){
	 l0cla2clu[l0class]=0;      
         if(l0classes[l0cluster][l0class]){		
           // AND of inputs including invertion
           if(!(trig=inputsAND(board,l0class,ism-1))) continue;
           // l0 function
           // if(!(trig=trig*functionL0(ism,l0class,board))) continue;
           // BUSY
           //sg=findSignal(board,l0cluster+11,"CLuBSYx");
           //if(!(trig=trig*bit(sms[board].sm[i],sg->channel))) continue;
           // p/f protection	  
           if(!getPaFu(board,l0class,ism-1)) continue;
           trigclu=trigclu+trig;
	   l0cla2clu[l0class]=1;
	 }
       }
       if(trigclu){
	 sm[ism]=wbit(sm[ism],1,l0cluster+smclst);  //l0clst
	 l0strobe=1;
         ctpbusy=CTPDT;	 
	 // Add l0data
	 for(l0class=0;l0class<NCLASS;l0class++)l0data[l0class]=l0data[l0class]+l0cla2clu[l0class];
       }else{
	 sm[ism]=wbit(sm[ism],0,l0cluster+smclst);
       }
     }
     // Dead Time up same BC as l0clst
     if(l0strobe)sm[ism]=wbit(sm[ism],1,smdt); 
     else sm[ism]=wbit(sm[ism],0,smdt); 
   }else{
      // l0clst
      for(l0cluster=0;l0cluster<NCLU;l0cluster++)sm[ism]=wbit(sm[ism],0,l0cluster+smclst);
     ctpbusy--;
     if(ctpbusy) sm[ism]=wbit(sm[ism],1,smdt);
     else  sm[ism]=wbit(sm[ism],0,smdt);    
   }
   ism++;
 }   
 if(*start<NCLASS)*start=NCLASS + *start; else *start=NCLASS;
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
 * pafu=0 - class killed
 * Parameters:
 * board = 1 : L0 board , pf taken from 'real' ssm
 * board = TEST: emulated pf taken from sms[TEST] 
 * l0class 
 * ibc - position (time) in ssm
 */ 
int getPaFu(int board,int l0class,int ibc){
 int pf,i,pafu;
 pf=l0pfprot[l0class];
 pafu=1;
 for(i=0;i<4;i++){
  if(!(pf & (1<<i)))pafu=pafu*(!bit(sms[board].sm[ibc],14+i));
 }
 //printf("%i %i %i %i \n",ibc,pf,l0class,pafu);
 return pafu;
}
/*---------------------------------------------------------inputANDom()
 * inputs AND in output mon mode
 * input works like (selection | input)
 */
int inputsAND(int board,int l0class,int ism){
 int trig,i;
 trig=1;
 switch(board){
 case(1):  //L0
 // Loop over input signals
 for(i=0;i<L0INP;i++)if(!l0input[i][l0class]) return 0;
 for(i=0;i<2;i++){ 
  if(!(l0scldown[l0class] & (1<<i))) trig = trig*bit(sms[board].sm[ism],23+i);
  if(!(l0rndtrig[l0class] & (1<<i))) trig = trig*bit(sms[board].sm[ism],25+i);
 }
 //if(l0class==1)printf("trig: %i %i \n",ism,trig);
 return trig;
 case(TEST): // FO test should be changed
 // Loop over input signals	    	    
 for(i=0;i<L0INP;i++){
   if(l0input[i][l0class]){	     
     // AND of input signals , inclusing invertion	    
     //sg=findSignal(board,i+44,"l0k");
     //trig=trig*(bit(sms[board0].sm[ism],sg->channel)-l0invert[k][l0class]);
   }
 }
 return trig;
 }
 return -1;
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
 if(board == 1)HardWare.delayINT=0;
 else if(board == 2)HardWare.delayINT=512*rnlx();
 else HardWare.delayINT=D12*rnlx();
 setPFCOMMON(board);
 // Set thresholds A
 HardWare.THa1=64*rnlx();
 HardWare.THa2=64*rnlx();
 HardWare.deltaTa=1+255*rnlx();
 HardWare.delayA=0;
 if(board == 1)HardWare.nodelayAf=1;
 else if((HardWare.nodelayAf=2*rnlx()))HardWare.delayA=D11*rnlx();
 setPFBLOCKA(board,ipf);
 // Set thresholds B
 HardWare.THb1=64*rnlx();//HardWare.THa1;
 HardWare.THb2=64*rnlx();//HardWare.THa2;
 HardWare.deltaTb=1+255*rnlx();//HardWare.deltaTa;
 HardWare.delayB=0; 
 if(board == 1)HardWare.nodelayBf=1;
 else if((HardWare.nodelayBf=2*rnlx()))HardWare.delayB=D11*rnlx();
 setPFBLOCKB(board,ipf);
 // Set P look up table (PFLUT word) to select only INTa 
 HardWare.lut12D=0xff*rnlx();
 HardWare.scaleA=32*rnlx();
 HardWare.scaleB=32*rnlx();
 setPFLUT(board,ipf);  
 printf(" %i \n",++ii);
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
 for(i=1;i<6;i++)ret=ret+calcPF(board,board,i);
 return ret;
}
/*--------------------------------------------------------------caclPF()
 * Test of FULL P/F.
 */
int calcPFall(int board){
 int i,ret=0;
 for(i=1;i<6;i++){
  getPFHW(board,i);
  ret=ret+calcPF(board,board,i);
 }
 return ret;
}
/*--------------------------------------------------------------calcPF()
 * Nodelay flag=1 -> no delay available
 * pf signal imediately follows inta 
 * used on L0 board
 */ 
int calcPF(int board1,int board2,int ipf){
 int ret0=Mega,ret=1;
 int start,w;
 int scalea,scaleb;
 int amin,bmin;
 int start1,start2,offset;
 w32 scale;
 Signal *spf1,*sint1,*sint2;

 //if(!sms[TEST].sm)sms[TEST].sm = (w32 *) malloc(Mega*sizeof(w32));
 if(!sms[TEST].sm)sms[TEST].sm=TestSSM;
 //spf1=findSignalS(board,0,"pf1");
 // pf1 .. pft = (69 .. 73)
 spf1=findSignal(board2,68+ipf,"pfx");
 sint1=findSignalS(board1,0,"int1");
 sint2=findSignalS(board1,0,"int2");

 if(HardWare.scaleA > HardWare.scaleB) scale=HardWare.scaleA;
 else scale=HardWare.scaleB;
 
 start=(scale+1)*256+256;
 printf("calcPF: start=%i \n",start);
 scaleb=0;
 while((ret) && scaleb<(HardWare.scaleB+1)){
 scalea=0;
   while((ret) && scalea<(HardWare.scaleA+1)){
     w = PFcircuit(board1,ipf,scalea,scaleb,sint1->channel,sint2->channel);
     printf("scalea scaleb w %i %i %i \n",scalea,scaleb,w);
     //ret=compSIG4(sms[TEST].sm,0,board1,spf1->channel,0,start,0);
     if(board1 != board2){
      start1=syncSIG1a(sms[TEST].sm,0,0); //channel 0 assumed
      start2=syncSIG1a(sms[board2].sm,spf1->channel,0); 
      offset=start1-start2;
      sms[board2].offset=0;
      sms[TEST].offset=offset;
     } else offset=0;
     //printf("calcPF: offset= %i \n",offset); 
     ret=compSIG3(sms[board2].sm,spf1->channel,TEST,ipf-1,offset,start);
     if(ret < ret0){ amin=scalea;bmin=scaleb;ret0=ret;}
     scalea++;
   } 
   scaleb++;
 }
 if(!ret) printf("testPF: SUCCES \n");
 else{
   printf("testPF: FAIL amin=%i bmin=%i \n",amin,bmin);
   //PFcircuit(board1,ipf,amin,bmin,sint1->channel,sint2->channel);
   //ret=compSIG4(sms[TEST].sm,0,board2,spf1->channel,0,start,1);
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
//--------------------------------------------------------------------
//
// Steering routines
// 
int startBoardsL0(int n, int *boards,w32 modecode,w32 submode){
   int i,rc=0;
   printf("\n startBoardsL0: modecode, submode 0x%x,%i \n",modecode,submode);
   if((modecode == 0x104) && (submode == 4)){
    for(i=1;i<n;i++)rc=rc+startSSM1(boards[i]);
    if(rc) return 1;
    usleep(30000);
    return 0;
   }
   printf(" startSSM default \n ");
   if(setomSSM(boards[0],modecode)) return 1;
   if(n>1){
     if(startSSM(n,boards)){    // first board start last
       printf("startBoardsL0 error: SSMs have not started inside one ORBIT. Sync not possible \n");
      return 1;
     }
     usleep(30000);
     return 0;
  }else{
    printf("n=%i \n",n);
    if(startSSM1(boards[0])) return 1;
    usleep(30000);
    return 0;
  }
}
/*---------------------------------------------------------checkModeL0()
 * mode 0x104 outgen
   submode == 0: connections
   submode == 1: testing P/F on L1 board
   submode == 2: testing l2 class logic
   submode == 3: generation of int signals for test of int board
                 (not used, not finished)
 * mode=0x2 outmon
   submode = 1: testing cluster generation on outmon
   submode = 2: testing P/F. Input of HW settings from python standard 
                interface.
   submode = 3: testing P/F. None input of HW. HW settings read from HW.
   submode = 4: testing P/F in loop. HW settings set by ranlux. Stops when
                error detected.
   submode = 5: testing P/F in loop. Like submode=4, but before writing 
                HW settings to HW , the emulation calculates result and 
                throw it away, if trivial. (Trivial is all 0 or all 1.)  
   submode = 6: testing all 5 P/F circuits;  
*/ 
int checkModeL0(int n, int *boards,w32 modecode,w32 submode){
 int ret=0;
 //printf("checkModeL0: modecode,submode %x %x \n",modecode,submode);
 switch(modecode){
	   case 0x104:  // L0 outgen mode
                if(submode == 0){
                 // L0 connection
	            ret=Connect(n,boards);
                }else if(submode == 4){ 
		// L0 connections long test
                        static int i=0,ier=0;		   
                        ret=ConnectPat(n,boards);
                        if(ret){
                          ier++;
                        }
                        HardWare.loop=0;
                        i++;
                        printf("%i TESTS DONE %i ERRORS------------------------------------ \n",i,ier);
		//stopSSM(boards[0]);
                }else if(submode == 1)
		// P/F testing on L1
			ret=calcPFTL1(boards[1],boards[2],HardWare.ipf,0);
		else if(submode == 2){
                // L1 logic test
                } else if(submode == 5){
                // l0,l1,l2,fo,ltu monitor, no generation 
                }else{
                    printf("checkModeL0: outgen mode 0x%x modecode, unknown submode %i\n",modecode,submode);
		    return 2;
                }
		break;
	   case 0x20c:  // L0 input generator
	        //L0ingen(n,boards);
		break;	
	   case 0x002:  // L0 output monitor mode
		// L0 logic
		if(submode == 1) ret = L0classoutmon(boards[0]);
		// Different PF tests
		else if(submode == 2) ret=calcPF(boards[0],boards[0],HardWare.ipf);
		else if(submode == 3) ret=calcPF(boards[0],boards[0],HardWare.ipf);
		else if(submode == 4) ret=calcPF(boards[0],boards[0],HardWare.ipf);
		else if(submode == 5) ret=calcPF(boards[0],boards[0],HardWare.ipf);
		else if(submode == 6) ret=testallPF(boards[0]);
                else if(submode == 7){
                                      ret=alignStrobes(n,boards);
                                      ret=ret+printALL();
                }else{
                 printf("CheckMOdeL0: mode 0x2 unknown submode=%i \n",submode);
                }
		break;
	   case 0x00a:  // L0 input monitor mode
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
          else if(submode == 7)HardWare.loop=0;
	  else HardWare.loop=0;
	  break;
  case 0x104:
          HardWare.loop=0;
          //if(submode == 0)HardWare.loop=1;
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
 printf("iniModeL0: modecode,submode 0x%x %i \n",modecode,submode);
 switch(modecode){
  case 0x104:
    if(submode == 4){
         if(setomSSM(boards[0],0x105)) return 1;; //continuous mode
         if(startSSM1(boards[0])) return 1;
         printf("initModeL0: submode 4 done \n"); 
         //setsmssw(boards[0],"l0_outgen");
         }else { 
           // Generic none mode, nothing to do
         }
	  break;
  default: // L0 output monitor mode
           break;
 }
 return 0;
}

/*--------------------------------------------------------writeBoardsL0()
*/ 
int writeBoardsL0(int n, int *boards, w32 modecode,w32 submode){
 int i;
 static int il0=0;
 switch(modecode){
  case 0x104:   // L0 output generator mode
     // L0connections
     if(submode == 0)for(i=1;i<13;i++)writeSPP(boards[0],0,i,"100000");
     // P/F testing on L1
     else if(submode == 1){
      // Write to int1 and int2
      writeSPP(boards[0],0,11,"10000");
      writeSPP(boards[0],0,12,"10000");
     }else if(submode == 2){
     // L1 class logic test
      writeSPn(boards[0],0,1,0);
      // strobe
      writeSPP(boards[0],0,8,"100000000000000000000000000000000000000000000000000000000000000");
      // data
      writeSPP(boards[0],0,9,"000000000000400000000000000000000000000000000000000000000000000");
     }else if(submode == 3){
      // Generate int1,int2 for interface board testing
      writeSPn(boards[0],0,1,0);
      writeSPF(boards[0],100,11,"200a3000a");
     }else if(submode == 4){
      for(i=1;i<13;i++)writeSPF(boards[0],0,i,"1a2a3a4a5a1a");
      //for(i=1;i<13;i++)writeSPP(boards[0],0,i,"10000000");
     }else return 2;
     break;
  case 0x20c:  // L0 input generator mode
      // Write 0 to all channels on generating board
      writeSPn(boards[0],0,1,0);
      // Signal in ith input signal
      writeSPb(boards[0],0,33,il0+8);
      il0++;
      break; 
  case 0x2: //L0 output monitor
       break;
  default:
    printf("writeBoardsL0: L0 modecode %i not found.\n",modecode);
  return 1;  
 }
 //for(i=1;i<n;i++)writeSPn(boards[i],0,1,0);   //Write all 0 to receiving boards
 for(i=0;i<n;i++)writeSSM(boards[i]);// Write ssm[board[i]].sm to hardware
return 0;	
}

