#include "TrigConf.h"
#include <iostream>
#include <stdlib.h>
#include "ctpcounters.h"

#define NWIDE 133
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
/////////////////////////////////////////////////////////////////////
InteractionwCount::InteractionwCount()
{
 Int1.SetName("INT1");
 Int1.SetIXs(CSTART_L0+92,CSTART_L0+13);
 Int2.SetName("INT2");
 Int2.SetIXs(CSTART_L0+93,CSTART_L0+13);
}
void InteractionwCount::Update(w32* buffer)
{
 Int1.Update(buffer);
 Int2.Update(buffer);
}
void InteractionwCount::DisplayInt1(char* text)
{
  sprintf(text,"INT1 -- %10u %20llu %12.3f %12.3f",Int1.GetCount(),Int1.GetCountTot(),Int1.GetRate(),Int1.GetRateA());
}
void InteractionwCount::DisplayInt2(char* text)
{
  sprintf(text,"INT2 -- %10u %20llu %12.3f %12.3f",Int2.GetCount(),Int2.GetCountTot(),Int2.GetRate(),Int2.GetRateA());
}
////////////////////////////////////////////////////////////////////
TrigTimeCounters::TrigTimeCounters()
:
TimeSec(),
TimeUsec(),
Orbit(),
PeriodCounter(0)
{
 // time indexes taken from L0, not used, can be used for consistency check
 TimeSec.SetName("TimeSec");
 TimeSec.SetIXs(CSTART_SPEC,CSTART_L0+13);
 TimeUsec.SetName("TimeUsec");
 TimeUsec.SetIXs(CSTART_SPEC+1,CSTART_L0+13);
 Orbit.SetName("Orbit");
 Orbit.SetIXs(CSTART_SPEC+2,CSTART_L2+5);
}
void TrigTimeCounters::Update(w32* buffer)
{
 TimeSec.Update(buffer);
 TimeUsec.Update(buffer);
 //bool first=Orbit.GetFirst(); should be ok since before=0
 Orbit.Update(buffer);
 bool orbit = Orbit.GetBefore() <= Orbit.GetNow();
 bool secsl = (TimeSec.GetBefore() < TimeSec.GetNow());
 bool secse = (TimeSec.GetBefore() ==  TimeSec.GetNow());
 bool usecs = (TimeUsec.GetBefore() <= TimeUsec.GetNow());
 if(secsl || (secse && usecs)){ 
    if(orbit) return; // everything ok
    else{             // period counter overflow
      PeriodCounter++;
      cout <<  "TrigTimeCounters::Update: Period counter updated: " << PeriodCounter << " " << TimeSec.GetNow() << " " << TimeUsec.GetNow() << endl;
    }
 }else{              // unexpected orderr
   cout << "TrigTimeCounters::Update: unexpected order of readings ";
   cout << TimeSec.GetBefore() << " " << TimeUsec.GetBefore() << " ";
   cout << TimeSec.GetNow() << " " << TimeUsec.GetNow() << " ";
   cout <<  endl;
   if(!orbit) return;
   else{
      PeriodCounter++;
      cout <<  "TrigTimeCounters::Update: Period counter updated: " << PeriodCounter << " " << TimeSec.GetNow() << " " << TimeUsec.GetNow() << endl;
   }
 }
}
////////////////////////////////////////////////////////////////////
TriggerInput::TriggerInput(string &name,int level,int position,string &detname)
{
 fname=name;
 flevel=level;
 fposition=position;
 fdetname=detname;
}
void TriggerInput::Print()
{
 cout << fname << " L" <<flevel << " CTP: "<< fposition << " "   << fdetname << endl;  
}
//---------------------------------------------------------------------------
TriggerInputwCount::TriggerInputwCount(string &name,int level,int position,string &detname)
:TriggerInput(name,level,position,detname)
{
 cnt.SetName(name);
 if(detname.find("SPD") != string::npos){
  cnt.SetFactor(4);
  cout << "TriggerInputwCount: factor set to 4 for input " << name << endl;
 }
 cnt.SetName(name);
 if(GetLevel()==0)cnt.SetIXs(CSTART_L0+65+GetPosition(),CSTART_L0+13);
 else if(GetLevel()==1)cnt.SetIXs(CSTART_L1+5+GetPosition(),CSTART_L1+5);
 else if(GetLevel()==2) cnt.SetIXs(CSTART_L2+5+GetPosition(),CSTART_L2+5);
 else{
   cout << "TrigInput level:" << GetLevel() << endl; 
   Print();
 }
}
void TriggerInputwCount::Update(w32* buffer)
{
 cnt.Update(buffer);
}
void TriggerInputwCount::Display(ofstream *file)
{
  char text[256];
  sprintf(text,"%s %2u %10u %20llu %12.3f %12.3f \n",GetName().c_str(),GetPosition(),cnt.GetCount(),cnt.GetCountTot(),cnt.GetRate(),  cnt.GetRateA());
  *file << text;
}
void TriggerInputwCount::Display(char* text)
{
  sprintf(text,"%s %2u %10u %20llu %12.3f %12.3f",GetName().c_str(),GetPosition(),cnt.GetCount(),cnt.GetCountTot(),cnt.GetRate(),  cnt.GetRateA());
}
////////////////////////////////////////////////////////////////////
Detector::Detector(string& name)
:
fname(name),
DAQdet(0),fo(0),focon(0),busyinp(0)
{
}
Detector::Detector(string& name,int DAQdet,int fo,int focon,int busyinp)
:
fname(name),
DAQdet(DAQdet),fo(fo),focon(focon),busyinp(busyinp)
{
}
Detector::Detector(const Detector &det)
:
fname(det.fname),
DAQdet(det.DAQdet),fo(det.fo),focon(det.focon),busyinp(det.busyinp)
{
}
Detector &Detector::operator =(const Detector& det)
{
 if(this==&det) return *this;
 ((Detector *)this)->operator=(det);
 fname = det.fname;
 DAQdet=det.DAQdet;
 fo=det.fo;
 focon=det.focon;
 busyinp=det.busyinp;
 return *this;
}
void Detector::Print()
{
 cout << fname << "=" << DAQdet << " " << fo << " " <<focon << " " << busyinp << endl;
}
//------------------------------------------------------------------
DetectorwCount::DetectorwCount(const Detector& det)
:
Detector(det)
{
 l2a.SetName("FOL2a");
 l2a.SetIXs(CSTART_FO+NCOUNTERS_FO*(fo-1)+44+(focon-1),CSTART_FO+ NCOUNTERS_FO*(fo-1));
 pp.SetName("FOPP"); 
 pp.SetIXs(CSTART_FO+NCOUNTERS_FO*(fo-1)+32+(focon-1),CSTART_FO+ NCOUNTERS_FO*(fo-1));
}
void DetectorwCount::Update(w32* buffer)
{
 l2a.Update(buffer);
 pp.Update(buffer);
}
////////////////////////////////////////////////////////////////////
TriggerCluster::TriggerCluster(string &name, int hwindex)
:
fname(name),
fcname(0),
fhwindex(hwindex),
ndet(0)
{
 //for(int i=0;i<NDET;i++)fDetectors[i]=0;
 fcname = new char(name.length()+1);
 strcpy(fcname,name.c_str());
}
void TriggerCluster::AddDetector(string& name)
{
 fDetectors[ndet] = name;
 //cout << "debug:" << *fDetectors[ndet] << endl;
 ndet++;
}
void TriggerCluster::Print()
{
 cout << fname << " ndet=" << ndet << endl;
 PrintDets();
}
TriggerCluster::~TriggerCluster()
{
 for(int i=0;i<ndet;i++)fDetectors[i].erase(); 
 if(fcname) delete [] fcname;
}
void TriggerCluster::PrintDets()
{
 for(int i=0;i<ndet;i++)cout << fDetectors[i] << " ";
 //cout << endl;
}
void TriggerCluster::PrintDets(ofstream* file)
{
 for(int i=0;i<ndet;i++){
   *file << fDetectors[i] << " ";
 }
 //cout << endl;
}

