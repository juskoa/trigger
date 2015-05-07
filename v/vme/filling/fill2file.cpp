/*
http://j2eeps.cern.ch/wikis/display/EN/DIP+and+DIM

Start browser in the pit:
[alidcscom026] export CLASSPATH=/opt/dip/lib/dip.jar
[alidcscom026] /home/trigger > java -jar /opt/dip/tools/dipBrowser.jar
use defaultDNS


name field value
"dip/acc/LHC/RunControl/InjectionBunchConfig/Beam1" value [1,1001,...]
int[288]
"dip/acc/LHC/RunControl/CirculatingBunchConfig/Beam1" value [1,1001,...]
int[2808] -probably this one -grows during 'INJECTION PHYSICS BEAM' mode:
1,1061,1261,...,0,0

"dip/acc/LHC/RunControl/FillNumber" value "1226"

"dip/acc/LHC/RunControl/MachineMode" value "PROTON PHYSICS"/"ION PHYSICS"
"dip/acc/LHC/RunControl/BeamMode" value "STABLE BEAMS"

"dip/acc/LHC/RunControl/RunConfiguration"
FILL_NO "1226"
ACTIVE_INJECTION_SCHEME "Single_13b_8_8_8"

Seems BEAM
*/
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "Dip.h"
#include "DipSubscription.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>   // ofstrem
#include <iomanip>
#include <sstream>
#include <boost/algorithm/string.hpp>

#define offsetA_run1 346
#define offsetC_run1 3019
#define offsetA 344
#define offsetC 3017

using namespace std;

