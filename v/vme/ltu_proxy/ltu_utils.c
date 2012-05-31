#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "infolog.h"

#include "vmewrap.h"
#include "vmeblib.h"
#include "ltu.h"
//#include "ttcvi.h"
#include "ltu_utils.h"

int quit;

void getdatetime(char *);

/***************************/
/* configuration variables */
/***************************/
/* minimum sleep time after sequence start (microsec) */
int ltu_sleep_time=1000;

/* internal variables */
w32 memstart[LTUNCOUNTERS]; // used by readcounters()
w32 memend[LTUNCOUNTERS];      // used by readcounters()

#define LTUCLOCK 40000000   /* LHC clock rate in Hz */
w32 convertBC(float rate) {
w32 ltu_downscaling;
ltu_downscaling= rounddown(1. * LTUCLOCK/rate); 
return(ltu_downscaling);
}

/* read and print LTU counters:
prt: 1 -> print counters to stdout
     0 -> only read to global mem[] */
void readcounters(w32 *mem, int prt) {
  w32 l0,l2a,start,busy,busyc;
  int avdt; 
  //char datetime[20];
  char msg[200];
  /* read counters and print them to logfile: */
  readCounters(mem, LTUNCOUNTERS, 0);
  if(prt==0) return;
  l0= dodif32(memstart[L0_COUNTERrp], mem[L0_COUNTERrp]);
  l2a= dodif32(memstart[L2a_COUNTERrp], mem[L2a_COUNTERrp]);
  start= dodif32(memstart[START_COUNTERrp], mem[START_COUNTERrp]);
  busy= dodif32(memstart[BUSY_TIMERrp], mem[BUSY_TIMERrp]);
  busyc= dodif32(memstart[BUSY_COUNTERrp], mem[BUSY_COUNTERrp]);
  //getdatetime(datetime);
  avdt=0;
  if(l0>0)avdt= (int)(busy*4/10/l0);
  sprintf(msg, "LTU counters: L0:%d L2a:%d START:%d BUSYS:%d avrg. deadtime %d micsec",
    (int)l0, (int)l2a, (int)start, (int)busyc, avdt);	
  infolog_trg(LOG_INFO, msg);
}
/*------------------------------------------*/ int ltu_configure(int global) {
/* initialise LTU and TTC path:
- at the start of ltu_proxy
- before standalone run  (global: 0)
- before going to GLOBAL (global: 1)
templtucfg: pointer to values to be loaded to LTU
rc: 0: ok
    -1: TTCinitgs: vmxopen() error
    1: TTCinitgs: not ready after FEEreset
Note: mode (stdalone or global) is always set -even in case of rc!=0
      which is very likely incorrect (if LTU is busy after being put
      in global, anyhow global run will be discarded)
*/
w32 ltu_downscaling,emustat;
int ixpptime, rc=0;
char sgmode[12], msg[200];
emustat= vmer32(EMU_STATUS);
if(emustat != 0) {   
  int rcq;
  rcq= SLMquit();
  sprintf(msg, "LTU emulation was active.rc from SLMquit():0x%x", rcq);
  infolog_trg(LOG_ERROR, msg);
};
ERenadis(0);	      	/* always disable errors */
vmew32(L1_FORMAT, templtucfg->l1format);   /* L1message */
vmew32(BC_DELAY_ADD,   templtucfg->bc_delay_add); setTTCint(templtucfg->L0); 
vmew32(ORBIT_BC,   templtucfg->orbitbc);
if(global==0) {
  ixpptime= IXSpp_time;
} else {
  ixpptime= IXGpp_time;
};
vmew32(PP_TIME,   templtucfg->plist[ixpptime]);
vmew32(RATE_LIMIT,   templtucfg->plist[IXrate_limit]);

if(global==0) {
  if(getsgmode()==0) {
    w32 b2;
    sprintf(msg, "LTU in GLOBAL,setting to STANDALONE mode");
    infolog_trg(LOG_ERROR, msg);
    if(templtucfg->flags & FLGextorbit) { b2=3; } else { b2=1;};
    setstdalonemode(b2);
  };
  if(  templtucfg->ltu_autostart_signal==3) {          //BC downscaled
    ltu_downscaling= convertBC(  templtucfg->ltu_event_rate);
    vmew32(COUNT_PERIOD, ltu_downscaling);  /* set BC downscaling factor */
  } else if(  templtucfg->ltu_autostart_signal==2) {   // random
    ltu_downscaling= rounddown(0x7fffffff/40. * templtucfg->ltu_event_rate/1000000.);
    vmew32(RANDOM_NUMBER, ltu_downscaling);  /* set BC downscaling factor */
  };
  strcpy(sgmode,"STANDALONE");
} else {
  setglobalmode(); 
  strcpy(sgmode,"GLOBAL");
};
if(templtucfg->ttcrx_reset==0) {
  sprintf(msg, "TTCinit() suppressed (by option ttcrx_reset)");
  infolog_trg(LOG_INFO, msg);
} else {
  char datetime[20];

  /* TTCinit() was done BEFORE putting LTU into global in February2008!
     Do not forget TRD/SSD: they are by default
     busy, and they release busy in the time of TTCrx reset */
  getdatetime(datetime);
  printf("%s: starting TTCinit(1)\n", datetime);
  printltuDefaultsMem(templtucfg);
  rc= TTCinitgs(1-global, 5, templtucfg);
  if (rc != 0) {
    if(rc==1) {
      sprintf(msg, "TTCinit(): detector not ready in 5 seconds after FEEreset ");
    } else {
      sprintf(msg, "TTCinit() retcode:%d in %s mode", rc, sgmode);
    };
    infolog_trg(LOG_INFO, msg);
  };
  //printf("Sleeping 3 secs after TTCinit...\n"); usleep(3000000);
};
//clearCounters();   // removed (to avoid spikes in monitoring)
// following moved to startemu()
//copyltucfg(templtucfg, ltc);   // restore defaults (if overwritten by ECS, to be prepared)
readcounters(memstart, 0);     // for next run -they can be rewritten differently
return(rc);
}

