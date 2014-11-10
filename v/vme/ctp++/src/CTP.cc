#include "CTP.h"
CTP::CTP()
{
 vspctp=-1;
 vspltu=-1;
 numofltus=0;
 numoffos=0;
 l0=0;l1=0;l2=0;inter=0;busy=0;
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
int CTP::readBCStatus(int n)
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
            if(name != "busy"){
              if(bcstatus != 2){
               cout << endl << "Incorrect BC status" << endl;
               //exit(1);
              }
            }
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
    if(l0ver < 0xc0) l0 = new L0BOARD1(vspctp);
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


