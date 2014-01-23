#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.c"

#define Mega 1024*1024

/*    Constants for analyze SSM */
#define DISTL0 119
#define DISTL1 119
#define DISTL2 119
#define NL1dat 108
#define NL2dat 149
/*****************************************************************/
/*  Snapshot memory   */
/****************************************************************/
/*----------------------------------------------------------------*/
void channelB(int canal, int bit, int i, int *COUNT,int *COUNTa, int *icount, int *data,char *name);
void txprintOP(int i,int *TXS,char *name);
void lsig(int ,int ,int ,int *,int *,int *,char *);
void ssig(int ,int ,int ,int *,int *,int *,int *,int *,char *);
int asig(int ,int ,int ,int *,int *,int *,int *,int *,char *);
int data(int ,int, int, int *,int *,int,  int *,char *); 
int txsig(int,int,int,int *,int *,int *);
void txprint(int,int *,char *);
int txprintOff(int *);
int readFile();
int writeLog();
void analTTCB();

int quit=0;
static int SSMem[Mega];
struct list *dump;
int ttcboffset=0;
int TTCLX=0;
/*********************************************************************/
/*FGROUP SSM_VME_Access ReadSSM
Analyze SSM memory - like AS python + check of serial versus TTC
*/
int analyze(){    
 int i,j,bit,word,ier;
 //int first=1;
 /*         L0 L1s L2s AE  */
 char *SIGname[]={"ORB","PP ","L0 ","L1s","L1d","L2s","L2d","sBU","lBU","1FF","2FF","ChA","ChB","TBU","PPT","SST","STA","AER"};
 int NPR=6;
 char *PRINT[]={"PP ","L0 ","L1s","L2s","AER","LBH"};
 int COUNT[18]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  /* COUNT SSM signals */
 int COUNTe[18]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; /* COUNT errors */
 int COUNTl[18]={0,0,-DISTL0,-DISTL1,0,-DISTL2,0,0,0,0,0,0,0,0,0,0,0,0}; /* How close they can be ? */
 int COUNTa[18]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; /* Is signal active ? */
 int DIST[18]={0,0,DISTL0,DISTL1,0,DISTL2,0,0,0,0,0,0,0,0,0,0,1,0}; /* How close the signals can be ? */
 int L1DATA[NL1dat],L2DATA[NL2dat],TT[64];
 int iL1d=0,iL2d=0,ivmes=0,ichb=0;
 int iorbi=0,ipp=0,ialls=0;
 int isdb=0,iltb=0,il1fi=0,il2fi=0,icha=0,ittcbusy=0,ippt=0;
 if(readFile()!=0) exit(9);
 dump=NULL;
 analTTCB();
 //ttcboffset=1;
 for(i=ttcboffset;i<Mega;i++){
   word=SSMem[i];
   // start only after first L0
   //bit= ( (word & 4) == 4);
   //if(bit) first=0;
   //if(first) continue;
   for(j=0;j<18;j++){
    bit= ( (word & (1<<j)) == (1<<j));
    switch(j){
     case  0:    /* ORBIT   */
	   lsig(0,bit,i,COUNT,COUNTa,&iorbi,"ORBIT");  
           break;
     case  1:    /* PREPULSE    */
           lsig(1,bit,i,COUNT,COUNTa,&ipp," PP");
	   break;
     case  2:  /* L0 */
	   ssig(2,bit,i,COUNT,COUNTa,COUNTl,COUNTe,DIST,"L0");
           break;
     case  3:  /* L1s */  	   
	   asig(3,bit,i,COUNT,COUNTa,COUNTl,COUNTe,DIST,"L1S");
           break;
     case  4:   /* L1data */
	   ier=data(4,bit,i,COUNTa,L1DATA,NL1dat,&iL1d,"L1DATA");
           break;
     case  5:   /* L2 strobe */
           asig(5,bit,i,COUNT,COUNTa,COUNTl,COUNTe,DIST,"L2S");
	   break;
     case  6:   /* L2 data */
	   ier=data(6,bit,i,COUNTa,L2DATA,NL2dat,&iL2d,"L2DATA");
           break;
     case  7: /* Sub Detector Busy */
	   lsig(7,bit,i,COUNT,COUNTa,&isdb,"SBUSY");
           break;
     case  8: /* LTU BUSY */
	   lsig(8,bit,i,COUNT,COUNTa,&iltb,"ALLBUSY");
           break;
     case  9: /* L1 FIFO Nearly Full */
	   lsig(9,bit,i,COUNT,COUNTa,&il1fi,"L1NF");
           break;
     case 10: /* L2 FIFO Nearly Full */
	   lsig(10,bit,i,COUNT,COUNTa,&il2fi,"L2NF");
           break;
     case 11: /* Channel A (L0) */
	   lsig(11,bit,i,COUNT,COUNTa,&icha,"ChanA");
           break;
     case 12:    /*  Channel B */
           channelB(12,bit,i,COUNT,COUNTa,&ichb,TT,"ChanB");
           break;
     case 13:   /* TTC  BUSY */
           lsig(13,bit,i,COUNT,COUNTa,&ittcbusy,"TTCBUSY"); 
           break;
     case 14:          /* PP transmit */
           lsig(14,bit,i,COUNT,COUNTa,&ippt,"PPT"); 
           break;
     case 15:    /*  vme SLAVE strobe  */
	   lsig(15,bit,i,COUNT,COUNTa,&ivmes,"VMES");
           break;
     case 16:   /* START ALL - emulator */
	   lsig(16,bit,i,COUNT,COUNTa,&ialls,"ALLSTART");
	   break;
     case 17: /* ANY ERROR */
	   ssig(17,bit,i,COUNT,COUNTa,COUNTl,COUNTe,DIST,"ANYERR");
           break;
    }
   }
 }
 // to take into acount signals up in all memory
 lsig(1,0,Mega,COUNT,COUNTa,&ipp," PP");
 lsig(7,0,Mega,COUNT,COUNTa,&isdb,"SBUSY");
 lsig(8,0,Mega,COUNT,COUNTa,&iltb,"ALLBUSY");
 lsig(9,0,Mega,COUNT,COUNTa,&il1fi,"L1NF");
 lsig(10,0,Mega,COUNT,COUNTa,&il2fi,"L2NF");
 lsig(11,0,Mega,COUNT,COUNTa,&icha,"LBHALT");
 //lsig(12,0,Mega,COUNT,COUNTa,&ivmem,"VMEM");
 lsig(13,0,Mega,COUNT,COUNTa,&ittcbusy,"TTCBUSY"); 
 lsig(15,0,Mega,COUNT,COUNTa,&ivmes,"VMES");
 lsig(16,0,Mega,COUNT,COUNTa,&ialls,"ALLSTART");
 for(i=0;i<NPR;i++){
  for(j=0;j<18;j++)if(SIGname[j] == PRINT[i])
	  printf("<%s=%i> ",SIGname[j],COUNT[j]);
 }
 printf("\n");
 writeLog();
 return 0;
}
/*------------------------------------------------------------------------------
 * analyse only channel B because you dont know where to start
 * you have 42 options
 */
