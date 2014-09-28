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
	int CheckCountersNoTriggers();
        // L1 counters starts at 300 in cnames
        // CL0CLST - pointer on T cluster - for 8 clusters check cnames
	enum{CL1TIME=5, CL1STRIN=35, CL1STROUT=36, CL1CLSB=40, CL1CLSA=140, CL1CLST=241};
 private:
         // vme addresses
         w32 const L1CONDITION;
         w32 const L1INVERT;
         w32 const L1DELAYL0;

};
#endif
