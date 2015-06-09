#include <string>   // system()
#include <fstream>
#include <iostream>
#include <stdlib.h>   // system()
using namespace std;

int main() {
   char cmd[256],msg[256];
   int rc=0; 
   int runnum=211738;
   string fileName;
   fileName="cnt/run211738.cnt";
   // copy counters to dcs
   sprintf(cmd,"./dcsFES_putData.sh %d GRP CTP_xcounters /home/tri/%s",runnum, fileName.c_str());
   cout << "Executing counters:" << endl;
   rc=system(cmd);
   sprintf(msg,"cmd:%s rc:%d  \n", cmd, rc);
   cout << msg << endl;
}
