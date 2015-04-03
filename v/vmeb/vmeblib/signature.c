#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned int w32;
#define Mega (1024*1024)
#define NUMOFCHAN 32
#define HEADERLEN 8
#define SIGNATLEN 7
#define HEADER 0xb1

#define SSMDUMP "ssm.dump"
// 0xb1=0x8d !
/*-------------------------------------------------------------------------
Debugging: Write binary file of the same foemat as in ssm.c
Same as dumpSSM defined in ssm.c. This one is for standalone debug.
*/
int dumpSSMlocal(w32 *sm){
 int i;
 w32 word;
 FILE *dump;
 dump= fopen(SSMDUMP,"w");
 if(dump==NULL) {
  printf("cannot open file %s\n", SSMDUMP);
  return 1;
 };
 for(i=0;i<Mega;i++){ 
  word=sm[i];
  fwrite(&word, sizeof(w32), 1, dump);
 }
 return 0;
}
/*-------------------------------------------------------------------------
Debugging: Reads binary file ssm.dump.
Equivalent of readSSMDump() in ssm.c on altri1
*/
int readSSMDumplocal(w32 *sm){
  int i,nwords;
 w32 word;
 FILE *dump;
 dump= fopen(SSMDUMP,"w");
 if(dump==NULL) {
  printf("cannot open file %s\n", SSMDUMP);
  return 1;
 };
 for(i=0;i<Mega;i++){ 
  nwords=fread(&word,sizeof(w32),1,dump);
  sm[i]=word;
 }
 return 0;
}
/*-----------------------------------------------------------------------------
Debugging: print ssm.
*/
void printSSMlocal(w32 *sm,int N){
 int i;
 for(i=0;i<N;i++)printf("%d ",sm[i]);
 printf("\n");
}
/*---------------------------------------------------------------------------
Debbuging: inverse to getNumber
*/
void writeNumber(w32 *sm,int i0,int length,w32 mask, w32 Number){
 int i;
 w32 bit,shift;
 for(i=i0;i<(i0+length);i++){ 
    if(i>=Mega){
     printf("writeNumber:i0=%i index >= Mega: %i >= %i \n",i0,i,Mega);
     exit(1);
    }   
    //Number=Number+(bit<<(length-1-(i-i0))); <- from getNumber
    //bit= (Number & (1<<(i-i0)))==(1<<(i-i0)); <- old writeNumber
    shift=1<<(length-1-(i-i0));
    bit= (Number & shift )==shift ;
    //if(rnlx()<errorrate)bit=!bit;
    sm[i]=(~mask) & sm[i];
    if(bit)sm[i]=mask | sm[i];
 }
 //printf("Number %x at %i \n written \n", Number, i0);
}
/*--------------------------------------------------------------------------
Debugging: generate pattern, inverse to checkSignature()
Inputs: *sm-sm mem
        channels[] - !=0 siganture to be generetad
        start[]    - from the start of memory
        dist[]     - distance between signatures
*/
int genSignature(w32 *sm,int *channels,int *start,int *dist){
 int i;
 int ism,jchan,kact,nactive;
 int activeChannels[NUMOFCHAN];
 w32 masks[NUMOFCHAN], signats[NUMOFCHAN];
 int skip[NUMOFCHAN],actdist[NUMOFCHAN];
 for(i=0;i<Mega;i++)sm[i]=0;
 // Find active channels (to optimise algorithm speed)
 kact=0;
 for(jchan=0;jchan<NUMOFCHAN;jchan++){
  if(channels[jchan]){
   activeChannels[kact]=jchan;
   masks[kact]=(1<<jchan);
   signats[kact]=channels[jchan];
   skip[kact]=start[jchan];
   actdist[kact]=dist[jchan];
   kact++;
  }
 }
 nactive=kact;
 //
 for(ism=0;ism<(Mega-HEADERLEN-2*SIGNATLEN);ism++){// loop over sm
   for(kact=0;kact<nactive;kact++){    // loop over active channels
      if(skip[kact] == 0){
        writeNumber(sm,ism,HEADERLEN,masks[kact],HEADER);
        writeNumber(sm,ism+HEADERLEN,SIGNATLEN,masks[kact],signats[kact]);
        writeNumber(sm,ism+HEADERLEN+SIGNATLEN,SIGNATLEN,masks[kact],~signats[kact]);
        skip[kact]=HEADERLEN+2*SIGNATLEN+actdist[kact];
      }else{
        skip[kact]=skip[kact]-1;
      } 
      //printf("ism, kact, skip %i %i %i\n",ism,kact,skip[kact]);  
   }
 } 
 return 0;
}
/*------------------------------------------------------------------getNumber()
  I interpret Pedjas binary numbers as follows:
  1.) the most right bit is the least segnificant so
         B"10110001" = 0xb1
  2.) the most right bit is comming first in the signal so
      sm[0]=1,sm[1]=0,sm[2]=0,sm[3]=0,sm[4]=1, ....
  Called by checkSignature()
*/
w32 getNumber(w32 *sm,int i0,int length,w32 mask){
 int i;
 w32 bit,Number=0;
 for(i=i0;i<(i0+length);i++){ 
    if(i>=Mega){
     printf("getNumber: i0=%i, i(%i)>=Mega(%i) \n", i0,i,Mega);
     exit(1);
    }   
    bit=(sm[i]&mask)==mask;
    Number=Number+(bit<<(length-1-(i-i0)));
 }
 return Number;
}
/*-----------------------------------------------------checkSignature()
Tested routine:
Inputs:
 w32 *sm - pointer to snapshot mem [Mega]
 int *channels - pointer to arraj [24]
               - !=0 - channel to be checked with signature stored in it
checkSignature should be called from steering routine, 
which does relations between channels and inputs.
*/
int checkSignature(w32 *sm,int *channels,int offset){
// Output info:
 int CountHeaders[NUMOFCHAN]; // Count number of found headers
 int CountHeadersF[NUMOFCHAN]; // Count number of attemted and failed headers
 int CountComplL[NUMOFCHAN];   // Count number of signature == complement ssm
 int CountComplG[NUMOFCHAN];   // Count number of signature == complement input
 int CountSignat[NUMOFCHAN];  // Count number of found signatures
 int CheckDistance[NUMOFCHAN];// Count number of changes between found headers
//--
 int ism,jchan,kact,nactive;
 int idist0[NUMOFCHAN],dist0[NUMOFCHAN],dist;
 int activeChannels[NUMOFCHAN];
 w32 masks[NUMOFCHAN],signats[NUMOFCHAN],skip[NUMOFCHAN];
 w32 Signatures[NUMOFCHAN];
 w32 header,signat1,signat2;
 // Init
 for(kact=0;kact<NUMOFCHAN;kact++){
   CountHeaders[kact]=0;
   CountHeadersF[kact]=0;
   CountComplL[kact]=0;
   CountComplG[kact]=0;
   CountSignat[kact]=0;
   CheckDistance[kact]=0;
   Signatures[kact]=0;
   idist0[kact]=0;
   dist0[kact]=0;
   skip[kact]=0;
 }
 // Find active channels (to optimise algorithm speed)
 kact=0;
 for(jchan=0;jchan<NUMOFCHAN;jchan++){
  if(channels[jchan]){
   activeChannels[kact]=jchan;
   masks[kact]=(1<<jchan);
   signats[kact]=channels[jchan];
   kact++;
  }
 }
 nactive=kact;
 printf("Number of active channels =%i \n",nactive);
 // Analyse
 for(ism=0;ism<(Mega-HEADERLEN-2*SIGNATLEN);ism++){// loop over sm
   //printf("ism= %i \n",ism);
   for(kact=0;kact<nactive;kact++){    // loop over active channels
    // if nonzero bit in channel check for HEADER
     if((sm[ism] & masks[kact]) && !skip[kact]){
       header=getNumber(sm,ism,HEADERLEN,masks[kact]);
       //printf("Checking header at %i kact=%i header=0x%x\n",ism,kact, header);
       if(header == HEADER){
         //printf("Header found in channel %i %i\n",activeChannels[kact],ism);
         CountHeaders[kact]++;
         dist=ism-idist0[kact];
         if(dist != dist0[kact]){
           if(CountHeaders[kact]>2){  // At start if ssm dist is not defined
              CheckDistance[kact]++;
              //printf("Dist error kact=%i ism=%i \n",kact,ism);
           }
         }
         dist0[kact]=dist;
         idist0[kact]=ism;
         //check signature
         signat1=getNumber(sm,ism+HEADERLEN,SIGNATLEN,masks[kact]);
         signat2=getNumber(sm,ism+HEADERLEN+SIGNATLEN,SIGNATLEN,masks[kact]);
         //printf("signatures: 0x%x 0x%x 0x%x\n",signat1,signat2,signats[kact]);
         if(signat1 == ((signats[kact]>>9)&0x7f))CountSignat[kact]++;
         if(signat1 == ((~signat2)&0x7f))CountComplL[kact]++;
         if(signat2 == ((signats[kact]>>2)&0x7f))CountComplG[kact]++; 
         skip[kact]=HEADERLEN+2*SIGNATLEN-1;
         Signatures[kact]=signat1;
       }else{
         CountHeadersF[kact]++;
         //printf("False header at %i \n",ism);
       }
     }else if(skip[kact])skip[kact]--;
   }
 } 
 for(kact=0;kact<nactive;kact++){
    //if((CountHeaders[kact]>1000) && (CountComplL[kact]>1000))    
    printf("inp:<%i> sig:<%i>",activeChannels[kact]+1-offset,Signatures[kact]);
 }
 printf("\n");
 printf("Channel  Headers FHeaders Signatures CmplCntsS CmplCntsI CheckDist\n");
 for(kact=0;kact<nactive;kact++){
  printf("%6i %8i %8i    %6i    %6i    %6i  %8i \n",activeChannels[kact]+1-offset,CountHeaders[kact],CountHeadersF[kact],CountSignat[kact],CountComplL[kact],CountComplG[kact],CheckDistance[kact]);
 }
 printf("Last distance=%i \n",dist);
 return 0; 
}

