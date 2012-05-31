#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif
#include "vmeblib.h"

#define MAXWAIT 2   // in secs
#define STEPMS 200000   // in micsecs
float Shift, Shift_failed=26.;
w32 reqsec,requsec;
char cmd[100]="PHASE_SHIFT_BPTX1";
char DNS[]="alidcsdimdns.cern.ch";

void callbackShift(void *tag, void *buf, int *size) {
int rc, secs,msecs;
printf("callbackShift tag:%d size:%d\n", *(int *)tag, *size);
/*
if(*size == sizeof(int)) {
  printf("%s OK, *buf:%d Shift:%6.4f\n", cmd, *(int *)buf, Shift);
} else {
  printf("%s not executed by server, size:%d\n", cmd, *size);
};
*/
rc= dic_get_timestamp(0, &secs, &msecs);
if(rc==1) { 
  printf("timestamp: %d s %d ms\n",secs,msecs);
  //timesecs= secs
};
rc= dic_get_quality(0);
printf("quality:%d\n", rc);
}

int main(int argc, char **argv) {
int ia,rc;
int secs,msecs;
unsigned int sid,waitus;
for(ia=0; ia<argc; ia++) {
  printf("arg%d: %s\n", ia, argv[ia]);
};
/* get service 
unsigned int dic_info_service (name, type, timeout, address, size, user_routine, tag, fill_address, fill_size)

sid= dic_info_service(cmd, ONCE_ONLY, 2, StatusString,MAXCTPINPUTS+1, 
      NULL, 3488, StatusFailed, MAXCTPINPUTS+1);
*/
rc= dic_set_dns_node(DNS);
if(rc==1) { printf("DNS:%s set\n", DNS); 
} else { printf("DNS:%s not set\n", DNS); };
Shift= Shift_failed;
GetMicSec(&reqsec, &requsec);
printf("req:%ds %dus\n", reqsec, requsec);
//sid= dic_info_service(cmd, 
sid= dic_info_service_stamped(cmd, 
  ONCE_ONLY,                  // disconnect from service itself
  MAXWAIT, &Shift ,sizeof(float),   // failed if not within 2 secs
  callbackShift, 3488,        // tag
  &Shift_failed, sizeof(float));
waitus=0;
while(waitus < MAXWAIT*1000000) {
  if( Shift <26.) {
    printf("%.4f\n", Shift);
    goto OK;
  };
  usleep(STEPMS); waitus= waitus+STEPMS;
};
printf("None %.4f\n", Shift);
OK: 
rc= dic_get_timestamp(sid, &secs, &msecs);
if(rc==1) { 
  printf("timestamp sid:%d: %d s %d ms\n",sid,secs,msecs);
  //timesecs= secs
};

return(0);

/* send command 
rc= dic_cmnd_callback(cmd, &ocinp[0], 8, callback, 33);
*/

}
