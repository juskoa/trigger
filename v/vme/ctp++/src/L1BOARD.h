#ifndef _L1BOARD_h_
#define _L1BOARD_h_
#include "BOARD.h"
class L1BOARD: public BOARD
{
 public:
	L1BOARD(int vsp);
	void setClass(w32 index,w32 inputs,w32 cluster,w32 vetos);
	void setClassesToZero();
	void setDELAYL0(w32 delay){vmew(L1DELAYL0,delay);};
	void printClasses();

 private:
         // vme addresses
         w32 const L1CONDITION;
         w32 const L1INVERT;
         w32 const L1DELAYL0;

};
#endif
