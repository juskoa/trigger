#include "L0BOARD2.h"
L0BOARD2::L0BOARD2(int vsp)
:
	L0BOARD(vsp),
	L0VETO(0x800),
	SCALED_1(0x224),SCALED_2(228)
{
}
//----------------------------------------------------------------------------
/* 
 * set single class at index with cluster,vetoes, no PF
*/ 
void L0BOARD2::setClassVetoes(w32 index,w32 cluster,w32 bcm,w32 rare,w32 clsmask)
{
 w32 word=0;
 // Downscaling to be added
 word=cluster + (0xf<<4) + (bcm<<8)+(rare<<20) + (clsmask<<23);
 vmew(L0VETO+4*index,word);
}
//----------------------------------------------------------------------------
/* 
 * set single class at index with cluster only, and mask 0x0 (active)
*/
void L0BOARD2::setClassVetoes(w32 index,w32 cluster)
{
 setClassVetoes(index,cluster,0xfff,0x1,0x0);
}
//----------------------------------------------------------------------------
/* 
 * read and print all classes
*/
void L0BOARD2::printClasses()
{
 printf("CTP classes from hardware:\n");
 for(w32 i=0; i<kNClasses; i++){
    w32 cond=vmer(L0CONDITION+4*(i+1));
    w32 veto=vmer(L0VETO+4*(i+1));
    printf("%i  0x%x 0x%x \n",i+1,cond,veto);
    //if((i+1)%10 == 0)printf("\n");
 }
}
