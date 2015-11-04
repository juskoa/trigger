#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "vmewrap.h"
#include "infolog.h"
#include "ctp.h"
#include "ctplib.h"
/* only 1 of TRGDBmysql TRGDBfiles can be defined */
//#define TRGDBmysql
#define TRGDBfiles
#include "Tpartition.h"
#include "lexan.h"
#include "vmeblib.h"

#define MAXPARNAME 80

char em1[ERRMSGL]="emptyem1";
/* rc 1:error  (err. msg in em1)  0: ok
------*/ int getival(char *line, char *parname, w32 *ival) {
char value[MAXCTPINPUTLENGTH];
int ix=0;
enum Ttokentype token;
token=nxtoken(line, value, &ix);
if((token==tINTNUM) || (token==tHEXNUM)) {   
  if(gethexdec(value, ival)!=0) {
    sprintf(em1,"%s: hex or int number expected", parname); return(1);
  };
  printf("getival: %s %d=0x%x\n", parname, *ival, *ival);
  return(0); //vmew32(BUSY_DELAY_ADD, ival);
} else {
  sprintf(em1,"bad BUSY_DELAY_BC parameter:%s",value); return(1);
};     
}
#define MAXSIMPLEPARS 50
typedef struct Tsimplepars {
  char name[MAXPARNAME];
  w32 address;
} Tsimplepars;
Tsimplepars simplepars[MAXSIMPLEPARS]= {
{"BUSY_DELAY_BC", BUSY_DELAY_ADD},
{"BUSY_CTPDEADTIME", BUSY_CTPDEADTIME},
{"BUSY_ORBIT_SELECT", BUSY_ORBIT_SELECT}, //3563|0x2000 = 0x2deb in local mode
{"ROIP_BUSY", ROIP_BUSY},
//{"L0_INTERACT1", L0_INTERACT1},
//{"L0_INTERACT2", L0_INTERACT2},
//{"L0_INTERACTT", L0_INTERACTT},
{"L0_INTERACTSEL", L0_INTERACTSEL}, 
{"LM_INTERACTSEL", LM_INTERACTSEL},  
{"INT_DDL_EMU", INT_DDL_EMU},
{"INT_MASK_1_24", INT_MASK_FOR_INPUTS_1_24},
{"INT_MASK_25_48", INT_MASK_FOR_INPUTS_1_24+4},
{"", 0}
};
/*-------------------------*/ int setSimplePar(char *parval, char *parname) {
/* rc: 0 ok, set
       1 not found
      >1 found with error
*/ 
int ix, rc, ret=0; w32 ival;
for(ix=0; ix<MAXSIMPLEPARS; ix++ ) {
  if(simplepars[ix].name[0]=='\0') break;
  if(strcmp(parname, simplepars[ix].name)==0) {
    if((rc=getival(parval, parname, &ival))==0) {
      w32 adr;
      adr=simplepars[ix].address;
      //if(adr==L0_INTERACTSEL) adr= getLM0addr(L0_INTERACTSEL);
      vmew32(adr, ival); 
      return(ret);
    } else { return(2); };
  };
};
return(1);
}
/* 1: error 0:ok */
int setcheckParErr(char *parname, char *parval, w32 vmeadr, w32 (*calcFunction)() ) {
int rc; w32 ival;
if((rc=getival(parval, parname, &ival))==0) {
  w32 cival;
  vmew32(vmeadr, ival);
  cival= calcFunction(); //check
  if(ival!= cival) {
    char emsg[200];
    sprintf(emsg, "ctp.cfg %s written: %d calculated:%d\n",
      parname,ival,cival); prtWarning(emsg);
  };
};
return(rc);
}

