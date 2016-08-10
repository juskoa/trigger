#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vmewrap.h"
#include "ctp.h"
#include "vmeblib.h"
#define DBMAIN
#include "Tpartition.h"
//#include "shmaccess.h"

int main(int argc, char **argv) {
char *eof; 
int customer; Tbakery *res;
char line[100]; char resource[16];
cshmInit();
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

