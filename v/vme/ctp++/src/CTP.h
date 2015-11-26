#ifndef _CTP_h
#define _CTP_h
#include <list>
#include "BOARD.h"
#include "L0BOARD.h"
#include "L0BOARD1.h"
#include "L0BOARD2.h"
#include "L1BOARD.h"
#include "L2BOARD.h"
#include "FOBOARD.h"
#include "INTBOARD.h"
#include "FOBOARD.h"
#include "BUSYBOARD.h"
#include "LTUBOARD.h"
#include "TTCITBOARD.h"
#include "DETECTOR.h"
#include "libctp++.h"
using namespace std;
#define NUMOFFO 6
#define NUMOFCON 4
//#define NUMOFLTU (NUMOFFO*NUMOFCON)
class CTP
{
 public:
	 CTP();
         ~CTP();
         BUSYBOARD *busy;
         //L0BOARD *l0;     //abandoning 2 versions of l0 boarsd
         L0BOARD2 *l0;     //Here should be pointers instead objects
	 L1BOARD *l1;     // otherwise compiler crashes probably due to the memory
         L2BOARD *l2;
         INTBOARD *inter;
	 FOBOARD *fo[NUMOFFO];
         LTUBOARD *ltu[NUMOFFO*NUMOFCON];
         DETECTOR *fo2det[NUMOFFO][NUMOFCON];
	 int setSWtrigger(char triggertype,w32 BC, w32 detectors,w32 lm);
	 int startSWtrigger(char triggertype,w32 lm);
	 void clearSWTriggerFlags(){l0->setTCCLEAR();l1->setTCCLEAR();l2->setTCCLEAR();};
	 int Parsecfg();
	 int ParseValidCTPInputs();
	 int readBCStatus(int n,w32 delta);
	 int readCounters();
	 void printCounters();
	 void readOrbits();
	 list<BOARD*> boards;
 private:
	 void readBICfile();
	 void readDBVALIDLTUS();
         void getboard(string const &line);
         void getdetector(string const &line);
         void getdetectorold(string const &line);
         void printboards();
         void checkFPGAversions() const;
	 int ProcessInputLine(const string &line);
	 int numofltus;  //Number of ltus in crate, up to 4
         int numoffos;   //Number of fos in crate up to 6
         int vspctp;      // ctp vme space
         int vspltu;	  // ltu vmespace
	 int debug;				     
	 string INT1,INT2;
	 vector<string> inputlist;
};
#endif
