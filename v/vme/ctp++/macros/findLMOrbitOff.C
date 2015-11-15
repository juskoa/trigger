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
         printf("findOrbit2 error: ERROR:deltas not equal \n");
         return 1;
       }
     }
     break;
   }
 }
 printf("DELTA: 0x%x\n",deltaold);  
 return 0;
} 
int INTBread(CTP* ctp, int what,w32 del, w32 off)
{
 INTBOARD *intb=ctp->inter;
 L0BOARD2* l0=ctp->l0;
 switch(what)
 {
 case 1:
 {
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
 if(l0->getOrbits()){ 
   printf("ERROR in getting orbit \n");
   return 1;
 }
 if(findOffset2(intb->getIRs(),l0->getIRs())) return 1;
 printf("OFFSET: 0x%x\n",l0->getOrbitOffset());
 return 0;
 }
 case 2:
 {
  //w32 offset=(del+off-1)%0xffffff;
  w32 offset=(del+off)%0xffffff;
  l0->setOrbitOffset(offset);
  return 0;
 }
 default:
  return 0;
 }
}
int main(int argc,char **argv){
 CTP ctp;
 if(argc==1)
 {
  INTBread(&ctp,1,0,0);
 }else if(argc==3){
  w32 del=atoi(argv[1]);
  w32 off=atoi(argv[2]);
  INTBread(&ctp,2,del,off);
 }else{
  printf("Expecting 0 or 2 arguments \n");
 }
 return 0;
}
 