void analTTCB(){
 int i,ioff,word,bit;
 int success;
 for(ioff=0;ioff<42;ioff++){
  printf("%i \n",ioff);
  int data[64];
  int active=0,icount=0;
  int datab=3;
  success=1;
  for(i=ioff;i<Mega;i++){
   word=SSMem[i];
   bit= (word & (1<<12)) == (1<<12);
   if(active){
    if(icount == 0){
      if(bit == 0) datab = 0;  //orbit and pp
      else datab = 1;          // data
      icount=icount+1;
      continue; 
    }
    //printf("datab=%i \n",datab);
    if(icount == 14 && datab == 0){
      //txprintOP(i,data,name);
      //printf("Orbit \n");
      active=0;
      continue ;
    }
    if(icount == 41 && datab == 1){
      //printf("Data \n");
      if(txprintOff(data))success=0;
      active=0;
      continue ;
    }
    //if(*icount>25  && *icount<(25+8)) data=data+ (bit<<(*icount-26));
    data[icount]=bit;
    icount=icount+1;
   }else{
    if(bit == 0){
     active=1 ;
     icount=0;
    } 
   }
  }
  printf("ioff=%i success=%i \n",ioff,success);
  if(success) break;  
 }
 if(success){
  ttcboffset=ioff; 
 }else{
  printf("Error in Channel B\n");
 }
}
/*------------------------------------------------------------------------------*/
void channelB(int canal, int bit, int i, int *COUNT,int *COUNTa, int *icount, int *data,char *name){
 static int com;
 if(COUNTa[canal]){
  if(*icount == 0){
    if(bit == 0) com= 0;  //orbit and pp
    else com = 1;
    *icount=*icount+1;
     return ; 
  }
  if(*icount == 14 && com == 0){
    txprintOP(i,data,name);
    COUNTa[canal]=0;
    return ;
  }
  if(*icount == 41 && com == 1){
      //add  to list data
     txprint(i,data,name);
     COUNTa[canal]=0;
     return ;
  }
  //if(*icount>25  && *icount<(25+8)) data=data+ (bit<<(*icount-26));
  data[*icount]=bit;
  *icount=*icount+1;
 }else{
  if(bit == 0){
     COUNTa[canal] =1 ;
     *icount=0;
  } 
 }
}
/*---------------------------------------------------------------------------------*/
void lxprint(int i,int rc,int NLxdat,int *LxDATA,char *name)
{
 int k,j,offset=0,Ntclstart=8;
 //int tcl1,tcl2,tcl3;
 char tcl[25];
 char mess[38],line[256]=" ",www[256];
 switch(rc){
  case(0):
   if(NLxdat == NL2dat){
     offset=1;  // L2ar flag is not included
     Ntclstart=49; 
   }
   for(k=0;k<38;k++)mess[k]=0;
   for(k=offset;k<NLxdat;k++){
    //printf("%1i",LxDATA[k]);
    j=k-offset;
    //printf("%i %i %i \n",j,j/4,k);
    mess[j/4]=mess[j/4]+(1 << (3-(j%4)))*LxDATA[k];
   }
   //printf("\n");
   strcpy(line,name);
   strcat(line,":"); 
   for(k=0;k<(NLxdat/4)+1-offset;k++){
     sprintf(www,"%1x",mess[k]);
     strcat(line,www);
     if( ((k%3) == 2) && (k != NLxdat/4-offset)) strcat(line,".");
   }
   if(offset){
     //printf("k= %i \n",k);
     sprintf(www,"%1x",mess[k]);
     strcat(line,www);
   }
   /* Trigger classes */
   /* class 1=left
    tcl1=0;
    for(k=0;k<32;k++) tcl1=tcl1+(1<<k)*LxDATA[NLxdat-k-1];
    tcl2=0;
    for(k=32;k<64;k++) tcl2=tcl2+(1<<(k-32))*LxDATA[NLxdat-k-1];
    tcl3=0;
    for(k=64;k<100;k++) tcl3=tcl3+(1<<(k-64))*LxDATA[NLxdat-k-1];
    //sprintf(www," TrCl: 0x%4x%8x%16x ",tcl2,tcl1,tcl3); //sprintf(www," TrCl: 0x%+04x%+08x ",tcl2,tcl1);
    sprintf(www," TrCl: 0x%8x%8x%8x ",tcl1,tcl2,tcl3);
   */ 
    /* Trigger classes */
   /* class 1=right */
    strcat(line," TrCl: 0x");
    for(k=0;k<25;k++)tcl[k]=0;
    for(k=Ntclstart;k<NLxdat;k++){
       int k4=(k-Ntclstart)/4;
       int kr=(k-Ntclstart) % 4;
       tcl[k4]=tcl[k4]+(1<<(3-kr))*LxDATA[k];
    }
    for(k=0;k<25;k++){
      //printf("%1x",tcl[k]);
      sprintf(www,"%1x",tcl[k]);
      strcat(line,www);
    } 
   //strcat(line,www);                             
   dump=addlist(dump,i-NLxdat+1,line);
   break;
  case(1):
   //sprintf(line,"Error: %s arrives before strobe.",name);
   //dump=addlist(dump,i,line);  
   break;
 } 
}
int data(int canal,int bit,int i,int *COUNTa,int *LxDATA,int NLxdat,int *iLxd, char *name)
/* Stores serial L1/L2 data .
   Checks if data are out od strobe.
*/
{
 /* printf("canal,COUNTa[canal-1] bit %i %i %i\n",canal,COUNTa[canal-1],bit); */
 if(COUNTa[canal-1]){
            LxDATA[*iLxd]=bit;
            *iLxd=*iLxd+1;
            if(*iLxd == NLxdat){
             lxprint(i,0,NLxdat,LxDATA,name);
	     *iLxd=0;
             COUNTa[canal-1]=0;
	    }	    
           }else{
	    if(bit){
             lxprint(i,1,NLxdat,LxDATA,name);		                 
	     return 1;	     
	    }	    
           }		
 return 0;	   
}
void txprintOP(int i,int *TXS,char *name){
 char text[256];
 int k,data,chck;
 data=0;
 for(k=0;k<8;k++)data=data+(1<<k)*TXS[8-k];
 chck=0;
 for(k=0;k<5;k++)chck=chck+(1<<k)*TXS[13-k];
 sprintf(text,"%s OP:0x%x 0x%x ",name,data,chck);
 dump=addlist(dump,i-15,text);
}
/*--------------------------------------------------------------------------
 * txprint when for finding offset - no printing
 */ 
