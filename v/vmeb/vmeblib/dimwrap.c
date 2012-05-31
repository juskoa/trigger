#include <stdio.h>
#include <string.h>
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif
#include "lexan.h"

/*-------------------------------------------
rc: -1 -cannot contact DIM ltu server
   >0: CALIBRATION_BC parameter from shared memory (ltuttc.cfg)
*/
int getCALIBBC(char *detname) {
int calibbc=6666, calibbcfailed=4444, ix=0;
char cmd[20];
strcpy(cmd,detname); LOWER(cmd); strcat(cmd, "/CALIBBC");
while(1) {
  int rc;
  rc= dic_info_service(cmd, ONCE_ONLY, 2, &calibbc,sizeof(int),
      NULL, 3488, &calibbcfailed, sizeof(int));
  sleep(1); ix++;
  printf("getCALIBBC:%s:%d after sleeping 1 secs...\n", cmd, calibbc);
  if(calibbc>3563) {
    if(ix>2) {return(-1); };
  } else {
    return(calibbc);
  };
};
}

