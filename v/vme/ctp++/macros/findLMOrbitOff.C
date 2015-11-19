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
int INTmeasure(CTP* ctp)
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
 if(l0->getOrbits()){ 
   printf("ERROR in getting orbit \n");
   return 1;
 }
 if(findOffset2(intb->getIRs(),l0->getIRs())) return 1;
 printf("OFFSET: 0x%x\n",l0->getOrbitOffset());
 return 0;
}
/////////////////////////////////////////////////////////////
int INTset(CTP* ctp, int what,w32 del, w32 off)
{
 L0BOARD2* l0=ctp->l0;
 w32 offset=0;
 if(del==0) return 0;
 offset=(del+off)%0x8000000;
 l0->setOrbitOffset(offset);
 return 0;
}
int INTconfigctp(CTP* ctp)
{
 L0BOARD2* l0=ctp->l0;
 // input switch
 l0->setSwitchAll0();
 l0->setSwitch(3,3);
 //l0->printSwitch();
 // enable rnd on input 3
 l0->setINRND1_24(3);
 // set lm rnd rate
 l0->setLMRND1rate(0xf000);
 // INT FUN 3
 for(int i=0;i<16;i++){
   w32 word=i+(0xf0f0<<16);
   l0->writeL0INTfunction(1,word);
 }
 // select fun1
 l0->setL0INTSEL(1);
 return 1;
}
///////////////////////////////////////////////////////////
int main(int argc,char **argv){
 CTP ctp;
 if(argc==1)
 {
  INTmeasure(&ctp);
 }else if(argc==3){
  w32 del=atoi(argv[1]);
  w32 off=atoi(argv[2]);
  INTset(&ctp,2,del,off);
 }else if(argc==2){
  INTconfigctp(&ctp);
 }else{
  printf("Expecting 0 or 2 arguments \n");
 }
 return 0;
}
 
