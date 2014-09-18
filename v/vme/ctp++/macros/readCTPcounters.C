#include "CTP.h"
#include <cmath>
int main()
{
  CTP* ctp = new CTP;
  ctp->readBCStatus(100);
  //ctp->l1->readCounters();
  //ctp->l1->printCounters();
  return 0;
}
