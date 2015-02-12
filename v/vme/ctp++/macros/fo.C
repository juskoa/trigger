#include "CTP.h"
int main()
{
  int ret=0;
  CTP* ctp = new CTP;
  //FOBOARD* fo=ctp->fo[5];
  FOBOARD* fo=ctp->fo[0];
  for(int i=0;i<10000;i++){
    printf("Checking FO ===================> %i \n",i);
    fo->SetMode("inmonl0",'a');
    fo->StartSSM();
    usleep(50000);
    fo->StopSSM();
    fo->ReadSSM();
    ret=fo->AnalSSMinmonl0();
    if(ret){
      //fo->DumpSSM("fo6");
      return ret;
    }
  }
  return ret;
}
