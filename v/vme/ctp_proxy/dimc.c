/* dimc.c -testing dims.c+dimservices.c. 
For examples, distributed/publicised on web see:
aj@pcalicebhm05:dimcdistrib */
#include <stdio.h>
//#include <unistd.h>
//#include <fcntl.h>
//#include <signal.h>

#include <stdlib.h>
#include <string.h> 

#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif
#include "vmewrap.h"
#include "dimctypes.h"
#include "ctp.h"
 
#define MAXRESULT 800
#define MAXLILE 200
#define NBST 40

int WAITING;
int bstinfo;
char cmd[80];
char inpline[MAXLILE];
char result[MAXRESULT];
char resultFailed[]="failed...";
unsigned int cnts[NCOUNTERS];
unsigned int cntsFailed=0xdeaddeed;
unsigned int bstmsg[NBST];

void gotresult(void *tag, void *buffer, int *size) {
//printf("gotresult tag:%d size:%d buffer:%s\n", (int *)tag, *size, (char *)buffer);
printf(">%s<\n", (char *)buffer);
WAITING=0;
}

void callback(void *tag, int *rc) {
//printf("callback tag:%d rc:%d\n", (int *)tag, *rc);
if(*rc != 1) {
  printf("Command:%s %s not executed by server\n",cmd,inpline);
}; 
}
/*-----------------------*/ void gotbst(void *tag, void *buffer, int *size) {
int ix;
printf("\ngotbst tag:%d size:%d (%d words)\n", *(int *)tag, *size, *size/4 );
if(*size != 4*NBST) {
  printf("error in gotbst. got:%d, expected:%d words\n\
First word of message (if any):0x%x\n",
    *size/4, NBST, *(w32 *)buffer);
};
//printBST((w32 *)buffer);
printf("bst0..3:");
for(ix=0; ix<4; ix++) {
  printf(" %8x", ((w32 *)buffer)[ix]);
}; printf("\n");
}

void execute() {
int rc;
//printf("executing:%s\n",inpline);
WAITING=1;
if(inpline[0]=='S') {
  rc= dic_cmnd_callback("CTPDIM/STATUS", &inpline[2], 
    strlen(&inpline[2])+1, callback, 33);
} else if(inpline[0]=='T'){
  Tswtrg trgmsg;
  if(strlen(inpline) < 4) {   // 'T d' +NL =4chars (minimum if det given)
    strcpy(trgmsg.name,"HMPID");
  } else {
    int i;
    for(i=0; i<=8; i++) {
      char c;
      c= inpline[i+2];
      if((c=='\0') || (c=='\n') || (c==' ') || (i==8)) {
        trgmsg.name[i]= '\0';
        break;
      };
      trgmsg.name[i]= c;
    };
  };
  /*printf("inpline:%s:\n", inpline);
  printf("detname:%s:\n", trgmsg.name); */
  strcpy(trgmsg.pf,"pfX");
  strcpy(trgmsg.bcmask,"bcmaskX");
  trgmsg.type='s';
  trgmsg.roc=0xa; /* trgmsg.bc=1000;*/ trgmsg.N=5;
  rc= dic_cmnd_callback("CTPDIM/SWTRG", (int *)&trgmsg, 
    sizeof(trgmsg), callback, 35);
} else if(inpline[0]=='g') {
  // Forced counters reading:
    rc= dic_cmnd_service("CTPDIM/GETCOUNTERS", NULL,0);
    printf("RC from dic_cmnd_service:%d\n", rc);
} else if(inpline[0]=='D') {
  rc= dic_cmnd_callback("CTPDIM/DO", &inpline[2], 
    strlen(&inpline[2])+1, callback, 34);
} else if(strncmp(inpline, "sm",2)==0) {
  bstinfo= dic_info_service("CTPDIM/MONBST", MONITORED, 0, 
  bstmsg,4*NBST, gotbst, 138, &cntsFailed, 4); 
  printf("%s service id:%d\n", "CTPDIM/MONBST", bstinfo);
} else if(strncmp(inpline, "so",2)==0) {
  bstinfo= dic_info_service("CTPDIM/MONBST", ONCE_ONLY, 0, 
  bstmsg,4*NBST, gotbst, 138, &cntsFailed, 4); 
  printf("%s service id:%d\n", "CTPDIM/MONBST", bstinfo);
} else if(inpline[0]=='u') {
  dic_release_service(bstinfo);
} else {
  printf("???\n");
};
/* wait for result: */
/*
for(ix=0; ix<1000; ix++) {   // 10 secs
  if(WAITING) {
    usleep(10000);
  } else {
    break;
  };
};
if(ix>=100) printf("timeout in execute\n"); 
*/
}

int main(int argc, char **argv) {
int inforc;
/*
if((argc<2) || (argc>3)) {
  printf("Start client with 1 or 2 parameters:\n\
client DETECTOR_NAME/DO string\n\
client DETECTOR_NAME/EXIT\n");
  //exit(4);
};
strncpy(cmd, argv[1], 80);
if(argc==3) {
  strncpy(inpline, argv[2], 80); inpline[79]='\0';
} else {
  strcpy(inpline, "");
};
*/
inforc= dic_info_service("CTPDIM/RESULT", MONITORED, 0, result,MAXRESULT,
  gotresult, 136, resultFailed, strlen(resultFailed)+1); 
printf("RESULT:%s: service_id:%d\n", result, inforc);
//sleep(1);
while(1) {
  printf("\n\
W N            -send 'CTPDIM/DO W' command -wait N loops (1 loop takes 1 sec)\n\
g              -send 'CTPDIM/GETCOUNTERS' command (forced counters read)\n\
S kw           -send 'CTPDIM/STATUS' kw: ALLRARE LTUS CLIENTS LHCSHIFT\n\
T detname      -send 'CTPDIM/SWTRG' command (HMPID is default detname)\n\
D other          -send 'CTPDIM/DO other' command\n\
sm             - subscribe to BST messages  (so: ONCE_ONLY   u: unsubscribe)\n\
qq             -quit server, client          q     -quit client\n:");
  fgets(inpline, MAXLILE, stdin);
  if(strcmp(inpline,"q\n")==0) break;
  execute();
  if(strcmp(inpline,"qq\n")==0) break;
};
dic_release_service(inforc);
sleep(2);
return(0);
} 
