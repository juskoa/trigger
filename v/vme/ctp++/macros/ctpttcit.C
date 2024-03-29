#include "CTP.h"
#include "TTCITBOARD.h"
#include <cmath>
int main()
{
  int ret=0;
  w32 ner=0;
  CTP* ctp = new CTP;
  printf("DAQ_BUSY before: 0x%x \n",ctp->busy->GetDAQBUSY());
  int vmesp=-1;
  string boardname("ttcit");
  TTCITBOARD *ttc= new TTCITBOARD(boardname.c_str(),0x8a0000,vmesp);
  for(int i=0;i<1000000;i++){
  //for(int i=0;i<2;i++){
     printf("Snapshot # %i =================================================================== ERR=%i\n",i,ner);
     if(ttc->start_stopSSM(ctp->busy)) return 1;
     ///ttc->start_stopSSM();
     ttc->Dump2quSSM();
     //ttc->DumptxtSSM();
     ret=ttc->AnalyseSSM();
     if(ret){
       char time[30];
       getdatetime(time);
       printf("Time: %s \n",time);
       ttc->DumptxtSSM();
       ttc->DumpqueSSM2file("test.txt");
       ner++;
       //return 1;
     }
     ttc->ClearQueues();
     fflush(stdout);
  }
  //printf("DAQ_BUSY after: 0x%x \n",ctp->busy->GetDAQBUSY());

  return 0;
}
