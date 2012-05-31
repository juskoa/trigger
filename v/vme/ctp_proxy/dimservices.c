/* dimservices.c
18.02.2007
Forced counters reading added (CTPDIM/GETCOUNTERS)
10.7. 2007 ltu voltages added
*/
#include <stdio.h>
#include <stdlib.h>   //atoi
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#ifdef CPLUSPLUS
#include <dis.hxx>
#else
#include <dis.h>
#endif
#include "vmewrap.h"
#include "lexan.h"
#include "udplib.h"
#include "vmeblib.h"
#include "ctp.h"
#include "ctplib.h"
#include "dimtypes.h"
#include "Tpartition.h"

//CTP_RUNXCOUNTERS:
#define NPARTIT 6
#define RUNXCOUNTERSSTART (CSTART_BUSY+NCOUNTERS_BUSY_RUNX1)
#define TAGstartcount 333
#define TAGstopcount 334
#define TAGprintruns 335
#define MAXNLUMCNTS 50   // max. # of fixed counters for DCS
int PartRunNum[NPARTIT];

#ifdef DEVELOP
#define MYNAME "CTPDIMt"
#else
#define MYNAME "CTPDIM"
#endif

#define TAGinterpret 18
#define TAGretstatus 19
#define TAGswtrgcmd 20
int TAGcthread=21;
#define TAGMONCOUNTERS 22
#define TAGMONBST 23
int TAGbstthread=24;

#define MAXFNL 44
#define MAXCIDAT 80
#define MAXCMDL 200
#define EMSGL 200
#define MAXLILE 1500
#define MAXMCclients 25
#define MAXSWTRGREQS 2
#define NBSTdcsmsg 11    // in 4-byte words

unsigned int RESULTid;
unsigned int MONBSTid;
unsigned int COUNTERSid;
unsigned int LUMCNTSid;
unsigned int BEAMMODEid;
int LUMSIM=0;   // 1: LUMSIM active
int cid;         /* active client id. 0: nobody active */
char cidat[80];  /* active client: pid@host */
#define RCL 16
char ReceivedCommand[RCL]="ok";   /* SWTRG,SWTRGerror, STATUS, or DO (ok only 1st time)*/
int secs=-1;

int QUIT=0;
int udpsock;
/*problem: float(-1.0) gives after (float)(w32)float_var 4294967295.000000 */
/*         float(-1.0 -1.0E0 ) gives after (float)(w32)float_const 0.000000 */
float minus1= -1.0; //100000000.0;

/*no1min: 
- do not update per minute (cthread not started)
- individual update request is sent back to all subscribers
  (i.e. 1 minute update is controlled by external process instead 
   of cthread) */
#define NO1MINFLAG 0x1
w32 dimsflags=0;
//w32 dimsflags=NO1MINFLAG;

// Info about clients using MONCOUNTERS service is kept in NMCclients table:
// see updateNMCclients()
typedef struct mcc{
  int cid;
  char cidat[MAXCIDAT];
  w32 Nupdated;
} TMCclient;

typedef struct monc{
  int NMCclients;       // number of active clients
  TMCclient MCclients[MAXMCclients];
} Tmonclient;

Tmonclient MONCOUNTERS;
Tmonclient MONBST;
Tmonclient MONLUMCNTS;
//-------------------------------------------------
w32 psecs=0,pmics=0;
w32 *CTPCNTS;
w32 *LUMCNTS; w32 *prevLUMCNTS; float *LUMCNTSrates;
w32 *buf1;
int vspbobr=-1;
w32 beammode=0xffffffff;
w32 bstmsg[NBSTdcsmsg];
//-----------------------------------end of MONCOUNTERS info data

// SW trigger requests: active requestes are kept in actrs table:
Tswtrgreq actrs[MAXSWTRGREQS];
//-----------------------------------end of SWTRG info data
int relpositions[MAXNLUMCNTS];
int NLUMCNTS=0; 
int LUMrunn=0;   // 0: no PHYSICS_1 present

char ResultString[MAXLILE+1]="Not initiated";
char foreignmsg[]="Error: service XXX used by other client\n";

void ds_stop();

#define fstrALLOC 1
#define fstrFIND  2
/*-------------------------------------------------------- findSwTrgReq
IN: detname: detector name
    mode: 
    fstrALLOC   -find free item and allocate it
    rc: -1 -not enough space
        -2 -detname already in
    fstrFIND    -find detname
    rc: -1 -detname not found
ret: rc >= 0 -> index to actrs pointing to found/allocated item
        -3   -> unknown mode (internal error)
        <0   -> error (see above)
*/
int findSwTrgReq(char *detname, int mode) {
int ix, freeix=-1, rc=-1;
for(ix=0; ix<MAXSWTRGREQS; ix++) {
  if(actrs[ix].cidat[0]=='\0') {
    if(freeix==-1) freeix=ix;   // first free ix
    continue;
  };
  if(mode==fstrALLOC) {
    if(strcmp(detname, actrs[ix].name)==0) {   // detname in
      rc=-2;   // already in
      goto RTRN;
    };
  } else if(mode==fstrFIND) {
    if(strcmp(detname, actrs[ix].name)==0) {   // detname in
      rc=ix;
      goto RTRN;
    };
  } else {
    rc=-3;
    goto RTRN;
  };
};
// detname/cidat not found, allocate:
if(freeix>=0) {
  strcpy(actrs[freeix].name, detname);
  rc= freeix;
};
RTRN:
if(DBGfindSwTrgReq) printf("findSwTrgReq: rc:%d\n",rc);
return(rc);
};
/*-------------------------------------------------------- findSwTrgReqcid
find by looking for client process id
IN: cid
    fromix 0..   from where to look for
rc: -1 -cidat not found
rc>=0  - cidat found at rc
*/
int findSwTrgReqcid(char *cidname, int fromix) {
int ix, rc=-1;
for(ix=fromix; ix<MAXSWTRGREQS; ix++) {
  if(strcmp(cidname, actrs[ix].cidat)==0) {   // cidat found
    rc=ix;
    goto RTRN;
  };
};
RTRN: return(rc);
}
/*-------------------------------------------------------- printactrs
find by looking for client process id
IN: cid
    fromix 0..   from where to look for
rc: -1 -cidat not found
rc>=0  - cidat found at rc
*/
void printactrs() {
int ix;
    printf("ix  name        type N     Ngen    roc   cid   cidat\n");
for(ix=0; ix<MAXSWTRGREQS; ix++) {
  if(actrs[ix].cidat[0]!='\0') {
    printf("%2d:%12s        %c   %5d   %5d     0x%x  %d    %s   \n",
      ix,
      actrs[ix].name,
      actrs[ix].type,
      actrs[ix].N,
      actrs[ix].Ngenerated,
      actrs[ix].roc,
      actrs[ix].cid,
      actrs[ix].cidat);
  };
};
}
/*---------------------------------------------------------- updateservice
This routine is called by:
SWTRGcmd
STATUScmd
DOcmd -updateservice is called only in case of 
       'DO W N' or DO CHECKPHASES
*/ 
int updateservice(int clientid) {
int cids[2]; int nclients;
/*printf("updateservice cid:%d :%s\n",cid,ResultString); */
cids[0]= clientid;
cids[1]= 0;   // end of the list
/* The folloving call activates RESULTcaba routine, which finds
out from variable "ReceivedCommand", which service was called */
nclients=dis_selective_update_service(RESULTid, cids);
ResultString[0]='\0';
return(nclients);
}

