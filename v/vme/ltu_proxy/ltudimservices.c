/* ltudimservices.c */
#ifdef CPLUSPLUS
#include <dis.hxx>
#else
#include <dis.h>
#endif
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <fcntl.h>
#include <sys/shm.h>
#include "vmewrap.h"
#include "lexan.h"
#include "ltu.h"

#define MAXFNL 44
#define MAXCMDL 200
#define ERRMSGL 200
#define MAXRESULT 1500
#define MAXLILE 1500
#define MAXCIDAT 80
#define MAXMCclients 12 

#define TAGcmdDO 18
#define TAGcmdPIPE 19
#define TAGputfile 20
#define TAGcthread 21
#define TAGcmdCMD 22
#define notinitialised "Server restarted."

int QUIT=0;     // 1: stop serving
int GETFILEC=0;
unsigned int RESULTid;
unsigned int EXECid;
unsigned int CALIBBCid;

// counters reading:
unsigned int COUNTERSid;
w32 *buf1=NULL;

int actcid=0;         /* active client id. 0: nobody active */
FILE *PUTFILE;   // !=NULL upload active
FILE *GETFILE;   // !=NULL download active
int LOGGING=1;   // 1:logon 0: logoff
FILE *logfile=NULL;
char servercwd[80];

char RWMODE='W'; // 'R': read only    'W': in STDALONE_STOPPED mode
char actcidat[80];  /* active client: pid@host */

// Info about all clients, i.e.:
// - using MONCOUNTERS service or
// - using RESULT
// is kept in MCclients table:
// see updateNMCclients()
typedef struct mcc{
  int cid;
  char cidat[MAXCIDAT];    // free item if cidat[0]=='\0'
  char type ;   // 'm' -MONCOUNTERS   'r' -result
} TMCclient;

int NMCclients;
TMCclient MCclients[MAXMCclients];
//-------------------------------------------------

int infp, outfp;
int popen2active=0;   // 1 if infp/outfp opened
int firstcaba=1;
char ResultString[MAXRESULT+1]; //"Server restarted.VMEWORKDIR\n";
char foreignmsg[]="Error: service /RESULT used by other client\n";

void getdatetime(char *);
pid_t popen2(const char *baseaddr, int *infp, int *outfp);
void ds_stop();

void UPPER(char *name);
w32 *mallocShared(w32 shmkey, int LTUNall, int *segid);

/*---------------------------------*/ void dimlogprt(char *typs, char *msg) {
/* Log server's message
typs:
  >   -message arrived from client ("NL" has to be at the end)
  <   -message sent to client ("NL:NL" should be trailing)
  ERROR -error message
  other -special (server's warning or info message)
*/
int lm;
char line[MAXLILE+1];
char mytyp[40];
char dattim[20];
getdatetime(dattim);
if(LOGGING==0) return;
if(logfile==NULL) {
  logfile= fopen("WORK/ltudimserver.log","w");
  if(logfile==NULL) {
    perror("Cannot open logfile: WORK/ltudimserver.log for write\n");
    return;
  };
  setlinebuf(logfile);
};

lm= strlen(msg);
if(lm >=MAXLILE) {
  fprintf(logfile, "%s:dimlogprt: too long message", dattim); 
  lm=MAXLILE; lm=80; }; // don't bother with more than 80 (anyhow long)
strncpy(line, msg, lm+1);
if(strcmp(typs,">")==0) {   //replace compulsory trailing NL by >
  strcpy(mytyp,">");
  if(line[lm-1]=='\n') {
    line[lm-1]='>';
  } else {
    fprintf(logfile, "%s:dimlogprt: NL missing at the end of message", dattim);
  };
} else if(strcmp(typs,"<")==0) {   //replace trailing "\n:\n" by  <::
  strcpy(mytyp,"<");
  if(strncmp(&line[lm-3], "\n:\n", 3)==0) {
    //line[lm-3]='<'; line[lm-2]='\0';
    strcpy(&line[lm-3], "<::");
  } else {
    strcat(line,"<");
    //printf("logfile, dimlogprt: NL:NL expected ");
  }
} else {
  sprintf(mytyp,"%s:",typs);
  strncpy(line, msg, MAXLILE);
};
//if(strcmp(typs,"<")!=0) {   //do not log what we sent back
  fprintf(logfile, "%s%s:%s\n", mytyp, dattim, line); fflush(logfile);
//};
}
/*------------------------------------------*/ void prerr(char *msg) {
dimlogprt("ERROR", msg);
//printf("ERROR:%s\n", msg); fflush(stdout);
}
/*---------------------------------------------------- error_handler
A severity code: 0 - info, 1 - warning, 2 - error, 3 - fatal.
*/
void error_handler(int severity, int error_code, char *message) {
char msg1[500];
char *sev[5]={"info", "warning", "error", "fatal", "???"};
if((severity<0) || (severity>3)) {
  severity=4;
};
sprintf(msg1,"*** DIM %s: %d:%s", sev[severity], error_code, message);
dimlogprt("error_handler",msg1);
}
/*--------------------------------------------------------------- exit_handler
*/
void exit_handler(int *exitcode) {
char msg1[100];
sprintf(msg1,"exitcode:%d", *exitcode);
dimlogprt("exit_handler",msg1);
}
/*---------------------------------------------------- client_exit_handler
*/
void client_exit_handler(int *tag) {
char msg1[100];
if((*tag<MAXMCclients) && (MCclients[*tag].cidat[0]!='\0')) {
  sprintf(msg1,"deleting:%c:%d:%s", 
    MCclients[*tag].type, *tag, MCclients[*tag].cidat);
  MCclients[*tag].cidat[0]='\0';
  NMCclients--;
} else {
  sprintf(msg1,"error (max:%d):%d",MAXMCclients, *tag); 
}
dimlogprt("client_exit_handler",msg1);
}

