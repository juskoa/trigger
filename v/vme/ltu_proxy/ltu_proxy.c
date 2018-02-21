/* proxy for ltu. 
Start: 
cd $VMEWORKDIR ; $VMECFDIR/ltu_proxy/Linux/ltu_proxy.exe ...
See v/DOC/history.txt for history of modifications */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <getopt.h>

#ifdef CPLUSPLUS
#include <smirtl.hxx>
#else
#include <smirtl.h>
#endif

#include "infolog.h"
#include "vmewrap.h"
#include "vmeblib.h"
/* we use intermediate DIM (ltucfg2daq()...
  #include "daqlogbook.h"

   */
#define LTUMAIN
#include "ltu.h"
#define PROXY_MAIN
#include "ltu_utils.h"

Tltucfg *ltc;
Tltucfg templtucfgmem;
int sodeodecs=1;
char state[32];
char obj[128];
char msg[500];
   
char ltu_dimservername[36];
char dimservername[36];
//char BoardName[]="ltu";
   
int ds_register(char *detname, char *base);
void setRWMODE(char mode);
void UPPER(char *name);
int syncemu();

extern int QUIT;
extern w32 *buf1;
void updateMONCOUNTERSservice(int uc);    // update DIM service 1/min
void updateMONBUSY(float newbusyf);   
void readltucounters();

