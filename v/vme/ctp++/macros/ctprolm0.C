#include "CTP.h"
//---------------------------------------------------------------------
// read INT inmon
void INTBread(CTP* ctp)
{
 INTBOARD *intb=ctp->inter;
 L2BOARD *l2=ctp->l2;
 L0BOARD* l0=ctp->l0;
 intb->StopSSM();
 l2->StopSSM();
 if(intb->SetMode("ddldat",'a')) return;
 if(l2->SetMode("outmon",'a')) return;
 l0->ddr3_reset();
 l0->ddr3_status();
 usleep(1000000);
 cout << "Starting INT ddldat" << endl;
 cout << "Starting L2 outmon" << endl;
 l0->ddr3_ssmstart(1);
 l2->StartSSM();
 intb->StartSSM();
 usleep(1000000);
 //cout << "Stopping" << endl;
 intb->StopSSM();
 l2->StopSSM();
 cout << "Reading" << endl;
 intb->ReadSSM();
 //intb->ssmtools.dumpSSM("intboard");
 intb->getCTPReadOutList();
 intb->printReadOutList();
 intb->printIRList();
 l2->ReadSSM();
 //l2->ssmtools.dumpSSM("l2board");
 l2->L2DataBackplane();
 l2->printL2DataBackplane();
 //cout << "offset: " << dec << intb->ssmtools.findOffset() << endl;
 l0->ddr3_ssmread();
 l0->getOrbits();
}
int main(){
 CTP ctp;
 INTBread(&ctp);
 return 0;
}
