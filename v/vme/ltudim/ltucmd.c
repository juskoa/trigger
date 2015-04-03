#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif

#define MAXRESULT 1500
#define TAGdo 33
#define ERRMSGL 200

int WAITING=1;
char DETNAME[10];
char DNDO[20];   // detname/DO
char DNCMD[20];   // detname/CMD
char result[MAXRESULT+1]="empty";   // final \0 included
char resultFailed[]="failed...";

char cmd[80];

/*------------------------------------------*/ void prerr(char *msg) {
printf("ERROR:%s\n:\n", msg); fflush(stdout);
}

/*--------------------------------------------------*/ int waitinfocall() {
/* rc: 1: ok   0: timeout */
#define MAX10ms 800
int ix,rc;
for(ix=0; ix<=MAX10ms; ix++) {   // 5 secs =1000 x 10ms
  if(WAITING) {
    usleep(10000);
  } else {
    break;
  };
};
if(ix>=MAX10ms) {
  char errmsg[ERRMSGL];
  sprintf(errmsg,"DBGwaitinfocall: timeout %d ms", ix*10); 
  prerr(errmsg); 
  rc=0;
} else {
  printf("DBGwaitinfocall:%d ms\n",ix*10);
  rc=1;
};
return(rc);
}
/*------------------*/ void infocallback(void *tag, void *buffer, int *size) {
WAITING=0;
}
/*----------------------------------*/void callback(void *tag, int *rc) {
//printf("callback tag:%d rc:%d\n", *tag, *rc);
if(*rc != 1) {
  char errmsg[ERRMSGL];
  sprintf(errmsg,"callback for tag:%d not executed by server %s. rc:%d",
    *(int *)tag, DETNAME, *rc);
  prerr(errmsg); 
}; 
}
/*-----------------------------*/ int executewait(char *cmd, char *inpline) {
/* rc: 1:ok   0: not executed */
int rc;
WAITING=1; /* wait for infocallback: */
rc= dic_cmnd_callback(cmd, inpline, strlen(inpline)+1, callback, TAGdo);
//rc= dic_cmnd_callback(DNDO, inpline, strlen(inpline)+1, callback, TAGdo);
printf("DBGexecute:%s:%d:%s<\nWaiting max. %d ms...\n",cmd, rc, 
  inpline,10*MAX10ms); 
rc= waitinfocall();
return(rc);
}
/*-----------------------------*/ int execute(char *cmd, char *inpline) {
/* rc: 1:ok   0: not executed */
int rc;
rc= dic_cmnd_callback(cmd, inpline, strlen(inpline)+1, callback, TAGdo);
return(rc);
}
/*----------------------------------*/ int main(int argc, char **argv) {
int rc, RESULTid;
int calibbc=6666, calibbcfailed=4444;
strcpy(DETNAME,"hmpid");
//strcpy(DETNAME,"t0");
strcpy(DNDO,DETNAME); strcat(DNDO, "/DO");
strcpy(DNCMD,DETNAME); strcat(DNCMD, "/CMD");

//------------------------------------ 1.             DO (pipe)+RESULT
//Before all, we have to register with /RESULT service: 
strcpy(cmd,DETNAME); strcat(cmd, "/RESULT");
WAITING=1;
RESULTid= dic_info_service(cmd, MONITORED, 0, result,MAXRESULT+1,
  infocallback, 136, resultFailed, strlen(resultFailed)+1); 
rc=waitinfocall();
printf("1st rc:%d:%s<\n", rc, result);

rc=executewait(DNDO, "TTCinit()\n");
if(rc==1) { printf("%s<\n", result); 
} else {
  printf("not executed. rc:%d\n", rc);
};
//------------------------------------ 1.             DO+RESULT 
//
/*------------------------------------ 2.             DO only
rc=execute(DNDO, "TTCinit()\n");
if(rc==1) { printf("OK, sent over DIM\n"); 
  sleep(1);   // at least 1secs, if not, server does not see the request!
} else {
  printf("not executed. rc:%d\n", rc);
};
*/
/*------------------------------------ .             EXEC (service)
not used
*/
/*------------------------------------ 3.             CMD only
rc=execute(DNCMD, "ttcrxreset");
if(rc==1) { printf("OK, sent over DIM\n"); 
  sleep(1);   // at least 1secs, if not, server does not see the request!
} else {
  printf("not executed. rc:%d\n", rc);
};
*/
//------------------------------------ 3.             CMD only
//
/*------------------------------------ 4.   only service, ONCE_ONLY
strcpy(cmd,DETNAME); strcat(cmd, "/CALIBBC");
while(1) {
  rc= dic_info_service(cmd, ONCE_ONLY, 2, &calibbc,sizeof(int),
      NULL, 3488, &calibbcfailed, sizeof(int));
  printf("%s:%d\n", cmd, calibbc);
  printf("sleeping 1 secs...\n");
  sleep(1);
  //sleep(3);
};
*/

}

