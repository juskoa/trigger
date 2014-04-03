#ifndef _FOBOARD_h_
#define _FOBOARD_h_
#include "BOARD.h"
class FOBOARD: public BOARD
{
 public:
	FOBOARD(w32 basehex,int vsp);
	void AnalSSMinmonl2();
 private:
};
#endif