/*--------------------------------------------------------- busystatus()
RC: 1: BUSY is ON       0: BUSY is OFF */
int busystatus() {
w32 status;
printf("busystatus:\n");
status=vmer32(BUSY_STATUS);
//printf("busystatus:%x\n",status);
if(status & BUSY_ACTIVE) return(1);
return(0);
}

/*----------------------------------------------*/ void busy12(int enable) {
/* used in CTP emulation mode to enable/disable busy
Input: enable:1 -enable configured busy(s)
       enable:0 -disable configured busy(s) */
  w32 busy1=0;   /* 1: busy1 should be always enabled 
                    0: busy1 disabled */
if(enable) {
    vmew32(BUSY_ENABLE, templtucfg->busy);   /* enable */
} else {
   vmew32(BUSY_ENABLE, busy1);   /* disable */
};
}

/* master/slave or standalone mode */
void Setstdalonemode() {
w32 b2; char msg[100];
//von infolog_SetStream(dimservernameCAP,-1);
SLMsetstart(templtucfg->ltu_LHCGAPVETO);          /* START not selected */ 
PARTITION_NAME[0]='\0';   // has to come with GLOBAL mode
readcounters(memend, 1);
if(templtucfg->flags & FLGextorbit) { b2=3; } else { b2=1;};
setstdalonemode(b2);
if(templtucfg->flags & FLGfecmd12) {
  int rc;
  // send FEEcmd 12 for hmpid
  sprintf(msg, "hmpid STDALONE: sending feecmd 12...");
  infolog_trg(LOG_INFO, msg);
  rc= ttcFEEcmd(12);
};  
}
int Setglobalmode() {
/* initialise ttcvi: */
int rc;
//von infolog_SetStream(PARTITION_NAME,-1);
rc= ltu_configure(1);
return(rc);
}

