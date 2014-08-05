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
 for(w32 i=0;i<qttcab.size();i++) delete qttcab[i];
 qttcab.clear();
}
void TTCITBOARD::ClearQueues()
{
}
void TTCITBOARD::Print()
{
 //printf("%s:",ttcitname.c_str());
 //printboardinfo("");
}
/*
 * Original routine written during debuging.
 */ 
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
/*
 * Modified routine
 */ 
int TTCITBOARD::start_stopSSM(LTUBOARD* ltu)
{
 ssm=GetSSM();
 // reset address
 resetSSMAddress();
 for(w32 i=0;i<Mega;i++){
  ssm[i]=0;
  vmew(READ_SSM_WORD,0);
 }
 usleep(10000);
 printf("address after reset= 0x%x\n",vmer(READ_SSM_ADDRESS));
 //
 // reset again - this makes ttc wait for input
 resetSSMAddress();
 if(ltu->SLMstart()) return 1;
 usleep(21000);
 ltu->SLMquit();
 while(vmer(READ_SSM_ADDRESS)==0)continue; 
 usleep(50000);
 printf("after reset status: 0x%x\n",getStatus());
 usleep(100000);
 printf("after usleep and control 2 status: 0x%x\n",getStatus());
 printf("# word= 0x%x\n",vmer(READ_SSM_ADDRESS));
 w32 stat=getStatus();
 printf("after usleep and control 3 status: 0x%x\n",stat);

 //resetSSMAddress(); 
 for(int i=0;i<Mega;i++){
  ssm[i]=vmer(READ_SSM_WORD);
  //usleep(100000);
 }
 return 0;
}
/*
 * Analyse que dump
 * Assume that TTC sequence start from beginning
 *
*/
void TTCITBOARD::AnalyseSSM()
{
 if(qttcab.size() ==  0){
   printf("Empty ssm memory \n");
   return;
 }
 if(qttcab[0]->issm != 262){
   printf("Error: first L1 expected at 262 but found at %i \n",qttcab[0]->issm);
   return ;
 }
 deque<w32> L1;
 deque<w32> L1m;
 deque<w32> L2m;
 w32 L0L1time=260;
 w32 l0=2;
 w32 l1mes[10],l2mes[14]; // one flag
 l1mes[0]=0;l2mes[0]=0;
 for(w32 i=0;i<qttcab.size();i++){
   //printf("%i data=%i head= %i \n",qttcab[i]->issm,qttcab[i]->data,qttcab[i]->ttcode);
   w32 il1,il2;
   ssmrecord *ss=qttcab[i];
   w32 issm=ss->issm;
   if(ss->ttcode == 0){
     if((ss->data==1)){
      // L0 received 
      if((issm-l0)<L0L1time){
        printf("L0L1 time violation - two L0 closer then %i:  %i \n",L0L1time,issm-l0);
        return;
      }
      l0=issm;
     } else {
      // L1 received
      if((issm-l0) != L0L1time){
        printf("L0L1 time violation - L1 arrived not in time: %i \n",issm-l0);
        return ;
      }
      L1.push_back(issm);
     }
   } else if(ss->ttcode == 1){
     // L1h
     if(l1mes[0]){
       printf("Error: L1 shorter than 9 issm=%i \n",issm);
       return ;
     }else{
       l1mes[0]=1;
       l1mes[1]=ss->data;
       il1=2;
     }
   } else if(ss->ttcode == 2){
     //L1 data
     if(l1mes[0]==0){
       printf("Error: L1 data without header issm=%i \n", issm);
       return ;
     }
     l1mes[il1]=ss->data;
     //printf("il1= %i \n",il1);
     if(il1==9){
       // last word of mess
       il1=0;
       l1mes[0]=0;      
       L1m.push_back(issm); // thos to be changed for pointers
     }
     il1++;
   } else if(ss->ttcode == 3){
       //L2h
       if(l2mes[0]){
         printf("Error: L2 shorter than 13 issm= %i \n",issm);
         return;
       }else{
         l2mes[0]=1;
         l2mes[1]=ss->data;
         il2=2;
       }
   } else if(ss->ttcode == 4){
       // L2 data
       if(l2mes[0]==0){
         printf("Error: L2 data without header issm=%i\n",issm);
         return ;
       }
       l2mes[il2]=ss->data;
       if(il2==13){
         // last word
         il2=0;
         l2mes[0]=0;
         L2m.push_back(issm);
       }
       il2++;
   }else{
    printf("Error: unexpected code %i",ss->ttcode);
    return ;
   }   
 }
 printf("# of L1: %i , # if L1 mess: %i , # of L2 mess: %i \n",L1.size(),L1m.size(),L2m.size());
}
/*
 * SSM dump as ssmrecord que
 * ttchead = 0  data=1  l0
 * ttchead = 0  data=2  l1
 * ttchead = 1  L1 head
 * ttchead = 2  L1 data
 * ttchead = 3  L2 head 
 * ttchead = 4  L2 data 
*/
void TTCITBOARD::Dump2quSSM()
{
 string ttcadl[]={"ZERO","L1h","L1d","L2h","L2d","L2r ","RoIh","RoId"};
 if(ssm==0){
   printf("Dump2quSSM: no ssm found , quiting. \n");
   return;
 }
 int notactive=Mega+10;
 int achan=notactive;
 for(int i =Mega-1;i>=0;i--){
   int j=Mega-1-i;
   if(ssm[i] & 0x20000){
     //printf("%7i A chanel \n",j);
     if(achan==notactive){
       // L0 or L1
       achan=i;
       //printf("First A %i \n",j);
     }else if(achan==i+1){
      //L1 
      printf("L1 at %i \n",j-1);
      achan=notactive;
      ssmrecord *l1 = new ssmrecord(j-1,2);
      qttcab.push_back(l1);
     }else{
      printf("Error \n");
     }
   }else{
      if(achan != notactive){
        printf("L0 at %i \n",j-1);
        achan=notactive;
        ssmrecord *l0 = new ssmrecord(j-1,1);
        qttcab.push_back(l0);
      }
   }
   if(ssm[i] & 0x10000){
     w32 header=(ssm[i]&0xf000)>>12;
     w32 data=ssm[i]&0xfff;
     //printf("%7i Data 0x%1x 0x%3x \n", j,ssm[i]&0xf000,data);
     string dd=ttcadl[header];
     printf("%7i %s 0x%1x 0x%3x \n",j,dd.c_str(),header,data);
     ssmrecord *ss = new ssmrecord(j,header,0,0,data,0);
     qttcab.push_back(ss);
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
