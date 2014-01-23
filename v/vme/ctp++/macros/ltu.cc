#include "CTP.h"
int main(){
 w32 err=0;
 //CTP ctp;
 w32 vmesp=-1;
 string ltuname("ltu");
 LTUBOARD *ltu= new LTUBOARD(ltuname.c_str(),0x811000,vmesp);
 ltu->Print();
 //ltu->SetStandalone();
 for(int i=0;i<10000;i++){
   ltu->SetMode("inmon",'a');
   ltu->StartSSM();
   usleep(50000);
   ltu->StopSSM();
   ltu->ReadSSM();
   //ltu->ssmtools.dumpSSM("SSM");
   ltu->AnalSSM();
   err += ltu->GetErrors();
   if(ltu->GetErrors()){
    string filename="ssm";
    ltu->ssmtools.dumpSSM(filename.c_str());
    return 1;
   }
   if((i%10) == 0)printf("------------------------------->%i ssm done. ERRORS: %i\n",i+1,err);
 }
 printf("-------------------------------> ssm done. ERRORS: %i\n",err);
 return 0;
}
