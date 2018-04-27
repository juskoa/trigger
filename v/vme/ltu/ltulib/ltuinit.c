#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "vmewrap.h"
#include "lexan.h"
#include "vmeblib.h"
#include "ltu.h"

extern char BoardBaseAddress[];
void  printltuDefaultsMem(Tltucfg *ltc);

/*---------------------------------------------------------------------
rc: 0: ok  1:error (printed to stdout)
*/
int procInteger(char *opt, char *optarg, int *value, int minval, int maxval) {
int a,rc=0;
if (!strcmp(optarg,"PRINTVALUE")) {
/* getOption option: */
  printf("%d\n", *value);
} else {
  if (sscanf(optarg,"%d ",&a)!=1) {
    printf("Error: Wrong %s option:%s (expected dec. number)\n",opt,optarg); 
    rc=1;
  } else {
    if( (a<minval) || (a>maxval)) {
      printf("Error: %d out of range(%d..%d) for %s:\n",a,minval,maxval,opt); 
      rc=1;
    } else {
      *value=a;
      //printf("OK\n");
    };
  };
};
return(rc);
}

/*---------------------------------------------------------------------
rc: 0: ok  1:error (printed to stdout)
*/
int procHexa(char *opt, char *optarg, w32 *value, w32 minval, w32 maxval) {
w32 a; int rc=0;
if (!strcmp(optarg,"PRINTVALUE")) {
/* getOption option: */
  printf("0x%x\n", *value);
} else {
  if((strncmp(optarg,"0x",2)!=0) && (strncmp(optarg,"0X",2)!=0)){
    printf("Error: Wrong %s option:%s (expected hex. number 0xabc)\n",
      opt,optarg); rc=1;
  } else if (sscanf(&optarg[2],"%x ",&a)!=1) {
    printf("Error: Wrong %s option:%s (expected hex. number 0xabc)\n",
      opt,optarg); rc=1;
  } else {
    if( (a<minval) || (a>maxval)) {
      printf("Error: %x out of range(%x..%x) for %s:\n",a,minval,maxval,opt); 
      rc=1;
    } else {
      *value=a;
      //printf("OK\n");
    };
  };
};
return(rc);
}
/*---------------------------------------------------------------------*/
int procFloat(char *opt, char *optarg,float *value,float minval,float maxval) {
float a; int rc=0;
if (!strcmp(optarg,"PRINTVALUE")) {
/* getOption option: */
  printf("%f\n", *value);
} else {
  if (sscanf(optarg,"%f ",&a)!=1) {
    printf("Error: Wrong %s option:%s (float expected)\n",opt,optarg); 
    rc=1;
  } else {
    if( (a<minval) || (a>maxval)) {
      printf("Error: %f out of range(%f..%f) for %s:\n",a,minval,maxval,opt); 
      rc=1;
    } else {
      *value=a;
      //printf("OK\n");
    };
  };
};
return(rc);
}

typedef struct {
  char argval[32];   // last one is: ""
  int intval;        // corresponding int value
} Titems;
Titems mode_options[]={
{"BC", 3}, {"PULSER_EDGE", 5}, {"PULSER_LEVEL", 1},
{"RANDOM", 2}, {"SW", 0}, {"UNDEFINED", 0}, {"", 0}};
Titems L0_options[]={{"TTC",8}, {"CABLE", 0}, {"", 0}};
Titems SODEOD_options[]={{"YES",1}, {"NO", 0}, {"1", 1}, {"0", 0}, {"", 0}};
Titems LHCGAPVETO_options[]={{"YES",8}, {"NO", 0}, {"1", 8}, {"0", 0}, {"", 0}};
Titems DIM_options[]={{"YES",1}, {"NO", 0}, {"1", 1}, {"0", 0}, {"", 0}};
Titems TTCRX_RESET_options[]={{"YES",1}, {"NO", 0}, {"1", 1}, {"0", 0}, 
  {"INIT", 2}, {"STDALONE", 4}, {"", 0}};
Titems LOG1SEC_options[]={{"YES",1}, {"NO", 0}, {"1", 1}, {"0", 0}};
Titems flags_options[]={{"YES",1}, {"NO", 0}, {"1", 1}, {"0", 0}, {"", 0}};

