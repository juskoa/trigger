#ifndef _Output_h_
#define _Output_h_
#include "TrigConf.h"
#include "Log.h"
#include <string>
#include <fstream>

using namespace std;
class DisplaySCAL : public Log
{
 private:
         const int runnum;
         const string name;
         static ofstream *fileSCAL;
         void InsertChar(char cc);
         void InsertChar(int nn,char cc);
         void DisplayRunHeader();
         void DisplayInputs(const int ninp, TriggerInputwCount* inps[]);
         void DisplayInputsInts2C(const int ninp, TriggerInputwCount* inps[], InteractionwCount* inter);
         void DisplayInputs2C(const int ninp, TriggerInputwCount* inps[]);
         void DisplayRatiosWU(const int ninp,TriggerInputwCount* inps[],const int nclust, TriggerClusterwCount* clusts[]);
         void DisplayRatiosVLH(const int ninp,TriggerInputwCount* inps[],const int nclust, TriggerClusterwCount* clusts[]);
         void DisplayCalibTrigs(const int ndet, DetectorwCount* dets[]);
	 void displayratio(double low,double high,TriggerClasswCount* tnom,TriggerClasswCount* tden,const char* name);
 public:
 	DisplaySCAL(const int runnum,const string& name);
 	~DisplaySCAL();
        static ofstream* GetfileSCAL(){return fileSCAL;};
        void DisplayRun(const int ninp,TriggerInputwCount* inps[],const int nclust, TriggerClusterwCount* clusts[],InteractionwCount* inter,const int ndet, DetectorwCount* dets[]);

};
//--------------------------------------------------------------------------------
class CountersOCDB: public Log
{
 private:
         const int runnum;
         const int version;  // file format version
         bool copy2dcs;       // 1 = copy; 0 = dont copy
         string fileName;
         string fileNameAliases;
         ofstream file;
 public:
         CountersOCDB();        
         CountersOCDB(const int version,const int runnum,const bool copy2dcs);
         ~CountersOCDB();
         int WriteHeader(const int nclass,TriggerClasswCount* tclass[]);    
         int WriteRecord(TrigTimeCounters* time, const int nclass,TriggerClasswCount* tclass[]);
};
//-----------------------------------------------------------------------------------------
class DAQlogbook : public Log
{
 private:
         const int runnum;
         string fileName;
         ofstream file;
         int log;  
         int count;
 public:
        static int daqnotopen;
	DAQlogbook(const int runnum,int log);
	~DAQlogbook();
        void UpdateClasses(const int nclass,TriggerClasswCount* tclass[]);
	void UpdateClusters(const int nclust,TriggerClusterwCount* tclust[]);
        void UpdateDetectors(const int ndet,DetectorwCount* dets[]);
	void UpdateInputs(const int ninp,TriggerInputwCount* inps[]);
        void UpdateL2a(Counter &L2a);
};
#endif
