#include "CTP.h"
#include <cmath>
int main()
{
 // lm level test 
 CTP *ctp=new CTP;
 L0BOARD* l0=ctp->l0;
 L1BOARD* l1=ctp->l1;
 switch(1){
 case 0:
    // prepare l1
    //l1->StopSSM();
    //l1->SetMode("inmon",'a');
    //l0->configL0classesonly();
    //return 0;
    l0->readcopyCounters();
    usleep(10000);
    l0->readcopyCounters();
    l0->printCountersDiff();
    l0->ddr3_reset();
    l0->SetMode("outmon",'a');
    usleep(5000000);
    //l1->StartSSM();
    //l0->ddr3_ssmstart(0);
    l0->StartSSM();
    l0->ddr3_ssmread();
    l0->DumpSSMLM("dummy");
    l0->AnalSSM();
    //l0->DumpSSM("dummy",2);
    //l1->StopSSM();
    //l1->ReadSSM();
    return 0;
 case (1):
    // bcmask tests
    w32 pat[3564];
    for(int i=0;i<3564;i++){
      pat[i]=0x0;
      //pat[i]=0xaaa;
      //if(i%3)pat[i]=0;
    }
    pat[0]=1;
    l0->writeBCMASKS(pat);
    l0->readBCMASKS();
    //printf("BC1: %i \n",l0->getBC1());
    printf("bcmasks written \n");
    return 0;
 case(2):
    // ssm tests
    l0->ddr3_reset();
    l0->ddr3_status();
    usleep(1000000);
    l0->ddr3_ssmstart(0);
    l0->ddr3_ssmread();
    string ssm("test1");
    //l0->DumpSSM(ssm.c_str(),2);
    l0->DumpSSM(ssm.c_str(),1);
    return 0;
 }
}
