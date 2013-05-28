/* clgroups.c: manage class groups to be active during separate time slots.
See  cfg2part.c clg_defaults[MAXCLASSGROUPS]= {0,1,1,3,4,5,6,7,8,9999}; */  
#include <stdio.h>
#include <string.h>

#ifdef CPLUSPLUS
#include <dic.hxx>
#else
#include <dic.h>
#endif
#include "vmewrap.h"
#include "infolog.h"
#include "vmeblib.h"
#include "vmeblib.h"
#include "ctp.h"
#include "Tpartition.h"
#include "ctp_proxy.h"

//ctplib.h:
int l0AB();

//#ifndef TEST

/*-------------------------------------------------------- nextclassgroup 
find next class group to become active.
rc: 0 -no class groups defined for this partition
   >0 -index pointing to part->classgroups[] array to the group which
       should become active
*/
int nextclassgroup(Tpartition *part) {
int ix;
if(part->nclassgroups == 0) return(0);
ix=part->active_cg+1; 
while(1) {
  if(ix>=MAXCLASSGROUPS) ix= 0;
  if(ix == part->active_cg) {
    char msg[200];
    sprintf(msg, "nextclassgroup not found in %s partition (groups:%d/%d)", 
      part->name, part->nclassgroups, ix);
    prtError(msg);
    return(0);
  };
  if(part->classgroups[ix] != 0) {
    if(part->classgroups[ix] != CG_NEVERACTIVE) return(ix);
  };
  ix++;
};
}
/*-------------------------------------------------------- enableclassgroup 
Operation:
for klas in 'partition classes':
  if klas is in any classgroup:
    if klas is in setclgroup:
      enable klas
    else
      disable klas
rc: #of enabled classes
*/
int enableclassgroup(Tpartition *part, int setclgroup) {
int iclass, rc=0, pcfgn=0; w32 mskCLAMASK;
char msg[900];
if(l0AB()==0) {   //firmAC
  mskCLAMASK=0x80000000;
} else {
  mskCLAMASK=0x10000;
};
part->active_cg= setclgroup; 
//ctpshmbase->active_cg= setclgroup; -will be done after counter reading in dims
for(iclass=0; iclass<NCLASS; iclass++) {
  //w32 l0inps;
  w32 mskbit;
  int hwclass, clgroup, bb; TKlas *klas;
  if((klas=part->klas[iclass]) == NULL) continue;
  pcfgn++;
  // 0: class always active:
  //mskbit= ((klas->l0vetos)&0x10000)>>16;
  mskbit= (klas->l0vetos)&mskCLAMASK; if(mskbit!=0) {mskbit=1;} else {mskbit=0;};
  clgroup=part->klas[iclass]->classgroup;
  if(clgroup == 0) {
    // this class is always active, check if class was enabled from .pcfg
    if(mskbit==0) rc++;
    continue;
  };
  hwclass= part->klas[iclass]->hwclass;  // 0..49
  //l0inps= part->klas[iclass]->l0inputs;
  bb=4*(hwclass+1);
  if(clgroup == setclgroup) {
    // activate   (i.e. use whatever was given in .pcfg file for this class)
    if(mskbit==0) rc++;
  } else {
    // deactivate (1: class is not active)
    mskbit= 1; 
  };
  vmew32(L0_MASK+bb, mskbit);
};
if(DBGCLGROUPS) {
  sprintf(msg, "%s: allclasses:%d activated:%d", part->name, pcfgn, rc);
  prtLog(msg);
};
return(rc);
}
/*-------------------------------------------------------- disableNEVERACTIVE()
rc: numbe of never active classes
*/
void disableNEVERACTIVE(Tpartition *part) {
int iclass,nacs=0;
char msg[1100], msg2[1100]="";
for(iclass=0; iclass<NCLASS; iclass++) {
  TKlas *klas; int clgroup;
  if((klas=part->klas[iclass]) == NULL) continue;
  clgroup=part->klas[iclass]->classgroup;
  if(part->classgroups[clgroup] == CG_NEVERACTIVE) {
    int hwclass,bb;
    hwclass= part->klas[iclass]->hwclass;  // 0..49
    bb=4*(hwclass+1);
    vmew32(L0_MASK+bb, 1);
    sprintf(msg2,"%s %d", msg2, hwclass+1);
    nacs++;
  };
};
sprintf(msg,"Partition:%s NEVERACTIVE group's hw classes:%s",part->name, msg2);
prtLog(msg);
}
/*-------------------------------------------------------- showTotals
*/
int totals=0;
void showTotals(Tpartition *part) {
char msg[1000]="";
int ix;
totals++;
if( (totals%20)!=0) return;
sprintf(msg,"cg(%%)      secs    mics   part:%s clgroups:%d\n", 
  part->name, part->nclassgroups);
for(ix=1; ix<MAXCLASSGROUPS; ix++) {
  if(part->classgroups[ix] != 0) {
    sprintf(msg,"%s%2d(%3d) %7d %7d\n", msg, ix, part->classgroups[ix],
      part->totalsecs[ix], part->totalmics[ix] );
  };
}; msg[strlen(msg)-1]='\0';   // remove last \n
/* printf("%s\n", msg); */ infolog_trg(LOG_INFO, msg);
}

