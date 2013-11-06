#ifndef _LXBOARD_h_
#define _LXBOARD_h_
#include "BOARD.h"
class LXBOARD: public BOARD
{
 public:
	LXBOARD(string const name,w32 const boardbase,int vsp,int nofssmmodes);
	void setClass(w32 index,w32 inputs,w32 cluster,w32 vetos);
	void setClassesToZero();
	void printClasses();
	enum { kNClasses = 100 };
 private:
         // vme addresses
         w32 const LXCONDITION;
         w32 const LXINVERT;
};
#endif
