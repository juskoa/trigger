#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "vmewrap.h"
#include "ctp.h"
#define DBMAIN
#include "Tpartition.h"
#ifdef CPLUSPLUS
#include <dis.hxx>
#else
#include <dis.h>
#endif
void ds_register();
void ds_update();
void ds_stop();

int quit=0; 
extern int QUIT;
extern w32 dimsflags;  // see dimservices.c
#define NO1MINFLAG 0x1

void gotsignal(int signum) {
char msg[100];
switch(signum) {
case SIGUSR1:  // kill -s USR1 pid
  signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
  sprintf(msg, "got SIGUSR1 signal:%d, fflush(stdout)\n", signum);
  prtLog(msg);
  //fflush(stdout);
  break;
case SIGINT:
  signal(SIGINT, gotsignal); siginterrupt(SIGINT, 0);
  printf("got SIGINT signal, quitting...:%d\n", signum);
  quit=signum; QUIT=1;
  break;
case SIGQUIT:
  signal(SIGQUIT, gotsignal); siginterrupt(SIGQUIT, 0);
  printf("got SIGQUIT signal, quitting...:%d\n", signum);
  quit=signum; QUIT=1;
  break;
case SIGBUS: 
  //vmeish(); not to be called (if called, it kills dims process)
  sprintf(msg, "got SIGBUS signal:%d\n", signum); prtLog(msg);
  break; 
default:
  printf("got unknown signal:%d\n", signum);
};
}

int main(int argc, char **argv)  {
int rc; char msg[200];
/*
if(argc<3) {
  printf("Usage: ltuserver LTU_name base\n\
where LTU_name is detector name\n\
      base is the base address of LTU (e.g. 0x811000)\n\
"); exit(8);
}; */
if(argc>1) {
  if(strcmp(argv[1],"no1min")==0) {
    dimsflags=dimsflags | NO1MINFLAG;
  };
};
setlinebuf(stdout);
signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
signal(SIGQUIT, gotsignal); siginterrupt(SIGQUIT, 0);
signal(SIGBUS, gotsignal); siginterrupt(SIGBUS, 0);
sprintf(msg,"dims start. dimsflags:%x",dimsflags);
prtLog(msg);
rc= vmeopen("0x820000", "0xd000");
if(rc!=0) {
  printf("vmeopen CTP vme:%d\n", rc); exit(8);
};

cshmInit();
checkCTP(); //we need it: ctpboards[] gets initialised
//readTables(); done ONLY in ctpproxy
//initCTP();
printf("No initCTP. initCTP left to be done by main ctp-proxy when started\n"); 
//ctpboards[0].vmever= 0xa3;
//ctpboards[1].vmever= 0xa3;
ds_register();
while(1)  {  
  /*printf("sleeping 10secs...\n");*/
  dtq_sleep(2); //sleep(10);  
  if(quit>0) break;
  //ds_update();
};  
ds_stop();
exit(0);
}   

