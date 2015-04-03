#include "CTP.h"
#include "TTCITBOARD.h"
#include <cmath>
int main()
{
  int ret=0;
  CTP* ctp = new CTP;
  printf("DAQ_BUSY before: 0x%x \n",ctp->busy->GetDAQBUSY());
  BUSYBOARD* bb=ctp->busy;
  for(int i=0;i<1;i++){
     printf("Snapshot # %i ===================================================================\n",i);
     // wait to clean fifo
     bb->SetDAQBUSY(0xff);
     usleep(20000);
     // start triggers
     bb->SetDAQBUSY(0x0);
     usleep(22000);
     // stop triggers
     bb->SetDAQBUSY(0xff);
     usleep(8000);
     fflush(stdout);
  }
  //printf("DAQ_BUSY after: 0x%x \n",ctp->busy->GetDAQBUSY());

  return ret;
}
