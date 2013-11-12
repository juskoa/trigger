#include "CTP.h"
#include <ctime>
//------------------------------------------------------------------
// write to l1 strobe and data for generating classes 1,2,28,50
// start ssm in ingen continuous mode
void L1gen(CTP *ctp){
 L1BOARD *l1=ctp->l1;
 L2BOARD *l2=ctp->l2;
 // set classes 1,49,50
 l1->setClass(1,0xffffff,1,0xe);
 l1->setClass(2,0xffffff,1,0xe);
 l1->setClass(28,0xffffff,1,0xe);
 l1->setClass(48,0xffffff,1,0xe);
 l1->setClass(49,0xffffff,1,0xe);
 l1->setClass(50,0xffffff,1,0xe);
 l1->printClasses();
 cout << "Starting L1 " << l1->getboardbase() << endl;
 // write l1 strobe to ssm in memory
 //char strobe[] ="8";
 //char strobe[]="7800008000003";
 //l1->ssmtools.writeSPP(0,39999-13*4+4,3,strobe);
 // write l1 data - classes 1,2,28,50
 //             50   28     1,2
 //             |    |      |
 //char l1data[]="3800008000003";
 //l1->ssmtools.writeSPP(0,39999-13*4+4,4,l1data);
 // 100 classes all
 char strobe[] ="7ffffffffffff";
 l1->ssmtools.writeSPP(0,39999-13*4+4,3,strobe);
 char l1data[]="3ffffffffffff";
 l1->ssmtools.writeSPP(0,39999-13*4+4,4,l1data);
 //
 // write ssm to hardware
 l1->WritehwSSM();
 // set mode c=contineous
 if(l1->SetMode("ingen",'c')) return;
 cout << "Starting L1 ingen" << endl;
 l1->StartSSM();
}
//------------------------------------------------------------------
// write to l1 strobe and data for generating classes 1,2,28,50
// start l1 ssm in ingen asyn mode
// start l2 ssm in inmon asyn mode
void L1ingenL2inmon(CTP *ctp){
 L1BOARD *l1=ctp->l1;
 BOARD *l2=ctp->l2;
 //l1->setClass(1,0,1);
 //l1->setClass(49,0,1);
 //l1->setClass(50,0,1);
 l1->printClasses();
 cout << "Starting L1 " << l1->getboardbase() << endl;
 l1->StopSSM();
 // write l1 strobe in ssm in memory
 char strobe[] ="1";
 l1->ssmtools.writeSPP(1,39999,3,strobe);
 // write l1 data - classes 1,2,28,50 in ssm in memory
 //             50   28     1,2
 //             |    |      |
 char l1data[]="1000040000003";
 l1->ssmtools.writeSPP(2,39999-13*4+4,4,l1data);
 // write ssm to hardware
 l1->WritehwSSM();
 // set mode
 if(l1->SetMode("ingen",'a')) return;
 //l2->ssmtools.write0();
 //l2->WritehwSSM();
 if(l2->SetMode("inmon",'a')) return;
 time_t init;
 time(&init);
 l2->StartSSM();
 usleep(1000);
 l1->StartSSM();
 time_t final;
 time(&final);
 printf("time %f \n",difftime(final,init));
 cout << " L1 ingen L2 inmon finished" << endl;
 usleep(1000000);
 l2->StopSSM();
 //l2->ReadSSM();
}
//---------------------------------------------------------------------
// read L2 inmon
void L2read(CTP* ctp){
 BOARD *l2=ctp->l2;
 l2->ssmtools.write0();
 l2->StopSSM();
 l2->WritehwSSM();
 if(l2->SetMode("inmon",'a')) return;
 cout << "Starting L2 inmon" << endl;
 l2->StartSSM();
 //cout << "Stopping" << endl;
 //l2->StopSSM();
 //cout << "Reading" << endl;
 usleep(100000);
 //l2->ReadSSM();
 cout << "offset: " << dec << l2->ssmtools.findOffset() << endl;
}
int main(){
 CTP ctp;
 ctp.l0->setClassesToZero();
 ctp.l0->printClasses();
 ctp.l1->setClassesToZero();
 ctp.l2->setClassesToZero();
 //L1ingenL2inmon(&ctp);
 L1gen(&ctp);
 //L2read(&ctp);
}
