#ifndef _MonScal_h_
#define _MonScal_h_
#include "ActiveRun.h"
#include "Log.h"
enum {kSCAL=1, kDAQ=2, kOCDB=4};
class MonScal:  public Log
{
 private:
         enum {NRUN=6};
         w32 *buffer;
         ActiveRun* inputs;
         w32 kPrint; // kPrint: 1st bit: display, 2 bit: counters
         int count;
         int lastmodtime;
         ActiveRun* activeruns[NRUN];
         bool copycount2OCDB; // 1=copy, 0=not copy
         int CheckVCTPINPUTStatus();
         bool SCAL(){return kPrint & kSCAL;};
         bool DAQ(){return kPrint & kDAQ;};
         bool OCDB(){return kPrint & kOCDB;};
 public:
         MonScal(w32 kPrint,bool copycount2OCDB);
         ~MonScal();
         void SetBuffer(w32* buffer){this->buffer=buffer;};
         void GetActiveRuns();
         void StartActiveRun(int index,int runnum);
         void StartInputs(); 
         int StopActiveRun(int index); 
         int UpdateActiveRun(int index,int runnum);
         int UpdateActiveRunInputs(int index,int runnum);
};
#endif
