#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
#include "TList.h"
#include "TObjArray.h"
#include "TCanvas.h"
#define Mega 1024*1024
typedef unsigned int w32;

using namespace std;
//-----------------------------------------------------------------------
void splitstring(const string& str,
                      vector<string>& tokens,
                      const string& delimiters = " ")
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}
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
Hists::Hists(){
}
Hists::~Hists(){
}
int Hists::addHist(string const &name,int delta,int cordist){
 int nbins=2*delta+1;
 float x0=cordist-delta;
 float xm=cordist+delta;
 TH1F *h = new TH1F(name.c_str(),name.c_str(),nbins,x0,xm);
 hists.Add(h);
 return 0;
}
int Hists::addHist2(string const &name,int nbins,float x0,float xmax){
 TH1F *h = new TH1F(name.c_str(),name.c_str(),nbins,x0,xmax);
 hists2.Add(h);
 return 0;
}
int Hists::fillHist(int i,int bin,float x){
 TH1F *h = (TH1F*) hists[i];
 //cout << "i= "<< i << ":" << h->GetName() << endl;
 h->SetBinContent(bin,x);
 return 0;
}
int Hists::fillHist2(int i,int bin,float x){
 TH1F *h = (TH1F*) hists2[i];
 //cout << "i= "<< i << ":" << h->GetName() << endl;
 h->SetBinContent(bin,x);
 return 0;
}
int Hists::writeHists(){
  TFile *file;
  file = new TFile("pdf/Histos.root","RECREATE","FILE");
  hists.Write();
  hists2.Write();
  file->Close();
 return 0;
}
void Hists::printAllHists(){
 int n=hists.GetEntriesFast();
 TCanvas c1("c1","c1",900,20,540,550);
 for(int i=0;i<n;i++){
   hists[i]->Draw(); 
   cout << (hists[i])->GetName() << " saved" << endl;
   string ext(".pdf");
   string pdf("pdf/");
   string file(pdf+hists[i]->GetName()+ext);
   c1.SaveAs(file.c_str());
 }
}
//---------------------------------
class ssmpoint
{
 public:
 int issm,chanabs,chanrel,position;
 ssmpoint(int is,int ch,int chr,int pos){
  issm=is;chanabs=ch;chanrel=chr;position=pos;
 };
 void Print(){
  cout << "ssm="<< issm << " ch=" << chanabs<<" ch2inp=" << chanrel << " pos=" << position << endl;
 }
};
//-------------------------------------------------------
class Input
{
 public:
         string name;
         int delay,index;
         Input(string& name,int delay,int index);
};
Input::Input(string &name,int delay,int index)
:
name(name),delay(delay),index(index)
{}
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
        extractData();
        int ReadConfig();
        void chooseInputs();
        int readFileList();
        int removeEmptySSMs();
        void checkAllSSM();
        void extractAllSSM();
        void checkAllSSM1by1();
        int correlateAllSSM(int cordist,int delta);
        int distance2Orbit(int cordist,int delta);
        void printFileList();
        void printData();
        void printCorrelations();
        void printDistance();
        void SetSkipNext(int skip){SkipNext=skip;}
        void SetNfiles(int n){nfiles=n;};
};
extractData::extractData():
Nchans(32)
{
 //FileList_d=FileList;
 //DataDir_d=DataDir;
 if(ReadConfig()) exit(1);
 cordist_d=0;delta_d=0;
 nfiles=0;
 chosenInputs = new int[Nchans];
 chan2inp = new int[Nchans];
 chans = new int[Nchans];
 chansd = new int[Nchans];
 nssms=0;
}
//------------------------------------------------------
int extractData::ReadConfig()
{
 ifstream in;
 string config("config.txt");
 in.open(config.c_str());
  if(!in.is_open()){
   cout << "ReadConfig: File not found: "<< config << endl;
   return 1;
  }
  int nlines=0;
  string line;
  while (getline(in,line)) {
      cout << nlines << " " << line << endl;
      if(line[0]=='#') continue;
      if(nlines==0){
       VALIDCTPINPUTS_d=line;
       if(ParseVALIDCTPINPUTS()) return 1;
       nlines++;
       continue;
      }
      if(nlines==1){
       DataDir_d=line;
       cout << "datadir " << DataDir_d << endl;
       nlines++;
       continue;
      }
      if(nlines==2){
       FileList_d=line;
       nlines++;
       continue;
      }
      if(nlines>2){
       nlines++;
      }
  }
  return 0;
}
//------------------------------------------------------
int extractData::ParseVALIDCTPINPUTS()
{
  ifstream in;
  in.open(VALIDCTPINPUTS_d.c_str());
  if(!in.is_open()){
   cout << "ParseVALIDCTPINPUTS: File not found: "<< VALIDCTPINPUTS_d   << endl;
   return 1;
  }
  string line;
  int nlines=0;
  while (getline(in,line)) {
      //cout << line << endl;
      if(line[0]=='#') continue;  
      if(line[0]=='l') continue;
      vector<string> items;
      splitstring(line,items," =");
      //cout << items[0] << " " << items[4] << " " << items[8] << endl;
      //cout << items.size() << endl;
      if(items.size() != 11){
       cout << "Wrong syntax in " << VALIDCTPINPUTS_d << endl;
       cout << line << endl;
       return 1;
      }
      inputs.push_back(Input(items[0],atoi(items[4].c_str()),atoi(items[8].c_str())));
      nlines++;
   }
   in.close();
 return 0;
}
//------------------------------------------------------
void extractData::chooseInputs(){
 for(int i=0;i<Nchans;i++){
    chosenInputs[i]=0;
    chan2inp[i] = -1;
 }
 //check these
 chosenInputs[0]=4;
 inputNames.push_back("0AMU");
 chosenInputs[1]=9; 
 inputNames.push_back("0SMB");
 chosenInputs[2]=12; 
 inputNames.push_back("0EMC");
 chosenInputs[3]=15;
 inputNames.push_back("0T0A");
 chosenInputs[4]=16;
 inputNames.push_back("0T0C");
 chosenInputs[5]=19;
 inputNames.push_back("0VGA");

 //0msl
 chosenInputs[6]=11;
 inputNames.push_back("0MSL");
 //vba
 chosenInputs[7]=7;
 inputNames.push_back("0VBA");
 //vbc
 chosenInputs[8]=20;
 inputNames.push_back("0VBC");

  chosenInputs[9]=17;
 inputNames.push_back("0ZSS");
 //chosenInputs[5]=20;
 //inputNames.push_back("0SCO");
 //chosenInputs[5]=21;
 //inputNames.push_back("0ASC");
 ninputs=0;
 for(int i=0;i<Nchans;i++){
   if(chosenInputs[i]){
     chan2inp[chosenInputs[i]+7]=1;
     cout << ninputs<< " ch="<<chosenInputs[i]+7<<" " << chan2inp[chosenInputs[i]+7] << endl;
     ninputs++;

     }
 }  
 cout << "ninputs=" << ninputs << endl;
 ncorrelations=ninputs*(ninputs+1)/2;
 correlations = new int*[ncorrelations];
 orbit = new int*[ninputs];
 int j=0;
 for(int i=0;i<Nchans;i++){
  if(chan2inp[i] != -1){
    chan2inp[i]=j;
    j++;
    }
 }
 for(int i=0;i<Nchans;i++){
 if(chan2inp[i] != -1){
    cout << "inp= "<< i-7 << " chan= " << i << " " << inputNames[chan2inp[i]] << endl;
   }
}
}
//------------------------------------------------------------
int extractData::extract1SSMfilt1(int issm){
// del : correlation only to the edge del[i]>0
//       del[i]=0 no limit
 ifstream in;
 string file(DataDir_d+ssmdumps[issm]);
 in.open(file.c_str(),ios::in | ios::binary);
 if(!in.is_open()){
    cout << "extract1SSMfile1: File not found: "<<  ssmdumps[issm]  << endl;
    return 1;
 }
 cout << file << " OK" << endl;
 int del[Nchans];
 for(int i=0;i<Nchans;i++)del[i]=0;
 int delorbit=0;
 w32 ssm[Mega];
 in.read((char*)ssm,sizeof(ssm));
 in.close();
 for (int i=0;i<Mega;i++){
  if(!ssm[i]) continue;
  for(int j=0;j<Nchans;j++){
   if(j==0){
    if((delorbit--)<=0 && (ssm[i] & 1)){
      ssmpoint o(issm,j,0,i);
      //p.Print();
      orbits.push_back(o);
      delorbit=41;
    }
   }
   if(chan2inp[j] != -1){
    if((del[j]--)<=0 && (ssm[i] & (1<<j))){
      ssmpoint p(issm,j,chan2inp[j],i);
      //p.Print();
      data.push_back(p);
      del[j]=SkipNext;
    }
   }
  }
 }
 return 0;
}
//------------------------------------------------------------
int extractData::extract1SSM(int issm){
 ifstream in;
 string file(DataDir_d+ssmdumps[issm]);
 in.open(file.c_str(),ios::in | ios::binary);
 if(!in.is_open()){
    cout << "extract1SSM: File not found: "<<  ssmdumps[issm]  << endl;
    return 1;
 }
 cout << file << " OK" << endl;
 w32 ssm[Mega];
 in.read((char*)ssm,sizeof(ssm));
 in.close();
 for (int i=0;i<Mega;i++){
  if(!ssm[i]) continue;
  for(int j=0;j<Nchans;j++){
   if(chan2inp[j] != -1){
    if(ssm[i] & (1<<j)){
      ssmpoint p(issm,j,chan2inp[j],i);
      //p.Print();
      data.push_back(p);
    }
   }
  }
 }
 return 0;
}
//-----------------------------------------------------------
int extractData::readFileList(){
// Read the list of ssm files
  ifstream in;
  in.open(FileList_d.c_str());
  if(!in.is_open()){
   cout << "readFileList: File not found: "<<  FileList_d  << endl;
   return 1;
  }
  nlines_d=0;
  while (1) {
      string line;
      in >> line;
      if (!in.good()) break;
      //cout << line << endl;
      if(line[0]=='#') continue;
      filelist.push_back(line); 
      nlines_d++;
      if(nfiles && nlines_d>=nfiles)break;
   }
   in.close();
   cout << " # found  lines " << nlines_d << endl;
   printFileList();
   cout << " # found  lines " << nlines_d << endl;
   return 0;
}
//--------------------------------------------------------------
void extractData::extractAllSSM(){
 //for(int i=0;i<nssms;i++)extract1SSM(i);
 for(int i=0;i<nssms;i++)extract1SSMfilt1(i);
}
//---------------------------------------------------------------
void extractData::checkAllSSM(){
 cout << "CheckALL SSM starts:" << endl;
 for(int j=0;j<Nchans;j++)chans[j]=0;
 for(int j=0;j<nlines_d;j++)checkInputs(j);
 for(int j=0;j<Nchans;j++){
  if(chans[j])
  {
    if(chan2inp[j] != -1){
     cout << "Channel: "<< j << " " <<inputNames[chan2inp[j]] << " "<< chans[j] << endl;
    }
    else cout << "Channel: "<< j << "    " << chans[j] << endl;
  }
 }
 cout << "CheckALL SSM finished." << endl;
}
//---------------------------------------------------------------
void extractData::check1SSM(int issm){
 for(int j=0;j<Nchans;j++)chans[j]=0;
 checkInputs(issm);
 for(int j=0;j<Nchans;j++){
  if((chan2inp[j] != -1) && chans[j]>0){
    if(j)cout << j-7 << " " << chans[j] << " " << chansd[0]<<endl;
    else cout << "ORBIT  " << chans[0] << " " << chansd[0] << endl;
  }
 }
}
//-------------------------------------------------------------------
void extractData::checkAllSSM1by1(){
 for(int i=0;i<nlines_d;i++)check1SSM(i);
}
//-------------------------------------------------------------------
int extractData::removeEmptySSMs(){
 int ret;
 nssms=0;
 for(int i=0;i<nlines_d;i++){
    if(!(ret=checkSSMforData(i))){
      ssmdumps.push_back(filelist[i]); 
      nssms++; 
    }else{
     if(ret==2) return 1;
    }
 }
 cout << "# of dumps after removing empty: " << nssms << endl;
 return 0;
}
//-------------------------------------------------------------------
int extractData::checkSSMforData(int issm){
 ifstream in;
 string file(DataDir_d+filelist[issm]);
 in.open(file.c_str(),ios::in | ios::binary);
 if(!in.is_open()){
    cout << "checkSSMforData: File not found: "<<  file.c_str()  << endl;
    return 2;
 }
 cout << file << " OK" << endl;
 w32 ssm[Mega];
 in.read((char*)ssm,sizeof(ssm));
 in.close();
 for(int i=0;i<Mega;i++){
  int nsig=0;
  for(int j=0;j<Nchans;j++){
   if(chan2inp[j] != -1){
     if(ssm[i]&(1<<j)){
       nsig++;
       if(nsig>0){
         cout << file << " first non zero at " << i << " " << j << endl;
         return 0;
       }
     }
   }
  }
 }
 cout << file << " removed" << endl;
 return 1; 
}
//-------------------------------------------------------------------
int extractData::checkInputs(int issm){
 ifstream in;
 string file(DataDir_d+filelist[issm]);
 in.open(file.c_str(),ios::in | ios::binary);
 if(!in.is_open()){
    cout << "checkInputs: File not found: "<<  ssmdumps[issm]  << endl;
    return 1;
 }
 int last[Nchans];
 for(int i=0;i<Nchans;i++){last[i]=0;chansd[i]=0;} // set variable later
 last[24]=Mega;
 cout << file << " ----------------------------------OK" << endl;
 w32 ssm[Mega];
 in.read((char*)ssm,sizeof(ssm));
 in.close();
 for (int i=0;i<Mega;i++){
  if(!ssm[i]) continue;
  for(int j=8;j<Nchans;j++){
   if(ssm[i] & (1<<j)){
     chans[j]++;
     last[j]=i;
   }
  }
  if(abs(last[21]-last[24])<10)chansd[0]++;
 }
 return 0;
}
//-----------------------------------------------------------
int extractData::fill(int chan1,int chan2,int dist){
 if(chan1>chan2){
  //return 0; // double counting
  dist=delta_d-dist;
  int ch=chan1; chan1=chan2;chan2=ch;
 } else dist=delta_d+dist;
 int i=(2*ninputs-chan1-1)*chan1/2+chan2;
 correlations[i][dist]++;
 return 0;
}
//-----------------------------------------------------------
int extractData::distance2Orbit(int cordist,int delta){
 for (int i=0;i<ninputs;i++){
   orbit[i]=new int[delta];
   for(int j=0;j<delta;j++)orbit[i][j]=0;
 }
 dcordist_d=cordist;
 ddelta_d=delta;
 int ndata=data.size();
 int norbit=orbits.size();
 cout << "# of orbit points= " << norbit << endl;
 for(int i=0;i < ndata; i++){
   ssmpoint *a = &data[i];
   int issm=a->issm;
   int pos=a->position;
   int chan=a->chanrel;
   int j=0;
   while((j<norbit)){
     //cout << i << " " << j << " " << pos << " " << orbits[j].position << " ";
     //cout << pos- orbits[j].position -cordist <<  endl;
     if(
        (issm == orbits[j].issm) &&
        ((pos - orbits[j].position - cordist) > 0) &&
        ((pos - orbits[j].position - cordist) < delta))
     {
     //cout << delta << " " << ((pos - orbits[j].position - cordist) < delta) << endl;
     //cout << i << " chan=" << chan << " " << pos- orbits[j].position - cordist << endl;
     orbit[chan][pos- orbits[j].position - cordist]++;
     }
    j++;
   }
 }
 return 0;
}
//-----------------------------------------------------------
int extractData::correlateAllSSM(int cordist,int delta){
 for (int i=0;i<ncorrelations;i++){
   correlations[i]=new int[2*delta+1];
   for(int j=0;j<2*delta+1;j++)correlations[i][j]=0;
 }
 delta_d=delta;
 cordist_d=cordist;
 int ndata=data.size();
 cout << "# of data points= " << ndata << endl;
 for(int i=0;i < ndata; i++){
   ssmpoint *a = &data[i];
   int issm=a->issm;
   int pos=a->position;
   int chan=a->chanrel;
   int j=i;
   while((j<ndata) &&
         (issm == data[j].issm) && 
         //(chan <= data[j].chanrel) &&
         (abs(pos - data[j].position - cordist) <= delta)
   ){
    fill(chan,data[j].chanrel,pos-data[j].position);
    j++;
   }
   j=i-1;
   while((j>=0) &&
         (issm == data[j].issm) && 
         //(chan <= data[j].chanrel) &&
         (abs(pos - data[j].position - cordist) <= delta)
   ){
    fill(chan,data[j].chanrel,pos-data[j].position);
    j--;
   }
 }
 return 0;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
void extractData::printFileList(){
 for(int i=0;i<nlines_d;i++) cout << filelist[i] << endl;
 //for(
 //    vector<string>::iterator i=ssmdumps.begin();
 //    i != ssmdumps.end();
 //    ++i) cout << ssmdumps[j] << endl;
}
//------------------------------------------------------------------
void extractData::printData(){
 for(
     vector<ssmpoint>::iterator i=data.begin();i !=
     data.end();++i){
     i->Print();
     }
}
//---------------------------------------------------------------------
void extractData::printDistance()
{
 for(int i=0;i<ninputs;i++){
  string name(inputNames[i]);
  cout << name << endl;
  addHist2(name,ddelta_d,dcordist_d,dcordist_d+ddelta_d);
  for(int j=0;j<ddelta_d;j++){
     fillHist2(i,j,orbit[i][j]);
     cout << " " << orbit[i][j];
  }
  cout << endl;
 }
}
//-------------------------------------------------------------------
void extractData::printCorrelations(){
 for(int i=0;i<ninputs;i++){
  for(int j=i;j<ninputs;j++){
    std::stringstream ss;
    ss << i << "x" << j;
    string name("");
    name=inputNames[i]+"x"+inputNames[j];
    addHist(name,delta_d,cordist_d);
    int k=(2*ninputs-i-1)*i/2+j;
    cout << name << ":";
    cout << i << "x" << j << ": ";
    int peak=0,offpeak=0;
    for(int l=0;l<2*delta_d+1;l++){
     //cout << correlations[k][l] << " ";
     fillHist(k,l+1,correlations[k][l]);
     if(l==delta_d)peak = correlations[k][l];
     else offpeak=offpeak+correlations[k][l];
    }
    cout << peak << " off: "<< offpeak << endl;
  }
 }
}
//-------------------------------------------------------------------
int main(){
 //string file("/data/SMAQ/Sept10_01/");
 //extractData a("oct25_9.txt","/data/SMAQ/2009_10_25-26_BT/oct25_9");
 extractData a;
 a.SetSkipNext(12);  // 0 for autocorrelations
 a.SetNfiles(1);
 if(a.readFileList()) return 1;
 return 0;
 a.chooseInputs();
 //a.checkAllSSM();
 //a.checkAllSSM1by1();
 //return 1;
 //
 a.removeEmptySSMs();
 a.extractAllSSM();
 //a.printData();
 a.correlateAllSSM(0,20);
 a.distance2Orbit(416, 20);//check this
 a.printCorrelations();
 a.printDistance();
 a.printAllHists();
 a.writeHists();
}
