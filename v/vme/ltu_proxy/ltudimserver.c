/* ltudimserver.c -main for testing ltudimservices.c
(normally, ltudimservices.c is linked together with ltu_proxy) */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "vmewrap.h"
#define LTUMAIN
#include "ltu.h"
int ds_register(char *detname, char *base);

int main(int argc, char **argv)  {
int rc;
if(argc<3) {
  printf("Usage: ltuserver LTU_name base\n\
where LTU_name is detector name\n\
      base is the base address of LTU (e.g. 0x811000)\n\
"); exit(8);
};

rc= ds_register(argv[1], argv[2]);
if(rc!=0) exit(rc);
while(1)  {  
  /*printf("sleeping 10secs...\n");*/
  sleep(10);  
};  
} 

