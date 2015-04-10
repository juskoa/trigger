#ifndef _L0BOARD1_h_
#define _L0BOARD1_h_
#include "L0BOARD.h"
#include <cmath>
class L0BOARD1: public L0BOARD
{
 public:
	L0BOARD1(int vsp);
	void setClassVetoes(w32 index,w32 cluster,w32 bcm,w32 rare,w32 clsmask);
	void setClassVetoes(w32 index,w32 cluster);
	void setBC1(w32 T){vmew(SCALED_1,T);};
	void setBC2(w32 T){vmew(SCALED_2,T);};
	w32 getBC1(){return vmer(SCALED_1);};
	w32 getBC2(){return vmer(SCALED_2);};
	void printClasses();
	void readBCMASKS(){}; // To be implemented or same as L0BOARD2 ?
	void writeBCMASKS(w32* pat){}; // To be implemented or same as L0BOARD2 ?

 private:
         // vme addresses
	 w32 const L0_VETO;
	 w32 const L0_MASK;
	 w32 const SCALED_1;
	 w32 const SCALED_2;
};
#endif
