#include "L1BOARD.h"
L1BOARD::L1BOARD(int vsp)
:
	BOARD("l1",0x82a000,vsp,4),
	L1CONDITION(0x400),
	L1INVERT(0x600),
	L1DELAYL0(0x4cc)
{
  this->AddSSMmode("inmon",0); 
  this->AddSSMmode("outmon",1); 
  this->AddSSMmode("ingen",2); 
  this->AddSSMmode("outgen",3); 
}
//----------------------------------------------------------------------------
// set single class at index, with input mask inputs and cluster
void L1BOARD::setClass(w32 index,w32 inputs,w32 cluster,w32 vetos)
{
 w32 word=inputs+(cluster<<28) + (vetos<<24)+(1<<31);
 vmew(L1CONDITION+4*index,word);
}
//----------------------------------------------------------------------------
// Set all classes to 0xfffff - dont care
void L1BOARD::setClassesToZero()
{
 for(w32 i=0; i<kNClasses; i++)setClass(i+1,0xffffff,0,0xf);
}
//----------------------------------------------------------------------------
// read and print all classes
void L1BOARD::printClasses()
{
 printf("CTP classes from hardware:\n");
 for(w32 i=0; i<kNClasses; i++){
    w32 word=vmer(L1CONDITION+4*(i+1));
    printf("0x%x ",word);
    if((i+1)%10 == 0)printf("\n");
 }
}
