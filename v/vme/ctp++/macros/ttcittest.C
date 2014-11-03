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
 for(int i=0;i<10000;i++){
  printf("------------------------> SSM # %i \n",i);
  ttc->start_stopSSM();
  err=ttc->CheckClassPatternSSM();
  if(err){
    ttc->DumptxtSSM();
    return 0;
  }
 }
 //ttc->Dump2quSSM();
 //ttc->AnalyseSSM();
 return 0;
}
