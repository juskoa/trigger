/*
22.11.2011 when ACKNOWLEDGE received clean ERROR_REASON smi parameter:
strcpy(errorReason,"not set"); smi_set_parER();
22.11.
smi_setState() introduced
*/
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
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

void ds_register();

char state[16];
char obj[128];

int quit=0;   // 1: do not start new runs >9 stop immediately   

void UPPER(char *strin);
char pname[64]="UNKNOWN"; 

/*---------------------------------------------*/ void gotsignal(int signum) {
char msg[100];
// SIGUSR1:  // kill -s USR1 pid
signal(signum, gotsignal); siginterrupt(signum, 0);
sprintf(msg, "got signal:%d", signum); prtLog(msg);
if((signum==SIGUSR1) || (signum==SIGQUIT) ) {
  int np;
  if((np= getNAllPartitions())) {
    quit=1; // wait till all partitions stopped
    sprintf(msg, "Waiting for the stop off all partitions before exit"); prtLog(msg);
  } else {
    quit=10;  // stop immediately (no partitions loaded)
  };
} else if(signum==SIGKILL) {
    sprintf(msg, "SIGKILL, immediate stop"); prtLog(msg);
  quit=10; // =9. stop immediately
};
}

void smi_set_parER() {
if(strlen(errorReason)>99) { errorReason[99]='\0'; };
smi_set_par("ERROR_REASON", errorReason, STRING);
}
void smi_setState(char *newstate) {
strcpy(state, newstate); smi_set_state(state);
}

