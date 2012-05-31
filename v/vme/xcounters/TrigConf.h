#ifndef _TrigConf_h_
#define _TrigConf_h_
#include <string>
#include <fstream>
#include "Counter.h"
#include <vector>

using namespace std;
//#########################################################################
class InteractionwCount
{
 private:
         // function definition not necessary now
         Counter Int1,Int2;
         //Counter IntT,IntA,IntB,IntD; do we need these also ?
 public:
         InteractionwCount();
         void Update(w32* buffer);
         void DisplayInt1(char* text);
         void DisplayInt2(char* text);
};
//#########################################################################
class TrigTimeCounters{
   private:
            Counter TimeSec,TimeUsec,Orbit;
            w32 PeriodCounter;
   public:
           TrigTimeCounters();
           void Update(w32* buffer);
           w32 GetOrbit(){return Orbit.GetNow();};
           w32 GetPeriodCounter(){return PeriodCounter;};
           w32 GetSecs(){return TimeSec.GetNow();};
           w32 GetUsecs(){return TimeUsec.GetNow();};
};
//#########################################################################
class TriggerInput{
 private:
        int fposition;
        int flevel;
	string fname;
        string fdetname;
 public:
        TriggerInput(string &name,int level,int position,string &detname);
        string& GetName(){return fname;}; 
        string& GetDetName(){return fdetname;}
        int GetPosition(){return fposition;};
        int GetLevel(){return flevel;} 
        void Print();
};
class TriggerInputwCount: public TriggerInput{
 private:
        Counter cnt;
 public:
        TriggerInputwCount(string &name,int level,int position,string &detname);
        void Update(w32* buffer);
        void Display(ofstream* file);
        void Display(char* text);
};
class TriggerDescriptor{
  private:
         string fname;
         TriggerInput* fDescriptors; 
  public:
};
class Detector{
 protected:
        string fname;
        int DAQdet;
        int fo,focon;
        int busyinp;
  public:
        Detector(string& name);
        Detector(string& name,int DAQdet,int fo,int focon,int busyinp);
        Detector(const Detector &dec);
        Detector& operator= (const Detector& dec);
        const string& GetName(){return fname;};
        int GetDAQdet(){return DAQdet;};
        int GetFo(){return fo;};
        int GetFoCon(){return focon;};
        void Print();
};
class DetectorwCount : public Detector
{
 private:
          Counter l2a;
 public:
        DetectorwCount(const Detector &dec);   
        w64 GetL2aCount(){return l2a.GetCountTot();};
        void Update(w32* buffer);
};
//###########################################################################
class TriggerClass;
class TriggerClasswCount;
class TriggerCluster{
 private:
         enum {NDET=24};
         string fname;
         int fhwindex;
         int ndet;
         string fDetectors[NDET];
         //TriggerClass* fTClasses[NCLASS];
 public:
         TriggerCluster(string &name,int hwindex);
	~TriggerCluster();
         void AddDetector(string& name);
         string& GetName() {return fname;};
         int GetIndex(){return fhwindex;};
         int GetNdet(){return ndet;};
         string* GetDetectors(){return fDetectors;};
         void Print();
         void PrintDets();
         void PrintDets(ofstream *file);
};
class TriggerClusterwCount:public TriggerCluster
{
 private:
         enum {NCLASS=50};
	 int nclass;
         Counter l0,l2,busy;
         TriggerClasswCount* fTClasses[NCLASS];
         void DisplayClusterHeader(ofstream* file);
         void DisplayClusterTotal(ofstream* file);
 public:
        TriggerClusterwCount(string &name,int hwindex);
        ~TriggerClusterwCount();
        void AddClass(TriggerClasswCount* cls){fTClasses[nclass++]=cls;};
        void Update3(w32* buffer);
        void DisplayCluster(ofstream* file);
};
//#########################################################################
class TriggerClass
{
 private:
                  string   fname;
                     int   fIndex;        // position of class in mask
//       TriggerDescriptor*  fDescriptor;   // pointer to the descriptor
          TriggerCluster*  fCluster;      // pointer to the cluster
                      int  fGroup;        // time sharing group
                      int  fTime;         // time allowed
//  AliTriggerPFProtection* fPFProtection; // pointer to the past-future protection
//        AliTriggerBCMask* fMask;         // pointer to bunch-crossing mask
//                  UInt_t  fPrescaler;    // Downscaling factor
//                  Bool_t  fAllRare;      // All or Rare trigger
	           
 public:
          TriggerClass(string &name,int index, TriggerCluster *cluster);
          ~TriggerClass();
          int GetIndex(){return fIndex;};
          unsigned char GetIndex0(){return (fIndex-1);};
          string& GetName(){return fname;};
          int GetGroup(){return fGroup;};
          int GetTime(){return fTime;};
          void SetTime(int time){fTime=time;};
          void SetGroup(int group){fGroup=group;};
          void Print();
};
class TriggerClasswCount:public TriggerClass
{
 private:
        Counter cnts[6];
 public: 
       TriggerClasswCount(string &name,int index, TriggerCluster *cluster);
       void Update(w32* buffer);
       Counter* GetCounters(){return cnts;};
       w64 GetL2aCount(){return cnts[5].GetCountTot();};
       void DisplayClass(ofstream* file);
};
//////////////////////////////////////////////////////////////////////////
class VALIDLTUS
{
 private:
         enum {NDET=24, NITEMS=8};
         int ndet;
         ifstream file;
         static Detector* dets[];
         static int count;
 public:
	 VALIDLTUS();
         int readVALIDLTUS();
         int ProcessLine(const string& line);
         void AddDetector(Detector &det);
         static Detector* GetDetector(const int daqdet);
         static Detector* GetDetector(const string& name);
         void Print();
};
#endif