/*---------------------------------------------------------------------*/
int procNamedInt(char *opt, char *optarg, int *value, Titems *table) {
int ix=0,rc=0;
while(1) {
  /*if (!strcmp("UNDEFINED",optarg)) 
    printf("MODE %s\n, doing nothing",optarg); */
  if (!strcmp(optarg,"PRINTVALUE")) {
    if (table[ix].intval == *value ) {
      printf("%s\n", table[ix].argval); 
      return(rc);
    };
  } else {
    if (!strcmp(table[ix].argval,optarg)) {
      *value= table[ix].intval; 
      //printf("OK\n");
      return(rc);
    };
  };
  if(table[ix].argval[0]=='\0') break;
  ix++;
};
rc=1;
if (!strcmp(optarg,"PRINTVALUE")) {
  printf("Error: attempt to set %s to: %d (not set)\n",opt, *value);
} else {
  printf("Error: Unknown MODE %s for %s\n",optarg, opt); 
};
return(rc);
}

/*-------------------------------------------------------- procFlag()
rc: 0:ok
*/
int procFlag(char *opt, char *optarg, w32 *flags, int bitn) {
int rc=0, ret, flag, bitinflag;
if((bitn>=0) && (bitn<=31)) {
  bitinflag= 1<<bitn;
  if (strcmp(optarg,"PRINTVALUE")==0) {
    if((*flags) & bitinflag) {
      printf("YES\n");    
    } else {
      printf("NO\n");    
    };
    goto RET;
  };
  ret= procNamedInt(opt, optarg, &flag, flags_options);
    if(ret!=0) {rc=ret; goto RET; };
    if(flag==0) {
      *flags= (*flags) & (~bitinflag);
    } else {
      *flags= (*flags) | bitinflag;
    };
} else {
  if (strcmp(optarg,"PRINTVALUE")==0) {
    printf("INTERR\n");
  }; rc=3;
};
RET: return(rc);
}