int txprintOff(int *TXS)
{
 int k,rc=0;
 int ttcadd,e,code,data,chck;
 ttcadd=0;
 for(k=0;k<14;k++)ttcadd=ttcadd+(1<<k)*TXS[14-k];
 e=TXS[15];
 code=0;
 for(k=0;k<4;k++)code=code+(1<<k)*TXS[20-k];
 data=0;
 for(k=0;k<12;k++)data=data+(1<<k)*TXS[32-k];
 chck=0;
 for(k=0;k<8;k++)chck=chck+(1<<k)*TXS[39-k];
 if(code > 7)rc=1;
 return rc;
}
/*--------------------------------------------------------------------------
 * txprint when offset is known
 */ 
void txprint(int i,int *TXS, char *name)
{
 int k;
 //char *ttcadl[]={"ZERO","L1h ","L1d ","L2h ","L2d ","L2r ","RoIh","RoId"};
 char *ttcadl[]={"ZERO","L1h","L1d","L2h","L2d","L2r ","RoIh","RoId"};
 char text[256];
 int ttcadd,e,code,data,chck;
 ttcadd=0;
 for(k=0;k<14;k++)ttcadd=ttcadd+(1<<k)*TXS[14-k];
 e=TXS[15];
 code=0;
 for(k=0;k<4;k++)code=code+(1<<k)*TXS[20-k];
 data=0;
 for(k=0;k<12;k++)data=data+(1<<k)*TXS[32-k];
 chck=0;
 for(k=0;k<8;k++)chck=chck+(1<<k)*TXS[39-k];
 if(code > 7){
  printf("txprint: unexpected code 0x%x %s \n",code,name);
  sprintf(text,"Error: unexpected code in ttc:");
  for(k=0;k<40;k++){
    char cc[2]="0";
    if(TXS[k]) cc[0]='1';
    strcat(text,cc);
  }
  dump=addlist(dump,i,text);
  writeLog();
  exit(2);
 }
 // Counting TTC words for L1 and L2 messages
 if(code==1 || code==3) TTCLX=0;
 TTCLX++;
 //sprintf(text,"%s %s:0x%04x %x 0x%x 0x%03x 0x%02x ",name,ttcadl[code],ttcadd,e,code,data,chck);
 sprintf(text,"%s %s%02i:0x%04x %x 0x%x 0x%03x 0x%02x ",name,ttcadl[code],TTCLX,ttcadd,e,code,data,chck);
 dump=addlist(dump,i-39,text);
}
int txsig(int canal, int bit, int i,int *COUNTa, int *itxs,int *TXS)
{
/* TTC LS - least significant 8 bits
                 logic:
		Active  Bit
		 1       1        write word                  
		 1       0        write word
		 0       1        start word
		 0       0        no action                */
 if(COUNTa[canal]){
   TXS[*itxs]=bit;
   *itxs=*itxs+1;
   if(*itxs == 8){
     COUNTa[canal]=0;
     *itxs=0;
     return 1;    
    }	     
 }else if(bit) COUNTa[canal]=1;
 return 0;  	       		   
}
void lsig(int canal, int bit, int i, int *COUNT,int *COUNTa, int *icount, char *name)
/* Long signal = no strobe. Measures the length of signal until next 0 */
{
 char text[256];
 if(COUNTa[canal]){
    if(bit) *icount=*icount+1;
    else{
         sprintf(text,"%s/%i",name,*icount+1);
	 dump=addlist(dump,i-*icount-1,text);	
	 *icount=0;
	 COUNTa[canal]=0;
	 COUNT[canal]++;      
    }		     
 }else if(bit) COUNTa[canal]=1;  
	   
}
void ssig(int canal,int bit,int i,int *COUNT,int *COUNTa,int *COUNTl,int *COUNTe,
		int *DIST,char *name)
