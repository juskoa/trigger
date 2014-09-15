#ifndef _BOARD_h_
#define _BOARD_h_
#include <fstream>
#include "BOARDBASIC.h"
#include "SSMTOOLs.h"
#include "ssmrecord.h"
#include <deque>
class BOARD: public BOARDBASIC
{
 public:
	 BOARD(string const name,w32 const boardbase,int vsp,int nofssmmodes);
	 ~BOARD();
	 int readCounters();
	 void SetNumofCounters(int nc){NCounters=nc;};
         int AddSSMmode(string const modename,int const imode);
         string getName() const {return d_name;};
	 int ReadSSM() const;
         int DumpSSM(const char *name) const;
	 int ReadSSMDump(const char *name) const;
	 int WritehwSSM() const;
	 void PrintSSM(int const start,int const n) const;
	 void WriteSSM(w32 const word) const;
	 void WriteSSM(w32 const word,int const start,int const last) const;
	 w32 *GetSSM(){return ssm;};
	 string *GetChannels(string const &mode) const;
	 w32 getModeChannel(string name,string channel);
	 w32 getChannel(string const &channel) const;
	 void PrintChannels(string const &mode) const;
	 int bit(w32 num,w32 channel){return ((num &(1u<<channel))==(1u<<channel));}
         int SetMode(string const &mode,char const c);
	 int StartSSM() const;
         void StopSSM() const;
         SSMTOOLs ssmtools;
	 void L2DataBackplane();
	 void printL2DataBackplane();
	 deque<L2Data>& getL2DataBackplaneList(){return ql2backplane;};
	 void L2DataSerial(int ch1,int ch2);
	 void printL2Serial();
	 deque<L2Data>& getL2SerialList(){return ql2backplane;};
         //deque<qorbit>& getSSMorbit(){return qorbit;};
	 enum {kNClasses=100};
         enum {NCOUNTERS_MAX=300};
 private:
	 void SetFile(string const &modename);
         int const numofmodes;
	 int parsemode(string const &mode) const;
	 ifstream modefile;
	 w32 *ssm;
	 w32 ssmmode;
         int SetMode(string const &mode,char const c,w32 &imode) const;
         int SetMode(w32 const modecode) const;
	 int setomvspSSM(w32 const mod) const;
         //
	 // vme adresses
         //
	 w32 const SSMcommand; // oxo:VMEREAD 0x1:VMEWRITE 0x2:RECAFTER 0x3:RECBEF
	 w32 const SSMstart;
	 w32 const SSMstop;
	 w32 const SSMaddress;
	 w32 const SSMdata;
	 w32 const SSMstatus;
	 w32 const SSMenable;
	 w32 const COPYCOUNT;
	 w32 const COPYBUSY;
	 w32 const COPYCLEARADD;
	 w32 const COPYREAD;
	 // constants
	 w32 const SSMomvmer;
	 w32 const SSMomvmew;
         w32 SSMbusybit;  // because it is different for ltu and ctp
	 //
	 struct SSMmode{
		string name;
		w32 modecode;
		string channels[32];
	 };
         SSMmode *SSMModes;
         //
         int NCounters;
         w32* counters1;
         w32* counters2;
         //
         deque<L2Data> ql2backplane;
	 deque<L2Data> qorbitl2data;
	 deque<ssmrecord> qorbit;
         deque<L2Data> ql2serial;
};
#endif
