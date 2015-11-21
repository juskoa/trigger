#include "CTP.h"
CTP::CTP():
busy(0),l0(0),l1(0),l2(0),inter(0),numofltus(0),numoffos(0),
vspctp(-1),vspltu(-1),
debug(0)
{
 for(int i=0;i<NUMOFFO;i++){
    fo[i]=0;
    for(int j=0;j<NUMOFCON;j++){
       fo2det[i][j]=0;
       ltu[i*NUMOFFO+j]=0;
    }
 }
 readBICfile();
 readDBVALIDLTUS();
 cout << "Following boards in crate: " << endl;
 printboards(); 
 cout << "CTP construction finished." << endl;
}
CTP::~CTP(){
 for
    (
     list<BOARD*>::iterator from = boards.begin();
            from != boards.end();
            ++from
        ){
          //cout << (*from)->getName() << " is being deleted." << endl;
          delete (*from);
         }
 cout<< "~CTP: " << boards.size() << endl;
}
int CTP::readBCStatus(int n, w32 delta)
{
 int ret=0;
 printf("#boards %i \n",boards.size());
 int nb=boards.size();
 int bcstat[nb][4];
 for(int i=0;i<nb;i++){
    for(int j=0;j<4;j++){
       bcstat[i][j]=0;
    }
 }
 for(int i=0;i<n;i++){
    int j=0;
    for (list<BOARD*>::iterator from = boards.begin();
        from != boards.end();++from)
   {
     w32 bcstatus = (*from)->getBCstatus();
     bcstatus=bcstatus & 0x3; // why ltu stat can be > 3 ?
     bcstat[j][bcstatus]++;
     j++;
     if(delta)usleep(delta);
   }
  }
  int j=0;
  for (list<BOARD*>::iterator from = boards.begin();
        from != boards.end();++from)
  {
   string name=(*from)->getName();
   printf("%5s: BCSTASTUSES: 0: %5i 1: %5i 2: %5i 3: %5i \n",name.c_str(),bcstat[j][0],bcstat[j][1],bcstat[j][2],bcstat[j][3]);
   if(bcstat[j][2] != n) ret =1;
   j++;
  }
  return ret;
}
int CTP::readCounters()
{
 for (list<BOARD*>::iterator from = boards.begin();
        from != boards.end();++from) (*from)->copyCounters();
 usleep(10); 
 printf("CTP: read counters: ");
 for (list<BOARD*>::iterator from = boards.begin();
        from != boards.end();++from){
    printf("%s ",(*from)->getName().c_str());
    (*from)->readCounters();
    (*from)->readCountersDiff();
 }
 printf("\n");
 return 0;
}
void CTP::printboards() 
{
 for
    (
     list<BOARD*>::iterator from = boards.begin();
            from != boards.end();
            ++from
        ){
            (*from)->printboardinfo("no text");
            string name=(*from)->getName();
            w32 bcstatus = (*from)->getBCstatus();
            bcstatus=bcstatus & 0x3;
            //if(name != "busy"){
              if(bcstatus != 2){
               //cout << endl << name << " Incorrect BC status" << endl;
               printf("Incorrect BC status: 0x%x , board %s \n",bcstatus,name.c_str());
               //exit(1);
              }
            //}
         }
}
//----------------------------------------------------------------------------
void CTP::getboard(string const &line)
{
 if(line.length() < 8) return ;
 //if(!line.compare(line.length()-8,4,"0x81")){
 if(!line.compare(0,3,"ltu")){
    // LTU BOARD
    // ltus should not be CTP ?
    printf("WARNING: LTU  ignored ! \n");
    return ;
    string name = "ltu";
    //name=name+int2char(numofltus+1);
    string base=line.substr(line.length()-8,line.length());
    w32 basehex;
    convertS2H(basehex,base);
    //ltu[numofltus] = new BOARD(name,basehex,vspltu,1);
    ltu[numofltus] = new LTUBOARD(name,basehex,vspltu);
    vspltu=ltu[numofltus]->getvsp();  
    boards.push_back(ltu[numofltus]);    
    numofltus++; 
 }else if(!line.compare(line.length()-8,4,"0x80")){
    //cout << "ttc found " << endl;
 }else if(!line.compare(0,4,"busy")){
    // BUSY board
    //cout << " busy board found " << endl;
    busy = new BUSYBOARD(vspctp);
    vspctp=busy->getvsp();
    boards.push_back(busy);    
 }else if(!line.compare(0,2,"l0")){
    // L0 board
    //printf("WARNING: L0 ignored ! \n");
    //return ;
    //cout << " l0 board found " << endl;
    // Get l0ver either from system or from reading board ny board
    BOARDBASIC* bb = new BOARDBASIC("l0",0x829000,vspctp);
    int l0ver = bb->getFPGAversion();
    printf("L0 board version: 0x%x \n",l0ver);
    delete bb;
    //if(l0ver < 0xc0) l0 = new L0BOARD1(vspctp);
    if(l0ver < 0xc0){
      printf("Unexpected version of L0 board \n");
      exit(1);
    }
    else l0=new L0BOARD2(vspctp);
    vspctp=l0->getvsp();
    boards.push_back(l0);    
 }else if(!line.compare(0,2,"l1")){
    // L1 board
    //cout << " l1 board found " << endl;
    l1 = new L1BOARD(vspctp);
    vspctp=l1->getvsp();
    boards.push_back(l1);    
 }else if(!line.compare(0,2,"l2")){
    //L2 board
    //cout << " l2 board found " << endl;
    l2 = new L2BOARD(vspctp);
    vspctp=l2->getvsp();
    boards.push_back(l2);
 }else if(!line.compare(0,3,"int")){
    // INT board
    inter = new INTBOARD(vspctp);
    vspctp=inter->getvsp();
    boards.push_back(inter); 
 }else  if(!line.compare(0,2,"fo")){
    // FO board
    //cout << "fo found "<< endl;
    const char *fonum=&line[7];
    int num=char2int(*fonum)-1;   //-1 since fo are now from 1
    string name= "fo";
    string base=line.substr(line.length()-8,line.length());
    name = name+base[4];
    w32 basehex;
    convertS2H(basehex,base);
    fo[num] = new FOBOARD(basehex,vspctp,name);
    vspctp=fo[num]->getvsp();
    boards.push_back(fo[num]);
    //name.append(fonum);   fo name is 'fo', fo are recognised by vmebaseaddress
    //fo[num]->SetName(name);
 }else{
  cout << "Unknown crate in board " << line << ". Exiting()" << endl;
  exit(1);
 }
}
//----------------------------------------------------------------------------
void CTP::readBICfile()
{
 ifstream file;
 string const BICfile = "/root/NOTES/boardsincrate";
 file.open(BICfile.c_str());
  if(! file){
  cout << "File "<< BICfile  << " cannot be opened, exiting." << endl;
  exit(1);
 }
 //cout << "File "<< bicfile << " opened." << endl;
 string line;
 while(! file.eof()){
  getline(file,line);
  //cout << line << endl;
  getboard(line);
 }
 file.clear();
 file.close();
}
//-----------------------------------------------------------------------------
void CTP::readDBVALIDLTUS()
{
 ifstream file;
 //string const DBfile = "../CFG/ctp/DB/VALID.LTUS";
 char* DBfile;
 DBfile=getenv("dbctp");
 string DBfilestr(DBfile);
 DBfilestr=DBfilestr+"/VALID.LTUS";
 file.open(DBfilestr.c_str());
  if(! file){
  cout << "File "<< DBfilestr  << " cannot be opened, exiting." << endl;
  exit(1);
 }
 //cout << "File "<< bicfile << " opened." << endl;
 string line;
 while(! file.eof()){
  getline(file,line);
  //cout << line << endl;
  getdetector(line);
 }
 file.clear();
 file.close();
}
//---------------------------------------------------------------------------
void CTP::getdetector(string const &line){
 vector<string> items; 
 //cout << line << endl;
 splitstring(line,items," =");
 if(items.size() != 6) return;
 if(stripstring(items[0])[0] == '#') return;
 int numname,fo,focon;
 string name;
 name=stripstring(items[0]);
 numname=string2int(stripstring(items[1]));
 fo=string2int(stripstring(items[2]));
 focon=string2int(stripstring(items[3]));
 DETECTOR *det = new DETECTOR;
 det->name=name;
 det->numname = numname;
 det->fo=fo;
 det->focon=focon;
 string vmeordim = stripstring(items[5]);
 if(vmeordim[0] == '0'){
   det->ltuvmeaddress = vmeordim;
   convertS2H(det->ltuvmeaddhex,det->ltuvmeaddress);
   det->dimserver="None";
 }else{
   det->ltuvmeaddress="None";
   det->ltuvmeaddhex=0;
   det->dimserver=vmeordim;
 }
 det->ltunum=-1;
 for(int i=0;i<numofltus;i++){
  if((ltu[i]->getboardbase()) == det->ltuvmeaddhex){
   det->ltunum=i;
   det->ltu=ltu[i];
  }
 }
 fo2det[fo-1][focon-1] = det;
 det->print();
 return ;
}
//---------------------------------------------------------------------------
// Reads ctp cfg file
int CTP::readCFG(string const &name){
 // this should be available in ctplib ?
return 0;
}
//---------------------------------------------------------------------------
// Set default trigger
int CTP::setSWtrigger(char triggertype,w32 BC, w32 detectors,w32 lm)
{
 w32 word,INTtcset;
 //     LM0         P/F      BCM4   BCM3    BCM2    BCM1
 word=(lm<<19)+(1<<18)+(1<<17)+(1<<16)+(1<<15)+(1<<14);
 // roc
 INTtcset=(1<<1);
 switch(triggertype){
  case 'a':
   // mask later
   break;
  case 's':
    // synchrneous
    word=word+(1<<12)+BC;
    //printf("setswtrig: synchr trigger 0x%x \n",word);
    break;
  case 'c':
    // calibration
    word=word+(1<<12)+(1<<13)+BC;
    INTtcset= INTtcset | 1;
    break;
  default:
    printf("Error: setswtrig: unknown type of trigger %c \n",triggertype);
    return 1;
 }
 // L0 board
 l0->setTCSET(word);
 // L1 board
 word=(1<<18);
 l1->setTCSET(word);
 // L2 board
 word=(1<<24)+detectors;
 l2->setTCSET(word);
 // INT board
 inter->setTCSET(INTtcset);
 // FO boards
 // all connectors
 // to be modified - now hardwired phos at lab
 //word = 0xf<<16;
 word = 0x120070;
 fo[0]->setTESTCLUSTER(word);
 // BUSY BOARD
 // all detectors - to be modified
 busy->setCluster(0,0x2);
 return 0;
}
int CTP::startSWtrigger(char triggertype,w32 lm)
{
 int TIMEOUT=100;
 // for calib check PP request
 // syn and calib check l0 request
 int i=0;
 usleep(200);
 while(l0->getL0rqst() && i<TIMEOUT)i++;
  if(i>=TIMEOUT){
    printf("Timeout at L0 request \n");
    return 1;
  } 
 usleep(200);
 i=0;
 while(!(l0->getl0ackn()) && i<TIMEOUT)i++;
  if(i>=TIMEOUT){
    printf("Timeout at L0 acknowledge \n");
    printf("TCSTATUS: 0x%x \n",l0->getTCSTATUS());
    return 2;
  }
 usleep(10000); 
 printf("l1a : %x \n",l1->getl1ackn());
 return 0;
 // L1 level
 i=0; 
 while(!(l1->getl1ackn()) && i<TIMEOUT)i++;
 //time[itime++]=CountTime();
 if(i>=TIMEOUT){
  printf("Timeout at L1 acknowledge \n");
  return 3;;
 };
 // L2 level
 w32 flag=l2->getl2ackn();
 i=0;
 while(((flag&0xc) == 0) && i<TIMEOUT) {   //wait L2a/L2r ACK
  flag=l2->getl2ackn();
  mysleep(10);
  i++;
 };
 //if(DBGswtrg4) printf("  l2ackn:0x%x loops(10us sleep) %i \n",flag, i);
 if(i >= TIMEOUT){
  printf("startswtrig: Timeout at l2ackn. \n");
  return 7;
 };
 if(flag == 4){         // L2r ack
  return 3;
 }else if(flag == 8){   // L2a ack
  return 0;
 }else{
  printf("startswtrig: FAIL, flag=%i %i I should never be here.\n",flag,i);
  return 4; // there should be l2a or l2r 
 }

 return 0;
}

void CTP::readOrbits()
{
 for(int i=0;i<100000;i++){
   CountTime();
   w32 l0o=l0->readOrbit();
   w32 l2o=l2->readOrbit();
   w32 into=inter->readOrbit();
   w32 time=CountTime();
   int del1=l0o-l2o;
   int del2=l0o-into;
   if(i%10000 == 0)printf("%i \n",i);
   if((abs(del1)>2) || (abs(del2)>1)){
     printf("%i Time: %i usecs Orbits l0,l2,int: 0x%x 0x%x 0x%x %i %i \n",i,time,l0o,l2o,into,l0o-l2o,l0o-into);
   }
   usleep(1);
 }
}
