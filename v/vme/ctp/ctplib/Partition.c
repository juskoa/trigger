#include "vmewrap.h"
#include "ctp.h"
#include "Tpartition.h"
#include "TpartitionPlus.h"
#include <sstream>
#include <iostream>

w32 getLM0addr(w32 addr);    // is in ctplib

///////////////////////////////////////////////////////////////////////
// New classes for testing new fw
///////////////////////////////////////////////////////////////////////

#define NWIDE 133
//-----------------------------------------------------------------------
// Routine for evaluation of logical expressions
//
char evaluate(string& eval){
// no brackets assumed in eval
 int ll = eval.length();
 if(ll<1) return 'e';
 //       
 int i=0;
 //cout << "   evaluate start:" << eval << endl;
 while(i<ll){
  // Negation
  if(eval[i]=='~'){
   // assuming no multiple !!!!!
   if(++i<ll){
     char buf[2];
     sprintf(buf,"%d",!atoi(&eval[i]));
     eval[i]=buf[0];
     eval.erase(i-1,1);
     ll--;i--;
   }else return 'e';
  }
  i++;
  }
  //cout << "evaluate After Neg " << eval << endl;
  // AND
 i=0;
 while(i<ll){
  if(eval[i]=='&'){
   if(i>0 && ++i<ll){
     char buf[2];
     sprintf(buf,"%d",atoi(&eval[i-2])*atoi(&eval[i]));
     eval[i-2]=buf[0];
     eval.erase(i-1,2);
     ll += -2; i+= -2;
   }else return 'e';
  }
  i++;
 }
  //cout << "evaluate After AND " << eval << " ll=" << ll << endl;
 // OR
 i=0;
 while(i<ll){
  if(eval[i]=='|'){
   if(i>0 && ++i<ll){
     char buf[2];
     sprintf(buf,"%d",atoi(&eval[i-2])+atoi(&eval[i]));
     eval[i-2] = buf[0];
     eval.erase(i-1,2);
     ll += -2; i+= -2;
   } else return 'e';
  }
  i++;
 }
 //cout << "   evaluate end:" << eval << " l=" << ll << endl;
 if (ll != 1) return 'e';
 else return eval[0];
}
//----------------------------------------------------------------
int removespaces(string& descr)
{
  // remove spaces from beginning and end
 int ll=descr.length();
 int i=0;
 while(i<ll){
   if(descr[i]==' '){
    descr.erase(i,1);
    ll--;
   }else i++;
 }
 return 0;
}
//----------------------------------------------------------------
int parse(string descr,int level)
{
 //cout << "parse level:" << level << ":'"  << descr <<"'" << endl;
 if(level>50) return 'e';
 int ll=descr.length();
 //cout << "parse string length:" << ll << endl;
 if(ll==0){
  cout << "parse Error: empty string" << endl;
  return 'e';
 }
 if(!level)removespaces(descr);
 level++;
 ll=descr.length();
 //cout << "parse After removed spaces:'" << descr <<"'"<< endl;
 //cout << "parse string length:" << ll << endl;
 //
 // Unfold brackets
 // 
 int rightparpos=0,leftparpos=0;
 int rightparnum,leftparnum=0;
 char val;
 int i=0;
 string buf("");
 while(i<ll){
        //cout << " parse start ll=" << ll << " i=" << i << endl;
        //buf.clear();
	rightparnum=0;
 	while(i<ll && descr[i] != '('){  // first left bra
   		// evaluate
		if(descr[i] == ')')rightparnum++;
		buf.append(1,descr[i]);
   		i++;
 	}
	//cout <<buf << endl;
	if(rightparnum){
	 cout << "parse syntax Error: left bracket missing ?" << endl;
	 return 'e';
	}
 	if(i==ll){
  	//cout << " parse Finished level:" << level <<endl;
	return evaluate(buf);
 	}
	i++;
 	leftparpos=i;
	rightparnum=0;
 	leftparnum=1;
 	while(i<ll && leftparnum != rightparnum){ // loop until (=)	
		//cout << i << endl;
  		if(descr[i]=='(')leftparnum++;
  		if(descr[i]==')')rightparnum++;
		//cout << "leftparnum="<<leftparnum<<" rightparnum="<< rightparnum << endl;
  		i++;
 	}
	if(leftparnum != rightparnum){
	 cout << " parse Syntax error: right bracket missing ?" << endl;
	 return 'e';
	}
 	rightparpos=i;
	//cout << "leftparpos="<<leftparpos<<" rightparpos="<< rightparpos << endl;
 	val=parse(descr.substr(leftparpos,rightparpos-leftparpos-1).c_str(),level++);
	if(val == 'e') return 'e';
        //cout << " parse after ll=" << ll << " i=" << i << endl;
	buf.append(1,val);
 }
 //cout << "parse val:"<< val << endl;
 if(buf.length()){
  val=evaluate(buf);
  return val;
 }else{
  cout << "parse unknown error " << endl;
  return 'e';
 }
}
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
//-----------------------------------------------------------------
// Global function for Printing log

