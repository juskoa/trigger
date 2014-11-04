#ifndef _L0BOARD_h_
#define _L0BOARD_h_
#include "BOARD.h"
#include <cmath>
class L0BOARD: public BOARD
{
 public:
	L0BOARD(int vsp);
	void setClass(w32 index,w32 inputs,w32 l0f,w32 rn,w32 bc);
	void setClass(w32 index,w32 inputs);
	void SetClass(w32 index,w32 inputs,w32 cluster);
	void setClassVetoes(w32 index,w32 cluster,w32 bcm,w32 rare,w32 clsmask);
	void setClassVetoes(w32 index,w32 cluster);
	void setClassesToZero();
	void setBC1(w32 T){vmew(SCALED_1,T);};
	void setBC2(w32 T){vmew(SCALED_2,T);};
	void printClasses();
	int CheckCountersNoTriggers();
	enum{CL0TIME=15,CL0CLSB=19,CL0STR=171,CL0CLSA=187,CL0CLST=289};
        // SSM
        int AnalSSM();

 private:
         // vme addresses
         w32 const L0CONDITION;
         w32 const L0INVERT;
	 w32 const L0VETO;
	 w32 const L0MASK;
	 w32 const SCALED_1;
	 w32 const SCALED_2;
};
#endif
