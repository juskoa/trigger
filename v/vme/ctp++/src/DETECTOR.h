#ifndef _DETECTOR_h
#define _DETECTOR_h
#include <stdlib.h>
#include "libctp++.h"
using namespace std;
class DETECTOR
{
 public:
     DETECTOR();
     string name;
     int numname; // used by DAQ/ECS
     int ltunum;  // ltunumber
     w32 ltuvmeaddhex;
     string ltuvmeaddress;
     int fo,focon;
     string dimserver;
     void print();
};
#endif