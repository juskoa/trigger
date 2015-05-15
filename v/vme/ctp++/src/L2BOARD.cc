#include "L2BOARD.h"
L2BOARD::L2BOARD(int vsp)
:
	BOARD("l2",0x82b000,vsp,5),
	TCSET(0x400),TCSTATUS(0x1c0),TCCLEAR(0x1c8),
	L2DEFINITION(0x400),
	L2INVERT(0x600)
{
  this->AddSSMmode("inmon",0); 
  this->AddSSMmode("outmon",1); 
  this->AddSSMmode("ingen",2); 
  this->AddSSMmode("outgen",3); 
  this->AddSSMmode("pf",4); 
}
//----------------------------------------------------------------------------
/* 
 * Check counters assuming classes are not configured
 */
int L2BOARD::CheckCountersNoTriggers()
{
 int ret=0;
 //w32 time = countdiff[CL2TIME]; 

 if(countdiff[CL2STRIN] != 0){
   printf("L2 strobe IN!= 0 %u \n",countdiff[CL2STRIN] );
   ret=1;
 }
 if(countdiff[CL2STROUT] != 0){
   printf("L2 strobe OUT != 0 %u \n",countdiff[CL2STROUT] );
   ret=1;
 }
 for(int i=0;i<100;i++){
  if(countdiff[i+CL2CLSB] != 0){
    printf("L2classB%02i != 0 %u \n",i,countdiff[i+CL2CLSB]);
    ret=1;
  }
  if(countdiff[i+CL2CLSA] != 0){
    printf("L2classA%02i != 0 %u \n",i,countdiff[i+CL2CLSA]);
    ret=1;
  }
 }
 for(int i=0;i<7;i++){
    if(countdiff[i+CL2CLST] != 0){
      printf("l2clst%1i != 0 %u \n",i, countdiff[i+CL2CLST]);
      ret=1;
    }
 }
 if(ret==0)printf("L2  CheckCountersNoTriggers: NO ERROR detected.\n");
 return ret;
} 

//----------------------------------------------------------------------------
// set single class at index, with input mask inputs and cluster
void L2BOARD::setClass(w32 index,w32 inputs,w32 cluster,w32 vetos,w32 invert)
{
 w32 word=inputs+(cluster<<28) + (vetos<<24) + (invert<<12);
 vmew(L2DEFINITION+4*index,word);
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
    w32 word=vmer(L2DEFINITION+4*(i+1));
    printf("0x%x ",word);
    if((i+1)%10 == 0)printf("\n");
 }
}