/*----------------------------------------------------------- setOptionMem */
int setOptionMem(char *opt_anycase, char *optarg_anycase, Tltucfg *ltucfg) {
/*Input:
opt_anycase   -variable name (will be converted to upper case before search)
optarg_anycase-variable value or
              -"PRINTVALUE"    -value is printed to stdout
ltucfg        -where is it set
Operation:
1. set ltucfg shared memory table or
2. get value from ltucfg shared memory table

Output:
----------------------- set
<0>    in case of no error, or

Error: error message
<1>    in case of error
----------------------- get (PRINTVALUE)
VALUE
<0>    in case of no error and optarg_anycase is PRINTVALUE
*/
int rc=0;   // 0: ok, 1: error
char opt[80];
char optarg[80]="";
strcpy(opt, opt_anycase); UPPER (opt);
if(optarg_anycase != NULL) {
  strcpy(optarg, optarg_anycase); UPPER (optarg);
};
//printf("setOption:%s:%s\n", opt, optarg);
if (!strcmp("MODE",opt)) {
  rc= procNamedInt(opt, optarg, &ltucfg->ltu_autostart_signal, mode_options);
} else if (!strcmp("ADDRESS",opt)) {
  int a;
  if (!strcmp("PRINTVALUE",optarg)) {
    printf("%s\n", BoardBaseAddress );
  } else {
    if (sscanf(optarg,"%X ",&a)!=1) {
      printf("Wrong address %s\n",optarg); rc=1;	    
    } else {
      strncpy(BoardBaseAddress,optarg,40);
    };
  }
} else if (!strcmp("RATE_LIMIT",opt)) {
  rc= procHexa(opt, optarg, &ltucfg->plist[IXrate_limit], 0, 0xffffffff);
} else if (!strcmp("LHCGAPVETO",opt)) {
  rc= procNamedInt(opt, optarg, &ltucfg->ltu_LHCGAPVETO, LHCGAPVETO_options);
} else if (!strcmp("BCRATE",opt)) {
  rc= procFloat(opt, optarg, &ltucfg->ltu_event_rate, 0.02, 40000000.);
} else if (!strcmp("ORBITBC",opt)) {
  rc= procInteger(opt, optarg, &ltucfg->orbitbc, 0, 3564);
} else if (!strcmp("BC_DELAY_ADD",opt)) {
  rc= procInteger(opt, optarg, &ltucfg->bc_delay_add, 0, 31);
} else if (!strcmp("BUSY",opt)) {
  rc= procInteger(opt, optarg, &ltucfg->busy, 0,3);
} else if (!strcmp("PP_TIME",opt)) {
  int pptime;
  if (!strcmp("PRINTVALUE",optarg)) {
    // check if they are the same:
    pptime= ltucfg->plist[IXGpp_time];
    if(pptime != w32toint(ltucfg->plist[IXSpp_time])) {
      printf("Error: PP_TIMEg:%d PP_TIMEs:%d differ (should be the same)\n",
        pptime, ltucfg->plist[IXSpp_time]); 
      rc=1;
    } else {
      printf("%d\n", pptime); rc=0;
    };
  } else {
    rc= procInteger(opt, optarg, &pptime, 0, 3564);
    if(rc==0) {
      ltucfg->plist[IXGpp_time]= pptime;
      ltucfg->plist[IXSpp_time]= pptime;
    };
  };
} else if (!strcmp("PPDELAY",opt)) {
  rc= procInteger(opt, optarg, &ltucfg->ppdelay, 0, 3564);
} else if (!strcmp("GPPDELAY",opt)) {
  rc= procInteger(opt, optarg, &ltucfg->Gppdelay, 0, 3564);
} else if (!strcmp("SBGO0DELAY",opt)) {
  rc= procInteger(opt, optarg, &ltucfg->Sbgo0delay, 0, 3564);
} else if (!strcmp("GBGO0DELAY",opt)) {
  rc= procInteger(opt, optarg, &ltucfg->Gbgo0delay, 0, 3564);
} else if (!strcmp("FINEDELAY1",opt)) {
  rc= procInteger(opt, optarg, &ltucfg->FineDelay1,0, 24897);
} else if (!strcmp("FINEDELAY2",opt)) {
  rc= procInteger(opt, optarg, &ltucfg->FineDelay2,0, 24897);
} else if (!strcmp("COARSEDELAY",opt)) {
  rc= procInteger(opt, optarg, &ltucfg->CoarseDelay,0,255);
} else if (!strcmp("L1_FORMAT",opt)) {
  rc= procInteger(opt, optarg, &ltucfg->l1format,0,3);
} else if (!strcmp("L0",opt)) {
  rc= procNamedInt(opt, optarg, &ltucfg->L0, L0_options);
} else if (!strcmp("SODEOD",opt)) {
  rc= procNamedInt(opt, optarg, &ltucfg->ltu_sodeod_present, SODEOD_options);
} else if (!strcmp("TTCRX_RESET",opt)) {
  rc= procNamedInt(opt, optarg, &ltucfg->ttcrx_reset, TTCRX_RESET_options);
} else if (!strcmp("LOG1SEC",opt)) {
  rc= procNamedInt(opt, optarg, &ltucfg->ttcrx_reset, LOG1SEC_options);
} else if (!strcmp("DIM",opt)) {
  rc= procNamedInt(opt, optarg, &ltucfg->dim, DIM_options);
} else if (!strcmp("GLOBAL_CALIBRATION",opt)) {
  rc= procFlag(opt, optarg, &ltucfg->flags, FLGG_calibration);
} else if (!strcmp("LOGTIMESTAMP",opt)) {
  rc= procFlag(opt, optarg, &ltucfg->flags, FLGlogtimestamp);
} else if (!strcmp("EXTORBIT",opt)) {
  rc= procFlag(opt, optarg, &ltucfg->flags, FLGextorbit);
} else if (!strcmp("GLOBAL_CALIBRATION_RATE",opt)) {
  rc= procFloat(opt, optarg, &ltucfg->global_calibration_rate, 
    0.0, 100.0);
} else if (!strcmp("GLOBAL_CALIBRATION_ROC",opt)) {
  rc= procInteger(opt, optarg, (int *)&ltucfg->plist[IXG_calibration_roc], 
    0, 7);
} else if (!strcmp("CALIBRATION_BC",opt)) {
  rc= procInteger(opt, optarg, &ltucfg->calibbc, 0, 3556);
} // special cases:
  else if (!strcmp("NOSODEOD",opt)) {
  //this is special case for -nosodeod parameter passed as arg. to ltu_proxy
  ltucfg->ltu_sodeod_present=0;
} else if (!strcmp("NODIM",opt)) {
  //this is special case for -nodim parameter passed as arg. to ltu_proxy
  ltucfg->dim=0;
} else if (!strcmp("L2ASEQ",opt)) {
  int ix;
  if (!strcmp("PRINTVALUE",optarg)) {
    printf("\"%s\"\n", ltucfg->mainEmulationSequence );
  } else {
    ltucfg->mainEmulationSequence[0]='\0';
    if(optarg_anycase != NULL) {
      for(ix=0; ix<63; ix++) {
        char c;
        c= optarg_anycase[ix];
        if( c!='\0') {
          ltucfg->mainEmulationSequence[ix]=c;
          ltucfg->mainEmulationSequence[ix+1]='\0';
        };
      };
    };
    //printf("l2aseq:%s\n", &ltucfg->mainEmulationSequence);
  };
} else {
  printf("Error: Unknown option: %s\n",opt_anycase); rc=1;
};
//printf("setOption:%s %s:\n", opt_anycase, optarg_anycase);
return(rc);
}
/*--------------------------------------------------------- setOption() */
int setOption(char *opt_anycase, char *optarg_anycase) {
w32 sec,usec;
Tltucfg *ltucfg;
ltucfg= &ltushm->ltucfg;
GetMicSec(&sec,&usec);
ltucfg->plist[IXdefedit_id]= sec; // see ltu_proxy.c SMI_handle_command()
return(setOptionMem(opt_anycase, optarg_anycase, ltucfg));
}
/*--------------------------------------------------------- copyltucfg() */
#define  copymac(x)  dest->x= src->x
void copyltucfg(Tltucfg *dest, Tltucfg *src) {
int ix;
copymac(l1format);
copymac(ppdelay);
copymac(FineDelay1);
copymac(FineDelay2);
copymac(CoarseDelay);
copymac(ltu_LHCGAPVETO);
copymac(ltu_event_rate);
copymac(ltu_sodeod_present);
copymac(ltu_autostart_signal);
copymac(busy);
copymac(L0);
copymac(orbitbc);
copymac(dim);
copymac(bc_delay_add);
copymac(ttcrx_reset);
strcpy(dest->mainEmulationSequence, src->mainEmulationSequence);
copymac(Sbgo0delay);
copymac(Gbgo0delay);
copymac(calibbc);
copymac(Gppdelay);
copymac(flags);
copymac(global_calibration_rate);
for(ix=0; ix<MAXPARLIST; ix++) {
  dest->plist[ix]= src->plist[ix];
};
}
/*--------------------------------------------------------- ltuDefaults() 
Set shared memory values before loading values from database.
See ttcsubs.c ttcDefaults for TTC part*/
void ltuDefaults(Tltucfg *ltc) {
ltc->flags=0;
ltc->l1format=3;            
ltc->ltu_LHCGAPVETO=8;
ltc->ltu_event_rate=100.0;
ltc->ltu_sodeod_present=1;
ltc->ltu_autostart_signal=3;
ltc->busy=1;
ltc->L0=8;
if(ltuviyes) {
  ltc->orbitbc=3560;    //valid only in stdalone (L0_BCOFFSET-4)mod 3564
  // 2854 pptime was 3399 till 31.3.2011
  ltc->plist[IXGpp_time]= 2854; // 3479-PPBC. PPBC: required pp-l0 distance  
  ltc->plist[IXSpp_time]= 2854;  // 3479= 3556-77
  ltc->plist[IXGorbit_time]= 161;
  ltc->plist[IXSorbit_time]= 161;
  ltc->calibbc=3011;   //valid only in STDalone. See: ctplib/timingpars.c
                       // and Testclass in ctp expertsoftware
  ltc->plist[IXrate_limit]= 0;
} else {
  ltc->orbitbc=0x5b;
  ltc->calibbc=3011; // valid only in STDalone
};
ltc->dim=1;
//ltc->ppdelay=3125;   is in ttcDefaults
ltc->bc_delay_add=0;
strcpy(ltc->mainEmulationSequence, "L2a.seq");
ltc->ttcrx_reset=1;
ltc->global_calibration_rate=0.;
ltc->plist[IXG_calibration_roc]=0;
ltc->plist[IXdefedit_id]=0;
}
/*----------------------------------------------------- printltuDefaults() */
void  printltuDefaults() {
Tltucfg *ltc= &ltushm->ltucfg;
printltuDefaultsMem(ltc);
}
/*--------------------------------------------------------- readValFile()  {
int fd1ps;
if (access(FD1FILE,F_OK) == 0) {
  FILE *fd1file;
  char inpstr[80];
  fd1file= fopen(FD1FILE,"r");
  fgets(inpstr, 50, fd1file);
  fd1ps=atoi(inpstr);
  fclose(fd1file);
};
return(fd1ps);
}
*/
#define L1RTIMELEN 23
typedef struct {
  char detname[12];
  unsigned int l1rtime;     // [ns]
} Tl1rtime;

