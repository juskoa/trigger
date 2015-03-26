/* server.c
Start: linux/server servername command, i.e.
                    CTPRCFG    RCFG
Operation:
- print to stdout any DIM message (each message is finished by NL):
  received with command 'servername/command'   i.e. dis_add_cmd
- print to stdout error/info messages: 
  ERROR error message
  INFO error message
- if empty message received ("\n"), quit server
- process stdin lines:
  quit
  class runN class1 clg1 clgtime1 classname1 ...
  cmd cmd_system
  where:
  class -keyword
  runN  -run number
  class* -class number ('1' to 'NCLASS')
  clg*, clgtime* -timesharing info for this group
  classname* - class name
  cmd   -keyword
  cmd_system -command to be executed through system()
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include "lexan.h"
#include "vmewrap.h"
#include "vmeblib.h"
#include "daqlogbook.h"
// Tltucfg1 from the following:
#define ONLY_Tltucfg1
#include "../ltu_proxy/ltu_utils.h"
#undef ONLY_Tltucfg1

#include "ctp.h"
#include "ctplib.h"
#define DBMAIN
#include "Tpartition.h"

#include "infolog.h"
#ifdef CPLUSPLUS
extern "C" {
#include <dis.hxx>
}
#else
#include <dis.h>
#endif

// aliases.c subroutines:
#define MAXALIASES 300
int readAliases();
void getClassAliases(char *cname, char **daqlist);
void printalist(char **daqlist);

// ctpcnames.c subroutines:
void ctpc_clear();
void ctpc_addclass(int classN, char *clname, int runn);
void ctpc_addinp(int inn, int runn);
void ctpc_delrun(int runn);
void ctpc_print(char *dimpublication);

typedef struct Tinstver {
  int runn;
  char parname[20];
  char insname[80];
  char insver[80];
} Tinstver;
Tinstver insver[6];

int ignoreDAQLOGBOOK=0;

#define MAXCIDAT 80
#define MAXINT12LINE 100
int INT1id, INT2id, CSid, CNAMESid, CTPRCFGRCFGid, CTPRCFGid,LTUCFGid,C2Did;
char INT1String[MAXINT12LINE]="int1 source";
char INT2String[MAXINT12LINE]="int2 source";
#define MAXCSString 80000
char CSString[MAXCSString]="collisions schedule";
#define MAXCNAMESString (100*6 + 60 + 5)*80  //100 classes, 60 inps, 5: epoch...
char CNAMESString[MAXCNAMESString]="CNAMES string";
/* nclients= dis_update_service(INT1id); */

//int QUIT=0;
void getdatetime(char *);
//int actdb_open();
//int actdb_close();
int actdb_getPartition(char *name, char *filter, char *actname, char *actver);

/*----------------------------------------------------*/ 
void stopserving() {
dis_remove_service(CTPRCFGRCFGid);
dis_remove_service(CTPRCFGid);
dis_remove_service(LTUCFGid);
dis_remove_service(C2Did);
dis_remove_service(INT1id);
dis_remove_service(INT2id);
dis_remove_service(CSid);
dis_stop_serving();
}
/*----------------------------------------------------*/ 
void myprtLog(char *msg) {
char dt[20];
getdatetime(dt);
printf("INFO %s: %s\n", dt, msg); fflush(stdout);
}

