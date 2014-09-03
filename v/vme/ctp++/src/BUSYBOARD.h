#ifndef _BUSYBOARD_h_
#define _BUSYBOARD_h_
#include "BOARD.h"
class BUSYBOARD: public BOARD
{
 public:
	BUSYBOARD(int vsp);
	void SetDAQBUSY(w32 mask){vmew(DAQ_BUSY,mask);};
	w32 GetDAQBUSY(){return vmer(DAQ_BUSY);};

 private:
         // vme addresses
         w32 const DAQ_BUSY;
};
#endif
