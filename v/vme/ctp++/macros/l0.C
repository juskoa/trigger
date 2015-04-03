#include "CTP.h"
#include <cmath>
int main()
{
 // lm level test 
 CTP *ctp=new CTP;
 L0BOARD* l0=ctp->l0;
 return 0
 // bcmask tests
 w32 pat[3564];
 for(int i=0;i<3564;i++){
   pat[i]=0xfff;
 }
 pat[0]=0;
 l0->writeBCMASKS(pat);
 l0->readBCMASKS();
 //printf("BC1: %i \n",l0->getBC1());
 return 0;
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
