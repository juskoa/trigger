#include <stdio.h>
#include <unistd.h>
#include "vmewrap.h"
#include "ltu.h"

w32 SSMSCHEDULER=0;   /* SSM recording planning, 2-After, 3-Before, 0-no plan */
extern int quit;

/* enadis: 0-> disable errors   1-> enable errors */
void ERenadis(int enadis) {
if(enadis==1) {
  vmew32(ERROR_ENABLE, 0x1);
} else {
  vmew32(ERROR_ENABLE, 0x0);
};
}
/*
selector: bits[6:0] */
void ERsetselector(w32 selector) {
vmew32(ERROR_SELECTOR, selector);
}
/*
demand: bits[2:0] valid values: 1-6
*/
void ERdemand(w32 demand) {
vmew32(ERROR_DEMAND, demand);
}

/* set START_SET word:
bits:
0x8  1: veto START in LHC gap (should be 1 by default)
0x4  1: edge sensitive mode, 0: level sensitive mode 
0x3  0: no input selected, 1: PULSER, 2: Random, 3: BC scaled down
*/
void SLMsetstart(w32 sel) {
/*printf("SLMsetstart:%x\n",sel); */
if((SSMSCHEDULER>=2) && (SSMSCHEDULER<=3) && ((sel&0x3)!=0) ) {
  w32 opmo;
  opmo= SSMSCHEDULER; 
  printf("Starting SSM recording %s\n",getAB(opmo));
  SSMstartrec(opmo);
};
vmew32(START_SET, sel);
}
/*FGROUP SLM
*/
int SLMgetstart() {
return(vmer32(START_SET)&0x0f);
}
/*FGROUP SLM
Generate start signal by SW
n       -number of SW triggers (0-endless loop)
milsecs -time interval, in miliseconds, between triggers 
return  -number of generated SW triggers 
*/
int SLMswstart(int n, int milsecs) {
int i,micsecs;
i=0; micsecs=milsecs*1000;
if((SSMSCHEDULER>=2) & (SSMSCHEDULER<=3)) {
  w32 opmo;
  opmo= SSMSCHEDULER; 
  printf("Starting SSM recording %s\n",getAB(opmo));
  SSMstartrec(opmo);
};
while(1){
  vmew32(SOFT_TRIGGER, DUMMYVAL);
  if(micsecs > 0) usleep(micsecs);
  i++;
  if((n!=0) && (i>=n)) break;
  if(quit !=0) {quit=0; break;};
};
return(i);
}
/*FGROUP SLM
Start emulation. Operation:
1. check if BC present
2. check if emulation is active
3. clear pipeline
4. start emulation (i.e. write DUMMY data to EMULATION_START register)
rc: 0 -> emulation started
*/
int SLMstart() {
w32 st;
int rc=0,ip;
/* wait at least 120micsec before start/stop global/stdalone: */
/*usleep(GLBSTDDELAY); */
/* check if emulation is active, BC present: */
if((vmer32(STDALONE_MODE)&0x1)==0) {
  printf("ERROR: GLOBAL mode active (emulation not started)\n");
  rc=4; goto ERRRET;
};
if((vmer32(BC_STATUS)&0x2) == 0) {
  printf("ERROR: BC not present\n");
  rc=2; goto ERRRET;
};
/* usleep(1000000); read from non existent register returns
 * last read value (even after 10milsec) */
/*st= vmer32(EMU_STATUS); printf("%x\n",st); */
if(vmer32(EMU_STATUS)&0x1) {
  printf("ERROR: emulation active, quitting it before starting:\n");
  SLMquit(); usleep(2000000); /* rc=3; goto ERRRET; */
};
vmew32(PIPELINE_CLEAR, DUMMYVAL);
for(ip=0; ip<500; ip++) {
  st= vmer32(EMU_STATUS)&0x2;   /* pipeline busy */
  if(st==0) break;
};
if(ip >499) {
  printf("problem when pipeline clearing, loops:%d\n",ip);
};
vmew32(EMULATION_START, DUMMYVAL);
#ifdef SIMVME
vmew32(EMU_STATUS, 1);
#endif
ERRRET: 
/*printf("<%d>\n",rc); */
return(rc);
}

