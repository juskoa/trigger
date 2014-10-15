#include "Log.h"
#include "iostream"
#include "sstream"
ofstream* Log::log=0;
Log::Log()
{
 if(log==0){
  //char time[30];
  //getdatetime(time);
  //stringstream ss;
  //ss << "logs/test" << time <<".log";
  //cout << "Opening " << ss.str() << endl;
  //string text(ss.str());
  //log = new ofstream();
  //log->open(text.c_str());
 }
}
Log::~Log()
{
 //cout << "Log destroyed." << endl;
}

void Log::getdatetime(char* dmyhms) {
/* format date/time for logs
   dmyhms:  string[20] dd.mm.yyyy hh:mm:ss */
/*int i; */
time_t T;
struct tm *LT;  /*LocalTime */
T=time(&T); LT = localtime(&T);
sprintf(dmyhms,"%2.2d.%2.2d.%4.0d %2.2d:%2.2d:%2.2d",
  LT->tm_mday,LT->tm_mon+1,LT->tm_year+1900,
  LT->tm_hour,LT->tm_min,LT->tm_sec);
  for(int i=0; i<28; i++) { if(dmyhms[i] == ' ') dmyhms[i] = '_'; }; 
}
void Log::PrintLog(const string& text)
{
 //*log << *this << " " << text << endl;
 *log << *this << " " << text << endl;
}
void Log::PrintLog(string& text)
{
 //*log << *this << " " << text << endl;
 *log << *this << " " << text << endl;
}
void Log::PrintLog(const char* text)
{
 cout << *this << " " << text << endl;
}
void Log::PrintLog(char* text)
{
 //*log << *this << " " << text << endl;
 cout << *this << " " << text << endl;
}
void Log::PrintLog(char* text,int i)
{
 // c like formatting expected
 stringstream ss;
 ss << *this << " " << text << "\n";
 printf(ss.str().c_str(),i);
}
ostream &operator<<(ostream &stream ,Log  &log)
{
 char time[30];
 log.getdatetime(time);
 return stream << time; 
}
/*
int main(){
 Log log;
 log.PrintLog("test");
 cout << log << " lala" << endl;
 ofstream *ll=log.GetLog();
 *ll << ll << " curino" << endl;
}
*/
