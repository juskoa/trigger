#include "TTCITBOARD.h"
#include "libctp++.h"
#include <string>
void PrintHelp()
{
 printf("Usage:\n");
 printf("ttcitRun3.e BoardVMeAddress\n");
 printf("ttcitRun3.e BoardVmeAddress Nperiod\n");
 printf("ttcitRun3.e BoardVmeAddress Nperiod Nssm\n");
 printf("Everything else is error.\n");
 printf("BoardVmeAddress - board VME address as hex number.\n");
 printf("Nperiod=10001 (default) - period of BC generated\n");
 printf("Nssm=10 (default) - number of ssm to take.\n");
// add also distance
}
int main(int argc, char *argv[]){
 uint boardvmeadd=0x8a0000;
 int Nperiod = 10000;
 int NN=10;
 if(argc==1)
 {
	PrintHelp();
	return 0;
 }
 else if(argc==2)
 {
	std::string sadd=argv[1];
	boardvmeadd=std::stoi(sadd,0,16);
	
 }
 else if(argc==3)
 {
	std::string sadd=argv[1];
	boardvmeadd=std::stoi(sadd,0,16);
	std::string speriod=argv[2];
	Nperiod = std::stoi(speriod);

 }
 else if(argc==4)
 {
	std::string sadd=argv[1];
	boardvmeadd=std::stoi(sadd,0,16);
	std::string speriod=argv[2];
	Nperiod = std::stoi(speriod);
	std::string snssm=argv[3];
	NN = std::stoi(snssm);
 }
 else
 {
	PrintHelp();
	return 0;
 } 
 Nperiod++;
 printf("Board: 0x%x: period: %i Nssm: %i\n",boardvmeadd,Nperiod,NN);
 //w32 err=0;
 int vmesp=-1;
 string boardname("ttcit");
 TTCITBOARD *ttc= new TTCITBOARD(boardname.c_str(),boardvmeadd,vmesp);
 printf("vsp= %i \n",ttc->getvsp());
 w32 ver= ttc->getFPGAversion();
 printf("Version: 0x%x %i\n",ver,ver);
 ttc->setNperiod(Nperiod);
 //int nerrorrs=0;
 for(int i=0;i<NN;i++){
   printf("--------------------> i= %i \n",i);
   ttc->start_stopSSM();
   ttc->Dump2quSSM();
   int ret=ttc->AnalyseSSMRun3();
   //ttc->DumptxtSSM();
   if(ret){
     char time[30];
     getdatetime(time);
     printf("Time: %s \n",time);
     ttc->DumptxtSSM();     
     if(ret != 2)return 1;
   }
   ttc->ClearQueues();
   fflush(stdout);
 }
 return 0;
}
