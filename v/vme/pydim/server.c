/* server.c
Start: linux/server servername command
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
  class* -class number ('1' to '50')
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
#include "ctp.h"
#include "ctplib.h"
/*von #define DBMAIN
#include "ctp.h"
#include "Tpartition.h" */

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
int INT1id, INT2id, CSid;
char INT1String[MAXINT12LINE]="int1 source";
char INT2String[MAXINT12LINE]="int2 source";
#define MAXCSString 80000
char CSString[MAXCSString]="collisions schedule";
/* nclients= dis_update_service(INT1id); */


//int QUIT=0;
void getdatetime(char *);
//int actdb_open();
//int actdb_close();
int actdb_getPartition(char *name, char *filter, char *actname, char *actver);

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
int ix, newfix=-1;
for(ix=0; ix<6; ix++) {
  if(insver[ix].runn==0) newfix= ix;
  //if(run==insver[ix].runn) fix= ix;
};
if(newfix!=-1) {
  insver[newfix].runn= runn;
  strcpy(insver[newfix].parname, pname);
  strcpy(insver[newfix].insname, inst);
  strcpy(insver[newfix].insver, ver);
};
printf("INFO insver:%d:%s %s %s\n",newfix, pname, inst, ver);
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


/*--------------------*/ void DOcmd(void *tag, void *msg, int *size)  {
/* msg: string finished by "\n\0" */
//printf("DOcmd: tag:%d size:%d msg:%s<-endofmsg\n", *tag, *size,msg);
char mymsg[400];
int stdoutyes=1;
strncpy(mymsg, (char *)msg, 400); 
mymsg[398]='\n'; mymsg[399]='\0';   // force \n (if not given)
if((strncmp(mymsg,"pcfg ",5)==0) || (strncmp(mymsg,"Ncfg ",5)==0)) {
/* pcfg RUNNUMBER partname    -try ACT download
   Ncfg RUNNUMBER partname    -NO ACT, change: Ncfg -> pcfg
   \n               -stop this server
*/
  char c; unsigned int rundec=0;
  int rc, infoerr, ix=0, runnactive=0;
  char instname[100]="";
  char version[100]="";
  char pname[60]="";
  char runc[16]="";
  char emsg[500];
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
  myprtLog(emsg);
  infolog_trg(infoerr, emsg);
  rc= add_insver(rundec, pname, instname, version);
  if(rc==-1) {
    sprintf(emsg,"run:%d, instance/ver not stored in ACT", rundec);
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
  ignoreDAQLOGBOOK=1;
  printf("INFO DAQlogbook will not be used (ignoreDAQLOGBOOK)\n");
} else if((strncmp(mymsg,"rcfgdel useDAQLOGBOOK",20)==0)) {
  ignoreDAQLOGBOOK=0;
} else if((strncmp(mymsg,"rcfgdel ALL 0",13)==0)) {
  reset_insver();
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
};
if(stdoutyes==1) {   /*
  pcfg 
    -prepare .pcfg (parted.py)
    - scp .pcfg
  rcfg partName NNN clu1 clu2 ... clu6 cl1 ... cl50 NewLine
    -prepare .rcfg (parted.py)
    -
  smaqmv
  */
  printf("%s",mymsg); fflush(stdout);
};
if(strcmp(mymsg,"\n")==0) {
  dis_stop_serving();
  myprtLog("Quitting server...");
  exit(0);
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
/*--------------------------------------------------- updateConfig
Code moved from ctp_proxy (end of June 2001).
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
  strcpy(aliname, "/data/dl/snapshot/alidcsvme001/home/alice/trigger/v/vme/WORK/");
} else if(strcmp(environ,"SERVER")==0) {
  strcpy(aliname, "/data/dl/snapshot/altri1/home/alice/trigger/v/vme/WORK/");
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

/* Input: class runNumber clid1 classname1 clid2 classname2 ... \n
from 28.7.2010:
   Input: class runNumber clid1 cg1 cgtime1 classname1 clid2 cg2 cgtime2 classname2 ... \n
Operation:
register clid:classname in DAQDB
rc: 0: ok
   !=0: ERROR message printed to stdout
1: runNumber not found
2: classname expected
3: classid 1..50 expected
4: DAQDB update call error
5: bad cg (0..20) allowed (anyhow, ctp_proxy allows only 0..9)
*/
int updateDAQDB(char *line) {
int ixl, ixiv, rc=0;
unsigned int runN; 
int ixc;
char value[256];
enum Ttokentype t1,t2;
printf("INFO updateDAQDB...\n");
ixl=6; t1= nxtoken(line, value, &ixl);   // "class runNumber n1 name n2 name2
if(t1==tINTNUM) {
  runN= str2int(value);
  ixiv= find_insver(runN);
  if(ixiv == -1) {
    printf("ERROR bad line (runn not found (pcfg req. missing?) ):%s",line);
    rc=1; return(rc);
  };
} else {
  printf("ERROR bad line (runn expected after class ):%s",line);
  rc=1; return(rc);
};
infolog_SetStream(insver[ixiv].parname, runN);
updateConfig(runN, insver[ixiv].parname, insver[ixiv].insname, insver[ixiv].insver);
del_insver(runN);
for(ixc=0; ixc<50; ixc++) {
  unsigned int classN, cg, cgtim; int rcdaq,daqlistpn;
  float cgtime;
  char *daqlist[MAXALIASES]; char **daqlistp;
  char msg[300];
  t1= nxtoken(line, value, &ixl);   // class number
  if(t1==tEOCMD) break;   
  if(t1==tINTNUM) {
    classN= str2int(value);
    if( (classN>50) || (classN<1) ) {
      rc=3; break;
    };
  } else {
    rc=3; break;
  };
  t1= nxtoken(line, value, &ixl);   // class group
  if(t1==tEOCMD) break;   
  if(t1==tINTNUM) {
    cg= str2int(value);
    if( cg>50 ) {
      rc=5; break;
    };
      } else {
    rc=5; break;
  };
  t1= nxtoken(line, value, &ixl);   // class group time
  if(t1==tEOCMD) break;   
  if(t1==tINTNUM) {
    cgtim= str2int(value);
  } else {
    rc=3; break;
  };
  t2= nxtoken1(line, value, &ixl);   // classname
  if(t2!=tSYMNAME) {rc=2; break;};
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
    //classN-1, value, cg, cgtime);
    classN-1, value, cg, cgtime, (const char **)daqlistp);
  sprintf(msg,"DAQlogbook_update_triggerClassName(%d,%d,%s,%d,%5.1f %d) rc:%d",
    runN, classN-1, value, cg, cgtime, daqlistpn, rcdaq);
  if(rcdaq!=0) {
    infolog_trg(LOG_ERROR, msg);
    printf("ERROR %s\n", msg);
    rc=4; break;
  } else {
    printf("INFO %s\n", msg);
    // without the test below, server crashes (or \n received indicating STOP)
    // ??? Possible reason: cannot print 2 consequtive INFO lines?
    if(daqlistpn>0) printalist(daqlistp);
  };
  fflush(stdout);
};
infolog_SetStream("",0);
return(rc);
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
if(rc==-1) {
  char emsg[200];
  strcpy(emsg,"aliases info from aliases.txt not updated correctly");
  infolog_trg(LOG_ERROR, emsg); printf("ERROR %s\n",emsg);
};
rc= getINT12fromcfg(INT1String, INT2String, MAXINT12LINE);
printf("INFO rc:%d INT1:%s INT2:%s\n", rc, INT1String, INT2String);

dis_add_error_handler(error_handler);
dis_add_exit_handler(exit_handler);
dis_add_client_exit_handler (client_exit_handler);
dis_add_cmnd(cmd,"C", DOcmd, 88);

sprintf(cmd, "%s/INT1", servername);   // CTPRCFG/INT1
INT1id=dis_add_service(cmd,"C:99", INT1String, MAXINT12LINE, NULL, 4567);
printf("INFO DIM service:%s id:%d\n", cmd, INT1id);
sprintf(cmd, "%s/INT2", servername);   // CTPRCFG/INT2
printf("INFO DIM service:%s\n", cmd);
INT2id=dis_add_service(cmd,"C:99", INT2String, MAXINT12LINE, NULL, 4568);
sprintf(cmd, "%s/CS", servername);   // CTPRCFG/CS
printf("INFO DIM service:%s\n", cmd);
CSid=dis_add_service(cmd,"C", CSString, MAXCSString, CScaba, 4569);

dis_start_serving(servername);  
while(1) {
  char *frc;
#define MAXLINECS 4000
  char line[MAXLINECS];   //80 chars per class for 50 classes
  //sleep(10);
  frc= fgets(line, MAXLINECS, stdin);
  if(frc==NULL) break;
  if(strncmp(line,"quit",4)==0) {
    break;
  } else if(strncmp(line,"class ",6)==0) {
    int rcdaq;
    printf("INFO igDAQLOGBOOK:%d line:%s",ignoreDAQLOGBOOK, line);
    rcdaq= daqlogbook_open();
    if(rcdaq==-1) {
      printf("ERROR DAQlogbook_open failed\n");
    } else {
      if(ignoreDAQLOGBOOK==0) {
        rcdaq= updateDAQDB(line);
        rcdaq= daqlogbook_close();
        if(rcdaq==-1) {
          printf("ERROR DAQlogbook_close failed\n");
        };
      };
    };
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
dis_remove_service(INT1id);
dis_remove_service(INT2id);
dis_remove_service(CSid);
dis_stop_serving();
return(grc);
}