/*----------------------------------------------- */ int sodeod(char *seqfile) {
/* Input: seqfile: sod.seq or eod.seq
   rc:    1: SOD/EOD/SYNC was not generated   0: OK, SOD/EOD/SYNC was generated
*/
#define WAITBUSYOFF 5000000       // in micsecs (was 3000000 before 24.4.2012 )
#define WAITBUSY_STEP 200000  
w32 emustat, waiting=0;
int rc, killsodeod;
char datetime[20];
char vcfname[180];
  char msg[200];
/* ltu_proxy is started from WORKDIR, i.e. relative path to
   its .seq files is: CFG/ltu/SLMproxy */
getdatetime(datetime);
printf("%s:sodeod:%s loading...\n",datetime,seqfile);
strcpy(vcfname, "CFG/ltu/SLMproxy/"); strcat(vcfname, seqfile);
rc=SLMload(vcfname);
if(rc!=0) {
  sprintf(msg, "Can't load file %s SLMload rc:%d", vcfname, rc);
  infolog_trg(LOG_FATAL, msg);
  return(1);
};
SLMsetstart(templtucfg->ltu_LHCGAPVETO);          /* START not selected */ 
  //readcounters(0);
  //busytime= mem[BUSY_TIMERrp]; busys= mem[BUSY_COUNTERrp];
SLMstart(); 
SLMswstart(1,0); usleep(1000);          /* SW trigger */
while(1) {
  /* if BUSY is ON, STARTsignal derived from SWtrigger is killed.
   The waiting for 'SOD/EOD success should match the waiting
   in global mode -see ctp_proxy/ctp_proxy.c generateXOD().
   Let's check if SOD went through by checking the emulation status: */
  emustat= vmer32(EMU_STATUS);
  if(emustat == 0) {   
    killsodeod=0; break;  // SOD/EOD was generated
  };
  SLMswstart(1,0);          /* SW trigger */
  usleep(WAITBUSY_STEP);   //give some time for SOD/EOD generation
  waiting=waiting+WAITBUSY_STEP;
  if(waiting > WAITBUSYOFF) {
    killsodeod=1; break;
  };
};
getdatetime(datetime);
if(killsodeod == 1) {
  SLMquit();
  sprintf(msg, "%s killed. Waiting:%d micsecs", seqfile, waiting);
  infolog_trg(LOG_ERROR, msg);
  printf("%s:%s killed. Waiting:%d micsecs\n", datetime, seqfile, waiting);
} else {
  sprintf(msg, "%s generated. Waiting:%d micsecs", seqfile, waiting);
  infolog_trg(LOG_INFO, msg);
  printf("%s:%s generated. Waiting:%d micsecs\n", datetime, seqfile, waiting);
};
return(killsodeod);
}
/*-----------------------------------------------------*/ int startemu() {
int rc=0;     /* 0: OK,   1: SOD was killed */
//int ltu_singleevent_sleep;/*minimum time to be sure 1 event occurs in BC mode */
Tltucfg *ltc= &ltushm->ltucfg;
char vcfname[180];
char msg[200];
/*-------------------------------------- initialise LTU and TTC path: */
rc= ltu_configure(0);     /* recalculate values, if changed in shmltu */
if(rc!=0) goto RET;
usleep(1000);   // SDD get stuck in BUSY after 1st event without this line
/*-------------------------------------- Start of Data event */
if (templtucfg->ltu_sodeod_present) {
  rc=sodeod("sod.seq");
  //printf("waiting in startemu after sod: 1 secs\n"); usleep(1000000);
};
/*
ltu_singleevent_sleep=2*1000000/templtucfg->ltu_event_rate;
if (ltu_singleevent_sleep<ltu_sleep_time) ltu_singleevent_sleep=ltu_sleep_time;
usleep(ltu_singleevent_sleep);
printf("waiting in startemu after sod:%d micsecs. Why?\n",ltu_singleevent_sleep);
*/
usleep(ltu_sleep_time);
printf("waiting in startemu after sod:%d micsecs.\n",ltu_sleep_time);
fflush(stdout);
//  cfdir= getenv("VMEWORKDIR"); strcpy(vcfname, cfdir); 
/* VMEWORKDIR is working directory, take this sequence 
   from directory: CFG/ltu/SLMproxy */
strcpy(vcfname, "CFG/ltu/SLMproxy/"); 
strcat(vcfname, templtucfg->mainEmulationSequence);
rc= SLMload(vcfname);
if(rc!=0) {
  sprintf(msg, "startemu: %s: bad file. SLMload rc:%d", vcfname, rc);
  infolog_trg(LOG_FATAL, msg);
};
if(  templtucfg->ltu_autostart_signal==3) {    
  strcpy(msg,"BC downscaled, ");
  sprintf(msg, "%s rate:%f", msg, templtucfg->ltu_event_rate);
} else if(  templtucfg->ltu_autostart_signal==2) {
  strcpy(msg,"Random, ");
  sprintf(msg, "%s rate:%f", msg, templtucfg->ltu_event_rate);
} else if(  templtucfg->ltu_autostart_signal==5) {
  strcpy(msg,"Pulser_edge, ");
} else if(  templtucfg->ltu_autostart_signal==1) {
  strcpy(msg,"Pulser_level, ");
} else if(  templtucfg->ltu_autostart_signal==0) {
  strcpy(msg,"SW, ");
} else {
  strcpy(msg, "Unknown trigger source,");
};
infolog_trg(LOG_INFO, msg);
SLMsetstart(templtucfg->ltu_autostart_signal | templtucfg->ltu_LHCGAPVETO);
SLMstart();
RET:
copyltucfg(templtucfg, ltc);   // restore defaults (if overwritten by ECS, to be prepared)
return(rc);
}
/*---------------------------------------------*/ void stopemu(int sodeodecs) {
  SLMquit();
  usleep(ltu_sleep_time);
  eodemu(sodeodecs);
}
void pauseemu() {
  /* disable triggers */
  SLMquit();
  usleep(ltu_sleep_time);
  readcounters(memend,1);
}
void resumeemu() {
  /* Main sequence */
  SLMstart();
  fflush(stdout);
}
int syncemu() {
/* load sync.seq, start emulation, relaod back SLM, return rc: 
!!! check templtucfg
*/
int rc,rc1; char vcfname[180];
rc=sodeod("sync.seq");
if(rc!=0) {
  infolog_trg(LOG_FATAL, "SYNC not sent");
};
strcpy(vcfname, "CFG/ltu/SLMproxy/"); 
strcat(vcfname, templtucfg->mainEmulationSequence);
printf("loading original sequence+start signal:%s\n",vcfname);
rc1= SLMload(vcfname);
printf("SLMload rc:%d\n",rc1); fflush(stdout);
if(rc1!=0) {
  infolog_trg(LOG_FATAL, "LTU SLM not loaded with original sequence after SYNC");
} else {
  SLMsetstart(templtucfg->ltu_autostart_signal | templtucfg->ltu_LHCGAPVETO);
  printf("SLMsetstart ok.\n");
};
fflush(stdout);
return(rc);
}

/*--------------------------------------------- */ int eodemu(int sodeodecs) {
/* generate End of Data event. Called when stopping the run
(run can be 'running' or 'paused') 
RC: 0:OK   1: EOD was killed 
*/
int rc;
if (sodeodecs) {
  //w32 emustat, busys, busytime, avdt;
  //readcounters(0);
  //busytime= mem[BUSY_TIMERrp]; busys= mem[BUSY_COUNTERrp];
  rc=sodeod("eod.seq");
  //readcounters(0);
  //busys= dodif32(busys, mem[BUSY_COUNTERrp]);   // number of BUSYS
  //busytime= dodif32(busytime, mem[BUSY_TIMERrp]);
  //avdt= busytime/2.5/(busys+1);  // average deadtime in micsecs
};
readcounters(memend,1);
return(rc);
}
