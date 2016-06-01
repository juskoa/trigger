#include "stdio.h"      // printf
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//#include "vmewrap.h"
#include "vmeblib.h"
#include "daqlogbook.h"

#define MAXCSString 80000
char CSString[MAXCSString];

char *readCS() {
int rl,ix; char *rcstr= CSString;
char *environ; char csname[200];
environ= getenv("VMECFDIR"); strcpy(csname, environ);
strcat(csname, "/CFG/ctp/DB/COLLISIONS.SCHEDULE");
rl= readfile(csname, rcstr, MAXCSString);
if(rl>= MAXCSString) {
  printf("INFO readCS(): too long file\n");
  return(NULL);
};
if(rl==-1) {
  printf("INFO readCS(): cannot open %s file\n", csname);
  return(NULL);
};
for(ix=0; ix<rl; ix++) {
  if(rcstr[ix]=='\n') break;
  csname[ix]=rcstr[ix];
}; csname[ix]='\0';
printf("INFO readCS:%s\n", csname);
return(rcstr);
}

int main(int argc, char **argv) {
int rcdaq,runN;
if(argc<2) {
  printf("Usage:\n\
./daqlogbookTest runn \n\
");
  return(8);
};
rcdaq= daqlogbook_open();
if(rcdaq!=0) {
  printf("ERROR DAQlogbook_open failed:%d\n", rcdaq);
} else {
  runN=atoi(argv[1]);
  if(readCS()) {
    printf("daqlogbook_update_cs(%d, %80.80s ...)\n", runN, CSString);
    rcdaq= daqlogbook_update_cs(runN, CSString);
    printf("daqlogbook_update_cs(%d, ....) rc:%d\n", runN, rcdaq);
  };
  rcdaq= daqlogbook_close();
};
}

