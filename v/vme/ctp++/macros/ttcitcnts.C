#include "TTCITBOARD.h"
int main(){
 int vmesp=-1;
 string boardname("ttcit");
 TTCITBOARD *ttc= new TTCITBOARD(boardname.c_str(),0x8a0000,vmesp);
 printf("vsp= %i \n",ttc->getvsp());
 w32 ver= ttc->getFPGAversion();
 printf("Version: 0x%x %i\n",ver,ver);
 for(w32 i=200;i<301;i++){
    ttc->ReadAllCounters(i);
 }
 return 0;
}