/*-------------------------------------------------------- SWTRGthread
operation:
- generate trigger (for tagix request)
- update SwTrgReq structure
- dis_selective_update_service (if client registered to this service)
- if not all requested triggers generated:
    - goto 'generate trigger'
- unallocate SwTrgReq structure
- stop (thread: not started as thread, so only for short requests -
  if lng request, another request won't be satisfied, but queued only)
*/
void SWTRGthread(int tagix) {
w32 detectors;
int succeeded, todo, ncls;
//printf("SWTRGthread:%d\n",tagix);
detectors= 1 << findLTUdetnum(actrs[tagix].name);
/*
while(actrs[tagix].Ngenerated < actrs[tagix].N) {
  // gen. swtrg
  actrs[tagix].Ngenerated++;
  updateservice(actrs[tagix].cid);
  sleep(5);
};
*/
// still to be added (in GenSwtrg): pf,bcmask as symb. names
todo= actrs[tagix].N;
succeeded=GenSwtrg(todo, actrs[tagix].type, 
  actrs[tagix].roc, actrs[tagix].bc,detectors,2);
actrs[tagix].Ngenerated= succeeded;
ncls= updateservice(actrs[tagix].cid);

//printf("SWTRGthread ix:%d for %s finished:\n",tagix,actrs[tagix].name);
actrs[tagix].cidat[0]='\0';   // release actrs entry
}
/*--------------------------------------------*/ float ratehz(int ix){
float retval;
if(ix<2) {
  retval= LUMCNTS[ix];
} else if(ix==2) {
  retval= dodif32(prevLUMCNTS[2], LUMCNTS[2])*0.4;
} else {
  if(prevLUMCNTS[ix]!=0) {   // at least 1 reading before
    float rate; float deltaus;
    deltaus= dodif32(prevLUMCNTS[2], LUMCNTS[2])*0.4;
    /*printf("deltacnts:%d deltaus:%f\n",
      dodif32(prevLUMCNTS[ix], LUMCNTS[ix]),deltaus); */
    rate= 1000000.0*dodif32(prevLUMCNTS[ix], LUMCNTS[ix])/deltaus;
    //retval= (w32)(int)rate;
    retval= rate;
  } else {
    //retval=(w32)(int)(minus1);
    retval= minus1;
  };
};
return(retval);
}
/*--------------------------------------------------------------- */
void prtNupdates(Tmonclient *mc, char *outmsg) {
int ix, cid;
char cidname[MAXCIDAT];
cid=dis_get_client(cidname); outmsg[0]='\0';
for(ix=0; ix<MAXMCclients; ix++) {
  if(mc->MCclients[ix].cidat[0]=='\0') continue; // this one is free
  //printf("%3d: %d %s %d", ix, mc->MCclients[ix].cid, mc->MCclients[ix].cidat,
  sprintf(outmsg, "%s %d:%d", outmsg, ix, mc->MCclients[ix].Nupdated); 
};
}
/*---------------------------------- void prtLUMCNTS() {
int realsize,i; char msg[900];
msg[0]='\0'; realsize=3;
  for(i=0; i<NLUMCNTS; i++) {   
    if(i<3) {
      sprintf(msg,"%s %d:%d", msg, i, LUMCNTS[i]);
    } else {
      sprintf(msg,"%s %d:%f", msg, i, (float)(int)LUMCNTS[i]);
      if((float)(int)LUMCNTS[i]>=0.0) { realsize=i+1; };
    };
  }; sprintf(msg,"%s realsize:%d\n",msg,realsize); prtLog(msg);
}*/
int oldnclients=0;
int nlog=0;
int nlogmoncount=0;
//von#define spare_epochsecs 763
//von#define spare_epochmics 764
//von#define spare_l2orbit 765
/*-------------------------------------------------------- readctpcounters()
clientid: 0: update all clients subscribing to MONCOUNTERS+MONLUMCNTS
        !=0: update only clientid MONCOUNTERS client (forced counters read
             supported ONLY for MONCOUNTERS).
clgroup: ==0xffffffff do not touch shm
         !=        update shm AFTER counters read (at SOR time)
*/
void readctpcounters(int clientid, w32 clgroup) {
int ix,nclients,nclientslum,firstreading,samereading; 
w32 *ctpc;
w32 secs, mics, difmics, l2orbit;
char msg[ERRMSGL];
if((dimsflags & NO1MINFLAG) != 0) {
  if(clientid!=0) {
    clientid=0;
  };
};
ctpc=buf1;
GetMicSec(&secs, &mics);
l2orbit= vmer32(L2_ORBIT_READ);
readCounters(ctpc, NCOUNTERS, 0, 1); readTVCounters(&ctpc[CSTART_SPEC+3]);
/* printf("readTVcounters:\n");
for(ix=CSTART_SPEC+3; ix<(CSTART_SPEC+3+16);ix++) {
  printf("%d:0x%x ", ix, ctpc[ix]);
};printf("\n"); */
ctpc[CSTART_SPEC]= secs; //ctpc[spare_epochsecs]= secs;
ctpc[CSTART_SPEC+1]= mics; //ctpc[spare_epochmics]= mics;
ctpc[CSTART_SPEC+2]= l2orbit; //ctpc[spare_l2orbit]= l2orbit; 
for(ix=RUNXCOUNTERSSTART; ix<(RUNXCOUNTERSSTART+NPARTIT);ix++) {
  ctpc[ix]=PartRunNum[ix-RUNXCOUNTERSSTART];
};
ctpc[CSTART_TSGROUP]= ctpshmbase->active_cg;
if( clgroup !=0xffffffff) {
  ctpshmbase->active_cg= clgroup;   // AFTER reading+update !
};
/*printf("secs mics l2orbit:%d %d %d\n", secs,  mics, l2orbit); */

/* regardless of MONLUMCNTS service active(PHYSICS_1 running) or not,
always do first 3:
*/
for(ix=0; ix<3; ix++) {
  int relpos;
  float fsecs;
  relpos= relpositions[ix];
  prevLUMCNTS[ix]= LUMCNTS[ix];
  LUMCNTS[ix]= ctpc[relpos];
  //LUMCNTSrates[ix]= 1.0*(LUMCNTS[ix]-1000000000);
  //LUMCNTSrates[ix]= fsecs;
  if(ix==0) {
    fsecs= LUMCNTS[ix]-1285000000;
    LUMCNTSrates[ix]= fsecs;
    //printf("secs %d float:%f\n", LUMCNTS[ix], fsecs);
  } else if(ix==1) {
    fsecs= LUMCNTS[ix];
    LUMCNTSrates[ix]= fsecs;
    //printf("mics %d float:%f\n", LUMCNTS[ix], fsecs);
  } else {
    LUMCNTSrates[ix]= ratehz(ix);
  };
};
//if((prevLUMCNTS[0]==0) || (LUMrunn==0)) {firstreading=1;} else {firstreading=0;};
if(prevLUMCNTS[0]==0) {firstreading=1;} else {firstreading=0;};
if(LUMrunn>0) {   // MONLUMCNTS service active (PHYSICS_1 running)
// prepare fixedcnts array for DCS:
  if(prevLUMCNTS[2]==LUMCNTS[2]) {samereading=1;} else {samereading=0;};
  if(samereading==0) {
    for(ix=3; ix<NLUMCNTS; ix++) {
      int relpos;
      relpos= relpositions[ix];
      //sprintf(msg,"relpossitions[%d] is %d\n", ix, relpos); prtLog(msg);
      if(relpos==-1) {   // first 3 always defined (never  -1.0)
        //LUMCNTS[ix]= (w32)(int)(minus1);
        LUMCNTSrates[ix]= minus1;
      } else {
        LUMCNTS[ix]= ctpc[relpos];
        LUMCNTSrates[ix]= ratehz(ix);
        prevLUMCNTS[ix]= ctpc[relpos];
      };
    };
  };
  //prtLUMCNTS();
} else {
  prevLUMCNTS[0]=0;
};
//here put message "ctpdims alive' for automatic monitoring
difmics= DiffSecUsec(secs,mics,psecs,pmics);
psecs=secs; pmics=mics; nclientslum=0;
if(clientid==0) {
  char msg[200];
  nclients= dis_update_service(COUNTERSid);
  if(LUMrunn>0) {   // MONLUMCNTS service active (PHYSICS_1 running)
    if((firstreading==0) && (samereading==0)) {
      nclientslum= dis_update_service(LUMCNTSid);
    };
  };
  /*sprintf(msg,"updating counters.firstreading:%d LUMrunn:%d clients:%d %d\n",
    firstreading, LUMrunn, nclients, nclientslum);
  prtLog(msg);*/
  nlog++;
  if((nlog % 60)==0) {
    sprintf(msg,"readctpcounters(1/hour): difmics:%d nclients:%d lum:%d\n",
      difmics, nclients, nclientslum);
    prtLog(msg);
    prtNupdates(&MONCOUNTERS, msg);
    sprintf(msg, "MONCOUNTERS Nupdates:%s", msg);
    prtLog(msg); 
  };
  /*printf("readctpcounters: difmics:%d nclients:%d elapsed L0,L1: %x %x\n", 
    difmics, nclients, ctpc[13], ctpc[CSTART_L1+5]);
  printf("readctpcounters spec secs, mics:%d %d\n", 
    ctpc[CSTART_SPEC], ctpc[CSTART_SPEC+1]); */
  if(oldnclients != nclients) {   // # of clients changed
    int ix;
    sprintf(msg, "clients now: %d", nclients); 
    for(ix=0; ix<MONCOUNTERS.NMCclients; ix++) {
      sprintf(msg,"readctpcounters: %3d: %s\n",MONCOUNTERS.MCclients[ix].cid, MONCOUNTERS.MCclients[ix].cidat);
    }; prtLog(msg);
    oldnclients= nclients;
  };
} else {   // no support for GETMONLUMCNTS
  int cids[2];
  cids[0]= clientid; cids[1]= 0;   // end of the list
  nclients= dis_selective_update_service(COUNTERSid, cids);
  /*nclients:0 if this clinet has not subsribed to MONCOUNTERS service */ 
  sprintf(msg,"Forced update for client:%d nclients:%d\n", clientid, nclients);
  prtLog(msg);
};
}
/*-------------------------------------------------------- bstthread
running as thread (started once, with dim server)
*/
#define BSTINTSECS 10
int seqn=0, nlogbst=0; w32 newbeammode;
void bstthread(void *tag) {
w32 l2orbit;
while(1) {   //run forever
  int ix, nclients;
  if(vspbobr == -1) {
    //simulate:
    for(ix=0; ix<NBSTdcsmsg; ix++) {bstmsg[ix]= seqn;}; seqn++;
    l2orbit= vmer32(L2_ORBIT_READ);
    if(bstmsg[iBeamMode]>21) bstmsg[iBeamMode]= 0;
  } else {
    //real bst msg:
    l2orbit= vmer32(L2_ORBIT_READ);
    nextBSTdcs(vspbobr, bstmsg, 1);    // 1:BST1/mem 2:BST2/mem 3:BST1/auxmem
  };
  nclients= dis_update_service(MONBSTid);
  newbeammode= bstmsg[iBeamMode];
  if(beammode != newbeammode) {
    w32 oldbeammode;
    //char msg[80];
    /* update beammode: */
    oldbeammode= beammode;
    beammode= newbeammode;
    //nclients=dis_selective_update_service(RESULTid, cids);
    nclients= dis_update_service(BEAMMODEid);
    //sprintf(msg,"bstthread:BEAMMODE %d old:%d nclients:%d\n",
    //  beammode, oldbeammode, nclients); prtLog(msg);
  };
  nlogbst++;
  //if((nlogbst % (60/2/BSTINTSECS))==0) {    // monitor every half minute
  if((nlogbst % (600/BSTINTSECS))==0) {    // monitor every 10 mins
    int rc;
    char buffer[50];
    sprintf(buffer, "mon ds003:ds004:ds050 N:%d:%d:%d", 
      bstmsg[iBeamMode], bstmsg[iTurnCount], bstmsg[iTurnCount]);
    //following udpsend crashes dims server (sometimes):
    rc= udpsend(udpsock, (unsigned char *)buffer, strlen(buffer)+1);
    prtLog(buffer);
  };
  //if((nlogbst % (60/BSTINTSECS))==0) {    // 1/min
  if((nlogbst % (20*60/BSTINTSECS))==0) {    // 1/20 mins
    char msg[200]; 
    sprintf(msg,"bst013: %x %x %x %x\n", bstmsg[iGPSusecs], bstmsg[iGPSsecs],
      bstmsg[iTurnCount], l2orbit);
    prtLog(msg);
  };
  if((nlogbst % (3600/BSTINTSECS))==0) {   // log msg 1/hour
    char msg[80];
    sprintf(msg,"bstthread(1/hour): nclients:%d\n",nclients);
    prtLog(msg);
  };
  dtq_sleep(BSTINTSECS);
  if(QUIT==1) break;
};
}
/*-------------------------------------------------------- cthread
running as thread (started once, with dim server)
*/
void cthread(void *tag) {
while(1) {   //run forever
  w32 bst2ctp, bst3124; int rc;
  char buffer[500];
  readctpcounters(0,0xffffffff);
  // check orbitsync (bobr vs. l2orbit):
  if(vspbobr != -1) {
    getlhc2ctpOrbit(vspbobr, &bst2ctp, &bst3124);
    //bst2ctp++; bst3124= bst3124+2;
  } else {  //simulate 
    bst2ctp= bst2ctp+5; bst3124=((bst3124>>24)+1)<<24;
  };
  sprintf(buffer, "mon ds001:ds002 N:%d:%d", bst2ctp, bst3124>>24);
  rc= udpsend(udpsock, (unsigned char *)buffer, strlen(buffer)+1);
  //prtLog(buffer);
  dtq_sleep(60);
  //dtq_sleep(1);  // debugging monscal
  if(QUIT==1) break;
};
}
/*-------------------------------------------------------------- checkcid
return:
<=0 -error
>0 -current cid, which is OK, (was before or just has been set)
*/
int checkcid() {
int loccid;
char loccidat[80];
loccid= dis_get_client(loccidat);
//printf(" cid:%d cidat:%s new_cid/cidat:%d/%s\n",cid, cidat, loccid,loccidat);
  // don't check it for now...
  cid=loccid; strcpy(cidat,loccidat); return(cid);

if(cid==0) {
  cid=loccid;
  strcpy(cidat,loccidat);
} else if(loccid != cid) {
  printf("checkcid error: requested by:%d <> opened by:%d\n",loccid, cid);
  return(-1);
} else {
  cid=loccid;
  strcpy(cidat,loccidat);
};
//printf("           cid:%d, cidat:%s\n",cid, cidat);
return(cid);
}
/*----------------------------------------*/ void resetClient(Tmonclient *mc) {
int ix;
for(ix=0; ix<MAXMCclients; ix++) {   // no MONCOUNTERS clients registered
  mc->MCclients[ix].cidat[0]='\0';
  mc->MCclients[ix].Nupdated=0;
};
mc->NMCclients=0;
}
/*----------------------------------------------- updateNMCclients
Purpose: keep track of all active (MONCOUNTERS/MONBST/MONLUMCNTS subsribers) clients.
*/
void updateNMCclients(Tmonclient *mc) {
int ix, ixfree=-1;;
int cid;
char cidname[MAXCIDAT];
cid=dis_get_client(cidname);
for(ix=0; ix<MAXMCclients; ix++) {
  if(mc->MCclients[ix].cidat[0]=='\0') {
    if(ixfree==-1) ixfree=ix;    // this one is free
    continue;
  };
  if((mc->MCclients[ix].cid == cid) && 
     (strncmp(mc->MCclients[ix].cidat, cidname,MAXCIDAT)==0)
    ) {
    mc->MCclients[ix].Nupdated= mc->MCclients[ix].Nupdated + 1;
    goto FOUND;   //after updating Nupdated just for this one client
  };
};
if(ixfree == -1) {
  char msg[100];
  sprintf(msg,"MCclients not large enough:now %d nclients, maximum.\n", mc->NMCclients);
  prtLog(msg);
} else {                // new client
  char msg[100];
  mc->MCclients[ixfree].cid= cid;
  strncpy(mc->MCclients[ixfree].cidat, cidname,MAXCIDAT);
  mc->MCclients[ixfree].Nupdated= 0;
  sprintf(msg,"updateNMCclients:%d %d %s new", ixfree,
    mc->MCclients[ixfree].cid, mc->MCclients[ixfree].cidat);
  if(mc==&MONCOUNTERS) {
    dis_set_client_exit_handler (cid, ixfree);
    sprintf(msg,"%sCOUNTERS",msg);
  } else if(mc==&MONBST) {
    dis_set_client_exit_handler (cid, ixfree+MAXMCclients);
    sprintf(msg,"%sBST",msg);
  } else if(mc==&MONLUMCNTS) {
    dis_set_client_exit_handler (cid, ixfree+2*MAXMCclients);
    sprintf(msg,"%sLUMCNTS",msg);
  } else {
    sprintf(msg,"%s internal error",msg);
  };
  sprintf(msg,"%s.", msg);
  prtLog(msg);
  mc->NMCclients++;
};
FOUND: return;
}
/*----------------------------------------------------------- BEAMMODEcaba
*/
void BEAMMODEcaba(void *tag, void **msgpv, int *size, int *blabla) {
unsigned int **msgp= (unsigned int **)msgpv;
char msg[100];
sprintf(msg, "BEAMMODEcaba size:%d NBST:%d bm:%d", *size, NBSTdcsmsg, beammode );
prtLog(msg); 
//updateNMCclients(&......); we do not keep track of these clients
*msgp= &beammode;
*size= 4*(sizeof(beammode));
//printf("BEAMMODEcaba size:%d beammode:%d \n", *size, beammode);
}
/*----------------------------------------------------------- MONBSTcaba
*/
void MONBSTcaba(void *tag, void **msgpv, int *size, int *blabla) {
unsigned int **msgp= (unsigned int **)msgpv;
char msg[100];
sprintf(msg, "MONBSTcaba size:%d NBST:%d\n", *size, NBSTdcsmsg );
prtLog(msg); 
updateNMCclients(&MONBST);
*msgp= (unsigned int *)bstmsg;
*size= 4*(NBSTdcsmsg);
printf("MONBSTcaba size:%d NBSTdcsmsg:%d \n", *size, NBSTdcsmsg);
}
/*----------------------------------------------------------- MONCOUNTERScaba
*/
void MONCOUNTERScaba(void *tag, void **msgpv, int *size, int *blabla) {
unsigned int **msgp= (unsigned int **)msgpv;
updateNMCclients(&MONCOUNTERS);
*msgp= (unsigned int *)CTPCNTS;
*size= 4*(NCOUNTERS);
nlogmoncount++;
/* moved to readctpcounters once /hour 
if((nlogmoncount%60)==0) {
  char msg[100]; char outmsg[200];
  prtNupdates(&MONCOUNTERS, outmsg);
  sprintf(msg, "MONCOUNTERScaba (1/hour) size:%d Nupdates:%s", *size, outmsg);
  prtLog(msg); 
}; */
//printf("MONCOUNTERScaba size:%d NCOUNTERS:%d \n", *size, NCOUNTERS);
}
/*----------------------------------------------------------- LUMCNTScaba
*/
void LUMCNTScaba(void *tag, void **msgpv, int *size, int *blabla) {
float **msgp= (float **)msgpv;
int realsize;char msg[900];
//von nlogmoncount++;
if((nlogmoncount%60)==0) {
  sprintf(msg, "LUMCNTScaba (1/hour) size:%d NLUMCNTS:%d\n", 
  *size, NLUMCNTS);
  prtLog(msg); 
};
if(NLUMCNTS>0) { realsize=3; } else { realsize=0; };
msg[0]='\0';
// prepare fixed_cnts (sent only with PHYSICS_1!)
/*for(i=0; i<MAXNLUMCNTS; i++) {   
  if(i<3) {
    ; //sprintf(msg,"%s %d:%d", msg, i, LUMCNTS[i]);
  } else {
    //sprintf(msg,"%s %d:%f", msg, i, (float)(int)LUMCNTS[i]);
    //if((float)(int)LUMCNTS[i]>=0.0) { realsize=i+1; };
    if(LUMCNTSrates[i]>=0.0) { realsize=i+1; };
  };
}; //sprintf(msg,"%s\n",msg); prtLog(msg);
*/ realsize= NLUMCNTS;
updateNMCclients(&MONLUMCNTS);
*msgp= LUMCNTSrates;
*size= 4*realsize;   //4*(NLUMCNTS);
//printf("LUMCNTScaba size:%d NLUMCNTS:%d \n", *size, NLUMCNTS);
}
/*--------------------------------------------------------------- RESULTcaba
*/
void RESULTcaba(void *tag, void **msgpv, int *size, int *blabla) {
int ix;
char **msgp= (char **)msgpv;
if(DBGRESULTcaba)
  printf("RESULTcaba: cid:%d, cidat:%s\n",cid, cidat);
*(int *)tag=4567;
if(checkcid()<0) {
  *msgp= foreignmsg;
  *size= strlen(foreignmsg)+1;
  return;
}; 
if((strcmp(ReceivedCommand,"ok")==0)) {
  strcpy(ResultString,"server ok.");
} else if((strcmp(ReceivedCommand,"SWTRG")==0)) {
  ix= findSwTrgReqcid(cidat, 0);
  /* update this part later (if more complicated scenarios,including
     the case when the overlaping of more SWTRG requests allowed...)
  */
  if(ix>=0) {   // 1 client can request swtrg for more different detectors !
    int fromix; //    find all of them:
    fromix=ix;
    strcpy(ResultString, "SWTRG "); //sprintf(ResultString,"cidat:%s ",cidat);
    while(fromix != -1) { 
      sprintf(ResultString,"%s%s:%d ", ResultString,
      actrs[ix].name, actrs[ix].Ngenerated);
      fromix++;
      if(fromix>=MAXSWTRGREQS) break;
      fromix= findSwTrgReqcid(cidat, fromix);
    };
  } else {
    //strcpy(ResultString,"ERROR: internal when processing SWTRG command");
    strcpy(ResultString,"no requests from this client");
  };
} else if(stringStarts(ReceivedCommand,"STATUS ALLRARE")) {
  w32 arf; char allraretxt[8];
  arf= vmer32(ALL_RARE_FLAG);
  if((arf&0x1) == 1) {
    strcpy(allraretxt,"ALL");
  } else {
    strcpy(allraretxt,"RARE");
  };
  sprintf(ResultString, "STATUS:%s\n",allraretxt);
} else if(stringStarts(ReceivedCommand,"STATUS CLIENTS")) {
  sprintf(ResultString, "MONCOUNTERS:%d MONBST:%d\n",
    MONCOUNTERS.NMCclients, MONBST.NMCclients);
} else if(stringStarts(ReceivedCommand,"STATUS LHCSHIFT")) {
  /*Tbstmsg bstmsg;
  rc= getLHCshift(&bstmsg);*/
  sprintf(ResultString,"%d %d %d %d \n", 1,2,3,4);
} else if((strncmp(ReceivedCommand,"STATUS LTUS",11)==0)) {
  /* Create output message:
  detname1 FB   
  detname2 FB
  ...
  where F -> 'F' if connected to Fan out (' ' not connected)
        B -> 'B' if connected to busy (' ' if not connected)
  */
  strcpy(ResultString, "STATUS:\n");
  for(ix=0; ix<NDETEC ;ix++) {
    char f,b;
    char line[80];
    if(validLTUs[ix].name[0]=='\0') continue;
    if(validLTUs[ix].fo!=0) f='F' ;else f=' ';
    if(validLTUs[ix].busyinp!=0) b='B' ;else b=' ';
    sprintf(line, "%8s %c%c\n", validLTUs[ix].name, f, b);
    //printf("line:%s",line);
    strcat(ResultString, line);
  };
  
} else if((strcmp(ReceivedCommand,"SWTRGerror")!=0) &&
          (strcmp(ReceivedCommand,"DO CHECKPHASES")!=0) &&
          (strcmp(ReceivedCommand,"DO TOGGLE")!=0)) {
  sprintf(ResultString,"ERROR: unknown command:%s cidat:%s MONCOUNTERS clients:%d",
    ReceivedCommand, cidat,MONCOUNTERS.NMCclients);
};
*msgp= ResultString;
*size= strlen(ResultString)+1;   // "" -empty string is 1 byte message
if(DBGRESULTcaba) {
  printf("RESULTcaba:result:%d:%s\n",*size,ResultString); //usleep(1000000);
};
strcpy(ReceivedCommand,"ok");   // or none?
}

