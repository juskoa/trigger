#include "CTP.h"
//---------------------------------------------------------------------
// read INT inmon
void INTBread(CTP* ctp)
{
 INTBOARD *intb=ctp->inter;
 intb->StopSSM();
 if(intb->SetMode("ddldat",'a')) return;
 cout << "Starting INT ddldat" << endl;
 intb->StartSSM();
 usleep(100000);
 cout << "Stopping" << endl;
 intb->StopSSM();
 cout << "Reading" << endl;
 intb->ReadSSM();
 intb->ssmtools.dumpSSM("intboard");
 intb->getCTPReadOutList();
 intb->printReadOutList();
 intb->printIRList();
 //cout << "offset: " << dec << intb->ssmtools.findOffset() << endl;
}
int main(){
 CTP ctp;
 INTBread(&ctp);
 return 0;
}
