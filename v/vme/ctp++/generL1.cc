#include "CTP.h"
#include <ctime>
//------------------------------------------------------------------
// write to l1 strobe and data for generating classes 1,2,28,50
// start ssm in ingen continuous mode
void L1gen(CTP *ctp){
 BOARD *l1=ctp->l1;
 cout << "Starting L1 " << l1->getboardbase() << endl;
 // l1 strobe
 char strobe[] ="1";
 l1->ssmtools.writeSPP(1,39999,3,strobe);
 // l1 data - classes 1,2,28,50
 //             50   28     1,2
 //             |    |      |
 char l1data[]="1000040000003";
 l1->ssmtools.writeSPP(2,39999-13*4+4,4,l1data);
 l1->WritehwSSM();
 if(l1->SetMode("ingen",'c')) return;
 cout << "Starting L1 ingen" << endl;
 l1->StartSSM();
}
//------------------------------------------------------------------
// write to l1 strobe and data for generating classes 1,2,28,50
// start l1 ssm in ingen asyn mode
// start l2 ssm in inmon asyn mode
void L1ingenL2inmon(CTP *ctp){
 BOARD *l1=ctp->l1;
 BOARD *l2=ctp->l2;
 cout << "Starting L1 " << l1->getboardbase() << endl;
 // l1 strobe
 l1->StopSSM();
 char strobe[] ="1";
 l1->ssmtools.writeSPP(1,39999,3,strobe);
 // l1 data - classes 1,2,28,50
 //             50   28     1,2
 //             |    |      |
 char l1data[]="1000040000003";
 l1->ssmtools.writeSPP(2,39999-13*4+4,4,l1data);
 l1->WritehwSSM();
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
 L1ingenL2inmon(&ctp);
 //L1gen(&ctp);
 //L2read(&ctp);
}
