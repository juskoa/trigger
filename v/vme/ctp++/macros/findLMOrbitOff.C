#include "CTP.h"
//---------------------------------------------------------------------
// find LM0 orbit offset wrt to INT usinf SSMs
// 
int findOffset(deque<IRDda>& intir,deque<IRDda>& lm0ir)
{
 printf("INT IR list: ");
 for(w32 i=0;i<intir.size();i++)printIRDda(intir[i]);
 printf("LM0 IR list: ");
 for(w32 i=0;i<lm0ir.size();i++)printIRDda(lm0ir[i]);
 return 0;
} 
void INTBread(CTP* ctp)
{
 INTBOARD *intb=ctp->inter;
 L0BOARD2* l0=ctp->l0;
 intb->StopSSM();
 if(intb->SetMode("ddldat",'a')) return;
 cout << "Starting INT ddldat ssm" << endl;
 cout << "Starting LM ssm" << endl;
 intb->StartSSM();
 l0->ddr3_ssmstart(0);
 usleep(1000000);
 cout << "Stopping" << endl;
 // int
 intb->StopSSM();
 cout << "Reading" << endl;
 intb->ReadSSM();
 intb->getCTPReadOutList();
 //intb->printReadOutList();
 //intb->printIRList();
 //cout << "offset: " << dec << intb->ssmtools.findOffset() << endl;
 // LM
 l0->ddr3_ssmread();
 l0->getOrbits();
 findOffset(intb->getIRs(),l0->getIRs());
}
int main(){
 CTP ctp;
 INTBread(&ctp);
 return 0;
}
 
