#include <stdio.h>
//#include <stdlib.h>
#include "vmewrap.h"
#include "vmeblib.h"
#include "ltu.h"

#define Mega 1024*1024
extern int quit;
extern int SSMSCHEDULER; /* SSM recording planning, 2-After, 3-Before, 0-no plan */
w32 *smsltu;

/*--------------------------------------------------- SnapShot memory */
const char *txtAfter="After (26ms)";
const char *txtBefore="Before (continuous)";
const char *txtNorec="no recording";
const char *txtBad="internal error-bad rec. mode";
const char *getAB(w32 opmo) {
if(opmo==2) return(txtAfter);
if(opmo==3) return(txtBefore);
if(opmo==0) return(txtNorec);
return(txtBad);
}
/*FGROUP TOP GUI Snapshot_memory "Snapshot Memory"
Snapshot memory:
- start (After/Before mode) or 
- register for start with 'START SIGNAL selection'
- stop
- dump to binary file (WORK/SSM.dump) and text file (ssm.lst)
*/
#define SSMomvmer 0
#define SSMomvmew 1
#define SSMomreca 2
#define SSMomrecb 3
/*FGROUP SSM
Set operation & mode. If SSM is BUSY, an attempt is made to stop
the recording. 
opmo values:
0x0 -VME access, read
0x1 -VME access, write
0x2 or 0x12 -RECORDING, After  (cca 26 milsec)
0x3 or 0x13 -RECORDING, Before (should be stopped by SSMstoprec)
if 0x10 bit set, record 7 fron panel input signals regardless of
the global/standalone LTU mode
RC:  0->ok, mode set   
     1->mode not set, possible errors (printed to stdout):
        BC signal not connected
	Cannot stop recording operation
*/
int SSMsetom(w32 opmo) {
w32 status,opmo2;
opmo2= opmo&0x3;
if((opmo2==SSMomreca || opmo2==SSMomrecb)&&((vmer32(BC_STATUS)&0x3)!=0x2)) {
  printf("ERROR: BC signal not connected\n");/* BC not necessary for vme R/W*/
  return(1);
};
status=vmer32(SSMstatus);
if( status & 0x4) {
  printf("SSM busy, stopping recording before setting new/op. mode...\n");
  vmew32(SSMstop,DUMMYVAL);
  status=vmer32(SSMstatus);
  if( status & 0x4) {
    printf("ERROR: Cannot stop recording, operation/mode\n");
    printf("       not set!\n");
    return(1);
  };
};
vmew32(SSMcommand, opmo);
return(0);
}
/*FGROUP SSM
Start recording. 
mode: 2    ->After            3 ->Before 
      0x102->After+TTCrxreset
      0x12 ->Afterr/FP2ssm 0x13 ->Before/FP2ssm 
! in FP2ssm wait for a change of L0_COUNTER or PP_COUNTER */
void SSMstartrec(w32 mode) {
w32 localmode;
if(mode==0x102) {
  localmode=2;
} else {
  localmode=mode;
};
if(SSMsetom(mode)) {
  return;
};
if((mode&0x10)==0x10) { /* FP2ssm mode, wait for counter change */
  w32 cntr1, cntr2;
  printf("Entering loop for waiting L0 or PP counters change.\n");
  printf("Press Kill button to get out of the loop.\n");
  cntr1= getCounter(L0_COUNTERrp);
  cntr2= getCounter(PP_COUNTERrp);
  while(1) {
    w32 cntr;
    if(quit !=0) {quit=0; 
      printf("interrupt received: %d\n",quit);
      break;};
    cntr= getCounter(L0_COUNTERrp);
    if(cntr != cntr1) {
      printf("L0 counter change detected, starting SSM for front panel\n");
      break;};
    cntr= getCounter(PP_COUNTERrp);
    if(cntr != cntr2) {
      printf("PP counter change detected, starting SSM for front panel\n");
      break;};
  };
};
  vmew32(SSMaddress,0);
  vmew32(SSMstart,DUMMYVAL);
  if(mode==0x102) {
    TTCrxreset();
    printf("SSM recording (26ms) + TTCrx reset started\n");
  };
}
/*FGROUP SSM
Stop recording. Should be issued only if recording
was started in BEFORE mode.
*/
void SSMstoprec() {
w32 status;
status=vmer32(SSMstatus);
if( (status & 0x4)==0) {
  printf("Warnining: recording not active. SSMstatus:%x going to VME/read mode\n",status);
};
vmew32(SSMstop,DUMMYVAL);
status=vmer32(SSMstatus);
if( status & 0x4) {
  printf("ERROR: Cannot stop recording. SSMstatus:%x, no action\n",status);
} else {
  SSMsetom(0);   /* go to default mode: VME/read */
};
}
/*FGROUP SSM
Read SSM to memory 
RC: 1->OK, 0->dump not successful
*/
int readSSM(w32 *sm) {
/* 
in After mode: 
  recording starts from address 1 and finishes at the address 0
  which is correct, SSMaddress is set to 0 by itself after After
  recording. Overflow flag is not set for After mode.
*/
w32 d;
int i,words=Mega;

if(SSMsetom(0)) {   /* VME access, read */
  printf("readSSM not successful\n");
  return(0);
} else {
  vmew32(SSMaddress,0xffffffff);  //-1
  d= vmer32(SSMdata);
  d= vmer32(SSMdata);
  for(i=0; i<words; i++) {
    /*   *buf= vmer32(SSMdata); buf++; */
    d= vmer32(SSMdata)&0x3ffff;    /* 18 bits wide */
    sm[i]=d; 
  };
  return(1);
};
}
/*FGROUP SSM
Dump SSM to file WORK/SSM.dump
RC: 1->OK, 0->dump not successful
*/
int SSMdump() {
/* 
in After mode: 
  recording starts from address 1 and finishes at the address 0
  which is correct, SSMaddress is set to 0 by itself after After
  recording. Overflow flag is not set for After mode.
in Before mode:
  OK -SSMaddress is set by itself to right position (if overflow flag
  is ON. If not, we start from beginning of the memory, as in mode
  After 
Reading out memory after being initialised by SSMclear() and 
  then filled from another LTU:
  Before calling SSMdump(), the SSMaddress has to be set to -1
*/
w32 d,ssma;
int i,words=Mega;
FILE *dump;

if(SSMsetom(0)) {   /* VME access, read */
  printf("dump not successful\n");
  return(0);
} else {
  ssma= vmer32(SSMaddress); 
  /*  printf("SSMaddress:%x\n",ssma); */
  if( ((ssma & 0x80000)==0) && (SSMSCHEDULER==3)) {
    /* recording was done in Before mode, we
    should read only part of the SSM */
    printf("Before mode data without Overflow flag, %d words\n",ssma);
    words= ssma;
  };
  dump= fopen("WORK/SSM.dump","w");
  if(dump==NULL) {
    printf("cannot open file WORK/SSM.dump\n");
    return(0);
  };
  d= vmer32(SSMdata);
  d= vmer32(SSMdata);
  for(i=0; i<words; i++) {
    /*   *buf= vmer32(SSMdata); buf++; */
    d= vmer32(SSMdata)&0x3ffff;    /* 18 bits wide */
    fwrite(&d, sizeof(w32), 1, dump); 
  };
  ssma= vmer32(SSMaddress); 
  /*  printf("SSMaddress end:%x\n",ssma); */
  fclose(dump);
  printf("WORK/SSM.dump created in VMEWORKDIR\n");
  return(1);
};
}
/*FGROUP SSM
Schedule SSM recording 
whenmode: <10 -reserved for after/before mode starts from
          CTP emulator window
 2 -start After when 1st SW START or STARTSIGNAL selected
 3 -start Before       ---
*/
void SSMschedule(w32 whenmode) {
if(whenmode<10) {
  /* see SLMswstart, SLMsetstart */
  printf("SSM recording scheduled:%s\n",getAB(whenmode));
  SSMSCHEDULER=whenmode;
};
}
/*FGROUP SSM
This should be called before starting write from another LTU.
*/
int SSMclearac() {
int rc;
rc= SSMsetom(SSMomvmew); vmew32(SSMaddress,0xffffffff);  //-1
return(rc);
}
/*FGROUP SSM
Prepare SSM for data recording from another LTU running in 'Test mode'
Operation:
- set address counter to the beginning of the SSM
- fill SSM with 0x00001 (18 bits wide)
- set address counter to the beginning of the SSM
- set operational mode to VME write
*/
void SSMclear() {
int i,rc;
w32 rnd;
rc= SSMclearac();
for(i=0; i<Mega; i++) {
/*   rnd= (0x3ffff+1)*rnlx(); */  rnd=1;
  vmew32(SSMdata, rnd);
};
rc= SSMclearac();
if(rc) {
  printf("SSM clear unsuccessful\n");
};
}
/*FGROUP SSM */
void setvmespy() {
SSMclear();
vmew32(SSMcommand, 1);
}
/*FGROUP SSM */
void setvmenormal() {
vmew32(SSMcommand, 0);
}
/*FGROUP SSM */
void printSSM(int words) {
w32 ssm[Mega];
int ix,rc;
rc=readSSM(ssm);
printf("readSSM rc:%d\n",rc);
for(ix=0; ix<words; ix++) {
  printf("%4d: %8x\n", ix, ssm[ix]);
};
}

