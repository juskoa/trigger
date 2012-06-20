#include <stdio.h>
#include <string.h>
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif
#include "lexan.h"
typedef unsigned int w32;

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
w32 get_DIMW32(char *service) {
w32 calibbc=0x12345678, calibbcfailed=0x12345678, ix=0;
while(1) {
  int rc;
  rc= dic_info_service(service, ONCE_ONLY, 2, &calibbc,sizeof(w32),
      NULL, 3488, &calibbcfailed, sizeof(w32));
  sleep(1); ix++;
  //printf("get_DIMW32:%s:%d after sleeping 1 secs...\n", service, calibbc);
  if(calibbc==calibbcfailed) {
    if(ix>2) {return(-1); };
  } else {
    return(calibbc);
  };
};
}
