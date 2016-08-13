#include "Output.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include "vmeblib.h"
#define NWIDE 133

ofstream* DisplaySCAL::fileSCAL=0;
DisplaySCAL::DisplaySCAL(const int runnum,const string& name)
:
Log(),
runnum(runnum),
name(name)
{
 cout << "Starting DisplaySCAL for run: " << runnum << endl;
 if(fileSCAL==0){
   char time[30];
   getdatetime(time);
   stringstream ss;
   ss << "MONSCAL/display" <<".log";
   //ss << "MONSCAL/dispdeb" <<".log";
   cout << "Opening " << ss.str() << endl;
   string text(ss.str());
   fileSCAL = new ofstream();
   fileSCAL->open(text.c_str());
 }
}
DisplaySCAL::~DisplaySCAL()
{
 cout << "Stopping DisplaySCAL for run: " << runnum << endl;
}
void DisplaySCAL::InsertChar(char cc)
{
  string ss;
  for(int i=0;i<NWIDE;i++)ss += cc;
  *fileSCAL << ss << endl;
}
void DisplaySCAL::InsertChar(int nn,char cc)
{
  string ss;
  for(int i=0;i<nn;i++)ss += cc;
  *fileSCAL << ss << endl;
}
void DisplaySCAL::DisplayRunHeader()
{
  char datetime[30];
  getdatetime(datetime);
  char head[50];
  sprintf(head,"==== RUN:%7i %s %s ",runnum,name.c_str(),datetime);
  string ss(head);
  for(int i=ss.size();i<50;i++)ss += '=';
  *fileSCAL << ss;
  InsertChar(NWIDE-50,'=');
}
void DisplaySCAL::DisplayInputs(const int ninp, TriggerInputwCount* inps[])
{
 char dt[20];
 getdatetime(dt);
 InsertChar('#');
 *fileSCAL << "INPUTS:" << "   " << dt << endl;
 *fileSCAL << "Name      Counts  CountsTot       Rate        <Rate>  \n";
 InsertChar('-');
 for(int i=0;i<ninp;i++)inps[i]->Display(fileSCAL);
}
void DisplaySCAL::DisplayRun(const int ninp,TriggerInputwCount* inps[],const int nclust, TriggerClusterwCount* clusts[],InteractionwCount *inter)
{
  //if(runnum==0)DisplayInputs(ninp,inps);
  //if(runnum==0)DisplayInputs2C(ninp,inps);
  if(runnum==0)DisplayInputsInts2C(ninp,inps,inter);
  else{
   DisplayRunHeader();
   for(int i=0;i<nclust;i++)clusts[i]->DisplayCluster(fileSCAL);
  }
}
void DisplaySCAL::DisplayInputsInts2C(const int ninp, TriggerInputwCount* inps[],InteractionwCount* inter)
{
 char dt[20];
 getdatetime(dt);
 *fileSCAL << "==== INPUTS:" << dt;
 InsertChar(NWIDE-31,'=');
 *fileSCAL << "Name        Counts            CountsTot        Rate        <Rate>  |  ";
 *fileSCAL << "Name        Counts            CountsTot        Rate        <Rate>  \n";
 InsertChar('-');
 // Intercations
 char t1[64];
 if(inter){
   inter->DisplayInt1(t1);
   *fileSCAL << t1 << "  |  ";
   inter->DisplayInt2(t1);
   *fileSCAL << t1 << endl;
 }
 // Inputs
 for(int i=0;i<ninp;i=i+2){
    inps[i]->Display(t1);
    *fileSCAL << t1 << "  |  ";
    if(i+1<ninp){
      inps[i+1]->Display(t1);
      *fileSCAL << t1; 
    }
    *fileSCAL << endl;
 }
}
void DisplaySCAL::DisplayInputs2C(const int ninp, TriggerInputwCount* inps[])
{
 char dt[20];
 getdatetime(dt);
 *fileSCAL << "==== INPUTS:" << dt;
 InsertChar(NWIDE-31,'=');
 *fileSCAL << "Name          Counts            CountsTot       Rate        <Rate> |  ";
 *fileSCAL << "Name          Counts            CountsTot       Rate        <Rate>  \n";
 InsertChar('-');
 for(int i=0;i<ninp;i=i+2){
    char t1[64];
    inps[i]->Display(t1);
    *fileSCAL << t1 << "  |  ";
    if(i+1<ninp){
      inps[i+1]->Display(t1);
      *fileSCAL << t1; 
    }
    *fileSCAL << endl;
 }
}
//---------------------------------------------------------------------------------
int DAQlogbook::daqnotopen=1;
DAQlogbook::DAQlogbook(const int runnum,int log)
:
runnum(runnum),
log(log),
count(0)
{
 cout << "Starting DAQlogbook for run: " << runnum << " log=" << log << endl;
 if(daqnotopen){
    int rcdaq= daqlogbook_open();
    if(rcdaq==-1) cout << "DAQlogbook open failed for run " << runnum<<endl;
    else{
     cout << "DAQ logbook opened succesfuly for run " << runnum << endl;
     daqnotopen=0;
    }
 }else cout << "DAQ logbook RUN " << runnum << "already opened" << endl;
 if(log){
   stringstream ss;
   ss << "MONSCAL/DAQlog" << runnum <<".log";
   fileName=ss.str();
   file.open(fileName.c_str());
   if(!file){
     PrintLog(("DAQlogbook: cannot open file: " +fileName).c_str());
   }else{
     PrintLog(("DAQlogbook: File: "+fileName+" opened.").c_str());
   }
 }
}
DAQlogbook::~DAQlogbook()
{
 cout << "Stopping DAQlogbook for run: " << runnum << " log= "<< log <<endl;
 // close in MonScal
 //daqlogbook_close(); 
 if(log){
  file.close();
  PrintLog(("DAQlogbook: File: " + fileName + " closed.").c_str());
 }
}
void DAQlogbook::UpdateClasses(const int nclass,TriggerClasswCount* tclass[])
{
 //cout << "DAQlogbook UpdateClasses called." << endl;
 for(int i=0;i<nclass;i++){
   // warning : 3rd argument w64 but in dalogbook only w32
   int ret=daqlogbook_update_triggerClassCounter(runnum ,tclass[i]->GetIndex0(), (w32)tclass[i]->GetL2aCount());
   if(ret){
     char text[255];
     sprintf(text,"DAQloogbook: RUN %i update for class %s failed.",runnum,tclass[i]->GetName().c_str());
     PrintLog(text);
     if(log) file << text;
   }
   if(log){
     char text[255];
     sprintf(text,"%i %i %i %i \n",count++, runnum ,tclass[i]->GetIndex0(), (w32)tclass[i]->GetL2aCount());
     file << text;
     file.flush();
   }
 }
}
void DAQlogbook::UpdateDetectors(const int ndet,DetectorwCount* dets[])
{
 //cout << "DAQlogbook UpdateDetector called." << endl;
 for(int i=0;i<ndet;i++){
   // warning : 3rd argument w64 but in dalogbook only w32
   int ret=0;
   //int ret=daqlogbook_update_detectorCounter(runnum ,tclass[i]->GetIndex0(), (w32)tclass[i]->GetL2aCount());
   if(ret){
     char text[255];
     sprintf(text,"DAQloogbook: RUN %i update for detector %s failed.",runnum,dets[i]->GetName().c_str());
     PrintLog(text);
     if(log) file << text;
   }
   if(log){
     char text[255];
     sprintf(text,"%i %i %s %i \n",count , runnum ,dets[i]->GetName().c_str(), (w32)dets[i]->GetL2aCount());
     file << text;
     file.flush();
   }
 }
}
//##################################################################################################
CountersOCDB::CountersOCDB()
:
runnum(0),
version(0),
copy2dcs(0)
{
}
CountersOCDB::CountersOCDB(const int version,const int runnum,const bool copy2dcs)
:
runnum(runnum),
version(version),
copy2dcs(copy2dcs)
{
 cout << "Starting CountersOCDB for run:" <<  runnum << endl;
 stringstream ss;
 ss << "cnt/run"<<runnum<<".cnt";
 fileName=ss.str();
 file.open(fileName.c_str());
 if(!file){
  PrintLog(("CountersOCDB: cannot open file: " +fileName).c_str());
 }else{
  PrintLog(("CountersOCDB: File: "+fileName+" opened.").c_str());
 }
}
CountersOCDB::~CountersOCDB()
{
 cout << "Stopping CountersOCDB for run:" <<  runnum << endl;
 file.close();
 PrintLog(("CountersOCDB: File: " + fileName + " closed.").c_str());
 // copy to dcs
 char cmd[256],msg[256];
 sprintf(cmd,"./dcsFES_putData.sh %d GRP CTP_xcounters /home/tri/%s",runnum, fileName.c_str());
 int rc=0;
 if(copy2dcs) rc=system(cmd);
 sprintf(msg,"cmd:%s rc:%d  copy2dcs:%d \n", cmd, rc, copy2dcs);
 PrintLog(msg);
//
 sprintf(cmd,"mv -f %s delme/%s",fileName.c_str(),fileName.c_str());
 rc=system(cmd);
 sprintf(msg,"cmd:%s rc:%d\n", cmd, rc);
 PrintLog(msg);
}
int CountersOCDB::WriteHeader(const int nclass,TriggerClasswCount* tclass[])
{
  file << version << endl;
  file << runnum << " " << nclass;
  for(int i=0;i<nclass;i++) file << " " << tclass[i]->GetIndex();
  file << endl;
  return 0;
}
int CountersOCDB::WriteRecord(TrigTimeCounters* time, const int nclass,TriggerClasswCount* tclass[])
{
 file << time->GetOrbit() << " " << time->GetPeriodCounter() << " ";
 file << time->GetSecs() << " " << time->GetUsecs() << endl;
 for(int i=0;i<nclass;i++){
  Counter* cnts=tclass[i]->GetCounters();
  for(int j=0;j<6;j++)file << cnts[j].GetNow() << " ";
  file << endl;
 }
 //cout << "OCDB record " << time->GetSecs() << " written" << endl;
 return 0;
}

