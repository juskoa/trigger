#include "CTP.h"
#include <cmath>
int main(int argc, char* argv[]) 
{
 printf("# fo args: %i \n",argc);
 for(int i=0;i<argc;i++) printf("%i  %s \n",i,argv[i]);
 int iwhat=256;
 if(argc==2)iwhat=atoi(argv[1]);
 // lm level test 
 CTP *ctp=new CTP;
 L0BOARD* l0=ctp->l0;
 //L1BOARD* l1=ctp->l1;
 //L2BOARD* l2=ctp->l2;
 BUSYBOARD *busy=ctp->busy;
 switch(iwhat){
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
    return 0;
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
 case 1:
    // bcmask tests
    w32 pat[3564];
    for(int i=0;i<3564;i++){
      pat[i]=0xf;
      if(i%10)pat[i]=0;
      //pat[i]=0xaaa;
      //if(i%3)pat[i]=0;
    }
    pat[0]=1;
    l0->writeBCMASKS(pat);
    l0->readBCMASKS();
    //printf("BC1: %i \n",l0->getBC1());
    printf("bcmasks written \n");
    return 0;
 case 2:
    {
    // ssm test
    //string ssm1("ctp4");
    //l0->ReadSSMDump(ssm1.c_str());
    //l0->SetSSM1(l0->GetSSM());
    //l0->getOrbits();
    //return 0;
    //
    //l0->ddr3_reset();
    //l0->ddr3_status();
    //usleep(1000000);
    l0->ddr3_ssmstart(0);
    usleep(1000000);
    l0->ddr3_ssmread();
    string ssm("test3");
    l0->DumpSSM(ssm.c_str(),3);
    //l0->DumpSSM(ssm.c_str(),1);
    l0->getOrbits();
    return 0;
    }
  case 3:
    l0->readHWClasses();
    l0->printClassConfiguration();
    l0->convertL02LMClassAll();
    l0->writeHWClasses();
    l0->readHWClasses();
    l0->printClassConfiguration();
    return 0; 
  case 4:
    ctp->setSWtrigger('s',0xff,7,1);
    ctp->startSWtrigger('s',1);
    //ctp->clearSWTriggerFlags();
    return 0;
  case 5:
    {
    bool mask[256];
    string ff("a");
    if(l0->calcLUT(ff,mask)) return 0;
    l0->setFunction(1,mask);
    return 0;
    }
  case 6:
    busy->measurephase();
    return 0;
  default:
    printf("0 = read counters,ssm; dump ssm\n");
    printf("1 = write bcmasks \n");
    printf("2 = simple ssm read and dump\n");
    printf("3 = convert didier config to LM level\n");
    printf("4 = software trigger\n");
    printf("5 = L0/LM fucntions \n");
    printf("6 = measure orbit phase\n");
    return 0;
 }
}
