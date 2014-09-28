#include "CTP.h"
#include <cmath>
int main()
{
  CTP* ctp = new CTP;
  //ctp->readBCStatus(1000000);
  ctp->readCounters();
  usleep(1000000);
  ctp->readCounters();
  //
  ctp->l0->printCountersDiff();
  ctp->l1->printCountersDiff();
  ctp->l2->printCountersDiff();
  //ctp->busy->printCountersDiff();
  ctp->fo[0]->printCountersDiff();
  //ctp->fo[0]->printCounters();

  ctp->l0->CheckCountersNoTriggers();
  ctp->l1->CheckCountersNoTriggers();
  ctp->l2->CheckCountersNoTriggers();
  ctp->fo[0]->CheckCountersNoTriggers();
  ctp->inter->CheckCountersNoTriggers();
  return 0;
}
