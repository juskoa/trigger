/* SSM routines used in v/vme/ctp package (i.e. all the
ctp boards accessed through 1 vme space. 4 ltus can
be accessed through vme spaces opened for them 
24.10. bug fixed in setomSSM (0x10 -> 0x100
       setSSM not used more
11.11. setomSSM(): opmo&3 changed to: opmo&7
       stopSSM() -now actually stops SSM (if it was busy)
07.08.06 readSSMDump added
*/

#include <stdio.h>
#include <stdlib.h>    /* malloc */
#include <unistd.h>    /* usleep */
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include "vmewrap.h"
#include "ctp.h"
#include "ctplib.h"
#define SSMCTP
#include "ssmctp.h"

void initSSM() {
int ix;
FILE *con;
char line[MAXLINE];

// Memory,offset,syncronisation flag
for(ix=0; ix<NSSMBOARDS; ix++) {
  sms[ix].sm= NULL;                
  sms[ix].mode[0]='\0';        
  sms[ix].syncmode=0; 
  sms[ix].offset= 0;
  sms[ix].syncflag= 0;
  sms[ix].signal=NULL;
  sms[ix].ltubase[0]='\0';   // NOT ltu board (i.e. not in the crate or CTP)
  //for(jx=0;jx<32;jx++)BoardChannels[ix][jx].start=NULL;  
};
// find LTUs in the crate:
if((con=fopen(BICfile,"r")) == NULL){
  printf("Cannot read %s file. LTUs in the crate not considered. \n", BICfile);
} else {;
  int nsms;
  nsms=NCTPBOARDS; 
  while(fgets(line,MAXLINE,con)!=NULL) {
    /* printf("readBICfiledbg: line:%s\n",line); */
    if(line[0]=='\n') continue;
    if(strncmp(line,"ltu",3)==0) {
      if(nsms>=NSSMBOARDS) {
        printf("readBICfile(): too many LTUs for global sms array\n");
        break;
      };
      strncpy(sms[nsms].ltubase, &line[4], 8); 
      sms[nsms].ltubase[8]='\0'; nsms++;
    };
  }; fclose(con);
};
// Assign board names
for(ix=0; ix<NCTPBOARDS; ix++) {
 strcpy(sms[ix].name,ctpboards[ix].name);	
};
strcpy(sms[NCTPBOARDS].name,"ltu1");	
strcpy(sms[NCTPBOARDS+1].name,"ltu2");
strcpy(sms[NCTPBOARDS+2].name,"ltu3");
strcpy(sms[NCTPBOARDS+3].name,"ltu4");
strcpy(sms[NCTPBOARDS+4].name,"test");
strcpy(sms[NCTPBOARDS+5].name,"none");
printf("initSSM: The number of boards is %i : \n",NSSMBOARDS);
for(ix=0; ix<NSSMBOARDS; ix++) printf(" %s",sms[ix].name);printf("\n");
printf("LTU boards in the crate:\n");
for(ix=NCTPBOARDS; ix<NSSMBOARDS; ix++) {
  if(sms[ix].ltubase[0]!='\0') {
    printf("%s: %s\n", sms[ix].name, sms[ix].ltubase);
  };
};
}

/*--------------------------- routines for LTU */
int Openvsp(char *bba) {
int vsp,rc;
vsp=-1; rc= vmxopen(&vsp, bba,"0x300");
/*printf("Openvsp: vmxopen rc:%d\n", rc); */
if(rc==0) rc=vsp;
else rc=-1;
return(rc);
}
void Closevsp(int vsp) {
int rc;
rc=vmxclose(vsp);
/*printf("Closevsp: vmxclose(%d) rc:%d\n",vsp,rc); */
}
w32 vmbr32(int vsp, w32 reladr) {
/* vsp: 0: -> ctp board
       >0: -> ltu board
   reladr: vme address relative to the LTUbase or CTP_BOARDbase
*/
if(vsp!=0) {   /* ltu */
  return(vmxr32(vsp,reladr));
}else{         /* ctp */
  return(vmer32(reladr));
};
}
void vmbw32(int vsp, w32 reladr, w32 value) {
/* vsp: 0: -> ctp board
       >0: -> ltu board
   reladr: vme address relative to the LTUbase or CTPFO1base
*/
if(vsp!=0) {   /* ltu */
  vmxw32(vsp, reladr, value);
}else{         /* ctp */
  vmew32(reladr, value);
};
}