/*---------------------------------------------------------loadcheckctpcfg()
load and check(timingspar.c) ctp.cfg file.
- fill HW
rc: 0: ok, rc>0 error loading/check
*/
int loadcheckctpcfg() {
#ifdef TRGDBfiles
FILE *cfgfile;
int ix, params=0, grc=0;
enum Ttokentype token;
char parname[MAXPARNAME];
char line[MAXLINELENGTH], value[MAXLINELENGTH];
char emsg[ERRMSGL];
cfgfile=openFile("ctp.cfg","r");
if(cfgfile == NULL) {
  printf("ctp.cfg not present...\n");
  return(1);
} else {
  printf("ctp.cfg load...\n");
};
while(fgets(line, MAXLINELENGTH, cfgfile)){
  int rc; w32 ival;
  //printf("Decoding line:%s:\n",line);
  if(line[0]=='#') continue;
  if(line[0]=='\n') continue;
  //if(line[strlen(line)-1]=='\n') line[strlen(line)-1]='0'; 
  ix=0; token= nxtoken(line, value, &ix);
  if(token!=tSYMNAME) {
    strcpy(emsg,"Symbolic name expected first"); goto ERRlineignore; };
  value[MAXPARNAME-1]='\0'; strcpy(parname, value); 
  /* cases:
   -just load
   -load and check (calc...)
  */
  if(strcmp(parname,"TIMING_BASE")==0) {
    int ixx; w32 a5[5];
    if(params!=0) {
      strcpy(emsg,"TIMING_BASE expected as first line"); goto ERRfatal;
    };
    for(ixx=0; ixx<5; ixx++) {
      token=nxtoken(line, value, &ix);
      if((token==tINTNUM) || (token==tHEXNUM)) {   
        int rc;
        // TL1 TL2 TBCL0 CALIBRATION_BC ORBITLENGTH
        rc= gethexdec(value, &a5[ixx]);
        if(rc!=0) {
          strcpy(emsg,"bad TIMING_BASE line"); goto ERRfatal;
        };
      };
    };
    setTimeParsDB(a5[0], a5[1], a5[2], a5[3], a5[4]);
    printf("TIMING_BASE:%d %d %d %d %d\n",a5[0], a5[1], a5[2], a5[3], a5[4]);
    goto CONT;
  };
  if(strcmp(parname,"TIMESHARING")==0) {
    int ixx; w32 a9[10];
    //printf("TIMESAHRING:nott done yet\n");
    for(ixx=0; ixx<10; ixx++) {
      token=nxtoken(line, value, &ix);
      if((token==tINTNUM) || (token==tHEXNUM)) {   
        int rc;
        // 9 numbers, each is: time in [sec] for time sharing group ix+1
        rc= gethexdec(value, &a9[ixx]);
        if(rc!=0) {
          strcpy(emsg,"bad TIMESHARING line"); goto ERRfatal;
        };
      } else {
          strcpy(emsg,"bad TIMESHARING line:9 numbers (time[sec] expected)"); goto ERRfatal;
      };
    };
    printf("TIMESHARING:(%d) %d %d %d %d %d %d %d %d %d\n",
      a9[0],a9[1],a9[2],a9[3],a9[4],a9[5],a9[6],a9[7],a9[8], a9[9]);
    for(ixx=0; ixx<10; ixx++) { clg_defaults[ixx]= a9[ixx]; };
    goto CONT;
  };
  if((strcmp(parname,"L0_INTERACT1")==0) ||
     (strcmp(parname,"L0_INTERACT2")==0) ||
     (strcmp(parname,"L0_INTERACTT")==0)) {
    /* we cannot use lookup table (we do not know which inputs are
       connected). Format for L0INTERACT1/2/T:
       L0_INTERACT1 0SMB & (0VBA | 0VNG)
       operation:
       - read
       - calculate lookup table (check if L0 inputs connected)
       - save in HW structure (lookup + text)
       - save in hardware 
    */
    //#define LOOKUPTLEN 300
    #define LOOKUPTLEN 256
    int rc;char lookupt[LOOKUPTLEN];  // longer (can return ERROR message)
    char cmd[120];
    //printf("line:%s:\n",&line[ix]);
    getRestLine(&line[ix], ' ',value);
    //printf("value:%s:\n",value);
    if(value[0]=='\0') {
      printf("%s is empty in ctp.cfg\n", parname); goto CONT;
    };
    /* nebavi:
    sprintf(cmd, "sh -c ""python $VMEBDIR/trigdb.py log2tab '%s'"" ", value);
    */
    //sprintf(cmd, "python $VMEBDIR/trigdb.py log2tab '%s'", value);
    sprintf(cmd, "python $VMEBDIR/trigdb.py log2tab8 '%s'", value);
    //printf("loadctpcfg:%s:%s:\n", parname, cmd);
    // ! following fails when used with ctp.exe started with
    // redirected in/out through cmdlin2.py...
    // REASON: ctp.cfg is in dos format (CR+LF), convert it with dow2unix!
    lookupt[0]= '\0'; rc= popenread(cmd, lookupt, LOOKUPTLEN);
    lookupt[66]='\0'; // popen adds new line at 66
    //printf("loadctpcfg.lookupt:%s:\n", lookupt);
    if((strncmp(lookupt,"0x", 2)!=0) || (rc != EXIT_SUCCESS) ) {
      printf("ERROR in %s definition:%s popenread rc:%d\n",parname, value, rc);
      printf("%s\n", lookupt);
      grc=2 ; goto CONT;
    } else {
      //save in HW structure
      w32 ltv,vmeadr; int ixx;
      if(parname[11]=='1') {
        ixx= ixintfun1; 
        //vmeadr= (L0_INTERACT1);
        vmeadr=1;
      } else if(parname[11]=='2') {
        ixx= ixintfun2; 
        //vmeadr=(L0_INTERACT2);
        vmeadr=2;
      } else {
        ixx= ixintfunt;
        //vmeadr= (L0_INTERACTT);
        vmeadr=3;
      };
      //ltv= hex2int(&lookupt[2]);
      ltv= vmeadr;
      //printf("lookupt:%s ltv:%x\n", lookupt, ltv);
      // to be fixed also here
      HW.rbif->rbif[ixx]= ltv;
      HW.rbif->rbifuse[ixx]= ixx;   // i.e.: INT1 allocated in INT1
      strcpy(&(HW.rbif->l0intfs[(ixx-ixl0fun1)*L0INTFSMAX]), value); // symbolic name
      //save in CTP L0 board:
      //vmew32(vmeadr, HW.rbif->rbif[ixx]);
      // do we need to copy luts also to HW (they are in shm)
      setINTLUT(vmeadr,lookupt);
      setINTLUT(vmeadr+3,lookupt);
      printf("INT %i: %s \n",vmeadr,lookupt);
    };
    goto CONT;
  };
  if(strcmp(parname,"PF_COMMON")==0) {
    int ixx; w32 a3[3];
    for(ixx=0; ixx<3; ixx++) {
      token=nxtoken(line, value, &ix);
      if((token==tINTNUM) || (token==tHEXNUM)) {   
        int rc;
        // TL1 TL2 TBCL0 CALIBRATION_BC ORBITLENGTH
        rc= gethexdec(value, &a3[ixx]);
        if(rc!=0) {
          strcpy(emsg,"bad PF_COMMON line"); goto ERRfatal;
        };
      };
    };
    for(ixx=0; ixx<3; ixx++) {
      w32 pfc, pfcadr;
      pfc= calcPFisd(ixx)<<12;
      if(a3[ixx] != pfc) {
        sprintf(emsg, "ctp.cfg %s(%d) written: 0x%x calculated:0x%x\n",
          parname,ixx,a3[ixx],pfc); prtWarning(emsg);
      };
      if(notInCrate(ixx+1)) continue;
      if(ixx==0) {  // L0 or LM0 board
        pfcadr= getLM0PFad(PF_COMMON);
      }else {
        pfcadr= PF_COMMON;
      };
      vmew32(pfcadr+BSP*ctpboards[ixx+1].dial, a3[ixx]);
    };
    printf("PF_COMMON:%d 0x%x 0x%x\n",a3[0], a3[1], a3[2]);
    goto CONT;
  };
  /*------- Simple parameters, i.e. 'parname value' without the check: */
  rc= setSimplePar(&line[ix], parname);
  if(rc==0) {           // parname found (in simplepars) and set
    //printf("INFO %s: %s\n", parname, &line[ix]);
    continue;
  };
  if(rc>1) {                // parname found, error
    sprintf(emsg, "Error in ctp.cfg when processing %s\n", parname);
    goto ERRfatal;
  };
  //rc==1:                       parname not found in simplepars[] table
  /*-------------------------------- parameters which can be checked: */
  if(strcmp(parname,"FO_DELAY_L1CLST")==0) {
    if((rc=getival(&line[ix], parname, &ival))==0) {
      w32 cival; int ifo;
      cival= calcFO_DELAY_L1CLST(); //check
      if(ival!= cival) {
        sprintf(emsg, "ctp.cfg %s written: %d calculated:%d\n",
          parname,ival,cival); prtWarning(emsg);
      };
      for(ifo=5; ifo<=10; ifo++) {
        if(notInCrate(ifo)) continue;
        vmew32(FO_DELAY_L1CLST+BSP*ctpboards[ifo].dial, ival);
      };
    } else { goto ERRfatal; };
  } else if(strcmp(parname,"FO_FILTER_L1")==0) {
    if((rc=getival(&line[ix], parname, &ival))==0) {
      w32 cival; int ifo;
      cival= calcFO_FILTER_L1(); //check
      if(ival!= cival) {
        sprintf(emsg, "ctp.cfg %s written: %d calculated:%d\n",
          parname,ival,cival); prtWarning(emsg);
      };
      for(ifo=5; ifo<=10; ifo++) {
        if(notInCrate(ifo)) continue;
        vmew32(FO_FILTER_L1+BSP*ctpboards[ifo].dial, ival);
      };
    } else { goto ERRfatal; };
  } else if(strcmp(parname,"BUSY_L0L1DEADTIME")==0) {
    if( setcheckParErr(parname, &line[ix], BUSY_L0L1DEADTIME, calcBUSY_L0L1DEADTIME )!=0) {
    goto ERRfatal; };
  } else if(strcmp(parname,"L0_BCOFFSET")==0) {
    w32 l0_bcoffset;
    if(l0C0()) {
    l0_bcoffset= L0_BCOFFSETr2;
    } else {
    l0_bcoffset= L0_BCOFFSET;
    };
    if( setcheckParErr(parname, &line[ix], l0_bcoffset, calcL0_BCOFFSET )!=0) {
    goto ERRfatal; };
  } else if(strcmp(parname,"L1_DELAY_L0")==0) {
    if( setcheckParErr(parname, &line[ix], L1_DELAY_L0, calcL1_DELAY_L0 )!=0) {
    goto ERRfatal; };
  } else if(strcmp(parname,"L2_DELAY_L1")==0) {
    if( setcheckParErr(parname, &line[ix], L2_DELAY_L1, calcL2_DELAY_L1 )!=0) {
    goto ERRfatal; };
  } else if(strcmp(parname,"L2_BCOFFSET")==0) {
    if( setcheckParErr(parname, &line[ix], L2_BCOFFSET, calcL2_BCOFFSET )!=0) {
    goto ERRfatal; };
  } else if(strcmp(parname,"INT_BCOFFSET")==0) {
    if( setcheckParErr(parname, &line[ix], INT_BCOFFSET, calcINT_BCOFFSET )!=0) {
    goto ERRfatal; };
  } else {
    sprintf(emsg,"%s unknown", parname); goto ERRlineignore;
  };
  CONT: params++; continue;
  ERRlineignore:
  sprintf(emsg, "ctp.cfg line ignored:%s\n %s",line,emsg); 
  prtWarning(emsg);
};
RET: 
setINTLUT(3,"0x0");
setINTLUT(3+3,"0x0");
fclose(cfgfile); return(grc);
ERRfatal:
sprintf(emsg,"Fatal error when processing ctp.cfg:%s\n", emsg);
prtError(emsg); grc=1;
goto RET;
#endif
}
