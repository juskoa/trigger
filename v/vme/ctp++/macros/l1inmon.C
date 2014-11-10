#include "CTP.h"
#include <cmath>
int main()
{
 CTP *ctp=new CTP;
 L0BOARD* l0=ctp->l0;
 L1BOARD* l1=ctp->l1;
 l1->StopSSM();
 w32 T=0;
 for(int i=0;i<200000;i++){
    //l0->setBC1(T);
    l1->SetMode("inmon",'a');
    l1->StartSSM();
    usleep(50000);
    l1->StopSSM();
    l1->ReadSSM();
    int err=l1->AnalSSM();
    if(err){
      l1->DumpSSM("l1inmon");
      return 1;
    }
    T=l0->getBC1();
    printf("%i-----------------T:%i ERR: %i \n",i,T,err);
    //T++;
    fflush(stdout);
 }
 return 0;
}