/*------------------------------------------------------- setomvspSSM()
called only form setomSSM(), readSSM(), writeSSM() */
int setomvspSSM(int vsp, w32 boardoffset, w32 opmo) {
w32 status,bcstatus,busybit,ssmen;
if(vsp==0) busybit=0x100;     /* ctp board */
else busybit=0x04;           /* ltu board */
bcstatus= vmbr32(vsp, BC_STATUS+boardoffset);
if(((0x7&opmo)!=SSMomvmer && (0x7&opmo)!=SSMomvmew)&&((bcstatus&0x3)!=0x2)) {
  printf("Warn: BC signal not connected\n");/* BC not necessary for vme R/W*/
  //return(1); by rl 15/12/07 1.39
};
/*status=vmer32(SSMstatus+BSP*ctpboards[board].dial); */
status=vmbr32(vsp, boardoffset+SSMstatus);
if( status & busybit) {
  printf("SSM busy, stopping recording before setting new/op. mode...\n");
  /*  vmew32(SSMstop+BSP*ctpboards[board].dial,DUMMYVAL); */
  vmbw32(vsp,SSMstop+boardoffset,DUMMYVAL);
  /*  status=vmer32(SSMstatus+BSP*ctpboards[board].dial); */
  status=vmbr32(vsp,SSMstatus+boardoffset);
  if( status & busybit) {
    printf("ERROR: Cannot stop recording, operation or mode wasn't set!\n");
    return(1);
  };
};
/*vmew32(SSMcommand+BSP*ctpboards[board].dial, opmo); */
vmbw32(vsp,SSMcommand+boardoffset, opmo&0x3f);

if(vsp==0) {   /* ctp board, set SSMenable word: */
  ssmen= (opmo>>8)&3;
  vmew32(SSMenable+boardoffset, ssmen);
};
return(0);
}

/*FGROUP DebugSSMcalls
-------------------------------------------------------------- getswSSM() 
return status word of SSM
LTU: [4] FrontPanel->SSM mode active
     [3] not used
     [2] BUSY bit
     [1..0] operation
CTP: [8] -BUSY
     [7..6] Enable SSM Input..Output flag
     [5..4] ConfSel bits
     [3..3] InOut flag   0:out   1:in
     [2..1] Operation bits 
     [0..0] mode bit 
error: 0xdeadbeaf
*/
w32 getswSSM(int board) {
int vsp,rc;
if(sms[board].ltubase[0]=='\0') {   /* ctp board */
  w32 boardoffset=BSP*ctpboards[board].dial;
  rc= vmbr32(0, boardoffset+SSMstatus);
}else{                              /* ltu board */
  vsp=Openvsp(sms[board].ltubase);
  if(vsp!=-1) {
    rc= vmbr32(vsp, SSMstatus);
  } else {
    rc= 0xdeadbeaf;
  };
  Closevsp(vsp);
}
return(rc);
}

/*FGROUP DebugSSMcalls
-------------------------------------------------------------- setsmssw() 
set sms[].mode
*/
void setsmssw(int ix, char *newmode) {
/*printf("setsmssw:%d %s:\n",ix,newmode); */
strcpy(sms[ix].mode, newmode);
}

