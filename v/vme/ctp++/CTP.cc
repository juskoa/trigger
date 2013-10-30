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
 //cout<< "~CTP: " << boards.size() << endl;
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
    string name = "ltu";
    //name=name+int2char(numofltus+1);
    string base=line.substr(line.length()-8,line.length());
    w32 basehex;
    convertS2H(basehex,base);
    ltu[numofltus] = new BOARD(name,basehex,vspltu,1);
    vspltu=ltu[numofltus]->getvsp();  
    ltu[numofltus]->AddSSMmode("inmon",0); 
    boards.push_back(ltu[numofltus]);    
    numofltus++; 
 }else if(!line.compare(line.length()-8,4,"0x80")){
    //cout << "ttc found " << endl;
 }else if(!line.compare(0,4,"busy")){
    //cout << " busy board found " << endl;
    busy = new BOARD("busy",0x828000,vspctp,4);
    vspctp=busy->getvsp();
    busy->AddSSMmode("inmon",0);
    busy->AddSSMmode("outmon",1);
    busy->AddSSMmode("ingen",2);
    busy->AddSSMmode("outgen",3);
    boards.push_back(busy);    
 }else if(!line.compare(0,2,"l0")){
    //cout << " l0 board found " << endl;
    l0 = new BOARD("l0",0x829000,vspctp,4);
    vspctp=l0->getvsp();
    l0->AddSSMmode("inmon",0); 
    l0->AddSSMmode("outmon",1); 
    l0->AddSSMmode("ingen",2); 
    l0->AddSSMmode("outgen",3); 
    boards.push_back(l0);    
 }else if(!line.compare(0,2,"l1")){
    //cout << " l1 board found " << endl;
    l1 = new BOARD("l1",0x82a000,vspctp,4);
    vspctp=l1->getvsp();
    l1->AddSSMmode("inmon",0); 
    l1->AddSSMmode("outmon",1); 
    l1->AddSSMmode("ingen",2); 
    l1->AddSSMmode("outgen",3); 
    boards.push_back(l1);    
 }else if(!line.compare(0,2,"l2")){
    //cout << " l2 board found " << endl;
    l2 = new BOARD("l2",0x82b000,vspctp,5);
    vspctp=l2->getvsp();
    l2->AddSSMmode("inmon",0); 
    l2->AddSSMmode("outmon",1); 
    l2->AddSSMmode("ingen",2); 
    l2->AddSSMmode("outgen",3); 
    l2->AddSSMmode("pf",4); 
    boards.push_back(l2);
 }else if(!line.compare(0,3,"int")){
    inter = new BOARD("int",0x82c000,vspctp,4);
    vspctp=inter->getvsp();
    inter->AddSSMmode("ddldat",0); 
    inter->AddSSMmode("ddllog",1); 
    inter->AddSSMmode("i2c",2); 
    inter->AddSSMmode("inmon",3);     
    boards.push_back(inter); 
 }else  if(!line.compare(0,2,"fo")){
    //cout << "fo found "<< endl;
    const char *fonum=&line[7];
    int num=char2int(*fonum)-1;   //-1 since fo are now from 1
    string name= "fo";
    string base=line.substr(line.length()-8,line.length());
    w32 basehex;
    convertS2H(basehex,base);
    fo[num] = new BOARD(name,basehex,vspctp,6);
    vspctp=fo[num]->getvsp();
    fo[num]->AddSSMmode("inmonl0",0); 
    fo[num]->AddSSMmode("inmonl1",1); 
    fo[num]->AddSSMmode("inmonl2",2); 
    fo[num]->AddSSMmode("igl0l1",3); 
    fo[num]->AddSSMmode("igl2",4); 
    fo[num]->AddSSMmode("outgen",5); 
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
 string const DBfile = "../CFG/ctp/DB/VALID.LTUS";
 file.open(DBfile.c_str());
  if(! file){
  cout << "File "<< DBfile  << " cannot be opened, exiting." << endl;
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
  if((ltu[i]->getboardbase()) == det->ltuvmeaddhex)det->ltunum=i;
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


