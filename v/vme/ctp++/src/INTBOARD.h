#ifndef _INTBOARD_h_
#define _INTBOARD_h_
#include "BOARD.h"
#include <deque>
class INTBOARD: public BOARD
{
 public:
	INTBOARD(int vsp);
	void getCTPReadOutList();
	void printReadOutList();
	void printIRList();
	int CheckCountersNoTriggers();
	// INT counters starts at 1492 ?
	enum{CL1STR=5,CL2STR=6,CL2R=7,CL2A=8,CORBERR=11};
 private:
         // vme addresses
         // ssm testing
         // Add structures herea
        deque <L2Data> qctpro;
        deque <IRDa> qirda;
};
#endif
