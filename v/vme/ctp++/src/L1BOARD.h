#ifndef _L1BOARD_h_
#define _L1BOARD_h_
#include "BOARD.h"
class L1BOARD: public BOARD
{
 public:
	L1BOARD(int vsp);
	w32 getl1ackn(){return (vmer(TCSTATUS)&0x8)/0x8;}
	void setClass(w32 index,w32 inputs,w32 cluster,w32 vetos);
	void setClassesToZero();
	void setDELAYL0(w32 delay){vmew(L1DELAYL0,delay);};
	void setTCSET(w32 w){vmew(TCSET,w);};
	void setTCCLEAR(){vmew(TCCLEAR,0);};
	void setL1DEFINITION(w32 w){vmew(L1DEFINITION,w);};
	void printClasses();
	int CheckCountersNoTriggers();
        // L1 counters starts at 300 in cnames
        // CL0CLST - pointer on T cluster - for 8 clusters check cnames
	enum{CL1TIME=5, CL1STRIN=35, CL1STROUT=36, CL1CLSB=40, CL1CLSA=140, CL1CLST=241};
	// SSM
	int AnalSSM();
 private:
         // vme addresses
         w32 const TCSET;
	 w32 const TCSTATUS;
	 w32 const TCCLEAR;
         w32 const L1DEFINITION;
         w32 const L1INVERT;
         w32 const L1DELAYL0;
};
#endif
