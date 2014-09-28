#include "FOBOARD.h"
FOBOARD::FOBOARD(w32 bashex,int vsp)
:
	BOARD("fo",bashex,vsp,6)
{
 this->AddSSMmode("inmonl0",0); 
 this->AddSSMmode("inmonl1",1); 
 this->AddSSMmode("inmonl2",2); 
 this->AddSSMmode("igl0l1",3); 
 this->AddSSMmode("igl2",4); 
 this->AddSSMmode("outgen",5);
 this->SetNumofCounters(72); 
}
FOBOARD::FOBOARD(w32 bashex,int vsp,string const name)
:
	BOARD(name,bashex,vsp,6)
{
 this->AddSSMmode("inmonl0",0); 
 this->AddSSMmode("inmonl1",1); 
 this->AddSSMmode("inmonl2",2); 
 this->AddSSMmode("igl0l1",3); 
 this->AddSSMmode("igl2",4); 
 this->AddSSMmode("outgen",5);
 this->SetNumofCounters(72); 
}
//-------------------------------------------------------------------------------
// All counters on FO should be 0 when no triggers
//
int FOBOARD::CheckCountersNoTriggers()
{
 int ret=0;
 for(int i=1;i<NCountersfromcnames;i++){
    if(countdiff[i] != 0){
      printf("Counter %s != 0 : %u \n",CounterNames[i].c_str(),countdiff[i]);
      ret=1;
    }
 }
 if(ret==0)printf("%s CheckCountersNoTriggers: NO ERROR detected. \n",getName().c_str());
 return ret;
}
//-----------------------------------------------------------------------------
void FOBOARD::SetFile(string const &modename) 
{
 // this should be done via $VME???
 //string name="CFG/ctp/ssmsigs/"+d_name+"_"+modename+".sig";
 string name="../CFG/ctp/ssmsigs/"+d_name.substr(0,2)+"_"+modename+".sig";
 //cout << "Mode file:"<< name << endl;
 modefile.open(name.c_str());
 if(!modefile.is_open()){
  cout << "File "<< name << " cannot be opened, exiting." << endl;
  exit(1);
 }else{
  //cout << "File "<< name << " opened successfully" << endl;
 }

}

void FOBOARD::AnalSSMinmonl2()
{
 
}

