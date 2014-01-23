#include "CTP.h"
#include <cmath>
#define NCLASSES 100
// configure boards
// ingen L0 for inputs
// calculate data and check them on l2 or int
//w32 nclasses=6;
//w32 classes[]={1,49,50,51,99,100};
w32 classes[NCLASSES];
w32 inputs[]={1,2,3,4,5,6};
void generateConfiguration()
{
 /* generate trigger mask and  input mask */
 // init random sequence
 //srand(time(0));
 // rundom number 0-2**99
 w32 clsmask[4];
 w32 icls=0;
 for(int i=0;i<4;i++){
   //clsmask[i]=(1<<(i+1))+3;
   w32 sum=0;
   classes[icls]=0;
   for(w32 k=0;k<32;k++){
     if(clsmask[i] & (1<<k)) {
       sum += 1;
       classes[icls]=1;
     }
     icls++;
     if(icls>=100) break;
   }
   printf("rnd 0x%x , ncls %i \n",clsmask[i],sum);
 }
}
void configureL0(L0BOARD *l0)
{
 //set classes
 l0->setClassesToZero();
 for(w32 i=0;i<NCLASSES;i++){
    if(classes[i])l0->SetClass(i+1,1,1);
 }
 //l0->SetClass(1,1,1);
 // set ssm ingen - inputs
 // channel 8 = 1st input
 char input[] ="1";
 l0->ssmtools.writeSPP(1,39999,8,input);
 // write ssm to hardware
 l0->WritehwSSM();
 // set mode c(ont) or s
 if(l0->SetMode("ingen",'s')){ 
  cout << "Error in seting l0 ssm mode" << endl;
  return;
 }
 //l0->StartSSM();
}
void configureL1(L1BOARD  *l1)
{
 if(l1->SetMode("inmon",'s')){ 
  cout << "Error in seting l1 ssm mode" << endl;
 }
}
void analyseL0inmon(CTP *ctp)
{
 //L0BOARD* l0=ctp->l0;
 L1BOARD* l1=ctp->l1;
 //l0->StopSSM();
 //l1->StopSSM();
 l1->ReadSSM();
}
int main()
{
  cout <<"classes " <<  sizeof(classes) << endl;
  CTP* ctp = new CTP;
  return 1;
  generateConfiguration();
  //return 0;
  configureL0(ctp->l0);
  configureL1(ctp->l1);
  ctp->l1->StartSSM();
  ctp->l0->StartSSM();
  usleep(50000);
  analyseL0inmon(ctp);
  delete ctp;
  cout <<"classes " <<  sizeof(classes) << endl;
}
