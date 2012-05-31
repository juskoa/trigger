/*
http://j2eeps.cern.ch/wikis/display/EN/DIP+and+DIM

Start browser in the pit:
[alidcscom026] export CLASSPATH=/opt/dip/lib/dip.jar
[alidcscom026] /home/trigger > java -jar /opt/dip/tools/dipBrowser.jar
use defaultDNS


name field value
"dip/acc/LHC/RunControl/InjectionBunchConfig/Beam1" value 
int[288]
31181,31201,31221,31241,31261,31281,...

"dip/acc/LHC/RunControl/CirculatingBunchConfig/Beam1" value
int[2808] -probably this one -grows during 'INJECTION PHYSICS BEAM' mode:
1,381,401,421,441,461,481,501,...,0,0
- filled after INJECTION PHYSICS BEAM (i.e. in PREPARE RAMP). Seems pilot is missing
at that time:
[alidcscom188] /data/dl/root/usr/local/trigger/v/vme/filling > ./linux/fill2file
opening 50ns_912b+1small_874_20_864_108bpi11inj.dip
fs 50ns_912b+1small_874_20_864_108bpi11inj 1805 913 1824
[alidcscom188] /data/dl/root/usr/local/trigger/v/vme/filling > diff 50ns_912b+1small_874_20_864_108bpi11inj.dip $dbctp/fs_auto/50ns_912b+1small_874_20_864_108bpi11inj.dip
0a1
> A 1
912a914
> C 1

"dip/acc/LHC/RunControl/FillNumber" value "1226"

"dip/acc/LHC/RunControl/MachineMode" value "PROTON PHYSICS"/"ION PHYSICS"
"dip/acc/LHC/RunControl/BeamMode" value "STABLE BEAMS"

"dip/acc/LHC/RunControl/RunConfiguration"
FILL_NO "1226"
ACTIVE_INJECTION_SCHEME "Single_13b_8_8_8"
IP2-NO-COLLISIONS "20"
*/
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "Dip.h"
#include "DipSubscription.h"
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>   // ofstrem
#include <iomanip>
#include <sstream>

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
    int fillno; int bunches; int beams;
    ofstream fs_file;
    int wlines;
  public:
    GeneralDataListener(Client *c):client(c) {
      beams=0; 
    }
    ~GeneralDataListener() {
      cout << "-------GeneralDataListener destructor" <<endl;
      if(fs_file.is_open()) { 
        fs_file.close(); 
        cout << "closed" << endl;
      };
    }
    void handleMessage(DipSubscription *subscription, DipData &message) {
      const DipInt *bc = 0x0;
      int offset = 0; 
      int size = 0;
      string item;
      string beamname = "";
      cout << "-----------------------handleMessage" << endl;
      try {
       if (subscription == client->sub[0]) {
         //cout << "Received data from " << subscription->getTopicName() << endl;
         int noFields;
         const char **tags = message.getTags(noFields);
         for (int i = 0; i < noFields; i++) {
           item= message.extractString(*tags);
           if(strcmp(*tags,"ACTIVE_INJECTION_SCHEME")==0) {
             fillsch= item; cout << "fs " << fillsch << endl;
           };
           if(strcmp(*tags,"FILL_NO")==0) {
             fillno= atoi(item.c_str());  cout << "FILL_NO " << item << endl;
           };
           if(strcmp(*tags,"NO_BUNCHES")==0) {
             bunches= atoi(item.c_str()) ; cout << "bunches " << item << endl;
           };
/*           cout << "Tag " << *tags << " type " << message.getValueType(*tags) 
           << " dim: " << message.getValueDimension(*tags) << " :" << item << endl;*/
           tags++;
         }
       } else if (subscription == client->sub[1]) {
         bc = message.extractIntArray(size, "value");
         beamname = "A"; offset = 346; beams++;
       } else {
         bc = message.extractIntArray(size, "value");
         beamname = "C"; offset = 3019; beams++;
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
          cout << "opening " <<  fname << ":" << endl;
          fs_file.open(fname, ios::out|ios::trunc);
          wlines=0;
        };
        //wlines=0;
	for (int i = 0; i < 2808; i++) {
	  if (bc[i] == 0) break;
	  fs_file << beamname << " " 
	       << setw(7) << bc[i] << " " 
	       << setw(7) << (bc[i]/10 + offset) % 3564 << endl;
          wlines++;
	}
        cout << "written:" << wlines <<" " << fillsch << " " << fillno <<" " << beamname << " " << size << endl;
        if(beams==2) {
          cout << fillsch <<" " << fillno <<" " << bunches <<" " << wlines << endl;
        };
      }
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
    dip = Dip::create(clientName.c_str());
    sub = new DipSubscription*[3];
    handler = new GeneralDataListener(this);
    cout << "creating subsriptions..." << endl;
    sub[0] = dip->createDipSubscription("dip/acc/LHC/RunControl/RunConfiguration", handler);
    sub[1] = dip->createDipSubscription("dip/acc/LHC/RunControl/CirculatingBunchConfig/Beam1", handler);
    sub[2] = dip->createDipSubscription("dip/acc/LHC/RunControl/CirculatingBunchConfig/Beam2", handler);
    //sub[2] = dip->createDipSubscription("dip/acc/LHC/RunControl/RunConfiguration/ACTIVE_INJECTION_SCHEME", handler);
  }
  ~Client(){
    dip->destroyDipSubscription(sub[0]);
    dip->destroyDipSubscription(sub[1]);
    dip->destroyDipSubscription(sub[2]);
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