/*--------------------*/ //void STATUScmd(int *tag, char *msg, int *size)  {  
/*--------------------*/ void STATUScmd(void *tag, void *msgv, int *size)  {  
int nclients; char *msg= (char *)msgv;
if(stringStarts(msg,"LTUS")) {
  strcpy(ReceivedCommand, "STATUS LTUS");
} else if(stringStarts(msg,"ALLRARE")) {
  strcpy(ReceivedCommand, "STATUS ALLRARE");
} else if(stringStarts(msg,"CLIENTS")) {
  strcpy(ReceivedCommand, "STATUS CLIENTS");
} else if(stringStarts(msg,"LHCSHIFT")) {
  strcpy(ReceivedCommand, "STATUS LHCSHIFT");
} else {
  strncpy(ReceivedCommand, msg,RCL); ReceivedCommand[RCL-1]='\0';
};
if(DBGCMDS)
printf("STATUScmd: tag:%d size:%d msg:%s<-endofmsg\n", *(int *)tag, *size,msg);
msg[*size-2]='\0';
ResultString[0]='\0';
if(checkcid()<0) return;
nclients= updateservice(cid);
/* nclients= dis_update_service(RESULTid); */
printf("num. of updated clients:%d cid:%d cidat:%s\n", nclients, cid, cidat);
}
/*--------------------*/ //void SWTRGcmd(int *tag, Tswtrg *msg, int *size)  {
/*--------------------*/ void SWTRGcmd(void *tag, void *msgv, int *size)  {
/* msg: detectorname[8] roc[4] N[4] bc[4]*/
int ix, nclients; Tswtrg *msg= (Tswtrg *)msgv;
Tdetector *ixp;
strcpy(ReceivedCommand, "SWTRG");
ResultString[0]='\0';
checkcid();
if(DBGCMDS)
  printf("SWTRGcmd: cid:%d cidat:%s size:%d roc:0x%x N:%d name:%s type:%c\n", 
  cid, cidat, *size, msg->roc, msg->N, msg->name, msg->type);
ixp= findLTU(msg->name);
if(ixp==NULL) {
  sprintf(ResultString,"ERROR: Unknown detector %s", msg->name);
} else if(ixp->fo==0) {
  sprintf(ResultString,"ERROR: Detector %s not connected", msg->name);
} else {
  // here add check if detector in active partition (i.e. GLOBAL state)
  // Allowing sw triggers for detectors in stdalone has no sense
  // (LTU is not receiving signals from CTP) 
  ix= findSwTrgReq(msg->name, fstrALLOC);
};
if(ResultString[0]!='\0') goto RETERR;
if(ix==-2) {  /* check the message: */
  sprintf(ResultString,"ERROR: Detector %s already being sent SW triggers", msg->name);
} else if(ix==-1) {
  sprintf(ResultString,"ERROR: Not enough space in actrs[] dimservices.c");
} else if(ix==-1) {   // dim server started from ctp_proxy
} else if(msg->N >20) { 
  strcpy(ResultString,"ERROR: N>20");
};
if(ResultString[0]!='\0') goto RETERR;
strcpy(actrs[ix].cidat, cidat);   // allocate this item
actrs[ix].cid= cid;
actrs[ix].N= msg->N;
actrs[ix].Ngenerated= -1;     // nothing generated yet
actrs[ix].bc= 8;   /* ignored for 'a', 8 for 'c' */
strcpy(actrs[ix].bcmask, msg->bcmask);
actrs[ix].roc= msg->roc;
strcpy(actrs[ix].pf, msg->pf);
actrs[ix].type= msg->type;
/* execute this request: */
//dim_start_thread(SWTRGthread, ix);   -bad idea
SWTRGthread(ix); 
return;
RETERR:
strcpy(ReceivedCommand, "SWTRGerror");
nclients= updateservice(cid); return;
}
/*--------------------*/ //void DOcmd(int *tag, char *msg, int *size)  {
/*--------------------*/ void DOcmd(void *tag, void *msgv, int *size)  {
/* msg: if string finished by "\n\0" remove \n */
char *msg= (char *)msgv;
int nclients;
strcpy(ReceivedCommand, "DO");
if(DBGCMDS)
printf("DOcmd1: tag:%d size:%d msg:%s<-endofmsg\n", *(int *)tag, *size,msg);
if(msg[*size-2]=='\n') {
  msg[*size-2]='\0';
} else {
  msg[*size-1]='\0';
};
//strcpy(ResultString,">"); strcat(ResultString,msg); strcat(ResultString,"<");
//if(strcmp(msg,"sleep2")==0) sleep(2);
if(checkcid()<0) {
  printf("bad cid\n");
  return;
};
if(strcmp(msg,"qq")==0) ds_stop();
if(stringStarts(msg,"ALL")) {
  vmew32(ALL_RARE_FLAG, 1);
  printf("ALLRARE flag set to ALL\n");
} else if(stringStarts(msg,"RARE")) {
  vmew32(ALL_RARE_FLAG, 0);
  printf("ALLRARE flag set to RARE\n");
} else if(stringStarts(msg,"CHECKPHASES")) {
  strcpy(ReceivedCommand, "DO CHECKPHASES");
  // here we should check if allowed...
  checkPhases(ResultString);
  printf("CHECKPHASES: %s\n",ResultString);
  nclients=updateservice(cid);
} else if(stringStarts(msg,"TOGGLE")) {
  // TOGGLE 1 detector   or TOGGLE 0 detector (e.g. TOGGLE 1 hmpid)
  // 0....,...9
  int rc,onoff; 
  char det[20];
  strcpy(ReceivedCommand, "DO TOGGLE");
  // here we should check if allowed...
  if(strlen(&msg[9])>16) {
    rc=1;
  } else {
    enum Ttokentype tt; int ixmsg=0;
    tt= nxtoken1(&msg[9], det, &ixmsg);
    //printf("TOGGLEdbg2 det:%s< tt:%d\n", det, tt);
    if(tt != tSYMNAME) {rc=1; goto UPDT; };
    if(msg[7]=='0') {
      onoff=0;
    } else if(msg[7]=='1') {
      onoff=1;
    } else { rc=1; goto UPDT; };
    rc= Toggle(det, onoff);
  };
  UPDT:
  sprintf(ResultString,"%d", rc);
  //printf("TOGGLE %d %s: >%s<\n",onoff,det,ResultString);
  nclients=updateservice(cid);
  printf("TOGGLE updated %d clients.>%s<\n",nclients,ResultString);
} else if(msg[0]=='W') {
  int wsecs;
  wsecs=atoi(&msg[2]); printf("wsecs:%d\n", wsecs);
  for(secs=0; secs<wsecs; secs++) {
    sleep(1);
  };
  // just wait (no response through RESULT service, i.e. no updateservice())
  //nclients=updateservice(cid);
  //nclients= dis_update_service(RESULTid);
  //printf("updated clients:%d\n", nclients);
} else if(stringStarts(msg,"actrs")) {
  printactrs();
} else if(stringStarts(msg,"LUMSIM ON")) {
  LUMSIM=1; NLUMCNTS=21;
} else if(stringStarts(msg,"LUMSIM OFF")) {
  LUMSIM=0; NLUMCNTS=0;
} else {
  printf("ERROR: unknown DO messsage:%s\n",msg);
};
}

