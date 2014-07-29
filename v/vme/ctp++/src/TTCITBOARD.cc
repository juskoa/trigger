#include "TTCITBOARD.h"
//---------------------------------------------------------------------
TTCITBOARD::TTCITBOARD(string const name,w32 const boardbase,int vsp)
:
BOARD(name,boardbase,vsp,1),
ssm(0),
VERSION(0x4),
CONTROL(0x8),
STATUS(0xc),
READ_SSM_ADDRESS(0x14),
READ_SSM_WORD(0x18),
RESET(0x28),
RESET_SNAPSHOT_N(0x8c)
{
}
void TTCITBOARD::Print()
{
 //printf("%s:",ttcitname.c_str());
 //printboardinfo("");
}
void TTCITBOARD::start_stopSSM()
{
 ssm=GetSSM();
 //vmew(RESET,0);
 // reset address
 resetSSMAddress();
 for(w32 i=0;i<Mega;i++){
  ssm[i]=0;
  vmew(READ_SSM_WORD,0);
 }
 // L0-L1 delay
 //vmew(0x24,259);
 // TTC reset
 //vmew(0x10,0xff);
 usleep(10000);
 // reset address
 printf("address after reset= 0x%x\n",vmer(READ_SSM_ADDRESS));
 //
 //vmew(RESET,0);
 //vmew(CONTROL,0);
 resetSSMAddress();
 while(vmer(READ_SSM_ADDRESS)==0)continue; 
 usleep(50000);
 printf("after reset status: 0x%x\n",getStatus());
 usleep(100000);
 printf("after usleep and control 2 status: 0x%x\n",getStatus());
 printf("# word= 0x%x\n",vmer(READ_SSM_ADDRESS));
 w32 stat=getStatus();
 printf("after usleep and control 3 status: 0x%x\n",stat);

 //return;

 //resetSSMAddress(); 
 for(int i=0;i<Mega;i++){
  ssm[i]=vmer(READ_SSM_WORD);
  //usleep(100000);
 }
}
void TTCITBOARD::AnalyseSSM()
{
 int notactive=Mega+10;
 int achan=notactive;
 for(int i =Mega-1;i>=0;i--){
   int j=Mega-1-i;
   if(ssm[i] & 0x20000){
     printf("%7i A chanel \n",j);
     if(achan==notactive){
       // L0 or L1
       achan=i;
       //printf("First A %i \n",j);
     }else if(achan==i+1){
      //L1 
      printf("L1 at %i \n",j-1);
      achan=notactive;
     }else{
      printf("Error \n");
     }
   }else{
      if(achan != notactive){
        printf("L0 at %i \n",j);
        achan=notactive;
      }
   }
   if(ssm[i] & 0x10000){
     w32 header=(ssm[i]&0xf000)>>12;
     w32 data=ssm[i]&0xfff;
     //printf("%7i Data 0x%1x 0x%3x \n", j,ssm[i]&0xf000,data);
     //string dd=ttcadl[header];
     //printf("%7i %s 0x%1x 0x%3x \n",j,dd.c_str(),header,data);
   }
 } 
}
void TTCITBOARD::DumptxtSSM()
{
 //char *ttcadl[]={"ZERO","L1h","L1d","L2h","L2d","L2r ","RoIh","RoId"};
 string ttcadl[]={"ZERO","L1h","L1d","L2h","L2d","L2r ","RoIh","RoId"};
 for(int i =Mega-1;i>=0;i--){
   int j=Mega-1-i;
   if(ssm[i] & 0x20000){
     printf("%7i A chanel  \n",j);
   }
   if(ssm[i] & 0x10000){
     w32 header=(ssm[i]&0xf000)>>12;
     w32 data=ssm[i]&0xfff;
     //printf("%7i Data 0x%1x 0x%3x \n", j,ssm[i]&0xf000,data);
     string dd=ttcadl[header];
     printf("%7i %s 0x%1x 0x%3x \n",j,dd.c_str(),header,data);
   }
 }
}
