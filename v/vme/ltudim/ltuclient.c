/* ltuclient.c
28.1.2007:
prerr now OK (before without NL:NL )
*/
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
 
#define LTU_CLIENT_VERSION "2.10"   /* see v/SPECS/ltuclient.spec */
#define notinitialised "Server restarted."
#define MAXRESULT 1500
#define MAXLILE 200
#define SERVERCWDLEN 100
#define SERVERNAMELEN 100
#define ERRMSGL 200
#define MAXFNL 44

#define TAGdo 33
#define TAGputfile 34
#define TAGvmeopmr32 35

int VMEOPMR32=0;
int WAITING=1;
int WAITINGFC=0;   // 1: waiting for last message after getting file
int LOGGING=1;     // 1:log   0: no log
int EXIT=0;        // stop client if >1
char clienthostname[SERVERNAMELEN]="";
char servername[SERVERNAMELEN]="";
char servercwd[SERVERCWDLEN]="";
char clientcwd[SERVERCWDLEN]="";
FILE *GETFILE=NULL;
char DETNAME[10];
char DNDO[20];   // DETNAME/DO
char DNPUTFILE[20];   // DETNAME/PUTFILE
char DNGETFILE[20];   // DETNAME/GETFILE
char cmd[80];
char result[MAXRESULT+1]="empty";   // final \0 included
char resultFailed[]="failed...";
FILE *dbgfile=NULL;
char emg[400];

int execute(char *cmd, char *inpline);

/*------------------------------------------*/ void dbgprt() {
if(LOGGING==0) return;
if(dbgfile==NULL) {
  char pid[40], fn[80];
  sprintf(fn, "WORK/ltuclient_%d.log",getpid());
  dbgfile= fopen(fn,"w");
  if(dbgfile==NULL) {
    printf("ERROR: %s file cannot be opened for write\n", fn); 
    printf("%s", emg);
    fflush(stdout);
    sleep(1);
    exit(8);
  };
  setlinebuf(dbgfile);
};
fprintf(dbgfile, "%s", emg); fflush(dbgfile);
}
/*------------------------------------------*/ void prerr(char *msg) {
/* \n:\n important -prerr used for error messages with ltu_u.py,
which expects NL:NL at the end */
sprintf(emg, "ERROR:%s\n:\n", msg); dbgprt();
printf("ERROR:%s\n:\n", msg); fflush(stdout);
}

/*---------------------------*/ int getnlines(char *msg) {
int ix, n=msg[0]-'0';
for(ix=1; ix<(MAXFNL-1); ix++) {
  if(msg[ix]==')') break;
  if(msg[ix]=='\0') break;
  n= n*10 + (msg[ix]-'0');
};
//printf("getnlines:%s %d\n",msg, n);
return(n);
}
/*---------------------------*/ int getfilename(char *msg, char *fname) {
/* msg:inp: first char of filename finished by " or \0
   fname:out: fname (" peeled off)
*/
int ix, ix2;
for(ix=0; ix<(MAXFNL-1); ix++) {
  ix2=ix;
  if(msg[ix2]=='"') break;
  if(msg[ix2]=='\0') break;
  fname[ix]=msg[ix2];
  fname[ix+1]='\0';
};
return(0);
}

/*----------------------------------*/void cb_putfile(void *tag, int *rc) {
//printf("cb_putfile tag:%d rc:%d\n", *tag, *rc);
if(*rc != 1) {
  char errmsg[ERRMSGL];
  sprintf(errmsg,"cb_putfile for tag:%d not executed by server. rc:%d",
    *(int *)tag, *rc); prerr(errmsg);
}; 
}

