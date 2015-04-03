////////////////////////////////////////////////////////////////////////////
// Classes for testing new firmware, 
// Usable for transition to c++ version of ctpcfg , loadtoHW, ..... 
////////////////////////////////////////////////////////////////////////////
#include <string>
#include <vector>
#include <fstream>
using namespace std;
//###########################################################################
enum { NINP=60+4, NDET=24, NFUN=4};
class TriggerClass;
class TriggerInput;
struct TKlas;
class TriggerCluster{
 private:
         string fname;
         char* fcname;
         int fhwindex;
         int nclass;
         int ndet;
         string fDetectors[NDET];
         TriggerClass* fTClasses[NCLASS];
 public:
         TriggerCluster(string &name,int hwindex);
	~TriggerCluster();
         void AddDetector(string& name);
         void AddClass(TriggerClass* cls){fTClasses[nclass++]=cls;};
         string& GetName() {return fname;};
         char* GetNamechar() {return fcname;};
         int GetIndex(){return fhwindex;};
         int GetNdet(){return ndet;};
         string* GetDetectors(){return fDetectors;};
         void Print();
         void PrintDets();
         void PrintDets(ofstream *file);
	 int Load2SW(w32 run);
};
class TriggerL0fun{
  private:
           string fname;
           string logic1;
           string logic2;
           int finps;  // =4 inputs for ols; 12 for new
           int ninps;  // actual number of inputs
	   w32 fpos;   // position of function in CTPHardware
           vector<string> inputs;
	   vector<int> index;
           bool* lut1; // look up table
           bool* lut2; // seconf lut for new functions
           void replace(string& logic,string inp,int bit);
           int calculateLUT(bool *lut,string& logic);
           int FindInputs(int ninp,TriggerInput* inputs[]);
           int splitl0fun34();
  public:
          TriggerL0fun(const string& name);
	  ~TriggerL0fun();
          int Initialise(int ninp,TriggerInput* inputs[],const string& line);
	  string& GetName(){return fname;};
	  w32 GetPos(){return fpos;};
 	  void Print();
	  int Load2SW(w32 run);
	  int Load2HW(w32 run);
};
class TriggerDescriptor{
  private:
         string fname;
         vector<string> inputs; // including L0fun
         TriggerL0fun* fL0fun[NFUN];
         TriggerInput* fTrigInputs[NINP];
         int nall;   // inputs + functions
	 int ninps,nl0f;
         int FindInputs(int ninp,int nl0f,TriggerInput* inputs[],TriggerL0fun* l0f[]);
  public:
         TriggerDescriptor(int nall,string&name, vector<string> inputs);
         int Initialise(int ninp,int nl0f,TriggerInput* inputs[],TriggerL0fun* fl0f[]);
         void SetName(string& name){fname=name;};
	 vector<string>& GetInputs(){return inputs;};
         string& GetName(){return fname;};
	 int Getninps(){return ninps;};
	 int Getnl0f(){return nl0f;};
	 TriggerInput** GetTInputs(){return fTrigInputs;};
	 TriggerL0fun** GetTL0fun(){return fL0fun;};
	 void Print();
};
//#########################################################################
class TriggerClass
{
 private:
                  string   fname;
                      w8   fIndex;        // position of class in mask
       TriggerDescriptor*  fDescriptor;   // pointer to the descriptor
          TriggerCluster*  fCluster;      // pointer to the cluster
//  AliTriggerPFProtection* fPFProtection; // pointer to the past-future protection
//        AliTriggerBCMask* fMask;         // pointer to bunch-crossing mask
                       w32 fPrescaler;    // Downscaling factor
                      bool  fAllRare;      // All or Rare trigger
		       w32  fGroup;        // time sharing group
        	       w32  fTime;         // time allowed
//                     TKlas* klas;          // interface to old klas
	           
