
#include "CTP.h"
int main(){
  setvbuf (stdout, NULL, _IONBF, BUFSIZ);

  unsigned long long frequency[1000];
  for (int i=0; i<1000; i++) {
    frequency[i] = 0;
  }
  unsigned long long **TableL1 = new unsigned long long *[3];
  for(int i = 0; i < 3; ++i) {
    TableL1[i] = new unsigned long long[1000];
  }

  unsigned long long **TableL2 = new unsigned long long *[3];
  for(int i = 0; i < 3; ++i) {
    TableL2[i] = new unsigned long long[1000];
  }

  for (int i=0; i< 3; i++) {
    for (int j=0; j<1000; j++) {
      TableL1[i][j] = 0;
      TableL2[i][j] = 0;
    }
  }
 int TotNtrigs=0;
 w32 err=0;
 CTP ctp;
 w32 vmesp=-1;
 string ltuname("ltu");
 //LTUBOARD *ltu= new LTUBOARD(ltuname.c_str(),0x811000,vmesp);  // 0xb8
 LTUBOARD *ltu= new LTUBOARD(ltuname.c_str(),0x812000,vmesp);  // 0xb8
 w32 vmespt=-1;
 string boardname("ttcit");
 TTCITBOARD *ttc= new TTCITBOARD(boardname.c_str(),0x8a0000,vmespt);
 printf("Running ctpltuttcit \n");
 ltu->Print();
 //ltu->SetStandalone();
  for(int i=0;i<100000;i++){
   ltu->SetMode("inmon",'a');
   ctp.busy->SetDAQBUSY(0xff);
   usleep(50000);
   ttc->startSSM();
   ltu->StartSSM();
   ctp.busy->SetDAQBUSY(0);
   usleep(22000);      // use 24 ms / SSM is ~27ms / 
   //ltu->SLMquit();
   ctp.busy->SetDAQBUSY(0xff);
   usleep(8000);       // run for remaining 3 ms but without generating // ensures complete L0-L1-L2a sequences in SSM
   // stop ssms (why ttc has to be first ?)
   ttc->readSSM();
   ltu->StopSSM();
   ltu->ReadSSM();
   // ttc analysis
   ttc->Dump2quSSM();
   int retttc=ttc->AnalyseSSM();
   char time[30];
   getdatetime(time);
   if(retttc){
    printf("TTC time: %s \n",time);
    ttc->DumptxtSSM();
    string filename="SSM";
    ltu->DumpSSM(filename.c_str());
   }
   // ltu analysis
   // ltu->DumpSSM("SSM");
   TotNtrigs += ltu->AnalyseSSM_Didier(0, TableL1, TableL2, frequency, i);
   err += ltu->GetErrors();
   // ctp.busy->SetDAQBUSY(0);

   if(ltu->GetErrors()){
    printf("------------------------------->dumped at ssm %i . ERRORS: %i\n",i+1,err);
    printf("LTU time: %s \n",time);
    string filename="SSM";
    ltu->DumpSSM(filename.c_str());
    ttc->DumptxtSSM();
    return 1;
   }
   ttc->ClearQueues();
   printf("------> SSM %i \n", i+1);
   if((i%10) == 0) {
     printf("------------------------------->%i ssm done. ERRORS: %i %s\n",i+1,err,time);
      for (int i=0; i<30; i++) {
	  printf("%-5i", i+1);
	  printf("L2Class pattern :  0x%-20llx", TableL2[0][i]);
	  printf("%-15llx      ;   ", TableL2[1][i]);
	  printf("appearances:   %llu \n", TableL2[2][i]);
      }
   }
  }
 printf("-------------------------------> ssm done. ERRORS: %i\n",err);
 


 printf("total number of triggers %i \n", TotNtrigs);
 string filename="SSM"; ltu->DumpSSM(filename.c_str());
 return 0;
}