/*--------------------------------------------------*/ int waitinfocall() {
/* rc: 1: ok   0: timeout */
#define MAX10ms 500
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

/*---------------------------------------------*/ void getfile(char *fname) {
int rc, ix=1;
FILE *file;
char errmsg[ERRMSGL];
char buffer[MAXRESULT+1]="abcdefghijkl";
  /*sprintf(errmsg,"getfile:%s opening for write. clienthost:%s", 
    fname, clienthostname);
  printf("%s\n",errmsg);*/
if(strcmp(clienthostname,servername)==0) {
  // server == client, if the same VMEWORKDIRs, do not transfer file!
  GETFILE=NULL;
  //sprintf(emg,"DBGgetfile: client on server, can be problem if WORK dirs are the same\n");
  //dbgprt();
/* from 28.6., ltuclient has special WORK directory (~/ltuclient), which
   is very likely different from server's 
  return; */
/* from 5.5.2008: do not transfer if the same file */
  if(strcmp(servercwd,clientcwd)==0) {
    sprintf(errmsg," server:%s:%s\n client:%s:%s\n:\n",
      servername,servercwd,clienthostname,clientcwd);
    printf("%s", errmsg); fflush(stdout);
    GETFILE=NULL; return;
  };
}; 
GETFILE= fopen(fname,"w");
if(GETFILE ==NULL) {
  sprintf(errmsg,"getfile:%s cannot be opened for write", fname);
  prerr(errmsg);
  return;
};
sprintf(emg,"DBGgetfile:%s\n", fname); dbgprt();
rc=execute(DNGETFILE, fname);
}

/*---------------------------------------------*/ void putfile(char *fname) {
int rc, ix=1;
FILE *file;
char errmsg[ERRMSGL];
char buffer[MAXRESULT+1];
if(strcmp(clienthostname,servername)==0) {
  file=NULL;
  if(strcmp(servercwd,clientcwd)==0) {
    sprintf(errmsg," server:%s:%s\n client:%s:%s\n:\n",
      servername,servercwd,clienthostname,clientcwd);
    printf("%s", errmsg); fflush(stdout);
    file=NULL; return;
  //sprintf(errmsg,"putfile:client on server. Are WORK dirs different? file:%s", fname );
  //printf("%s\n", errmsg);
  };
};
file= fopen(fname,"r");
if(file ==NULL) {
  sprintf(errmsg, "%s -cannot be open for read", fname);
  prerr(errmsg);
  return;
};
rc= dic_cmnd_callback(DNPUTFILE, fname, strlen(fname)+1, cb_putfile, 34);
while(1) {
  int lng;
  //lng=ix; if(ix>3) lng=0;
  if(feof(file) || ferror(file)) {
    fclose(file); lng=0;
  } else {
    lng= fread(buffer, 1, MAXRESULT, file);
  };
  rc= dic_cmnd_callback(DNPUTFILE, buffer, lng, cb_putfile, TAGputfile);
  // rc:1 if OK
  //printf("putfile. rc:%d after sent:%d bytes\n", rc, lng);
  if(lng==0) break;
  //ix++;
};
printf("%s sent\n", fname);
printf(":\n");
}

int FIRSTCALLB=0;
/*------------------*/ void infocallback(void *tag, void *bufferv, int *size) {
char buf1[81]; char *buffer= (char *)bufferv;
/*
strncpy(buf1,buffer,80); buf1[80]='\0'; if(*size<80) buf1[*size]='\0';
sprintf(emg,"DBGinfcb:%d:%d:%s\n", *tag, *size, buf1); dbgprt();
*/
if(*size>MAXRESULT) {   // has to be > (== is not an error)
  sprintf(emg,"long message(%d bytes)", *size);
  prerr(emg);   //dbgprt() perhaps better?
};

if(servername[0]=='\0') {   // we use only 1 server, enough 1st time
  int rc,nil;
  nil= strlen(notinitialised);
  if(strncmp(buffer,notinitialised, nil)==0) {
    strcpy(servercwd, &buffer[nil]);
    servercwd[strlen(servercwd)-1]='\0';   // no NL
  };
  rc= dic_get_server(servername);
  //printf("rc:%d servername:%s clienthostname:%s\n", rc,servername,clienthostname);
//rc:2 servername:TRIGGER::LTU-HMPID@altri1 clienthostname:altri1
  if(rc==0) {
    prerr((char*)("infocallback error"));
  } else {
    int ix;
    for(ix=0; ix<SERVERNAMELEN; ix++) {
      char c;
      c=servername[ix];
      if(c=='@') {
        int ixx;
        for(ixx=0; ixx<SERVERNAMELEN; ixx++) {
          c=servername[ix+ixx+1];
          if( (c!=' ') && (c!='.') && (c!='\0') ) {
            servername[ixx]=c;
          } else {
            servername[ixx]='\0';
          };
        };
      };
    };
  };
};
  //printf("servername:%s clienthostname:%s\n", servername,clienthostname);
buffer[*size]='\0';
if(GETFILE!=NULL) {
  //printf("getfile:%d:%10.10s\n", *size,buffer);
  if(*size==0) {
    fclose(GETFILE); GETFILE=NULL;
    WAITINGFC=1;
  } else {
    fwrite(buffer, *size, 1, GETFILE); 
  };
} else if(WAITINGFC==1) {
    WAITING=0;
    WAITINGFC=0;
    printf(":\n");
} else {
  if(FIRSTCALLB==0) {
    FIRSTCALLB=0;
  } else if(FIRSTCALLB==1) {
    if(strncmp(buffer, "PID ",4)==0) {
      int ix,restmsg;
      for(ix=4; ix<strlen(buffer); ix++) {  // find first char after "PID N "
        char c;
        c= buffer[ix];
        if(c ==' ') {restmsg=ix+1; buffer[ix]='\0'; break; };
        if(c =='\0') {restmsg=ix; break; };
      };
      printf("%s ltuclient:%s %s", 
        buffer, LTU_CLIENT_VERSION, &buffer[restmsg]);
    } else {
      printf("ERROR: PID expected as first line, but got:%s\n", buffer);
    };
  } else {
    printf("%s", buffer);
  };
  FIRSTCALLB++;
};
WAITING=0;
}

/* The purpose of cbEXEC, executeEXEC (start with '/EXEC EXEC' after:
cd $VMEWORKDIR
$VMECFDIR/ltudim/linux/ltuclient daq
) 
is to test dic_info_service() -to be used with ltu/EXEC service -
see ltu_proxy/ltudimservices.c
*/
/*--------------------------*/void cbEXEC(void *tag, void *buf, int *size) {
sprintf(emg, "cbEXEC tag:%d msg:%s size:%d\n", *(int *)tag, (char *)buf, *size);
dbgprt();
}
/*-----------------------------*/ int executeEXEC(char *cmd) {
/* rc: 1:ok   0: not executed */
int rc=1, execid;
strcpy(result,resultFailed);
execid= dic_info_service(cmd, ONCE_ONLY, 2, result , MAXRESULT,
      cbEXEC, 3488, resultFailed, strlen(resultFailed)+1); 
//      NULL, 3488, resultFailed, strlen(resultFailed)+1); 
sprintf(emg, "executeEXEC1:cmd:%s:execid:%d result:%s<\n",cmd, execid, result); dbgprt();
sleep(8);   /* has to be here (+ check for timeout, or use cbEXEC */
sprintf(emg, "executeEXEC2:cmd:%s:execid:%d result:%s<\n",cmd, execid, result); dbgprt();
if(strcmp(result,resultFailed)==0) rc=0;
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
rc= dic_cmnd_callback(cmd, inpline, strlen(inpline)+1, callback, TAGdo);
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
rc= gethostname(clienthostname, SERVERNAMELEN);
if(rc!=0) {
  char errmsg[ERRMSGL];
  sprintf(errmsg,"gethostname rc: %d got:%s",rc, clienthostname);
  prerr(errmsg);
};
environ= getenv("VMEWORKDIR"); 
if(environ ==NULL) {
  char errmsg[ERRMSGL];
  sprintf(errmsg,"%s","VMEWORKDIR not defined");
  prerr(errmsg);
}else {
  strcpy(clientcwd, environ);
};
strncpy(DETNAME, argv[1], 9);
/*if(argc==3) {
  strncpy(inpline, argv[2], 80); inpline[79]='\0';
} else {
  strcpy(inpline, "");
};*/

//printf("qc -quit this client (q closes pipe on server)\n");
setlinebuf(stdout);
VMEOPMR32=0;
strcpy(DNDO,DETNAME); strcat(DNDO, "/DO");
strcpy(DNPUTFILE,DETNAME); strcat(DNPUTFILE, "/PUTFILE");
strcpy(DNGETFILE,DETNAME); strcat(DNGETFILE, "/GETFILE");

/* Before all, we have to register with /RESULT service: */
strcpy(cmd,DETNAME); strcat(cmd, "/RESULT");
WAITING=1;
RESULTid= dic_info_service(cmd, MONITORED, 0, result,MAXRESULT+1,
  infocallback, 136, resultFailed, strlen(resultFailed)+1); 
waitinfocall();

strcpy(cmd,DETNAME); strcat(cmd, "/PIPE"); 
//strcpy(inpline,"open "); strcat(inpline, DETNAME); strcat(inpline,"\n");
sprintf(inpline,"open %s %s\n", DETNAME, LTU_CLIENT_VERSION);
/*WAITING=1;   // always set to 1 here !
rc= dic_cmnd_callback(cmd, inpline, strlen(inpline)+1, callback,33); 
if(rc != 1) {
  prerr("/PIPE open not successfull"); goto EXIT4;
};
waitinfocall();*/
rc= execute(cmd, inpline);
if(rc != 1) {
  prerr((char*)"/PIPE open not successfull"); goto EXIT4;
};

//printf("dic_info_service:rc:%d result:%s:\n", RESULTid, result);
/* if(strncmp(result,"PID ",4)!=0) {
  char errmsg[ERRMSGL];
  sprintf(errmsg,"PID expected...:%s", result);
  prerr(errmsg); 
  goto EXIT4; 
};*/
while(1) {
  char *fgetsrc;
  fgetsrc=fgets(inpline, MAXLILE, stdin);
  if(fgetsrc==NULL) {
    strcpy(emg, "NULL fgets on input\n");
    dbgprt() ; break;
  };
  //printf("DBG>:%s<GBD\n", inpline); fflush(stdout);
  if(strcmp(inpline,"q\n")==0) break;  // never quit server
  /* q:
  q or qc: quit only client
  qc - close pipe on server (do not quit server)
  qs: close pipe + EXIT server
  */
  if(strcmp(inpline,"qc\n")==0) {
    strcpy(cmd,DETNAME); strcat(cmd, "/PIPE"); 
    sprintf(inpline,"close %s %s\n", DETNAME, LTU_CLIENT_VERSION);
    rc= execute(cmd, inpline);
    break;
  /*
  if(VMEOPMR32>0) NACOTOTO?
    rc= dic_cmnd_callback(DNDO, inpline, strlen(inpline)+1, 
      callback, TAGvmeopmr32);
    VMEOPMR32--;
    if(VMEOPMR32==0) {
      WAITING=1;
      waitinfocall();
    }; NACOTOTO
  */
  } else if(inpline[0]=='/') {
    int ix,ix2, ixeofcmd,inplinelen,waserror=0;
    int exec=0, ixafterslash;
    if(strncmp(inpline,"/EXEC",5)==0) {
      exec=1;
    };
    /* inpline: /abc def       ->
       cmd:     DETNAME/abc 
       inpline: def
    */
    inplinelen= strlen(inpline);
    strcpy(cmd,DETNAME); strcat(cmd, "/"); 
    ixeofcmd=strlen(cmd); ixafterslash= ixeofcmd;
    for(ix=1; ix<inplinelen; ix++) {
      char c;
      c= inpline[ix];
      if(c=='\0') {
        prerr((char *)"bad input / line"); waserror=1; break;
      } else if(c==' ') {
        ix++; break;   // ix points on 1st char to be in inpline
      } else {
        cmd[ixeofcmd]= c; ixeofcmd++; cmd[ixeofcmd]= '\0'; 
      }
    };
//strcat(cmd, "/PIPE"); 
    if(waserror) continue;
    for(ix2=0; ix2<inplinelen; ix2++) {
      inpline[ix2]= inpline[ix2+ix];
    };
    if(exec==1) {
      /* incorrect usage!  i.e. '/EXEC EXEC'
      strcpy(&cmd[ixafterslash], inpline);
      cmd[strlen(cmd)-1]='\0';   // cut off NL
      */
      rc= executeEXEC(cmd);
    } else {
      rc= execute(cmd, inpline);
    };
    if(rc != 1) {
      char msg[200];
      sprintf(msg, "%s %s not successfull",cmd,inpline); 
      prerr(msg); goto EXIT4;
    };
  } else if(strncmp(inpline,"vmeopmr32(",10)==0) {
    int rc, ix,inleng;
    char linput[MAXRESULT];
    VMEOPMR32=getnlines(&inpline[10]);
    /* send all the input in 1 go: */
    ix=0; inleng=0;
    inleng= strlen(inpline); strcpy(&linput[ix], inpline); ix= ix+ inleng; 
    for(rc=0; rc<VMEOPMR32; rc++) {
      fgets(inpline, MAXLILE, stdin);
      inleng= strlen(inpline);
      if( (ix+inleng)>MAXRESULT) {
        prerr((char *)"Too long input in vmeopmr32()"); break;
      };
      strcpy(&linput[ix], inpline); 
      ix= ix+ inleng; 
    };
    execute(DNDO, inpline);
    /*WAITING=1;
    rc= dic_cmnd_callback(DNDO, linput, strlen(linput)+1, 
      callback, TAGvmeopmr32);
    waitinfocall(); */
  } else if(strncmp(inpline,"getfile(",8)==0) {
    char fname[MAXFNL];
    getfilename(&inpline[9], fname);
    getfile(fname);
  } else if(strncmp(inpline,"putfile(",8)==0) {
    char fname[MAXFNL];
    getfilename(&inpline[9], fname);
    putfile(fname);
  } else {
    if(strcmp(inpline,"logon\n")==0) {
      LOGGING=1; strcpy(emg, inpline); dbgprt();
    } else if(strcmp(inpline,"logoff\n")==0) {
      strcpy(emg, inpline); dbgprt(); LOGGING=0;
    };
    execute(DNDO, inpline);
  };
  if(EXIT>0) break;
  if(strcmp(inpline,"qs\n")==0) break;
};
EXIT4:
/* strcpy(inpline,"release");
rc= dic_cmnd_service("DN/PIPE", inpline, strlen(inpline)+1);  */
dic_release_service(RESULTid);
if(dbgfile !=NULL) fclose(dbgfile);
sleep(2);
} 
