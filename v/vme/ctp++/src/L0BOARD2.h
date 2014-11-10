#ifndef _L0BOARD2_h_
#define _L0BOARD2_h_
#include "L0BOARD.h"
#include <cmath>
class L0BOARD2: public L0BOARD
{
 public:
	L0BOARD2(int vsp);
	void setClassVetoes(w32 index,w32 cluster,w32 bcm,w32 rare,w32 clsmask);
	void setClassVetoes(w32 index,w32 cluster);
	void setBC1(w32 T){vmew(SCALED_1,T);};
	void setBC2(w32 T){vmew(SCALED_2,T);};
	w32 getBC1(){return vmer(SCALED_1);};
	w32 getBC2(){return vmer(SCALED_2);};
	void printClasses();

 private:
         // vme addresses
	 w32 const L0VETO;
	 w32 const SCALED_1;
	 w32 const SCALED_2;
};
#endif
