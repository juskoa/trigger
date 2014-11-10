#include "L0BOARD1.h"
L0BOARD1::L0BOARD1(int vsp)
:
	L0BOARD(vsp),
	L0VETO(0x900),
	L0MASK(0xb00),
	SCALED_1(0x5dc),SCALED_2(0x5e0)
{
}
//----------------------------------------------------------------------------
/* 
 * set single class at index with cluster,vetoes, no PF
*/ 
void L0BOARD1::setClassVetoes(w32 index,w32 cluster,w32 bcm,w32 rare,w32 clsmask)
{
 w32 word=0;
 word=cluster + (0xf<<4) + (bcm<<8)+(rare<<20);
 vmew(L0VETO+4*index,word);
 vmew(L0MASK+4*index,clsmask);
}
//----------------------------------------------------------------------------
/* 
 * set single class at index with cluster only, and mask 0x0 (active)
*/
void L0BOARD1::setClassVetoes(w32 index,w32 cluster)
{
 setClassVetoes(index,cluster,0xfff,0x1,0x0);
}
//----------------------------------------------------------------------------
/* 
 * read and print all classes
*/
void L0BOARD1::printClasses()
{
 printf("CTP classes from hardware:\n");
 for(w32 i=0; i<kNClasses; i++){
    w32 cond=vmer(L0CONDITION+4*(i+1));
    w32 veto=vmer(L0VETO+4*(i+1));
    w32 mask=vmer(L0MASK+4*(i+1));
    printf("%i  0x%x 0x%x %1i \n",i+1,cond,veto,mask);
    //if((i+1)%10 == 0)printf("\n");
 }
}
