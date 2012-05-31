/* server.c -example of DIM server (note CTPDIMt) publishing
   counters in regular interval (60secs) */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dis.h>
#include "ctpcounters.h"

#define MYNAME "CTPDIMt"

void ds_register();

int quit=0; 
int QUIT=0;       // if >0, thread to be finished
int NCLIENTS=0;   // # of clients
int actcid=0;         /* active client id. 0: nobody active */
char actcidat[80];  /* active client: pid@host */

unsigned int *buf1;
unsigned int COUNTERSid, RESULTid, GETCOUNTERSid;

void gotsignal(int signum) {
switch(signum) {
case SIGUSR1:  // kill -s USR1 pid
  signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
  printf("got SIGUSR1 signal:%d, fflush(stdout)\n", signum);
  fflush(stdout);
  break;
case SIGINT:
  signal(SIGINT, gotsignal); siginterrupt(SIGINT, 0);
  printf("got SIGINT signal, quitting...:%d\n", signum);
  quit=signum; QUIT=1;
  break;
default:
  printf("got unknown signal:%d\n", signum);
};
}
/*--------------------------------------------------------------- error_handler
A severity code: 0 - info, 1 - warning, 2 - error, 3 - fatal.
*/
void error_handler(int severity, int error_code, char *message) {
char *sev[5]={"info", "warning", "error", "fatal", "???"};
if((severity<0) || (severity>3)) {
  severity=4;
};
printf("*** DIM %s: %d:%s\n", sev[severity], error_code, message);
if(severity>1) quit=1;
}
/*------------------------------------------------------------- exit_handler
*/
void exit_handler(int *exitcode) {
printf("exitcode:%d\n", *exitcode);
}
/*------------------------------------------------------------- readCounters
clientid: 0: update all clients subscribing to MONCOUNTERS
        !=0: update only clientid client (forced counters read)
*/
void readCounters(int clientid) {
int ix, nclients; 
for(ix=0; ix<NCOUNTERS; ix++) { // 'real counters' reading replaced by
  buf1[ix]= buf1[ix]+1;         // the increment in this example
};
if(clientid==0) {
  nclients= dis_update_service(COUNTERSid);
  /*
  if(NCLIENTS != nclients) {   // # of clients changed
    NCLIENTS= nclients;
    printf("clients now: %d\n", nclients); fflush(stdout);
  };*/
} else {
  int cids[2];
  cids[0]= clientid; cids[1]= 0;   // end of the list
  nclients= dis_selective_update_service(COUNTERSid, cids);
}; 
}
/*-------------------------------------------------------- cthread
running as thread (started once, with dim server)
*/
void cthread() {
while(1) {   //run forever
  readCounters(0);
  dtq_sleep(60);
  if(QUIT==1) break;
};
}
/*------------------------------------------- checkcid() 
return:
<=0 -error
>0 -current cid, which is OK, (was before or just has been set)
*/
int checkcid() {
int loccid;
char loccidat[80];
loccid= dis_get_client(loccidat);
//printf("           actcid1:%d, actcidat:%s\n",actcid, actcidat);
  // don't check it for now...
  actcid=loccid; strcpy(actcidat,loccidat); return(actcid);
}
/*----------------------------------------------------------- MONCOUNTERScaba
*/
void MONCOUNTERScaba(int *tag, unsigned int **msgp, int *size) {
//printf("MONCOUNTERScaba sizeorig:%d NCOUNTERS:%d\n", *size, NCOUNTERS);
*msgp= buf1;
*size= 4*(NCOUNTERS);
//printf("MONCOUNTERScaba size:%d NCOUNTERS:%d \n", *size, NCOUNTERS);
}
/*--------------------*/ void getcnts(int *tag, char *msg, int *size)  {  
/* Forced counters reading. Command GETCOUNTERS is to
be invoked by client after EndOfRun when it needs precise values, and
without waiting till the end of 60sec. interval. */
int clientid;
clientid=checkcid();
if(clientid<0) return;
printf("getcnts: tag:%d size:%d cid:%s\n", *tag, *size, actcidat);
readCounters(clientid);
}
/*-----------------*/ void RESULTcaba(int *tag, char **msgp, int *size) {
//char logmsg[100];
*tag=4567;
/*if(checkcid()<0) {
  *msgp= foreignmsg;
  *size= strlen(foreignmsg)+1;
  return;
};*/ 
//printf("RESULTcaba actcid:%d, actcidat:%s\n",actcid, actcidat); 
*msgp= (char *)buf1;
//updateNMCclients();
*size= 4*(NCOUNTERS);
}

/*-------------------------------------------------------------- ds_stop
*/
void ds_stop() {
QUIT=1;   // stop thread reading ctp counters
printf("exiting...\n");
dis_remove_service(COUNTERSid);
dis_remove_service(GETCOUNTERSid);
dis_remove_service(RESULTid);
dis_stop_serving();
free(buf1); buf1=NULL;
}
/*--------------------------------------------------------------- ds_register
*/
void ds_register() {
int ix;
char command[100];
buf1= malloc(NCOUNTERS*sizeof(unsigned int));
for(ix=0; ix<NCOUNTERS; ix++) {
  buf1[ix]= ix;
};
printf("DIM server:%s\n",MYNAME);
dis_add_error_handler(error_handler);
dis_add_exit_handler(exit_handler);
printf("\nCommands:\n");
strcpy(command, MYNAME); strcat(command, "/GETCOUNTERS");
GETCOUNTERSid=dis_add_cmnd(command,NULL, getcnts, 333);  printf("%s\n", command);
strcpy(command, MYNAME); strcat(command, "/GETC1");
dis_add_cmnd(command,0, getcnts, 4568); printf("%s\n", command);
printf("\nServices:\n");
strcpy(command, MYNAME); strcat(command, "/MONCOUNTERS");
COUNTERSid=dis_add_service(command,0, buf1, 4*(NCOUNTERS),
  MONCOUNTERScaba, 4567);  printf("%s\n", command);
strcpy(command, MYNAME); strcat(command, "/RESULT");
RESULTid=dis_add_service(command,0, buf1, sizeof(buf1), 
  RESULTcaba, 4568); 

printf("serving...\n");
dis_start_serving(MYNAME);  
printf("Starting the counters reading thread...\n");
dim_start_thread(cthread, 333);
}

/*----------------------------------*/ int main(int argc, char **argv)  {
int rc=0;
/*
if(argc<3) {
  printf("Usage: ltuserver LTU_name base\n\
where LTU_name is detector name\n\
      base is the base address of LTU (e.g. 0x811000)\n\
"); exit(8);
}; */
setlinebuf(stdout);
signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
signal(SIGINT, gotsignal); siginterrupt(SIGINT, 0);
ds_register();
while(1)  {  
  if(quit>0) break;
  /*printf("sleeping 10secs...\n");*/
  dtq_sleep(10); //sleep(10);  
};  
ds_stop();
return(rc);
}   

