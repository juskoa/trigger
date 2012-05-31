#include "ActiveRun.h"
#include "TrigConf.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <sys/time.h> 
void splitstring(const string& str,vector<string>& tokens,const string& delimiters = " ");

ActiveRun::ActiveRun()
:
fRunNumber(0),
fname(""),
frcfgfile(""),
partifile(""),
ninp(0),nclass(0),nclust(0),ndet(0),
error(0),
copycounters2dcs(0),
ocdb(0),daq(0),scal(0),
fINT(0)
{
}
ActiveRun::ActiveRun(int runnum)
:
fRunNumber(runnum),
fname(""),
frcfgfile(""),
partifile(""),
ninp(0),nclass(0),nclust(0),ndet(0),
error(0),
ocdb(0),daq(0),scal(0),
fINT(0)
{
 for(int i=0;i<NINP;i++)fTrigInputs[i]=0;
 for(int i=0;i<NCLUST;i++)fClusters[i]=0;
 for(int i=0;i<NCLASS;i++)fClasses[i]=0;
 for(int i=0;i<NDET;i++)fDetectors[i]=0;
 ParseConfigFile(runnum);
 ParsePartitionFile(runnum);
 FindDetectors();
 //PrintDetectors();
}
ActiveRun::~ActiveRun()
{
 if(fINT) delete fINT;
 for(int i=0;i<ninp;i++) if(fTrigInputs[i])delete fTrigInputs[i];
 for(int i=0;i<nclust;i++) if(fClusters[i])delete fClusters[i];
 // classes are deleted in clusters
 //for(int i=0;i<nclass;i++) if(fClasses[i])delete fClasses[i];
 for(int i=0;i<ndet;i++) if(fDetectors[i])delete fDetectors[i];
 if(ocdb) delete ocdb;
 if(daq) delete daq;
 if(scal) delete scal;
}

