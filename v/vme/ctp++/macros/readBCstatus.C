#include "CTP.h"
#include <cmath>
int main()
{
  int loop=0;
  CTP* ctp = new CTP;
  while(1){
   printf("loop = %i -----------------------------------------------\n",loop);
   if(ctp->readBCStatus(1000000)) break;
   loop++;
   fflush(stdout);
  }
  return 0;
}
