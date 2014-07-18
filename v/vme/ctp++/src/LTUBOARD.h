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
        w32 GetSSMBC(w32 issm);
	int AnalSSM();
	int AnalTotalSSM();
	int AnalTotalSSM2();
	int CheckLx(int level);
	int CheckL2TTC(w32 bc,w32 orbit1,w32 orbit2,w32 issm);
	int CheckOrbits();
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
        deque<w32> qorbit0;
	deque<w32> ql0strobe;
	deque<w32> ql1strobe;
	deque<w32> ql2strobe;
        deque<ssmrecord*> ql1data;
        deque<ssmrecord*> ql2data;
        deque<ssmrecord*> qttcb;
        deque<ssmrecord*> qttcL1;
        deque<ssmrecord*> qttcL2;
        
	enum{NL1words=9,NL2words=13};

	void ClearQueues();	
	int CreateRecordSSM();
	int CreateTTCL12();
	int shortsignal(w32 level,w32 bit,w32 issm);
	int longsignal(w32 &lsigflag,w32 bit,w32 issm,w32 &icount);
	int activesignal(w32 level,w32 &ssigflag,w32 bit,w32 issm,w32 &icount);
	int lxdata(w32 NLxdata,w32 &l2daflag,w32 bit,w32 issm,w32 &icount,w32 *data);
	void txprint(int i,w32 *TXS);
	void L1Serial2Words(w32 i,w16* dwords);
	void L2Serial2Words(w32 i,w16* dwords);
 	void FindOrbits();
	void analTTCB();
};
#endif
