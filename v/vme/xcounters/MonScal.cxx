#include "MonScal.h"
#include "ctpcounters.h"
#include "ActiveRun.h"
#include <iostream>
#include "vmeblib.h"
#include <sstream>
#define RUNXCOUNTERSSTART (CSTART_BUSY+NCOUNTERS_BUSY_RUNX1)
MonScal::MonScal(w32 kPrint,bool copycount2OCDB)
:
kPrint(kPrint),
copycount2OCDB(copycount2OCDB)
{
 count=0;
 for(int i=0;i<NRUN;i++)activeruns[i]=0;
 if(SCAL()){
  inputs = new ActiveRun(0);
  inputs->CreateDisplaySCAL();
 }
 cout << "Monscal created. Output option= " << kPrint << endl;
}
MonScal::~MonScal()
{
 // close all static files
 DisplaySCAL::GetfileSCAL()->close();
 daqlogbook_close(); 
 PrintLog("Exiting MonScal.");
}
void MonScal::GetActiveRuns()
{
 this->buffer=buffer;
 for(int i=0;i<NRUN;i++){
    int run=buffer[RUNXCOUNTERSSTART+i];
    //cout << i << " " << run << " " << buffer[CSTART_SPEC] << endl;
    if(run && activeruns[i]==0){
      // new run
      StartActiveRun(i,run);
    }else if(run==0 && activeruns[i]){
      // stop run
      StopActiveRun(i);
    }else if(run && activeruns[i]){
      // running run
      UpdateActiveRun(i,run);
    }
 }
 UpdateActiveRun(0,0);
}
void MonScal::StartActiveRun(int index,int runnum)
{
 activeruns[index] = new ActiveRun(runnum);
 ActiveRun* ar = activeruns[index];
 ar->UpdateRunCounters(buffer);
 if(runnum && OCDB()){
   ar->CreateCountersOCDB(copycount2OCDB);
   ar->WriteHeaderOCDB();  
   ar->WriteRecordOCDB();
 }
 if(runnum && DAQ()){
   ar->CreateDAQlogbook(1);
   ar->Write2DAQlogbook();
 }
 if(runnum && SCAL())ar->CreateDisplaySCAL();
}
int MonScal::StopActiveRun(int index)
{
 ActiveRun* ar = activeruns[index];
 int runnum = ar->GetRunNumber();
 ar->UpdateRunCounters(buffer);
 if(runnum && OCDB())ar->WriteRecordOCDB();
 if(runnum && DAQ())ar->Write2DAQlogbook();
 if(runnum && SCAL())ar->DisplayRun(); 
 delete activeruns[index];
 activeruns[index]=0;
 return 0;
}
int MonScal::UpdateActiveRun(int index,int runnum)
{
 if(runnum==0){
    if(SCAL()){
      inputs->UpdateRunCounters(buffer);
      inputs->DisplayRun();
    }
    return 0;
 }
 ActiveRun* ar = activeruns[index];
 ar->UpdateRunCounters(buffer);
 if(runnum && OCDB())ar->WriteRecordOCDB();
 if(runnum && DAQ())ar->Write2DAQlogbook();
 if(runnum && SCAL())ar->DisplayRun(); 
 return 0;
}

