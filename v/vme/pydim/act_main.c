#include <string.h>
#include <stdio.h>
#include "vmeblib.h"

int actdb_getInstances(char *itemname);
int actdb_getPartition(char *name, char *filterpar, char *a, char *b);

int main(int argc, char **argv) {
int grc=0;
if(argc>1) {
  if(strcmp(argv[1], "p")==0) {
  char filter[2000]; char instance[300]; char version[30];
  grc= actdb_getPartition("PHYSICS_1", filter, instance, version);
  printf("filter %d:\n%s\n", (int)strlen(filter),filter);
  };
} else {
  //grc= actdb_getInstances("/CTP/ctp.cfg");
  grc= actdb_getInstances("/CTP");
};
return(grc);
}
