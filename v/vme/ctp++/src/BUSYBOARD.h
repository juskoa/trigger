#ifndef _BUSYBOARD_h_
#define _BUSYBOARD_h_
#include "BOARD.h"
class BUSYBOARD: public BOARD
{
 public:
	BUSYBOARD(int vsp);
	void SetDAQBUSY(w32 mask){vmew(DAQ_BUSY,mask);};
        // iclu=0 - test cluster
	void setCluster(w32 iclu,w32 detmask){vmew(SET_CLUSTER+4*iclu,detmask);};
	w32 GetDAQBUSY(){return vmer(DAQ_BUSY);};
	void measurephase();

 private:
         // vme addresses
         w32 const DAQ_BUSY;
	 w32 const SET_CLUSTER;
	 w32 const DELAY_ADD;
	 w32 const ORBIT_SELECT;
	 w32 const BUSYMAX_DATA;
	 w32 const BUSYMINI_DATA;
	 w32 const MINIMAX_SELECT;
	 w32 const MINIMAX_CLEAR;
	 w32 const MINIMAX_LIMIT;
	 w32 const BUSYLAST_SELECT;
	 w32 const BUSY_CLUSTER;
};
#endif