int totlenResult;
/*----------------------------------------------- startResult
Usage:
startResult();
while(1) {
  if(appendResult(anystring) {
     error goto ERR
  };
};
finishResult()
*/
void startResult() {
ResultString[0]='\0'; totlenResult=0;
}
/*----------------------------------------------- finishResult
*/
void finishResult() {
if(strcmp(&ResultString[totlenResult-3],"\n:\n")!=0) {
  if((totlenResult + 3)>=MAXRESULT) {
    strcat(&ResultString[totlenResult-3],"\n:\n");
    totlenResult= totlenResult+0;
  } else {
    strcat(ResultString,"\n:\n");
    totlenResult= totlenResult+3;
  };
};
}
/*----------------------------------------------- appendResult
rc: 0: line (or next part of result) appended
    1: cannot be appended, message finished with "\n:\n"
*/
int appendResult(char *msg) {
int ml, rc=0;
ml=strlen(msg);
if((ml + totlenResult + 3)>=MAXRESULT) {
  prerr("Short ResultString variable");
  finishResult();
  rc=1;
} else {
  strcat(ResultString, msg); totlenResult= totlenResult+ml;
};
return(rc);
}
/*----------------------------------------------- printMCclients
*/
void printMCclients() {
int ix;
char line[80];
startResult();
sprintf(line,"Number of clients:%d\n", NMCclients);
if(appendResult(line)) { return; };
for(ix=0; ix<MAXMCclients; ix++) {
  if(MCclients[ix].cidat[0]=='\0') {
    continue;
  };
  sprintf(line,"%c:%d %s\n", MCclients[ix].type, MCclients[ix].cid, MCclients[ix].cidat);
  if(appendResult(line)) { return; };
};
finishResult();
}
/*----------------------------------------------- updateNMCclients
Input: type: 'm' -MONCOUNTERS   'r' -RESULT
Operation:
1. find curent client's or free item in MCclients table
2. if client not in the table:
     insert it and call
   else:
     replace it in MCclients table if the same CID
3. CALL dis_set_client_exit_handler 
return: -1 error  (too many clients)
       >=0 ix into MCclients table
*/
int updateNMCclients(char type) {
int ix, ixfree=-1;;
int cid,rc;
char cidname[MAXCIDAT];
  char msg[100];
rc=-1;
cid=dis_get_client(cidname);
for(ix=0; ix<MAXMCclients; ix++) {
  if(MCclients[ix].cidat[0]=='\0') {
    if(ixfree==-1) ixfree=ix;
    continue;
  };
  if(MCclients[ix].cid == cid) {
    if(strncmp(MCclients[ix].cidat, cidname,MAXCIDAT)==0) {
      rc=ix;
      goto FOUND;
    };
    // let's overwrite this item
    ixfree=ix; break;
  };
};
sprintf(msg,"ixfreemsg:%d\n",ixfree);
dimlogprt("ixfree",msg);
if(ixfree == -1) {
  sprintf(msg,"Table not large enough:now %d nclients, maximum.\n", NMCclients);
  dimlogprt("MCclients",msg);
} else {                // new client, or 'new replacing' client
  if(MCclients[ixfree].cidat[0]=='\0') {
    sprintf(msg,"new client:%c:%d %s\n", type, cid, cidname);
    NMCclients++;
  } else {
    sprintf(msg,"%c:%d replacement: %s -> %c:%s\n",
      MCclients[ix].type, cid, MCclients[ix].cidat, type, cidname);
  }; firstcaba=1;
  dimlogprt("updateNMCclients",msg);
  MCclients[ixfree].cid= cid;
  strncpy(MCclients[ixfree].cidat, cidname,MAXCIDAT);
  MCclients[ixfree].type= type;
  dis_set_client_exit_handler (cid, ixfree);
  rc=ixfree;
};
FOUND: return(rc);
}

