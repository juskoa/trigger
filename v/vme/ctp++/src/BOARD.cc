#include "BOARD.h"
std::vector<string> BOARD::AllCounterNames;

BOARD::BOARD(string const name,w32 const boardbase,int vsp,int nofssmmodes)
:
        BOARDBASIC(name,boardbase,vsp),
	NCounters(NCOUNTERS_MAX),NCountersfromcnames(0),
        countdiff(0),
	SSMcommand(0x19c),SSMstart(0x1a0),SSMstop(0x1a4),SSMaddress(0x1a8),SSMdata(0x1ac),SSMstatus(0x1b0),SSMenable(0x1b4),
	COPYCOUNT(0x1d4),COPYBUSY(0x1d8),COPYCLEARADD(0x1dc),COPYREAD(0x1e0),
        numofmodes(nofssmmodes),
        ssm(0),
	ssmmode(0),
	SSMomvmer(0),SSMomvmew(1),SSMbusybit(0),
	SSMModes(0)
{
 if(d_name == "ltu")SSMbusybit=0x04; else SSMbusybit=0x100;
 //cout << d_name << " BOARD done"<<endl;
 ssm = new w32[Mega];
 SSMModes = new SSMmode[numofmodes];
 ssmtools.setssm(ssm);
 counters1 =  new w32[NCOUNTERS_MAX];
 counters2 =  new w32[NCOUNTERS_MAX];
 countdiff =  new w32[NCOUNTERS_MAX];
 printf("WARNING: reading counter names off\n");
 //getCounterNames(name);
}
BOARD::~BOARD()
{
 delete [] SSMModes;
 delete [] counters1;
 delete [] countdiff;
 delete [] counters2;
}
//---------------------------------------------------------------------------
int BOARD::readAllCountersNames()
{
 ifstream counternamesfile;
 char *environ;
 environ= getenv("VMECFDIR");
 if(environ==0){
   printf(" VMECFDIR does not exist \n");
   return 1;
 }
 string envi(environ);
 //string file("/CFG/ctp/DB/cnames.sorted2");
 string file("/CFG/ctp/DB/cnamesLM.sorted2");
 file=envi+file;
 printf("Reading file %s \n",file.c_str());
 counternamesfile.open(file.c_str());
 if(!counternamesfile.is_open()){
   printf("Cannot open file %s \n",file.c_str());
   return 1;
 }
 while(! counternamesfile.eof()){
  string buffer;
  getline(counternamesfile,buffer);
  AllCounterNames.push_back(buffer);
 }
 printf("# of lines in cnames: %i \n",(int)AllCounterNames.size());
 return 0;
}
int BOARD::getCounterNames(const string& board)
{
 printf("%s: Getting counter names \n",getName().c_str());
 if(AllCounterNames.empty()){
   readAllCountersNames();
 }
 int nc=0;
 for(w32 i=0;i<AllCounterNames.size();i++){
   //if(i==1511) break;
   if(i==1711) break;
   vector<string> items;
   splitstring(AllCounterNames[i],items," ");
   if(items.size() == 0) continue;
   if(items.size() != 5){
     printf("Error: wrong syntax CounterNames: %i %s \n",(int)items.size(),AllCounterNames[i].c_str());
     return 1;	
   }
   //cout << "#"<<items[2] << "#board:#" << board << "#compare: " << stripstring(items[2]).compare(0,2,board) << endl;
   if((items[2][0] == board[0]) && (items[2][1] == board[1])){
     // fo has to be treated specially
     if((board[0]=='f') && (items[2][2]==board[2])){
       CounterNames.push_back(items[0]);
       nc++;
     }else if(board[0] != 'f'){
       CounterNames.push_back(items[0]);
       nc++;
     }
   }
 }
 NCountersfromcnames=nc;
 printf("%s Number of counters from cnames: %i \n",d_name.c_str(),nc);
 return 0;
}
int BOARD::readCounters()
{
 vmew(COPYCLEARADD,0x0);
 for(int i=0;i<NCounters;i++){
    counters1[i]=counters2[i];
    counters2[i]=vmer(COPYREAD);
 }
 return 0;
}
int BOARD::readcopyCounters()
{
 w32 loop=0;;
 while(vmer(COPYBUSY) && loop<10 ){
   usleep(10);
   loop++;
 }
 if(loop==10){
   printf("readCounters: cannot read , counters busy after 10 attempts \n");
   return 1;
 }
 vmew(COPYCOUNT,0x0);
 usleep(10);
 vmew(COPYCLEARADD,0x0);
 for(int i=0;i<NCounters;i++){
    counters1[i]=counters2[i];
    counters2[i]=vmer(COPYREAD);
 }
 return 0;
}
int BOARD::readCountersDiff()
{
 for(int i=0;i<NCounters;i++){
    w32 cur=counters2[i];
    w32 prev=counters1[i];
    w32 dif;
    if(cur >= prev) {
        dif= cur-prev;
    } else {
        dif= (0xffffffff - prev) + cur +1;
    };
    countdiff[i]=dif;
 }
 return 0;
}
void BOARD::printCounters()
{
 printf("Board %s counters:\n",getName().c_str());
 for(int i=0;i<NCounters;i++)printf("%s %i %x \n",CounterNames[i].c_str(),i,counters2[i]);
}
void BOARD::printCountersDiff()
{
 for(int i=0;i<NCounters;i++){
    w32 cur=counters2[i];
    w32 prev=counters1[i];
    w32 dif;
    if(cur >= prev) {
        dif= cur-prev;
    } else {
        dif= (0xffffffff - prev) + cur +1;
    };
    countdiff[i]=dif;
 }
 printf("Board %s counters:\n",getName().c_str());
 for(int i=0;i<NCounters;i++){
   if(countdiff[i]) printf("%s %i %u \n",CounterNames[i].c_str(),i,countdiff[i]);
 }
}
//---------------------------------------------------------------------------
string *BOARD::GetChannels(string const &mode) const
{
 for(int i=0; i<numofmodes; i++){
   if(SSMModes[i].name == mode) return SSMModes[i].channels;
 }
 return 0;
}
w32 BOARD::getChannel(string const &channel) const
{
 for(int i=0;i<32;i++){
   if(SSMModes[ssmmode].channels[i]==channel) return i;
 }
 cout << "GetChannel: " << channel << " not found in mode " << SSMModes[ssmmode].name << endl;
 return 100;
}
//---------------------------------------------------------------------------
void BOARD::PrintChannels(string const &mode) const
{
 cout << "Board " << d_name << "mode " << mode ;
 for(int i=0; i<numofmodes; i++){
   if(SSMModes[i].name == mode){
      cout << endl;
      for(int j=0;j<32;j++)cout << SSMModes[i].channels[j] << endl;
      return;
   }
 }
 cout << "mode not found." << endl; 
}
//--------------------------------------------------------------------------
// mode  = ingen,inmon,outgen,outmon as in the file names CFG/ctp/ssmsigs
int BOARD::AddSSMmode(string const modename,int const imode) 
{
 SetFile(modename);
 for(int i=0;i<32;i++)SSMModes[imode].channels[i]="empty";
 SSMModes[imode].name=modename;
 bool modflag=1;
 while(! modefile.eof()){
  string buffer;
  getline(modefile,buffer);
  //cout << buffer << endl;
  int channel=atoi(buffer.substr(0,2).c_str());
  //cout << channel << endl;  
  if(buffer.length() < 3) continue;
  if(channel <= 32){
    int i=0;while(buffer[i] != ' ')i++;
    //cout << buffer.length() << endl;
    string name=stripstring(buffer.substr(i,buffer.length()));
    //string name=buffer.substr(3,buffer.length());
    //cout << name << endl;
    if(channel < 32){
      SSMModes[imode].channels[channel]=name;
    }
    else if(channel == 32){    // mode
      modflag=0;
      SSMModes[imode].modecode=parsemode(name);
    }
  }
 }
 modefile.clear();
 modefile.close();
 if(modflag){
   cout << "GetCHannels: no mode in file " << modename << endl;
  return 1;
 }
 return 0;
}
//-----------------------------------------------------------------------------
void BOARD::SetFile(string const &modename) 
{
 char *environ;
 environ= getenv("VMECFDIR"); 
 string cfgdir(environ);
 string name=cfgdir+"/CFG/ctp/ssmsigs/"+d_name+"_"+modename+".sig";
 //cout << "Mode file:"<< name << endl;
 modefile.open(name.c_str());
 if(!modefile.is_open()){
  cout << "File "<< name << " cannot be opened, exiting." << endl;
  exit(1);
 }else{
  //cout << "File "<< name << " opened successfully" << endl;
 }

}
//-----------------------------------------------------------------------------
int BOARD::parsemode(string const &mode) const
{
 w32 modecode=0;
 for(int i=0;i<6;i++){
    if(mode[i] == '1') modecode=modecode + (1<<i);
 }
 if(mode[6] == '1') modecode=modecode + (1<<8);
 if(mode[7] == '1') modecode=modecode + (1<<9);
 //cout << "parsemode: " << hex << modecode << endl;
 return modecode;
}
//-----------------------------------------------------------------------------
void BOARD::WriteSSM(w32 const word) const 
{
 for(int i=0;i<Mega;i++)ssm[i]=word;
}
//-----------------------------------------------------------------------------
void BOARD::WriteSSM(w32 const word,int const start,int const last) const
{
 for(int i=start;i<last;i++){
	 ssm[i%Mega]=word;
 cout << Mega <<" WriteSSM: "<< (i%Mega) << endl;
 }
}
//----------------------------------------------------------------------------
int BOARD::DumpSSM(const char *name) const
{
 char filename[200];
 char *environ;
 char fnpath[1024];
 FILE *dump;
 sprintf(filename,"%s",name);
  // Open file
 environ= getenv("VMEWORKDIR"); strcpy(fnpath, environ);
 strcat(fnpath,"/"); strcat(fnpath,"WORK/"); 
 strcat(fnpath, filename); strcat(fnpath, ".dump");
 dump=fopen(fnpath,"w");
 if(dump == NULL){
  printf("Cannot open file: fnpath: %s\n", fnpath);
  exit(1);
 }
 for(int i=0; i<Mega; i++) {
    //w32 d=ssm[i]&0x3ffff;
    //w32 d=ssm[i]&0xffffffff;
    w32 d=ssm[i];
    fwrite(&d, sizeof(w32), 1, dump);
  };
 fclose(dump); 
 printf("Dump written in fnpath %s\n", fnpath);
 return 0;
}
//-----------------------------------------------------------------------------
int BOARD::ReadSSMDump(const char *name) const
{
  int i,nwords;
  w32 word; 
  char filename[200];
  char *environ;
  char fnpath[1024];
  FILE *dump;
  sprintf(filename,"%s",name);
  environ= getenv("VMEWORKDIR"); strcpy(fnpath, environ);
  strcat(fnpath,"/"); strcat(fnpath,"WORK/"); 
  strcat(fnpath, filename); strcat(fnpath, ".dump");
  dump = fopen(fnpath,"rb");
   if(dump==NULL) {
    printf("cannot open file %s\n",filename);
    return(1);
  };
  for(i=0;i<Mega;i++){
    nwords=fread(&word,sizeof(w32),1,dump);
    //printf("%i 0x%x %i\n",i,word,nwords);
    ssm[i]=word;
  }

 return 0;
}
//-----------------------------------------------------------------------------
int BOARD::ReadSSM() const
{
 w32 status;
 status=vmer(SSMstatus); 
 //cout << "status= " << hex << status << endl;
 w32 enable=(status&0xc0)<<2;
 w32 mod=SSMomvmer | (status&0x38) | enable;
 w32 ssma=vmer(SSMaddress);
 int rc=setomvspSSM(mod);
 if(rc) return rc;
 w32 nwords=Mega;
 if((ssma & 0x100000)==0){ //overflow bit is 0
    
   //vmew(SSMaddress,Mega-1);  
   vmew(SSMaddress,(w32)-1);
   nwords=ssma;
   printf("Warning: readins SSM without overflow flag: %i read \n",nwords);
 }  
 w32 d= vmer(SSMdata);
 d= vmer(SSMdata);
 for(w32 i=0; i<nwords; i++) {
   ssm[i]= vmer(SSMdata);
 };
 return 0;
}
//-----------------------------------------------------------------------------
int BOARD::WritehwSSM() const
{
  w32 status= vmer(SSMstatus);
  w32 enable= (status&0xc0)<<2;
  w32 mod= SSMomvmew | (status&0x38) | enable;
  int rc= setomvspSSM(mod);
  if(rc) return rc;
  w32 address=0;
  vmew(SSMaddress, address-1);
  for(int ix=0; ix<Mega; ix++)vmew(SSMdata,ssm[ix]);
  /* Final write addr. should be == Mega-1 */
  w32 data= vmer(SSMaddress);
  if( data != (Mega-1)) {
  cout << "ERROR: final write SSMaddress: " << hex << data <<
	  " expected: " << hex << Mega-1 << dec <<endl;
  rc=-1;
};
 return(rc);
}
//-----------------------------------------------------------------------------
void BOARD::PrintSSM(int const start,int const n) const
{
 for(int i=start;i<n;i++) cout << hex << i << " " << hex << ssm[i] << endl;
}
//----------------------------------------------------------------------------
// version from ssm.c
int BOARD::setomvspSSM(w32 const mod) const
{
 w32 bcstatus = getBCstatus();
 if(((0x7&mod)!=SSMomvmer && (0x7&mod)!=SSMomvmew)&&((bcstatus&0x3)!=0x2)) {
  cout << "Warning: BC signal not connected"<< endl;
  /* BC not necessary for vme R/W*/
  //return(1);
 }
 w32 status=vmer(SSMstatus);
 if( status & SSMbusybit) {
   cout << "SSM busy, stopping recording before setting new/op. mode..."<< endl;
   /*  vmew32(SSMstop+BSP*ctpboards[board].dial,DUMMYVAL); */
   vmew(SSMstop,0x0);
   /*  status=vmer32(SSMstatus+BSP*ctpboards[board].dial); */
   status=vmer(SSMstatus);
   if( status & SSMbusybit) {
     cout<<"ERROR: Cannot stop recording, operation or mode wasn't set!"<<endl;
     return(1);
   }
 }
 vmew(SSMcommand,mod&0x3f);
 if(d_name != "ltu"){
  w32  ssmen= (mod>>8)&3;
  vmew(SSMenable, ssmen);    
 }
 return 0;
}
//----------------------------------------------------------------------------
// cont  = 'c' :continous
//      != 'c': 1 pass
//      - assuming that modes in files are always 1 pass
int BOARD::SetMode(string const &mode,char const cont,w32 &imode) const
{
 int rc=1;
 for(int i=0;i<numofmodes;i++){
  if(SSMModes[i].name == mode){
    w32 modecode=SSMModes[i].modecode;
    if((cont == 'c'))modecode=modecode+1;
    rc= SetMode(modecode);
    imode=i;
    return rc;
  }
 }
 cout << "SetMode: " << mode << " not found." << endl;
 return rc;
}
int BOARD::SetMode(string const &mode,char const cont)
{
 w32 imode=0;
 int rc=SetMode(mode,cont,imode);
 if(rc==0) ssmmode=imode;
 return rc;
}
//----------------------------------------------------------------------------
int BOARD::SetMode(w32 const modecode) const
{
 int rc;
 ///printf("Setting mode for %s: %x \n",d_name.c_str(), modecode);
 if(d_name == "ltu"){
   if( (modecode&7) >3) {
     cout << "ERROR: setomSSM: " << modecode << ">3 for ltu board" << endl;
     return(2);
   }
   if( (modecode&0x10) != 0x10) {
    cout << "WARNING: setomSSM: "<< modecode <<" bit 0x10 not set for ltu board" <<endl;
   }
   rc= setomvspSSM(modecode&3);
 }else
   printf("setomSSM: 0x%x \n",modecode);
   rc= setomvspSSM(modecode);   //CTP Board
 //ssmmode=modecode;
 return rc;
}
//---------------------------------------------------------------------------
int BOARD::StartSSM() const{
   vmew(SSMaddress,0x0);
   vmer(SSMstatus);   // not used but follow anton
   vmew(SSMstart,0xffffffff);
   return 0;
}
//----------------------------------------------------------------------------
void BOARD::StopSSM() const
{
 w32 status = vmer(SSMstatus);
 if((status&SSMbusybit) == 0) {} // warn;
 vmew(SSMstop,0x0);
}
//--------------------------------------------------------------------------------------------
void BOARD::printL2DataBackplane()
{
 cout << d_name << " BOARD: Printing L2 list - decoded backplane serial L2 data,  size: " << ql2backplane.size() << endl;
 for(w32 i=0;i<ql2backplane.size();i++)printL2Data(ql2backplane[i]);
}
//--------------------------------------------------------------------------------------------
void BOARD::L2DataBackplane()
{
 int i=0,j,iorbit=0;
 int l2clusters,bcid,orbit,esr;
 int rc=0;
 w64 l2class1,l2class2;
 w32 sl2strobech,sdata1ch,sdata2ch;
 if((sl2strobech=getChannel("l2strobe"))>32)rc=1;
 if((sdata1ch=getChannel("l2data1"))>32) rc=1;
 if((sdata2ch=getChannel("l2data2"))>32) rc=1;
 if(rc){
   printf("Error in L2DataBackplane: channels not found.\n");
   return;
 }
 printf("L2dataBackplane: l2strobe l2data1 l2data2 channels= %i %i %i\n",sl2strobech,sdata1ch,sdata2ch);
 L2Data L2a,ORBIT;
 clearL2Data(L2a);
 clearL2Data(ORBIT);
 w32* sm=GetSSM();
 while(i<Mega){
  // ORBIT pulse
  if(bit(sm[i],0)){
   if(!iorbit){
    ORBIT.issm=i;
    qorbitl2data.push_back(ORBIT);
    iorbit=1;
   }
  }else iorbit=0;
  if(bit(sm[i],sl2strobech)){
   L2a.issm=i;
   // L2 clusters -> one integer
   //w32 l2clstt=bit(sm[i],sdata1ch);  // test cluster
   i++;
   l2clusters=0;
   j=0;
   //while((j<6) && (i+j)<Mega){    // 6 clusters
   while((j<8) && (i+j)<Mega){      // 8 clusters
    //l2clusters=l2clusters+bit(sm[i+j],sdata1ch)*(1<<(5-j));
    l2clusters=l2clusters+bit(sm[i+j],sdata1ch)*(1<<(7-j));
    j++;
   }
   i=i+8;
   // BCID
   j=0;
   bcid=0;
   while((j<12) && (i+j)<Mega){
    bcid=bcid+bit(sm[i+j],sdata1ch)*(1<<(11-j));
    j++;
   }
   i=i+12;
   //ORBIT
   j=0;
   orbit=0;
   while((j<24) && (i+j)<Mega){
    orbit=orbit+bit(sm[i+j],sdata1ch)*(1<<(23-j));
    j++;
   }
   i=i+24;
   esr=bit(sm[i],sdata1ch);
   i=i+1+10;   // 10 gap
   // L2class
   j=0;
   //100 classes
   l2class1=0;
   while((j<40) && (i+j)<Mega){
    if(bit(sm[i+j],sdata2ch))l2class1=l2class1+(1ull<<(39-j));
    j++;
   }
   i=i+40;
   j=0;
   l2class2=0;
   while((j<60) && (i+j)<Mega){
    if(bit(sm[i+j],sdata2ch))l2class2=l2class2+(1ull<<(59-j));
    j++;
   }
   L2a.l2clusters=l2clusters;
   L2a.l2classes1=l2class1;
   L2a.l2classes2=l2class2;
   L2a.bcid=bcid;
   L2a.orbit=orbit; 
   L2a.esr=esr;
   L2a.clt=0;  // to be read from hw
   L2a.swc=0;  // to be read from hw
   ql2backplane.push_back(L2a);
   clearL2Data(L2a);
   //printf("comparel2aCTPreadout: l2 clusters: 0x%x BCID: %i ORBIT: %i L2class: 0x%llx \n",l2clusters,bcid,orbit,l2class);
  }else i++;
 }
 //printlistN(L2alist);
 return; 
}

