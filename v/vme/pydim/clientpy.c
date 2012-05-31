/* clientpy.c
Python extension (swig) for DIM services.
For communications with more 1 DIM servers.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dic.h>
#define ERRMSGL 400
#define MAXDIMCONS 20
#define MAXRESULT 1500
char resultFailed[]="failed...";

typedef struct Tdimcon {
 int RESULTid;
 int messages_sent;
 int messages_sent_acked;
 int messages_sent_answd;
 int resultALLOCATED;             // 1: allocated 0: free
 char *result;
} Tdimcon;

Tdimcon dimcons[MAXDIMCONS];
/*
-------------------------------------------*/ void callback(int *tag, int*rc) {
int ixcon=*tag;
//printf("callback tag:%d rc:%d\n", *tag, *rc);
if(*rc == 1) {
  dimcons[ixcon].messages_sent_acked++;
  //printf("%s %s OK\n",cmd,message);
  /*printf("Callback: OK sent:%d sent_acked:%d sent_answd:%d\n", 
    messages_sent, messages_sent_acked, messages_sent_answd);*/
  //sleep(3); printf("Callback: OK+3secs\n");
} else {
  //printf("%s %s not executed by server\n",cmd,message);
  printf("Callback: Error, command not sent\n");
};
}
/*
----------------------*/ void infocallback(int *tag, char *buffer, int *size) {
int ixcon=*tag;
int msglen;
char buf1[81]; 
dimcons[ixcon].messages_sent_answd++;
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
} else if(dimcons[ixcon].resultALLOCATED!=0) {
  printf("infocallback: Error: response lost(not acquired)\n");
};
dimcons[ixcon].resultALLOCATED=1;
strncpy(dimcons[ixcon].result, buffer, msglen); 
}

//----------------------- functions visible from python (see clientpy.i)
/*
---------------------------*/ void dicxinit() {
//Tdimcon *dimcons[MAXDIMCONS];
int ix;
for(ix=0; ix<MAXDIMCONS; ix++) {
  dimcons[ix].result=NULL;
};
}
/*
---------------------------*/ int dicxcmnd_callback(int tag, char *cmd, char *message) {
int rc=3,ixcon=tag;
//printf("dicxcmnd_callback:%s %s\n", cmd, message);
dimcons[ixcon].messages_sent++;
rc= dic_cmnd_callback(cmd, message, strlen(message)+1, callback, tag);
return(rc);   // rc:1 . request OK
}

/* To be called only once.
rc: 0 -not successfull
   >0 - tag to be used with dicxcmnd_callback(),...
------------------------------------------*/ int dicxinfo_service(char *cmd) {
int RESULTid, tag, ixcon=-1,ix;
// find free item in dimcons:
for(ix=1; ix<MAXDIMCONS; ix++) {
  if(dimcons[ix].result==NULL) {
    ixcon=ix; break; };
};
//printf("found:%d\n", ixcon);
if(ixcon==-1) { return(0); };
dimcons[ixcon].result= malloc(MAXRESULT+1);
if(dimcons[ixcon].result==NULL) return(0);
dimcons[ixcon].messages_sent=0;
dimcons[ixcon].messages_sent_acked=0;
dimcons[ixcon].messages_sent_answd=0;
dimcons[ixcon].resultALLOCATED=0; 
tag=ixcon;
//RESULTid= dic_info_service(cmd, MONITORED, 0, result,MAXRESULT+1,
dimcons[ixcon].RESULTid= dic_info_service(cmd, MONITORED, 0, NULL,MAXRESULT+1,
  infocallback, tag, resultFailed, strlen(resultFailed)+1); 
//if(strcmp( resultFailed
dimcons[ixcon].messages_sent++; dimcons[ixcon].messages_sent_acked++;
return(tag);
}
/*
-----------------------------------------*/ void dicxrelease_service(int tag) {
int ixcon=tag;
printf("dicxrelease_service tag:%d\n", tag);
dic_release_service(dimcons[ixcon].RESULTid);
free(dimcons[ixcon].result);
dimcons[ixcon].result= NULL;
}

#define WAITSTEP 10         // in milisecs
/* 
rc:0 -timeout
   1 -OK, answer received in less then secs seconds 
---------------------------------*/ char *waitinfocall(int tag, int secs) {
int ixcon=tag;
int ix,rc,steps=secs*1000/WAITSTEP;
if(tag==0) return("");
for(ix=0; ix<steps; ix++) {   // 10 secs =1000 x 10ms
  if(dimcons[ixcon].messages_sent_answd<dimcons[ixcon].messages_sent) {
    usleep(1000*WAITSTEP);
  } else {
    break;
  };
};
if(ix>=steps) {
  char errmsg[ERRMSGL];
  sprintf(errmsg,"DBGwaitinfocall: timeout %d ms sent/answd:%d/%d", 
    ix*10, dimcons[ixcon].messages_sent, dimcons[ixcon].messages_sent_answd); 
  printf("%s\n", errmsg); //prerr(errmsg);
  rc=0;
  return("");
} else {
  //printf("DBGwaitinfocall:%d ms\n", ix*100);
  rc=1;
};
dimcons[ixcon].resultALLOCATED=0; return(dimcons[ixcon].result);
}

char *test(char *instr) {
char resp[60];
printf("test: instr:%s\n", instr);
//strcpy(instr,"blabla"); ILLEGAL!  max. strlen(instr) chars are passed back
// correct:
strcpy(resp, instr); strcat(resp, instr);
return(resp); 
/* resp: will be copied to new python string object before return
Warning:
clientpy.c:152: warning: function returns address of local variable
can be ignored
*/
}

