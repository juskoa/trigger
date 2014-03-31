#include "L2BOARD.h"
L2BOARD::L2BOARD(int vsp)
:
	BOARD("l2",0x82b000,vsp,5),
	L2CONDITION(0x400),
	L2INVERT(0x600)
{
  this->AddSSMmode("inmon",0); 
  this->AddSSMmode("outmon",1); 
  this->AddSSMmode("ingen",2); 
  this->AddSSMmode("outgen",3); 
  this->AddSSMmode("pf",4); 
}
//----------------------------------------------------------------------------
// set single class at index, with input mask inputs and cluster
void L2BOARD::setClass(w32 index,w32 inputs,w32 cluster,w32 vetos,w32 invert)
{
 w32 word=inputs+(cluster<<28) + (vetos<<24) + (invert<<12);
 vmew(L2CONDITION+4*index,word);
}
//----------------------------------------------------------------------------
// Set all classes to 0xfffff - dont care
void L2BOARD::setClassesToZero()
{
 for(w32 i=0; i<kNClasses; i++)setClass(i+1,0xfff,0,0xf,0xf);
}
//----------------------------------------------------------------------------
// read and print all classes
void L2BOARD::printClasses()
{
 printf("CTP classes from hardware:\n");
 for(w32 i=0; i<kNClasses; i++){
    w32 word=vmer(L2CONDITION+4*(i+1));
    printf("0x%x ",word);
    if((i+1)%10 == 0)printf("\n");
 }
}
//==============================================================================================================
void L2BOARD::printL2aList()
{
 cout << "L2BOARD: Printing L2 list - decoded backplane serial L2 data,  size: " << ql2a.size() << endl;
 for(w32 i=0;i<ql2a.size();i++)printCTPR(ql2a[i]);
}
void L2BOARD::getL2aList()
{
 int i=0,j,iorbit=0;
 int l2clusters,bcid,orbit,esr;
 int rc=0;
 w64 l2class1,l2class2;
 w32 sl2strobech,sdata1ch,sdata2ch;
 if((sl2strobech=getChannel("l2strobe"))>32)rc=1;
 if((sdata1ch=getChannel("l2data1"))>32) rc=1;
 if((sdata2ch=getChannel("l2data2"))>32) rc=1;
 if(rc){
   printf("Error in getL2aList: channels not found.\n");
   return;
 }
 printf("compareL2aCTPreadout: l2strobe l2data1 l2data2 channels= %i %i %i\n",sl2strobech,sdata1ch,sdata2ch);
 CTPR L2a,ORBIT;
 clearCTPR(L2a);
 clearCTPR(ORBIT);
 w32* sm=GetSSM();
 while(i<Mega){
  // ORBIT pulse
  if(bit(sm[i],0)){
   if(!iorbit){
    ORBIT.issm=i;
    qorbit.push_back(ORBIT);
    iorbit=1;
   }
  }else iorbit=0;
  if(bit(sm[i],sl2strobech)){
   L2a.issm=i;
   // L2 clusters -> one integer
   //w32 l2clstt=bit(sm[i],sdata1ch);  // test cluster
   i++;
   l2clusters=0;
   j=0;
   //while((j<6) && (i+j)<Mega){    // 6 clusters
   while((j<8) && (i+j)<Mega){      // 8 clusters
    //l2clusters=l2clusters+bit(sm[i+j],sdata1ch)*(1<<(5-j));
    l2clusters=l2clusters+bit(sm[i+j],sdata1ch)*(1<<(7-j));
    j++;
   }
   i=i+8;
   // BCID
   j=0;
   bcid=0;
   while((j<12) && (i+j)<Mega){
    bcid=bcid+bit(sm[i+j],sdata1ch)*(1<<(11-j));
    j++;
   }
   i=i+12;
   //ORBIT
   j=0;
   orbit=0;
   while((j<24) && (i+j)<Mega){
    orbit=orbit+bit(sm[i+j],sdata1ch)*(1<<(23-j));
    j++;
   }
   i=i+24;
   esr=bit(sm[i],sdata1ch);
   i=i+1+10;   // 10 gap
   // L2class
   j=0;
   //100 classes
   l2class1=0;
   while((j<40) && (i+j)<Mega){
    if(bit(sm[i+j],sdata2ch))l2class1=l2class1+(1ull<<(39-j));
    j++;
   }
   i=i+40;
   j=0;
   l2class2=0;
   while((j<60) && (i+j)<Mega){
    if(bit(sm[i+j],sdata2ch))l2class2=l2class2+(1ull<<(59-j));
    j++;
   }
   L2a.l2clusters=l2clusters;
   L2a.l2classes1=l2class1;
   L2a.l2classes2=l2class2;
   L2a.bcid=bcid;
   L2a.orbit=orbit; 
   L2a.esr=esr;
   L2a.clt=0;  // to be read from hw
   L2a.swc=0;  // to be read from hw
   ql2a.push_back(L2a);
   clearCTPR(L2a);
   //printf("comparel2aCTPreadout: l2 clusters: 0x%x BCID: %i ORBIT: %i L2class: 0x%llx \n",l2clusters,bcid,orbit,l2class);
  }else i++;
 }
 //printlistN(L2alist);
 return; 
}

