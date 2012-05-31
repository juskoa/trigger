#include "MonScal.h"
#include "ctpcounters.h"
#include "ActiveRun.h"
#include <iostream>
#include "vmeblib.h"
#include <sstream>
#include <sys/stat.h> 
#define RUNXCOUNTERSSTART (CSTART_BUSY+NCOUNTERS_BUSY_RUNX1)

MonScal::MonScal(w32 kPrint,bool copycount2OCDB)
:
buffer(0),
inputs(0),
kPrint(kPrint),
count(0),
lastmodtime(0),
copycount2OCDB(copycount2OCDB)
{
 for(int i=0;i<NRUN;i++)activeruns[i]=0;
 if(SCAL())StartInputs();
 cout << "Monscal created. Output option= " << kPrint << endl;
}
MonScal::~MonScal()
{
 // close all static files
 DisplaySCAL::GetfileSCAL()->close();
 daqlogbook_close();
 if(inputs) delete inputs; 
 PrintLog("Exiting MonScal.");
}
int MonScal::CheckVCTPINPUTStatus()
// check status of VALID.CTPINPURS file
// If changed parse it 
{
 stringstream ss;
 ss << "/CFG/ctp/DB/VALID.CTPINPUTS";
 string filename = getenv("VMECFDIR")+ss.str();

 struct stat VCTPINPUTS;
 stat(filename.c_str(),&VCTPINPUTS);
 time_t mtime = VCTPINPUTS.st_mtime;
 //cout << "run="<<fRunNumber << " " <<VCTPINPUTS.st_ctime << " " <<VCTPINPUTS.st_mtime << " " <<VCTPINPUTS.st_atime << endl;
 if(mtime != lastmodtime){
   lastmodtime=mtime;
   return 1;
 }
 //cout << "Run="<<fRunNumber<<" VALID.CTPINPUTS Status unchanged." << endl;
 return 0;
}
void MonScal::GetActiveRuns()
{
 this->buffer=buffer;
 for(int i=0;i<NRUN;i++){
    //int run=buffer[RUNXCOUNTERSSTART+i];
    int run=buffer[CSTART_RUNX+i];
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
 // dealing with valid.ctpinputs
 UpdateActiveRun(0,0);
}
void MonScal::StartInputs()
{
  inputs = new ActiveRun(0);
  inputs->CreateDisplaySCAL();
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
 if(runnum && SCAL()){
   ar->CreateDisplaySCAL();
    if(count){
      if(CheckVCTPINPUTStatus()){
        cout << "ERROR: VCTPINPUTS status changed while active runs" << endl;
      }
    }else{
     delete inputs;
     StartInputs();
   }
 }
 count++;
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
 count--;
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