/* Short signal = should be one BC , otherwise error logged in COUNTe,
   program continues */
{
 if(bit){
   COUNT[canal]++;
   dump=addlist(dump,i,name);
   if( (i-COUNTl[canal])<DIST[canal]){
     COUNTe[canal]++;             
    }
    COUNTl[canal]=i;
 }	
}
int asig(int canal,int bit,int i,int *COUNT,int *COUNTa,int *COUNTl,int *COUNTe,
		int *DIST,char *name)
/* active signal = short signal + activates data
L1S/L2S logic
 bit  Active
  1    1     error
  1    0     ok -> activate
  0    1     ok
  0    0     ok
*/	  
{
if(bit){
  if(!COUNTa[canal]){		   
     COUNT[canal]++;
     dump=addlist(dump,i,name);
     if( (i-COUNTl[canal])<DIST[canal]){
       COUNTe[canal]++;             
     }
     COUNTl[canal]=i;
     COUNTa[canal]=1;
     }else{
      char text[256];
      sprintf(text,"Error: %s arrives while data active.",name);
      dump=addlist(dump,i,text);
      return 1;
  }		
}
return 0;
}
/*FGROUP SSM_VME_Access
 * Read from the binary file - for debugging.
 */ 
int readFile(){
 FILE *f;
 int i=0;
 int word;
 size_t nmemb=1,nread; 
 /* f=fopen("/home/alice/rl/boards/vme/WORK/SSM.dump","rb");*/
 //f=fopen("WORK/SSM.dump","rb");
 f=fopen("/home/alice/trigger/v/vme/WORK/SSM.dump","rb");
 if(f == NULL){
  printf("File WORK/SSM.dump not opened.\n");
  return 2;
 } 
 while((nread=fread(&word,sizeof(int),nmemb,f)) == nmemb){
  if(i>Mega){
   printf("File bigger than Mega \n");
   return 1;   
  }	  
  SSMem[i]=word;	 
  i++;
 }
 printf("File successfuly read, nwords=%i \n",i); 
 return 0;
}
/***************************************************************************/
int writeLog()
{
 FILE *ff;
 //ff=fopen("WORK/SSMa.txt","w");
 ff=fopen("/home/alice/trigger/v/vme/WORK/SSMa.txt","w");
 if(ff==0){
   printf("Cannot open file WORK/SSMa.txt \n");
   return 2;
 }
 printlist(dump,ff);
 return 0;
}
/***************************************************************************/
int main(int argn, char **argv) {
 analyze();
 return 1;
}                                                                                             
	
