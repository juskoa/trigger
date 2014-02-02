#ifndef _BOARD_h_
#define _BOARD_h_
#include <fstream>
#include "BOARDBASIC.h"
#include "SSMTOOLs.h"
class BOARD: public BOARDBASIC
{
 public:
	 BOARD(string const name,w32 const boardbase,int vsp,int nofssmmodes);
         int AddSSMmode(string const modename,int const imode);
         string getName() const {return d_name;};
	 int ReadSSM() const;
	 int WritehwSSM() const;
	 void PrintSSM(int const start,int const n) const;
	 void WriteSSM(w32 const word) const;
	 void WriteSSM(w32 const word,int const start,int const last) const;
	 w32 *GetSSM(){return ssm;};
	 string *GetChannels(string const &mode) const;
	 void PrintChannels(string const &mode) const;
         int SetMode(string const &mode,char const c) const;
	 int StartSSM() const;
         void StopSSM() const;
         SSMTOOLs ssmtools;
	 enum {kNClasses=100};
 private:
	 void SetFile(string const &modename);
         int const numofmodes;
	 int parsemode(string const &mode) const;
	 ifstream modefile;
	 w32 *ssm;
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

};
#endif
