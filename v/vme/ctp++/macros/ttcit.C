#include "TTCITBOARD.h"
#include "libctp++.h"
int main(){
 w32 err=0;
 int NN=10000000;
 int vmesp=-1;
 string boardname("ttcit");
 TTCITBOARD *ttc= new TTCITBOARD(boardname.c_str(),0x8a0000,vmesp);
 printf("vsp= %i \n",ttc->getvsp());
 w32 ver= ttc->getFPGAversion();
 printf("Version: 0x%x %i\n",ver,ver);
 for(int i=0;i<NN;i++){
   printf("--------------------> i= %i \n",i);
   ttc->start_stopSSM();
   ttc->Dump2quSSM();
   int ret=ttc->AnalyseSSM();
   if(ret){
     char time[30];
     getdatetime(time);
     printf("Time: %s \n",time);
     ttc->DumptxtSSM();
     //return 1;
   }
   ttc->ClearQueues();
   fflush(stdout);
 }
 return 0;
}
