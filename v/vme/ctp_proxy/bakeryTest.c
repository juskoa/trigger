#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vmewrap.h"
#include "ctp.h"
#include "vmeblib.h"
#define DBMAIN
#include "Tpartition.h"
//#include "shmaccess.h"
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

int main(int argc, char **argv) {
char *eof; 
int timeout=0;
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
  printf("Give 1 resource (swtriggers ccread or ssmcr) as parameter to control it, current status:\n");
  printBakery(&ctpshmbase->swtriggers);
  printBakery(&ctpshmbase->ccread);
  printBakery(&ctpshmbase->ssmcr);
  return(8);
};
while(1) {
  printf("Resource:%s. l cust   u cust   t[imeout] secs h[elp] p[rint] q[uit]:", resource);
  eof=fgets(line, 100, stdin);
  if(eof==NULL) break;
  if(line[0]=='q') break;
  if(line[0]=='l') {
    customer= atoi(&line[2]);
    if(timeout ==0) {
      lockBakery(res, customer);
    } else {
      int rc;
      rc= lockBakeryTimeout(res, customer, timeout);
      printf("rc:%d (1:locked 0:timeout,i.e. not locked) after %ds timeout\n", rc, timeout);
    };
  } else if(line[0]=='u') {
    customer= atoi(&line[2]);
    unlockBakery(res, customer);
  } else if(line[0]=='t') {
    timeout= atoi(&line[2]);
  } else if(line[0]=='p') {
    printf("Timeout:%ds\n", timeout);
    printBakery(res);
  } else if(line[0]=='h') {
printf("l[ock] u[nlock] p[rint].\n\
Now l/u/p commands work on %s%s%s resource. bakery algorithm used for\n\
3 groups of tasks (see vmeblib/bakery.h, DOC/ssmsync):\n\
initBakery(swtriggers,4): 0:SOD/EOD 1:gcalib 2:ctp.exe 3:dims\n\
initBakery(ccread,6): 0:proxy 1:dims 2:ctp+busytool 3:smaq 4:inputs 5:orbitddl2\n\
initBakery(ssmcr,4): 0:smaq 1:orbitddl2 2:ctp+busytool 3:inputs\n\
\n\
t[imeout] N  -set timeout to N seconds. If not 0, l[ock] command will return\n\
after N seconds, abandoning the wait for the resource. Current timeout is set to: %d\n\
", KRED,resource,KNRM, timeout);
  } else {
    continue;
  };
};
cshmDetach();
}

