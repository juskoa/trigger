#ifndef _Counter_h_
#define _Counter_h_
#include "Log.h"
#include <string>
typedef unsigned int w32;
typedef unsigned char w8;
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
        w32 fGroup,fTime,fActiveTGroup;
        w64 timetotG,counttotG;
        double ratetotG; 
        w32 dodif32(w32 before, w32 now);
 public:
	Counter();
        void Update(w32* buffer);
        int SetIXs(w32 ixcount,w32 ixtime);
        void SetName(string& name){fname=name;};
        void SetName(char const *name){fname=name;};
        void SetFactor(w32 fact){factor=fact;};
	void SetGroupName(w32 group){fGroup=group;};
	void SetGroupTime(w32 time){fTime=time;};
        bool GetFirst(){return first;};
        string& GetName(){return fname;};
        w64 GetCountTot(){return counttot;};
        w64 GetCountTotG(){return counttotG;};
        w32 GetCountTot32G();
        w64 GetTimeTot(){return timetot;};
        w64 GetTimeTotG(){return timetotG;};
        w32 GetTimeTot32G();
        w32 GetCount(){return cdiff;}
        w32 GetTime(){return tdiff;}
        float GetTimeSec(){return timetot*0.4/1.e6;};
        float GetTimeSecG(){return timetotG*0.4/1.e6;};
        double GetRate(){return rate1min;}
        double GetRateA(){return ratetot;}
        w32 GetBefore(){return countbefore;}
        w32 GetNow(){return countnow;}
	w32 GetActiveTGroup(){return fActiveTGroup;};
        void PrintName();
        void Print();
};
#endif