/*--------------------------------------------------------------- error_handler
A severity code: 0 - info, 1 - warning, 2 - error, 3 - fatal.
*/
void error_handler(int severity, int error_code, char *message) {
char msg1[100];
char *sev[5]={"info", "warning", "error", "fatal", "???"};
if((severity<0) || (severity>3)) {
  severity=4;
};
sprintf(msg1,"*** DIM %s: %d", sev[severity], error_code);
myprtLog(msg1); myprtLog(message);
}
/*--------------------------------------------------------------- exit_handler
*/
void exit_handler(int *exitcode) {
char msg1[100];
sprintf(msg1,"exit_handler exitcode:%d", *exitcode);
myprtLog(msg1);
}
/*---------------------------------------------------- client_exit_handler
*/
void client_exit_handler(int *tag) {
char msg1[100];
sprintf(msg1,"client_exit_handler");
myprtLog(msg1);
}
/*----------------------------------------------------------- add_insver
rc: -1 error (full) or index where added
*/
int add_insver(int runn, char *pname, char *inst, char *ver) {
int ix, newfix=-1, allcted=0;
for(ix=0; ix<6; ix++) {
  if(insver[ix].runn==0) {
    newfix= ix;
  } else {
    allcted++;
  };;
  //if(run==insver[ix].runn) fix= ix;
};
if(newfix!=-1) {
  allcted++;  // count in also the one just allocated
  insver[newfix].runn= runn;
  strcpy(insver[newfix].parname, pname);
  strcpy(insver[newfix].insname, inst);
  strcpy(insver[newfix].insver, ver);
};
printf("INFO insver:%d:%s %s %s allcted:%d\n",newfix, pname, inst, ver, allcted);
return(newfix);
}
/*----------------------------------------------------------- find_insver
rc:-1 not found
*/
int find_insver(int runn) {
int ix, rc=-1;
for(ix=0; ix<6; ix++) {
  if(insver[ix].runn==runn) rc= ix;
};
return(rc);
}
/*----------------------------------------------------------- del_insver
*/
void del_insver(int runn) {
int ix,found=0;
for(ix=0; ix<6; ix++) {
  if(insver[ix].runn==runn) {insver[ix].runn=0; found=1;};
};
printf("INFO del_insver:%d found:%d\n",runn, found);
}
/*----------------------------------------------------------- reset_insver
*/
void reset_insver() {
int ix;
for(ix=0; ix<6; ix++) {
  insver[ix].runn= 0;
};
printf("INFO reset_insver:\n");
}
/*----------------------------------------------------------- readCS
-check file length, allocate memory, read
5.5.2011 note:
- it is enough to call this at the start of pydim server + in time of
  CTPRCFG/RCFG csupdate command (i.e. not always when message sent to
  DIM client). I.e. following modifications done:
  - readCS() call in CScaba   -to be removed
  - readCS() called in main() + csupdate time
*/
char *readCS() {
int rl,ix; char *rcstr= CSString;
char *environ; char csname[200];
environ= getenv("VMECFDIR"); strcpy(csname, environ);
strcat(csname, "/CFG/ctp/DB/COLLISIONS.SCHEDULE");
rl= readfile(csname, rcstr, MAXCSString);
if(rl>= MAXCSString) {
  printf("INFO readCS(): too long file\n");
  return(NULL);
};
if(rl==-1) {
  printf("INFO readCS(): cannot open %s file\n", csname);
  return(NULL);
};
for(ix=0; ix<rl; ix++) {
  if(rcstr[ix]=='\n') break;
  csname[ix]=rcstr[ix];
}; csname[ix]='\0';
printf("INFO readCS:%s\n", csname);
return(rcstr);
}
/*--------------------------------------------------- getname_rn()
rc: 0 ok
   !=0 error, "ERROR msg\" printed to stdout
*/
int getname_rn(char *mymsg, char *pname, unsigned int *runn) {
char c; unsigned int rundec=0;
int rc=0, ix=0, runnactive=0;
char runc[16]="";
while(1) {
  int cix;
  c=mymsg[5+ix];
  if((c==' ') || (c=='\n') || (c=='\0')) {
    if(runnactive==0) {
      if(c==' ') {
        ix++; runnactive=1; cix=0;
        continue;
      } else { break; };
    } else {
      if(runc[0]!='\0') {
        rundec=atoi(runc); break;
      };
    };
  };
  if(runnactive==1) {
    runc[cix]=c; runc[cix+1]='\0'; cix++;
  } else {
    pname[ix]= c; pname[ix+1]='\0';
  };
  ix++;
  if(ix>=60) {
    printf("ERROR bad rcfg message\n"); rc=1;
    break;
  };
};
if(runnactive!=1) {
  printf("ERROR bad rcfg message, runN missing\n"); rc=1;
};
*runn= rundec;
return(rc);
}

void updateCNAMES() {
int nclients;
ctpc_print(CNAMESString);
nclients= dis_update_service(CNAMESid);
printf("INFO CNAMES update for %d clients\n", nclients);
}