/*--------------------*/ //void getcnts(int *tag, char *msg, int *size)  {  
/*--------------------*/ void getcnts(void *tag, void *msgv, int *size)  {  
/* Client forced counters reading. */
if(checkcid()<0) return;
if(DBGCMDS) {
 printf("getcnts:tag:%d size:%d cid:%d cidat:%s\n", 
   *(int *)tag, *size, cid, cidat);
};
readctpcounters(cid,0xffffffff);
}
/*--------------------------------------------------------- startruncounter */ 
//void startruncounter(int *tag, char *msg, int *size)  {  
void startruncounter(void *tag, void *msgv, int *size)  {  
/* Forced counters reading at the start of run.
*/
int i,id, run; w32 clgroup; int *runcg= (int *)msgv;
char string[256],proces[100];
// Check if tag is correct - this is done in dim example on the web
// although it works withou that
if(*(int *)tag != TAGstartcount){
  sprintf(string,"Wrong tag=%d for command STARTRUNCOUNTER",*(int *)tag);
  prtLog(string);
  return;
}
// Check if sizeof(char) ==1
if(sizeof(char) != 1){
  prtLog("sizeof(char) != 1 !!! => Run number ERROR !!!"); return;
}
run= runcg[0]; clgroup= runcg[1]; 
id=dis_get_client (proces); // Get client id
sprintf(string,"Proces %s, id=%d requested command STARTRUNCOUNTER run:%d clg:%d",proces,id, run, clgroup);
prtLog(string);
if(*size>8) {
  char msg[1900];
  msg[0]='\0';
  // prepare fixed_cnts (sent only with PHYSICS_1!)
  for(i=2; i<MAXNLUMCNTS; i++) {   
    if(runcg[i]==-2) { NLUMCNTS= i-2; LUMrunn= run; break;
    } else { relpositions[i-2]= runcg[i]; };
    sprintf(msg,"%s %d:%d", msg, i, runcg[i]);
  }; sprintf(msg,"%s",msg); prtLog(msg);
};
sprintf(string,"startrun: *tag=%i,run=%i,0x%x, clg:0x%x size=%i NLUMCNTS:%d", 
  *(int *)tag,run,run,clgroup,*size, NLUMCNTS);
prtLog(string);
 //// Add runnumber to the list
 // First check for error: run already exists
 if(run != 0){
  int ix;
  for(ix=0;ix<NPARTIT;ix++){
   if(PartRunNum[ix] == run){
     char msg[200];
     sprintf(msg,"Run %d already exists !!!",run);
     prtLog(msg);
     return;
   }
  }
 }else {
   goto READ;
 };
 for(i=0;i<NPARTIT;i++){
  if(PartRunNum[i] == 0){
    PartRunNum[i]=run;
    break;
  }
 }
 if(i == NPARTIT){
   prtLog("NO free partition."); return;
 }
 READ:
 readctpcounters(0,clgroup);  // update ALL clients at the SOR
}
/* ---------------------------------------------------------- stopruncounter */
//void stopruncounter(int *tag, char *msg, int *size){
void stopruncounter(void *tag, void *msgv, int *size){
 unsigned int ch; char *msg= (char *)msgv;
 int i,id, run;
 char string[256],proces[100];
 // First read counters at the end of run
 //readCounters(); //run number is not updated
 // Run Number has to be decoded from msg
 run=0;
 for(i=0;i<*size;i++){
   ch=msg[i]&0xff;
   run=run+(ch<<(8*i));
   //printf("%i,0x%x,\n",i,ch);
 }
 //printf("stoprun: *tag=%i,run=%i,0x%x,size=%i \n", *(int *)tag,run,run,*size);
// just reset (all 0) PartRunNum array when ctp_proxy restarted:
if(run==0) {
 int ix; char msginfo[100];
 for(ix=0;ix<NPARTIT;ix++)PartRunNum[ix]=0;
 sprintf(msginfo,"Partition list cleared");
 prtLog(msginfo); return;
};
 for(i=0;i<NPARTIT;i++){
  if(PartRunNum[i] == run){
   PartRunNum[i]=0;
   break;
  }
 }
 // Error: run number not in list
 if(i == NPARTIT){
   char msginfo[200];
   sprintf(msginfo,"NO RUN NUMBER 0x%x in partition list !!!",run);
   prtLog(msginfo); return;
 }
 //usleep();
 readctpcounters(0,0xffffffff);   // update ALL clients at the EOR
if(run==LUMrunn) {
  LUMrunn=0;  // deactivate MONLUMCNTS service
  NLUMCNTS=0;
};
  // Check if tag is correct - this is done in dim example on the web
 // although it works withou that
 if(*(int *)tag != TAGstopcount){
  sprintf(string,"Wrong tag=%d for command STOPRUNCOUNTER",*(int *)tag);
  prtLog(string);
  return;
 }
 // Get client id
 id=dis_get_client (proces);
 sprintf(string,"Proces %s, id=%d requested command STOPRUNCOUNTER",proces,id);
 prtLog(string);
}
/*------------------------------------------------------------printruns */ 
//void printruns(int *tag, char *msg, int *size){
void printruns(void *tag, void *msgv, int *size){
 int i; //char *msg= (char *)msgv;
 char string[200],num[10];
 // Check if tag is correct - this is done in dim example on the web
 // although it works withou that
sprintf(string,"printruns: *tag:%d msg:%s size:%d\n", 
  *(int *)tag,(char *)msgv,*size); prtLog(string);
 if(*(int *)tag != TAGprintruns){
  sprintf(string,"Wrong tag=%d for command PRINTRUNS",*(int *)tag);
  prtLog(string);
  return;
 }
 strcpy(string," List of active runs: ");
 for(i=0;i<NPARTIT;i++){
  sprintf(num,"%i ",PartRunNum[i]);
  strcat(string,num);
 }
 prtLog(string);
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
prtLog(msg1); prtLog(message);
}
/*--------------------------------------------------------------- exit_handler
*/
void exit_handler(int *exitcode) {
char msg1[100];
sprintf(msg1,"exit_handler exitcode:%d", *exitcode);
prtLog(msg1);
}
/*---------------------------------------------------- client_exit_handler
MONCOUNTERS: *tag 0..MAXMCclients
MONBST     : *tag MAXMCclients..2*MAXMCclients-1
*/
void client_exit_handler(int *tag) {
int utag; Tmonclient *mc;
char msg1[100];
if(*tag < MAXMCclients) {mc= &MONCOUNTERS; utag=*tag;
} else if(*tag < 2*MAXMCclients) { mc= &MONBST; utag= *tag-MAXMCclients;
} else if(*tag < 3*MAXMCclients) { mc= &MONLUMCNTS; utag= *tag-2*MAXMCclients;
} else { 
  sprintf(msg1,"client_exit_handler error (max:%d):tag:%d",MAXMCclients, *tag); 
  prtLog(msg1);
  return;
};
if((utag<MAXMCclients) && (mc->MCclients[utag].cidat[0]!='\0')) {
  sprintf(msg1,"client_exit_handler deleting:%d:%s", 
    *tag,mc->MCclients[utag].cidat);
  mc->MCclients[utag].cidat[0]='\0';
  mc->NMCclients--;
} else {
  sprintf(msg1,"client_exit_handler error (max:%d):%d",MAXMCclients, *tag); 
}
prtLog(msg1);
}

