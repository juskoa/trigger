#include "TTCITBOARD.h"
int main(){
 w32 err=0;
 //CTP ctp;
 int vmesp=-1;
 string boardname("ttcit");
 TTCITBOARD *ttc= new TTCITBOARD(boardname.c_str(),0x8a0000,vmesp);
 printf("vsp= %i \n",ttc->getvsp());
 w32 ver= ttc->getFPGAversion();
 printf("Version: 0x%x %i\n",ver,ver);
 ttc->start_stopSSM();
 //ttc->ssmtools.dumpSSM("ttcit");
 /*
 //
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
 */
 return 0;
}
