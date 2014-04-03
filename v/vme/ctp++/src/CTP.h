#ifndef _CTP_h
#define _CTP_h
#include <list>
#include "BOARD.h"
#include "L0BOARD.h"
#include "L1BOARD.h"
#include "L2BOARD.h"
#include "INTBOARD.h"
#include "FOBOARD.h"
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
         BOARD *busy;
         L0BOARD *l0;     //Here should be pointers instead objects
	 L1BOARD *l1;     // otherwise compiler crashes probably due to the memory
         L2BOARD *l2;
         INTBOARD *inter;
	 FOBOARD *fo[NUMOFFO];
         LTUBOARD *ltu[NUMOFFO*NUMOFCON];
         DETECTOR *fo2det[NUMOFFO][NUMOFCON];
	 int readCFG(string const &name);
	 list<BOARD*> boards;
 private:
	 int numofltus;  //Number of ltus in crate, up to 4
         int numoffos;   //Number of fos in crate up to 6
	 void readBICfile();
	 void readDBVALIDLTUS();
         void getboard(string const &line);
         void getdetector(string const &line);
         void getdetectorold(string const &line);
         void printboards();
         void checkFPGAversions() const;
         int vspctp;      // ctp vme space
         int vspltu;	  // ltu vmespace				     
};
#endif
