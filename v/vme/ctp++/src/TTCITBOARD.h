#ifndef _TTCITBOARD_h_
#define _TTCITBOARD_h_
#include "BOARD.h"
#include <deque>
#include "LTUBOARD.h"
class TTCITBOARD: public BOARD
{
 public:
	TTCITBOARD(string const name,w32 const boardbase,int vsp);
	w32 getFPGAversion(){ return vmer(VERSION);}
	w32 getStatus(){return vmer(STATUS);};
	void resetSSMAddress(){vmew(RESET_SNAPSHOT_N,0xff);}
	void start_stopSSM();
	int start_stopSSM(LTUBOARD* ltu);
        // SSM analysis
        void ClearQueues();
        void AnalyseSSM();
        void Dump2quSSM();
        void DumptxtSSM();
        void Print();
 private:
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
	deque<ssmrecord*> qttcab;
};
#endif
