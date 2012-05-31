#include "CTP.h"
void FOgen(BOARD *fo,int i){
 cout << "Starting FO " << fo->getboardbase() << endl;
 fo->ssmtools.genSeq(i,0);
 fo->WritehwSSM();
 if(fo->SetMode("outgen",'c')) return;
 //fo->StartSSM();
}
void StartAll(CTP *ctp){
 for(int i=0;i<NUMOFFO;i++){
  if(ctp->fo[i])FOgen(ctp->fo[i],i+2);
 }
}
void StopAll(CTP *ctp){
 for(int i=0;i<NUMOFFO;i++)if(ctp->fo[i])ctp->fo[i]->StopSSM();
}
int main(){
 CTP ctp;
 char c=' ';
 while(c != 'e'){
  cin >> c;
  cout << c << endl;
  switch(c){
    case 's': StartAll(&ctp); break;
    case 'f': StopAll(&ctp);break;
  }
 }
}
