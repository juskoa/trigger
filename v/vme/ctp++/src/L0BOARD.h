#ifndef _L0BOARD_h_
#define _L0BOARD_h_
#include "BOARD.h"
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
	void printClasses();

 private:
         // vme addresses
         w32 const L0CONDITION;
         w32 const L0INVERT;
	 w32 const L0VETO;
	 w32 const L0MASK;
};
#endif