/*------------------------------------------------------------ isNotNONE()
rc: 1 if defined
    0 if NONE, UNDEFINED or ""
*/
int isNotNONE(char *parval) {
if((strncmp(parval,"NONE",4) != 0) && (parval[0]!='\0') &&
  (strncmp(parval,"UNDEFINED",9) != 0) ){
  return(1);
} else {
  return(0);
};
}
void SMI_handle_command() {
   char action[64], param[64], parname[64];
   char configuration[32]="";
   char mode[32]="";
   char FineDelay1[32]="";
   char NAME[32]="";
   char VALUE[32]="";
   int n_params, ptype, psize, i; int RUN_NUMBER=-1;
   int vmeopened=0; // let's open/close VME with each SMI command

if(templtucfg->plist[IXdefedit_id] != ltc->plist[IXdefedit_id]) {
  // parameters were changed interactively in Defautls editor', take fresh copy
  copyltucfg(templtucfg, ltc);
  printf("SHM->templtucfg:%d\n", ltc->plist[IXdefedit_id]);
};
   smi_get_action(action,&n_params);
   UPPER (action);
   for (i=1;i<=n_params;i++) {
      smi_get_next_par(parname,&ptype,&psize);
      smi_get_par_value(parname,param);
      if (strcmp(parname,"CONFIGURATION") == 0 ) {
        strcpy(configuration,param);
      } else if (strcmp(parname,"MODE") == 0 ) {
        strcpy(mode,param);
        /* configuration/mode will be consider only in case of START cmd */ 
      } else if (strcmp(parname,"PARTITION_NAME") == 0 ) {
        strcpy(PARTITION_NAME,param);
      } else if (strcmp(parname,"FINEDELAY1") == 0 ) {   // upper case in ECS!
        strcpy(FineDelay1,param);
      } else if (strcmp(parname,"NAME") == 0 ) {
        strcpy(NAME,param);
      } else if (strcmp(parname,"VALUE") == 0 ) {
        strcpy(VALUE,param);
      } else if (strcmp(parname,"RUN_NUMBER") == 0 ) {
        RUN_NUMBER= atoi(param);
      } else {
        printf("Error: %s <- %s ignored.\n", parname, param); 
        strcpy(state,"ERROR"); smi_set_state(state); goto RETURN;
      };
   };   
   if(vmeopen(BoardBaseAddress,BoardSpaceLength)) {
     printf(" Cannot open vme %s\n", BoardBaseAddress);
     goto SETERROR;
   }else {
     vmeopened=1;
   };
   infolog_SetStream(dimservernameCAP,RUN_NUMBER);
   if (strcmp(state,"STANDALONE_STOPPED") == 0) {
     if (strcmp(action,"START") == 0) {
       /* options given from ECS overwrite options
       given in defaults file loaded at the time of ltu_proxy start */
       int rcse;
       strcpy(state,"STANDALONE_STARTING"); smi_set_state(state);
       //von infolog_SetStream(dimservernameCAP,RUN_NUMBER);
       if( isNotNONE(configuration) ) {
         int rc;
         rc= setOptionMem((char *)"L2aseq", configuration,templtucfg);
         if(rc!=0) {
           sprintf(msg, "Bad CONFIGURATION:%s", configuration);
           infolog_trg(LOG_FATAL, msg); goto SETERROR;
         };
       }; /* do nothing if not supplied!  */
       if( isNotNONE(mode) ) {
         int rc;
         rc= setOptionMem((char *)"mode", mode,templtucfg);
         if(rc!=0) {
           sprintf(msg, "Bad MODE:%s", mode);
           infolog_trg(LOG_FATAL, msg); goto SETERROR;
         };
       }; /* do nothing if not supplied!  */
       if( isNotNONE(FineDelay1) ) {
         int rc;
         rc= setOptionMem((char *)"FineDelay1", FineDelay1,templtucfg);
         if(rc!=0) {
           sprintf(msg, "Bad FineDelay1:%s", FineDelay1);
           infolog_trg(LOG_FATAL, msg); goto SETERROR;
         };
       };
       if((rcse= checkSEU())) {
         infolog_trg(LOG_ERROR, (char *)"Single event upset detected, reconfigure LTU FPGA");
       };
       busy12(1);
       // check busy:
       rcse= busystatus();
       if(rcse==0) {
         printf("%s --> BUSY OFF, starting emulator %s\n",
           obj,  templtucfg->mainEmulationSequence);
         //  obj,  ltc->mainEmulationSequence);
         sodeodecs= templtucfg-> ltu_sodeod_present;
         rcse= startemu();   // templtucfg is restored back from ltushm here after start
       } else {
         sprintf(msg, "BUSY ON, going to ERROR state\n");
         printf(msg);
         infolog_trg(LOG_ERROR, msg);
       };
       if(rcse!=0) {
         goto SETERROR;
       } else {
         setRWMODE('R');
         strcpy(state,"STANDALONE_RUNNING"); smi_set_state(state);
       };
     } else if (strcmp(action,"NV_GOTOGLOBAL") == 0) {
       int rcdaq; 
       Tltucfg1 ltucfg;
       //w32 fd1,fd2,bcd; 
       if((rcdaq= checkSEU())) {
         infolog_trg(LOG_ERROR, (char *)"Single event upset detected, reconfigure LTU FPGA");
       };
       Setglobalmode();
       printf("%s -->    GLOBAL.\n",obj);
       busy12(1);
       setRWMODE('R');
       strcpy(state,"GLOBAL"); smi_set_state(state);
       //von infolog_SetStream(dimservernameCAP,RUN_NUMBER);
       /* ---> DIM -> alitri -> daqlogbook way: */
       ltucfg.run= RUN_NUMBER;
       strcpy(ltucfg.detector, dimservernameCAP);
       ltucfg.LTUFineDelay1= templtucfg->FineDelay1; 
       ltucfg.LTUFineDelay2= templtucfg->FineDelay2;
       ltucfg.LTUBCDelayAdd= templtucfg->bc_delay_add;
       rcdaq= ltucfg2daq(&ltucfg);
       sprintf(msg,"ltucfg2daq(%d,%s,%d,%d,%d) rc:%d",
         ltucfg.run,ltucfg.detector, ltucfg.LTUFineDelay1,
         ltucfg.LTUFineDelay2, ltucfg.LTUBCDelayAdd, rcdaq);
       //
       /* --->                  daqlogbook way: 
       rcdaq= daqlogbook_open();
       if(rcdaq==-1) {
         sprintf(msg,"DAQlogbook not opened");
         infolog_trg(LOG_ERROR, msg);
       };
       rcdaq= daqlogbook_update_LTUConfig(RUN_NUMBER, dimservernameCAP,
         fd1, fd2, bcd);
       if(rcdaq!=0) {
         sprintf(msg,"DAQlogbook_update_LTUConfig(%d,%s,%d,%d,%d) rc:%d",
           RUN_NUMBER,dimservernameCAP, fd1, fd2, bcd, rcdaq);
         infolog_trg(LOG_INFO, msg);
       };
       rcdaq= daqlogbook_close();
       */
       infolog_trg(LOG_INFO, msg);
     } else if( strcmp(action, "SET_DEFAULT")==0) {
       //printltuDefaultsMem(templtucfg);
       if(setOptionMem(NAME, VALUE,templtucfg)!=0) {
         strcpy(state,"ERROR"); smi_set_state(state); goto RETURN;
       }; 
       //printltuDefaultsMem(templtucfg);
       printf("staying in state:%s\n",state);
         /* strcpy(state,"ERROR"); */smi_set_state(state); goto RETURN;
     } else {goto RETERR;}
   } else if (strcmp(state,"STANDALONE_RUNNING") == 0) {
      if (strcmp(action,"STOP") == 0) {
        int rcse;
        strcpy(state,"STANDALONE_STOPPING"); smi_set_state(state);
        stopemu(sodeodecs);
        printf("%s --> Stop the emulator and waiting 2 secs why?\n",obj);
        sleep (2);
        strcpy(state,"STANDALONE_STOPPED"); smi_set_state(state);
        setRWMODE('W');
        if((rcse= checkSEU())) {
          infolog_trg(LOG_ERROR, (char *)"Single event upset detected, reconfigure LTU FPGA");
        };
        //von RUN_NUMBER=-1; infolog_SetStream(dimservernameCAP,RUN_NUMBER);
      } else if (strcmp(action,"PAUSE") == 0) {
        //strcpy(state,"STANDALONE_STOPPING"); smi_set_state(state);
        // keep commented (from 10.4.)
        pauseemu();
        printf("%s --> Stops the emulator and waiting 2 secs why?\n",obj);
        sleep (2);
        strcpy(state,"STANDALONE_PAUSED"); smi_set_state(state);
      } else {goto RETERR;}
   } else if (strcmp(state,"STANDALONE_PAUSED") == 0) {
      if (strcmp(action,"STOP") == 0) {
         //strcpy(state,"STANDALONE_STOPPING"); smi_set_state(state);
         eodemu(sodeodecs);
         sleep (2);
         strcpy(state,"STANDALONE_STOPPED"); smi_set_state(state);
         setRWMODE('W');
      } else if (strcmp(action,"RESUME") == 0) {
         //strcpy(state,"STANDALONE_STARTING"); smi_set_state(state);
         printf("%s --> resuming emulator\n",obj); fflush(stdout);
         resumeemu();
         sleep (2);
         strcpy(state,"STANDALONE_RUNNING"); smi_set_state(state);
      } else if (strcmp(action,"SYNC") == 0) {
         int rc;
         rc=syncemu();
         printf("%s --> SYNC rc: %d\n",obj, rc); fflush(stdout);
         sleep (2);
         strcpy(state,"STANDALONE_PAUSED"); smi_set_state(state);
         printf("STANDALONE_PAUSED set again.\n"); fflush(stdout);
      } else {goto RETERR;}
   } else if (strcmp(state,"GLOBAL") == 0 ) {
      if (strcmp(action,"NV_GOTOSTANDALONE_STOPPED") == 0) {
        int rcse;
        Setstdalonemode();
        printf("%s -->    STDALONE. \n",obj);
        busy12(1);
        strcpy(state,"STANDALONE_STOPPED"); smi_set_state(state);
        setRWMODE('W');
        if((rcse= checkSEU())) {
          infolog_trg(LOG_ERROR, (char *)"Single event upset detected, reconfigure LTU FPGA");
        };
        //von RUN_NUMBER=-1; infolog_SetStream(dimservernameCAP,RUN_NUMBER);
      } else {goto RETERR;}
   } else if (strcmp(state,"ERROR") == 0) {
      if (strcmp(action,"RESET") == 0) {
         if(vmeopened==1) SLMquit();
         strcpy(state,"STANDALONE_STOPPED"); smi_set_state(state);
         setRWMODE('W');
         printf("%s in state %s\n",obj, state);
         //sleep (2);        
      } else {goto RETERR;}
   };
RETURN: 
vmeclose(); vmeopened=0; return;
SETERROR:
setRWMODE('W'); strcpy(state,"ERROR"); smi_set_state(state); goto RETURN;
RETERR:
sprintf(msg, "SMI command:%s ignored in state:%s\n", action, state);
printf(msg);
infolog_trgboth(LOG_ERROR, msg); goto RETURN;
}