class Client {
public:
  DipSubscription **sub;
  DipFactory *dip;
  class GeneralDataListener:public DipSubscriptionListener{
  private:
    // allow us to access subscription objects
    Client * client;
    string fillsch;
    string beammode;
    int fillno; int bunches; int beams;
    ofstream fs_file;
    int wlines;
  public:
    GeneralDataListener(Client *c):client(c) {
      beams=0; beammode="";
    }
    ~GeneralDataListener() {
      //cout << "-------GeneralDataListener destructor" <<endl;
      if(fs_file.is_open()) { 
        fs_file.close(); 
        //cout << "closed" << endl;
      };
    }
    void handleMessage(DipSubscription *subscription, DipData &message) {
      const DipInt *bc = 0x0;
      int offset = 0; 
      int size = 0;
      string item;
      string beamname = "";
      //cout << "-----------------------handleMessage" << endl;
      try {
       if (subscription == client->sub[0]) {   //RunConfiguration
         //cout << "Received data from " << subscription->getTopicName() << endl;
         int noFields;
         const char **tags = message.getTags(noFields);
         for (int i = 0; i < noFields; i++) {
           item= message.extractString(*tags);
           if(strcmp(*tags,"ACTIVE_INJECTION_SCHEME")==0) { 
           //if(strcmp(*tags,"ACTIVE_FILLING_SCHEME")==0) { // changed 24.4.2015, but returns ""
             fillsch= item; //cout << "ACTIVE_INJECTION_SCHEME " << fillsch << endl;
             boost::erase_all(fillsch, " ");
             // following does not work properly:
             //fillsch.replace(fillsch.begin(), fillsch.end(), ' ', '_');
           };
           if(strcmp(*tags,"FILL_NO")==0) {
             fillno= atoi(item.c_str());  //cout << "FILL_NO " << item << endl;
           };
           if(strcmp(*tags,"NO_BUNCHES")==0) {
             bunches= atoi(item.c_str()) ; //cout << "bunches " << item << endl;
           };
           /*cout << "Tag " << *tags << " type " << message.getValueType(*tags) 
           << " dim: " << message.getValueDimension(*tags) << " :" << item << endl; */
           tags++;
         }
       } else if (subscription == client->sub[1]) {   // Beam1
         bc = message.extractIntArray(size, "value");
         beamname = "A"; offset = offsetA; beams++;
       } else if (subscription == client->sub[2]) {   // Beam2
         bc = message.extractIntArray(size, "value");
         beamname = "C"; offset = offsetC; beams++;
       } else if (subscription == client->sub[3]) {   // BeamMode
         int noFields;
         const char **tags = message.getTags(noFields);
         for (int i = 0; i < noFields; i++) {
           item= message.extractString(*tags);
           if(strcmp(*tags,"value")==0) {
             beammode= item;
           };
         };
       } else {   // not expected
         cout << "Unexpected subscription, ignored" << endl;
       }
      } catch (DipException e) {
	cout << "Unexpected exception occured: " << e.what() << endl;
	return;
      }
      if (bc) {
        //string fname; 
        char fname[80]; 
        //fname= fillsch; fname.append(".dip");
        if(not fs_file.is_open()) {
          strcpy(fname, (char *)(fillsch.c_str())); strcat(fname,".dip");
          cout << "opening " <<  fname << endl;
          fs_file.open(fname, ios::out|ios::trunc);
          wlines=0;
        };
        //wlines=0;
	for (int i = 0; i < 2808; i++) {
	  if (bc[i] == 0) break;
	  //fs_file << beamname << " " << setw(7) << bc[i] << " " << setw(7) << (bc[i]/10 + offset) % 3564 << endl;
	  fs_file << beamname << " " << bc[i] << endl;
          wlines++;
	}
        //cout << "written:" << wlines <<" " << fillsch << " " << fillno <<" " << beamname << " " << size << endl;
        /* stdout line:
        When fill2file invoked in correct time (see bemmodeok() ):
        fs shema_name fill_number DIPbunches Written_bunches(lines_in_dip_file)
        When fill2file invoked in bad time:
        fs badtime
        */
        if(beams==2) {
          if(beammodeok()==1) {
            cout << "fs "<< fillsch <<" " << fillno <<" " << bunches <<" " << wlines << endl;
          } else {
            cout << "fs "<< "badtime:" << beammode << endl;
          };
        };
      }
    }
    int beammodeok() {
      // remove following line when LHC testing finished:
      cout << "beammodeok:" << beammode << ": ok (not checked)" << endl;
      return(1);
      if((beammode == "INJECTION PHYSICS BEAM")) return(1);   // just for testing in April 2015
      if((beammode == "PREPARE RAMP") or (beammode == "RAMP") or
         (beammode == "FLAT TOP") or (beammode == "SQUEEZE") or
         (beammode == "ADJUST") or (beammode == "STABLE BEAMS")) {
        return(1);
      } else {
        return(0);
      };
    }
    void connected(DipSubscription *arg0) { }
    void disconnected(DipSubscription *arg0, char *arg1) { }
    void handleException(DipSubscription* subscription, DipException& ex){
      printf("Subs %s has error %s\n", subscription->getTopicName(), ex.what());
    }
  };
  GeneralDataListener *handler;
public:
  Client() {
    string clientName = "client_ctp"; 
    cout << "Client Dip::create... " <<  clientName << endl;
    dip = Dip::create(clientName.c_str());
    cout << "Client new DipSubscription..." << endl;
    sub = new DipSubscription*[4];
    cout << "Client new GeneralDataListener..." << endl;
    handler = new GeneralDataListener(this);
    sub[3] = dip->createDipSubscription("dip/acc/LHC/RunControl/BeamMode", handler);
    sub[0] = dip->createDipSubscription("dip/acc/LHC/RunControl/RunConfiguration", handler);
    sub[1] = dip->createDipSubscription("dip/acc/LHC/RunControl/CirculatingBunchConfig/Beam1", handler);
    sub[2] = dip->createDipSubscription("dip/acc/LHC/RunControl/CirculatingBunchConfig/Beam2", handler);
    //sub[2] = dip->createDipSubscription("dip/acc/LHC/RunControl/RunConfiguration/ACTIVE_INJECTION_SCHEME", handler);
  }
  ~Client(){
    dip->destroyDipSubscription(sub[0]);
    dip->destroyDipSubscription(sub[1]);
    dip->destroyDipSubscription(sub[2]);
    dip->destroyDipSubscription(sub[3]);
    delete handler;
    delete dip;
  }
  friend class GeneralDataListener; //Uncomment if you need to allow access.
};


int main(const int argc, const char ** argv){
Client * theClient = new Client();
sleep(1); 
delete theClient;
return(0);
}

