#include <signal.h>
#include <iostream>
#include "Dim.h"
#include "MonScal.h"
//=========================================================================
bool forever=1;
void sighandler(int sig)
{
         if(sig == SIGABRT) cout<< "Signal SIGABRT caught..." << endl;
    else if(sig == SIGTERM) cout<< "Signal SIGTERM caught..." << endl;
    else if(sig == SIGINT ) cout<< "Signal SIGINT caught..." << endl;
    else cout << "Signal " << sig << " caught..." << endl;
    forever = false;
}
int main(int argc, char **argv) {
 //ActiveRun r(81197);
 //ActiveRun r(0);
 //r.PrintInputs();
 //r.PrintClusters();
 //r.PrintClasses();
 //return 0;
 if((argc != 2) && (argc !=3 )){
  cout << "Expected 2 or 3 arguments:" << endl;
  cout << "1st arg: bitwise: 1 = Display; 2 = DAQlogbook; 4 = Counters" << endl;
  cout << "2nd arg: 1=copy counters to DCS, 0=do not copy; default=0" << endl;
  return(1);
 }
 int copy2dcs=0;
 if(argc == 3) copy2dcs = atoi(argv[2]);
 int output = atoi(argv[1]);
 cout << "output=" << output << " copy2dcs= "<< copy2dcs <<  endl;
 ErrorHandler errHandler;
 VALIDLTUS ltus;
 ltus.readVALIDLTUS();
 //ltus.Print();
 //return 0;
 OpenDim monscal(output,copy2dcs);
 //OpenDim monscal(1);
 signal(SIGABRT, &sighandler);
 signal(SIGTERM, &sighandler);
 signal(SIGINT, &sighandler);
 signal(SIGKILL, &sighandler);
 while(forever) {
   pause();
 };
 return(0);
} 
