/*
g++  -g -I/opt/dim/dim  -DCPLUSPLUS ./cmd1.c -L/opt/dim/linux -ldim -o linux/cmd1  -lpthread
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif

#define TAGdo 33
char DETNAME[10]="";
char DNCMD[20];   // detname/CMD
char cmdglobal[80]="bad cmd";
int rcglob=8;
/*------------------------------------------*/ void prerr(char *msg) {
printf("ERROR:%s\n", msg); fflush(stdout);
}

/*----------------------------------*/void callback(void *tag, int *rc) {
//printf("callback tag:%d rc:%d\n", *tag, *rc);
if(*rc != 1) {
  char errmsg[200];
  sprintf(errmsg,"callback for tag:%d not executed by server %s. rc:%d",
    *(int *)tag, DETNAME, *rc);
  prerr(errmsg); 
  rcglob=8;
} else {
  printf("OK, %s sent over DIM to %s\n", cmdglobal, DETNAME); 
  rcglob=0;
}; 
}
/*-----------------------------*/ int execute(char *cmd, char *inpline) {
/* rc: 1:ok   0: not executed */
int rc;
strcpy(cmdglobal, cmd);
rc= dic_cmnd_callback(cmd, inpline, strlen(inpline)+1, callback, TAGdo);
return(rc);
}
/*----------------------------------*/ int main(int argc, char **argv) {
int rc;
if(argc!=2) {
  printf("Give detector name, for example: linux/cmd1 acorde\n");
  exit(4);
};
strcpy(DETNAME,argv[1]);
strcpy(DNCMD,DETNAME); strcat(DNCMD, "/CMD");
rc=execute(DNCMD, (char *)"ttcrxreset");   // TTCrx reset + TTCrx regs init
//rc=execute(DNCMD, "ttcrxreset fee");   // ttcrxreset + FEE reset
if(rc==1) { 
  sleep(2);   // at least 1secs, if not, server does not see the request!
} else {
  printf("not executed. rc:%d\n", rc);
  rcglob=8;
};
return(rcglob);
}