/*--------------------------------------------------- SLMreadasci()
Input:
filen: .seq file to be read
Output:
slmdata: binary data to be loaded into SLM
rc: 1 file cannot be accessed
    2 error in .seq file
*/
#define SLMSKIPL 3
#define MAXLINE 80
int SLMreadasci(char *filen, w32 *slmdata) {
int rc=0,ixx;
int linenum;
FILE *slm;
char line[MAXLINE];
/* convert file to binary: */
slm= fopen(filen, "r");
if(slm ==NULL) {
  printf("File %s cannot be opened\n",filen);
  rc=1; return(rc);
};
linenum=0,ixx=0; 
while(1){
  int ix;
  char *frc;
  w32 dw;
  frc= fgets(line, MAXLINE, slm);
  if((frc==NULL) && (linenum<10) ) {line[0]='\0'; goto ERRLINE;};
  if(frc==NULL) break;
  if(line[0]=='\n') {
    if(((linenum-3)%8)==0) {
      break;
    } else {
      goto ERRLINE;
    };
  };
  linenum++; 
  if(linenum <= SLMSKIPL) continue;
  dw=0;
  for(ix=15; ix>=0; ix--) {
    if(line[ix]=='0') {
    } else if(line[ix]=='1') {
      dw= dw | (1<<(15-ix));
    } else {
      goto ERRLINE;
    };
  };
  /*printf("%d/%d:%4x\n", linenum,ixx, dw); */
  slmdata[ixx]=dw; ixx++;
  if(linenum >=259) break;
};
/*printf("lines read from %s file:%d\n",filen, linenum); */
ERRRET:
fclose(slm);
return(rc);
ERRLINE:
printf("ERROR in SLMreadasci %s. Line:%d %s\n", filen,linenum,line); 
rc=2; goto ERRRET;
}

/*FGROUP SLM 
filen: load this file into SLM memory. Absolute path or
       relative path to $VMEWORKDIR has to be given, i.e.
       CFG/ltu/SLM/one.seq, or CFG/ltu/SLMproxy/sod.seq
rc: 0: ok, loaded
   >0: not loaded, error printed to stdout
       3: emulation active   4: global mode
*/
int SLMload(char *filen) {
int rc=0;
int ixx;
w32 slmdata[MAXSLMW];
/*printf("SLMload: file:%s  working directory:\n",filen); 
rc=system("pwd"); */
/* load binary file into SLM: */
rc= SLMreadasci(filen, slmdata); if(rc!=0) goto ERRRET;
if(vmer32(EMU_STATUS)&0x1) {
  printf("ERROR: SLMload: emulation active\n");
  rc=3; goto ERRRET;
};
if((vmer32(STDALONE_MODE)&0x1)==0) {
  printf("ERROR: SLMload: GLOBAL mode active (load not possible)\n");
  rc=4; goto ERRRET;
};
vmew32(SLM_ADD_CLEAR, DUMMYVAL);   /* clear SLM address counter */
/*dw=vmer32(SLM_ADD_CLEAR); */
usleep(1000);
for(ixx=0; ixx<MAXSLMW; ixx++) {
  vmew32(SLM_DATA,slmdata[ixx]);
  /* usleep(1); (100) 1 word shift in written data */
  /* usleep(1000);    -removed 1.11.2007 */
  /*printf("%d:%4x\n", ixx, slmdata[ixx]); */
};
vmew32(SLM_DATA, DUMMYVAL);   /* force last word to be written */
ERRRET:
return(rc);
}
/* quit emulation. RC: EMU_STATUS after quit
*/
int SLMquit() {
w32 st;
vmew32(QUIT_SET, DUMMYVAL);
if((SSMSCHEDULER>=3) & (SSMSCHEDULER<=3)) {
  w32 opmo;
  opmo= SSMSCHEDULER; 
  printf("Stoppping SSM-BEFORE recording %s\n",getAB(opmo));
  SSMstoprec();
}; usleep(100);
#ifdef SIMVME
vmew32(EMU_STATUS, 0);
#endif
st= vmer32(EMU_STATUS); return(st);
}
/*
Wait for the end of emulation:
- check the EMU_STATUS word once per second, until emulation finished
micsec: 0 -fast wait (no usleep after wmer32(EMU_STATUS...))
       >0 -slow, wait micsec after unsuccessful test
RC: 0 -emulation finished
    1 -emulation didn't finished (SLMwaitemuend() probably killed)
*/
int SLMwaitemuend(int micsec) {
int rc;
while(1) {
  if(vmer32(EMU_STATUS)&0x3) {   /* EMU finished & PIPELINE flashed out */
    if(quit !=0) {quit=0; rc=1; break;};
  } else {
    rc=0; break;
  };
  if(micsec !=0) {
    usleep(micsec);
  };
};
return(rc);
}