 public:
          TriggerClass(string &name,w8 index, TriggerCluster *cluster);
          TriggerClass(string &name,w8 index, TriggerCluster *cluster,w32 group,w32 time);
          TriggerClass(string &name,w8 index, TriggerCluster *cluster,TriggerDescriptor *td,w32 group,w32 time);
          ~TriggerClass();
          w8 GetIndex(){return fIndex;};
          w8 GetIndex0(){return (fIndex-1);};
          string& GetName(){return fname;};
          //TriggerDescriptor& GetDescriptor(){return *fDescriptor;};
          TriggerDescriptor* GetDescriptor(){return fDescriptor;};
	  //int Load2SW(w32 run);  loads vie clusters
          void Print();
};
//#########################################################################
class TriggerInput{
 private:
	string fname;
        int flevel;
        int fposition;
	int fsignature;
	w32 fpos; // position of bc/rnd in CTPHardware
        string fdetname;
 public:
        TriggerInput(string &name,int level,int position,string &detname);
        TriggerInput(string &name,int level,int position,int signature,string &detname);
        string& GetName(){return fname;}; 
        string& GetDetName(){return fdetname;}
        int GetPosition(){return fposition;};
        int GetSignature(){return fsignature;};
        int GetLevel(){return flevel;} 
	w32 GetPos(){return fpos;};
	int Load2SW(w32 run);
        void Print();
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
	~Detector();
        Detector(const Detector &dec);
        Detector& operator= (const Detector& dec);
        const string& GetName(){return fname;};
        int GetDAQdet(){return DAQdet;};
        int GetFo(){return fo;};
        int GetFoCon(){return focon;};
        void Print();
};
//////////////////////////////////////////////////////////////////////////
class VALIDLTUS
{
 private:
         enum { NITEMS=8};
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
//================================================================================
class ActiveRun 
{
 private:
         //enum {NCLASS=50, NCLUST=6, NINP=60, NDET=24}; already defined before
         //enum { NINP=60, NDET=24, NFUN=4}; // defined globally
         int fRunNumber;
         string fname;
         string frcfgfile;
         string partifile;
         int ninp,nclass,nclust,ndet,ntd,nl0f;
         vector<string> inputlist;
         TriggerL0fun* fL0fun[NFUN];
         TriggerDescriptor* fTrigDesc[NCLASS];
         TriggerInput* fTrigInputs[NINP];
         TriggerClass* fClasses[NCLASS];
         TriggerCluster* fClusters[NCLUST];
         Detector* fDetectors[NDET];
         void SetDefaultInputs();
         int ProcessCfgLine(const string& line,int& level);
         int ProcessInputLine(const string& line);
         int ProcessPartitionLine(const string& line,int& level);
         int ParseConfigFile(int runnum);
         int ParsePartitionFile(int runnum);
         int ParsePartitionClass(const string& classstring);
         int FindDetectors();
         int ParseInputsList();
         int ParseValidCTPInputs();
 public:
         ActiveRun();
         ActiveRun(const int runnum);
         ~ActiveRun();
         int GetRunNumber(){return fRunNumber;};
         int Getninp(){return ninp;};
	 void AddL0fun(TriggerL0fun* l0f){fL0fun[nl0f++]=l0f;};
         void AddTrigDesc(TriggerDescriptor* td){fTrigDesc[ntd++]=td;};
         void AddClass(TriggerClass* clss){fClasses[nclass++]=clss;};
         void AddCluster(TriggerCluster* cls){fClusters[nclust++]=cls;};
         void AddInput(TriggerInput* inp){fTrigInputs[ninp++]=inp;};
         void UpdateRunCounters(w32* buffer);
         void PrintActiveRun();
         void PrintInputs();
	 void PrintL0fun();
	 void PrintDescriptors();
         void PrintClusters();
         void PrintClasses();
         void PrintDetectors();
         int Load2SW();
         void Load2HW();
};
class VMEaddress
{
  private:
          string name;
          const bool shared;  // specify address is shared amonf runs
          const w32 address;
          w32 value;          // use when only one value for address
	  w32 nruns;           // number of runs using this address
          w32 runnumber[6];   // list of runs which use address
  public:
         VMEaddress(string name,const w32 address,const bool shared);
	 virtual ~VMEaddress();
         virtual void Write();
         int SetValue(w32 run,w32 val);
         void SetValue(w32 val){value=val;};
         void SetRunnumber(w32 run){runnumber[nruns++]=run;};
	 void UnloadRun(w32 run);
         w32* GetRunnumbers(){return runnumber;};
         w32 GetRunnumber(){return runnumber[0];};
         w32 GetValue(){return value;};
         w32 GetAddress(){return address;};
	 bool GetShared(){return shared;};
	 w32 Getnruns(){return nruns;};
	 string& GetName(){return name;};
	 void Print();
};
class VMEaddressL0fun : public VMEaddress
{
 // New L0 function
 private:
        enum {NSIZE = 0x1000};
        bool *lut1,*lut2,*lut3,*lut4;
        w32 nruns2;
	w32  runnumber2[6];   
 public:
        VMEaddressL0fun(string name,const w32 address);
	virtual ~VMEaddressL0fun(){};
        virtual void Write();
	virtual int SetData(w32 run,bool* lut1,bool* lut2,w32& pos);
};
class CTPHardware
{
 private:
         //enum { NCLASS=50 };
         static w32* ClustersInRuns;   // max NCLUST, position is HW slot
         static VMEaddress L0FUNCTION_1,L0FUNCTION_2;
	 static VMEaddress RND1,RND2,BC1,BC2;
         static VMEaddressL0fun L0FUNCTION_34;
	 static VMEaddress* L0CONDITION[NCLASS];
	 static VMEaddress* L0VETO[NCLASS];
	 static VMEaddress* L0MASK[NCLASS];
	 static VMEaddress* L0INVERT[NCLASS];
 public:
         CTPHardware();
         ~CTPHardware();
	 static vector<VMEaddress*> Addresses;
         static int SetValueL0fun4(w32 run,w32 value,w32& pos);
         static int SetValueL0fun12(w32 run,bool* lut1,bool* lut2,w32& pos);
	 static int SetCTPCondition(w32 run,string& name,w32 value,w32& pos);
         static int SetClass(w32 valueL0COND,w32 valueL0VETO,w32 valueL0INV);
	 static w32 AddCluster(w32 run);  // Add cluster of the run
	 static bool IsRunLoaded(w32 run);
  	 static void Load(w32 run);
	 static void Unload(w32 run);
         static w32 GetL0version(){return vmer32(FPGAVERSION_ADD+0x9000);};
	 static void Print();
};