/*------------------------------------------- checkcid() 
return:
<=0 -error
>0 -current cid, which is OK, (was before or just has been set)
*/
int checkcid() {
int loccid;
char loccidat[80];
//char msg[500];
loccid= dis_get_client(loccidat);
  //sprintf(msg,"actcid1:%d, actcidat1:%s\n",actcid, actcidat); dimlogprt("checkcid",msg);
  actcid=loccid; strcpy(actcidat,loccidat); 
  //sprintf(msg,"actcid2:%d, actcidat2:%s\n",actcid, actcidat); dimlogprt("checkcid",msg);
  // don't check it for now...
  return(actcid);

if(actcid==0) {
  actcid=loccid;
  strcpy(actcidat,loccidat);
} else if(loccid != actcid) {
  char emsg[ERRMSGL];
  sprintf(emsg, "checkcid error: requested by:%d <> opened by:%d",
    loccid, actcid);
  prerr(emsg);
  return(-1);
} else {
  actcid=loccid;
  strcpy(actcidat,loccidat);
};
//printf("           actcid:%d, actcidat:%s\n",actcid, actcidat);
return(actcid);
}

/*-----------------------------------*/ void writepipe(char *msg, int size) {
int wrc;
wrc=write(infp, msg, size);   /* NO TRAILING '\0' ! */
if(wrc == -1) {
  char errmsg[300];
  sprintf(errmsg,"wrc:%d %d:%s", wrc, errno, strerror(errno));
  prerr(errmsg);
};
}

/*--------------------------------------------*/ void updateservice() {
int nupdated;
int cids[2];
//char logmsg[200];
cids[0]= actcid;
cids[1]= 0;   // end of the list

nupdated= dis_selective_update_service(RESULTid, cids);
/*sprintf(logmsg, "actcid:%d, actcidat:%s n:%d", actcid,actcidat,nupdated);
dimlogprt("updateservice", logmsg); */
/*
nupdated= dis_update_service(RESULTid);
printf("updateservice RESULTid:%d n:%d\n", RESULTid, nupdated);*/
strcpy(ResultString,"empty");
}

/*--------------------------------------------*/ int updateserviceButme() {
int ix, nupdated,rc;
int cid;
char cidname[MAXCIDAT];
int cids[2];
rc=0;
cids[1]= 0;   // end of the list
cid=dis_get_client(cidname);
for(ix=0; ix<MAXMCclients; ix++) {
  if(MCclients[ix].cidat[0]=='\0') {
    continue;
  };
  if(MCclients[ix].cid == cid) {
    dimlogprt("updateserviceButme:", cidname);
    continue;
  };
  cids[0]= MCclients[ix].cid;
  dimlogprt("updateserviceButme:updating:", cidname);
  nupdated= dis_selective_update_service(RESULTid, cids);
  if(nupdated==1) rc++;
};
strcpy(ResultString,"empty");
return(rc);
}
/*---------------------------*/ int getfilename(char *msg, char *fname) {
int ix, ix2;
for(ix=0; ix<(MAXFNL-1); ix++) {
  ix2=ix;
  if(msg[ix2]=='\"') break;
  if(msg[ix2]=='\0') break;
  fname[ix]=msg[ix2];
  fname[ix+1]='\0';
};
return(0);
}
/*------------------------------------------------------------ putfile()
msg: 
1. name of the file relative to VMECFDIR directory
2.... blocks of length>0
3. empty block (*size==0) ->end of transfer
*/
void putfile(void *tag, void *msgv, int *size)  {  
char logmsg[MAXLILE]; char *msg=(char *)msgv;
msg[*size]='\0';
//printf("putfile: tag:%d size:%d msg:%s<\n", *tag, *size,msg);
//printf("putfile: tag:%d size:%d\n", *tag, *size);
if(checkcid()<0) return;
if(PUTFILE==NULL)  {
  char fname[MAXFNL];
  strcpy(fname, msg);
  PUTFILE= fopen(fname,"w");
  if(PUTFILE!=NULL) {
    sprintf(logmsg, "%s opened\n", fname);
    dimlogprt("putfile", logmsg);
  } else {
    sprintf(logmsg, "putfile: %s opened", fname);
    prerr(logmsg);
    return;
  };
} else {
  if(*size > 0) {
    int lng;
    lng=fwrite(msg, *size, 1, PUTFILE); 
    //printf("pufile written bytes:%d\n",lng);
  } else {
    fclose(PUTFILE); PUTFILE=NULL;
  };
};
}

/*------------------------------------------------------------ getfile()
msg: 
1. name of the file relative to VMECFDIR directory
2. open 
*/
void getfile(void *tag, void *msgv, int *size)  {  
char logmsg[100]; char *msg=(char *)msgv;
msg[*size]='\0';
//printf("getfile: tag:%d size:%d msg:%s<\n", *tag, *size,msg);
sprintf(logmsg,"tag:%d size:%d name:%s<\n", *(int *)tag, *size, msg);
dimlogprt("getfile", logmsg);
if(checkcid()<0) return;
  char fname[MAXFNL];
  strcpy(fname, msg);
  GETFILE= fopen(fname,"r");
  if(GETFILE==NULL) {
    GETFILEC=1; updateservice();
    GETFILEC=0;
    strcpy(ResultString, "file cannot be opened\n:"); updateservice();
  } else { while(1) {
    updateservice();
    if(feof(GETFILE) || ferror(GETFILE)) {
      fclose(GETFILE);
      GETFILE=NULL; GETFILEC=1;
      updateservice();
      GETFILEC=0;
      strcpy(ResultString, ":"); updateservice();
      break;
    };
  }; };
}

