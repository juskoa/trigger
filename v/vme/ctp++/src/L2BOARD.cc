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
void L2BOARD::getL2aList(){}
/*{
 int i=0,j,iorbit=0;
 int l2clusters,bcid,orbit,esr;
 lint l2class;
 Signal *sl2strobe,*sdata1,*sdata2;
 int sl2strobech,sdata1ch,sdata2ch;
 CTPR L2a,ORBIT;
 sl2strobe=findSignalS(boardl2,0,"l2strobe");
 sdata1=findSignalS(boardl2,0,"l2data1");
 sdata2=findSignalS(boardl2,0,"l2data2");
 if(sl2strobe && sdata1 && sdata2){
   sl2strobech=sl2strobe->channel;
   sdata1ch=sdata1->channel;
   sdata2ch=sdata2->channel;
 }else{
   sl2strobech=1;
   sdata1ch=2;
   sdata2ch=3;
 }
 printf("compareL2aCTPreadout: l2strobe l2data1 l2data2 channels= %i %i %i\n",sl2strobech,sdata1ch,sdata2ch);
 clearCTPR(&L2a);
 clearCTPR(&ORBIT);
 while(i<Mega){
  // ORBIT pulse
  if(bit(sms[boardl2].sm[i],0)){
   if(!iorbit){
    ORBIT.issm=i;
    L2alist=addCTPR(L2alist,&ORBIT);
    iorbit=1;
   }
  }else iorbit=0;
  if(bit(sms[boardl2].sm[i],sl2strobech)){
   L2a.issm=i;
   // L2 clusters -> one integer
   j=0;
   l2clusters=0;
   while((j<7) && (i+j)<Mega){
    l2clusters=l2clusters+bit(sms[boardl2].sm[i+j],sdata1ch)*(1<<(6-j));
    j++;
   }
   i=i+7;
   // BCID
   j=0;
   bcid=0;
   while((j<12) && (i+j)<Mega){
    bcid=bcid+bit(sms[boardl2].sm[i+j],sdata1ch)*(1<<(11-j));
    j++;
   }
   i=i+12;
   //ORBIT
   j=0;
   orbit=0;
   while((j<24) && (i+j)<Mega){
    orbit=orbit+bit(sms[boardl2].sm[i+j],sdata1ch)*(1<<(23-j));
    j++;
   }
   i=i+24;
   esr=bit(sms[boardl2].sm[i],sdata1ch);
   i=i+11;
   // L2class
   j=0;
   l2class=0;
   while((j<50) && (i+j)<Mega){
    if(bit(sms[boardl2].sm[i+j],sdata2ch))l2class=l2class+(1ll<<(49-j));
    j++;
   }
   L2a.l2clusters=l2clusters;
   L2a.l2classes=l2class;
   L2a.bcid=bcid;
   L2a.orbit=orbit; 
   L2a.esr=esr;
   L2a.clt=0;  // to be read from hw
   L2a.swc=0;  // to be read from hw
   L2alist=addCTPR(L2alist,&L2a);
   clearCTPR(&L2a);
   //printf("comparel2aCTPreadout: l2 clusters: 0x%x BCID: %i ORBIT: %i L2class: 0x%llx \n",l2clusters,bcid,orbit,l2class);
  }else i++;
 }
 //printlistN(L2alist);
 return L2alist; 
}
*/
