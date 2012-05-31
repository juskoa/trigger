/* allrare_client.c -client example setting/getting ALL/RARE condition  in CTP*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h> 
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif
 
#define SERVERNAME "CTPDIM"
#define MAXRESULT 800
#define MAXLILE 200

char STATUScmd[20]=SERVERNAME;
char DOcmd[20]=SERVERNAME;
char RESULTsrv[20]=SERVERNAME;
char inpline[MAXLILE];
char result[MAXRESULT]="balabl";
char resultFailed[]="ERROR: server down.";

/* User routine activated in response to CTPDIM/STATUS ALLRARE
String returned in buffer:
STATUS:ALL
STATUS:RARE
ERROR: error message
---------------------*/ void gotresult(void *tag, void *buffer, int *size) {
//printf("gotresult tag:%d size:%d buffer:%s\n", *tag, *size, buffer);
printf("gotresult:%s<\n", (char *)buffer); printf("result:%s<\n", result);
}

/*----------------------------*/ void callback(void *tag, int *rc) {
//printf("callback tag:%d rc:%d\n", *tag, *rc);
if(*rc != 1) {
  printf("Command:%s %s not executed by server\n",inpline,inpline);
}; 
}

/*-----------------------------------------------*/ void execute() {
int rc;
//printf("executing:%s\n",inpline);
if(inpline[0]=='S') {
  rc= dic_cmnd_callback(STATUScmd, (void *)"ALLRARE",
    strlen("ALLRARE")+1, callback, 33);
} else {
  char *nl;
  nl= strchr(inpline,'\n');    // remove NL
  if(nl==NULL) {
    inpline[0]='\0';
  } else {
    *nl='\0';
  };
  rc= dic_cmnd_service(DOcmd, inpline, strlen(inpline)+1);
};
printf("rc:%d\n",rc);
}

/*------------------------------------*/ int main(int argc, char **argv) {
int inforc;
strcat(STATUScmd, "/STATUS");
strcat(DOcmd, "/DO");
strcat(RESULTsrv, "/RESULT");
/* CTPDIM/RESULT service returns the information about the status
   of your client kept by CTPDIM server. This information is
   solicited by CTPDIM/STATUS ALLRARE command 
*/
inforc= dic_info_service(RESULTsrv, MONITORED, 0, result,MAXRESULT,
  gotresult, 136, resultFailed, strlen(resultFailed)+1); 
while(1) {
  printf("\n\
S         -send 'STATUS ALLRARE' command\n\
ALL       -send 'DO ALL' command (no callback)\n\
RARE      -send 'DO RARE' command (no callback)\n\
q         -quit client\n:");
  fgets(inpline, MAXLILE, stdin);
  if(strcmp(inpline,"q\n")==0) break;
  execute();
};
dic_release_service(inforc);
sleep(2);
return(0);
} 
