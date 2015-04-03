#include "CTP.h"
//---------------------------------------------------------------------
// read INT inmon
void INTBread(CTP* ctp)
{
 FOBOARD *fo1=ctp->fo[0];
 L2BOARD *l2=ctp->l2;
 //intb->StopSSM();
 l2->StopSSM();
 if(fo1->SetMode("inmonl2",'a')) return;
 if(l2->SetMode("outmon",'a')) return;
 cout << "Starting FO inmonl2" << endl;
 cout << "Starting L2 outmon" << endl;
 l2->StartSSM();
 fo1->StartSSM();
 usleep(100000);
 //cout << "Stopping" << endl;
 fo1->StopSSM();
 l2->StopSSM();
 cout << "Reading" << endl;
 fo1->ReadSSM();
 fo1->L2DataBackplane();
 fo1->printL2DataBackplane();
 //intb->ssmtools.dumpSSM("intboard");
 //intb->getCTPReadOutList();
 //intb->printReadOutList();
 //intb->printIRList();
 l2->ReadSSM();
 //l2->ssmtools.dumpSSM("l2board");
 l2->L2DataBackplane();
 l2->printL2DataBackplane();
 cout << "l2 offset: " << l2->ssmtools.findOffset() << endl;
 cout << "fo offset: " << fo1->ssmtools.findOffset() << endl;
 //cout << "offset: " << dec << intb->ssmtools.findOffset() << endl;
}
int main(){
 CTP ctp;
 INTBread(&ctp);
 return 0;
}
