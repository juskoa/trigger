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
 private:
         // vme addresses
         // ssm testing
         // Add structures herea
        deque <L2Data> qctpro;
        deque <IRDa> qirda;
};
#endif
