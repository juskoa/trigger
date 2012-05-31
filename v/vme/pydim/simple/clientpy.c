/* clientpy_simple.c
Python extension (swig) for DIM services.
Simple one for communication with only 1 DIM server
 */
#include <stdio.h>
#include <string.h>
#include <dic.h>
/* 
#define OCCMD "STATUS\n"
#define STATLEN strlen(OCCMD)
#define MAXSTATUS 80
#define MAXMESSAGE 400

char cmd[80];
char message[MAXMESSAGE];

char StatusString[MAXSTATUS];   
char StatusFailed[MAXSTATUS];   // /STATUS service failed
*/
#define ERRMSGL 400
int messages_sent=0;
int messages_sent_acked=0;
int messages_sent_answd=0;
/*
-------------------------------------------*/ void callback(int *tag, int*rc) {
//printf("callback tag:%d rc:%d\n", *tag, *rc);
if(*rc == 1) {
  messages_sent_acked++;
  //printf("%s %s OK\n",cmd,message);
  /*printf("Callback: OK sent:%d sent_acked:%d sent_answd:%d\n", 
    messages_sent, messages_sent_acked, messages_sent_answd);*/
  //sleep(3); printf("Callback: OK+3secs\n");
} else {
  //printf("%s %s not executed by server\n",cmd,message);
  printf("Callback: Error, command not sent\n");
};
}
#define MAXRESULT 1500
char resultFailed[]="failed...";
char result[MAXRESULT+1]="empty";   // final \0 included
int resultALLOCATED=0;             // 1: allocated 0: free
/*
----------------------*/ void infocallback(int *tag, char *buffer, int *size) {
int msglen;
char buf1[81]; 
messages_sent_answd++;
/*strncpy(buf1,buffer,80); buf1[80]='\0'; if(*size<80) buf1[*size]='\0';
printf("DBGinfcb:%d:%d:%s<<<\n", *tag, *size, buf1);
*/
msglen= *size;
/* malloc to be used here (or the pool of buffers alloceated beforehand)
*/
if(msglen > MAXRESULT) {
  printf("infocallback: Error: too long message received (>%d chars)\n", MAXRESULT);
  msglen= MAXRESULT;
} else if(buffer[msglen-1]!= '\0') {
  printf("infocallback: Error: last char in message forced to 0\n");
  buffer[msglen-1]='\0';
} else if(resultALLOCATED!=0) {
  printf("infocallback: Error: response lost\n");
};
resultALLOCATED=1;
strncpy(result, buffer, msglen); 
}

//----------------------- functions visible from python (see clientpy.i)
/*
---------------------------*/ int dicxcmnd_callback(char *cmd, char *message) {
int rc=3;
//printf("dicxcmnd_callback:%s %s\n", cmd, message);
messages_sent++;
rc= dic_cmnd_callback(cmd, message, strlen(message)+1, callback, messages_sent);
return(rc);   // rc:1 -> request OK
}

/* To be called only once.
------------------------------------------*/ int dicxinfo_service(char *cmd) {
int RESULTid;
messages_sent++; messages_sent_acked++;
//RESULTid= dic_info_service(cmd, MONITORED, 0, result,MAXRESULT+1,
RESULTid= dic_info_service(cmd, MONITORED, 0, NULL,MAXRESULT+1,
  infocallback, 136, resultFailed, strlen(resultFailed)+1); 
  return(RESULTid);
}
/*
-----------------------------------------*/ void dicxrelease_service(int id) {
printf("dicxrelease_service id:%d\n", id);
dic_release_service(id);
}

#define WAITSTEP 10         // in milisecs
/* 
rc:0 -timeout
   1 -OK, answer received in less then secs seconds 
---------------------------------*/ char *waitinfocall(int secs) {
int ix,rc,steps=secs*1000/WAITSTEP;
for(ix=0; ix<steps; ix++) {   // 10 secs =1000 x 10ms
  if(messages_sent_answd<messages_sent) {
    usleep(1000*WAITSTEP);
  } else {
    break;
  };
};
if(ix>=steps) {
  char errmsg[ERRMSGL];
  sprintf(errmsg,"DBGwaitinfocall: timeout %d ms sent/answd:%d/%d", 
    ix*10, messages_sent, messages_sent_answd); 
  printf("%s\n", errmsg); //prerr(errmsg);
  rc=0;
  return("");
} else {
  //printf("DBGwaitinfocall:%d ms\n", ix*100);
  rc=1;
};
resultALLOCATED=0; return(result);
}

char *test(char *instr) {
char resp[60];
printf("test: instr:%s\n", instr);
//strcpy(instr,"blabla"); ILLEGAL!  max. strlen(instr) chars are passed back
// correct:
strcpy(resp, instr); strcat(resp, instr);
return(resp);  // resp: will be copied to new python string object
}

