#include "L0BOARD.h"
L0BOARD::L0BOARD(int vsp)
:
	BOARD("l0",0x829000,vsp,4),
	L0CONDITION(0x400),
	L0INVERT(0x600),
	L0VETO(0x900)
{
  this->AddSSMmode("inmon",0); 
  this->AddSSMmode("outmon",1); 
  this->AddSSMmode("ingen",2); 
  this->AddSSMmode("outgen",3); 
}
//----------------------------------------------------------------------------
// set single class at index, with input mask inputs and cluster
void L0BOARD::setClass(w32 index,w32 inputs,w32 l0f,w32 rn,w32 bc)
{
 w32 word=inputs+(l0f<<24) + (rn<<26)+(bc<<28);
 vmew(L0CONDITION+4*index,word);
}
//----------------------------------------------------------------------------
// set single class at index woth vetoes, no PF
void L0BOARD::setClassVetoes(w32 index,w32 cluster,w32 bcm,w32 rare)
{
 w32 word=cluster + (0xf << 4) + (bcm<<8)+(rare<<12);
 vmew(L0VETO+4*index,word);
}
//----------------------------------------------------------------------------
// Set all classes to 0xfffff - dont care
void L0BOARD::setClassesToZero()
{
 for(w32 i=0; i<kNClasses; i++){
    setClass(i+1,0xfabcde,0x3,0x3,0x3);
    setClassVetoes(i+1,0,0xf,0x1);
 }
}
//----------------------------------------------------------------------------
// read and print all classes
void L0BOARD::printClasses()
{
 printf("CTP classes from hardware:\n");
 for(w32 i=0; i<kNClasses; i++){
    w32 cond=vmer(L0CONDITION+4*(i+1));
    w32 veto=vmer(L0VETO+4*(i+1));
    printf("%i  0x%x 0x%x \n",i+1,cond,veto);
    //if((i+1)%10 == 0)printf("\n");
 }
}