Tl1rtime L1RTIME[L1RTIMELEN]= { 
{"spd", 0}, {"sdd",7000}, {"ssd",8100}, {"tpc",7150}, {"trd",6750}, 
{"tof",6705}, {"hmpid",6835}, {"phos",7900}, {"cpv",7325}, {"pmd",13130}, 
{"muon_trk",14275}, {"muon_trg",7100}, {"fmd",8260}, {"t0",6525}, {"v0",0}, 
{"zdc",9200}, {"acorde",7025}, {"-",0}, {"emcal",8125}, {"daq",0},
{"-",0},{"ad",0}, {"-",0}};

/*--------------------------------------------------------- readltuttcdb() */
void readltuttcdb(Tltucfg *ltc) {
enum Ttokentype token;
char *vmecfdir; int ix; char detname[16]="";
FILE *cfgfile;
char fnpath[80]; char line[80]; char msg[200];
vmecfdir= getenv("VMEWORKDIR"); 
if(vmecfdir == NULL){
  printf("readltuttcdb ERROR: VMEWORKDIR environment variable not defined:\n");
  return;
};
/* Init parameters not accessible through ltuttc.cfg. Why here ?
The only way to get det name is from vmecfdir, e.g.:
/home/alice/trigger/v/phos
todo: fill ltc->plist[IXl1rtime] (0 default) according to CNTRRD/l1rusecs[] *1000
*/
for(ix=strlen(vmecfdir)-1; ix>0; ix--) {
  if(vmecfdir[ix]=='/') {
    strcpy(detname, &vmecfdir[ix+1]);
    break;
  };
};
ltc->plist[IXl1rtime]=0;
ltc->plist[IXecsnum]= 0xff;
if(strlen(detname)>0) {
  for(ix=0; ix<L1RTIMELEN; ix++) {
    if(strcmp(detname, L1RTIME[ix].detname) == 0) break;
  };
  if(ix>=L1RTIMELEN) {
    printf("L1Rtime set to 0 (unknown detector: %s)\n", detname);
  } else {
    ltc->plist[IXl1rtime]= L1RTIME[ix].l1rtime;
    ltc->plist[IXecsnum]= ix;
    printf("L1Rtime %s: %d ns\n", detname, ltc->plist[IXl1rtime]);
  }
} else {
  printf("L1Rtime set to 0, detname not known\n");
};
strcpy(fnpath, vmecfdir);
strcat(fnpath, "/CFG/ltu/ltuttc.cfg");
//printf("readltuttc(): fnpath:%s\n", fnpath);
if(detectfile(fnpath, 0) <= 0) {
  printf("readltuttcdb WARNING: $VMEWORKDIR/ltuttc.cfg not present\n");
  return;
};
cfgfile=fopen(fnpath,"r");
if(cfgfile == NULL){
  sprintf(msg, "readltuttcdb ERROR. fnpath:%s:\n", fnpath);
  perror(msg); perror(strerror(errno));
  return;
};
printf("\nReading %s settings from file:%s\n",detname, fnpath);
while(fgets(line, 80, cfgfile)){
  int ix;
  char name[80]; char value[80];
  //printf("%s ",line);
  if(line[0]=='#') continue;
  if(line[0]=='\n') continue;
  ix=0;
  if(line[0]=='!') ix=1;  // 'system' variable (not seen by users)
  token= nxtoken(line, name, &ix);
  if(token==tSYMNAME) {
    token= nxtoken(line, value, &ix);
    if((token==tHEXNUM) || (token==tINTNUM) || (token==tFLOAT) ||
       (token==tSYMNAME) || (token==tSTRING) ) {
      int rc;
      if(token==tSTRING) {
        char value2[80];
        strcpy(value2, value);
        value2[strlen(value2)-1]='\0';
        strcpy(value, &value2[1]);
      };
      //printf("readltuttcdb:%s %s %d\n",name,value, token);
      rc= setOption(name, value);
      if(rc!=0) {
        printf("Bad value %s for parameter %s in file %s\n",
          value, name, fnpath);
      };
    } else {
      printf("%s is not hex. or decimal number (after name %s) in file %s\n", 
        value, name, fnpath);
    };
  } else {
    printf("Bad symbolic name %s in file %s\n", name, fnpath);
  };
};
fclose(cfgfile);
}

