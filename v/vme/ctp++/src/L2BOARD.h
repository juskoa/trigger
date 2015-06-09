#ifndef _L2BOARD_h_
#define _L2BOARD_h_
#include "BOARD.h"
#include <deque>
class L2BOARD: public BOARD
{
 public:
	L2BOARD(int vsp);
        w32 getl2ackn(){return vmer32(TCSTATUS);}
	void setClass(w32 index,w32 inputs,w32 cluster,w32 vetos,w32 invert);
	void setClassesToZero();
	void setTCSET(w32 w){vmew(TCSET,w);};
	void setTCCLEAR(){vmew(TCCLEAR,0);};
	void setL2DEFINITION(w32 w){vmew(L2DEFINITION,w);};
	void printClasses();
	int CheckCountersNoTriggers();
        // L2 counters starts at 600 in cnames
        // CL2CLST - pointer on T cluster - for 8 clusters check cnames
	enum{CL2TIME=5, CL2STRIN=23, CL2STROUT=24, CL2CLSB=26, CL2CLSA=126, CL2CLST=227};

 private:
         // vme addresses
         w32 const TCSET;
	 w32 const TCSTATUS;
	 w32 const TCCLEAR;
         w32 const L2DEFINITION;
         w32 const L2INVERT;
};
#endif
