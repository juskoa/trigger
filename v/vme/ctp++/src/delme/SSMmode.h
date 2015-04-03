#ifndef _SSMmode_h_
#define _SSMmode_h_
#include "vmewrap.h"
#include <iostream>
#include <cstdlib>
#include "libctp++.h"
#include <fstream>
using namespace std;
class SSMmode
{
public:
	SSMmode();
	SSMmode(string name,string board,w32 const modecode);
	//w32 Init(string boardname,string modename,w32 mode);
	w32 Init();
	w32 GetModecode() const {return modecode;}
	string GetName()  const {return modename;}
        string* GetChannels() {return channels;}
	string GetChannel(w32 i){return channels[i];}
private:
        int parsemode(string const &mode) const;
 	string const modename;
	string boardname;
 	w32 modecode;
        ifstream modefile;
 	string channels[32];
};
#endif

