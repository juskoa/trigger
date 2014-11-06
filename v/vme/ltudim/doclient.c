int WAITING=1;

char DETNAME[10];
char DNDO[20];   // DETNAME/DO
void dbgprt() {
printf("%s", emg);
}
/*--------------------------------------------------*/ int waitinfocall() {
/* rc: 1: ok   0: timeout */
#define MAX10ms 900
int ix,rc;
for(ix=0; ix<=MAX10ms; ix++) {   // 10 secs =1000 x 10ms
  if(WAITING) {
    usleep(10000);
  } else {
    break;
  };
};
if(ix>=MAX10ms) {
  char errmsg[ERRMSGL];
  /*sprintf(errmsg,"DBGwaitinfocall: timeout %d ms", ix*10); 
  prerr(errmsg); */
  rc=0;
} else {
  //printf("DBGwaitinfocall:%d ms\n",ix*10);
  rc=1;
};
return(rc);
}

/*----------------------------------*/void callback(void *tag, int *rc) {
//printf("callback tag:%d rc:%d\n", *tag, *rc);
if(*rc != 1) {
  char errmsg[ERRMSGL];
  sprintf(errmsg,"callback for tag:%d not executed by server %s. rc:%d",
    *(int *)tag, DETNAME, *rc);
  prerr(errmsg); EXIT=1;
}; 
}
/*-----------------------------*/ int execute(char *cmd, char *inpline) {
/* rc: 1:ok   0: not executed */
int rc;
WAITING=1; /* wait for infocallback: */
rc= dic_cmnd_callback(cmd, inpline, strlen(inpline)+1, callback, 33);
//rc= dic_cmnd_callback(DNDO, inpline, strlen(inpline)+1, callback, TAGdo);
sprintf(emg, "DBGexecute:%s:%d:%s<\n",cmd, rc, inpline); dbgprt();
rc= waitinfocall();
return(rc);
}

/*----------------------------------*/ int main(int argc, char **argv) {
int i,rc;
int RESULTid; char *environ;
char inpline[MAXLILE];
if( argc<2 ) {
  printf("Start client with 1 parameter:\n\
ltuclient DETECTOR_NAME\n\
ltuclient DETECTOR_NAME/EXIT\n");
  exit(4);
};
rc= gethostname(clienthostname, 20);
if(rc!=0) {
  char errmsg[ERRMSGL];
  sprintf(errmsg,"gethostname rc: %d got:%s",rc, clienthostname);
  prerr(errmsg);
};
strncpy(DETNAME, argv[1], 9);

setlinebuf(stdout);
strcpy(DNDO,DETNAME); strcat(DNDO, "/DO");
/* Before all, we have to register with /RESULT service: */
strcpy(cmd,DETNAME); strcat(cmd, "/RESULT");
WAITING=1;
RESULTid= dic_info_service(cmd, MONITORED, 0, result,MAXRESULT+1,
  infocallback, 136, resultFailed, strlen(resultFailed)+1); 
waitinfocall();
rc= execute(cmd, inpline);
if(rc != 1) {
  prerr("/PIPE open not successfull"); goto EXIT4;
};
while(1) {
  char *fgetsrc;
  fgetsrc=fgets(inpline, MAXLILE, stdin);
  if(fgetsrc==NULL) {
    strcpy(emg, "NULL fgets on input\n");
    dbgprt() ; break;
  };
  if(strcmp(inpline,"qc\n")==0) {
    strcpy(cmd,DETNAME); strcat(cmd, "/PIPE"); 
    sprintf(inpline,"close %s %s\n", DETNAME, LTU_CLIENT_VERSION);
    rc= execute(cmd, inpline);
    break;
  } else {
    execute(DNDO, inpline);
  };
  if(EXIT>0) break;
};
dic_release_service(RESULTid);
sleep(2);
} 

