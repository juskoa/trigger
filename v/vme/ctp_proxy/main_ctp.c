/*
22.11.2011 when ACKNOWLEDGE received clean ERROR_REASON smi parameter:
strcpy(errorReason,"not set"); smi_set_parER();
22.11.
smi_setState() introduced
4.8.2014: executectp() added
*/
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <semaphore.h>    // sem_init
#ifdef CPLUSPLUS
#include <smirtl.hxx>
#else
#include <smirtl.h>
#endif

#include "infolog.h"
#include "vmewrap.h"
#include "vmeblib.h"
#include "ctp.h"
#define DBMAIN
#include "Tpartition.h"
#include "ctp_proxy.h"

#define PQWAY
//#undef PQWAY
void ds_register();

char errorReason[ERRMSGL];
char state[16];
char obj[128];

int quit=0;   // 1: do not start new runs >9 stop immediately   

void UPPER(char *strin);
#ifdef PQWAY
// see ctplib/posixq.c
#include <mqueue.h>
mqd_t mq_sendmsg;
mqd_t mq_rec;
mqd_t pq_open();
mqd_t pq_connect();
void pq_close(mqd_t mq, int unlink);
int pq_send(mqd_t mq, char *msg);
void pq_receive(mqd_t mq, char *msg);

#endif
char pname[64]="UNKNOWN"; 
char ORBIT_NUMBER[12]=""; 
char mask[128];
char ACT_CONFIG[8]="YES";
int run_number=0;
int detectors=0;   // ECS bits set for detectors in DETECTORS SMI par

/*---------------------------------------------*/ void gotsignal(int signum) {
char msg[100];
// SIGUSR1:  // kill -s USR1 pid
signal(signum, gotsignal); siginterrupt(signum, 0);
sprintf(msg, "got signal:%d", signum); prtLog(msg);
if((signum==SIGUSR1) || (signum==SIGQUIT) ) {
  int np;
  np= getNAllPartitions();
  if(np!=0) {
    //quit=1; // wait till all partitions stopped
    //sprintf(msg, "Waiting for the stop of all partitions before exit");
    sprintf(msg, "Unsuccessful attempt to stop ctpproxy, %d partitions active", np);
    prtLog(msg);
    pq_send(mq_sendmsg,"quitignored");
  } else {
    quit=10;  // stop immediately (no partitions loaded)
  };
} else if((signum==SIGKILL) || (signum==SIGINT) ) {
    sprintf(msg, "SIGINT, immediate stop"); prtLog(msg);
  quit=11; // =9. stop immediately
};
#ifdef PQWAY
if(quit>9) {
  pq_send(mq_sendmsg,"quit");
};
#endif
}

void smi_setState(char *newstate) {
if(strcmp(newstate, "RUNNING")==0) {
  // this probably (UNKNOWN) should be set for any newstate
  // (never called with "EXECUTING_FOR" anyhow)
  strcpy(pname,"UNKNOWN"); smi_set_par("EXECUTING_FOR",pname,STRING);
};
strcpy(state, newstate); smi_set_state(state);
}
void smi_set_parER() {
if(strlen(errorReason)>99) { errorReason[99]='\0'; };
smi_set_par("ERROR_REASON", errorReason, STRING);
}
void smi_set_parEF(char *pname) {
smi_set_par("EXECUTING_FOR",pname,STRING); smi_setState("EXECUTING");
}

