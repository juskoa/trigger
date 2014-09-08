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
void TTCITBOARD::ClearQueues()
{
 for(w32 i=0;i<qttcab.size();i++) delete qttcab[i];
 qttcab.clear();
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
 * Modified routine : CTP (BUSY board) start/stop
 */ 
int TTCITBOARD::start_stopSSM(BUSYBOARD* bb)
{
 ssm=GetSSM();
 // reset address
 resetSSMAddress();
 for(w32 i=0;i<Mega;i++){
  ssm[i]=0;
  vmew(READ_SSM_WORD,0);
 }
 // switch off triggers
 bb->SetDAQBUSY(0xff);
 // wait to clean buffers
 usleep(20000);
 // reset again - this makes ttc wait for input
 resetSSMAddress();
 // start triggers
 bb->SetDAQBUSY(0x0);
 usleep(22000);
 // stop triggers
 bb->SetDAQBUSY(0xff);
 usleep(8000);
 while(vmer(READ_SSM_ADDRESS)==0)continue; 
 usleep(50000);
 //printf("after reset status: 0x%x\n",getStatus());
 usleep(100000);
 //printf("after usleep and control 2 status: 0x%x\n",getStatus());
 //printf("# word= 0x%x\n",vmer(READ_SSM_ADDRESS));
 //w32 stat=getStatus();
 //printf("after usleep and control 3 status: 0x%x\n",stat);

 //resetSSMAddress(); 
 for(int i=0;i<Mega;i++){
  ssm[i]=vmer(READ_SSM_WORD);
  //usleep(100000);
 }
 bb->SetDAQBUSY(0x0);
 return 0;
}
/*
 * Modified routine : LTU start/stop
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
 usleep(24000);
 ltu->SLMquit();
 usleep(3000);
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
 * Analyse que dump.
 * Assume that TTC sequence start from beginning
 *
*/
int TTCITBOARD::AnalyseSSM()
{
 if(qttcab.size() ==  0){
   printf("Empty ssm memory \n");
   return 0;
 }
 w32 issm0=qttcab[0]->issm;
 if((issm0 != 262)){
 //if((issm0 != 262) && (issm0 != 266) && (issm0 != 265)){
   //printf("Error: first L1 expected at 262,266  but found at %i \n",qttcab[0]->issm);
   printf("Warning: first L1 expected at 262  but found at %i \n",qttcab[0]->issm);
   return 2;
 }
 w32 cl0=1,cl1=0,cl1m=0,cl2a=0,cl2r=0;
 deque<w32> L1;
 deque<w32*> L1m;
 deque<w32*> L2m;
 w32 L0L1time=260;
 w32 l0=2;
 w32 l1mes[NL1words+1],l2mes[NL2words+1]; // one flag
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
        return 1;
      }
      l0=issm;
      cl0++;
     } else {
      // L1 received
      if((issm-l0) != L0L1time){
        printf("L0L1 time violation - L1 arrived not in time: %i %i %i \n",issm-l0,issm,l0);
        return 1;
      }
      L1.push_back(issm);
      cl1++;
     }
   } else if(ss->ttcode == 1){
     // L1h
     if(l1mes[0]){
       printf("Error: L1 shorter than 9 issm=%i \n",issm);
       return 1;
     }else{
       l1mes[0]=1;
       l1mes[1]=ss->tdata;
       il1=2;
       cl1m++;
     }
   } else if(ss->ttcode == 2){
     //L1 data
     if(l1mes[0]==0){
       printf("Error: L1 data without header issm=%i \n", issm);
       return 1;
     }
     l1mes[il1]=ss->tdata;
     //printf("il1= %i \n",il1);
     if(il1==NL1words){
       // last word of mess
       il1=0;
       l1mes[0]=0;      
       //L1m.push_back(issm);
       w32* pp = new w32[NL1words+1];
       pp[0]=issm;
       for(int ii=1;ii<NL1words+1;ii++)pp[ii]=l1mes[ii];
       L1m.push_back(pp); // thos to be changed for pointers
     }
     il1++;
   } else if(ss->ttcode == 3){
       //L2h
       if(l2mes[0]){
         printf("Error: L2 shorter than 13 issm= %i \n",issm);
         return 1;
       }else{
         l2mes[0]=1;
         l2mes[1]=ss->tdata;
         il2=2;
	 cl2a++;
       }
   } else if(ss->ttcode == 4){
       // L2 data
       if(l2mes[0]==0){
         printf("Error: L2 data without header issm=%i\n",issm);
         return 1;
       }
       l2mes[il2]=ss->tdata;
       if(il2==NL2words){
         // last word
         il2=0;
         l2mes[0]=0;
         //L2m.push_back(issm);
         w32* pp = new w32[NL2words+1];
         pp[0]=issm;
         for(int ii=1;ii<NL2words+1;ii++)pp[ii]=l2mes[ii];
         L2m.push_back(pp);
       }
       il2++;
   }else if(ss->ttcode == 5){
         w32* pp = new w32[NL2words+1];
         pp[0]=issm;
         pp[1]=ss->tdata;
         for(int ii=2;ii<NL2words+1;ii++)pp[ii]=0;
         L2m.push_back(pp);
         cl2r++; 
   }else{ 
    printf("Error: unexpected code %i at %i \n",ss->ttcode,issm);
    return 1;
   }   
 }
 printf("No error detected: L0: %i L1: %i L1m: %i L2a: %i L2r: %i \n",cl0,cl1,cl1m,cl2a,cl2r);
 //return 0;
 // Measurement of timeout due to the fifos
 //printf("###### of L1: %i , # if L1 mess: %i , # of L2 mess: %i \n",L1.size(),L1m.size(),L2m.size());
 if(L1.size() == 0){
   printf("No L1 \n");
   return 0;
 }
 if(L1.size() != L2m.size()){
   printf("Error: different # of L1 and L2m : L1 %i L1m %i L2m %i\n",L1.size(),L1m.size(),L2m.size());
   return 1;
 }
 // Looking for max delays for L1 and L2messages
 w32 delmaxL2=0;
 w32 delmaxL1=0;
 w32 bcl1=L1[0]%3564;
 int delta0 = L2m[0][1] - bcl1;
 //printf("%i %i %i \n", L1[0],bcl1,delta0);
 if(delta0<0) delta0=delta0+3564;
 for(w32 i=0;i<L2m.size();i++){
    w32 issml1=L1[i];
    w32 issml2m=L2m[i][0];
    w32 bcl2m=L2m[i][1];
    w32 delay1=L1m[i][0]-issml1;
    w32 delay2=issml2m-issml1;
    if(delmaxL2<delay2)delmaxL2=delay2;
    if(delmaxL1<delay1)delmaxL1=delay1;
    w32 bcl1=issml1%3564;
    int delta=bcl2m-bcl1;
    if(delta<0) delta = delta+3564;
    //printf("L1 issm: %7i  L1 bc: %4i L2m issm: %7i L2m bc: %4i delta: %i delay: %i\n",issml1,bcl1,issml2m,bcl2m,delta,delay2);
    if(delta0 != delta){
      printf("Error: delta0= %i \n",delta0);
      return 1;
    }
 }
 printf("Max L2 delay: %i Max L1 delay: %i \n",delmaxL2,delmaxL1);
 return 0;
}
/*
 * SSM dump as ssmrecord que used for AnalyseSSM
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
     if(achan==notactive){
       // L0 or L1
       achan=i;
     }else if(achan==i+1){
      //L1 
      //printf("L1 at %i \n",j-1);
      achan=notactive;
      ssmrecord *l1 = new ssmrecord(j-1,2);
      qttcab.push_back(l1);
     }else{
      printf("Error \n");
     }
   }else{
      if(achan != notactive){
        //printf("L0 at %i \n",j-1);
        achan=notactive;
        ssmrecord *l0 = new ssmrecord(j-1,1);
        qttcab.push_back(l0);
      }
   }
   if(ssm[i] & 0x10000){
     w32 header=(ssm[i]&0xf000)>>12;
     w32 data=ssm[i]&0xfff;
     ////printf("%7i Data 0x%1x 0x%3x \n", j,ssm[i]&0xf000,data);
     string dd=ttcadl[header];
     //printf("%7i %s 0x%1x 0x%3x \n",j,dd.c_str(),header,data);
     ssmrecord *ss = new ssmrecord(j,header,0,0,data,0);
     qttcab.push_back(ss);
   }
 } 
}
/*============================================================================
 * Text dump for visual debuging
 */
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
