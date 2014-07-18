#include "CTP.h"
int main(){
 w32 err=0;
 //CTP ctp;
 w32 vmesp=-1;
 string ltuname("ltu");
 LTUBOARD *ltu= new LTUBOARD(ltuname.c_str(),0x811000,vmesp);
 ltu->Print();
 //ltu->SetStandalone();
 string filename="ssm";
 for(int i=0;i<100000;i++){
   ltu->SetMode("inmon",'a');
   ltu->StartSSM();
   usleep(50000);
   ltu->StopSSM();
   ltu->ReadSSM();
   ///ltu->ReadSSMDump(filename.c_str());
   //ltu->AnalSSM();
   //ltu->AnalTotalSSM();
   ltu->AnalTotalSSM2();
   err += ltu->GetErrors();
   if(ltu->GetErrors()){
    ltu->DumpSSM(filename.c_str());
    return 1;
   }
   if((i%100) == 0)printf("------------------------------->%i ssm done. ERRORS: %i\n",i+1,err);
   fflush(stdout);
 }
 printf("-------------------------------> ssm done. ERRORS: %i\n",err);
 return 0;
}
