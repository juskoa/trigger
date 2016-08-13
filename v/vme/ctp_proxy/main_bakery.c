#include <stdio.h>
//#include <stdlib.h>

#include "vmeblib.h"
#include "ctp.h"
#define DBMAIN
#include "Tpartition.h"
int main(int argc, char **argv)  {
int rc=0, customer; char msg[200];
if(argc>1) {
  /*if(strcmp(argv[1],"no1min")==0) {
    dimsflags=dimsflags | NO1MINFLAG;
  };*/
  customer= atoi(argv[1];
  printf("customer: %s = %d\n", argv[1], customer);
};

cshmInit(); cshmPrint();
return(rc);
}