void getdatetime(char* dmyhms);
void PrintLog(const char* text)
{
 char time[30];
 getdatetime(time);
 cout << time << " " << text << endl;
 //printf("%s %s\n",time,text);
}
/////////////////////////////////////////////////////////////////////////
VMEaddress::VMEaddress(string name,const w32 address,const bool shared)
:name(name),shared(shared),address(address),value(0),nruns(0)
{
 for(int i=0;i<6;i++)runnumber[i]=0;
}
VMEaddress::~VMEaddress()
{}
void  VMEaddress::Write()
{
 if(nruns){
 	cout << "Writing RUNS";
 	for(w32 i=0;i<nruns;i++)cout << " " << runnumber[i]; 
 	cout << " Address " << name  << " value=" << value << endl;
 	vmew32(address,value);
 }
}
int VMEaddress::SetValue(w32 run,w32 val)
// set value and add run to the list
// return 1 if all runs taken
{
 w32 nrunsmax;
 if(shared)nrunsmax=6; else nrunsmax=1;
 if(nruns>nrunsmax){
     PrintLog("Internal error in VMEaddress. Number of runs > nrunsmax, exiting");
     cout << "nrunsmax= " << nrunsmax << " nruns= " << nruns << endl;
     exit(1);
 }
 // check if already there
 if(shared && (value==val)){
      runnumber[nruns++]=run;
      return 0;
 }
 if(!nruns){
   runnumber[nruns++]=run;
   value=val;
   return 0;
 }else return 1;
}
void VMEaddress::UnloadRun(w32 run)
{
 if(shared && nruns>1){
   for(w32 i=0;i<nruns;i++){
     if(run==runnumber[i]){
       nruns--; 
       for(w32 j=i;j<nruns;j++)runnumber[j]=runnumber[j+1];
       return;
     }
   }
 }
 else{
   if(run==runnumber[0]){
     SetValue(0);
     runnumber[0]=0;
     nruns=0;
   }
 }
}
void VMEaddress::Print()
{
 printf("===> %s adrress=0x%x value=0x%x \n",name.c_str(),address,value);
 cout << "     nruns= " << nruns;
 for(int i=0;i<6;i++) cout << " " << runnumber[i];
 cout << endl;
}
VMEaddressL0fun::VMEaddressL0fun(string name,const w32 address):
VMEaddress(name,address,1),
lut1(0),lut2(0),lut3(0),lut4(0),
nruns2(0)
{
 for(int i=0;i<6;i++)runnumber2[i]=0;
}
int VMEaddressL0fun::SetData(w32 run,bool* lut1,bool* lut2,w32& pos)
// value = how many L)funs are defined (0,1,2)
{
 if(GetValue()==0){
   SetValue(run,1);
   this->lut1=lut1;
   this->lut2=lut2;
   pos=3;
 }else if(GetValue()==1){
   int i=0,j=0;
   while((i<NSIZE) && lut1[i] == this->lut1[i])i++;
   if(i==NSIZE){
     i=0;
     while((i<NSIZE) && lut2[i] == this->lut2[i])i++;
     if(i==NSIZE){
       SetRunnumber(run);
       pos=3;
       return 0;
     }     
   }
   while((j<NSIZE) && lut2[j] == this->lut1[j])j++;
   if(j==NSIZE){
     j=0;
     while((j<NSIZE) && lut1[j] == this->lut2[j])j++;
     if(j==NSIZE){
       SetRunnumber(run);
       pos=3;
       return 0;
     }
   }
   // l0f not equal existing one
   this->lut3=lut1;
   this->lut4=lut2;
   runnumber2[nruns2++]=run;
   pos=4;
   return 0;
 }else if(GetValue()==2){
   // check all 4 options
   return 1;
 }else{
   cout << "TriggerL0fun:Internal error." << endl;
   return 1;
 }
 return 0;
}
void VMEaddressL0fun::Write()
{
 if(GetValue()){
 	cout << "Writing RUNS L0FUN34 " << endl;;
 	w32* run1;
 	run1=GetRunnumbers();
        if(Getnruns()) cout << " L0Fun3: ";
 	for(w32 i=0;i<Getnruns();i++)cout << " " << run1[i]; 
 	if(nruns2)cout << endl << "Runs L0Fun4 ";
 	for(w32 i=0;i<nruns2;i++)cout << " " << runnumber2[i]; 
 	cout << " Address " << GetName()  << endl;
 	for(int i=0;i<NSIZE;i++){
    	w32 expr=0;
    	if(lut1 && lut3)expr=(lut1[i]+2*lut2[i]+4*lut3[i]+8*lut4[i])<<12;
    	else if(lut1 && !lut3) expr=(lut1[i]+2*lut2[i])<<12; 
    	else{
        	cout << "VMEadrressL0fun::Write() internal error" << endl;
		exit(4);
    	}
    	vmew32(GetAddress(),i+expr); 
 	}
 }
}
////////////////////////////////////////////////////////////////////////
// list of loaded runs
w32* CTPHardware::ClustersInRuns=0;
// Shared resources
VMEaddress CTPHardware::L0FUNCTION_1("L0FUNCTION_1",0x9000+4*0x137,1);
VMEaddress CTPHardware::L0FUNCTION_2("L0FUNCTION_2",0x9000+4*0x138,1);