//-----------------------------------------------------------------
TriggerClusterwCount::TriggerClusterwCount(string &name,int hwindex)
:TriggerCluster(name,hwindex)
{
 nclass=0;
 for(int i=0;i<NCLASS;i++)fTClasses[i]=0;
 int index=GetIndex();
 l0.SetName("L0after");
 l0.SetIXs(CSTART_L0+152+index,CSTART_L0+13);
 busy.SetName("BUSY");
 busy.SetIXs(CSTART_L0+0+index,CSTART_L0+13);
 l2.SetName("L2after");
 l2.SetIXs(CSTART_L2+127+index,CSTART_L2+5); 
}
TriggerClusterwCount::~TriggerClusterwCount()
{
 for(int i=0;i<nclass;i++) if(fTClasses[i]){ 
    delete fTClasses[i];
    fTClasses[i]=0;
 }
}
void TriggerClusterwCount::Print()
{
 TriggerCluster::Print();
 cout << "nclasses= " << nclass << endl;
 for(int i=0;i<nclass;i++)fTClasses[i]->Print();
}
void TriggerClusterwCount::Update3(w32* buffer)
{
 busy.Update(buffer);
 l2.Update(buffer); 
 for(int i=0; i<nclass;i++)fTClasses[i]->Update(buffer);
}
void TriggerClusterwCount::DisplayCluster(ofstream *file)
{
 DisplayClusterHeader(file);
 for(int i=0; i<nclass;i++)fTClasses[i]->DisplayClass(file);
 DisplayClusterTotal(file);
}
void TriggerClusterwCount::DisplayClusterSortBCM(ofstream *file)
// sort classes acoording to BC mask
{
 TriggerClasswCount* clssorted[nclass];
 bcmtype bcm;
 clssorted[0]=fTClasses[0];
 if(nclass>1){  
   bcm =fTClasses[1]->GetBCMtype();
   clssorted[1]=fTClasses[1];   
   if(bcm<fTClasses[0]->GetBCMtype()){
     clssorted[0]=fTClasses[1];  
     clssorted[1]=fTClasses[0];  
   }
   for(int i=2; i<nclass;i++){
     bcm=fTClasses[i]->GetBCMtype();
     //cout <<fTClasses[i]->GetName() << " " << i << " " <<bcm << endl;
     int j=i;
     while(j>0 && (bcm < clssorted[j-1]->GetBCMtype()))j--;
     for(int k=i;k>j;k--)clssorted[k]=clssorted[k-1];
     clssorted[j]=fTClasses[i];
   }
 }
 //cout << "SORTED:" << endl;
 //for(int i=0; i<nclass;i++)cout << clssorted[i]->GetName() << " " << fTClasses[i]->GetName() << endl;
 DisplayClusterHeader(file);
 for(int i=0; i<nclass;i++)clssorted[i]->DisplayClass(file);
 DisplayClusterTotal(file);
}
void TriggerClusterwCount::DisplayClusterHeader(ofstream* file)
{
 *file << ">>>>>>>>> CLUSTER(" << GetIndex()<<"): " << GetName() << " = ";
 PrintDets(file); 
 *file << endl;
 *file << "CLASS                         MASK      L0       L2     L0rate     L2rate      L2/L0 ";
 *file << "  L0tot    L2tot   <L0rate>   <L2rate>    <L2/L0>\n";
 string ss("-");
 for(int i=0;i<NWIDE;i++)ss += "-";
 *file << ss << endl;
}
void TriggerClusterwCount::DisplayClusterTotal(ofstream *file)
{
 double lt=0,alt=0;
 //cout << "time = "  << busy.GetTime() << endl;
 //if(busy.GetTime())lt=((double)busy.GetCount())/((double)busy.GetTime());
 //if(busy.GetTimeTot())alt=((double)busy.GetCountTot())/((double)busy.GetTimeTot());
 lt=1.-busy.GetRate()*1.e-6*0.4;
 alt=1.-busy.GetRateA()*1.e-6*0.4;
 char text[256];
 sprintf(text,"TOTAL                         :            %8u            %10.3f %10.3f         %8llu            %10.3f %10.3f \n",
         l2.GetCount(),l2.GetRate(),lt,l2.GetCountTot(),l2.GetRateA(),alt);
 *file << text;
}
//////////////////////////////////////////////////////////////////////
TriggerClass::TriggerClass(string &name,w8 index, TriggerCluster *cluster)
:
fname(name),
fIndex(index),
fCluster(cluster)
{
}
TriggerClass::~TriggerClass()
{
 //if(fCluster) delete fCluster;
}
void TriggerClass::Print()
{
 //cout << fname << " Index=" << fIndex << " " << fCluster->GetName();
 //cout << " " << fCluster <<  endl;
 printf("%s Index= %i %s %p \n",fname.c_str(),fIndex,fCluster->GetName().c_str(),fCluster);
}
void TriggerClass::ParseClassName()
{
 vector<string> items;
 splitstring(fname,items,"-");
 // Descriptor part of name
 fnamedesc=items[0];
 // BC mask type
 char bcm1stchar=items[1][0];
 if(bcm1stchar == 'B') BCMtype=B; 
 else if(bcm1stchar == 'A') BCMtype=A;
 else if(bcm1stchar == 'C') BCMtype=C;
 else if(bcm1stchar == 'E') BCMtype=E;
 else{
    cout << "Unknown BCM mask in class: " << fname << endl;
    BCMtype=U;
 }
 //cout << "TriggerClass BCMtype= " << BCMtype << endl;
}
//--------------------------------------------------------------------------
TriggerClasswCount::TriggerClasswCount(string &name,w8 index, TriggerCluster *cluster)
:TriggerClass(name,index,cluster),
fGroup(0),fTime(0)
{
 cnts[0].SetName("L0before");
 cnts[0].SetIXs(CSTART_L0+15+GetIndex(),CSTART_L0+13);
 cnts[1].SetName("L0after");
 cnts[1].SetIXs(CSTART_L0+99+GetIndex(),CSTART_L1+13);
 cnts[2].SetName("L1before");
 cnts[2].SetIXs(CSTART_L1+39+GetIndex(),CSTART_L1+ 5);
 cnts[3].SetName("L1after");
 cnts[3].SetIXs(CSTART_L1+89+GetIndex(),CSTART_L1+ 5);
 cnts[4].SetName("L2before");
 cnts[4].SetIXs(CSTART_L2+25+GetIndex(),CSTART_L2+ 5);
 cnts[5].SetName("L2after");
 cnts[5].SetIXs(CSTART_L2+75+GetIndex(),CSTART_L2+ 5);
 if(TriggerClass::GetName().at(2)=='S'){
  cout << "TriggerClasswCount: Factor NOT set to 4 for L0B for class " << GetName() << endl;
  //cout << "TriggerClasswCount: Factor set to 4 for L0B for class " << GetName() << endl;
  //cnts[0].SetFactor(4);
 }
 CreateShortName();
}
TriggerClasswCount::TriggerClasswCount(string &name,w8 index, TriggerCluster *cluster,w32 groupname,w32 grouptime)
:TriggerClass(name,index,cluster),
fGroup(groupname),fTime(grouptime),isActive(0)
{
 cnts[0].SetName("L0before");
 cnts[0].SetIXs(CSTART_L0+15+GetIndex(),CSTART_L0+13);
 cnts[1].SetName("L0after");
 cnts[1].SetIXs(CSTART_L0+99+GetIndex(),CSTART_L1+13);
 cnts[2].SetName("L1before");
 cnts[2].SetIXs(CSTART_L1+39+GetIndex(),CSTART_L1+ 5);
 cnts[3].SetName("L1after");
 cnts[3].SetIXs(CSTART_L1+89+GetIndex(),CSTART_L1+ 5);
 cnts[4].SetName("L2before");
 cnts[4].SetIXs(CSTART_L2+25+GetIndex(),CSTART_L2+ 5);
 cnts[5].SetName("L2after");
 cnts[5].SetIXs(CSTART_L2+75+GetIndex(),CSTART_L2+ 5);
 if(TriggerClass::GetName().at(2)=='S'){
  cout << "TriggerClasswCount: Factor NOT set to 4 for L0B for class " << GetName() << endl;
  //cout << "TriggerClasswCount: Factor set to 4 for L0B for class " << GetName() << endl;
  //cnts[0].SetFactor(4);
 }
 // Set GroupName and GroupTime
 for(int i=0;i<6;i++){
  cnts[i].SetGroupName(groupname);
  cnts[i].SetGroupTime(grouptime);
 }
 // Creating short name;
 CreateShortName();
}
void TriggerClasswCount::CreateShortName()
{
 int n = TriggerClass::GetName().length();
 int ic=0;
 for(int i=0;i<n;i++){
    if(TriggerClass::GetName().at(i)=='-')ic++;
    if(i > (NSHORT-1)) break;
    if(ic==2) break;
    fnameS[i]=TriggerClass::GetName().at(i);
    fnameS[i+1]='\0';
 }
}
void TriggerClasswCount::Update(w32* buffer)
{
 for(int i=0;i<6;i++)cnts[i].Update(buffer);
 isActive=(buffer[CSTART_TSGROUP]==fGroup);
}
void TriggerClasswCount::DisplayClass(ofstream* file)
{
  double livetime = ((double)cnts[5].GetCount())/((double)cnts[0].GetCount()); 
  double alivetime = ((double)cnts[5].GetCountTot())/((double)cnts[0].GetCountTot()); 
  //string display(32,' ');
  string display(GetName().substr(0,30));
  char TimeGroup[8]; 
  sprintf(TimeGroup," %i/%i",GetGroup(),GetTime());
  display.append(TimeGroup);
  for(int i=display.size();i<30;i++)display += ' ';
  char text[256];
  sprintf(text,"%5s:%3i %7i %8i %10.3f %10.3f %10.3f ",display.c_str(),GetIndex(),cnts[0].GetCount(),cnts[5].GetCount(),cnts[0].GetRate(),cnts[5].GetRate(),livetime);
  sprintf(text,"%s%7lli %8lli %10.3f %10.3f %10.3f\n",text,cnts[0].GetCountTot(),cnts[5].GetCountTot(),cnts[0].GetRateA(),cnts[5].GetRateA(),alivetime);
  *file << text;
}
void TriggerClasswCount::Print()
{
 TriggerClass::Print();
 cout << " Group=" << fGroup << " Time=" << fTime  << endl;
}
///////////////////////////////////////////////////////////////////////////////////////
Detector* VALIDLTUS::dets[NDET];
int VALIDLTUS::count=0;
#include <sstream>
VALIDLTUS::VALIDLTUS()
{
 if(!(VALIDLTUS::count))for(int i=0;i<NDET;i++)VALIDLTUS::dets[i]=0;
 VALIDLTUS::count++;
}
void VALIDLTUS::AddDetector(Detector &det)
{
 //cout << "ndet= "  << ndet << endl;
 for(int i=0;i<ndet;i++){
  if((dets[i]->GetName()).find((det.GetName())) != string::npos){
   cout << "Detector: " << det.GetName() << " already in VALIDLTUS" << endl;
   return ;
  }
 }
 VALIDLTUS::dets[ndet] = new Detector(det);
}
int VALIDLTUS::readVALIDLTUS()
{ 
 string frcfgfile;
 stringstream ss;
 char* path;
 path = getenv("VMECFDIR");
 if(path==0){
  cout << "VALIDLTUS: VMECFDIR not defined, looking for pwd/VALID.LTUS \n";
  frcfgfile = "VALID.LTUS";  
 }else{
   ss << "/CFG/ctp/DB/VALID.LTUS";
   frcfgfile = path + ss.str();
 }
 file.open(frcfgfile.c_str());
 if(!file){
  cout << "readVALIDLTUS: cannot open file: "+frcfgfile << endl;
  return 1;
 }else{
  cout << "readVALIDLTUS: File: "+frcfgfile+" opened." << endl;
 } 
 string line;
 ndet=0;
 while(getline(file,line)){
   if(ProcessLine(line)) return 1;
 }
 return 0;
}
int VALIDLTUS::ProcessLine(const string& line)
{
 if(line.size()==0) return 0;
 int ix=0;
 while(ix<(int)line.size() && line.at(ix)==' ')ix++;
 if(line.at(ix)=='#') return 0;
 vector<string> items;
 splitstring(line,items," =");
 int nitems = items.size();
 //cout << nitems << endl;
 if(nitems != NITEMS){
  cout << "# of items != " << NITEMS << " nitems=" << nitems << endl;
  return 1;
 }
 Detector d(items[0],atoi(items[1].c_str()),atoi(items[2].c_str()),atoi(items[3].c_str()),atoi(items[4].c_str()));
 AddDetector(d);
 ndet++;
 return 0;
}
Detector* VALIDLTUS::GetDetector(const int DAQdet){
 for(int i=0;i<NDET;i++){
  if(dets[i]){
    if(dets[i]->GetDAQdet() == DAQdet) return dets[i];
  }
 }
 cout << "Detector " << DAQdet << " not found !!!" << endl;
 return 0;
}
Detector* VALIDLTUS::GetDetector(const string& name){
 for(int i=0;i<NDET;i++){
  if(dets[i]){
    //if((dets[i]->GetName().find(name) != string::npos))return dets[i];
    if(strcasecmp(dets[i]->GetName().c_str(),name.c_str())==0) return dets[i]; 
  }
 }
 cout << "Detector " << name << " not found !!!" << endl;
 return 0;
}
void VALIDLTUS::Print()
{
 for(int i=0;i<NDET;i++)if(dets[i]){
   cout << i << " " ; dets[i]->Print();
 }
}



