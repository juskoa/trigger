#include "CTP.h"
#include <cmath>
#define NCLASSES 100
// configure boards
int main()
{
  CTP* ctp = new CTP;
  //FOBOARD* fo=ctp->fo[5];
  FOBOARD* fo=ctp->fo[0];
  fo->SetMode("inmonl2",'a');
  fo->StartSSM();
  usleep(50000);
  fo->StopSSM();
  fo->ReadSSM();
  fo->AnalSSMinmonl2();
  fo->DumpSSM("fo6");
  return 0;
}
