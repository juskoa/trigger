#include "DETECTOR.h"
DETECTOR::DETECTOR()
{
 name="None";
 numname=100;
 ltunum=-1;
 ltuvmeaddhex=0;
 ltuvmeaddress="None";
 fo=100;
 focon=100;
 dimserver="None";
 ltu=0;
}
void DETECTOR::print()
{
   cout << name << " " << numname;
   if(ltuvmeaddress != "None") cout << " vme:"<< ltuvmeaddress;
   else if(dimserver != "None")cout << " dim:" << dimserver;
   else{
    cout << endl; 
    cout << "Warning: ltuvmeaddress and dimserver = None" << endl;
   }
 cout << " fo:"<< fo;
 cout << " con:"<< focon; 
 cout << endl;
}

