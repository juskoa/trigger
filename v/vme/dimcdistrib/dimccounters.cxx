/* dimccounters.cxx -an example of dim client reading CTP counters */
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
*/
#include <iostream>
using namespace std;
#include <dic.hxx>
#include "ctpcounters.h"
typedef unsigned int w32;

class ErrorHandler : public DimErrorHandler {
  void errorHandler(int severity, int code, char *msg) {
    int index = 0;
    char **services;
    cout << "Error, " << severity << " " << msg << endl;
    services = DimClient::getServerServices();
    cout<< "from "<< DimClient::getServerName() << " services:" << endl;
    while(services[index]) {
      cout << services[index] << endl;
      index++;
    }
  }
public:
ErrorHandler() {DimClient::addErrorHandler(this);}
};

class CTPcounters: public DimInfo {
typedef struct {
  int reladdr;   // rel. addr. of the counter
  w32 prevcs;    // previous value
  w32 currcs;    // current value
} Tcnt1;
const static int NCS=6;
Tcnt1 cs[NCS];   // watched counters
public:
  CTPcounters():DimInfo("CTPDIM/MONCOUNTERS",-1){
    cout << "CTPcounters init" << endl;
    initcntsstr();
  };
  void infoHandler() {
    //cout << "infohandler:" << getSize() << endl;
    int size=getSize();
    w32 *buffer= (w32 *)getData();
    //cout << "infohandler: data size:" << size << endl;
    gotcnts(buffer, &size);
  };

  w32 dodif32(w32 before, w32 now) {
  // Substract 2 32 bits values (representing counters)
  w32 dif;
  if(now >= before) dif= now-before;
  else dif= now+ (0xffffffff-before) +1;
  return(dif);
  }
  void initcntsstr() {
    cs[0].reladdr= CSTART_BUSY+39; // elapsed time for BUSY, L0,1,2,FO1, FO3
    cs[1].reladdr= CSTART_L0+13; 
    cs[2].reladdr= CSTART_L1+5; 
    cs[3].reladdr= CSTART_L2+5;
    cs[4].reladdr= CSTART_FO+0;
    cs[5].reladdr= CSTART_FO+2*NCOUNTERS_FO+0;
  }
  void gotcnts(w32 *buffer, int *size) {
    int ix;
    //printf("gotcnts size:%d\n", *size );
    if(*size != 4*NCOUNTERS) {
      printf("error in gotcnts. First word of message (if any):0x%x\n",
        buffer[0]);
      return;
    };
    printf(" addr 0x abs         diff\n");
    for(ix=0; ix<NCS; ix++) {
      cs[ix].currcs= buffer[cs[ix].reladdr];
      printf(" %3d %8x %10.4f\n", cs[ix].reladdr, cs[ix].currcs, 
        dodif32(cs[ix].prevcs, cs[ix].currcs)/2500000.);
      cs[ix].prevcs= cs[ix].currcs;
    };
    printf("\n");
  }
};

int main(int argc, char **argv) {
ErrorHandler errHandler;
CTPcounters Counters;
while(1) {
  pause();
};
return(0);
} 