int main(int argc, char **argv) {
int rcso=0;
int argi=0;  
int notupdated=0, isecs=60; float prevbusyf, newbusyf=0;
/*----------------------------------------------------------- defaults: */
strcpy(BoardBaseAddress, "0xdeadde");
strcpy(BoardSpaceLength, "0x1000");
templtucfg= &templtucfgmem;
/* following done only once in 1st ltu.exe popened:
ltuDefaults(&ltushm->ltucfg); ttcDefaults(&ltushm->ltucfg); readltuttcdb(&ltushm->ltucfg);
*/
sprintf(msg, "ltu_proxy started. LTU_SW_VER:%s compiled:%s %s",LTU_SW_VER, __DATE__, __TIME__); prtLog(msg);
/*--------------------------------------------------------cmdline args: */
while (1) {
  int this_option_optind = optind ? optind : 1;
  int c; 
  int option_index = 0;
  char *opt;
  static struct option long_options[] = {
    {"mode", 1, 0, 0},
    {"address", 1, 0, 0}, {"orbitbc", 1, 0, 0},
    {"BCrate", 1, 0, 0}, {"busy", 1, 0, 0}, {"L0", 1, 0, 0},
    {"nosodeod", 0, 0, 0}, {"nodim", 0, 0, 0},
    {0, 0, 0, 0}
  };
  c = getopt_long_only (argc, argv, "h", long_options, &option_index);
  //printf("c:%d option_index:%d\n",c, option_index);
  if (c == -1) { break; }
  switch (c) {
    case 0:
      opt=(char *)long_options[option_index].name;
      /*printf("opt:%s has_arg:%d flag:%d val:%d\n",opt,
        long_options[option_index].has_arg,
        *(long_options[option_index].flag),
        long_options[option_index].val); */
      printf("opt:%s optarg:%s\n", opt, optarg);
      /*------------------------------ consider 'address' option only !!!
       I.E. IGNORE ALL THE OTHERS
      rcso= setOptionMem(opt, optarg) cannot be here (ltushm not yet defined) ;*/
      if(strcmp(opt,"address")==0) {
        strcpy(BoardBaseAddress, optarg);
      } else {
        printf("opt:%s optarg:%s IGNORED\n", opt, optarg);
      };
      break;
    case 'h':
      rcso=1;
      break;
    default:
      printf("Bad option %s\nUse -h for help\n",argv[this_option_optind]);
      rcso=1;
  };
  if(rcso!=0) {
    printf("Usage: %s SMIOBJECT [options]\n\n",argv[0]);
    printf("Options: (case insensitive)\n");
    printf("  -mode=BC | pulser_edge | pulser_level | random | sw (default:BC)\n");
    printf("  -busy=1 | 2 | 3 | 0                      (default:1 )\n");
    printf("  -L0=ttc | cable                          (default:ttc )\n");
    printf("  -address=hexa VME base address of LTU    (default:0x810000)\n");
    printf("  -BCrate=event rate in BC or random mode (Hz) (default:100.0)\n");
    printf("  -nosodeod                                (disable Start of Data / End of Data events)\n");
    printf("  -h                                       (this help)\n");
    exit(1);
  };
};

//printf("ddimname:%d %d %s.\n", optind, argc, argv[1]);
if (optind < argc) {
  while (optind < argc) {
    int ix;
    argi++;
    switch (argi) {
      case 1: /* SMI object */
        strncpy(obj,argv[optind],128);
        /* "TRIGGER::LTU-HMPID", i.e. dim name is after '-': */
        //printf("dimname:%s\n", obj);
        dimservername[0]='\0';
        for(ix=0; ix<128; ix++) {
          if(obj[ix]=='-') {
            int ix2;
            //strcpy(dimservername, &obj[ix+1]); 
            for(ix2=0; ix2<(128-ix); ix2++) {
              dimservername[ix2]= tolower(obj[ix+1+ix2]);
              if(dimservername[ix2] == '\0') break;
            };
            break;
          };
        };
        break;
      default:
        break;
    };
    optind++;
  };
};
if (argi!=1) {
  printf("Wrong number of parameters\nUse -h for help\n");
  exit(1);        	
};
strcpy(dimservernameCAP,dimservername); UPPER(dimservernameCAP);
strcpy(ltu_dimservername,"LTU_"); strcat(ltu_dimservername,dimservernameCAP);
infolog_SetFacility(ltu_dimservername);
infolog_SetStream(dimservernameCAP,-1);
printf("Stream:%s Facility:%s\n", dimservernameCAP, ltu_dimservername);
printenvironment();
/*---------------------------------------------------------------- DIM */
initStatic();   // necessary for FineDelay1/2 hw constant calculations
printf("Starting DIM services, popen2, assigning shared memory...\n");
rcso= ds_register(dimservername, BoardBaseAddress);  // has to be here (ltushm)
if(rcso != 0) {
  sprintf(msg, " Cannot register DIM services LTUname:%s LTUbase: %s\n", 
    dimservername, BoardBaseAddress);
  //strcpy(state,"ERROR"); smi_set_state(state);
  exit(1);
} else {
  char msg[200];
  sprintf(msg, "ltu_proxy and DIM services started for %s", dimservername);
};
infolog_trg(LOG_INFO, msg);
ltc= &ltushm->ltucfg;
templtucfg= &templtucfgmem; 
//copyltucfg(templtucfg, ltc);  moved down
/*---------------------------------------------------------------- SMI */
smi_attach(obj, SMI_handle_command);
printf("LTU attaching to %s\n",obj);
printf("%s --> Set Initial conditions (STANDALONE_STOPPED):\n",obj);
printf("BoardBaseAddress: %s\n", BoardBaseAddress);
printf("mode:    %d\t0:sw 1:pulser_level 2:random 3:BC 5:pulser_edge\n",   ltc->ltu_autostart_signal);
printf("BC rate: %f\n",   ltc->ltu_event_rate);
printf("L0:      %d\t0:over cable   8:over TTC\n",   ltc->L0);
printf("BUSY:    %d\t0:not used,  1:busy1 used\n",   ltc->busy);
printf("orbitbc: %d (0x%x)\n",   ltc->orbitbc,   ltc->orbitbc);
if (  ltc->ltu_sodeod_present) {
  printf("SOD/EOD events will be generated\n");
} else {
  printf("SOD/EOD events will NOT be generated\n");
}
fflush(stdout);
/* VME is opened:
 - in piped ltu.exe (all the time)
 - opened/closed with each SMI command
*/
if(vmeopen(BoardBaseAddress,BoardSpaceLength)) {
  printf(" Cannot open vme access for LTU %s\n", BoardBaseAddress);
  strcpy(state,"ERROR");
  smi_set_state(state);
} else {
  /* removed 7.11.2014 (done above in ds_register when ltu.exe started)
   but we need to have Gltuver defined... so let's leave it aslo here
  */
    ltuInit(&ltushm->ltucfg, 1,0);   // executes TTCinit() too
  //
  /* removed 29.10.2014 (on Manlio Minervini request)
  if(strcmp(dimservername,"hmpid")==0) {
    ltc->flags= ltc->flags | FLGfecmd12;
  }; */
  copyltucfg(templtucfg, ltc);
  ltu_configure(0);        /* configure internal variables (BC, etc)  + LTU*/
  vmeclose();
  strcpy(state,"STANDALONE_STOPPED");
  smi_set_state(state);
};
printf(" main loop /1sec\n"); fflush(stdout);
{ 
while(1) {
  if(isecs>=60) {
    updateMONCOUNTERSservice(0);    // update DIM service 1/min
    isecs= 0;
  };
  dtq_sleep(1); isecs++;   // dtq_sleep(60) before 24.9.2014
  //usleep(1000000); isecs++;  // nebavi ani toto
  prevbusyf= newbusyf;
  newbusyf= ltushm->busyfraction;
  //printf("ltuproxymain: %d prev/now:%6.4f %6.4f\n", isecs, prevbusyf, newbusyf); fflush(stdout);
  if( (fabs(newbusyf-prevbusyf)>0.01) || (notupdated>=120) ) {
    updateMONBUSY(newbusyf);   
    notupdated= 0;
  } else {
    notupdated++;
  };
  if(QUIT==1) {
    // freeShared(buf1,...);     -for SSM yes, but not for counters
    buf1=NULL;
    // see ds_stop shmdt(ltushm);
    break;
  };
};
};
return (0);
}