/* -------------------------------------------------------- setomSSM() */
/*FGROUP DebugSSMcalls
Set operation & mode. If SSM is BUSY, an attempt is made to stop
the recording.
Input parameteres:
board -number of the board (index into global sms array) 
       0..NSSMBOARDS    (busy,L0,1,2,int,fo1,2,3,4,5,6,ltu1,2,3,4)
opmo  -mode/operation bit for SSMcommand word. symbolic names 
       are defined in ctp.h (as SSMom*)

opmo for LTU boards:
--------------
0x0 -VME access, read
0x1 -VME access, write
0x2 -RECORDING, After  (cca 26 milsec)
0x3 -RECORDING, Before (should be stopped by SSMstoprec)

Bit opmo[4] (0x10) should be set to 1 for LTU/RECORDING mode
    (new feature 'FrontPanel->SSM' introduced for LTU 7.10.2005)
    If not set, signals from LTU-FPGA will be recorded.

opmo for CTP boards:
--------------
The codes above (0-3) are valid, in addition 7 bits (opmo[9..3] are
meaningfull, and 2 more codes are added:
0x4 - GENERATING, single pass
0x5 - GENERATING, continuous
opmo[9..8] - bits to be used for selecting SSMenable word
             10 ->enable Input    01 ->enable Output
opmo[7..6] - not used
opmo[5..4] - ConfSel bits  (defined in 
               $VMECFDIR/CFG/ctp/ssmsigs/.sig files)
opmo[3]      - InOut flag   1:in   0:out (side of FPGA logic)
Examples: 
0x20d - generate continuously  inputs for board logic
0x20c - generate 1 pass (27ms) inputs for board logic
0x102 - record   1 pass (27ms) of board logic outputs
RC:  0->ok, mode set
     1->mode not set, possible errors (printed to stdout):
        -BC signal not connected
        -Cannot stop recording operation
     2->bad mode for LTU board
*/
int setomSSM(int board, w32 opmo) {
int vsp,rc;
if(sms[board].ltubase[0]=='\0') {   /* ctp board */
  w32 boardoffset=BSP*ctpboards[board].dial;
  rc= setomvspSSM(0, boardoffset, opmo);
}else{                              /* ltu board */
  if( (opmo&7) >3) {
    printf("ERROR: setomSSM: %x >3 for LTU board\n",(int)opmo);
    return(2);
  };
  if( (opmo&0x10) != 0x10) {
    printf("WARNING: setomSSM: %x bit 0x10 not set for LTU board\n",(int)opmo);
  };
  vsp=Openvsp(sms[board].ltubase);
  if(vsp!=-1) {
    rc= setomvspSSM(vsp, 0, opmo);
  } else {
    rc= vsp;
  };
  Closevsp(vsp);
}
return(rc);
}

/*------------------------------------------------------ startSSM()
Start recording or generating for more boards (ctp or ltu boards)
n  -number of items in boards
Operation:
- Boards are started in given order from boards[1]
  1st board (boards[0]) is started as the last one
- minimal checking is made for errors
- if time between starting 1st board and last board is
  > 80 micsecs, rc>0 is returned
RC: 0    ->OK
    1, 2 -> LTU cannot be started
    10   -> time between 1st and last start > 80micsecs
*/
int startSSM(int n, int *boards) {
int ix, board,ixltus,nltus,vsp,rc=0;
w32 boardoffset, seconds1,micseconds1, seconds2,micseconds2,diff;
w32 status;
int vsps[5];
/* find ltus and open VME space for them: */
nltus=0; ix=1;
while(1) {
  if(ix>=n) ix=0;
  board= boards[ix];
  if(board>=NCTPBOARDS) {   /* ltu */
    printf("startSSM: boards[%d] LTU %s\n", ix, sms[board].ltubase);
    if(nltus>=5) {
      printf("startSSM: too many LTUs\n"); rc=1; goto RTRN;
    };
    vsps[nltus]= Openvsp(sms[board].ltubase);
    if(vsps[nltus]<0) {
      printf("startSSM: LTU %s -vme cannot be opened\n",
        sms[board].ltubase); rc=2; goto RTRN;
    };
    nltus++;
  };
  if(ix==0) break;
  ix++;
};
ixltus=0; ix=1;
GetMicSec(&seconds1, &micseconds1);
while(1) {
  if(ix>=n) ix=0;
  board= boards[ix];
  if(board>=NCTPBOARDS) {   /* ltu */
    vsp= vsps[ixltus]; ixltus++;
    vmxw32(vsp, SSMaddress,0);   // start always from beginning
    vmxw32(vsp, SSMstart,DUMMYVAL);
  } else {                  /* ctp board */
    boardoffset=BSP*ctpboards[board].dial; 
    vmew32(SSMaddress+boardoffset,0);
    /*    micwait(30); */
    status= vmer32(SSMstatus+boardoffset);
    vmew32(SSMstart+boardoffset, DUMMYVAL);
  };
  if(ix==0) break;
  ix++;
};
GetMicSec(&seconds2, &micseconds2);
for(ix=0; ix<nltus; ix++) {
  vsp= vsps[ix];
  Closevsp(vsp);
};
if((diff=DiffSecUsec(seconds2, micseconds2, seconds1, micseconds1))>80) {
  rc=10;
  printf("startSSM: diff[usecs]=%d SSMstatus before start:%x\n",diff, status);
};
RTRN:return(rc);
}

/*FGROUP DebugSSMcalls */
int startSSM1(int board) {
int boards[1];
boards[0]= board;
return(startSSM(1, boards));
}