/*-------------------------------------------------------------- fpgainit()
rc: 0 not LTU  (or cannot load FPGA)
   >0 LTU version. Should be: 0xf3 or 0x?  */
int fpgainit() {
w32 code,ver, rc=0;
#ifdef SIMVME
vmew32(CODE_ADD, 0x56);
vmew32(LTUVERSION_ADD, 0xb9);
vmew32(VERSION_ADD, 0xa8);   // a0 for ctp boards
#endif
code= 0xff&vmer32(CODE_ADD);
ver= 0xff&vmer32(LTUVERSION_ADD);
if(code==0x56) {
  if(ver==0xff) {
    /* LTU FPGA configuration, if not configured: */
    loadFPGA(0);
    ver= 0xff&vmer32(LTUVERSION_ADD);
    if(((ver&0xf0)==0xb0) ||
       (ver==0xc1)   // trigger input generator
      ) {
      rc=ver;
    } else {
      printf("Bad LTU version: %d (should be at least 0xb3!)\n", ver);
    };
  } else {
    rc=ver;
  };
  printf("LTUfpga:0x%x LTUsw:%s\n", rc, LTU_SW_VER);
} else {
  rc=0;
   printf("Incorrect base address or board. Board:0x%x expected:0x56 ver:0x%x\n",    code, ver);
};
return(rc);
}