int check_xcounters() {
int ix, xrc; char xpid[20]="";
char emsg[ERRMSGL];
// check xcountersdaq active:
xrc= popenread((char *)"ps --no-headers -C xcountersdaq -o pid=", xpid, 20);
for(ix=0; ix< (int)strlen(xpid); ix++) {
  if(xpid[ix]=='\n') xpid[ix]=' ';
};
sprintf(emsg,"INFO DOrcfg xpid:%s popen rc:%d\n", xpid, xrc);
printf(emsg);
if(xpid[0]=='\0') {
  infolog_trg(LOG_FATAL, "xcounters problem, stop all global runs, call CTP expert");
  printf("ERROR xcounters problem, stop all global runs, call CTP expert\n");
  xrc=1;
} else {
  xrc=0;
};return(xrc);
}
/*--------------------*/ void DOrcfg(void *tag, void *bmsg, int *size)  {
// bmsg: binary message TDAQInfo
TDAQInfo *dain; int rc; unsigned int rundec; char pname[40];
printf("INFO Dorcfg len:%d %ld\n", *size,sizeof(TDAQInfo));
if(*size != sizeof(TDAQInfo)){
 char emsg[ERRMSGL];
 sprintf(emsg, "DOrcfg: Structure dim size different from command size.");
 infolog_trg(LOG_FATAL, emsg);
 printf("ERROR %s\n", emsg);
 return ;
} 
dain= (TDAQInfo *)bmsg;
//printTDAQInfo(dain);
printf("INFO DOrcfg msg:%s\n", dain->run1msg); 
rc= getname_rn(dain->run1msg, pname, &rundec);
if(check_xcounters()) return;
if(rc==0) {
  rc= daqlogbook_update_clusters(rundec, pname, dain, ignoreDAQLOGBOOK);
  printf("INFO Dorcfg rc=%i \n",rc);
  //printf("%s",dain->run1msg); fflush(stdout);  moved down
  if(rc==0) { // inputs -> DAQ
    int level,maxinp,ix,ind,rcu;
    for(level=0; level<3; level++) {
      if(level==2) {maxinp=12; }
      else         {maxinp=24; }
      for(ix=0; ix<maxinp; ix++) {
        ind= findInput(level, ix+1);
        if(ind==-1) continue;
        if(ignoreDAQLOGBOOK) { rcu=0;
          //printf("INFO L%d.%d %s\n", level, ix+1, validCTPINPUTs[ind].name);
        } else {
          rcu= daqlogbook_insert_triggerInput(rundec,   
            ix+1, validCTPINPUTs[ind].name, level);
          printf("INFO L%d.%d %s\n", level, ix+1, validCTPINPUTs[ind].name);
        };
        if(rcu != 0) {
          char emsg[ERRMSGL];
          sprintf(emsg, "daqlogbook_insert_triggerInput(%d,%d,%s,%d) rc:%d",
            rundec,ix+1, validCTPINPUTs[ind].name, level, rcu);
          infolog_trg(LOG_FATAL, emsg);
          printf("ERROR %s\n", emsg);
          break;
        };
      }; 
    };
    printf("%s",dain->run1msg); fflush(stdout);
  } else {
    char emsg[ERRMSGL];
    sprintf(emsg,"DAQlogbook_update_cluster failed. rc:%d", rc);
    infolog_trgboth(LOG_FATAL, emsg);
  };
};
}
/*--------------------*/ void DOltucfg(void *tag, void *bmsg, int *size)  {
// bmsg: binary message Tltucfg
Tltucfg1 *dain; int rc;
dain= (Tltucfg1 *)bmsg;
printf("INFO Dltucfg len:%d run:%d det:%s\n",*size,dain->run,dain->detector); 
rc= daqlogbook_update_LTUConfig(dain->run, dain->detector,
  dain->LTUFineDelay1, dain->LTUFineDelay2, dain->LTUBCDelayAdd);
// INFO msg in daqlogbook...
}

