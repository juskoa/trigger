#ifndef _INTBOARD_h_
#define _INTBOARD_h_
#include "BOARD.h"
#include <deque>
class CTPReadOut;
class INTBOARD: public BOARD
{
 public:
	INTBOARD(int vsp);

 private:
         // vme addresses
         // ssm testing
         deque <CTPReadOut> ctpro;
	 void getCTPReadOutList();
         // Add structures here
};
#endif
