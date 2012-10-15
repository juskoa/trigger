#include "Output.h"
#include <iostream>
#include <iomanip>
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
void DisplaySCAL::DisplayRun(const int ninp,TriggerInputwCount* inps[],const int nclust, TriggerClusterwCount* clusts[],InteractionwCount *inter,const int ndet, DetectorwCount* dets[])
{
  //if(runnum==0)DisplayInputs(ninp,inps);
  //if(runnum==0)DisplayInputs2C(ninp,inps);
  if(runnum==0)DisplayInputsInts2C(ninp,inps,inter);
  else{
   DisplayRunHeader();
   //for(int i=0;i<nclust;i++)clusts[i]->DisplayCluster(fileSCAL);
   for(int i=0;i<nclust;i++)clusts[i]->DisplayClusterSortBCM(fileSCAL);
   DisplayCalibTrigs(ndet,dets);
   //DisplayRatiosWU(ninp,inps,nclust,clusts);
   //DisplayRatiosVLH(ninp,inps,nclust,clusts);
  }
}
void DisplaySCAL::DisplayCalibTrigs(const int ndet,DetectorwCount* dets[])
{
 *fileSCAL << ">>>>>>>>> CALIB trigs: " ;
 for(int i=0;i<ndet;i++){
    if(dets[i]->GetPPCount()) *fileSCAL << dets[i]->GetName() << "=" << dets[i]->GetPPCount() << ": ";
 }
 *fileSCAL << endl;
}
void DisplaySCAL::DisplayRatiosVLH(const int ninp,TriggerInputwCount* inps[],const int nclust, TriggerClusterwCount* clusts[])
{
 TriggerClasswCount *tvln=0,*tvhn=0,*tcent=0,*tsemi=0;      
 for(int i=0;i<nclust;i++){
    if(strncmp(clusts[i]->GetName().c_str(),"ALLNOTRD",8) == 0){
      for(int j=0; j<clusts[i]->GetNumofClasses();j++){
         TriggerClasswCount* tcl=clusts[i]->GetTriggerClass(j);
         if(tcl->GetBCMtype() != B) continue;
         //cout << tcl->GetNameDesc() << " " << tcl->GetNameDesc().length() << endl;
	 string cdesc(tcl->GetNameDesc());
         if(cdesc.find("CVLN") != string::npos && cdesc.length()<5)tvln=tcl;
         if(cdesc.find("CVHN") != string::npos && cdesc.length()<5)tvhn=tcl;
         if(cdesc.find("CCENT") != string::npos && cdesc.length()<6)tcent=tcl;
         if(cdesc.find("CSEMI") != string::npos && cdesc.length()<6)tsemi=tcl;
      }
    }
 }
 if(!tvln) cout << "DisplayRatiosVLH: Cluster ALLNOTRD or class CVLN not found" << endl;
 if(!tvhn) cout << "DisplayRatiosVLH: Cluster ALLNOTRD or class CVHN not found" << endl;
 if(!tcent) cout << "DisplayRatiosVLH: Cluster ALLNOTRD or class CCENT not found" << endl;
 if(!tsemi) cout << "DisplayRatiosVLH: Cluster ALLNOTRD or class CSEMI not found" << endl;
 if(tvln && tvhn) displayratio(5.2,5.5,tvln,tvhn," CVLN/CVHN"); 
 if(tcent && tvhn)displayratio(0.86,0.91,tcent,tvhn,"CCENT/CVHN");
 if(tsemi && tvln)displayratio(0.86,0.91,tsemi,tvln,"CSEMI/CVLN");
}
void DisplaySCAL::displayratio(double low,double high,TriggerClasswCount* tnom,TriggerClasswCount* tden,const char* name)
{
  //cout << "RATIO classes:  " << tvln->GetName() << " " << tvhn->GetName() << endl;
  double ratlast= (double) tnom->GetL0bCount()/ ((double) tden->GetL0bCount());
  double rataver= (double) tnom->GetL0bCountTot()/ ((double) tden->GetL0bCountTot());
  // *fileSCAL << "\033[1:31m>>>>>>>>> CVLN/CVHN RATIOS: last="<< ratlast << " aver="<< rataver <<"\033[0m"<< endl;
  bool lastout = ratlast > high || ratlast < low;
  bool averout = rataver > high || rataver < low;
  if(lastout || averout) *fileSCAL << "\033[1;5;31m>>>>>>>>>>> "<<name<<" RATIOS: last="<< setiosflags(ios::fixed)<< setprecision(5) << ratlast << " aver=" << setprecision(5) << rataver << "\033[0m\n";else
*fileSCAL << "\033[1;32m>>>>>>>>>>> "<<name<<" RATIOS: last="<< setiosflags(ios::fixed)<<setprecision(5)<<ratlast << " aver=" << setprecision(5)<<rataver << "\033[0m\n";

}
void DisplaySCAL::DisplayRatiosWU(const int ninp,TriggerInputwCount* inps[],const int nclust, TriggerClusterwCount* clusts[])
{
 // needs to be optimised, does not look every time.
 TriggerClusterwCount *all=0;
 TriggerClusterwCount *allnotrd=0;
 int flag=0;
 for(int i=0;i<nclust;i++){
    //cout << clusts[i]->GetName() << " " << clusts[i]->GetName().length() << endl;
    if(strncmp("ALLNOTRD",clusts[i]->GetName().c_str(),8) == 0){
      //cout << "DisplayScal::DisplayRatios: found ALLNOTRD" << endl;
      allnotrd = clusts[i];
      flag++;
    } else if(strncmp(clusts[i]->GetName().c_str(),"ALL",3) == 0){
      //cout << "DisplayScal::DisplayRatios: found ALL" << endl;
      all = clusts[i];
      flag++;
    }
 }
 if(flag != 2){
   //cout << "Clusters ALL and ALLNOTRD not found " << flag << endl;
   return ;
 }
 //cout << "Found clusters: "  << allnotrd->GetName() << " " << all->GetName() << endl;
 //InsertChar('-');
 *fileSCAL << ">>>>>>>>> RATIOS: " ;
 for(int i=0; i<allnotrd->GetNumofClasses();i++){
   TriggerClasswCount* tcl=allnotrd->GetTriggerClass(i);
   if(tcl->GetBCMtype() != B) continue;
   if(strncmp("CBEAMB",tcl->GetName().c_str(),5) == 0) continue;
   for(int j=0; j<all->GetNumofClasses();j++){
     TriggerClasswCount* tcl2=all->GetTriggerClass(j);
     //cout << "Doing class: " << tcl2->GetName() << endl;
     if(strncmp(tcl2->GetName().c_str(),tcl->GetName().c_str(),4)==0 &&
       ((tcl2->GetName().at(8) == tcl->GetName().at(6)) &&   // matches abce
        (tcl->GetName().at(6)=='B')) ||
       ((tcl2->GetName().at(7) == tcl->GetName().at(5)) &&   // matches abce
        (tcl->GetName().at(5)=='B')) 
       ){      
      //cout << "match " << tcl2->GetName().at(8) << " "<< tcl->GetName().at(6) << endl;
      //cout << tcl2->GetName() << "/" << tcl->GetName() << " = ";
      *fileSCAL << tcl2->GetShortName() << "/" << tcl->GetShortName() << " = ";
      Counter *c1=tcl->GetCounters();
      Counter *c2=tcl2->GetCounters();
      //cout << (double)c1[0].GetCountTot()/(double)c2[0].GetCountTot() << " ";
      *fileSCAL << (double)c2[0].GetCountTot()/(double)c1[0].GetCountTot() << " (L0) ";
      *fileSCAL << (double)c2[5].GetCountTot()/(double)c1[5].GetCountTot() << " (L2) ";
      break;
     }
   }
 }
 //cout << endl;
 *fileSCAL << endl; 
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
    if((i+1)<ninp){
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
    cout << "Opening daqlogbook ..." << endl;
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
 //cout << "Stopping DAQlogbook for run: " << runnum << " log= "<< log <<endl;
 // close in MonScal
 //daqlogbook_close(); 
 if(log){
  file.close();
  PrintLog(("DAQlogbook: File: " + fileName + " closed.").c_str());
 }
}
void DAQlogbook::UpdateClusters(const int nclust,TriggerClusterwCount* tclust[])
{
 PrintLog("DAQlogbook UpdatedClusters called.");
 for(int i=0;i<nclust;i++){
    int ret=0;
    ret=daqlogbook_update_triggerClusterCounter(runnum,tclust[i]->GetIndex(),tclust[i]->GetL2aCount());
   if(ret){
     char text[255];
     sprintf(text,"DAQloogbook: RUN %i update for cluster %s failed.",runnum,tclust[i]->GetName().c_str());
     PrintLog(text);
     if(log) file << text;
   }
   if(log){
     char text[255];
     sprintf(text,"Clusters: %i %i %s %lli \n",count , runnum ,tclust[i]->GetName().c_str(), tclust[i]->GetL2aCount());
     file << text;
     file.flush();
   }
 }
}
void DAQlogbook::UpdateClasses(const int nclass,TriggerClasswCount* tclass[])
{
 PrintLog("DAQlogbook UpdateClasses called.");
 for(int i=0;i<nclass;i++){
   // warning : 3rd argument w64 but in dalogbook only w32
   //int ret=daqlogbook_update_triggerClassCounter(runnum ,tclass[i]->GetIndex0(), (w32)tclass[i]->GetL2aCount());
   int ret=0;
   if(!tclass[i]->IsActive()){
     //cout << "Class " << tclass[i]->GetName() << " not active" << endl;
     continue;
   }
   //cout << "Class " << tclass[i]->GetName() << " is active" << endl;
   Counter *cnts=tclass[i]->GetCounters();
   if(!cnts){
     cout << "Internal error in DAQlogbook::UpdateClasses" << endl;
     continue;
   }
   w64 l0b=cnts[0].GetCountTotG();
   w64 l0a=cnts[1].GetCountTotG();
   w32 l1b=cnts[2].GetCountTot32G();
   w32 l1a=cnts[3].GetCountTot32G();
   w32 l2b=cnts[4].GetCountTot32G();
   w32 l2a=cnts[5].GetCountTot32G();
   float time = cnts[5].GetTimeSecG(); // time should be same for all class counters
   ret=daqlogbook_update_triggerClassCounter(runnum ,tclass[i]->GetIndex0(), l0b,l0a,l1b,l1a,l2b,l2a,time);

   if(ret){
     char text[255];
     sprintf(text,"DAQloogbook: RUN %i update for class %s failed.",runnum,tclass[i]->GetName().c_str());
     PrintLog(text);
     if(log) file << text;
   }
   if(log){
     char text[255];
     //sprintf(text,"%i %i %i %i \n",count++, runnum ,tclass[i]->GetIndex0(), (w32)tclass[i]->GetL2aCount());
     sprintf(text,"Classes: %i %i %i %i %lli %lli %i %i %i %i %f\n",count++, runnum,tclass[i]->GetGroup() ,tclass[i]->GetIndex0(), l0b,l0a,l1b,l1a,l2b,l2a,time);
     file << text;
     file.flush();
   }
 }
}
void DAQlogbook::UpdateDetectors(const int ndet,DetectorwCount* dets[])
{
 PrintLog("DAQlogbook UpdateDetector called.");
 for(int i=0;i<ndet;i++){
   // warning : 3rd argument w64 but in dalogbook only w32
   int ret=0;
   ret=daqlogbook_update_triggerDetectorCounter(runnum , dets[i]->GetName().c_str(), dets[i]->GetL2aCount());
   if(ret){
     char text[255];
     sprintf(text,"DAQloogbook: RUN %i update for detector %s failed.",runnum,dets[i]->GetName().c_str());
     PrintLog(text);
     if(log) file << text;
   }
   if(log){
     char text[255];
     sprintf(text,"Detectors: %i %i %s %lli \n",count , runnum ,dets[i]->GetName().c_str(), dets[i]->GetL2aCount());
     file << text;
     file.flush();
   }
 }
}
void DAQlogbook::UpdateInputs(const int ninp,TriggerInputwCount* inps[])
{
 PrintLog("DAQlogbook UpdateInputs called");
 for(int i=0;i<ninp;i++){
    int ret=0;
    ret=daqlogbook_update_triggerInputCounter(runnum,inps[i]->GetPosition(), inps[i]->GetLevel(),inps[i]->GetCounter()->GetCountTot());
   if(ret){
     char text[255];
     sprintf(text,"DAQloogbook: RUN %i update for input %s failed.",runnum,inps[i]->GetName().c_str());
     PrintLog(text);
     if(log) file << text;
   }
   if(log){
     char text[255];
     sprintf(text,"Inputs: %i %i %s %i %i %lli\n",count , runnum ,inps[i]->GetName().c_str(), inps[i]->GetPosition(),inps[i]->GetLevel(), inps[i]->GetCounter()->GetCountTot());
     file << text;
     file.flush();
   }
 }
}
void DAQlogbook::UpdateL2a(Counter& L2a)
{
 PrintLog("DAQlogbook UpdateL2a called.");
 int ret=0;
 // skontroluj types
 ret = daqlogbook_update_triggerGlobalCounter(runnum,L2a.GetCountTot(),L2a.GetTimeTot());
 if(log){
     char text[255];
     sprintf(text,"L2a= %i %i %lli %lli \n",count , runnum ,L2a.GetCountTot(),L2a.GetTimeTot());
     file << text;
     file.flush();
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
 // Aliases fike: runXXXXX.als
 ss.str("");
 ss << "run"<<runnum<<".als";
 fileNameAliases=ss.str(); 
}
CountersOCDB::~CountersOCDB()
{
 cout << "Stopping CountersOCDB for run:" <<  runnum << endl;
 file.close();
 PrintLog(("CountersOCDB: File: " + fileName + " closed.").c_str());
 if(copy2dcs){
   char cmd[256],msg[256];
   int rc=0;
   // copy counters to dcs
   sprintf(cmd,"./dcsFES_putData.sh %d GRP CTP_xcounters /home/tri/%s",runnum, fileName.c_str());
   cout << "Executing counters:" << endl;
   PrintLog(cmd);
   rc=system(cmd);
   sprintf(msg,"cmd:%s rc:%d  copy2dcs:%d \n", cmd, rc, copy2dcs);
   PrintLog(msg);
   //  copy  counters to log
   sprintf(cmd,"mv -f %s delme/%s",fileName.c_str(),fileName.c_str());
   rc=system(cmd);
   sprintf(msg,"cmd:%s rc:%d\n", cmd, rc);
   PrintLog(msg);
  }
  bool aliases2dcs=1;
  if(aliases2dcs){
   char cmd[256],msg[256];
   int rc=0;
   // copy aliases from act to tri:~tri/CFG/ctp/DB
   sprintf(cmd,"getactaliases.bash");
   cout << "Executing aliases:" << endl;
   PrintLog(cmd);
   rc=system(cmd);
   sprintf(msg,"cmd:%s rc:%d\n", cmd, rc);
   PrintLog(msg);
   sprintf(cmd,"mv -f ./CFG/ctp/DB/aliases.txt ./CFG/ctp/DB/%s",fileNameAliases.c_str());
   rc=system(cmd);
   sprintf(msg,"cmd:%s rc:%d\n", cmd, rc);
   PrintLog(msg);
   // copy aliases to dcs
   sprintf(cmd,"./dcsFES_putData.sh %d GRP CTP_aliases ./CFG/ctp/DB/%s",runnum, fileNameAliases.c_str());
   rc=system(cmd);
   sprintf(msg,"cmd:%s rc:%d\n", cmd, rc);
   PrintLog(msg);
   sprintf(cmd,"mv -f ./CFG/ctp/DB/%s delme/aliases/%s",fileNameAliases.c_str(),fileNameAliases.c_str());
   rc=system(cmd);
   sprintf(msg,"cmd:%s rc:%d\n", cmd, rc);
   PrintLog(msg);
 }
}
int CountersOCDB::WriteHeader(const int nclass,TriggerClasswCount* tclass[])
{
  //cout << "Writing Counters" << endl;
  file << version << endl;
  file << runnum << " " << nclass;
  for(int i=0;i<nclass;i++){
     //tclass[i]->Print();
     file << " " << (w32)tclass[i]->GetIndex();
  }
  file << endl;
  return 0;
}
int CountersOCDB::WriteRecord(TrigTimeCounters* time, const int nclass,TriggerClasswCount* tclass[])
{
 file << time->GetOrbit() << " " << time->GetPeriodCounter() << " ";
 file << time->GetSecs() << " " << time->GetUsecs() << " ";
 file << time->GetActiveTGroup();
 file  << endl;
 for(int i=0;i<nclass;i++){
  Counter* cnts=tclass[i]->GetCounters();
  for(int j=0;j<6;j++)file << cnts[j].GetNow() << " ";
  file << endl;
 }
 //cout << "OCDB record " << time->GetSecs() << " written" << endl;
 return 0;
}

