#include "CTP.h"
#include <cmath>
#include <stdlib.h>
int main()
{
 CTP *ctp=new CTP;
 L0BOARD* l0=ctp->l0;
 L1BOARD* l1=ctp->l1;
 l1->StopSSM();
 w32 T=0;
 w32 counte=0;
 for(int i=0;i<200000;i++){
    //l0->setBC1(T);
    l1->SetMode("inmon",'a');
    l1->StartSSM();
    usleep(50000);
    l1->StopSSM();
    l1->ReadSSM();
    int err=l1->AnalSSM();
    if(err){
      string ssmdump("l1inmon");
      char buf[5];
      sprintf(buf,"%d",i);
      string num(buf);
      ssmdump=ssmdump+num;
      l1->DumpSSM(ssmdump.c_str());
      if(counte==1)return 1;
      counte++;
    }
    T=l0->getBC1();
    printf("%i-----------------T:%i ERR: %i \n",i,T,err);
    //T++;
    fflush(stdout);
 }
 return 0;
}