/*---------------------------------------- void CALIBBCcaba()*/
//#ifdef CPLUSPLUS
void CALIBBCcaba(void *tagvoid, void **calibbcv, int *size, int *blabla) {
int *tag= (int *)tagvoid; int **calibbc= (int **)calibbcv;
//#else
//void CALIBBCcaba(int *tag, void **msgp, int *size) {
//#endif
char msg[200];
sprintf(msg, "CALIBBCcaba: tag:%d size:%d calibbc:%d:<\n", *tag, *size, **calibbc);
dimlogprt(">", msg);

//strcpy(ResultString,"OK:\n"); 
*calibbc= &(ltushm->ltucfg.calibbc);
*size=sizeof(int);
}

/*---------------------------------------- void EXECcaba()*/
#ifdef CPLUSPLUS
void EXECcaba(void *tagvoid, void **msgpvoid, int *size, int *blabla) {
int *tag= (int *)tagvoid; char **msgp= (char **)msgpvoid;
#else
void EXECcaba(int *tag, char **msgp, int *size) {
#endif
char msg[200];
sprintf(msg, "EXECcaba: tag:%d size:%d msg:%s:<\n", *tag, *size, *msgp);
dimlogprt(">", msg);
//printf("EXECcaba actcid:%d, actcidat:%s\n",actcid, actcidat); 
/*
writepipe(msg, *size-1);   // NO TRAILING '\0' !
readUntilColon(1);
*/
sleep(4);
strcpy(ResultString,"OK:\n"); 
*msgp= ResultString; *size= strlen(ResultString)+1; /* + end of string */
}
/*----------------------------------------*/ 
#ifdef CPLUSPLUS
void RESULTcaba(void *tagvoid, void **msgpvoid, int *size, int *blabla) {
int *tag= (int *)tagvoid; char **msgp= (char **)msgpvoid;
#else
void RESULTcaba(int *tag, char **msgp, int *size) {
#endif
//char logmsg[100];
*tag=4567;
int rc;
if(checkcid()<0) {
  *msgp= foreignmsg;
  *size= strlen(foreignmsg)+1;
  return;
}; 
//printf("RESULTcaba actcid:%d, actcidat:%s\n",actcid, actcidat); 
*msgp= ResultString;
if(GETFILE!=NULL) {
  int bytes;
  bytes= fread(ResultString, 1, MAXLILE, GETFILE);
  *size=bytes;
  //sprintf(logmsg, "getfile size:%d\n",*size);
  /*if(*size>0) {
    if(*size>60) {
      sprintf(logmsg,"%s%60.60s\n",logmsg,ResultString);
    } else {
      sprintf(logmsg,"%s%s\n",logmsg,ResultString);
    }
  } else strcat(logmsg,"\n");
  */
  //dimlogprt("RESULTcaba:",logmsg);
} else {
  if(GETFILEC==1) {
    *size=0; ResultString[0]='\0';
  } else {
      rc=updateNMCclients('r');
      if(rc==-1) {
        strcpy(ResultString, 
          "XXERROR: Too many clients. Stop this and possibly others clients\n");
      };
    if(firstcaba==1) {
      strcpy(ResultString,notinitialised); strcat(ResultString, servercwd); 
      strcat(ResultString,"\n");
      firstcaba=0;
    };
    *size= strlen(ResultString)+1;   // "" -empty string is 1 byte message
  };
  dimlogprt("<",ResultString); //sleep(1);
};
}

/*----------------------------------------------------------- MONCOUNTERScaba
*/
#ifdef CPLUSPLUS
void MONCOUNTERScaba(void *tagvoid, void **msgpvoid, int *size, int *blabla) {
/*int *tag= (int *)tagvoid; */ char **msgp= (char **)msgpvoid;
#else
void MONCOUNTERScaba(int *tag, unsigned int **msgpint, int *size) {
char **msgp= (char **)msgint;
#endif
int rc;
//char msg[100];
/*sprintf(msg, "sizeorig:%d LTUNCOUNTERSall:%d\n", *size, LTUNCOUNTERSall);
dimlogprt("MONCOUNTERScaba",msg); */
rc=updateNMCclients('m');
if(rc==-1) {
  *msgp= ResultString;
  strcpy(ResultString, "XERROR: Too many clients. Stop this and possibly others clients\n");
  *size= strlen(ResultString)+1;   // "" -empty string is 1 byte message
  return;
};
*msgp= (char *)buf1;
*size= 4*LTUNCOUNTERSall;
/*sprintf(msg,"MONCOUNTERScaba size:%d LTUNCOUNTERSall:%d \n", 
  *size, LTUNCOUNTERSall);
dimlogprt("MONCOUNTERScaba2",msg); */
}
/*--------------------------------------------------------- readUntilColon()
if us==1: update service() called, i.e. ResultString is 
          sent after each line to client
if us==0: update service() not called, i.e. ResultString is filled
          until it is full

while(1) {
  if(appendResult(anystring) {
     error goto ERR
  };
};
finishResult()
*/
void readUntilColon(int us) {
char partResult[MAXRESULT];
char logmsg[MAXRESULT+MAXLILE];
int outl;
startResult();
while(1) { // read until ":\n"
  outl=read(outfp, partResult, MAXRESULT);
  if(outl== -1) {   /* pipe closed */
    sprintf(logmsg, "pipe closed\n"); dimlogprt("ReadUntilColon", logmsg);
    close(infp); popen2active=0;
    //ResultString[outl]='\0';
    strcpy(ResultString, "pipe closed on server\n:\n");
    break;
  } else {
    partResult[outl]='\0';
    if(outl==0) { usleep(1000000); continue; };
    if(us==1) {
      if(appendResult(partResult)) {
        break;   // short ResultString variable
      };
    };
    /* sprintf(logmsg,"Ooutl:%d us:%d ResultString:%s<---\n", 
       outl,us,ResultString); dimlogprt("rUC", logmsg); */
  };
  if(strcmp(&partResult[outl-2],":\n")==0) break;
};
if(us==1)updateservice();
}

/*-------------------------------------------------------- cmdPIPE()
msg: open
     getclients
     MSG2ALL
     close    -not implemented (probably not necessary)
*/
void cmdPIPE(void *tag, void *msgv, int *size)  {  
int ix; char *msg=(char *)msgv;
//printf("cmdPIPE: tag:%d size:%d msg:%s<\n", *tag, *size,msg);
if(checkcid()<0) return;
if(strncmp(msg,"open ", 5)==0) {   /*--------------------------------- open */
  dimlogprt(">", msg); 
  //sleep(1);
  if( popen2active !=0) {
    //strcpy(ResultString, "PID xxxxx (popen2 already active)\n");
    strcpy(ResultString, "PID ");
    //return caller's pid...
    for(ix=0; ix<8; ix++) {
      ResultString[4+ix]= actcidat[ix];
      if( (actcidat[ix]=='@') || (actcidat[ix]=='\0') ) {
        ResultString[4+ix]= '\0'; break;
      };
    };
/*
    strcat(ResultString, " (popen2 already active)\n");
    updateservice();
    printf("cmdPIPE:%s<---", ResultString);
    strcpy(ResultString, ":\n");
    updateservice();
    printf("cmdPIPE:%s<---", ResultString);
*/
    sprintf(ResultString,"%s (popen2 already active, clients:%d)\n:\n",
      ResultString, NMCclients);
    goto RTRN;
  } else {
    strcpy(ResultString, "popen2active==0\n:\n");
    prerr(ResultString);
  /*
  pidt= popen2(BASEADDR, &infp, &outfp);
  //fsync(infp) ;  nohelp
  if (pidt <= 0) {
    prerr("popen2 error");
  } else {
    //outl= prtfdfs(infp); outl= prtfdfs(outfp);
    popen2active=1;
    readUntilColon(1);
    goto RTRNupdated;
  };
  */
  };
} else if(strncmp(msg,"close ",6)==0) { /*---------------------------- close */
    strcpy(ResultString, "Error: close not implemented yet\n:\n");
    prerr(ResultString);
} else if(strncmp(msg,"getclients",10)==0) {   /*---------- getclients */
    printMCclients();
} else if(strncmp(msg,"MSG2ALL",7)==0) {   /*---------- MSG2ALL */
    int nupdated; char typs[80];  //can consist of more lines, has to be finished
    startResult(); appendResult(msg); finishResult(); // by \n:\n
    ResultString[0]='m';   //seems must be BEFORE updateserviceButme!
    updateservice();
    ResultString[0]='M';
    nupdated= updateserviceButme();
    sprintf(typs, "MSG2ALL to %d clients", nupdated);
    dimlogprt(typs, msg);
    goto RTRNupdated;
} else {
    sprintf(ResultString, "Error:got:%s, but open or close expected\n:\n", msg);
    prerr(ResultString);
};
RTRN: //printf("=%s=", ResultString); 
    updateservice();
RTRNupdated:
actcid= 0;
return;
}

EXTRN char BoardBaseAddress[40]; /* LTU VME address */
EXTRN char BoardSpaceLength[40];
/*--------------------*/ void cmdCMD(void *tag, void *msgv, int *size)  {  
/* msg: string IS NOT finished by "\n\0" 
! VME not OPEND (opens with SMI command!)

*/
char *msg=(char *)msgv;
printf("cmdCMD: tag:%d size:%d msg:%s<\n", *(int *)tag, *size, msg);
fflush(stdout); 
/*ixlast= strlen(msg)-1;
if(msg[ixlast]!='\n') {
  msg[ixlast]= '\n';
  prerr("NL forced at the end of the following message");
};*/
if(checkcid()<0) return;
if(vmeopen(BoardBaseAddress,BoardSpaceLength) ) {
  printf(" Cannot open vme access for LTU %s\n", BoardBaseAddress);
  return;
};
if(strcmp(msg,"ttcrxreset")==0) {
  TTCrxreset(); usleep(10000); TTCrxregs(&(ltushm->ltucfg));
  dimlogprt("cmdCMD", "rxreset ok\n");
  printf("rxreset ok\n"); fflush(stdout);
} else if(strcmp(msg,"ttcrxregs")==0) {
  TTCrxregs(&(ltushm->ltucfg));
  dimlogprt("cmdCMD", "rxregs ok\n");
} else {
  dimlogprt("cmdCMD", "bad command\n");
};
vmeclose();
}

/*--------------------*/ void cmdDO(void *tag, void *msgv, int *size)  {  
/* msg: string finished by "\n\0" */
int ixlast; char *msg=(char *)msgv;
//printf("cmdDO: tag:%d size:%d msg:%s<\n", *tag, *size,msg);
ixlast= strlen(msg)-1;
if(msg[ixlast]!='\n') {
  msg[ixlast]= '\n';
  prerr("NL forced at the end of the following message");
};
dimlogprt(">", msg);
if(checkcid()<0) return;
if(strcmp(msg,"logon\n")==0) {
  LOGGING=1;
  strcpy(ResultString,"logon\n:\n");
  //ResultString[0]='\0';
  updateservice();
} else if(strcmp(msg,"logoff\n")==0) {
  strcpy(ResultString,"logoff\n:\n");
  //ResultString[0]='\0';
  updateservice();
  LOGGING=0;
} else if(popen2active==0) {
  sprintf(ResultString,"popen2 not active for client:%s:\n",actcidat);
  updateservice();
} else if(strncmp(msg,"getfile(",8)==0) {  // TEXT FILES ONLY ! 
  dimlogprt("cmdDO","getfile: not here, special command\n");
} else if(strncmp(msg,"putfile(",8)==0) {
  dimlogprt("cmdDO","putfile: not here, special command\n");
} else {            // --->pipe
  if((RWMODE=='R') && ( 
     (strncmp(msg, "getCounters(", 12)!=0) && 
     (strncmp(msg, "getsgmode(", 10)!=0) &&
     (strncmp(msg, "vmeopr", 6)!=0) &&
     (strncmp(msg, "ERgetselector(", 14)!=0) &&
     (strncmp(msg, "getERenadis(", 12)!=0) &&
     (strncmp(msg, "SLMgetstart(", 12)!=0) &&
     (strncmp(msg, "prtfnames(", 10)!=0) &&
     (strncmp(msg, "SLMdump(", 8)!=0) &&
     (strncmp(msg, "SLMbreak(", 9)!=0) &&
     (strncmp(msg, "SSMstartrec(", 12)!=0) &&
     (strncmp(msg, "measureBusy", 11)!=0) &&
     (strncmp(msg, "SSMdump(", 8)!=0) &&
     (strncmp(msg, "setOption(", 10)!=0) &&
     (strncmp(msg, "getgltuver(", 11)!=0) &&
     (strncmp(msg, "RateLimit(", 10)!=0) &&

     (strncmp(msg, "vmeopw", 6)!=0) &&
     (strncmp(msg, "setBC_DELAY_ADD(", 16)!=0) &&
     (strncmp(msg, "SLMsetstart(", 12)!=0) &&
     (strncmp(msg, "TTCinit(", 8)!=0) &&
     (strncmp(msg, "ERenadis(", 9)!=0) &&
     (strncmp(msg, "ERdemand(", 9)!=0) && 
     (strcmp(msg, "qs\n")!=0)
    )) {  /* setBC_DELAY_ADD  allowed -we may use it when run active to find
            'bad' value 
TTCinit() -allowed for MUON_TRK (always busy) -to test TTCrxreset protocol
ERenadis/ERdemand -for tests with detectors (to be disabled later)
*/
    strcpy(ResultString,"Error: LTU is not in STANDALONE_STOPPED\n:\n");
    updateservice();
  } else {              // STDALONE_STOPPED
    if(strcmp(msg,"qs\n")==0) strcpy(msg,"q\n");
    writepipe(msg, *size-1);   /* NO TRAILING '\0' ! */
    //printf("written:%d %s<\n",wrc, msg);
    //dimlogprt(">", msg);
    /*for(wrc=0; wrc<=*size; wrc++) {
      printf("%2x",msg[wrc]);
    }; printf("\n"); */
    if(strcmp(msg,"q\n")==0) {
      dimlogprt("cmdDO", "qs -finishing\n");
      ResultString[0]='\0';
      updateservice();
      ds_stop();
    /*} else if(strcmp(msg,"readSSM2SHM()\n")==0) {
      readUntilColon(0);
      ResultString[2]='X';
      updateservice(); */
    } else {
      readUntilColon(1);
    };
  };
};
actcid= 0;
}
int oldnclients=0;
/*-------------------------------------------------------- readltucounters()
clientid: 0: update all subscribing clients
        !=0: update only clientcid client (forced counters read)
*/
void readltucounters(int clientid) {
int nclients; 
w32 ix;
w32 *ResultStringBin= (w32 *)ResultString;
char msg[ERRMSGL];

if(buf1==NULL) {
  prerr("shared memory alloc problem in readltucounters");
  return;
};
strcpy(msg,"readCNTS2SHM()\n");
writepipe(msg, strlen(msg));   //NO TRAILING '\0'!
readUntilColon(0);
                           // dimlogprt("readCNTS2SHMd", ResultString);
for(ix=0; ix<LTUNCOUNTERSall; ix++) {
  ResultStringBin[ix]=buf1[ix];
};
/*  sprintf(msg, "readltucounters: %d %d %d\n", buf1[23],buf1[24], buf1[25]); 
    dimlogprt("readltucounters",msg);*/
if(clientid==0) {
  nclients= dis_update_service(COUNTERSid);
  /*printf("readltucounters: difmics:%d nclients:%d elapsed L0,L1: %x %x\n", 
    difmics, nclients, ltuc[13], ctpc[CSTART_L1+5]);
  printf("readltucounters spec secs, mics:%d %d\n", 
    ctpc[CSTART_SPEC], ctpc[CSTART_SPEC+1]); */
  if(oldnclients != nclients) {   // # of client changed
    int ix;
    sprintf(msg, "clients now: %d\n", nclients); 
    dimlogprt("readltucounters",msg);
    for(ix=0; ix<NMCclients; ix++) {
      printf("%c:%3d: %s\n", MCclients[ix].type, 
        MCclients[ix].cid, MCclients[ix].cidat);
    }; fflush(stdout);
    oldnclients= nclients;
  };
} else {
  int cids[2];
  cids[0]= clientid; cids[1]= 0;   // end of the list
  nclients= dis_selective_update_service(COUNTERSid, cids);
  /*nclients:0 if this clinet has not subsribed to MONCOUNTERS service */ 
  sprintf(msg,"Forced update for client:%d nclients:%d\n", clientid, nclients);
  dimlogprt("readltucounters",msg);
};
}
/*-------------------------------------------------------- cthread
running as thread (started once, with dim server)
*/
void cthread( void *blabla) {
while(1) {   //run forever
  readltucounters(0);
  dtq_sleep(60);
  if(QUIT==1) {
    // freeShared(buf1,...);     -for SSM yes, but not for counters
    buf1=NULL;
    shmdt(ltushm);
    break;
  };
};
}
/*------------*/ void cmdGETCOUNTERS(void *tag, void *msgv, int *size)  {  
/* Forced counters reading. */
//char *msg=(char *)msgv;
if(*size>0) {
  char msgerr[200];
  sprintf(msgerr,"cmdGETCOUNTERS msg size: %d\n", *size);
  dimlogprt(">", msgerr);
} else {
  dimlogprt(">","empty message\n");
};
if(checkcid()<0) return;
/*
if(DBGCMDS) {
 printf("cmdGETCOUNTERS:tag:%d size:%d\n", *tag, *size);
 printf("cmdGETCOUNTERS:cid:%d cidat:%s\n", cid, cidat);
};
*/
readltucounters(actcid);
}
/* -----------------------following routines used from outside (ltu_proxy) */
/*----------------------------------------------------- setRWMODE(char rwmode)
Input: chrmode: 'R': readonly (if state != from STDALONE,ERROR)
                'W': r/w mode -in stdalone or ERROR state
*/
void setRWMODE(char rwmode) {
if( (rwmode=='R') || (rwmode=='W')) {
  RWMODE=rwmode;
} else {
  char msg[100];
  sprintf(msg, "bad mode:%c. 'R' or 'W' expected", rwmode);
  dimlogprt("setRWMODE",msg);
};
};
/*----------------------------------------------------- ds_register(name,base)
Input:
detname: sdd, daq, tof,...
base: 0x812000, ....
Return code:
0  -ok, registered
>0 -error found when registering
*/
int ds_register(char *detname, char *base) {
char logmsg[1500];
int pidt, segid, ix, rc=0;
w32 shmkey;
char *environ;
char MYDETNAME[10];
char BASEADDR[10];
char dattim[20];
char command[MAXCMDL];

strncpy(MYDETNAME, detname, 9); //UPPER(MYDETNAME);
strncpy(BASEADDR, base, 9);
shmkey=hex2int(&BASEADDR[2]);
popen2active=0; actcid=0;   // nobody active now
firstcaba=1;
PUTFILE=NULL;
GETFILE=NULL;
NMCclients=0;
for(ix=0; ix<MAXMCclients; ix++) {   // no MONCOUNTERS clients registered
  MCclients[ix].cidat[0]='\0';
};
environ= getenv("DIM_DNS_NODE"); 
getdatetime(dattim);
if(environ ==NULL) {
  sprintf(logmsg,"DIM_DNS_NODE not defined, DIM services not registered...\n");
  dimlogprt("ds_register", logmsg);
  return(10);
};
sprintf(logmsg, "DIM_DNS_NODE:%s   DETECTOR:%s\n", environ, MYDETNAME);
environ=getcwd(servercwd, 80); environ= getenv("VMECFDIR"); 
if(environ !=NULL) {
  sprintf(logmsg, "%sVMECFDIR:%s\ncurrdir:%s\n", logmsg,environ, servercwd);
};
dimlogprt("ds_register", logmsg);
dis_add_error_handler(error_handler);
dis_add_exit_handler(exit_handler);
dis_add_client_exit_handler(client_exit_handler);

/* popen2 has to be before mallocShared: in ltu.exe, shared memory
is initialised + loaded from ltuttc.cfg only in case shared memory
was just allocated */
pidt= popen2(BASEADDR, &infp, &outfp);
if (pidt <= 0) {
  prerr("popen2 error");
  ds_stop();
} else {
  popen2active=1;
  readUntilColon(0);
};

ltushm= (Tltushm *)mallocShared(shmkey, 0, &segid);  //only attch
buf1= ltushm->ltucnts;
if(ltushm->id==0) {   //just allocated
  prerr("shared memory alloc problem in ds_register()");
  ltushm->id=shmkey;
  //strcpy(logmsg, "shared memory alloc problem in readltucounters\n");
} else {
  strcpy(logmsg, "got shared memory\n");
};
/* see RESULTcaba.
strcpy(ResultString,notinitialised); strcat(ResultString, servercwd); 
strcat(ResultString,"\n");
*/
sprintf(logmsg,"%spopen ok.\n",logmsg);
dimlogprt("ds_register", logmsg);

sprintf(logmsg, "\nServices:\n");
strcpy(command, MYDETNAME); strcat(command, "/RESULT");
RESULTid=dis_add_service(command,"C", ResultString, MAXLILE+1, 
  RESULTcaba, 4567);  
sprintf(logmsg, "%s%s RESULTid:%d\n", logmsg, command, RESULTid);

strcpy(command, MYDETNAME); strcat(command, "/MONCOUNTERS");
COUNTERSid=dis_add_service(command,0, buf1, 4*LTUNCOUNTERSall,
  MONCOUNTERScaba, 4568);  
sprintf(logmsg, "%s%s COUNTERSid:%d\n", logmsg, command,COUNTERSid);

strcpy(command, MYDETNAME); strcat(command, "/CALIBBC");
//CALIBBCid= dis_add_service(command,"I:1", NULL, sizeof(int), CALIBBCcaba, 4569);  
CALIBBCid= dis_add_service(command,"I:1", &(ltushm->ltucfg.calibbc), sizeof(int), NULL, 4569);  
sprintf(logmsg, "%s%s CALIBBCid:%d\n", logmsg, command,CALIBBCid);
printf("%s\n", command);

strcpy(command, MYDETNAME); strcat(command, "/EXEC");
EXECid=dis_add_service(command,"C", ResultString, MAXLILE+1,
  EXECcaba, 4570);  
sprintf(logmsg, "%s%s EXECid:%d\n", logmsg, command,EXECid);
dimlogprt("ds_register", logmsg);

sprintf(logmsg, "Commands:\n");
strcpy(command, MYDETNAME); strcat(command, "/CMD");
dis_add_cmnd(command,"C", cmdCMD, TAGcmdCMD);  
sprintf(logmsg, "%s%s\n", logmsg, command);
strcpy(command, MYDETNAME); strcat(command, "/DO");
dis_add_cmnd(command,"C", cmdDO, TAGcmdDO);  
sprintf(logmsg, "%s%s\n", logmsg, command);
strcpy(command, MYDETNAME); strcat(command, "/PIPE");
dis_add_cmnd(command,"C", cmdPIPE, TAGcmdPIPE);  
sprintf(logmsg, "%s%s\n", logmsg, command);
strcpy(command, MYDETNAME); strcat(command, "/PUTFILE");
dis_add_cmnd(command,0, putfile, TAGputfile);  
sprintf(logmsg, "%s%s\n", logmsg, command);
strcpy(command, MYDETNAME); strcat(command, "/GETFILE");
dis_add_cmnd(command,0, getfile, TAGputfile);  
sprintf(logmsg, "%s%s\n", logmsg, command);
strcpy(command, MYDETNAME); strcat(command, "/GETCOUNTERS");
dis_add_cmnd(command,NULL, cmdGETCOUNTERS, TAGcthread);  printf("%s\n", command);
sprintf(logmsg, "%s%s\n", logmsg, command);
dimlogprt("ds_register", logmsg);
dis_start_serving(MYDETNAME);  
environ= getenv("VMESITE"); 
if((strcmp(environ,"ALICE")==0) ||
   (strcmp(environ,"SERVER")==0)) {
  dimlogprt("ds_register", "Starting the LTUcounters reading thread...\n");
  dim_start_thread(cthread, (void *)TAGcthread);
} else {
  sprintf(logmsg, "LTUcounters reading thread not started:VMESITE:%s\n", environ);
  dimlogprt("ds_register", logmsg);
};
/*usleep(2000000);
nupdated= dis_update_service(RESULTid);
sprintf(logmsg,"RESULTid service, updated clients:%d\n", nupdated);
dimlogprt("ds_register", logmsg);*/
return(rc);
}
/*----------------------------------------------------*/ void ds_stop() {
QUIT=1;   // stop thread reading ltu counters
close(infp); close(outfp); popen2active=0; 
dis_remove_service(RESULTid);
dis_remove_service(COUNTERSid);
dis_remove_service(CALIBBCid);
dis_stop_serving();
if(logfile !=NULL) fclose(logfile);
exit(0);
}