/*--------------------------------------------------------- setBC_DELAY_ADD 
del: is value 2ns lower than 'edge' measured in ADC_scan
*/
void setBC_DELAY_ADD(int del) {
vmew32(BC_DELAY_ADD,del); 
//if(ltuvino) setTTCint(0xffffffff);
setTTCint(0xffffffff);
}
/*--------------------------------------------------------- setTTCint() */
void setTTCint(w32 ttcint) {
/* 
TTCvi and LTUvi version: this has to be called always after BC_DELAY_ADD change!
      input:
      ttcint: 0xffffffff -in case we want to change TTC_INTERFACE just
      because BC_DELAY_ADD was changed!*/
w32 ttin,del, newttin;
int prt=1;
char ttcorltu[8];
ttin= vmer32(TTC_INTERFACE); del= vmer32(BC_DELAY_ADD);
if(ttcint==0xffffffff) {prt=0; ttcint=ttin;};
if(ltuviyes) {
  strcpy(ttcorltu,"LTUvi");
  /*if(del<=8) { valid till 18.7.2008 
    newttin=0x2;
  } else if((del>=9) && (del<=20)) { 
    newttin=0x1;
  } else if((del>=21) && (del<=31)) { 
    newttin=0x2;*/
  /*if(del<=4) { valid till 22.7.2008
    newttin=0x2;
  } else if(((del>=5) && (del<=17)) || (del==31)) { 
    newttin=0x1;
  } else if((del>=18) && (del<=30)) { 
    newttin=0x2; 
  */
  if((vmer32(LTUVERSION_ADD)&0xff)<0xbb) {
    //following valid till 17.3.2015 (ltuver <=0xba)
    if(((del<=6) && (del>=0)) || ((del >=20) && (del<=31))) {
      newttin=0x2;   // chanA +BC/2
    } else if((del>=8) && (del<=18)) { 
      newttin=0x1;   // chanB +BC/2
    } else if(del==7 ) { 
      newttin=0x3;   // both TTC chans A/B +BC/2
    } else if(del== 19) { 
      newttin=0x0; 
    } else { newttin=0;
      printf("BC_DELAY_ADD out of the allowed range (0-31)\n");
    };
  } else {
    // following valid from 17.3.2015 (ltuver >=0xbb)
    if(((del<=7) && (del>=0)) || ((del >=21) && (del<=31))) {
      newttin=0x2;   // chanA +BC/2
    } else if((del>=9) && (del<=19)) { 
      newttin=0x1;   // chanB +BC/2
    } else if(del==8 ) { 
      newttin=0x3;   // both TTC chans A/B +BC/2
    } else if(del== 20) { 
      newttin=0x0; 
    } else { newttin=0;
      printf("BC_DELAY_ADD out of the allowed range (0-31)\n");
    };
  };
} else {   // TTCvi
  strcpy(ttcorltu,"TTCvi");
  if( (del<=1) || ((del>=20) && (del<=27)) ) { newttin=0;
  } else if( (del>=15) && (del<=19) ) { newttin=1;
  } else if( (del>=28) || ((del>=2) && (del<=8)) ) { newttin=2;
  } else if( (del>=9) && (del<=14) ) { newttin=3;
  } else { newttin=0;
    printf("BC_DELAY_ADD out of the allowed range (0-31)\n");
  };
};
newttin= newttin | (ttcint & 0xc);
vmew32(TTC_INTERFACE, newttin);
if(prt==1) {
  printf("BC_DELAY_ADD:%d ns. TTC_INTERFACE before:0x%x now:0x%x (%s)\n",
    del,ttin,newttin, ttcorltu);
};
}

