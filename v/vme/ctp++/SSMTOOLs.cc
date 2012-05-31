#include "SSMTOOLs.h"
SSMTOOLs::SSMTOOLs(){};
//-----------------------------------------------------------------------------
void SSMTOOLs::setssm(w32 *ssm){
 this->ssm=ssm;
}
/*------------------------------------------------------------------getNumber()
  I interpret Pedjas binary numbers as follows:
  1.) the most right bit is the least segnificant so
         B"10110001" = 0xb1
  2.) the most right bit is comming first in the signal so
      sm[0]=1,sm[1]=0,sm[2]=0,sm[3]=0,sm[4]=1, ....
  Called by checkSignature()
*/
w32 SSMTOOLs::getNumber(int i0,int length,w32 mask){
 int i;
 w32 bit,Number=0;
 for(i=i0;i<(i0+length);i++){
    if(i>=Mega){
     printf("getNumber: i0=%i, i(%i)>=Mega(%i) \n", i0,i,Mega);
     exit(1);
    }
    bit=(ssm[i]&mask)==mask;
    Number=Number+(bit<<(length-1-(i-i0)));
 }
 return Number;
}
/*---------------------------------------------------------------------------
Debbuging: inverse to getNumber
*/
void SSMTOOLs::writeNumber(int i0,int length,w32 mask, w32 Number){
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
    ssm[i]=(~mask) & ssm[i];
    if(bit)ssm[i]=mask | ssm[i];
 }
 //printf("Number %x at %i \n written \n", Number, i0);
}
//-----------------------------------------------------------------WriteSPP()
/*Write bit pattern Pattern to channel n from Start. 
//Pattern is string of (0-9),(a-f) which is interpreted as hexadecimal number.
//Least significant bits on the left.
*/
int SSMTOOLs::writeSPP(int Start,int Channel,char *Pattern){
 int i,j,i0,length;
 w32 bit,mask0,mask1;
 int pat;
 if(Pattern)length=strlen(Pattern);
 else return 3;
 //printf("Pattern=%x %s \n",Pattern,Pattern);
 printf("writeSPP: strlen= %i \n",length);
 if(!length) return 1;     
 mask1=1<<Channel;
 mask0= ~(0xffffffff & mask1);
 i=Start;
 j=0;         // char count
 while(i<Mega){
   pat = char2int(Pattern[j]);
   if(pat<0) return 4;
   i0=i;
   while(((i-i0)<4) && i<Mega){  
     bit=(1<<(i-i0)) & pat;	   
     if(bit)ssm[i]=ssm[i] | mask1;
     else ssm[i]=ssm[i] & mask0;
     i++;
   }
   j= (j+1) % length;
 }
 return 0;
}
//-----------------------------------------------------------------------------
int SSMTOOLs::genSignatureAll(){
 int channels[NUMOFCHAN],start[NUMOFCHAN],dist[NUMOFCHAN];
 for(int i=0;i<NUMOFCHAN;i++){
    channels[i]=77;
    start[i]=0;
    dist[i]=1024;
 }
 return genSignature(channels,start,dist);
}
/*--------------------------------------------------------------------------
Debugging: generate pattern, inverse to checkSignature()
Inputs: *sm-sm mem
        channels[] - !=0 siganture to be generetad
        start[]    - from the start of memory
        dist[]     - distance between signatures
*/
int SSMTOOLs::genSignature(int *channels,int *start,int *dist){
 int i;
 int ism,jchan,kact,nactive;
 int activeChannels[NUMOFCHAN];
 w32 masks[NUMOFCHAN], signats[NUMOFCHAN];
 int skip[NUMOFCHAN],actdist[NUMOFCHAN];
 for(i=0;i<Mega;i++)ssm[i]=0;
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
        writeNumber(ism,HEADERLEN,masks[kact],HEADER);
        writeNumber(ism+HEADERLEN,SIGNATLEN,masks[kact],signats[kact]);
        writeNumber(ism+HEADERLEN+SIGNATLEN,SIGNATLEN,masks[kact],~signats[kact]);
        skip[kact]=HEADERLEN+2*SIGNATLEN+actdist[kact];
      }else{
        skip[kact]=skip[kact]-1;
      } 
      //printf("ism, kact, skip %i %i %i\n",ism,kact,skip[kact]);  
   }
 } 
 return 0;
}
//---------------------------------------------------------------------
int SSMTOOLs::checkSignatureCon(list<Connection> &Connections){
 int channels[NUMOFCHAN];
 //int n=Connections.size();
 //cout << "n= " << n << endl;
 list<Connection>::iterator con;
 int i=0;
 for(con=Connections.begin();con != Connections.end();++con){
  channels[(con->channel2)]=77;;
  i++;
 }
 return checkSignature(channels,0); 
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
int SSMTOOLs::checkSignature(int *channels,int offset){
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
     if((ssm[ism] & masks[kact]) && !skip[kact]){
       header=getNumber(ism,HEADERLEN,masks[kact]);
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
         signat1=getNumber(ism+HEADERLEN,SIGNATLEN,masks[kact]);
         signat2=getNumber(ism+HEADERLEN+SIGNATLEN,SIGNATLEN,masks[kact]);
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
//-----------------------------------------------------------------------------
int SSMTOOLs::genSeq(int Period,int Start){
 for(int i=0;i<Start;i++)ssm[i]=0;
 for(int i=Start;i<Mega;i++){
  if( ((i-Start) % Period) == 0)ssm[i]=0xffffffff; else ssm[i]=0;    
 }
return 0;
}
//----------------------------------------------------------------------------
int SSMTOOLs::findOffset() const
{
 int i=0,j=0;
 while(i<Mega && j<10){
 j=j+(ssm[i] != 0);
 i++;
 }
 return i;
}
//----------------------------------------------------------------------------
int SSMTOOLs::CompSSM(w32 *ssm2){
 int i;
 while(i<Mega && ssm2[i] == ssm[i])i++;
 return (i<Mega);
}
int SSMTOOLs::CompSSM(w32 *ssm2,int start){
 int i=start;
 while(i<Mega && ssm2[i] == ssm[i])i++;
 return (i<Mega);
}
int SSMTOOLs::CompSSMch(w32 *ssm2,int offset,int channel){
 int i=offset;
 w32 mask=1<<channel;
 while(i<Mega && (ssm2[i]&mask) == (ssm[i-offset]&mask))i++;
 return (i<Mega);
}
/*-------------------------------------------------------------------------------------------------------
When first error found the routine returns.
*/
int SSMTOOLs::CompSSMch(w32 *ssm2,int const offset,list<Connection> &Connections,int *channel,int *position) const
{
 int n=Connections.size();
 //cout << "n= " << n << endl;
 w32 masks[n][2];
 string name[n];
 list<Connection>::iterator con;
 *channel=0;
 *position=0;
 int i=0;
 for(con=Connections.begin();con != Connections.end();++con){
  masks[i][0]=(1<<(con->channel1));
  masks[i][1]=(1<<(con->channel2));
  name[i]=con->name;
  i++;
 }
 //for(i=0;i<n;i++) cout << name[i]<<":"<<hex<<masks[i][0] << " " << masks[i][1] << endl;
 //cout << "offset= "<< dec<< offset << endl; 
 i=offset;
 while(i<Mega){
  int error=0;
  for(int j=0;j<n;j++){
   int bit1=((ssm[i-offset]&masks[j][0])==masks[j][0]);
   int bit2=((ssm2[i]&masks[j][1])==masks[j][1]);
   //cout << "i="<<i<<" " <<hex <<ssm[i-offset] <<" "<< ssm2[i]<<" " <<bit1 << bit2 << endl;
   if(bit1 != bit2){
    //cout << "Error in channel " << name[j] << endl;
    //cout << "error at " <<  i << endl;
    *channel=j;
    *position=i;    
    error++;
   }
  }
  if(error) break;
  i++;
 }
 return (i<Mega);
}
//--------------------------------------------------------------------------
int SSMTOOLs::CompSSMch(w32 *ssm2,w32 const mask1,w32 const mask2) const
{
 int i;
 while(i<Mega && (ssm2[i]&mask2) == (ssm[i]&mask1))i++;
 return (i<Mega);
}
//----------------------------------------------------------------------------
int SSMTOOLs::find0(){
 int i=0;
 while((i<Mega) && (ssm[i] == 0))i++;
 return i; 
}
//----------------------------------------------------------------------------
int SSMTOOLs::dumpSSM(char *name){
 char filename[200];
 char *environ;
 char fnpath[1024];
 FILE *dump;
 sprintf(filename,"%s",name);
  // Open file
 environ= getenv("VMEWORKDIR"); strcpy(fnpath, environ);
 strcat(fnpath,"/"); strcat(fnpath,"WORK/"); 
 strcat(fnpath, filename); strcat(fnpath, ".dump");
 dump=fopen(fnpath,"w");
 if(dump == NULL){
  printf("Cannot open file: fnpath: %s\n", fnpath);
  exit(1);
 }
 for(int i=0; i<Mega; i++) {
    w32 d=ssm[i];
    fwrite(&d, sizeof(w32), 1, dump);
  };
 fclose(dump); 
 printf("Dump written in fnpath %s\n", fnpath);
 return 0;
}

