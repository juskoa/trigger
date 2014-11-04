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
 char *environ;
 environ= getenv("VMECFDIR"); 
 string cfgdir(environ);
 string name=cfgdir+"/CFG/ctp/ssmsigs/"+d_name.substr(0,2)+"_"+modename+".sig";
 //cout << "Mode file:"<< name << endl;
 modefile.open(name.c_str());
 if(!modefile.is_open()){
  cout << "File "<< name << " cannot be opened, exiting." << endl;
  exit(1);
 }else{
  //cout << "File "<< name << " opened successfully" << endl;
 }

}
int FOBOARD::L2DataOut(char focon)
{
 int rc=0;
 w32 sl2strobech,sdatach;
 string l2strobename("l2strobe[");
 l2strobename = l2strobename+focon+"]";
 if((sl2strobech=getChannel(l2strobename))>32)rc=1;
 string l2dataname("l2data[");
 l2dataname = l2dataname+focon+"]";
 if((sdatach=getChannel(l2dataname))>32) rc=1;
 if(rc){
   printf("Error in FOBOARD::L2DataOut: channels not found.\n");
   return 1;
 }
 printf("L2DataOut: %i %i \n",sl2strobech,sdatach);
 return 0;
}
int FOBOARD::AnalSSMinmonl1()
{
 return 0;
}

int FOBOARD::AnalSSMinmonl2()
{
 L2DataBackplane();
 printL2DataBackplane();
 //L2DataOut('4');
 return 0;
}