/*--------------------------------------------------------------- ds_register
*/
void ds_register() {
int ix;
char command[MAXCMDL];
cid=0;   // nobody active now
buf1= (w32 *)malloc(NCOUNTERS*sizeof(w32)); CTPCNTS=buf1;
LUMCNTS= (w32 *)malloc((MAXNLUMCNTS+1)*sizeof(w32));
LUMCNTSrates= (float *)malloc((MAXNLUMCNTS+1)*sizeof(float));
prevLUMCNTS= (w32 *)malloc((MAXNLUMCNTS+1)*sizeof(w32));
for(ix=0; ix<MAXNLUMCNTS+1; ix++) {
  LUMCNTS[ix]=0;   // nothing defined for fixedcnts
  //if(ix>=3) LUMCNTS[ix]=(w32)(int)(minus1);   // nothing defined for fixedcnts
  if(ix>=3) LUMCNTSrates[ix]= minus1;   // nothing defined for fixedcnts
  prevLUMCNTS[ix]=0;
};
for(ix=0; ix<MAXSWTRGREQS; ix++) {   // no sw trg. requests registered
  actrs[ix].name[0]='\0';
};
if(envcmp("VMESITE", "ALICE")==0) {
  vspbobr= bobrOpen();
  udpsock= udpopens("alidcscom026", send2PORT);
} else {
  udpsock= udpopens("pcalicebhm05", send2PORT);
};
printf("vspbobr:%d udpsock:%d\n", vspbobr, udpsock);

resetClient(&MONCOUNTERS); resetClient(&MONBST);
/* If already some partitions are running
   existing run numbers  should be stored in PartRunNUm.
   0 cannot be run number;
   To be done: PartRunNum should be initialised from Trigger database
   (CFG/ctp/paractives)
*/
for(ix=0;ix<NPARTIT;ix++)PartRunNum[ix]=0;

printf("DIM server:%s\n",MYNAME);
dis_add_error_handler(error_handler);
dis_add_exit_handler(exit_handler);
dis_add_client_exit_handler (client_exit_handler);
printf("Commands:\n");
strcpy(command, MYNAME); strcat(command, "/DO");
dis_add_cmnd(command,"C", DOcmd, TAGinterpret);  printf("%s\n", command);
strcpy(command, MYNAME); strcat(command, "/STATUS");
dis_add_cmnd(command,"C", STATUScmd, TAGretstatus);  printf("%s\n", command);
strcpy(command, MYNAME); strcat(command, "/SWTRG");
dis_add_cmnd(command,NULL, SWTRGcmd, TAGswtrgcmd);  printf("%s\n", command);
strcpy(command, MYNAME); strcat(command, "/GETCOUNTERS");
dis_add_cmnd(command,NULL, getcnts, TAGcthread);  printf("%s\n", command);

/* RUNXCOUNTERS commands:
*/
printf("\n RUNXCOUNTER commands:\n");
strcpy(command, MYNAME); strcat(command, "/STARTRUNCOUNTER");
dis_add_cmnd(command,NULL, startruncounter, TAGstartcount);  
printf("%s\n", command);
strcpy(command, MYNAME); strcat(command, "/STOPRUNCOUNTER");
dis_add_cmnd(command,NULL, stopruncounter, TAGstopcount);  
printf("%s\n", command);
strcpy(command, MYNAME); strcat(command, "/PRINTRUNS");
dis_add_cmnd(command,NULL, printruns, TAGprintruns);  
printf("%s\n", command);

printf("\nServices:\n");
strcpy(command, MYNAME); strcat(command, "/RESULT");
RESULTid=dis_add_service(command,0, ResultString, MAXLILE+1, 
  RESULTcaba, 4567);  printf("%s:%d\n", command, RESULTid);
strcpy(command, MYNAME); strcat(command, "/MONCOUNTERS");
COUNTERSid=dis_add_service(command,0, CTPCNTS, 4*(NCOUNTERS),
  MONCOUNTERScaba, 4567);  printf("%s:%d\n", command, COUNTERSid);
strcpy(command, MYNAME); strcat(command, "/MONLUMCNTS");
/* see MAXNLUMCNTS 50
I:50, L:50 -> Did crashes
*/
//LUMCNTSid=dis_add_service(command,"L:3;F:30", LUMCNTS, 4*(MAXNLUMCNTS),
LUMCNTSid=dis_add_service(command,0, LUMCNTSrates, 4*(MAXNLUMCNTS),
  LUMCNTScaba, 4569);  printf("%s:%d\n", command,LUMCNTSid);
strcpy(command, MYNAME); strcat(command, "/MONBST");
MONBSTid=dis_add_service(command,0, bstmsg, 4*(NBSTdcsmsg),
  MONBSTcaba, 4567);  printf("%s:%d\n", command,MONBSTid);
strcpy(command, MYNAME); strcat(command, "/BEAMMODE");
BEAMMODEid=dis_add_service(command,0, &beammode, 4*(sizeof(w32)),
  BEAMMODEcaba, 4567);  printf("%s:%d\n", command, BEAMMODEid);

printf("serving...\n");
dis_start_serving(MYNAME);  
#ifdef DEVELOP
printf("Counters reading thread not started (DEVELOPment version)...\n");
#else
if((dimsflags & NO1MINFLAG)==0) {
  printf("Starting the counters reading thread...\n");
  dim_start_thread(cthread, (void *)&TAGcthread);
} else {
  printf("The counters reading thread not started (no1min option).\n");
};
printf("Starting the BST messages reading thread...\n");
dim_start_thread(bstthread, (void *)&TAGbstthread);
#endif
}
/*-------------------------------------------------------------- ds_update
*/
void ds_update() {
/*int nclients;
nclients= updateservice();
nclients= dis_update_service(RESULTid);
printf("ds_update: updated clients:%d\n", nclients);
*/
printf("ds_update: no update now here (see thread cthread) \n");
}
/*-------------------------------------------------------------- ds_stop
*/
void ds_stop() {
#define EXITSERVER 1
if(EXITSERVER==1) {
  QUIT=1;   // stop thread reading ctp counters and bstthread
  printf("qq...\n");
  dis_remove_service(RESULTid);
  dis_remove_service(COUNTERSid);
  dis_remove_service(LUMCNTSid);
  dis_remove_service(MONBSTid);
  dis_remove_service(BEAMMODEid);
  dis_stop_serving();
  free(buf1); CTPCNTS=NULL;
  free(LUMCNTS); free(prevLUMCNTS); LUMCNTS=NULL; free(LUMCNTSrates);
  vmeclose();
  if(vspbobr!=-1) bobrClose(vspbobr);
  exit(0);
};
}