/*FGROUP DbgSSMBROWSERcalls
Extract 1 signal to stdout:
Input:
board:   (0...) according to sms global array
bit:     SSM bit (0-31)
frombc: bc number. 
         0 corresponds to word with address sms[board].offset
bits:    number of bits to be examined (but don't print more then
         102 lines)
Output:
value_of_the_1st_bit      or <0 if error
bit_number_for_which_value_changed
bit_number_for_which_value_changed
...
Errors:
-1 -> required SSM not read
*/
void getsigSSM(int board, int bit, int frombc, int bits) {
int adr,adr1,adr2,adrprint,curvalbin,rc=0;
w32 *ssmbase;
char curval;
adr1=frombc; 
//adr1=frombc+sms[board].offset;   //mod
/*adr2= adr1+ bits -1; */
adr2=Mega;
ssmbase= smsltu;       //mod
if(ssmbase==NULL) {
  printf("-1\n"); return;
};
if(ssmbase[adr1] & (1<<bit)) {
  curval='1'; curvalbin=1;
} else {
  curval='0'; curvalbin=0;
};
printf("%c\n", curval); rc++;
adrprint=1;
for(adr=adr1+1; adr<=adr2; adr++) {
  if(w32toint(((ssmbase[adr]>>bit) & 1)) != curvalbin) {
    printf("%d\n", adrprint); rc++;
    /*    if(rc>bits+1) break; */
    if(rc>102) break;
    if(curvalbin) {
      curval='0'; curvalbin=0;
    } else {
      curval='1'; curvalbin=1;
    };
  };
  adrprint++;
};
}
/*FGROUP DbgSSMBROWSERcalls
Find signal change.
Input:
board,bit,frombc: as in getsigSSM()
Output (on stdout):
-1 -signal does not change (or memory not accessible)
n  - pointing to the last bit with the same value, next bit
     is different
*/
void finddifSSM(int board, int bit, int frombc) {
int adr,adr1,adr2,adrprint,curvalbin;
w32 *ssmbase;
char rctxt[12]="-1";
//ssmbase= sms[board].sm;
ssmbase= smsltu;
if((ssmbase==NULL) || (frombc>=(Mega-1))) {
  printf("-1\n"); return;
};
//adr1=frombc+sms[board].offset; adr2= Mega -1;
adr1=frombc; adr2= Mega -1;
if(ssmbase[adr1] & (1<<bit)) {
  curvalbin=1;
} else {
  curvalbin=0;
};
adrprint=frombc+1;
for(adr=adr1+1; adr<=adr2; adr++) {
  if(w32toint(((ssmbase[adr]>>bit)) & 1) != curvalbin) {
    sprintf(rctxt, "%d", adrprint-1);
    break;
  };
  adrprint++;
};printf("%s\n", rctxt);
}


