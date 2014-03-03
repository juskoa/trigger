/* load CTP config files or .partition file from ACT
rc: 0: ok
   !=0 not loaded
Usage:
1. Download ctp configs:
linux/act.exe                 -download ctp configs

2. Downlaod partition PHYSICS_*, TEST_*:
linux/act.exe PHYSICS_*

3. Print to stdout "VALUE anystring" where anystring is from ACT:
linux/act.exe VALUE name
example:
ctp_proxy > linux/act.exe VALUE /CTP/filter
INFO Opening ACT:daq:daq@pcald30/ACT
INFO ACT opened succesfuly.
VALUE abcd

4. Download (active instance of ctp config file:
linux/act.exe VALID.BCMASKS

5. TODO: Print to stdout current FILTER (OR of /CTP/trgInput_ACORDE,... 
   names which value is OFF or DISABLED )
ctp_proxy > linux/act.exe FILTER
FILTER ACORDE SPD    -2 triggering detecors filtered out
FILTER               - all triggering detectors available
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vmewrapdefs.h"
//#include "ctp.h"
/* toto von
#include <sys/types.h>
#include <sys/shm.h>
#include "shmaccess.h"
*/
#define DBMAIN
#include "Tpartition.h"

int actdb_open();
int actdb_close();
int actdb_getdbfiles();
int actdb_getdbfile_openclose(char *cfgname);
//int actdb_getPartition(char *partname, char *filter);
int actdb_getPartition(char *name, char *filterpar, char *actname, char *actversion);
int actdb_getdbstring(char *fn, int openclose, char *value, int maxl);
void actdb_getff(char *filter);

int checkproxy() { // done in ctpproxy.py
return(0);
}

/*----------------------------------------
rc: 0: ok
8: ctpproxy active, no action (not done yet!)
   incorrect parameters
7: cannot open ACT

*/ int main(int argc, char **argv) {
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
    printf("rc:%d part:%s INSTANCE:%s version:%s", rc, argv[1], actinst,actver);
  } else if(strcmp(argv[1],"VALUE")==0) {
    char value[1000]="";
    if(argc==3) {
      rc= actdb_getdbstring(argv[2], 1, value, 1000);
      printf("VALUE %s",value);
    } else {
      rc=8;
    };
  } else if(strcmp(argv[1],"FILTER")==0) {
    char filter[260];
    actdb_getff(filter);
    printf("FILTER %s\n", filter);
  } else {
    rc= actdb_getdbfile_openclose(argv[1]);
  };
};
exit(rc);
}