/*--------------------*/ void DOcom2daq(void *tag, void *msg, int *size)  {
// msg: runN "title" "comment" 
int ixl, runN; char errmsg[200]="";
char *line;
#define MAXDAQCOMMENT 500
char value[MAXDAQCOMMENT];
char title[MAXDAQCOMMENT];
enum Ttokentype t1;
line= (char *)msg;
printf("INFO DOcom2daq len:%d m:%s\n", *size, line); 
if(*size > MAXDAQCOMMENT) {
  printf("ERROR too long title+comment for DAQlogbook\n"); return;
};
ixl=0; t1= nxtoken(line, value, &ixl);   // runN
if(t1==tINTNUM) {
  runN= str2int(value);
  t1= nxtoken(line, value, &ixl);   // title
  if(t1==tSTRING) {
    strcpy(title, value);
    t1= nxtoken(line, value, &ixl);   // message
    if(t1==tSTRING) {
      int rcdl;
      rcdl= daqlogbook_add_comment(0,title,value);
      printf("INFO DAQlogbook comment: %d %s %s rc:%d\n",
        runN, title, value,rcdl);
    } else {
      strcpy(errmsg,"Bad message (\"string\" expected)");
    };
  } else {
    strcpy(errmsg,"Bad title (\"string\" expected)");
  };
} else {
  strcpy(errmsg,"Bad run number (int expected)");
};
if(errmsg[0]!='\0') {
  printf("ERROR DOcom2daq:%s:%s\n",errmsg, line);
};
}
/*--------------------*/ void DOcmd(void *tag, void *msg, int *size)  {
/* msg: string finished by "\n\0" */
//printf("DOcmd: tag:%d size:%d msg:%s<-endofmsg\n", *tag, *size,msg);
char mymsg[400];
int stdoutyes=1;
strncpy(mymsg, (char *)msg, 400); 
mymsg[398]='\n'; mymsg[399]='\0';   // force \n (if not given)
if((strncmp(mymsg,"pcfg ",5)==0) || (strncmp(mymsg,"Ncfg ",5)==0)) {
/* pcfg RUNNUMBER partname    -try ACT download (ECS INIT)
   Ncfg RUNNUMBER partname    -NO ACT, change: Ncfg -> pcfg
   \n               -stop this server
   rcfgdel useDAQLOGBOOK
   rcfgdel ignoreDAQLOGBOOK
   rcfgdel ALL 0            -ctpproxy restart
   rcfgdel PARTNAME RUNN    -process in .py, (ECS STOP)
   csupdate
   aliasesupdate
   intupdate
   clockshift
   rcfg  -OBSOLETE!
   resetclock   -just write out
*/
  char c; unsigned int rundec=0;
  int rc, infoerr, ix=0, runnactive=0;
  char instname[100]="";
  char version[100]="";
  char pname[60]="";
  char runc[16]="";
  char emsg[500];
  //rc= getname_rn(mymsg, pname, rundec);   moved to DOrcfg()
  while(1) {
    int cix;
    c=(mymsg)[5+ix];
    if((c==' ') || (c=='\n') || (c=='\0')) {
      if(runnactive==0) {
        if(c==' ') {
          ix++; runnactive=1; cix=0;
          continue;
        } else { break; };
      } else {
        if(runc[0]!='\0') {
          rundec=atoi(runc); break;
        };
      };
    };
    if(runnactive==1) {
      runc[cix]=c; runc[cix+1]='\0'; cix++;
    } else {
      pname[ix]= c; pname[ix+1]='\0';
    };
    ix++;
    if(ix>=60) break;
  };
  infolog_SetStream(pname, rundec);
  if(mymsg[0]=='p') {
    char filter[20000]="";
    //actdb_open(); 
    rc=actdb_getPartition(pname,filter, instname, version);
    //actdb_close();
    if(rc==0) {
      sprintf(emsg,"%s (run:%d inst:%s ver:%s) downloaded from ACT.", 
        pname, rundec, instname, version); 
      infoerr=LOG_INFO;
    } else if(rc==1) {
      sprintf(emsg,"%s (run:%d) not found in ACT, might be OK if shift leader disabled it in ACT (i.e. is in 'Local File' mode)", pname, rundec); 
      infoerr=LOG_ERROR;
    } else {
      sprintf(emsg,"actdb_getPartition(%s) run:%d rc:%d (-2: partition not available in ACT)", pname,rundec,rc); 
      infoerr=LOG_ERROR;
    };
  } else {   // Ncfg runnumber partname
    sprintf(emsg,"%s (run:%d) not searched in ACT (ECS:ACT_CONFIG=NO)", 
      pname, rundec); 
    infoerr=LOG_INFO;
    mymsg[0]= 'p';
  };
  //prtLog(emsg);
  //myprtLog(emsg);
  infolog_trgboth(infoerr, emsg);
  rc= add_insver(rundec, pname, instname, version);
  if(rc==-1) {
    sprintf(emsg,"run:%d, instance/ver will not be stored in ACT", rundec);
    infolog_trg(LOG_FATAL, emsg);
    printf("ERROR %s", emsg);
  };
  infolog_SetStream("",0);
/*---- moved to .rcfg time
  if(ignoreDAQLOGBOOK==1) {
    rcdaq=0;
  } else {
    rcdaq= daqlogbook_open(); //rcdaq=0;
    if(rcdaq!=0) {
      printf("ERROR update_cs: DAQlogbook_open failed rc:%d",rcdaq); rc=4;
      //prtError("DAQlogbook_open failed");
    }else{
      char itemname[200];
      rc= daqlogbook_update_cs(rundec, CSString);
      do_partitionCtpConfigItem(pname, itemname);
      rc_insert= daqlogbook_update_ACTConfig(rundec, itemname,instname,version);
      rcdaq= daqlogbook_close();
    };
    if((rc==0) && (rc_insert==0)) {
      infoerr=LOG_INFO;
    } else {
      infoerr=LOG_FATAL;
    };
    sprintf(emsg,"daglogbook_update_cs rc:%d _update_ACTConfig rc:%d", rc, rc_insert);
    infolog_trg(infoerr, emsg);
  };
-------- moved to .rcfg */
//                        1...,...10....,....20..3
} else if((strncmp(mymsg,"rcfgdel ignoreDAQLOGBOOK",23)==0)) {
  int rcdaq;
  ignoreDAQLOGBOOK=1;
  printf("INFO closing DAQlogbook (ignoreDAQLOGBOOK from ctpproxy received)\n");
  rcdaq= daqlogbook_close();
  if(rcdaq==-1) {
    printf("ERROR DAQlogbook_close failed\n");
  };
} else if((strncmp(mymsg,"rcfgdel useDAQLOGBOOK",20)==0)) {
  int rcdaq;
  rcdaq= daqlogbook_open(); //rcdaq=0;
  if(rcdaq!=0) {
    printf("ERROR DAQlogbook_open failed rc:%d",rcdaq);
    ignoreDAQLOGBOOK=1;
  } else {
    ignoreDAQLOGBOOK=0;
  };
} else if((strncmp(mymsg,"rcfgdel ALL 0",13)==0)) {
  int irc;
  reset_insver();
  readTables();
  ctpc_clear(); updateCNAMES();
  irc= check_xcounters();
} else if((strncmp(mymsg,"rcfgdel ",8)==0)) {   // rcfgdel partname runn
  enum Ttokentype t1; int ixl, runn; char pname[16]; char intval[16];;
  char emsg[200];
  emsg[0]='\0';
  ixl=8; t1= nxtoken(mymsg, pname, &ixl);   // runNumber
  if(t1==tSYMNAME) {
    t1= nxtoken(mymsg, intval, &ixl);   // runNumber
    if(t1==tINTNUM) {
      runn= str2int(intval);
      // from 26.2. maybe not needed here, but seems ok when INIT brings ctpproxy to LOAD_FAILURE
      del_insver(runn);  
    } else {
      sprintf(emsg,"pydimserver: bad run number in rcfgdel %s cmd", pname);
    };
  } else {
    sprintf(emsg,"pydimserver: bad part. name in %s cmd", mymsg);
  };
  if(emsg[0]=='\0') {
    ctpc_delrun(runn); updateCNAMES();
  } else {
    infolog_trg(LOG_ERROR, emsg); printf("ERROR %s\n",emsg);
  };
} else if((strncmp(mymsg,"csupdate",8)==0)) {
  int csclients; char *cs;
  cs= readCS();
  csclients= dis_update_service(CSid);
  printf("INFO CS update for %d clients\n", csclients);
  stdoutyes=0;
} else if((strncmp(mymsg,"aliasesupdate",13)==0)) {
  int rc;
  rc= readAliases(); 
  if(rc==-1) {
    char emsg[200];
    strcpy(emsg,"aliasesupdate: info from aliases.txt not updated correctly");
    infolog_trg(LOG_INFO, emsg); printf("ERROR %s\n",emsg);
  };
  stdoutyes=0;
} else if((strncmp(mymsg,"intupdate",9)==0)) {
  int rc1, rc2;
  rc1= getINT12fromcfg(INT1String, INT2String, MAXINT12LINE);
  printf("INFO rc:%d INT1:%s INT2:%s\n", rc1, INT1String, INT2String);
  rc1= dis_update_service(INT1id);
  rc2= dis_update_service(INT2id);
  printf("INFO INT1/INT2 update for %d/%d clients\n", rc1, rc2);
  stdoutyes=0;
} else if((strncmp(mymsg,"clockshift ",11)==0)) {
  char halfns[20]; int ix,rc;
  unsigned int hns,cordeval,last_applied;
  /* char c; halfns[0]='\0';
  for(ix=0; ix<40; ix++) {
    c= mymsg[11+ix]; 
    if((c=='\0') || (c=='\n')) {
      halfns[ix]= '\0'; break;
    };
    halfns[ix]= c;
  }; 
  halfns[20]= '\0';*/
  ix= sscanf(&mymsg[11], "%d %d %d\n", &hns, &cordeval, &last_applied);
  if( (ix<2) || (hns>63) || (cordeval>1023) ) {
    printf("ERROR set clockshift %d %d incorrect, not updated.\n",hns, cordeval);
  } else {
    //int csclients;
    sprintf(halfns, "%d %d %d", hns, cordeval, last_applied);
    rc= writedbfile("clockshift", halfns);   // 2ints: halfns cordereg
    //csclients= dis_update_service(CSid);  is not here
    printf("INFO set clockshift %s. rc(=chars):%d \n", halfns, rc);
  };
  stdoutyes=0;
} else if((strncmp(mymsg,"rcfg ",5)==0)) {   // moved to DOrcfg()
  printf("ERROR rcfg cmd ignored (processed by CTPRCFG cmd\n");
  stdoutyes=0;
} else if((strncmp(mymsg,"resetclock",9)==0)) {
};
if(strcmp(mymsg,"\n")==0) {
  //stopserving();
  myprtLog("Quitting server...");
  printf("stop\n"); fflush(stdout);
  //sleep(1);
  //exit(0);
};
if(stdoutyes==1) {   /*
  pcfg 
    -prepare .pcfg (parted.py)
    - scp .pcfg
  rcfg partName NNN clu1 clu2 ... clu6 cl1 ... clNCLASS NewLine
    -prepare .rcfg (parted.py)
    -
  smaqmv
  */
  printf("%s",mymsg); fflush(stdout);
};
}
/*----------------------------------------------------------- CScaba
*/
void CScaba(void *tag, void **msgpv, int *size, int *blabla) {
char **msgp= (char **)msgpv;
int cid; char cidname[MAXCIDAT];
char *cs;
//char msg[100];
//sprintf(msg,"INFO CScaba size:%d msg:%20.20s\n",*size, *msgp ); prtLog(msg); 
//updateNMCclients(&......); we do not keep track of CS clients
cid=dis_get_client(cidname);
cs= CSString; // cs= readCS();
if(cs==NULL) {
  *msgp= CSString; *size=0;
} else {
  *msgp= cs; *size= strlen(cs);
};
//printf("INFO CScaba size:%d cs:%30.30s \n", *size, cs); cannot (NLs!)
printf("INFO CScaba cid:%d client:%s size:%d\n", cid, cidname, *size);
// free(cs)
}
/*----------------------------------------------------------- CNAMEScaba
*/
void CNAMEScaba(void *tag, void **msgpv, int *size, int *blabla) {
char **msgp= (char **)msgpv;
int cid; char cidname[MAXCIDAT];
char *cs;
//char msg[100];
//sprintf(msg,"INFO CNAMEScaba size:%d msg:%20.20s\n",*size, *msgp ); prtLog(msg); 
//updateNMCclients(&......); we do not keep track of CS clients
cid=dis_get_client(cidname);
cs= CNAMESString; // cs= readCS();
if(cs==NULL) {
  *msgp= CNAMESString; *size=0;
} else {
  *msgp= cs; *size= strlen(cs);
};
//printf("INFO CNAMEScaba size:%d cs:%30.30s \n", *size, cs); cannot (NLs!)
printf("INFO CNAMEScaba cid:%d client:%s size:%d\n", cid, cidname, *size);
// free(cs)
}
/*--------------------------------------------------- updateConfig
update following info in DAQlogbook::
- .rcfg + alignment
- col. schedule
- partition instance name/version (was sent in time of .pcfg)
*/
void updateConfig(int runn, char *pname, char *instname, char *instver) {
int rc, rl;
char *mem; char *environ;
char cfgname[200], aliname[200], itemname[200];
char emsg[1000];

#define MAXALIGNMENTLEN 4000
#define MAXRCFGLEN 30000
char alignment[MAXALIGNMENTLEN];
environ= getenv("VMESITE");
if(strcmp(environ,"ALICE")==0) {
  strcpy(aliname, "/home/dl6/snapshot/alidcsvme001/home/alice/trigger/v/vme/WORK/");
} else if(strcmp(environ,"SERVER")==0) {
  strcpy(aliname, "/home/dl6/snapshot/altri1/home/alice/trigger/v/vme/WORK/");
} else {
  printf("ERROR bad VMESITE env. var:%s",environ); return;
};
strcat(aliname, "alignment2daq");
rl= readdbfile(aliname, alignment, MAXALIGNMENTLEN); alignment[rl]='\0';
if(alignment=='\0') {
  infolog_trg(LOG_FATAL, "Alignment info in DAQlogbook is empty");
  printf("ERROR Alignment info in DAQlogbook is empty");
};
printf("INFO alignment file len:%d\n",rl);
environ= getenv("VMEWORKDIR"); strcpy(cfgname, environ);
sprintf(cfgname,"%s/WORK/RCFG/r%d.rcfg", environ, runn);
mem= (char *)malloc(MAXRCFGLEN+1); mem[0]='\0';
rl= readdbfile(cfgname, mem, MAXRCFGLEN); mem[rl]='\0';
printf("INFO %s file len:%d\n",cfgname, rl);
if(rl < 10) {
  sprintf(emsg, "updateConfig: File: %s read error\n",cfgname); 
  infolog_trg(LOG_FATAL, emsg);
  printf("ERROR %s", emsg);
} else {  
  rc= daqlogbook_update_triggerConfig(runn, mem, alignment);
  if(rc!=0) {
    sprintf(emsg, "DAQlogbook_update_triggerConfig: rc:%d\n",rc); 
    infolog_trg(LOG_FATAL, emsg);
    printf("ERROR %s", emsg);
  };
};
rc= daqlogbook_update_cs(runn, CSString);
do_partitionCtpConfigItem(pname, itemname);
rc= daqlogbook_update_ACTConfig(runn, itemname,instname,instver);
}
/* line: inpupd runn ix1 ix2 ...
 * ix1: 1..60  (24+24+12)
*/
void update_ctpins(char *line) {
unsigned int runn; 
int ixc, ixl;
char value[16];
enum Ttokentype t1;
char emsg[100];
emsg[0]='\0';
ixl= 7; t1= nxtoken(line, value, &ixl);   // runn
if(t1==tINTNUM) {
  runn= str2int(value);
} else {
  sprintf(emsg,"pydim update_ctpins: bad line:%60s",line);
  infolog_trg(LOG_ERROR, emsg); printf("%s\n",emsg);
  return;
};
for(ixc=0; ixc<NCTPINPUTS; ixc++) {
  t1= nxtoken(line, value, &ixl);   // 1..60
  if(t1==tEOCMD) {break;};
  if(t1==tINTNUM) {
    int ixin;
    ixin= str2int(value);
    ctpc_addinp(ixin, runn);
  } else {
    sprintf(emsg,"pydim update_ctpins: bad line:%60s",line);
    infolog_trg(LOG_ERROR, emsg); printf("%s\n",emsg);
    ctpc_delrun(runn);
    break;
  };
};
updateCNAMES();
}

