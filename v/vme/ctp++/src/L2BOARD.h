#ifndef _L2BOARD_h_
#define _L2BOARD_h_
#include "BOARD.h"
#include <deque>
class L2BOARD: public BOARD
{
 public:
	L2BOARD(int vsp);
	void setClass(w32 index,w32 inputs,w32 cluster,w32 vetos,w32 invert);
	void setClassesToZero();
	void getL2aList();
	void printClasses();
	void printL2aList();
 private:
         // vme addresses
         w32 const L2CONDITION;
         w32 const L2INVERT;
	 // ssm analysis
	 deque<CTPR> ql2a;
	 deque<CTPR> qorbit;
};
#endif
