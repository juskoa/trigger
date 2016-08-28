#ifndef _TTCITBOARD_h_
#define _TTCITBOARD_h_
#include "BOARD.h"
#include <deque>
#include "LTUBOARD.h"
#include "BUSYBOARD.h"
class TTCITBOARD: public BOARD
{
 public:
	TTCITBOARD(string const name,w32 const boardbase,int vsp);
	w32 getFPGAversion(){ return vmer(VERSION);}
	w32 getStatus(){return vmer(STATUS);};
	void resetSSMAddress(){vmew(RESET_SNAPSHOT_N,0xff);}
        int readSSM();
	int startSSM();
	void start_stopSSM();
	int start_stopSSM(LTUBOARD* ltu);
	int start_stopSSM(BUSYBOARD* bb);
        void addclasspattern(w32 *pat);
	int ReadAllCounters(w32 l0l1time);
        // SSM analysis
        void ClearQueues();
        int AnalyseSSM();
        void Dump2quSSM();
        void DumptxtSSM();
        void DumpqueSSM2file(const char *filename);
	int CompareL1L2Data(w32* l1m,w32* l2m);
	int CheckClassPatternSSM();
        void Print();
 private:
	enum{NL1words=9,NL2words=13};
	//
	w32 *ssm;
        // vme addresses
        w32 const VERSION;
        w32 const CONTROL;
        w32 const STATUS;
	w32 const READ_SSM_ADDRESS;
        w32 const READ_SSM_WORD;
        w32 const RESET;
        w32 const RESET_SNAPSHOT_N;
        w32 const TIME_L0_L1;
	w32 const RESET_COUNTERS;
 	w32 const COUNT_ERR_BCNT;
	deque<ssmrecord*> qttcab;
        deque<w32*> classpatterns;
};
#endif
