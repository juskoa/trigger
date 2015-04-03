#include <iostream>
#include "Dim.h"
#include <sstream>
#include "ctpcounters.h"

using namespace std;
ErrorHandler::ErrorHandler()
:
Log(),
DimErrorHandler()
{
DimClient::addErrorHandler(this);
}

void ErrorHandler::errorHandler(int severity, int code, char *msg) {
    int index = 0;
    char **services;
    stringstream ss;
    ss << "Error, " << severity << " " << msg << endl;
    services = DimClient::getServerServices();
    ss << "from "<< DimClient::getServerName() << " services:" << endl;
    while(services[index]) {
      ss << services[index] << endl;
      index++;
    }
    ss << endl << "Exiting !" << endl;
    PrintLog(ss.str().c_str());
    exit(1);
}
OpenDim::OpenDim(w32 kPrint,bool copy2dcs)
:
MonScal(kPrint,copy2dcs),
DimInfo("CTPDIM/MONCOUNTERS",-1),
count(0)
{
}
void OpenDim::infoHandler()
{
    int size=getSize();
    if(size != 4*NCOUNTERS) {
      cout << "error in CTPCounters. size=" << size<<" instead of " << 4*NCOUNTERS << endl;
      return;
    };
    //cout << "Handler called "  << count++ << endl;
    SetBuffer((w32 *)getData());
    GetActiveRuns();
}