/*FGROUP DebugSSMcalls
------------------------------------------------------- stopSSM()
Opearation:
- check if board is in BUSY status
- stop (recording or generation)
rc: == 0 OK
       1 board not busy, no action
       2 problem with openvme for LTU
*/
int stopSSM(int board) {
int vsp=0, rc=0;
w32 ssmstatus, boardoffset, busybit;
if(board>=NCTPBOARDS) {   /* ltu */
  vsp= Openvsp(sms[board].ltubase);
  if(vsp<0) {
    printf("stopSSM: LTU %s -vme cannot be opened\n",
      sms[board].ltubase); rc=2; goto RTRN;
  };
  boardoffset=0;
  busybit=0x04;
} else {     
  boardoffset=BSP*ctpboards[board].dial;
  busybit=0x100;
};
ssmstatus= vmbr32(vsp, SSMstatus+boardoffset);
if( (ssmstatus&busybit)==0 ) {
  printf("stopSSM: %d board not busy(SSMstatus:%x), no action\n", 
    board, ssmstatus); 
  rc=1; goto RTRN;
};
vmbw32(vsp, SSMstop+boardoffset, DUMMYVAL);
//printf("stopSSM: %d OK (SSMstatus before stopping:%x)\n", board, ssmstatus);
RTRN:
return(rc);
}

/*FGROUP DebugSSMcalls
Opearation:
- read counter cntpos
- check 'the change of this counter' + check maxloops
- sleep 'sleepafter' micsecs (should be 0 or at least 100)
- stop (recording or generation) -> call stopSSM()
Inputs:
board: 0:busy, 1:L0,..., 10:INT
      21: L0+L1 -> i.e. stop 2 SSM for L0/1 board
cntpos: position of the counter (starting from 0) for this board
        see 2. column in dimcdistrib/cnames.sorted2 file
maxloops: max. number of loops (1 loop == cca 2 ms)
          0: loop forever
customer: bakery customer (see ctplib/readCounters.c)
rc: == 0 OK
   1 board not busy, no action (i.e. SSM was not started before in
     'BEFORE/continuous' mode)
   2 problem with openvme for LTU
  10 timeout (counter did not change even after 'maxloops' reads)
--------------------- */ 
int condstopSSM(int board, int cntpos, int maxloops, int sleepafter, int customer) {
w32 cntval1,cntval2, secs1,mics1,secs2,mics2,diffusecs,incr;
int loops, board1, board2=0;
w32 cntmem1[NCOUNTERS]; w32 cntmem2[NCOUNTERS];  //the all counters version
int rc=10;
if(board<20) {board1=board;} else {
  board1= board-20;
  board2= board1+1;
  if(board2>10) board2=0;
};
GetMicSec(&secs1, & mics1);
//cntval1= getCounter(board1, cntpos); //the 'current board counters only' version
readCounters(cntmem1, NCOUNTERS, 0, customer); cntval1=cntmem1[cntpos];
//for(loops=0; loops<maxloops; loops++) {
loops=0;
while(loops<maxloops){
  //cntval2= getCounter(board1, cntpos);
  readCounters(cntmem2, NCOUNTERS, 0, customer); cntval2=cntmem2[cntpos];
  if( cntval1 != cntval2) {rc=0; 
    if(sleepafter >0) usleep(sleepafter);
    rc= stopSSM(board1);
    if(board2!=0) {
      rc= stopSSM(board2);
    };
    break;
  };
  if(maxloops>0)loops++;
};
GetMicSec(&secs2, & mics2); incr= dodif32(cntval1, cntval2);
diffusecs= DiffSecUsec(secs2,mics2,secs1,mics1);
printf("Counter:%d(increase:%d) loops:%d(%d micsecs)\n", 
  cntpos, incr, loops, diffusecs);
return(rc);
}

