#ifndef _Counter_h_
#define _Counter_h_
#include "Log.h"
#include <string>
typedef unsigned int w32;
typedef unsigned long long int w64;


class Counter : public Log
{
 private:
        string fname;
        bool first;
        w32 ixcount,ixtime;
        w32 cdiff,tdiff;
        w32 countbefore,countnow,timebefore,timenow;
        w32 factor;  // correction for SPD 4 BC
        w64 timetot,counttot;
        double rate1min,ratetot;
        w32 dodif32(w32 before, w32 now);
 public:
	Counter();
        void Update(w32* buffer);
        int SetIXs(w32 ixcount,w32 ixtime);
        void SetName(string& name){fname=name;};
        void SetName(char const *name){fname=name;};
        void SetFactor(w32 fact){factor=fact;};
        bool GetFirst(){return first;};
        string* GetName(){return &fname;};
        w64 GetCountTot(){return counttot;};
        w64 GetTimeTot(){return timetot;};
        w32 GetCount(){return cdiff;}
        w32 GetTime(){return tdiff;}
        double GetRate(){return rate1min;}
        double GetRateA(){return ratetot;}
        w32 GetBefore(){return countbefore;}
        w32 GetNow(){return countnow;}
        void PrintName();
        void Print();
};
#endif