/* Input: class runNumber clid1 classname1 clid2 classname2 ... \n
from 28.7.2010:
   Input: class runNumber 
          clid1 cg1 cgtime1 dsc1 classname1 
          clid2 cg2 cgtime2 dsc2 classname2 ... \n
Operation:
register clid:classname in DAQDB
rc: 0: ok
   !=0: ERROR message printed to stdout
1: runNumber not found
2: classname expected
3: classid 1..NCLASS expected
4: DAQDB update call error
5: bad cg (0..20) allowed (anyhow, ctp_proxy allows only 0..9)
6: bad dsc factor. Expected: dec. number (32 bits, i.e. unsigned int)
7: bad dsc factor: >0x1fffff but bit31 not set
8: premature EOCMD (unfinished line)
*/
int updateDAQDB(char *line) {
int ixl, ixiv, rcex=8;
unsigned int runN; 
int ixc;
char value[256];
enum Ttokentype t1,t2;
printf("INFO updateDAQDB...\n");
ixl=6; t1= nxtoken(line, value, &ixl);   // runNumber
if(t1==tINTNUM) {
  runN= str2int(value);
  ixiv= find_insver(runN);
  if(ixiv == -1) {
    printf("ERROR bad line (runN not found (pcfg req. missing?) ):%s",line);
    rcex=1; return(rcex);
  };
} else {
  printf("ERROR bad line (runN expected after class ):%s",line);
  rcex=1; return(rcex);
};
infolog_SetStream(insver[ixiv].parname, runN);
updateConfig(runN, insver[ixiv].parname, insver[ixiv].insname,
  insver[ixiv].insver);
del_insver(runN);
for(ixc=0; ixc<NCLASS; ixc++) {
  unsigned int classN, cg, cgtim; int rcdaq,daqlistpn;
  float cgtime;
  char dsctxt[24];   // (0.xxx% - 100%) or xxxus or xxx.xxxms
  char *dsctxtp;
  char *daqlist[MAXALIASES]; char **daqlistp;
  char msg[300];
  t1= nxtoken(line, value, &ixl);   // class number
  //printf("INFO ixc:%d token:%d tokenvalue:%s\n",ixc,t1,value);
  if(t1==tEOCMD) {rcex=0; break;};
  if(t1==tINTNUM) {
    classN= str2int(value);
    if( (classN>NCLASS) || (classN<1) ) {
      rcex=3; break;
    };
  } else {
    rcex=3; break;
  };
  t1= nxtoken(line, value, &ixl);   // class group
  if(t1==tEOCMD) break;   
  if(t1==tINTNUM) {
    cg= str2int(value);
    if( cg>NCLASS ) {
      rcex=5; break;
    };
      } else {
    rcex=5; break;
  };
  t1= nxtoken(line, value, &ixl);   // class time group
  if(t1==tEOCMD) break;   
  if(t1==tINTNUM) {
    cgtim= str2int(value);
  } else {
    rcex=3; break;
  };
  t1= nxtoken(line, value, &ixl);   // dsc
  if(t1==tEOCMD) break;   
  if(t1==tINTNUM) {
    unsigned int dsc;
    dsc= str2int(value);
    dsctxtp= dsctxt;
    if(dsc>0x1fffff) {   // class busy
      int dscus;
      if((dsc & 0x80000000)!=0x80000000) {
        printf("ERROR in downscaling factor:0x%x\n", dsc); 
        rcex=7; break;
      };
      dscus= 10*(dsc & 0x1ffffff);
      sprintf(dsctxt, "%dus", dscus);
    } else if(dsc==0) {  // not downscaled
      dsctxt[0]='\0'; dsctxtp= NULL;
    } else {             // random downscaling
      float dscrat;
      dscrat= 100- dsc*100./0x1fffff;
      sprintf(dsctxt, "%2.3f%%", dscrat);
    }
  } else {
    rcex=6; break;
  };
  t2= nxtoken1(line, value, &ixl);   // classname
  if(t2!=tSYMNAME) {rcex=2; break;};
  cgtime= cgtim;
  getClassAliases(value, daqlist); 
  daqlistpn=0;
  if(daqlist[0]==NULL) {
    daqlistp=NULL; 
  } else {
    char *dp;
    daqlistp=daqlist; dp= daqlist[0];
    while(dp!=NULL) {
      daqlistpn++; dp=daqlist[daqlistpn];
    };
  };
  rcdaq= daqlogbook_update_triggerClassName(runN, 
    //classN-1, value, cg, cgtime, (const char **)daqlistp);
    classN-1, value, cg, cgtime, dsctxtp, (const char **)daqlistp);
  sprintf(msg,
    "DAQlogbook_update_triggerClassName(%d,%d,%s,%d,%5.1f, %s, %d) rc:%d",
    runN, classN-1, value, cg, cgtime, dsctxt, daqlistpn, rcdaq);
  if(rcdaq!=0) {
    infolog_trg(LOG_ERROR, msg);
    printf("ERROR %s\n", msg);
    rcex=4; break;
  } else {
    printf("INFO %s\n", msg);
    // without the test below, server crashes (or \n received indicating STOP)
    // ??? Possible reason: cannot print 2 consequtive INFO lines?
    if(daqlistpn>0) printalist(daqlistp);
  };
  ctpc_addclass(classN, value, runN);
  fflush(stdout);
};
if(rcex!=0) {
  ctpc_delrun(runN);
};
updateCNAMES();
infolog_SetStream("",0);
return(rcex);
}

