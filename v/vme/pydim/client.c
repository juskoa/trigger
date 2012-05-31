/* client.c
Start: linux/client servername/cmd
stdin: 
1 line message to be sent to servername/cmd: "rcfg partname NNN 0xmask clu1..."
or

stdout: OK (server exists)
        Error -cmd not executed by server (unknow cmd)
        EOF   -end of stdin
        quit  -"q\n" received from stdin
 */
#include <stdio.h>
#include <string.h>
#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif
 
#define OCCMD "STATUS\n"
#define STATLEN strlen(OCCMD)
#define MAXSTATUS 100 
#define MAXMESSAGE 400

char cmd[80];
char message[MAXMESSAGE];

char StatusString[MAXSTATUS]="blabla";   
char StatusFailed[MAXSTATUS]="Status failed";   /* /STATUS service failed */

void printhelp() {
printf("Enter 1 line. examples:\n\
qc         -quit client\n\
qs         -quit server (i.e. empty line sent)\n\
int1       -test INT1  (or 2 with int2)\n\
clockshift ttcmi corde last_shift    (3 dec numbers see $dbctp/clockshift)\n\
cs         -test CTPRCFG/CS\n\
csupdate   -test CTPRCFG/CS update\n\
intupdate  -test CTPRCFG/INT1,2 update\n\
rcfg partname runNumber 0xmask clu1 clu2 ... clu6 cla1 ... cla50\n\
STATUS     -to demonstrate DIM response when service used for command\n\
");
}

void callback(void *tag, int *rc) {
//printf("callback tag:%d rc:%d\n", *(int *)tag, *rc);
if(*rc == 1) {
  //printf("%s %s OK\n",cmd,message);
  printf("Callback: OK\n");
} else {
  //printf("%s %s not executed by server\n",cmd,message);
  printf("Callback: Error, command not sent\n");
};
}
void CScallback(void *tag, void *buffer, int *size) {
char *buf= (char *)buffer;
printf("CScallback tag:%d size:%d buf200:%200.200s\n", *(int *)tag, *size, buf);
//printf("CScallback:%s\n",StatusString); //not changed (callback)
}

int main(int argc, char **argv) {
int rc=0, csinfo;
if((argc<2) || (argc>3)) {
  printf("Usage:    linux/client servername/command\n");
  printf("  i.e.      linux/client CTPRCFG/RCFG\n");
  printf("or:\n");
  printf("          linux/client CTPRCFG/RCFG stop\n");
  printf("  -sends empty line to CTPRCFG/RCFG\n");
  printf("or:\n");
  printf("          linux/client CTPRCFG/RCFG csupdate\n");
  printf("          linux/client CTPRCFG/RCFG intupdate\n");
  printf("  -sends 'csupdate' line to CTPRCFG/RCFG (update CTPRCFG/CS service)\n");
  return(4);
};
strncpy(cmd, argv[1], 80);
if(argc==3) {
  if(strcmp(argv[2],"stop")==0) {
    strcpy(message,"\n");
    rc= dic_cmnd_callback(cmd, message, strlen(message)+1, callback, 33);
    printf("rc:%d\n", rc);
    usleep(1000000);
  } else if((strcmp(argv[2],"csupdate")==0)||(strcmp(argv[2],"intupdate")==0)){
    strcpy(message,argv[2]); strcat(message,"\n");
    rc= dic_cmnd_callback(cmd, message, strlen(message)+1, callback, 33);
    printf("argv2:%s rc:%d\n", argv[2], rc);
    usleep(1000000);
  }; return(rc);
};
csinfo= dic_info_service("CTPRCFG/CS", MONITORED, 0, 
  NULL,MAXSTATUS+1, 
  CScallback, 3488, StatusFailed, strlen(StatusFailed)+1);
printf("CTPRCFG/CS MONITORED started:%d:%s:\n", csinfo, StatusString);

setlinebuf(stdout);
printhelp();
while(1) {
  char *mrc;
  mrc= fgets(message, MAXMESSAGE, stdin);
  if(mrc==NULL) {
    printf("EOF\n"); break;
  };
  if(strcmp(message,"\n")==0) {
    printhelp(); continue;
  };
  if(strcmp(message,"qc\n")==0) {
    printf("quit...\n"); break;
  };
  if(strcmp(message,"qs\n")==0) {
    strcpy(message,"\n");
  };
  if(strcmp(message,"int1\n")==0) {
    rc= dic_info_service("CTPRCFG/INT1", ONCE_ONLY, 1, 
      StatusString,MAXSTATUS+1, 
      NULL, 3488, StatusFailed, strlen(StatusFailed)+1);
    usleep(1000000);
    printf("%s INT1:%s:\n",cmd, StatusString);
  } else if(strcmp(message,"int2\n")==0) {
    rc= dic_info_service("CTPRCFG/INT2", ONCE_ONLY, 1, 
      StatusString,MAXSTATUS+1, 
      NULL, 3488, StatusFailed, strlen(StatusFailed)+1);
    usleep(1000000);
    printf("%s INT2:%s:\n",cmd, StatusString);
  } else if( (strlen(message)==STATLEN) &&      // get status
      (strcmp(message, OCCMD)==0) ) {
    // service 
    rc= dic_info_service(cmd, ONCE_ONLY, 1, StatusString,MAXSTATUS+1, 
        NULL, 3488, StatusFailed, MAXSTATUS+1);
    usleep(1000000);
    printf("%s Status:%s:\n",cmd, StatusString);
  } else if(strcmp(message,"cs\n")==0) {
    rc= dic_info_service("CTPRCFG/CS", ONCE_ONLY, 1, 
      StatusString,MAXSTATUS+1, 
      NULL, 3488, StatusFailed, strlen(StatusFailed)+1);
    usleep(1000000);
    printf("%s CS:%s:\n",cmd, StatusString);
  } else {                           // send command
    // command:
    // clockshift ttcmi_halfns cordedelPS/10 last_measuredshiftPS*10
    // rc= dic_cmnd_callback(cmd, ctpinput, strlen(ctpinput)+1, callback, 33);
    rc= dic_cmnd_callback(cmd, message, strlen(message)+1, callback, 33);
    printf("dic_cmnd_callback rc:%d (1:ok, but callback print is relevant)\n", rc);
  };
};
//printf("rc:%d\n",rc); 
//sleep(1);
dic_release_service(csinfo);
return(rc);
} 
