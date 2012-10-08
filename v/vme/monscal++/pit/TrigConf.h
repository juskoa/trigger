#ifndef _TrigConf_h_
#define _TrigConf_h_
#include <string>
#include <fstream>
#include "Counter.h"
#include <vector>

enum {NINP=24};
enum {NCLASS=50};
enum bcmtype {B, A, C, E, S, U};
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
         double GetRate1(){return Int1.GetRate();};
         double GetRate2(){return Int2.GetRate();};
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
	   w32 GetActiveTGroup(){return TimeSec.GetActiveTGroup();};
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
        Counter* GetCounter(){return &cnt;};
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
          Counter l2s,l2r,pp;
 public:
        DetectorwCount(const Detector &dec);   
        w64 GetL2aCount(){return (l2s.GetCountTot()-l2r.GetCountTot());};
        w64 GetPPCount(){return pp.GetCountTot();};
        void Update(w32* buffer);
};
//###########################################################################
class TriggerClass;
class TriggerClasswCount;
class TriggerCluster{
 private:
         enum {NDET=24};
         string fname;
         char* fcname;
         int fhwindex;
         int ndet;
         string fDetectors[NDET];
         //TriggerClass* fTClasses[NCLASS];
 public:
         TriggerCluster(string &name,int hwindex);
	~TriggerCluster();
         void AddDetector(string& name);
         string& GetName() {return fname;};
         char* GetNamechar() {return fcname;};
         int GetIndex(){return fhwindex;};
         int GetIndex0(){return (fhwindex-1);};
         int GetNdet(){return ndet;};
         string* GetDetectors(){return fDetectors;};
         void Print();
         void PrintDets();
         void PrintDets(ofstream *file);
};
class TriggerClusterwCount:public TriggerCluster
{
 private:
	 int nclass;
         Counter l0,l2,busy;
         TriggerClasswCount* fTClasses[NCLASS];
         void DisplayClusterHeader(ofstream* file);
         void DisplayClusterTotal(ofstream* file);
 public:
        TriggerClusterwCount(string &name,int hwindex);
        ~TriggerClusterwCount();
        void AddClass(TriggerClasswCount* cls){fTClasses[nclass++]=cls;};
        int GetNumofClasses(){return nclass;};
        TriggerClasswCount* GetTriggerClass(int pos){return fTClasses[pos];};
        Counter* GetL0Counter(){return &l0;};
        w64 GetL2aCount(){return l2.GetCountTot();};
        void Update3(w32* buffer);
        void DisplayCluster(ofstream* file);
        void DisplayClusterSortBCM(ofstream* file);
        void Print();
};
//#########################################################################
class TriggerClass
{
 private:
                  string   fname;
	    	  string   fnamedesc;     // first part of classname corresponding to descriptor
                      w8   fIndex;        // position of class in mask
//       TriggerDescriptor*  fDescriptor;   // pointer to the descriptor
          TriggerCluster*  fCluster;      // pointer to the cluster
//  AliTriggerPFProtection* fPFProtection; // pointer to the past-future protection
//        AliTriggerBCMask* fMask;         // pointer to bunch-crossing mask
	           bcmtype  BCMtype;
//                  UInt_t  fPrescaler;    // Downscaling factor
//                  Bool_t  fAllRare;      // All or Rare trigger
	           
 public:
          TriggerClass(string &name,w8 index, TriggerCluster *cluster);
          ~TriggerClass();
          w8 GetIndex(){return fIndex;};
          w8 GetIndex0(){return (fIndex-1);};
          string& GetName(){return fname;};
          string& GetNameDesc() {return fnamedesc;};
          bcmtype GetBCMtype(){return BCMtype;};
	  void ParseClassName();
          void Print();
};
class TriggerClasswCount:public TriggerClass
{
 private:
        enum {NSHORT = 16};
        Counter cnts[6];
        char fnameS[16];  // first part of trigger class
        w32  fGroup;        // time sharing group
        w32  fTime;         // time allowed
        bool isActive;      // 1 = fGroup active
        void CreateShortName();
 public: 
       TriggerClasswCount(string &name,w8 index, TriggerCluster *cluster);
       TriggerClasswCount(string &name,w8 index, TriggerCluster *cluster,w32 groupname,w32 grouptime);
       void Update(w32* buffer);
       void SetTime(w32 time){fTime=time;};
       void SetGroup(w32 group){fGroup=group;};
       char* GetShortName(){return fnameS;};
       Counter* GetCounters(){return cnts;};
       w64 GetL2aCountTot(){return cnts[5].GetCountTot();};
       w64 GetL0bCountTot(){return cnts[0].GetCountTot();};
       w32 GetL0bCount(){return cnts[0].GetCount();};
       int GetGroup(){return fGroup;};
       int GetTime(){return fTime;};
       bool IsActive(){if(fGroup) return isActive; else return 1;};
       void DisplayClass(ofstream* file);
       void Print();
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