/*-------------------------------------------------------- updateTotalTime
clgroup:
==0xfffffffe -do not force counters reading ! (was done already at EOR)
- update total time for active TS group
called from: cgInterrupt()
             stopTimer(, 0xfffffffe) -when EOR
*/
void updateTotalTime(Tpartition *part, w32 clgroup) {
w32 secs1, mics1;
GetMicSec(&secs1, &mics1);
//printf("uTT: %d %d - %d %d\n", secs1, mics1, part->lastsecs, part->lastmics);
SubSecUsec(&secs1, &mics1, part->lastsecs, part->lastmics);
//printf("uTTSub: %d %d\n", secs1, mics1);
AddSecUsec(&part->totalsecs[part->active_cg], 
  &part->totalmics[part->active_cg], secs1, mics1);
//printf("uTTAdd: %d %d\n", part->totalsecs[part->active_cg], part->totalmics[part->active_cg]); 
if(clgroup!=0xfffffffe) {
  xcountersStart(0, clgroup);    // force xcounters read 
};
showTotals(part);
}

/*-------------------------------------------------------- cgInterrupt
*/
void cgInterrupt(void *tagv) {
//void cgInterrupt(int tag) {
Tpartition *part;
int clgroup, oldclgroup, tag= (int)tagv; //tag=(int)tag;;
part= AllPartitions[tag];
if(DBGCLGROUPS) {
  printf("cgInterrupt, tag:%d part:%s\n", tag, part->name);
};
oldclgroup= part->active_cg;
clgroup= nextclassgroup(part);
if(clgroup > 0) {
  setPartDAQBusy(part); // disable triggers
  // following has to be here (see DOC/devdbg/TimeSharing)
  updateTotalTime(part, clgroup);   // +force counters reading
  enableclassgroup(part, clgroup);  // in HW only
  startTimer(part, 0, 0xfffffffe);
  unsetPartDAQBusy(part); // enable triggers
};
}
 
/*-------------------------------------------------------- startTimer 
Input:cginterval: 0 -use interval given by part->active_cg
                 >0 -use cginterval (in seconds)
- store current time for this group 
- start timer with TAG pointing to part->positionInAllPartitions
called from:
- cgInterrupt()          ...(,,0xfffffffe)
- ctp_StartPartition     ...(,,0xfffffffe)
- ctp_ResumePartition    ...(,,0,part->active_cg)

*/
void  startTimer(Tpartition *part, int cginterval, w32 clgroup) {
w32 secs,mics;
int tag;
GetMicSec(&secs, &mics);
part->lastsecs= secs; part->lastmics= mics;
tag= part->positionInAllPartitions;
if(cginterval<0) {
  char msg[200];
  sprintf(msg, "startTimer: bad cginterval:%d for %s partition (%d)", 
    cginterval, part->name, part->positionInAllPartitions);
  prtError(msg);
  cginterval=0;
};
if(cginterval==0) {
  cginterval= part->classgroups[part->active_cg];
};
if(DBGCLGROUPS) {
  printf("startTimer: %d secs cl.group:%d for part:%s (%d)\n",
    cginterval, part->active_cg, part->name, tag);
};
if(clgroup!=0xfffffffe) {
  xcountersStart(0, clgroup);    // force xcounters read 
};
dtq_start_timer(cginterval, cgInterrupt, (void *)tag);
//dtq_start_timer(cginterval, cgInterrupt, tag);
}

/*-------------------------------------------------------- stopTimer 
clgroup:
==0xffffffff -do not tocuh shm
>=0        set to clgroup after reading
- force the timer to stop
- update total time (involves counter reading)
- print totals
called from:
- ctp_proxy: EOR     clgroup:0xfffffffe
- ctp_proxy: pause partition clgroup:0
*/
int  stopTimer(Tpartition *part, w32 clgroup) {
int remsecs,waserr=0;
remsecs= dtq_stop_timer((void *)part->positionInAllPartitions);
if(remsecs==-1) {
  if(part->cshmpart->paused==0) {
    char msg[200];
    sprintf(msg, "stopTimer: timer was not started for %s partition (%d)", 
      part->name, part->positionInAllPartitions);
    intError(msg);
    waserr=1;
  };
};
if(waserr==0) {
  updateTotalTime(part, clgroup);
};
return(remsecs);
}

