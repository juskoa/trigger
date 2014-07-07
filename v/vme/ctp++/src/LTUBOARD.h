#ifndef _LTUBOARD_h_
#define _LTUBOARD_h_
#include "BOARD.h"
#include <deque>
class ssmrecord;
class LTUBOARD: public BOARD
{
 public:
	LTUBOARD(string const name,w32 const boardbase,int vsp);
	void SetGlobal(){vmew(STANDALONE_MODE,0x0);};
	void SetStandalone(){vmew(STANDALONE_MODE,0x1);};
        // SSM analysis
	int AnalSSM();
	int AnalTotalSSM();
	int CheckLx(int level);
	w32 GetErrors(){return ierror;};
        void Print();
 private:
	string ltuname;
        // emulation constants
        w32 const NL1dat;
        w32 const NL2dat;
        // vme addresses
        w32 const STANDALONE_MODE;
	// emulation working variables
	w32 ierror;
        deque<ssmrecord*> qorbit;
	deque<w32> ql0strobe;
	deque<w32> ql1strobe;
	deque<w32> ql2strobe;
        deque<ssmrecord*> ql1data;
        deque<ssmrecord*> ql2data;
        deque<ssmrecord*> qttcb;

	void ClearQueues();	
	int CreateRecordSSM();
	int CreateRecordSSM(bool first);
	int shortsignal(w32 level,w32 bit,w32 issm);
	int longsignal(w32 &lsigflag,w32 bit,w32 issm,w32 &icount);
	int activesignal(w32 level,w32 &ssigflag,w32 bit,w32 issm,w32 &icount);
	int lxdata(w32 NLxdata,w32 &l2daflag,w32 bit,w32 issm,w32 &icount,w32 *data);
	int channelB(w32 &flag,w32 bit,w32 issm,w32 &icount,w32 &datab,w32* data);
	void txprint(int i,w32 *TXS);
	w16* L1Serial2Words();
};
#endif