/*--------------------------------------------------- SMI_handle_command()
*/
void SMI_handle_command() {
char action[64], param[64], parname[64], msg[256];
int n_params, ptype, psize, i, run_number=0;
char mask[64],run_number_str[64];
char ACT_CONFIG[8]="YES";

smi_get_action(action,&n_params); UPPER(action); strcpy(msg,"");
if(n_params != 2){
  printf("# of pars received: %i. \n",n_params);
  //exit(8);
};
for (i=1;i<=n_params;i++) {
  smi_get_next_par(parname,&ptype,&psize);
  smi_get_par_value(parname,param);
  printf("SMI: parname=%s  param=%s \n", parname,param);
  if(strcmp(parname,"PARTITION") == 0)strcpy(pname,param);
  if(strcmp(parname,"MASK") == 0)strcpy(mask,param);
  if(strcmp(parname,"MODE") == 0){
    if((strcmp(param,"NOTHING")!=0) &&
       (strcmp(param,"PHYSICS")!=0) &&
       (strcmp(param,"UNDEFINED")!=0)) {
      strcpy(partmode,param);
    } else {
      strcpy(partmode,"");
    };
  };
  if(strcmp(parname,"RUN_NUMBER") == 0) {
    strcpy(run_number_str,param);
    //run_number=atoi(run_number_str)
    run_number= (int) strtol(run_number_str, (char **)NULL, 10);
    if(run_number<=0) {
      printf("RUN_NUMBER <=0\n");
      smi_setState("ERROR"); goto RETSMI;
    };
  };
  if(strcmp(parname,"ACT_CONFIG") == 0) {
    strcpy(ACT_CONFIG,param);
  };
  strcat(msg," ");
  strcat(msg,param);
};   
printf("%s Got action %s with parameters %s\n",obj,action,msg);
   
if (strcmp(state,"RUNNING") == 0 ) {
  if (strcmp(action,"LOAD_PARTITION") == 0) {
    int rc;
    if(quit>0) {
      sprintf(msg,"LOAD_PARTITION ignored. ctp_roxy stopping, waiting for the stop of all partitions");
      infolog_trgboth(LOG_FATAL, msg);
    } else {
      smi_set_par("EXECUTING_FOR",pname,STRING); smi_setState("EXECUTING");
      printf("Loading partition: %s mask: %s run:%d\n", pname,mask,run_number);
      rc= ctp_LoadPartition(pname,mask,run_number,ACT_CONFIG, errorReason);
      if(rc){
        printf("ctp_LoadPartition() rc:%d %s\n", rc, errorReason);
        smi_set_parER();
        sleep(1); smi_setState("LOAD_FAILURE");
      }else{
        sleep (1); smi_setState("RUNNING");
      }
    };
  } else if (strcmp(action,"START_PARTITION") == 0) {
    int rc;
    smi_set_par("EXECUTING_FOR",pname,STRING); smi_setState("EXECUTING");
    printf("Starting partition: %s\n", pname);
    rc= ctp_StartPartition(pname, errorReason);
    if(rc){
      printf("ctp_StartPartition() rc:%d %s\n", rc,errorReason);
      smi_set_parER();
      sleep(1); smi_setState("LOAD_FAILURE");
    }else{
      sleep (1); smi_setState("RUNNING");
    }
  } else if (strcmp(action,"STOP_PARTITION") == 0) {
    smi_set_par("EXECUTING_FOR",pname,STRING); smi_setState("EXECUTING");
    printf("Send EOD and then unload the partition\n");
    if(ctp_StopPartition(pname)){
      sleep(1); smi_setState("ERROR");
    }else{
      sleep (1); smi_setState("RUNNING");
    }
  } else if (strcmp(action,"PAUSE_PARTITION") == 0) {
    if(ctp_PausePartition(pname)){
      sleep (1); smi_setState("ERROR");
    }else{
      sleep (1); smi_setState("RUNNING");
    }
  } else if (strcmp(action,"RESUME_PARTITION") == 0) {
    if(ctp_ResumePartition(pname)){
      sleep (1); smi_setState("ERROR");
    } else {
      sleep (1); smi_setState("RUNNING");
    }
  } else {
    printf("Illegal action %s in state RUNNING\n",action);
  }
} else if (strcmp(state,"EXECUTING") == 0 ) {
  printf("Illegal action %s in state EXECUTING\n",action);
} else if (strcmp(state,"LOAD_FAILURE") == 0 ) {
    if(strcmp(action,"ACKNOWLEDGE") == 0 ) {
      strcpy(errorReason,"not set"); smi_set_parER();
      smi_setState("RUNNING");
    } else {
      printf("Illegal action %s in state LOAD_FAILURE\n",action);
    }   
} else if (strcmp(state,"ERROR") == 0 ) {
    if (strcmp(action,"RESET") == 0 ) {
      smi_setState("RUNNING");
    } else {
      printf("Illegal action %s in state ERROR\n",action);
    }   
};      
RETSMI:
return;
}
/*----------------------------------------*/ int main(int argc, char **argv) {
int rc;
signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
signal(SIGQUIT, gotsignal); siginterrupt(SIGQUIT, 0);
signal(SIGKILL, gotsignal); siginterrupt(SIGKILL, 0); // -9
signal(SIGTERM, gotsignal); siginterrupt(SIGTERM, 0); // kill pid
signal(SIGINT, gotsignal); siginterrupt(SIGINT, 0);   // CTRL C
partmode[0]='\0';
infolog_SetFacility("CTP"); infolog_SetStream("",0);
printf("cshmInit i.e. initBakery(swtriggers/ccread if shm allocated)...\n");
/*printf("initBakery(swtriggers,3): 0:SOD/EOD 1:gcalib 2:ctp.exe + dims\n");
printf("initBakery(ccread,5): 0:proxy 1:dims 2:ctp+busytool 3:smaq 4:inputs\n");
*/
cshmInit();

printf("initBakery(swtriggers,3): 0:SOD/EOD 1:gcalib 2:ctp.exe + dims\n");
initBakery(&ctpshmbase->swtriggers, "swtriggers", 3);
printf("initBakery(ccread,5): 0:proxy 1:dims 2:ctp+busytool 3:smaq 4:inputs\n");
initBakery(&ctpshmbase->ccread, "ccread", 5);

setglobalflags(argc, argv);
if((rc=ctp_Initproxy())!=0) exit(8);

// DIM services not registered here (see ctpdims.c), they run in separae task:
// ds_register();
strcpy(obj,argv[1]);
smi_attach(obj, SMI_handle_command);
printf("CTP attached to: %s\n",obj);
/* if smi_volatile: stops the ctp_proxy in case TRIGGER domain is down
smi_volatile();   
*/
strcpy(errorReason,"not set"); smi_set_parER();
strcpy(pname,"UNKNOWN"); smi_set_par("EXECUTING_FOR",pname,STRING);
smi_setState("RUNNING");
while(1) {
  usleep(1000000);
  if(quit>9) break;
};
rc= ctp_Endproxy();
return (0);
}