/*FGROUP DebugSSMcalls
--------------------------------------------------------- readSSM()
read whole SSM into array of unsigned ints
Note: InOut, ConfSel bits and SSMenable word left unchanged
Input:
board: board according to sms global array
Output:
rc:  0 SSM read into sms[board].sm->
       i.e. Mega words were read
    >0 only part of SSM was read into sms[board].sm-> 
       reason in After or Before mode:
       - SSM stopped early (<26ms)
*/
int readSSM(int board) {
w32 ssmoffset,d, ssma,words, i; int rc, vsp;
w32 *array;
if(sms[board].sm==NULL) {
  array= (w32 *)malloc(Mega*sizeof(w32));
  sms[board].sm= array;
} else {
  array= sms[board].sm;   /* rewrite already allocated memory */
};
/*sms[board].syncflag= -1;    this SSM is not synchronised */
/* pattern for debugging: 
for(i=0; i<Mega; i++) {
  array[i]= i<<board;
};
return(0); */
ssmoffset= BSP*ctpboards[board].dial;
/* Read Mega words from SSM into buf.*/
/*rc= setomSSM(board, SSMomvmer); if(rc!=0) return(rc); */
if(sms[board].ltubase[0]=='\0') {   /* ctp board */
  w32 status,enable,mod;
  /* don't touch InOut, ConfSel bits and SSMenable word when VME access*/
  status= vmer32(SSMstatus+ssmoffset);
  enable= (status&0xc0)<<2;
  mod= SSMomvmer | (status&0x38) | enable;
  /*printf("readSSM: status enable mod: %x %x %x\n", status,enable,mod); */
  ssma= vmer32(SSMaddress+ssmoffset); 
  //printf("SSMaddress:%x\n",ssma);
  // SSMaddress has to be read before setom, otherwide overflow bit is cleared 
  rc= setomvspSSM(0, ssmoffset, mod);
  if(rc!=0) return(rc);
  if( ((ssma & 0x100000)==0) ) {
    /* recording was done in Before mode, we
    should read only part of the SSM */
    printf("After or Before mode data without Overflow flag, %d words\n",ssma);
    vmew32(SSMaddress+ssmoffset,(w32)-1);
    words= ssma;
  } else {
    words= Mega;
  };
  d= vmer32(SSMdata+ssmoffset);
  d= vmer32(SSMdata+ssmoffset);
  for(i=0; i<words; i++) {
    array[i]= vmer32(SSMdata+ssmoffset);
  };
} else {                            /* ltu board */
  vsp=Openvsp(sms[board].ltubase);
  rc= setomvspSSM(vsp, 0, SSMomvmer);
  if(rc!=0) return(rc);
  //vmxw32(vsp, SSMaddress,-1);
  ssma= vmer32(SSMaddress); 
  /*  printf("SSMaddress:%x\n",ssma); */
  if( ((ssma & 0x80000)==0) ) {
    /* recording was done in Before mode, we
    should read only part of the SSM */
    printf("After or Before mode data without Overflow flag, %d words\n",ssma);
    vmew32(SSMaddress+ssmoffset,(w32)-1);
    words= ssma;
  } else {
    words= Mega;
  };
  d= vmxr32(vsp, SSMdata);
  d= vmxr32(vsp, SSMdata);
  for(i=0; i<words; i++) {
    array[i]= vmxr32(vsp, SSMdata);
  };
  Closevsp(vsp);
};
return(rc);
}
/*FGROUP DebugSSMcalls
------------------------------------------------------ writeSSM()
write whole sms[].sm into hardware
Note: InOut, ConfSel bits and SSMenable word left unchanged
Input:
board: board according to sms global array
rc:    0: no errors found during writing
*/
int writeSSM(int board){
int n,rc,ix,vsp;
w32 ssmoffset, data, address;
w32 *array;
n=Mega; address=0;    /* all, from the beginning of the memory */
if(sms[board].sm==NULL) {
  /*array= (w32 *)malloc(Mega*sizeof(w32));
  sms[board].sm= array; */ rc=-1; return(rc);
} else {
  array= sms[board].sm;
};
printf("Writing sms[%i].sm to hardware \n",board);
ssmoffset= BSP*ctpboards[board].dial;
if(sms[board].ltubase[0]=='\0') {          /* ctp board */
  w32 status,enable,mod;
  /* don't touch InOut, ConfSel bits and SSMenable word when VME access*/
  status= vmer32(SSMstatus+ssmoffset);
  enable= (status&0xc0)<<2;
  mod= SSMomvmew | (status&0x38) | enable;
  rc= setomvspSSM(0, ssmoffset, mod);
  if(rc!=0) return(rc);
  vmew32(SSMaddress+ssmoffset, address-1);
  for(ix=0; ix<Mega; ix++) {
    vmew32(SSMdata+ssmoffset, array[ix]);
    /*  usleep(1); */
  };  
  /* Final write addr. should be == n-1 */
  data= vmer32(SSMaddress+ssmoffset);
} else {                                    /* LTU board */
  vsp=Openvsp(sms[board].ltubase);
  rc= setomvspSSM(vsp, 0, SSMomvmew);
  if(rc!=0) return(rc);
  vmxw32(vsp, SSMaddress, address-1);
  for(ix=0; ix<Mega; ix++) {
    vmxw32(vsp, SSMdata, array[ix]);
    /*  usleep(1); */
  };  
  /* Final write addr. should be == n-1 */
  data= vmxr32(vsp, SSMaddress);
  Closevsp(vsp);
};
if( data != (w32)(n-1)) {
  printf("ERROR: final write SSMaddress:%d=0x%x expected:%d\n",
         data, data, n-1);
  rc=-1;
};
return(rc);
}
/*FGROUP DebugSSMcalls
------------------------------------------------------ dumpSSM()
read SSM from hw and write it into file
Input:
board: board according to sms global array
rc:    0: no errors found during writing
*/
int dumpSSM(int board, char *fname) {
w32 d, ssmoffset;
int vsp,i,retcode=0;
char fn[100]="WORK/";
FILE *dump;
ssmoffset= BSP*ctpboards[board].dial;
strcat(fn,fname);
dump= fopen(fn,"w");
if(dump==NULL) {
  printf("cannot open file %s\n", fn);
  retcode=1; goto RET;
};
if(sms[board].ltubase[0]=='\0') {   /* ctp board */
  w32 status,enable,mod;
  /* don't touch InOut, ConfSel bits and SSMenable word when VME access*/
  status= vmer32(SSMstatus+ssmoffset);
  enable= (status&0xc0)<<2;
  mod= SSMomvmer | (status&0x38) | enable;
  /*printf("readSSM: status enable mod: %x %x %x\n", status,enable,mod); */
  retcode= setomvspSSM(0, ssmoffset, mod);
  if(retcode!=0) return(retcode);
  vmew32(SSMaddress+ssmoffset,(w32)-1);
  d= vmer32(SSMdata+ssmoffset);
  d= vmer32(SSMdata+ssmoffset);
  for(i=0; i<Mega; i++) {
    /*   *buf= vmer32(SSMdata+ssmoffset); buf++; */
    d= vmer32(SSMdata+ssmoffset);            /* 32 bits wide */
    fwrite(&d, sizeof(w32), 1, dump);
  };
} else {                            /* ltu board */
  vsp=Openvsp(sms[board].ltubase);
  retcode= setomvspSSM(vsp, 0, SSMomvmer);
  if(retcode!=0) return(retcode);
  vmxw32(vsp, SSMaddress,(w32)-1);
  d= vmxr32(vsp, SSMdata);
  d= vmxr32(vsp, SSMdata);
  for(i=0; i<Mega; i++) {
    /*   *buf= vmxr32(vsp, SSMdata); buf++; */
    d= vmxr32(vsp, SSMdata);            /* 32 bits wide */
    fwrite(&d, sizeof(w32), 1, dump);
  };
  Closevsp(vsp);
};
/* ssma= vmxr32(vsp, SSMaddress);
  printf("SSMaddress end:%x\n",ssma); */
fclose(dump);
printf("%s created\n", fn);
RET:return(retcode);
}
#define NOCOMPRESS
/*FGROUP DebugSSMcalls
Dump computer ssm (not hw ssm)
compress: 0: old way 1; new way i.e. compressed
*/
int dumpssm_compress(int board,char *fname, int compress){
 int i;
w32 repetitions=1; w32 d1; int nomore=0;
 w32 d,ssmoffset;
 char fn[100]="WORK/";
 FILE *dump;
 if(sms[board].sm==NULL){
  printf("dumpssm: ssm memory not read.\n");
  return 2;
 }
 ssmoffset= BSP*ctpboards[board].dial;
 strcat(fn,fname);
 dump= fopen(fn,"w");
 if(dump==NULL) {
  printf("cannot open file %s\n", fn);
  return 1;
 };
if(compress==0) {
for(i=0; i<Mega; i++){d= sms[board].sm[i]; fwrite(&d,sizeof(w32),1,dump);};
} else {   
d= sms[board].sm[0];
for(i=1; i<Mega; i++) {
  d1= sms[board].sm[i]; if(i>=(Mega-1)) nomore=1;
  if(d==d1) {
    repetitions++;
    if(nomore==1) {
      fwrite(&d1, sizeof(w32), 1, dump);
      fwrite(&repetitions, sizeof(w32), 1, dump);
      break;
    };
  } else {
    fwrite(&d, sizeof(w32), 1, dump);
    if(repetitions>1) {
      fwrite(&d, sizeof(w32), 1, dump);
      fwrite(&repetitions, sizeof(w32), 1, dump);
    };
    d= d1;
    if(nomore==1) {
      fwrite(&d1, sizeof(w32), 1, dump);
      break;
    };
  };
};
};
 fclose(dump);
 return 0;
}

