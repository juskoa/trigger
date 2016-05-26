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
int findOffset2(deque<IRDda>& intir,deque<IRDda>& lm0ir,w32 &deltaret)
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
     bool eq=1;
     for(int k=0;k<255;k++){
       if(intir[j].bc[k]!=lm0ir[i].bc[k]){
	 eq=0;
         break;
       }
     }
     if(eq==0)continue;
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
         deltaret=0xffffffff;
         return 1;
       }
     }
     break;
   }
 }
 deltaret=deltaold;
 printf("DELTA: 0x%x\n",deltaold);  
 return 0;
} 
int INTmeasure(CTP* ctp,int what)
/*
 * if what==1 it also sets new offset
 */ 
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
 if(intb->getCTPReadOutList()){
  printf("ERROR in getting INT board IR/Redaout \n");
  intb->DumpSSM("intb");
  return 1;
 }
 //intb->printReadOutList();
 //intb->printIRList();
 //cout << "offset: " << dec << intb->ssmtools.findOffset() << endl;
 // LM
 cout << "LM status:" << endl;
 l0->ddr3_status();
 //
 l0->ddr3_ssmread();
 if(l0->getOrbits(1,2,3)){ 
   printf("ERROR in getting orbit \n");
   return 1;
 }
 w32 delta;
 if(findOffset2(intb->getIRs(),l0->getIRs(),delta)) return 1;
 w32 offset=l0->getOrbitOffset();
 printf("OFFSET: 0x%x\n",offset);
 if(what){
   if(delta==0) return 0;
   w32 off;
   off=(delta+offset)%0x8000000;
   l0->setOrbitOffset(off);
   printf("Warning: orbitoffset changed: oldoffset: 0x%x newoffset: 0x%x \n",offset,off);
 }
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
 l0->setSwitch(1,1);
 l0->setSwitch(2,2);
 l0->setSwitch(3,3);
 //l0->printSwitch();
 // enable rnd on input 3
 l0->setINRND1_24(0x7);
 // set lm rnd rate
 l0->setLMRND1rate(0x3e000);
 // INT FUN 3
 for(int i=0;i<16;i++){
   w32 word=i+(0xf0f0<<16);
   l0->writeL0INTfunction(1,word);
   w32 word2=i+(0x8888<<16);
   l0->writeL0INTfunction(2,word2);
 }
 // select fun1
 l0->setL0INTSEL(0x21);
 return 0;
}
int INTconfigctpClass(CTP* ctp)
{
 L0BOARD2* l0=ctp->l0;
 L1BOARD* l1=ctp->l1;
 L2BOARD* l2=ctp->l2;
 BUSYBOARD* busy=ctp->busy;
 l0->setClassConditionL0(1,0xfffffb,0x3,0x3,0xfff);
 l0->setClassVetoesL0(1,0x1,0,0);
 l1->setClass(1,0xffffff,1,0xf);
 l2->setClass(1,0xfff,1,0xf,0x0);
 busy->SetDAQBUSY(0);
 return 0;
}
///////////////////////////////////////////////////////////
int main(int argc,char **argv){
 CTP ctp;
 if(argc==1)
 {
  // measure without ctp configuration
  INTmeasure(&ctp,0);
 }else if(argc==3){
  // set offset
  w32 del=atoi(argv[1]);
  w32 off=atoi(argv[2]);
  INTset(&ctp,2,del,off);
 }else if(argc==2){
  w32 what=atoi(argv[1]);
  if(what==0){
   // only configure ctp
   INTconfigctp(&ctp);
  }
  else if(what==1){
   // configure and measure
   INTconfigctp(&ctp);
   INTconfigctpClass(&ctp);
   usleep(3000);
   INTmeasure(&ctp,1);
  }else if(what==2){
   // read orbits
   printf("Starting to read orbit========================================\n");
   ctp.readOrbits();
  }
  else{
   printf("Unexpected argument: %i \n",what);
   return 0;
  }
 }else{
  printf("Expecting 0 or 2 arguments \n");
 }
 return 0;
}
 
