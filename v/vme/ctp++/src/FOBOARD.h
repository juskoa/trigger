#ifndef _FOBOARD_h_
#define _FOBOARD_h_
#include "BOARD.h"
class FOBOARD: public BOARD
{
 public:
	FOBOARD(w32 basehex,int vsp);
	FOBOARD(w32 basehex,int vsp,string const name);
	int CheckCountersNoTriggers();
	// FO counters start at 900
	enum{CTIME=0};
	//
	int L2DataOut(char focon);
	int AnalSSMinmonl2();
	int AnalSSMinmonl1();
	int AnalSSMinmonl0();
 private:
	void SetFile(string const &modename); 
};
#endif
