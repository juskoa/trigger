#include "CTP.h"
int main(){
 w32 err=0;
 //CTP ctp;
 w32 vmesp=-1;
 string ltuname("ltu");
 LTUBOARD *ltu= new LTUBOARD(ltuname.c_str(),0x810000,vmesp);  // 0xb8
 ltu->Print();
 //ltu->SetStandalone();
  for(int i=0;i<101;i++){
   ltu->SetMode("inmon",'a');
   //ltu->SLMstart();
   ltu->StartSSM();
   usleep(24000);
   //ltu->SLMquit();
   usleep(3000);
   ltu->StopSSM();
   ltu->ReadSSM();
   //ltu->DumpSSM("SSM");
   ltu->AnalSSM();
   err += ltu->GetErrors();



   if(ltu->GetErrors()){
    string filename="SSM";
    ltu->DumpSSM(filename.c_str());
    return 1;
   }
   printf("------> SSM %i \n", i+1);
   if((i%10) == 0)printf("------------------------------->%i ssm done. ERRORS: %i\n",i+1,err);
  }
 printf("-------------------------------> ssm done. ERRORS: %i\n",err);
 return 0;
}
