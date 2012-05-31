/* dimcswtrg.c -client example requesting SW triggers */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h> 

#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif
#include "dimctypes.h"
 
#define SERVERNAME "CTPDIM"
#define MAXRESULT 800
#define MAXLILE 200
char  SWTRGcmd[20]=SERVERNAME;
char STATUScmd[20]=SERVERNAME;
char DOcmd[20]=SERVERNAME;
char RESULTsrv[20]=SERVERNAME;
char inpline[MAXLILE];
char result[MAXRESULT];
char resultFailed[]="ERROR: server down.";

/* User routine activated in response to CTPDIM/STATUS or
   CTPDIM/SWTRG commands or an asynchronous event in CTPDIM server
   (for example "server down").
CTPDIM/SWTRG command returns 1 of the following strings:

"SWTRG X:x"
if command was successfully executed.  X is detector name,
x is the number of generated Software triggers 
(i.e.L0,L1,L2a was generated for each calibration trigger) 
in response to CTPDIM/SWTRG command.
NB: The number of generated triggers can be lower than requested.
    if x <requested_number_of_triggers, your client should
    repeat the SWTRG command.

"ERROR: error_message" if error was encountered, which
prevented the execution of the command. Possible errors:   
"ERROR: Unknown detector XXX" 
"ERROR: server down."

CTPDIM/STATUS command has no use for now. 
It returns string with current CTP cabling
(accessed through ECS/DAQ database too):
"STATUS: 
name FB        
...
"
where:
name is the name of detector followed by chars:
F -if detector is connected to fanout
B -if detector BUSY input is connected to CTP
*/
void gotresult(void *tag, void *buffer, int *size) {
//printf("gotresult tag:%d size:%d buffer:%s\n", *tag, *size, buffer);
printf("gotresult:%s<\n", (char *)buffer);
}

void callback(void *tag, int *rc) {
//printf("callback tag:%d rc:%d\n", *(int *)tag, *rc);
if(*rc != 1) {
  printf("Command:%s %s not executed by server\n",inpline,inpline);
}; 
}

int findspace(char *inpline){
int i;
for(i=0; i<(int)strlen(inpline); i++) {
  if(inpline[i]==' ') break;
  if(inpline[i]=='\n') {i=-1; break; };
  if(inpline[i]=='\0') {i=-1; break; };
};
return(i);
}

void execute() {
int rc;
//printf("executing:%s\n",inpline);
if(inpline[0]=='S') {
  rc= dic_cmnd_callback(STATUScmd, &inpline[2], 
    strlen(inpline)+1-2, callback, 33);
} else if(inpline[0]=='T'){
  Tswtrg trgmsg;
  trgmsg.N=5;   //default
  if(strlen(inpline) < 4) {   // 'T d' +NL =4chars (minimum if det given)
    strcpy(trgmsg.name,"HMPID");
  } else {    // copy detector name
    int i;
    for(i=0; i<=8; i++) {   //max. 8chars in name
      char c;
      c= inpline[i+2];
      if((c=='\0') || (c=='\n') || (c==' ') || (i==8)) {
        trgmsg.name[i]= '\0';
        break;
      };
      trgmsg.name[i]= c;
    };
    i=findspace(&inpline[2]);
    if(i>0) {
      int n;
      n= strtol(&inpline[i+2], NULL,10);
      printf("space ix:%d n:%d\n", i, n);
      if(errno != 0) {
        printf("bad number in line:%s\n",inpline);
      } else {
        trgmsg.N=n;
      };
    };
  };
  strcpy(trgmsg.pf,"pfX");
  strcpy(trgmsg.bcmask,"bcmaskX");
  trgmsg.type='c';
  trgmsg.roc=0xa; 
  printf("SWTRG to %s. Type:%c ROC:0x%x N:%d\n", 
    trgmsg.name, trgmsg.type, trgmsg.roc, trgmsg.N);
  rc= dic_cmnd_callback(SWTRGcmd, (int *)&trgmsg, 
    sizeof(trgmsg), callback, 35);
} else {
  rc= dic_cmnd_service(DOcmd, &inpline[2], strlen(inpline)+1-2);
  printf("rc:%d\n",rc);
};
}

int main(int argc, char **argv) {
int inforc;
strcat(SWTRGcmd, "/SWTRG");
strcat(STATUScmd, "/STATUS");
strcat(DOcmd, "/DO");
strcat(RESULTsrv, "/RESULT");
/* CTPDIM/RESULT service returns the information about the status
   of your client kept by CTPDIM server. This information is
   solicited by CTPDIM/STATUS command (not used in this version)
   or CTPDIM/SWTRG command. 
*/
inforc= dic_info_service(RESULTsrv, MONITORED, 0, result,MAXRESULT,
  gotresult, 136, resultFailed, strlen(resultFailed)+1); 
while(1) {
  printf("\n\
S LTUS      -send 'STATUS LTUS' command\n\
S ALLRARE   -send 'STATUS ALLRARE' command\n\
D ALL       -send 'DO ALL' command (no callback)\n\
D RARE      -send 'DO RARE' command (no callback)\n\
D W secs    -send 'DO W secs' cmd  (callback)\n\
D actrs     -send 'DO Ractrs' cmd  (no callback)\n\
D qq        -send 'DO qq' cmd -styops server\n\
T detname N -send '%s' command (HMPID is default detname) requesting N triggers\n\
q           -quit client\n:", SWTRGcmd);
  fgets(inpline, MAXLILE, stdin);
  if(strcmp(inpline,"q\n")==0) break;
  execute();
};
dic_release_service(inforc);
sleep(2);
return(0);
} 
