#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#ifdef CPLUSPLUS
#include <dis.hxx>
#else
#include <dis.h>
#endif
#define MYNAME "TTCMI"

unsigned int qpllstat=0, QPLLid;
int quit=0;

void gotsignal(int signum) {
char msg[100];
switch(signum) {
case SIGUSR1:  // kill -s USR1 pid
  signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
  sprintf(msg, "got SIGUSR1 signal:%d, fflush(stdout)\n", signum);
  printf(msg);
  fflush(stdout);
  break;
case SIGINT:
  signal(SIGINT, gotsignal); siginterrupt(SIGINT, 0);
  printf("got SIGINT signal, quitting...:%d\n", signum);
  quit=signum;
  break;
case SIGQUIT:
  signal(SIGQUIT, gotsignal); siginterrupt(SIGQUIT, 0);
  printf("got SIGQUIT signal, quitting...:%d\n", signum);
  quit=signum;
  break;
case SIGBUS: 
  //vmeish(); not to be called (if called, it kills dims process)
  printf(msg, "got SIGBUS signal:%d\n", signum); //prtLog(msg);
  break; 
default:
  printf("got unknown signal:%d\n", signum);
};
}

int update_qpll() {
int rc;
qpllstat=qpllstat+1;
rc= dis_update_service(QPLLid);   // rc:1 at least 1 client active, 0: no clients active
printf("QPLL update qpllstat:%d rc:%d\n", qpllstat, rc);
return(rc);
}
/*
void QPLLcaba(void *tag, void **addr, int *size, int *dmmy) {
printf("QPLLcaba tag:%d\n", *(int *)tag);
*addr= &qpllstat;
*size= sizeof(qpllstat);
}*/
int main(int argc, char **argv)  {
char command[200];
setlinebuf(stdout);
signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
signal(SIGQUIT, gotsignal); siginterrupt(SIGQUIT, 0);
signal(SIGBUS, gotsignal); siginterrupt(SIGBUS, 0);
strcpy(command, MYNAME); strcat(command, "/QPLL");
QPLLid=dis_add_service(command, "L", &qpllstat, 4,
//  QPLLcaba, 88);  printf("%s\n", command);
  NULL, 88);  printf("%s\n", command);

printf("serving...\n");
dis_start_serving(MYNAME);  
while(1)  {  
  int rc;
  rc= update_qpll();
  //if(rc!=1) break;
  printf("sleeping 5secs...\n"); fflush(stdout);
  sleep(5) ; 
  //dtq_sleep(5);
  //printf("slept 5...\n"); fflush(stdout);
};  
dis_remove_service(QPLLid);
exit(0);
}   