/*--------------------------------------------------- main */
int main(int argc, char **argv) {
int rc,grc=0;
char *cs; char command[40];
char servername[40];
char cmd[80];

setlinebuf(stdout);
if(argc!=3) {
  printf("ERROR: Usage: linux/server servername command\n"); return(8);
};
infolog_SetFacility("CTP");
infolog_SetStream("",0);
strcpy(servername, argv[1]);
strcpy(command, argv[2]);
sprintf(cmd, "%s/%s", servername, command);   // CTPRCFG/RCFG
// following line has to be 1st line printed!
printf("DIM server:%s cmd:%s\n", servername, command); // should be 1st line

reset_insver();
cs= readCS();
rc= readAliases(); 
cshmInit();
readTables();   // + when ctpprxy restaretd, i.e. in time of rcfgdel 0 ALL
if(rc==-1) {
  char emsg[200];
  strcpy(emsg,"aliases info from aliases.txt not updated correctly");
  infolog_trg(LOG_ERROR, emsg); printf("ERROR %s\n",emsg);
};
ctpc_clear(); ctpc_print(CNAMESString);
//updateCNAMES();
rc= getINT12fromcfg(INT1String, INT2String, MAXINT12LINE);
printf("INFO rc:%d INT1:%s INT2:%s\n", rc, INT1String, INT2String);

dis_add_error_handler(error_handler);
dis_add_exit_handler(exit_handler);
dis_add_client_exit_handler (client_exit_handler);
// commands:
CTPRCFGRCFGid= dis_add_cmnd(cmd,"C", DOcmd, 88);
printf("INFO DIM cmd:%s id:%d\n", cmd, CTPRCFGRCFGid);
sprintf(cmd, "%s", servername);   // CTPRCFG binary (after LS1) RCFG request
CTPRCFGid= dis_add_cmnd(cmd,NULL, DOrcfg, 89);
printf("INFO DIM cmd:%s id:%d\n", cmd, CTPRCFGid);

sprintf(cmd, "%s/LTUCFG", servername);   // LTUCFG binary (after LS1)
LTUCFGid= dis_add_cmnd(cmd,NULL, DOltucfg, 90);
printf("INFO DIM cmd:%s id:%d\n", cmd, LTUCFGid);

sprintf(cmd, "%s/COM2DAQ", servername);   // CTPRCFG/COM2DAQ
C2Did= dis_add_cmnd(cmd,NULL, DOcom2daq, 91);
printf("INFO DIM cmd:%s id:%d\n", cmd, C2Did);

// services:
sprintf(cmd, "%s/INT1", servername);   // CTPRCFG/INT1
INT1id=dis_add_service(cmd,"C:99", INT1String, MAXINT12LINE, NULL, 4567);
printf("INFO DIM service:%s id:%d\n", cmd, INT1id);
sprintf(cmd, "%s/INT2", servername);   // CTPRCFG/INT2
INT2id=dis_add_service(cmd,"C:99", INT2String, MAXINT12LINE, NULL, 4568);
printf("INFO DIM service:%s id:%d\n", cmd, INT2id);
sprintf(cmd, "%s/CS", servername);   // CTPRCFG/CS
CSid=dis_add_service(cmd,"C", CSString, MAXCSString, CScaba, 4569);
printf("INFO DIM service:%s id:%d\n", cmd, CSid);
sprintf(cmd, "%s/CNAMES", servername);   // CTPRCFG/CNAMES
CNAMESid=dis_add_service(cmd,"C", CNAMESString, MAXCNAMESString, CNAMEScaba, 4570);
printf("INFO DIM service:%s id:%d\n", cmd, CNAMESid);

dis_start_serving(servername);  
while(1) {
  char *frc;
#define MAXLINECS 8000
  char line[MAXLINECS];   //80 chars per class for NCLASS classes
  //sleep(10);
  frc= fgets(line, MAXLINECS, stdin);
  if(frc==NULL) break;
  if(strncmp(line,"quit",4)==0) {
    break;
  } else if(strncmp(line,"class ",6)==0) {
    int rcdaq;
    printf("INFO igDAQLOGBOOK:%d line:%s",ignoreDAQLOGBOOK, line);
    /* from 28.5.2013: DAQlogbook opened/closed when ctp_proxy restarted
    rcdaq= daqlogbook_open();
    if(rcdaq==-1) {
      printf("ERROR DAQlogbook_open failed\n");
    } else {
      if(ignoreDAQLOGBOOK==0) {
        rcdaq= updateDAQDB(line);
        if(rcdaq!=0) {
          printf("ERROR updateDAQLOGBOOK failed. rc=%d\n", rcdaq);
        };
        rcdaq= daqlogbook_close();
        if(rcdaq==-1) {
          printf("ERROR DAQlogbook_close failed\n");
        };
      };
    };*/
    if(ignoreDAQLOGBOOK==0) {
      rcdaq= updateDAQDB(line);
      if(rcdaq!=0) {
        printf("ERROR updateDAQLOGBOOK failed. rc=%d\n", rcdaq);
      };
    };
  } else if(strncmp(line,"inpupd ",7)==0) {
    update_ctpins(line);
  } else if(strncmp(line,"cmd ",4)==0) {
    int unsigned ix,rcsystem;
    for(ix=0; ix<strlen(line); ix++) {
      if(line[ix]=='\n'){ line[ix]='\0'; break; };
    };
    rcsystem= system(&line[4]);
    printf("INFO rc:%d cmd:%s\n", rcsystem, &line[4]);   
    continue;
  } else {
    printf("ERROR %s", line); fflush(stdout);
  };
};
rc= daqlogbook_close();
if(rc==-1) {
  printf("ERROR DAQlogbook_close failed\n");
};
stopserving();
cshmDetach(); printf("INFO shm detached.\n");
return(grc);
}
