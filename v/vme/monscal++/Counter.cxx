#include "Counter.h"
#include "ctpcounters.h"
#include <iostream>
#include <sstream>
using namespace std;

Counter::Counter()
:
Log(),
fname(""),
first(1),
ixcount(0),ixtime(0),
cdiff(0),tdiff(0),
countbefore(0),countnow(0),
timebefore(0),timenow(0),
factor(1),
timetot(0),counttot(0),
rate1min(0.),ratetot(0.),
fGroup(0),fTime(0),fActiveTGroup(0),
timetotG(0),counttotG(0),ratetotG(0)
{
 //cout << "Creating counter" << endl;
}
w32 Counter::dodif32(w32 before, w32 now) 
{
  // Substract 2 32 bits values (representing counters)
  w32 dif;
  if(now >= before) dif= now-before;
  else dif= now+ (0xffffffff-before) +1;
  return(dif);
};
int Counter::SetIXs(w32 ixcount,w32 ixtime)
{
 if(ixcount > NCOUNTERS || ixtime > NCOUNTERS){
  stringstream ss; 
  ss << "Counter::SetIX: ixcount or ixtime > NCOUNTERS: " << ixcount << " " <<  ixtime << endl;
  PrintLog(ss.str().c_str());
  PrintName();
  return 1;
 }
 this->ixcount=ixcount;
 this->ixtime=ixtime;
 return 0;
}
void Counter::Update(w32* buffer)
{    
    if(ixcount > NCOUNTERS || ixtime > NCOUNTERS)
    {
      stringstream ss; 
      ss << "Counter::Update: ixcount or ixtime > NCOUNTERS: " << ixcount << " " <<  ixtime << endl; 
      PrintLog(ss.str().c_str());
      PrintName();     
    }
    countbefore=countnow;
    countnow=buffer[ixcount];
    timebefore=timenow;
    timenow=buffer[ixtime];
    fActiveTGroup = buffer[CSTART_TSGROUP];
    if(!first){
      cdiff=dodif32(countbefore,countnow)/factor;
      tdiff=dodif32(timebefore,timenow);
      counttot=counttot+cdiff;
      bool countGroup = (fActiveTGroup==fGroup) || (fGroup==0);  //group zero is always active
      countGroup = countGroup && (fActiveTGroup != 255);   // pause
      if( countGroup ){ 
        counttotG=counttotG+cdiff;
        timetotG=timetotG+tdiff;
        ratetotG=0;
        if(timetotG)ratetotG=((double) counttotG)/((double)timetotG*0.4)*1e6;
        // debug
        //cout << "Counter " << fname << " group "<< fGroup << " updated ok" << endl;
      }else{
        //cout << "Counter " << fname << " group a/c "<<  buffer[CSTART_TSGROUP]  << "/" <<fGroup << " not updated" << endl;      
      }
      timetot=timetot+tdiff;
      rate1min=0;
      if(tdiff)rate1min=((double)cdiff)/((double)tdiff*0.4)*1.e6;
      ratetot=0;
      if(timetot)ratetot=((double) counttot)/((double)timetot*0.4)*1e6;
      //Print();
    }else first=0;
}
void Counter::PrintName()
{
 stringstream ss; 
 ss << fname << " Indexes: " << ixcount << " " << ixtime << endl;
 PrintLog(ss.str().c_str());
}
w32 Counter::GetCountTot32G()
{
 if(counttotG < (w64)(1<<31)) return (w32)counttotG;
 else {
   stringstream ss; 
   ss << fname << " attempt to read w64 as w32" << endl;
   return (w32)123456789;
 }
}
w32 Counter::GetTimeTot32G()
{
 if(timetotG < (w64) (1<<31)) return timetotG;
 else {
   stringstream ss; 
   ss << fname << " attempt to read w64 as w32" << endl;
   return 123456789;
 }
}

void Counter::Print()
{
 stringstream ss; 
 ss << fname << " Indexes: " << ixcount << " " << ixtime << endl;
 ss << countbefore << " " << countnow << " " << timebefore << " " << timenow << " " << counttot << " " << timetot << endl;
 PrintLog(ss.str().c_str());
}