void executectp(char *sendcmd) {
int rc, errorway=0;
char cmd[100];
#ifdef PQWAY
if(strcmp(sendcmd,"wait")==0) {
  pq_receive(mq_rec, cmd);
  printf("executectp received:%s.\n", cmd);
} else {
  printf("executectp send:%s.\n", sendcmd);
  pq_send(mq_sendmsg, sendcmd);
  fflush(stdout); return;
};
#endif
errorReason[0]= '\0'; smi_set_parER();
if(strcmp(cmd,"LOAD")==0) {
  printf("%s partition: %s mask: %s run:%d\n", cmd,pname, mask,run_number);
  rc= ctp_LoadPartition(pname,mask,run_number,ACT_CONFIG, errorReason);
} else if(strcmp(cmd,"INIT")==0) {
  printf("%s partition: %s mask: %s run:%d\n", cmd,pname, mask,run_number);
  rc= ctp_InitPartition(pname,mask,run_number,ACT_CONFIG, errorReason);
} else if(strcmp(cmd,"START_PARTITION")==0) {
  printf("Starting partition: %s\n", pname);
  rc= ctp_StartPartition(pname, errorReason);
} else if(strcmp(cmd,"STOP_PARTITION")==0) {
  printf("Stopping partition: %s\n", pname);
  rc= ctp_StopPartition(pname); errorway=1;
} else if(strcmp(cmd,"PAUSE_PARTITION")==0) {
  rc= ctp_PausePartition(pname, detectors); errorway= 1;
} else if(strcmp(cmd,"RESUME_PARTITION")==0) {
  rc= ctp_ResumePartition(pname, detectors); errorway= 1;
} else if(strcmp(cmd,"SYNC")==0) {
  w32 orbitn;
  rc= ctp_SyncPartition(pname, errorReason, &orbitn);
  if(rc!=0) {
    printf("ctp_SyncPartition() rc:%d %s\n", rc,errorReason);
    smi_set_parER();
    errorway= 1;
  } else {
    sprintf(ORBIT_NUMBER,"%u", orbitn);
    smi_set_par("ORBIT_NUMBER", ORBIT_NUMBER, STRING);
  }
} else if(strcmp(cmd,"quitignored")==0) {
    infolog_trgboth(LOG_ERROR, "Unsuccessful attempt to stop ctpproxy, partitions active");
} else {
  printf("executectp cmd ignored: %s\n", cmd);
  fflush(stdout); return;
};
if(rc){
  if(errorway) { // i.e. errorReason set already in smi
    sleep(1); smi_setState("ERROR");
  } else {
    printf("%s rc:%d %s\n", cmd, rc, errorReason);
    smi_set_parER();
    sleep(1); smi_setState("LOAD_FAILURE");
  };
}else{
  sleep (1); smi_setState("RUNNING");
};
fflush(stdout);
}
/*--------------------------------------------------- SMI_handle_command()
*/
void SMI_handle_command() {
char action[64], param[128], parname[64], msg[256];
char run_number_str[64];
int n_params, ptype, psize, i;

prtProfTime(NULL);
smi_get_action(action,&n_params); UPPER(action); strcpy(msg,"");
if(n_params != 2){
  printf("# of pars received: %i. \n",n_params);
  //exit(8);
};
for (i=1;i<=n_params;i++) {
  smi_get_next_par(parname,&ptype,&psize);
  smi_get_par_value(parname,param);
  printf("SMI: parname=%s  param=%s \n", parname,param);
  if(strcmp(parname,"PARTITION") == 0) {
    strcpy(pname,param);
  } else if(strcmp(parname,"MASK") == 0){
    strcpy(mask,param);
  } else if(strcmp(parname,"MODE") == 0){
    if((strcmp(param,"NOTHING")!=0) &&
       (strcmp(param,"PHYSICS")!=0) &&
       (strcmp(param,"UNDEFINED")!=0)) {
      strcpy(partmode,param);
    } else {
      strcpy(partmode,"");
    };
  } else if(strcmp(parname,"RUN_NUMBER") == 0) {
    strcpy(run_number_str,param);
    //run_number=atoi(run_number_str)
    run_number= (int) strtol(run_number_str, (char **)NULL, 10);
    if(run_number<=0) {
      printf("RUN_NUMBER <=0\n");
      smi_setState("ERROR"); goto RETSMI;
    };
  } else if(strcmp(parname,"ACT_CONFIG") == 0) {
    strcpy(ACT_CONFIG,param);
  } else if(strcmp(parname,"DETECTORS") == 0) {
    // comma separated list of dets (e.g. "PMD,MUON_TRK")
    detectors= detList2bitpat(param);   // "CTP" will be ignored (bit not set)
    if(detectors==-1) {
      sprintf(errorReason, "Bad list of detectors:%s...",param);
      smi_set_parER();
      smi_setState("ERROR"); goto RETSMI;
    };
    printf("detectors:%s= 0x%x\n",param, detectors);
  } else {
    char wmsg[200];
    sprintf(wmsg,"unknown parameter from ECS:%s", param); prtWarning(wmsg);
  };
  strcat(msg," ");
  strcat(msg,param);
};   
printf("%s Got action %s with parameters %s\n",obj,action,msg);
   
if (strcmp(state,"RUNNING") == 0 ) {
  if((strcmp(action,"LOAD_PARTITION") == 0) || (strcmp(action,"INIT_PARTITION") == 0)) {
    char INITLOAD[8];
    if(strcmp(action,"LOAD_PARTITION") == 0) {
      strcpy(INITLOAD,"LOAD");
    } else {
      strcpy(INITLOAD,"INIT");
    };
    if(quit>0) {
      sprintf(msg,"%s_PARTITION ignored. ctp_proxy stopping, waiting for the stop of all partitions",INITLOAD);
      infolog_trgboth(LOG_FATAL, msg);
    } else {
      smi_set_parEF(pname);
      executectp(INITLOAD);
    };
  } else if (strcmp(action,"START_PARTITION") == 0) {
    smi_set_parEF(pname);
    executectp(action);
  } else if (strcmp(action,"STOP_PARTITION") == 0) {
    smi_set_parEF(pname);
    executectp(action);
  } else if (strcmp(action,"PAUSE_PARTITION") == 0) {
    smi_set_parEF(pname);
    executectp(action);
  } else if (strcmp(action,"SYNC") == 0) {   // correct: strcmp("SYNC")
    smi_set_parEF(pname);
    executectp(action);
  } else if (strcmp(action,"RESUME_PARTITION") == 0) {
    smi_set_parEF(pname);
    executectp(action);
  } else {
    char msg[200];
    sprintf(msg, "Illegal action %s in state RUNNING ignored.\n",action);
    infolog_trgboth(LOG_ERROR, msg);
    smi_setState("RUNNING");
  }
} else if (strcmp(state,"EXECUTING") == 0 ) {
  char msg[200];
  sprintf(msg, "Illegal action %s in state EXECUTING ignored.\n",action);
  smi_setState("EXECUTING");
} else if (strcmp(state,"LOAD_FAILURE") == 0 ) {
    if(strcmp(action,"ACKNOWLEDGE") == 0 ) {
      strcpy(errorReason,"not set"); smi_set_parER();
      smi_setState("RUNNING");
    } else {
      printf("Illegal action %s in state LOAD_FAILURE\n",action);
      smi_setState("LOAD_FAILURE");
    }   
} else if (strcmp(state,"ERROR") == 0 ) {
    if (strcmp(action,"RESET") == 0 ) {
      smi_setState("RUNNING");
    } else {
      printf("Illegal action %s in state ERROR\n",action);
      smi_setState("ERROR");
    }   
};      
RETSMI:
prtProfTime("SMI_handle end");
return;
}
/*----------------------------------------*/ int main(int argc, char **argv) {
int rc;
infolog_SetFacility("CTP"); infolog_SetStream("",0);
#ifdef PQWAY
printf("main_ctp: opening rec/send queues...\n");
mq_rec= pq_open();
if(mq_rec==(mqd_t)-1) {
  infolog_trgboth(LOG_FATAL, "posix mq_rec not created");
  exit(8);
}
mq_sendmsg= pq_connect();
if(mq_sendmsg==(mqd_t)-1) {
  infolog_trgboth(LOG_FATAL, "posix mq_sendmsg not connected");
  exit(8);
}
#endif
signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
signal(SIGQUIT, gotsignal); siginterrupt(SIGQUIT, 0);
signal(SIGKILL, gotsignal); siginterrupt(SIGKILL, 0); // -9
signal(SIGTERM, gotsignal); siginterrupt(SIGTERM, 0); // kill pid
signal(SIGINT, gotsignal); siginterrupt(SIGINT, 0);   // CTRL C   2
partmode[0]='\0';
printf("cshmInit i.e. initBakery(swtriggers/ccread/ssmcr ONLY once, when shm allocated)...\n");
/*printf("initBakery(swtriggers,4): 0:SOD/EOD 1:gcalib 2:ctp.exe 3:dims\n");
printf("initBakery(ccread,5): 0:proxy 1:dims 2:ctp+busytool 3:smaq 4:inputs\n");
*/
cshmInit();
setglobalflags(argc, argv);

/* changed in aug2016 (initBakery only once from now, when shm allocated)
printf("initBakery(swtriggers,4): 0:SOD/EOD 1:gcalib 2:ctp.exe 3:dims\n");
initBakery(&ctpshmbase->swtriggers, "swtriggers", swtriggers_N);
printf("initBakery(ccread,6): 0:proxy 1:dims 2:ctp+busytool 3:smaq 4:inputs 5:orbitddl2\n");
initBakery(&ctpshmbase->ccread, "ccread", ccread_N);
printf("initBakery(ssmcr,4): 0:smaq 1:orbitddl2 2:ctp 3:inputs\n");
initBakery(&ctpshmbase->ssmcr, "ssmcr", ssmcr_N);
*/
printBakery(&ctpshmbase->swtriggers);
printBakery(&ctpshmbase->ccread);
printBakery(&ctpshmbase->ssmcr);
unlockBakery(&ctpshmbase->swtriggers,swtriggers_ctpproxy);
unlockBakery(&ctpshmbase->ccread,ccread_ctpproxy);
unlockBakery(&ctpshmbase->ssmcr,ssmcr_ctpproxy);

/* Let' synchronise with smcr only, i.e. 
- ccread_dims,... can go in parallel with ctp_Initproxy, 
  orbitddl2: should use ccread_orbitddl2 customer when reading counters
  
- swtriggers_gcalib/_dims cannot appear (no global runs becasue ctpproxy being restarted)
*/
while(1) {  // do not allow SSM usage (smaq) because ctp_Initproxy is initialising it
  rc= lockBakeryTimeout(&ctpshmbase->ssmcr,ssmcr_ctpproxy, 10);
  if(rc==1) break;  // locked
  infolog_trgboth(LOG_WARNING, "Waiting for ssmcr resource. Is smaq stopped?");
};
if(isArg(argc, argv, "configrunset")) {
  /* do we need before orbitddl2.py INT/L2 orbit in SYNC (automatic with L2a)?
  */
  int rc, reslen;
  char cmd[200];
  char result[1000]="";   // ~ 10 lines
  char orbchanged[]="Warning: orbitoffset changed:";
  if(envcmp("VMESITE", "PRIVATE")==0) {
    strcpy(cmd, "who");
  } else {
    strcpy(cmd, "orbitddl2.py configrunset");
  };
  infolog_trgboth(LOG_INFO, "Starting L0 orbit calibration (30s...)");
  rc= popenread(cmd, result, 1000);   // opens vme...
  reslen= strlen(result);
  if((rc==EXIT_FAILURE) || (reslen<=1)) { 
    infolog_trgboth(LOG_ERROR, "L0 orbit calibration problem");
  } else {
    int ixr= reslen;
    if(result[ixr-1]=='\n') {   //remove last NL character if present
      result[ixr-1]='\0';
      ixr= reslen-2;
    } else {
      ixr= reslen-1;   // pointer to the last non-NEWLINE character
    };
    while(ixr>=0) {    // find last line
      if(result[ixr]=='\n') {
        ixr++; break;
      };
      ixr--;
    };
    sprintf(cmd, "L0 orbit calibration: %s", &result[ixr]);
    if((strcmp(&result[ixr], "Everything ok")==0) ||
       (strncmp(&result[ixr], orbchanged, strlen(orbchanged))==0)
      ) {
      infolog_trgboth(LOG_INFO, cmd);
    } else {
      infolog_trgboth(LOG_ERROR, cmd);
      infolog_trgboth(LOG_ERROR, "ctpproxy not started"); 
      unlockBakery(&ctpshmbase->ssmcr,ssmcr_ctpproxy);
      rc=8; goto STP;
    };
  };
};
// init CTP after calibration (e.g. to repair modifications done by orbitddl2.py)
rc=ctp_Initproxy();
unlockBakery(&ctpshmbase->ssmcr,ssmcr_ctpproxy);
if(rc!=0) goto STP;

// DIM services not registered here (see ctpdims.c), they run in separae task:
// ds_register();
strcpy(obj,argv[1]);
smi_attach(obj, SMI_handle_command);
printf("CTP attached to: %s\n",obj);
/* if smi_volatile: stops the ctp_proxy in case TRIGGER domain is down
smi_volatile();   
*/
strcpy(errorReason,"not set"); smi_set_parER();
strcpy(ORBIT_NUMBER,""); smi_set_par("ORBIT_NUMBER",ORBIT_NUMBER,STRING);
smi_setState("RUNNING");
while(1) {
#ifdef PQWAY
  executectp("wait");
#else
  usleep(1000000);
#endif
  if(quit>9) break;
};
rc= ctp_Endproxy();
STP:
#ifdef PQWAY
pq_close(mq_sendmsg, 0);
pq_close(mq_rec, 1);
#endif
printf("Calling cshmDetach()...\n"); cshmDetach();
return (rc);
}
