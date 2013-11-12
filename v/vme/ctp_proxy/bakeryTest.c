#include <stdio.h>
#include <stdlib.h>
#include "vmewrap.h"
#include "ctp.h"
#define DBMAIN
#include "Tpartition.h"
//#include "shmaccess.h"

int main() {
char *eof; char line[100];
int customer;
cshmInit();
while(1) {
  printf("l cust   u cust   h[elp] p[rint] q[uit]:");
  eof=fgets(line, 100, stdin);
  if(eof==NULL) break;
  if(line[0]=='q') break;
  if(line[0]=='l') {
    customer= atoi(&line[2]);
    lockBakery(&ctpshmbase->swtriggers, customer);
  } else if(line[0]=='u') {
    customer= atoi(&line[2]);
    unlockBakery(&ctpshmbase->swtriggers, customer);
  } else if(line[0]=='p') {
    printBakery(&ctpshmbase->swtriggers);
  } else if(line[0]=='h') {
printf("Now l/u/p concerns swtriggers. bakery alg. used for\n\
2 groups of tasks (initialised in main_ctp.c):\n\
initBakery(swtriggers,4): 0:SOD/EOD 1:gcalib 2:ctp.exe 3:dims\n\
initBakery(ccread,5): 0:proxy 1:dims 2:ctp+busytool 3:smaq 4:inputs\n\
");
  } else {
    continue;
  };
};
cshmDetach();
}

