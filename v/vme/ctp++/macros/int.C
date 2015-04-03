#include "CTP.h"
#include <cmath>
int main()
{
 CTP* ctp = new CTP;
 for(int i=0;i<3564;i++){
   ctp->inter->setBCOFFSET(i);
   ctp->inter->readcopyCounters();
   //ctp->inter->printCounters();
   usleep(100000);
   ctp->inter->readcopyCounters();
   //ctp->inter->printCountersDiff();
   ctp->inter->readCountersDiff();
   printf("%i oe=%i \n",i,ctp->inter->getCounterDiff(11));
   fflush(stdout);
 }
 return 0;
}
