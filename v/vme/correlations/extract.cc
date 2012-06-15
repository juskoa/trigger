#include "extract.h"
#include <fstream>
#include <sstream>
#include "TH1F.h"
#include "TH2F.h"
#include "TObjArray.h"
#include "TCanvas.h"
#define Mega 1024*1024
typedef unsigned int w32;

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
//-------------------------------------------------------
Hists::Hists(){
}
Hists::~Hists(){
}
int Hists::addHist(string const &name,int delta,int cordist){
 int nbins=2*delta+1;
 float x0=cordist-delta-0.5;
 float xm=cordist+delta+0.5;
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
//-------------------------------------------------------------
void ssmpoint::Print(){
  cout << "ssm="<< issm << " ch=" << chanabs<<" ch2inp=" << chanrel << " pos=" << position << endl;
}
//-------------------------------------------------------
Input::Input(string &name,int delay,int index)
:
name(name),delay(delay),index(index),used(0)
{}
//---------------------------------------------------
extractData::extractData(char* config):
Nchans(60)
{
 //FileList_d=FileList;
 //DataDir_d=DataDir;
 nfiles=0;
 if(ReadConfig(config)) exit(1);
 cordist_d=0;delta_d=0;
 chosenInputs = new int[Nchans];
 chan2inp = new int[Nchans];
 chans = new int[Nchans];
 chansd = new int[Nchans];
 nssms=0;
}
//------------------------------------------------------
int extractData::ReadConfig(char* config)
{
 ifstream in;
 //string config("config.txt");
 char file[256];
 sprintf(file,"%s.txt",config);
 in.open(file);
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
      }else if(nlines==1){
       DataDir_d=line;
       nlines++;
      } else if(nlines==2){
       FileList_d=line;
       nlines++;
      }else if(nlines==3){
       vector<string> items;
       splitstring(line,items," ");
       unsigned int nit=items.size();
       for(unsigned int i=0;i<nit;i++){
          unsigned int j=0;
          while(j < inputs.size() && (inputs.at(j)).name.find(items[i]) == string::npos)j++;
          if(j==inputs.size()){
           cout << "Input " << items[i] << " not found in VALID.CTPINPUTS" << endl;
           return 1;
          }
          cout << "Choosing input: " << items[i] << endl;
          inputs.at(j).used=1;
       }
       nlines++;
      }else if(nlines==4){
        SetSkipNext(atoi(line.c_str()));  // 0 for autocorrelations
        nlines++;
      }else if(nlines==5){
        SetNfiles(atoi(line.c_str()));
        cout << "# of files: " << nfiles << endl;
        nlines++;
      }else if(nlines==6){
         vector<string> items;
         splitstring(line,items," ");
         if(items.size() != 2){
          cout << "confif.txt: wrong syntax in line:" << endl;
          cout << line << endl;
          return 1;
         }
         dcordist_d=atoi(items[0].c_str());
         ddelta_d=atoi(items[1].c_str());
         cout << "config.txt: FROM SIZE " << dcordist_d << " " << ddelta_d << endl;
         nlines++;
      }else{
       cout << "config.txt: Unexpected number of lines." << endl;
       return 1;
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
      if(items.size() < 11){
       cout << "Wrong syntax in " << VALIDCTPINPUTS_d << endl;
       cout << line << endl;
       return 1;
      }
      int index=atoi(items[4].c_str());
      vector<Input>::iterator i= inputs.begin();
      while(i != inputs.end() && (i->index<index))i++;
      inputs.insert(i,Input(items[0],atoi(items[8].c_str()),index));
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
 for(unsigned int i=0;i<inputs.size();i++){
  if(inputs.at(i).used){
   chosenInputs[i]=inputs.at(i).index;
   inputNames.push_back(inputs.at(i).name);
   cout << "Adding input: " << inputs.at(i).name << " " << inputs.at(i).index << " i=" << i << endl;
  }
 }
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
    cout << "extract1SSMfilt1: File not found: "<<  file  << endl;
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
  cout << "readFileList: nfiles= " << nfiles << endl;
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
int extractData::removeEmptySSMs(bool remove){
 int ret;
 nssms=0;
 if(!remove){
  for(int i=0;i<nlines_d;i++){
    ssmdumps.push_back(filelist[i]); 
    nssms++; 
  }
  return 0;
 }
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
//int extractData::distance2Orbit(int cordist,int delta){
int extractData::distance2Orbit(){
 //dcordist_d=cordist;
 //ddelta_d=delta;
 for (int i=0;i<ninputs;i++){
   orbit[i]=new int[ddelta_d];
   for(int j=0;j<ddelta_d;j++)orbit[i][j]=0;
 }
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
        ((pos - orbits[j].position - dcordist_d) > 0) &&
        ((pos - orbits[j].position - dcordist_d) < ddelta_d))
     {
     //cout << delta << " " << ((pos - orbits[j].position - cordist) < delta) << endl;
     //cout << i << " chan=" << chan << " " << pos- orbits[j].position - cordist << endl;
     orbit[chan][pos- orbits[j].position - dcordist_d]++;
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
     //cout << " " << orbit[i][j];
  }
  //cout << endl;
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

