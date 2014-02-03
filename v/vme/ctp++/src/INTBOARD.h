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
         typedef struct CTPR{
 		int l2clusters;
 		w64 l2classes1;
 		w64 l2classes2;
 		int bcid;
 		int orbit;
 		int eob;   // used as eob flag, other items shoyld be zero
 		int esr;
 		int clt;    // calibration trigger
 		int swc;    // software class
 		int issm;   // position of the first word in ssm
	}CTPR;
	//Interaction Record
	typedef struct IRDda{
		int error;
		int orbit;
		int Inter[251];
		int bc[251];
		int issm;
	}IRDa;
        deque <CTPR> qctpro;
        deque <IRDa> qirda;
	void printCTPR(CTPR &ctpr);
	void clearCTPR(CTPR &ctpr);
	void clearIRDda(IRDda &irda);
	void printIRDda(IRDda &irda);

};
#endif