/*FGROUP DebugSSMcalls
print to stdout SSM board from word 'fromadr'
*/
void printSSM(int board, int fromadr) {
w32 *array; int adr;
array= sms[board].sm;   /* rewrite already allocated memory */
for(adr=fromadr; adr<fromadr+10; adr++) {
  printf("%7d: 0x%8.8x\n", adr, array[adr]);
};
}
/*
int readSSMDumpMega(int board,char *filename){
  int i,nwords;
  w32 word; 
  FILE *dump;
  dump = fopen(filename,"rb");
   if(dump==NULL) {
    printf("cannot open file %s\n",filename);
    return(1);
  };
  if(sms[board].sm==NULL)sms[board].sm = (w32 *)malloc(Mega*sizeof(w32));
  for(i=0;i<Mega;i++){
    nwords=fread(&word,sizeof(w32),1,dump);
    //printf("%i 0x%x %i\n",i,word,nwords);
    sms[board].sm[i]=word;
  }
  return 0;
} */
/*FGROUP SimpleTests
 Read binary dump written before by dumpSSM() to the sms[board].sm
board: 0,1,2,... ->   BUSY, L0, L1,...
filename: 'WORK/name.dmp'
compress: 0: old way 1; new way i.e. compressed
*/
int readSSMDump_compress(int board,char *filename, int compress){
  int i,nwords;
  w32 word,word1,rep; 
  FILE *dump;
  dump = fopen(filename,"rb");
   if(dump==NULL) {
    printf("cannot open file %s\n",filename);
    return(1);
  };
  if(sms[board].sm==NULL)sms[board].sm = (w32 *)malloc(Mega*sizeof(w32));
if(compress==0) {
for(i=0;i<Mega;i++){
  nwords=fread(&word,sizeof(w32),1,dump);
  //printf("%i 0x%x %i\n",i,word,nwords);
  sms[board].sm[i]=word;
}
} else {
i=0;
while(1) {
  w32 ix;
  nwords=fread(&word,sizeof(w32),1,dump); if(nwords!=1) goto STP;
  while(1) {
    nwords=fread(&word1,sizeof(w32),1,dump); if(nwords!=1) goto STP;
    if(word==word1) break;
    sms[board].sm[i]=word; i++;
    word= word1;
  };
  nwords=fread(&rep,sizeof(w32),1,dump); 
  if(rep<2) {
    printf("readSSMDump: rep:%d<2!\n",rep); rep=2;
  };
  for(ix=0; ix<rep; ix++) {
    sms[board].sm[i]=word; i++;
  };
  //printf("%i 0x%x %i\n",i,word,nwords);
};
STP:i=i;
};
fclose(dump);
return 0;
}

int dumpssm(int board,char *fname){
return(dumpssm_compress(board,fname, 0));
}
/*FGROUP SSMcalls
Read binary dump written before by dumpSSM() to the sms[board].sm
Inputs:
board: 0,1,2,... ->   BUSY, L0, L1,L2,INT,FO1,...,FO6,ltu1,ltu2,
       ltu3, ltu4, test, none  (see ssmctp.h)
       ltu1: LTU sitting in the same VME crate, base addr: 0x811000
filename: e.g. 'WORK/name.dmp'
mode: text indicating mode, i.e. the names of 32 signals assigned
      to SSM bits in file $dbctp/../ssmsigs/mode.sig
Output: rc: 0: ok
            5: incorrect mode (too long)
*/
int readSSMDumpMode(int board,char *filename, char *mode){
int rc;
rc= readSSMDump_compress(board,filename, 0);
if(rc==0) {
  if(strlen(mode)> (MAXSSMMODE-1)) return(5);
  strcpy(sms[board].mode, mode);        
};
return(rc);
}
int readSSMDump(int board,char *filename){
return(readSSMDump_compress(board,filename, 0));
}