getLM0addr -probably not working (it uses global var. ctpboards[]...)
VMEaddress CTPHardware::RND1("RANDOM_1",getLM0addr(0x9000+4*0x139),1);
VMEaddress CTPHardware::RND2("RANDOM_2",getLM0addr(0x9000+4*0x13a),1);
VMEaddress CTPHardware::BC1("SCALED_1",getLM0addr(0x9000+4*0x13b),1);
VMEaddress CTPHardware::BC2("SCALED_2",getLM0addr(0x9000+4*0x13c),1);

LM0: 0x90 while old L0: 0x1fb
VMEaddressL0fun CTPHardware::L0FUNCTION_34("L0FUNCTION_3",0x9000+4*0x1fb);
// Classes
VMEaddress* CTPHardware::L0CONDITION[NCLASS];
VMEaddress* CTPHardware::L0VETO[NCLASS];
VMEaddress* CTPHardware::L0MASK[NCLASS];
VMEaddress* CTPHardware::L0INVERT[NCLASS];
// All adresses
vector<VMEaddress*> CTPHardware::Addresses;
CTPHardware::CTPHardware()
{
 ClustersInRuns = new w32[NCLUST];
 for(int i=0;i<NCLUST;i++)ClustersInRuns[i]=0;
 Addresses.push_back(&L0FUNCTION_1);
 Addresses.push_back(&L0FUNCTION_2);
 Addresses.push_back(&RND1);
 Addresses.push_back(&RND2);
 Addresses.push_back(&BC1);
 Addresses.push_back(&BC2);
 Addresses.push_back(&L0FUNCTION_34);
 for(w32 i=0;i<NCLASS;i++){
   L0CONDITION[i] = new VMEaddress("L0_CONDITION",0x9000+4*(0x101+i),0);
   if(l0C0()) {
     L0VETO[i]      = new VMEaddress("L0_VETOr2",     0x9000+4*(0x201+i),0);
     L0MASK[i]      = new VMEaddress("L0_MASK",     0x9000+4*(0x1c1+i),0);
   } else {
     L0VETO[i]      = new VMEaddress("L0_VETO",     0x9000+4*(0x181+i),0);
   };
   L0INVERT[i]    = new VMEaddress("L0_INVERT",   0x9000+4*(0x16d+i),0);
   Addresses.push_back(L0CONDITION[i]);  
   Addresses.push_back(L0VETO[i]);  
   Addresses.push_back(L0INVERT[i]);  
   Addresses.push_back(L0MASK[i]);  
 }
 cout << "CTPHardware created" << endl;
}
CTPHardware::~CTPHardware()
{
 // delete everything tbd
 cout << "CTPHardware destroyed" << endl;
}
int CTPHardware::SetClass(w32 valueL0COND,w32 valueL0VETO,w32 valueL0INV)
{
 return 0;
}
int CTPHardware::SetValueL0fun4(w32 run,w32 value,w32& pos)
// Try to set value for 4 input l0fun.
// If not available return 1
{
 if(!L0FUNCTION_1.SetValue(run,value)){
   pos=1;
   return 0;
 }
 else if(!L0FUNCTION_2.SetValue(run,value)){
  pos=2;
  return 0;
 }
 else return 1;
}
int CTPHardware::SetValueL0fun12(w32 run,bool* lut1,bool* lut2,w32& pos)
// set lut1,lut2 for 12 input l0fun
{
 return L0FUNCTION_34.SetData(run,lut1,lut2,pos);
}
int CTPHardware::SetCTPCondition(w32 run,string& name,w32 value,w32& pos)
{
 if(name.compare("BC1")==0 || name.compare("BC2")){
    if(BC1.SetValue(run,value)){
      pos=1;
      return 0;
    }
    else if(BC2.SetValue(run,value)){
      pos=2;
      return 0;
    }
    else return 1;
 }
 if(name.compare("RND1")==0 || name.compare("RND2")){
    if(RND1.SetValue(run,value)){
      pos=3;
      return 0;
    }
    else if(RND2.SetValue(run,value)){
       pos=4;
       return 0;
    }
    else return 1;
 }
 cout << "CTPHardware::SetCTPCondition(): unknown condition:"<<name<<endl;
 return 1;
}
bool CTPHardware::IsRunLoaded(w32 run)
{
 for(w32 i=0;i<NCLUST;i++)if(ClustersInRuns[i]==run)return 1;
 return 0;
}
w32 CTPHardware::AddCluster(w32 run)
// ret=0 - all clusters taken
{
 w32 i=0;
 while(i<NCLUST && ClustersInRuns[i])i++;
 if(i==NCLUST) return 0; 
 ClustersInRuns[i]=run;
 return (i+1);
}
void CTPHardware::Load(w32 run)
{ 
 int n = Addresses.size();
 // need to check for all runs or write everything
 for(int i=0;i<n;i++){
  //if(Addresses[i]->GetRunnumber()==run) (Addresses[i]->Write)();
  Addresses[i]->Write();
 }
}
void CTPHardware::Unload(w32 run)
{
 int n = Addresses.size();
 //cout << "naddresses= " << n << endl;
 for(int i=0;i<n;i++)Addresses[i]->UnloadRun(run);
}
void CTPHardware::Print()
{
 int n = Addresses.size();
 for(int i=0;i<n;i++)Addresses[i]->Print();
}
////////////////////////////////////////////////////////////////////
TriggerInput::TriggerInput(string &name,int level,int position,int signature,string &detname)
:fname(name),
 flevel(level),
 fposition(position),
 fsignature(signature),
 fpos(0),
 fdetname(detname)
{
}
void TriggerInput::Print()
{
 cout << fname << " L" <<flevel << " CTP: "<< fposition << " signature: "<< fsignature << " " << fdetname << endl;  
}
int TriggerInput::Load2SW(w32 run)
{
 return CTPHardware::SetCTPCondition(run,fname,fsignature,fpos);
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
Detector::~Detector()
{}
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
////////////////////////////////////////////////////////////////////
TriggerCluster::TriggerCluster(string &name, int hwindex)
:
fname(name),
fcname(0),
fhwindex(hwindex),
nclass(0),ndet(0)
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
 cout << endl;
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
int TriggerCluster::Load2SW(w32 run)
// Calculates L0CONDITION,L0VETO,L0MASK,L0INVERT
{
 w32 clst = CTPHardware::AddCluster(run);
 for(int i=0;i<nclass;i++){
    // L0 condition
    w32 valueL0COND=0;
    TriggerClass* cls=fTClasses[i];
    // Inputs
    TriggerDescriptor* td=cls->GetDescriptor();
    int ninps=td->Getninps();
    for(int j=0;j<ninps;j++){
	TriggerInput* inp=td->GetTInputs()[i];     
        if(inp->GetDetName().compare("CTP")){
          // input
	  valueL0COND |= 1<<inp->GetPosition(); 
        }else{
          // bc1,bc2,rnd1,rnd2
          valueL0COND |= (1<<(inp->GetPos()-1))<<29; 
	} 
    }
    // l0f
    int nl0f=td->Getnl0f();
    for(int j=0;j<nl0f;j++){
       TriggerL0fun* l0f=td->GetTL0fun()[j];
       valueL0COND |= (1<<(l0f->GetPos()-1))<<24;
    }
    // L0 VETO
    w32 valueL0VETO=clst; // cluster
    // pf,bcmask,all/rare
    // L0_MASK
    // L0_INVERT
    w32 valueL0INV=0;
    //
    CTPHardware::SetClass(valueL0COND,valueL0VETO,valueL0INV);
 }
 return 0;
}
//////////////////////////////////////////////////////////////////////////
TriggerL0fun::TriggerL0fun(const string& name)
:
fname(name),logic1(""),logic2(""),
finps(4),
ninps(0),
fpos(0),
lut1(0),lut2(0)
{
}
TriggerL0fun::~TriggerL0fun()
{
 if(lut1) delete [] lut1;
 if(lut2) delete [] lut2;
}
void TriggerL0fun::Print()
{
 cout << fname;
 if(finps==4)cout << " " << logic1  << " " << finps << " " << ninps;
 else 
 cout << " " << logic1  << "|" << logic2 << " "<< finps << " " << ninps;
 for(int i =0;i<ninps;i++)cout << " " << inputs[i];
 cout << endl;
/*
 w32 pr=0;
 cout << "0x";
 for(int i=0; i<(1<<finps) ;i++){
    pr += (1<<i)*lut1[i];
    if((i+1)%32 == 0){
      printf("%x",pr);
    }
 }
 printf("%x \n",pr);
*/
}
void TriggerL0fun::replace(string& logic,string inp,int bit)
// Replaces input "inp" in lofun "logic" with bit
{
 int ll=inp.length();
 char bchar[1];
 sprintf(bchar,"%i",bit);
 string bstr(bchar);
 w32 pos;
 while((pos=logic.find(inp)) != string::npos){
   logic.replace(pos,ll,bstr);
   //cout << "in " << logic << endl;
 }
}
int TriggerL0fun::calculateLUT(bool *lut,string& logicin)
// loops over all inputs combination and calculates lut
{
 for(int i=0;i<(1<<finps);i++){  // address in lut
    string logic(logicin);
    for(int j=0;j<ninps;j++){
       replace(logic,inputs[j],(i&(1<<(index[j]-1)))==(1<<(index[j]-1)));
    }
    //cout << i << " " <<logic << endl;
    char val=parse(logic,0);
    //printf("%s parse= %c \n",fname.c_str(),val);
    if(val=='0')lut[i]=0;
    else if(val=='1')lut[i]=1;
    else {
       cout << "TriggerL0fun::calculateLUT error" << endl;
       return 1;
    }
 }
 return 0; 
}
int TriggerL0fun::Load2SW(w32 run)
// Loads l0fun data to CTPHardware
{
 if(finps==4){
   w32 pr=0;
   for(int i=0;i<(1<<4);i++)pr += (1<<i)*lut1[i];
   if (CTPHardware::SetValueL0fun4(run,pr,fpos))return CTPHardware::SetValueL0fun12(run,lut1,lut2,fpos);
   else return 0;
 }else{
   // new l0 fun
   return CTPHardware::SetValueL0fun12(run,lut1,lut2,fpos);
 }
 return 1; 
}
int TriggerL0fun::splitl0fun34()
// Assming new (3 and 4) lof in format (logic1)|(logic2)
// Extract logic1 and logic2
{
 removespaces(logic1);
 int ll=logic1.length();
 if(logic1[0] != '('){
     PrintLog("Wrong syntax of l0fun, does not start with (");
     exit(3);
 }
 int i=1,lp=1,rp=0;
 while(i<ll && lp != rp){
     if(logic1[i]=='(')lp++;
     if(logic1[i]==')')rp++;
     i++;
 }
 if(i>ll-2 || logic1[i] != '|' || logic1[i+1] != '('){
     PrintLog("Wrong syntax of l0fun");
     exit(3);
 }
 logic2=logic1.substr(i+1,logic1.length());
 logic1.erase(i,logic1.length());
 return 0;
}
int TriggerL0fun::Initialise(int ninp,TriggerInput* fInputs[],const string& line)
// Find Inputs and calculate luts
{
 unsigned int i=0;
 // name 
 while(i<line.length() && line[i] != ' ')i++;
 // Find inputs
 logic1=line.substr(i,line.length());
 splitstring(logic1,inputs," ~&|()");
 ninps = inputs.size(); 
 if(FindInputs(ninp,fInputs)) return 1;;
 for(int i=0;i<ninps;i++){
   if(index[i]>4)finps=12;
   if(index[i]>12){
     PrintLog("Wrong l0 function, index>12");
     exit(2);
   }
 }
 //cout << fname << " finps=" << finps << endl;
 //
 if(finps==4){
   // old l0f
   lut1 = new bool[1<<finps];
   return calculateLUT(lut1,logic1);
 }else{
   // new l0f ()|()
   splitl0fun34();
   //cout << "logics: " << logic1 << " | " << logic2 << endl;
   lut1 = new bool[1<<finps];
   lut2 = new bool[1<<finps];
   if(calculateLUT(lut1,logic1))return 1;
   return calculateLUT(lut2,logic2);
 }
 return 0;
}
int TriggerL0fun::FindInputs(int ninp,TriggerInput* fInputs[])
{
 // find indexes (positions in CTP)
 for(int i=0;i<ninps;i++){
    int j;
    for(j=0;j<ninp;j++){
       if(inputs[i].find(fInputs[j]->GetName()) != string::npos) break;
    }  
  if(j<ninp){
	index.push_back(fInputs[j]->GetPosition());
  }else{
	cout << "TriggerL0fun: can not find input " << inputs[i] << endl;
     	return 1;
  }
 }
 // check if inputs are in first 4 or 12 postions
 return 0;
}
//////////////////////////////////////////////////////////////////////////
TriggerDescriptor::TriggerDescriptor(int nall, string& name,vector<string> inps)
:
fname(name),
nall(nall),
ninps(0),nl0f(0)
{
 //for(int i=0;i<ninps;i++)inputs.push_back(inps[i+1]);
 inputs=inps;
 inputs.erase(inputs.begin());
 for(int i=0;i<NFUN;i++)fL0fun[i]=0;
 for(int i=0;i<NINP;i++)fTrigInputs[i]=0;
}
void TriggerDescriptor::Print()
{
 cout << fname << " " << ninps << " fun="<< nl0f << " inps=" << ninps;
 for(int i =0;i<nall;i++)cout << " " << inputs[i];
 cout << endl;
 for(int i=0;i<nl0f;i++)if(fL0fun[i])fL0fun[i]->Print();
 for(int i=0;i<ninps;i++)if(fTrigInputs[i])fTrigInputs[i]->Print();
}
int TriggerDescriptor::FindInputs(int ninp,int nl0frun,TriggerInput* fInputs[],TriggerL0fun* fl0f[])
{
 // makes links to ActiveRun l0 function and inputs
 for(int i=0;i<nall;i++){
    int j;
    if(inputs[i].find("l0f") != string::npos){
       for(j=0;j<nl0frun;j++){
          if(inputs[i].compare(fl0f[j]->GetName()) == 0) break;
       }
       if(j<nl0frun){
           fL0fun[nl0f++]=fl0f[j];
       }else{
          cout << "TriggerDescriptor: can not find l0f " << inputs[i] << endl;
         return 1;
       }
    }
    else{
       for(j=0;j<ninp;j++){
          if(inputs[i].find(fInputs[j]->GetName()) != string::npos) break;
       }  
       if(j<ninp){
 	 fTrigInputs[ninps++]=fInputs[j];
       }else{
	   cout << "TriggerDescriptor: can not find input " << inputs[i] << endl;
     	  return 1;
      }
    }
 }
 return 0;
}
int TriggerDescriptor::Initialise(int ninp,int nl0f,TriggerInput* fInputs[],TriggerL0fun* fl0f[])
{
 return FindInputs(ninp,nl0f,fInputs,fl0f);
}
//////////////////////////////////////////////////////////////////////
TriggerClass::TriggerClass(string &name,w8 index, TriggerCluster *cluster)
:
fname(name),
fIndex(index),
fCluster(cluster)
{
}
TriggerClass::TriggerClass(string &name,w8 index, TriggerCluster *cluster,w32 group,w32 time)
:
fname(name),
fIndex(index),
fCluster(cluster),
fGroup(group),fTime(time)
{
}
TriggerClass::TriggerClass(string &name,w8 index, TriggerCluster *cluster,TriggerDescriptor* td,w32 group,w32 time)
:
fname(name),
fIndex(index),
fDescriptor(td),
fCluster(cluster),
fGroup(group),fTime(time)
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
 printf("%s Index= %i %s %p ",fname.c_str(),fIndex,fCluster->GetName().c_str(),fCluster);
 printf("DESC= %s \n",fDescriptor->GetName().c_str());
}
///////////////////////////////////////////////////////////////////////////////////////
Detector* VALIDLTUS::dets[NDET];
int VALIDLTUS::count=0;
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
ActiveRun::ActiveRun()
:
fRunNumber(0),
fname(""),
frcfgfile(""),
partifile(""),
ninp(0),nclass(0),nclust(0),ndet(0),ntd(0),nl0f(0),
inputlist(0)
{
}
ActiveRun::ActiveRun(int runnum)
:
fRunNumber(runnum),
fname(""),
frcfgfile(""),
partifile(""),
ninp(0),nclass(0),nclust(0),ndet(0),ntd(0),nl0f(0),
inputlist(0)
{
 cout << "Starting run " << fRunNumber << endl;
 for(int i=0;i<NINP;i++)fTrigInputs[i]=0;
 for(int i=0;i<NCLUST;i++)fClusters[i]=0;
 for(int i=0;i<NCLASS;i++){fClasses[i]=0;fTrigDesc[i]=0;}
 for(int i=0;i<NDET;i++)fDetectors[i]=0;
 ParseValidCTPInputs();
 if(runnum){
   if(ParseConfigFile(runnum)) goto ERR;
 }
 else{
     // inputs, parse valid.ctpinputs and special input config
     //ParseInputsList();
     //ParseValidCTPInputs();
 }
 //ParsePartitionFile(runnum); // this is not necessary now, all info is in rcfg
 FindDetectors();
 //PrintDetectors();
 return ;
ERR:
 cout << "Error when creating run " << runnum << endl;
 exit(7);
}
ActiveRun::~ActiveRun()
{
 for(int i=0;i<ninp;i++) if(fTrigInputs[i])delete fTrigInputs[i];
 for(int i=0;i<nclust;i++) if(fClusters[i])delete fClusters[i];
 // classes are deleted in clusters
 for(int i=0;i<ntd;i++) if(fTrigDesc[i])delete fTrigDesc[i];
 for(int i=0;i<ndet;i++) if(fDetectors[i])delete fDetectors[i];
 cout << "Deleting instance run " << fRunNumber << endl;
}
void ActiveRun::PrintActiveRun()
{
 PrintInputs();
 PrintL0fun();
 PrintDescriptors();
 PrintClusters();
 //PrintClasses();
}
//-----------------------------------------------------------
int ActiveRun::FindDetectors()
// Find detectors of this active run
{
 for(int i=0;i<nclust;i++){
   string* dets = fClusters[i]->GetDetectors();
   for(int j=0;j<fClusters[i]->GetNdet();j++){
      int k=0;
      while(k<ndet && (fDetectors[k]->GetName().find(dets[j]) == string::npos))k++;
      if(k==ndet){
         if(VALIDLTUS::GetDetector(dets[j])){
           fDetectors[ndet++] = new Detector(*VALIDLTUS::GetDetector(dets[j]));
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
  //cout << "INTERACTION found." << endl;
  //fINT = new InteractionwCount; 
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
 //cout << "# of items: " << nitems <<  " level= " << level << endl;
 switch (level){
   case 1:  // inputs
          {
          if(nitems != 5){
            PrintLog(("Invalid input syntax: "+line).c_str());
            return 1;
          }
          TriggerInput* inp = new TriggerInput(items[0],atoi(items[2].c_str()),atoi(items[4].c_str()),atoi(items[3].c_str()),items[1]); 
          AddInput(inp);
          return 0;
          }
   case 2:  //interactions
          //cout << "INTERACTION found." << endl;
          //fINT = new InteractionwCount; 
          return 0;
   case 3:   // descriptors  and l0f
	  {
           bool nol0f=items[0].find("l0f") == string::npos;
           if(nol0f){
             //descr
             TriggerDescriptor* td = new TriggerDescriptor(nitems-1,items[0],items);
             //cout << nitems << " " << items[0] << endl;
             if(td->Initialise(ninp,nl0f,fTrigInputs,fL0fun)) return 1;
             AddTrigDesc(td);
             return 0;
           }else{
           //l0fun
             //cout << "----------------- " << items[0] << endl;
             removespaces(items[0]);
             TriggerL0fun* l0f = new TriggerL0fun(items[0]);
             if(l0f->Initialise(ninp,fTrigInputs,line))return 1;
             AddL0fun(l0f);
             return 0;
	   }
	  }
   case 4: //clusters
          {
          if(nitems<3){
            PrintLog(("Invalid cluster syntax: "+line).c_str());
            return 1;
          }
          TriggerCluster* cls = new TriggerCluster(items[0],atoi(items[1].c_str()));
         for(int i=2;i<nitems;i++){
           if(items[i].find("DAQ_TEST") != string::npos){
            string a("DAQ");
            cls->AddDetector(a);
            cout << "Changing DAQ_TEST to DAQ." << endl;
           }else cls->AddDetector(items[i]);
         }
         AddCluster(cls); 
         return 0;
         }
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
	    TriggerDescriptor *td=0;
            for(int j=0;j<ntd;j++){
               if((fTrigDesc[j]->GetName().find(items[2]) != string::npos)){
		 td=fTrigDesc[j];	
                 break;
	       }
            } 
            if(td==0){
              cout << "ProcessCfgLine(): Trigger Descriptor " << items[2] << " not found" << endl; 
	      return 1;
            }
            TriggerClass* clss;
            clss = new TriggerClass(items[0],atoi(items[1].c_str()),fClusters[i],td,atoi(items[8].c_str()),atoi(items[9].c_str()));
            AddClass(clss);
            //clss->Print();
            fClusters[i]->AddClass(clss);
            return 0;
          }
         }
         stringstream ss; 
         ss << "Class " << line << " cluster " << items[3] << " not found !" << endl;
         PrintLog(ss.str().c_str());
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
//-----------------------------------------------------------
int ActiveRun::ParseConfigFile(int runnumber){
 ifstream file;
 stringstream ss;
 if(runnumber){
   ss << "/WORK/RCFG/r" <<runnumber << ".rcfg";
   frcfgfile = getenv("VMEWORKDIR")+ss.str();
 }else{
   cout << "ActiveRun::ParseConfigFile runnum=0, internal error." << endl;
   return 1;
 }
 file.open(frcfgfile.c_str());
 if(!file){
  PrintLog(("ActiveRun: cannot open file: "+frcfgfile).c_str());
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
//-----------------------------------------------------------
int ActiveRun::ParseValidCTPInputs()
{
  stringstream ss;
 ifstream file;
 ss << "/CFG/ctp/DB/VALID.CTPINPUTS";
 string filename = getenv("VMECFDIR")+ss.str();
 file.open(filename.c_str());
 if(!file){
  PrintLog(("ActiveRun: cannot open file: "+filename).c_str());
  return 1;
 }else{
  PrintLog(("ActiveRun: File: "+filename+" opened.").c_str());
 }
 int rt=0;
 string line;
 while(getline(file,line)){
   if(line[0]=='#') continue ;  // comment
   if(line[0]=='l') continue ;  // l0function
   rt=ProcessInputLine(line);
   //cout << line << endl;
 }
 file.close();
 return rt;
}
int ActiveRun::ProcessInputLine(const string &line)
// Process input line from VALID.CTPINPUTS
{
 vector<string> items;
 splitstring(line,items," ");
 int nitems = items.size();
 if(nitems!=12){
   cout << "unexpected number of items in VALID.CTPINPURS: line:" << endl;
   cout << line << endl;
   return 1;
 }
 // loop input list and add to configuration only if in the list
 for(unsigned int i=0;i<inputlist.size();i++){
  if(items[0].find(inputlist[i]) != string::npos){  
    // check position of signature
    TriggerInput* inp = new TriggerInput(items[0],atoi(items[3].c_str()),atoi(items[5].c_str()),atoi(items[4].c_str()),items[2]); 
    inp->Print();
    AddInput(inp);
  } 
 }
 return 0;
}
void ActiveRun::PrintL0fun()
{
 cout << "L0FUN:" << endl;
 for(int i=0;i<nl0f;i++)fL0fun[i]->Print();
}
void ActiveRun::PrintDescriptors()
{
 cout << "DESCRIPTORS:" << endl;
 for(int i=0;i<ntd;i++)fTrigDesc[i]->Print();
}
void ActiveRun::PrintInputs()
{
 cout << "INPUTS: " << endl;
 for(int i=0;i<ninp;i++)fTrigInputs[i]->Print();
}
void ActiveRun::PrintClusters()
{
 cout << "CLUSTERS:" << endl;
 for(int i=0;i<nclust;i++)fClusters[i]->Print();
}
void ActiveRun::PrintClasses()
{
 cout << "CLASSES:" << endl;
 for(int i=0;i<nclass;i++)fClasses[i]->Print();
}
void ActiveRun::PrintDetectors()
{
 for(int i=0;i<ndet;i++)fDetectors[i]->Print();
}
int ActiveRun::Load2SW()
{
 // Inputs (BC1,NC2,RND1,RND2
 for(int i=0;i<ninp;i++)if(fTrigInputs[i]->Load2SW(fRunNumber)){
        cout << "RUN " << fRunNumber << " ";
	PrintLog("Cannot load2sw inputs BC/RND.");
        return 1;
    }
 // L0fun
 for(int i=0;i<nl0f;i++){
    if(fL0fun[i]->Load2SW(fRunNumber)){
        cout << "RUN " << fRunNumber << " ";
	PrintLog("Cannot load2sw l0f.");
        return 1;
    }
 }
 return 0;
}
void ActiveRun::Load2HW()
{
 CTPHardware::Load(fRunNumber);
}