//-----------------------------------------------------------
int ActiveRun::FindDetectors()
{
 for(int i=0;i<nclust;i++){
   string* dets = fClusters[i]->GetDetectors();
   for(int j=0;j<fClusters[i]->GetNdet();j++){
      int k=0;
      while(k<ndet && (fDetectors[k]->GetName().find(dets[j]) == string::npos))k++;
      if(k==ndet){
         if(VALIDLTUS::GetDetector(dets[j])){
           fDetectors[ndet++] = new DetectorwCount(*VALIDLTUS::GetDetector(dets[j]));
         }
      }
   }
 }
 return 0;
}
//-----------------------------------------------------------
int ActiveRun::ProcessCfgLine(const string &line,int& level)
{
 //cout << line << endl;
 if(line.size()==0) return 0;
 size_t ix=0;
 while(ix<line.size() && line.at(ix)==' ')ix++;
 if(line.at(ix)=='#') return 0;
 if((ix=line.find("PARTITION:")) != string::npos){
  fname=line.substr(ix+11);
  //cout << "name: " <<fname << endl;
  return 0;
 }
 if((ix=line.find("VERSION:") != string::npos)){
  string ver=line.substr(ix+8);
  //cout << "ver: " << ver << endl;
  return 0;
 }
 if((line.find("INPUTS:") != string::npos)){
  level=1;
  return 0;
 }
 if((line.find("INTERACTIONS:") != string::npos)){
  cout << "INTERACTION found." << endl;
  fINT = new InteractionwCount; 
  level=2;
  return 0;
 }
 if((line.find("DESCRIPTORS:") != string::npos)){
  level=3;
  return 0;
 }
 if((line.find("CLUSTERS:") != string::npos)){
  level=4;
  return 0;
 }
 if((line.find("PFS:") != string::npos)){
  level=5;
  return 0;
 }
 if((line.find("BCMASKS:") != string::npos)){
  level=6;
  return 0;
 }
 if((line.find("CLASSES:") != string::npos)){
  level=7;
  return 0;
 }
 vector<string> items;
 splitstring(line,items," ");
 int nitems = items.size();
 //cout << "# of items: " << nitems <<  "level= " << level << endl;
 switch (level){
   case 1:  // inputs
          if(nitems != 5){
            PrintLog(("Invalid input syntax: "+line).c_str());
            return 1;
          }
          if((items[0].find("BC1") != string::npos) || (items[0].find("BC2") != string::npos) || (items[0].find("RND") != string::npos)){
             cout << "ActiveRun::ProcessCfgLine: input "  << items[0] << " skipped." << endl;
             return 0;
          }
          TriggerInputwCount* inp = new TriggerInputwCount(items[0],atoi(items[2].c_str()),atoi(items[4].c_str()),items[1]); 
          AddInput(inp);
          return 0;
   case 2:  //interactions
          //cout << "INTERACTION found." << endl;
          //fINT = new InteractionwCount; 
          return 0;
   case 3:   // descriptors
          return 0;
   case 4: //clusters
          if(nitems<3){
            PrintLog(("Invalid cluster syntax: "+line).c_str());
            return 1;
          }
          TriggerClusterwCount* cls = new TriggerClusterwCount(items[0],atoi(items[1].c_str()));
         for(int i=2;i<nitems;i++){
           if(items[i].find("DAQ_TEST") != string::npos){
            string a("DAQ");
            cls->AddDetector(a);
            cout << "Changing DAQ_TEST to DAQ." << endl;
           }else cls->AddDetector(items[i]);
         }
         AddCluster(cls); 
         return 0;
   case 5:  // pfs
          return 0;
   case 6:   //bcmasks
          return 0;
   case 7:  // classes
         {
          if((nitems < 8) || (nitems >10)){
           PrintLog(("Invalid class syntax: "+line).c_str());
          return 1;
         }
         // looking for cluster
         //assuming that clusters are before classes
         for(int i=0;i<nclust;i++){
          if((fClusters[i]->GetName().find(items[3]) != string::npos)){
            TriggerClasswCount* clss = new TriggerClasswCount(items[0],atoi(items[1].c_str()),fClusters[i]);
            AddClass(clss);
            fClusters[i]->AddClass(clss);
            return 0;
          }
         }
         stringstream ss; 
         ss << "Class " << line << " cluster " << items[3] << " not found !" << endl;
         //PrintLog(ss.str().c_str());
         return 1;
          }
   default:
          {
          stringstream ss; 
          ss << "Unknown item (level) in rcfg file: " << level << endl;
          PrintLog(ss.str().c_str());
          break;
          }
 }

 return 1;
}
//--------------------------------------------------------------
#define MAXCLASSGROUPS 10
int ActiveRun::ParsePartitionClass(const string& cl)
{
 int clg_defaults[MAXCLASSGROUPS]= {0,1,1,1,1,1,6,7,119,9};  
 vector<string> classitems;
 //cout << cl << endl;
 splitstring(cl,classitems,"()=,");
 int nsize=classitems.size();
 //for( int i=0;i< nsize;i++)cout << classitems[i] << " ";
 // Find class in classes: name -> 2 , group-> last
 for(int i=0;i<nclass;i++){
   if((fClasses[i]->GetName()).find(classitems[2]) != string::npos){
     // cout << "Name found: "  << classitems[2] << " " << classitems[nsize-1] << endl;
     // add group to it
     int group = atoi(classitems[nsize-1].c_str());
     fClasses[i]->SetGroup(group);
     fClasses[i]->SetTime(clg_defaults[group]);
     return 0;
   }
 }
 cout << "ParsePartitionClass warning: class not found: " << cl << endl;
 return 1;
}
//--------------------------------------------------------------
int ActiveRun::ProcessPartitionLine(const string &line,int& level)
{
 //cout << line << endl;
 if(line.size()==0) return 0;
 size_t ix=0;
 while(ix < line.size() && line.at(ix)==' ')ix++;
 if(line.at(ix)=='#') return 0;
 if((ix=line.find("Version:")) != string::npos){
  string ver=line.substr(ix+8);
  //cout << "ver: " << ver << endl;
  return 0;
 }
 if((line.find("Clusters:") != string::npos)){
  level=1;
  return 0;
 }
 if(((ix=line.find("BCM")) != string::npos) && (ix==0)){
  //cout << "ix BCM = " << ix << endl;
  level=2;
  return 0;
 }
 vector<string> items;
 splitstring(line,items," ");
 int nitems = items.size();
 //cout << "# of items: " << nitems <<  " level= " << level << endl;
 switch (level){
   case 1:  // Clusters
        if(nitems == 1){
          // Cluster name
          //cout << "Cluster: " << line << endl;
        }else if(line.find("cn") != string::npos){
          // cluster classes
          for(int i=0;i<nitems;i++)ParsePartitionClass(items[i]);
        }else{
          // cluster detector
        };          
        return 0;
 }
 return 0;
}
//-----------------------------------------------------------
int ActiveRun::ParseConfigFile(int runnumber){
 ifstream file;
 stringstream ss;
 if(runnumber){
   ss << "/WORK/RCFG/r" <<runnumber << ".rcfg";
   frcfgfile = getenv("VMEWORKDIR")+ss.str();
 }else{
   ss << "/CFG/ctp/monscal/inputs.rcfg";
  frcfgfile = getenv("VMECFDIR")+ss.str();
 }
 file.open(frcfgfile.c_str());
 if(!file){
  PrintLog(("ActiveRun: cannot open file: "+frcfgfile).c_str());
  error=1;
  return 1;
 }else{
  PrintLog(("ActiveRun: File: "+frcfgfile+" opened.").c_str());
 }
 string line;
 while(getline(file,line)){
   //cout << line << endl;
   int level;
   if(ProcessCfgLine(line,level)) return 1;
 }
 return 0;
}
int ActiveRun::ParsePartitionFile(int runnumber){
 ifstream file;
 stringstream ss;
 //runnumber=104160;
 if(runnumber){
   ss << "/WORK/PCFG/r" <<runnumber << ".partition";
   partifile = getenv("VMEWORKDIR")+ss.str();
 }else{
  return 0;
 }
 file.open(partifile.c_str());
 if(!file){
  PrintLog(("ActiveRun: cannot open file: "+partifile).c_str());
  error=1;
  return 1;
 }else{
  PrintLog(("ActiveRun: File: "+partifile+" opened.").c_str());
 }
 string line;
 while(getline(file,line)){
   //cout << line << endl;
   int level;
   if(ProcessPartitionLine(line,level)) return 1;
 }
 return 0;
}
void ActiveRun::PrintInputs()
{
 for(int i=0;i<ninp;i++)fTrigInputs[i]->Print();
}
void ActiveRun::PrintClusters()
{
 for(int i=0;i<nclust;i++)fClusters[i]->Print();
}
void ActiveRun::PrintClasses()
{
 for(int i=0;i<nclass;i++)fClasses[i]->Print();
}
void ActiveRun::PrintDetectors()
{
 for(int i=0;i<ndet;i++)fDetectors[i]->Print();
}
void ActiveRun::UpdateRunCounters(w32* buffer)
{
 times.Update(buffer);
 if(fINT)fINT->Update(buffer);
 for(int i=0;i<ninp;i++)fTrigInputs[i]->Update(buffer);
 for(int i=0;i<nclust;i++)fClusters[i]->Update3(buffer);
 for(int i=0;i<ndet;i++)fDetectors[i]->Update(buffer);
}
void ActiveRun::Write2DAQlogbook()
{
 daq->UpdateClasses(nclass,fClasses);
 daq->UpdateDetectors(ndet,fDetectors);
}

