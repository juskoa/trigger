#include "CTP.h"
//---------------------------------------------------------------------
// find LM0 orbit offset wrt to INT usinf SSMs
// 
int findOffset(deque<IRDda>& intir,deque<IRDda>& lm0ir)
{
 w32 intlen=intir.size();
 w32 lm0len=lm0ir.size();
 printf("INT IR list: \n");
 for(w32 i=0;i<intlen;i++)printIRDda(intir[i]);
 printf("LM0 IR list: \n");
 for(w32 i=0;i<lm0len;i++)printIRDda(lm0ir[i]);
 // start comp from lmo
 w32 min=lm0len;
 if(min>intlen)min=intlen;
 printf("lengths: int IR list: %i lm0 IR list: %i min: %i -----------------------------------------------------\n",intlen,lm0len,min);
 for(w32 i=0;i<min;i++){
  w32 ir0=lm0ir[i].orbit;
  w32 iri=intir[i].orbit;
  w32 delta;
  if(ir0>iri){
    delta=0xffffff-ir0+iri+1;
  }else{
    delta=iri-ir0;
  }
  printf("lm0 orb int orb delta: 0x%x 0x%x 0x%x\n",ir0,iri,delta);  
 }
 return 0;
} 
//--------------------------------------------------------------------------------
// look also to bcid, should be more robust
//
int findOffset2(deque<IRDda>& intir,deque<IRDda>& lm0ir)
{
 w32 intlen=intir.size();
 w32 lm0len=lm0ir.size();
 printf("INT IR list: \n");
 for(w32 i=0;i<intlen;i++)printIRDda(intir[i]); 
 printf("LM0 IR list: \n");
 for(w32 i=0;i<lm0len;i++)printIRDda(lm0ir[i]);
 // start comp from lmo
 printf("lengths: int IR list: %i lm0 IR list: %i -----------------------------------------------------\n",intlen,lm0len);
 w32 deltaold=0xffffffff;
 for(w32 i=1;i<lm0len;i++){   // skip first because it is 0
   w32 ir0=lm0ir[i].orbit;
   if(lm0ir[i].bc[0]==0) continue;
   // lm0 int with some int found
   for(w32 j=(i-1);j<intlen;j++){
     if(intir[j].bc[0]==0) continue;
     if(intir[j].bc[0]!=lm0ir[i].bc[0]) continue;
     // here can be loop on all bcs, only one now
     w32 delta;
     w32 iri=intir[j].orbit;
     if(ir0>iri){
       delta=0xffffff-ir0+iri+1;
     }else{
       delta=iri-ir0;
     }
     printf("lm0 orb int orb delta: 0x%x 0x%x 0x%x\n",ir0,iri,delta);  
     if(deltaold==0xffffffff)deltaold=delta;
     else{
       if(delta != deltaold){
         printf("findOrbit2 error: deltas not equal \n");
         return 1;
       }
     }
     break;
   }
 }
 printf("delta: 0x%x\n",deltaold);  
 return 0;
} 
int INTBread(CTP* ctp)
{
 INTBOARD *intb=ctp->inter;
 L0BOARD2* l0=ctp->l0;
 intb->StopSSM();
 if(intb->SetMode("ddldat",'a')) return 1;
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
 if(l0->getOrbits()) return 1;
 findOffset2(intb->getIRs(),l0->getIRs());
 return 0;
}
int main(){
 CTP ctp;
 INTBread(&ctp);
 return 0;
}
 
