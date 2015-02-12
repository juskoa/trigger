#include "CTP.h"
#include <cmath>
int main()
{
 CTP *ctp=new CTP;
 L0BOARD* l0=ctp->l0;
 //printf("BC1: %i \n",l0->getBC1());
 l0->ddr3_reset();
 l0->ddr3_status();
 usleep(1000000);
 l0->ddr3_ssmstart(0);
 l0->ddr3_ssmread();
 string ssm("test");
 l0->DumpSSM(ssm.c_str(),2);
 return 0;
}
