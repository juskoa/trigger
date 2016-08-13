/* an example of possible C++ wrapper for bakey.[ch].
Compilation: uncomment/comment bakeryTest.cc line in make_new and run make
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//extern "C" {
#include "vmewrap.h"
#include "ctp.h"
#include "vmeblib.h"
#define DBMAIN
#include "Tpartition.h"
//}
class BAORD {
  public:
    BAORD();
    void mylock(Tbakery *bakery, int customer);
    void myunlock(Tbakery *bakery, int customer);
    void print_lockunlock(Tbakery *bakery);
};

BAORD::BAORD() {
  // following 3 lines to be executed when orbitddl2 started (not necesarily
  // in BAORD's constructor)
  cshmInit();
  unlockBakery(&ctpshmbase->ssmcr,ssmcr_orbitddl2);
  unlockBakery(&ctpshmbase->ccread,ccread_orbitddl2);
};
void BAORD::mylock(Tbakery *bakery, int customer) {
  lockBakery(bakery, customer);
};
void BAORD::myunlock(Tbakery *bakery, int customer) {
  unlockBakery(bakery, customer);
};
void BAORD::print_lockunlock(Tbakery *bakery) {
  printBakery(bakery);
};

int main(int argc, char **argv) {
char *eof; 
int customer; Tbakery *res;
char line[100]; char resource[16];
//cshmInit();
BAORD board;
board.mylock(&ctpshmbase->ssmcr, ssmcr_orbitddl2);
board.print_lockunlock(&ctpshmbase->ssmcr);
board.myunlock(&ctpshmbase->ssmcr, ssmcr_orbitddl2);
printf("\n\n now C code (from bakeryTest.c)...\n");
if(isArg(argc, argv, "swtriggers")) {
  strcpy(resource, "swtriggers");
  res= &ctpshmbase->swtriggers;
} else if(isArg(argc, argv, "ccread")) {
  strcpy(resource, "ccread");
  res= &ctpshmbase->ccread;
} else if(isArg(argc, argv, "ssmcr")) {
  strcpy(resource, "ssmcr");
  res= &ctpshmbase->ssmcr;
} else {
  printf("Give 1 resource (swtriggers ccread or ssmcr) as parameter\n");
  printBakery(&ctpshmbase->swtriggers);
  printBakery(&ctpshmbase->ccread);
  printBakery(&ctpshmbase->ssmcr);
  return(8);
};
while(1) {
  printf("Resource:%s. l cust   u cust   h[elp] p[rint] q[uit]:", resource);
  eof=fgets(line, 100, stdin);
  if(eof==NULL) break;
  if(line[0]=='q') break;
  if(line[0]=='l') {
    customer= atoi(&line[2]);
    lockBakery(res, customer);
  } else if(line[0]=='u') {
    customer= atoi(&line[2]);
    unlockBakery(res, customer);
  } else if(line[0]=='p') {
    printBakery(res);
  } else if(line[0]=='h') {
printf("l[ock] u[nlock] p[rint].\n\
Now l/u/p concerns swtriggers. bakery alg. used for\n\
2 groups of tasks (initialised in main_ctp.c, defined in baklery.h):\n\
initBakery(swtriggers,4): 0:SOD/EOD 1:gcalib 2:ctp.exe 3:dims\n\
initBakery(ccread,5): 0:proxy 1:dims 2:ctp+busytool 3:smaq 4:inputs\n\
");
  } else {
    continue;
  };
};
cshmDetach();
}

