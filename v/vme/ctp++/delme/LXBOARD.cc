#include "LXBOARD.h"
LXBOARD::LXBOARD(string const name,w32 const boardbase,int vsp,int nofssmmodes)
:
        BOARD(name,boardbase,vsp,nofssmmodes),
	LXCONDITION(0x400),
	LXINVERT(0x600)
{
}
//----------------------------------------------------------------------------
// set single class at index, with input mask inputs and cluster
void LXBOARD::setClass(w32 index,w32 inputs,w32 cluster,w32 vetos)
{
 w32 word=inputs+(cluster<<28) + (vetos<<24)+(1<<31);
 vmew(LXCONDITION+4*index,word);
}
//----------------------------------------------------------------------------
// Set all classes to 0xfffff - dont care
void LXBOARD::setClassesToZero()
{
 for(w32 i=0; i<kNClasses; i++)setClass(i+1,0xffffff,0,0xf);
}
//----------------------------------------------------------------------------
// read and print all classes
void LXBOARD::printClasses()
{
 printf("CTP classes from hardware:\n");
 for(w32 i=0; i<kNClasses; i++){
    w32 word=vmer(LXCONDITION+4*(i+1));
    printf("0x%x ",word);
    if((i+1)%10 == 0)printf("\n");
 }
}
