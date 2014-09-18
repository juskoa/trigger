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
 ttc->DumptxtSSM();
 ttc->Dump2quSSM();
 ttc->AnalyseSSM();
 return 0;
}
