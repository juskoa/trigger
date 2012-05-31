/*
CC=g++
CFLAGS=-I/opt/dip/include
dump_bc: dump_bc.cpp /opt/dip/lib/libdip.so
        $(CC) $(CFLAGS) dump_bc.cpp -L/opt/dip/lib -ldip -ldl -o linux/dump_bc
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

  public:
    GeneralDataListener(Client *c):client(c) {}

    void handleMessage(DipSubscription *subscription, DipData &message) {

      const DipInt *bc = 0x0;
      int size = 0;

      try {
//	cout << "Received data from " << subscription->getTopicName() << endl;
//	int noFields;
//	const char **tags = message.getTags(noFields);
//	for (int i = 0; i < noFields; i++) {
//	  cout << "Tag " << *tags << " is of type " << message.getValueType(*tags) 
//	       << " and has dimension " << message.getValueDimension(*tags) << endl;
//	  tags++;
//	}
	bc = message.extractIntArray(size, "value");
      }
      catch (DipException e) {
	cout << "Unexpected exception occured: " << e.what() << endl;
	return;
      }

      string name = "";
      int offset = 0;
      if (subscription == client->sub[0]) {
	name = "Beam 1";
	offset = 346;
      }
      else {
	name = "Beam 2";
	offset = 3019;
      }

      if (bc) {
	cout << name << endl;
	cout << "bucket bunch pretrigger" << endl;
	cout << "-----------------------" << endl;
	for (int i = 0; i < 2808; i++) {
	  if (bc[i] == 0)
	    break;
	  cout << setw(7) << bc[i] << " " 
	       << setw(7) << (bc[i]/10 + offset) % 3564 << " " 
	       << setw(7) << (bc[i]/10 + offset - 42) % 3564 << endl;
	}
	cout << endl;
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
  
  Client()
  {
    string clientName = "client_trd"; 
    
    dip = Dip::create(clientName.c_str());

    sub = new DipSubscription*[2];

    handler = new GeneralDataListener(this);
    
    sub[0] = dip->createDipSubscription("dip/acc/LHC/RunControl/CirculatingBunchConfig/Beam1", handler);
    sub[1] = dip->createDipSubscription("dip/acc/LHC/RunControl/CirculatingBunchConfig/Beam2", handler);
  }
  
  ~Client(){
    dip->destroyDipSubscription(sub[0]);
    dip->destroyDipSubscription(sub[1]);
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

