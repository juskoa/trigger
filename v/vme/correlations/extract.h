#include <string>
#include <vector>
#include <iostream>
#include "TFile.h"
#include "TObjArray.h"
typedef unsigned int w32;
using namespace std;

class Hists
{
 private:
         TObjArray hists;  // correlations
         TObjArray hists2; // length and orbit
 public:
         Hists();
	 ~Hists();
         int addHist(string const &name,int delta,int cordist);
         int addHist2(string const &name,int nbins,float x0, float xmax);
         int fillHist(int i,int bin,float x);
         int fillHist2(int i,int bin,float x);
         int writeHists();
         void printAllHists();
};
//---------------------------------
class ssmpoint
{
 public:
 int issm,chanabs,chanrel,position;
 ssmpoint(int is,int ch,int chr,int pos){
  issm=is;chanabs=ch;chanrel=chr;position=pos;
 };
 void Print();
};
//-------------------------------------------------------
class Input
{
 public:
         string name;
         int delay,index;
         bool used;
         Input(string& name,int delay,int index);
};
//---------------------------------------------------
class extractData : public Hists
{
 private:
        int checkInputs(int issm);
        void check1SSM(int issm);
        int extract1SSM(int issm);
        int extract1SSMfilt1(int issm);
        int checkSSMforData(int issm);
        int fill(int chan1,int chan2,int dist);
        int ParseVALIDCTPINPUTS();
        vector<string> filelist;
        vector<string> ssmdumps;
        vector<ssmpoint> data;
        vector<ssmpoint> orbits;
        vector<string> inputNames;
        vector<Input> inputs;
        string FileList_d;
        string DataDir_d;
        string VALIDCTPINPUTS_d;
        int cordist_d,delta_d;
        int dcordist_d,ddelta_d;
        int nfiles;    // if < nlines only nfiles are read
        int nlines_d;    // # of lines in file list = # files (filelist)
        int nssms;     // # of non empty ssms= at least 2 signals (ssmsumps)
        int ninputs,ncorrelations;
        int *(*correlations);
        int *(*length);
        int *(*orbit);
        TFile *outFile;
        int const Nchans;
        int *chosenInputs;
        int *chan2inp;
        int *chans;     // number of signal
        int *chansd;     // number of signals closer than xxx
        int SkipNext;   // number of BCs to skip after first detected
 public:
        //extractData(string const &FileList,string const &DataDir);
        extractData(char* config);
        int ReadConfig(char* config);
        void chooseInputs();
        int readFileList();
        int removeEmptySSMs(bool remove);
        void checkAllSSM();
        void extractAllSSM();
        void checkAllSSM1by1();
        int correlateAllSSM(int cordist,int delta);
        int distance2Orbit();
        void printFileList();
        void printData();
        void printCorrelations();
        void printDistance();
        void SetSkipNext(int skip){SkipNext=skip;}
        void SetNfiles(int n){nfiles=n;};
};
