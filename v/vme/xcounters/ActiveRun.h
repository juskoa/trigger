#ifndef _ActiveRun_h_
#define _ActiveRun_h_
#include "Output.h"
#include "TrigConf.h"
#include <vector>
#include <string>
typedef unsigned int w32;
using namespace std;
//---------------------------------------------------------------------------------------------------------
class ActiveRun : public Log
{
 private:
         enum {NCLASS=50, NCLUST=6,NINP=60, NDET=24};
         const int fRunNumber;
         string fname;
         string frcfgfile;
         string partifile;
         int ninp,nclass,nclust,ndet;
         bool error;
         bool copycounters2dcs;
         CountersOCDB* ocdb;
         DAQlogbook *daq;
         DisplaySCAL *scal;
         TrigTimeCounters times;
         InteractionwCount* fINT;
         TriggerInputwCount* fTrigInputs[NINP];
         TriggerClasswCount* fClasses[NCLASS];
         TriggerClusterwCount* fClusters[NCLUST];
         DetectorwCount* fDetectors[NDET];
         int ActiveRun::ProcessCfgLine(const string& line,int& level);
         int ActiveRun::ProcessPartitionLine(const string& line,int& level);
         int ParseConfigFile(int runnum);
         int ParsePartitionFile(int runnum);
         int ParsePartitionClass(const string& classstring);
         int FindDetectors();
 public:
         ActiveRun();
         ActiveRun(const int runnum);
         ~ActiveRun();
         int GetRunNumber(){return fRunNumber;};
         int Getninp(){return ninp;};
         bool GetError(){return error;};
         void AddClass(TriggerClasswCount* clss){fClasses[nclass++]=clss;};
         void AddCluster(TriggerClusterwCount* cls){fClusters[nclust++]=cls;};
         void AddInput(TriggerInputwCount* inp){fTrigInputs[ninp++]=inp;};
         void UpdateRunCounters(w32* buffer);
         void PrintInputs();
         void PrintClusters();
         void PrintClasses();
         void PrintDetectors();
         //----------DisplayonScreen
         void CreateDisplaySCAL(){scal = new DisplaySCAL(fRunNumber,fname);};
         void DisplayRun(){scal->DisplayRun(ninp,fTrigInputs,nclust,fClusters,fINT);DisplaySCAL::GetfileSCAL()->flush();};
         //----------CountersOCDB
         void CreateCountersOCDB(bool copy2dcs){ocdb = new CountersOCDB(1,fRunNumber,copy2dcs);};
         void WriteHeaderOCDB(){ocdb->WriteHeader(nclass,fClasses);};
	 void WriteRecordOCDB(){ocdb->WriteRecord(&times,nclass,fClasses);};
         //-----------DAQlogbook
         void CreateDAQlogbook(int log=0){daq = new DAQlogbook(fRunNumber,log);}
         void Write2DAQlogbook();
};
#endif
