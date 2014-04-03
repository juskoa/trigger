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

