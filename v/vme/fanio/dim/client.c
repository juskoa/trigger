#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif
#include "lexan.h"
//#include <ctype.h>
 
#define STATLEN strlen(OCCMD)
#define MAXINPUTS 24

typedef struct {
  char name[16];  //detname
  unsigned int mask;
} Tenablemsg;
typedef struct {
  unsigned int ed;
  unsigned int busy;
} Tstatusmsg;

char cmd[80];
char detector[16];   // detector name

char StatusString[MAXINPUTS];   // E or D
char StatusInput[MAXINPUTS];    // 1 or 0
Tstatusmsg StatusFailed={0xdeadbeaf,0xdeadbeaf}; /* det/STATUS service failed */
Tstatusmsg statusmsg;

void callback(void *tag, int *rc) {
printf("callback tag:%d rc:%d\n", *(int *)tag, *rc);
if(*rc == 1) {
  printf("OK\n");
} else {
  printf("Command not executed by server. tag:%d rc:%d\n", *(int *)tag, *rc);
};
}

void info_cb(void *tag, void *msgv, int *size) {
int ix;
Tstatusmsg *msg= (Tstatusmsg *)msgv;
/*printf("tag:%d size:%d enabled:0x%x busy:0x%x\n",
  *tag, *size, msg->ed, msg->busy);
*/
printf("FANIN:%s BUSY_MASK:%x READ_INPUTS:%x\n", detector, msg->ed, msg->busy); 
if(msg->ed==0xdeadbeaf) {
  printf("server not responding (different from daq fmd hmpid ssd?)\n"); return;
};
printf("...20....,...10....,...1\n");
for(ix=23; ix>=0; ix--) {
  char ed;
  if( msg->ed & (1<<ix) ) { ed='E'; } else { ed='D'; }; printf("%c", ed);
}; printf("\n");
for(ix=23; ix>=0; ix--) {
  char ed;
  if( msg->busy & (1<<ix) ) { ed='1'; } else { ed='0'; }; printf("%c", ed);
}; printf("\n");
}

void print_help() {
  printf("Start client by one of the following commands:\n\
1.\n\
client status \n\
To get the current FANIN status in the format:\n\
...20....,...10....,...1\n\
EDE...                      Enabled/Disbaled channel\n\
100...                      current status of FANIN inputs\n\
\n\
2.\n\
client enable 0xmask \n\
To enable mask (will be set in FANIN)\n\
3.\n\
client save 0xmask \n\
To save mask as default (will be loaded after the server restart/reboot)\n\
\n\
where:\n\
mask: hexadecimal number reppresenting channels 24..1.\n\
      bits corresponding to enabled channels are set to 1\n\
\n\
\n");
}

int mod_detector(char *newdet) {
int rc;
if(strcmp(detector,"trigger")==0) {
  strncpy(detector, newdet, 16); rc=0;
} else {
  printf("Last parameter:%s is allowed only from trigger account\n", newdet);
  rc=1;
};
return(rc);
}

/*
 I: hdnum: 0xhexa 
 O: rc:0, *num is internal representation of the number
    rc:2  syntax error 
int gethexdec(char *hdnum, unsigned int *num) {
unsigned int nn; int rcs;
rcs= sscanf(hdnum, "0x%x", &nn);
//printf("   %s -> 0x%x   rcs:%d\n", hdnum, nn, rcs);
if(rcs==1) {
  rcs=0;
} else {
  rcs=2;
};
return(rcs);
}
void UPPER (char *name) {
char *p;
p=name;  
while ( *p != '\0' ) { *p=toupper(*p); p++; };
} 

*/
int main(int argc, char **argv) {
int rc;
char *environ;
if(argc<2) {
  print_help();
  exit(4);
};
strncpy(cmd, argv[1], 80);
environ= getenv("LOGNAME");
if(environ ==NULL) {
  printf("LOGNAME environment not set!");
  detector[0]='\0';
} else {
  strcpy(detector, environ);
};
if( strcmp(cmd, "status")==0 ) {           // status
  char detcmd[80];
  if(argc==3) { 
    if(mod_detector(argv[2])) goto RET;
  };
  /* service */
  strcpy(detcmd, detector); strcat(detcmd, "/STATUS");
  rc= dic_info_service(detcmd, ONCE_ONLY, 1, &statusmsg, sizeof(statusmsg),
      info_cb, 3488, &StatusFailed, sizeof(StatusFailed));
  //usleep(1000000);
} else if((strcmp(cmd, "enable")==0) ||
         ((strcmp(cmd, "save")==0))) {      // enable or save
  Tenablemsg msg;
  if(argc==4) { 
    if(mod_detector(argv[3])) goto RET;
  };
  if(argc<3) {
    printf("mask not given, disabling all channels (0x000000)\n");
    msg.mask=0;
  } else {
    int rc;
    rc= gethexdec(argv[2], &msg.mask);
    if(rc>0) {
      printf("%s -bad mask (0xHEXDIGITS or integer expected)\n", argv[2]);
      exit(4);
    };
  };
  /* command */
  /* rc= dic_cmnd_callback(cmd, ctpinput, strlen(ctpinput)+1, callback, 33);*/
  strcpy(msg.name, detector); UPPER(cmd);
  rc= dic_cmnd_callback(cmd, &msg, sizeof(msg), callback, 33);
} else {
  print_help();
};
/*printf("rc:%d\n",rc); */
RET:
sleep(1);
} 
