#include "extract.h"
//-------------------------------------------------------------------
int main(int argc, char **argv) {
 if(argc != 2){
  cout << "One argument expected." << endl;
  return 1;
 }
 cout << argv[1] << endl;
 extractData a(argv[1]);
 //a.SetSkipNext(12);  // 0 for autocorrelations
 //a.SetNfiles(10);
 if(a.readFileList()) return 1;
 a.chooseInputs();
 //a.checkAllSSM();
 //a.checkAllSSM1by1();
 //return 1;
 //
 a.removeEmptySSMs(0);
 a.extractAllSSM();
 //a.printData();
 a.correlateAllSSM(0,16);
 a.distance2Orbit();// set in config.cfg
 a.printCorrelations();
 a.printDistance();
 a.printAllHists();
 a.writeHists();
 cout << "Finished----------------------------------------" << endl;
}