/*--------------------------------------------------------- ltuInit() 
Here are the necessary steps to add new parameter in ltuttc.cfg:
- modify:
  ltu/ltu.h/Tltucfg structure (shared memory holding the value) -> calibbc
  ltulib/ltuinit.c/setOption()        -convert TEXT-> sahred memory value
                  /ltuDefaults()      -default (if not given in ltuttc.cfg)
                  /printltuDefaults() -show current defaults
                  /ltuInit()          -load value to the hardware
- compile/restart: vmecomp ltu; cd ltu_proxy ; make and restart ltu_proxy...

*/
void ltuInit(Tltucfg *ltc, int stdalone, int secs) {
w32 b2;
Gltuver= fpgainit();
vmew32(SOFT_RESET, DUMMYVAL); /* clear L1_FIFO (and consequently BUSYout) */
if( ltc->flags & FLGextorbit){
  b2=1;
} else {
  b2=0;
};
printf("Switching to STANDALONE mode...(ext orbit:%d)\n", b2); 
setstdalonemode((b2<<1) | 0x1);
printf("SETTING BUSY1,BUSY2\n"); setBUSY(ltc->busy);
printf("Setting LAST_BC, PREPULSE_BC, CALIBRATION_BC, GAP_BC, L1/2_DEALY\n");
vmew32(LAST_BC,3563);
vmew32(PREPULSE_BC,128);
vmew32(CALIBRATION_BC,ltc->calibbc);
vmew32(GAP_BC,3446);
//vmew32(L1_DELAY,223);   // TL1-2 from 4.6.2008 (should give 224 ns L0-L1 on LTU)
//vmew32(L1_DELAY,259);   // TL1-1 (see ctplib/timingpars.c) 224
vmew32(L1_DELAY,279);   // from 16.3.2015
//vmew32(L2_DELAY,3520);  till 11.6.2008 (see ctplib/timigpars.c)
//vmew32(L2_DELAY,3952);  till 29.8.2011
vmew32(L2_DELAY,4274);  // from 16.3.2015
/* With run2 ctp+ltu firmwares, equal L1-L2 delay (global == stdalone) 
reached when:
ltu L2_DELAY modified from 4208 -> 810 and
ctp L2_DELAY_L1 modified from   3884 -> 500
*/
/* following overwritten from file CFG/ltu/ltuttc.cfg:*/
vmew32(ORBIT_BC,ltc->orbitbc);
vmew32(L1_FORMAT,ltc->l1format);
vmew32(BC_DELAY_ADD,ltc->bc_delay_add); setTTCint(ltc->L0);
vmew32(RATE_LIMIT, ltc->plist[IXrate_limit]);
printf(
  "ORBIT_BC:%d L1_FORMAT:%d BC_DELAY_ADD:%d cable/TTC:%d RATE_LIMIT:0x%x\n", 
  ltc->orbitbc, ltc->l1format, ltc->bc_delay_add, ltc->L0, 
  ltc->plist[IXrate_limit]);
if(ltuviyes) {
  if(stdalone>0) {
    vmew32(PP_TIME, ltc->plist[IXSpp_time]);
    vmew32(ORBIT_TIME, ltc->plist[IXSorbit_time]);
  } else {
    vmew32(PP_TIME, ltc->plist[IXGpp_time]);
    vmew32(ORBIT_TIME, ltc->plist[IXGorbit_time]);
  };
//} else {
};
printf("setting normal mode (no toggle), write to TTCvi...\n"); 
vmew32(TIMING_TEST, 0); vmew32(MASTER_MODE, 0);
printf("Disabling Error generation circuit...\n");
  ERenadis(0);
printf("Setting Error selector:0 (Pre-pulse), ERdemand:0 (no error on demand)...\n");
ERsetselector(0); ERdemand(0);
printf("LHC Gap Veto ON, Automatic START signal: OFF...\n"); SLMsetstart(0x8);/*
printf("LHC Gap Veto OFF, Automatic START signal: OFF...\n"); SLMsetstart(0); */
printf("Quitting emulation...\n"); SLMquit();
printf("Emulation status:%x\n", vmer32(EMU_STATUS));
printf("Clearing Emulator pipeline...\n"); vmew32(PIPELINE_CLEAR, DUMMYVAL);
printf("Disabling BACKPLANE and Logic analyser outputs...\n"); vmew32(BACKPLANE_EN, 0);
printf("Setting Scope A/B outs to 'not selected'...\n"); setAB(23,23);
printf("Initialising Snapshot memory to mode vme read...\n"); SSMsetom(0);
/* Counters should be cleared after setting TTCinterface (setTTCint)
 * to avoid spurious L1 in L1_COUNTER */
//printf("Clearing counters, including FIFO_MAX counter...\n"); 
//clearCounters();
vmew32(L1MAX_CLEAR, DUMMYVAL); vmew32(L2MAX_CLEAR, DUMMYVAL);
if(ltc->ttcrx_reset==0) {
  char msg[100];
  sprintf(msg, "TTCinit() suppressed (by option ttcrx_reset)");
  printf("Warning:%s\n",msg); //infolog(LOG_INFO, msg);
} else {
  TTCinitgs(stdalone, secs, ltc);
};
}
int getgltuver() {
return(Gltuver);
}
/*
rc: >0 in case of SEU was registered.
rc:  0 ok, no SEU registered from last checkSEU() activation
*/
int checkSEU() {
w32 serial, pll;
pll= vmer32(TEMP_STATUS)&0x2;
serial= 0x7f&vmer32(SERIAL_NUMBER);   // 0x7f for ltu2 (bit 0x40 is 0 for ltu1)
if((serial>=64) || (serial==0)) {
  if(serial==0) {
    printf("Serial number:0 ->this board accounted LTU version 2\n");
  };
  // check only from fpgaver 0xb8
  if(Gltuver>=0xb8) {
    if(pll==0x2) {
      printf("CRC error bit set, reconfigure LTU!\n");
      vmew32(TEMP_STATUS, 0);    // is needed here?
      return(1);
    } else {
      printf("CRC bit ok\n");
    };
  } else {
    printf("CRC bit not checked (LTU version 2, but FPGAver <0xb8 )\n");
  };
} else {
  printf("CRC bit not checked (LTU version 1, manufactured before 2012)\n");
};
return(0);
}

