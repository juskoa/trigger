/* load CTP config files or .partition file from ACT
rc: 0: ok
   !=0 not loaded
Usage:
Download ctp configs:
linux/act.exe                 -download ctp configs

Downlaod partition PHYSICS_*, TEST_*:
linux/act.exe PHYSICS_*

Print to stdout "VALUE anystring" where anystring is from ACT:
linux/act.exe VALUE name
example:
ctp_proxy > linux/act.exe VALUE /CTP/filter
INFO Opening ACT:daq:daq@pcald30/ACT
INFO ACT opened succesfuly.
VALUE abcd

Download (active instance of ctp config file:
linux/act.exe VALID.BCMASKS
*/
#include <stdio.h>
#include <stdlib.h>
#include "vmewrap.h"
#include "ctp.h"
#define DBMAIN
#include "Tpartition.h"

int actdb_getdbfiles();
int actdb_getdbfile_openclose(char *cfgname);
//int actdb_getPartition(char *partname, char *filter);
int actdb_getPartition(char *name, char *filterpar, char *actname, char *actversion);
int actdb_getdbstring(char *fn, int openclose, char *value, int maxl);

int checkproxy() { // done in ctpproxy.py
return(0);
}
/*----------------------------------------*/ int main(int argc, char **argv) {
int rc=0;
if(argc==1) { // get CTP db files
  // check if ctp_proxy is on:
  //cshmInit(); cannot be done through shm (we are not on alidcsvme001)
  if((rc=checkproxy())==0) {
    rc= actdb_getdbfiles();
    if(rc==0) {
      printf("CTP config files downloaded from ACT.\n");
    };
  } else {
    rc=8;
    printf("ctpproxy is active!, no action...\n"); rc=8;
  };
} else { // get .partition file
  if((strncmp(argv[1],"PHYSICS_",8)==0) || (strncmp(argv[1],"TEST_",5)==0)) {
    char filter[200]="nothing";
    char actinst[100], actver[100];
    rc= actdb_getPartition(argv[1], filter,actinst, actver);
    printf("part:%s INSTANCE:%s version:%s", argv[1], actinst,actver);
  } else if(strcmp(argv[1],"VALUE")==0) {
    char value[1000]="";
    if(argc==3) {
      rc= actdb_getdbstring(argv[2], 1, value, 1000);
      printf("VALUE %s",value);
    } else {
      rc=8;
    };
  } else {
    rc= actdb_getdbfile_openclose(argv[1]);
  };
};
exit(rc);
}

