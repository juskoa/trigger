/* cmdbase.c
cc -g cmdbase.c -o vmb
10.7.3003
- indirect parameters in func. calls
- # of pars for func. call checked
- the release temporary vars at the end of doexpr()
11.7.
- logfile variable added
- comment (#...) now allowed
20.7.
- macros (without parameters, .mac files are macros
28.8.
- Error: 'wrong # of parameter...' in function call now breaks
  line execution (before: more errors issued, due to the attempt
  to process the rest of line)
4.11.
- vmer/w16, vmer/w8 added (flags flsVME16 flsVME8)
15.11.
- init_allnames i.e. default functions now directly here
8.12.
- "uninitialized parameter" warning added
26.2.
- BoardBaseAddress is char * ("0x1000", "1000" or "VXI0::450")
3.11.
- hex2ui added (AIX MapVME was called with incorrect base addr.)
10.11.   VMERCC support added
todo:
done - base address as program parameter (now fixed through 
  #define DefaultBaseAddress
done - length of the mapped memory as parameters too
- macro parameters
- signal() -works only for AIX,linux
16.11.
Intention: vme emulation (see vmesim.c,vmesim.h)
 - first SIMVME (conditional compilation)
 - then dynamically (the same code used as for SIMVME)
3.12.
BoardSpaceLength added in board.c file (and in BOARD header of .cf file)
5.12.
-if VMEADDR in expression, address is taken (before: value resulting
   from vmeread was taken
-strings improved
7.12.
- now vme calls are in separate source file (vmwrap.c, vmewrap.h)
13.12. initmain() added
21. 1.2004 vmeopmr32(n, ...) with stdarg added -at least 1 parameter
     But it seems it is not any help ...
22. 1.
vmeoprX() -now return value (before ret. value was void), due
     to macros -now assignment of VME value to variable possible
18.2. vmeish() -SIGBUS added in gotsignal() see vmewrap.c
19.2.2004 tSTRING can be parameter of the function
20.2. -bug fixed. 'Segmentation fault when closing " mising for 
       string constant, string func. parameters are checked
25.2. -bug fixed (incorrect processing of hexa-numbers in semantic,
       A,...,F was not processed correctly
10.3. -now non-void functions (int, char, float w*) result is
       printed to stdout: w*: <0x%x> int:<%d> char:<%c> float:<%f>
28.5. BoardName added. board.exe is started in $VMECFDIR directory
      (...v/vme/). Macros are in $VMECFDIR/CFG/board directory
23.6. init.mac now executed after boardInit() directly
28.6. STRING constants, if not assigned to variable, are deleted.
      Modified:
      semantic() -STRING const. is always tagged with flsTEMP
      apply2()   -arg2's flag flsTEMP is removed if string assignment (2x)
      ?names     -empty items are not printed (?dbg prints them)
20.8. micwait() added 
29.8. corrections for WIN32 (cygwin):
      - '\r' everywhere with '\n'
      - ? name now better
      #define linux or WIN32
26.8. micwait() moved to vmewrap.c, should be calibrated (i.e. called
      once at the start, before real use)
17.5. EMPTYSTRING added ("" was incorrectly processed)
14.9.2005
    tCHAR works now i.e. func('A')
15.11.2005 bug fixed (introduced 14.9) tokenSymbol,opPriorities,Ttokentype
    should be consistent (they were not after adding tCHAR to
    Ttokentype only)
21.2.2006 Bug: a=1  or a="1" is adding to allnames[] item named 1. 
    Correction: 
    - strings are added with surrounding "".
    - long string (>MAXNAMELENGTH) are added with the name _intLongString
    - MAXLILE changed to 4000 (was 81)
11/10/06 -noprompt argument added
23/10/06 -now Address modifier allowed with VMERCC/VMECCT
*/
/*#define linux  */
/* #define cygwin */
/* #define WIN32 */

#define TRUE 1
#define FALSE 0
#include "vmewrap.h"
#include "lexan.h"
#include "vmeaistd.h"

/* for dbg prints:*/
/*von #define MAXPRINT 100 */

#ifdef WIN32
  #include <stdio.h>
  #include <signal.h>
  #include <process.h>
#endif
#if defined(linux) || defined(AIX) || defined(cygwin)
  #include <stdio.h>
  /* #include <stdlib.h> */
  #include <unistd.h>
  #include <fcntl.h>
  #include <signal.h>
#endif 

#include <stdlib.h>
#include <stdarg.h>
#include <string.h> 

extern char BoardName[];
extern char BoardBaseAddress[];
extern char BoardSpaceLength[];
extern char BoardSpaceAddmod[];
/*char BoardBaseAddress[40]="VXI0::450"; */
/* #define FILTEST 32766 */

int quit=0;
#define DBGstacks 0x0001
#define DBGlex 0x0002
int DBG= 0; /*DBGstacks; */

#define MAXFUNPARS 10
#define MAXLILE 4000

/* variables:
 -user defined
 -temporary (when evaluating exps)
 -cmdbase defined
*/

/* following 2 arrays have to be consistent with Ttokentype ! (in lexan.h)*/
const char *tokenSymbol[]={"NONE", "ERROR", "SYMNAME", "STRING", "INTNUM", "HEXNUM",
                     "VAR", "FUN", "VMEADR", "CHAR", "EOCMD", "FLOATNUM",
                     "=", "==", "+", "-", "*", "/", "(", ")", "[", "]", ","};
int opPriorities[]={0, 0, 0, 0, 0, 0,   /* HEXNUM */
                    0, 0, 0, 0,
                    100, 0, /*EOCMD, FLOATNUM */
                    100, 0, 110, 110, 120, 120,
                    100, 100, 0, 0, 0};

/*----------------------------------------- flags in Tpardesc.fls: */
/* low 8 bits type of the paramater: 1 -int 2 -w8,w16,w32 3 -char 
 * see comp.py */
#define parINDIRECTPAR 0x80000000
#define flsVARIABLENP1 0x40000000   /* var. # of pars, at least 1 required*/ 
#define parMASK 0xff
#define parINT 0x1
#define parW81632 0x2
#define parCHAR 0x3
#define parFLOAT 0x4   /* from 6.2.2011 */
/*------------------------------ end of flags in Tpardesc.fls: */

/*------------------------------- flags in Tname.fls (vmeaistd.h): */
/* 8 bit for type (e.g. tVAR,...) */
#define flsTTM  0x000000FF
/* 0xF00 bits: result type of the function */
#define flsFMSK 0xF00
#define flsFw81632 0x100
#define flsFint    0x200
#define flsFfloat  0x300
#define flsFvoid   0x400
#define flsFchar   0x500
/* float variable */
#define flsFLOAT    0x80000000
/* indirect tVAR (Tname.intvar is pointer to value */
#define flsINDIRECT 0x40000000
/* temporary variable (when computing expression) */
#define flsTEMP     0x20000000
/* constant (name is asci representation of the constant */
#define flsCONS     0x10000000
/* variable is string (intvar is pointer to string "string" 
string constant: intvar points to name, which is "name"
string var:      intvar points to string constant name
*/
#define flsSTRING   0x08000000
#define flsVME16    0x04000000
#define flsVME8     0x02000000
/* undefined variable: */
#define flsUNDEF    0x01000000

/* operators, arguments stacks: */
#define MAXnops 5
#define MAXnargs 5
int nops, nargs;   /* # of operators, args in corresponding stack */
enum Ttokentype opstack[MAXnops];
Tname *argstack[MAXnargs];

extern int nnames;
extern Tname allnames[];

/* macros */
typedef struct {
  FILE *macfile;
  int nmp; 
  char macpars[MAXFUNPARS][MAXNAMELENGTH];
} Tactmac;
Tactmac actmac1;
Tactmac *actmac= NULL;

void initmain();
void endmain();
void boardInit();
void printop();
void printarg();
w32 getFlags(Tname *ix);
w32 getValueName(Tname *ix);
void printTname(Tname *arg, int printnice);
Tname *findName(char *name); 
Tname *addName(const char *name, w32 fls);
enum Ttokentype typeName(Tname *ix);
enum Ttokentype nxtoken(char *line, char *cmd, int *ix);

/*----------------------------------------------*/ void printHelp(char *line) {
int iline=0, longout=1;
char namehelp[80]="";
Tname *nm;
if( line[1] != '\n' && line[1]!= '\r') {
  if(line[1] == '?' ) {
    longout=1; iline=2;
  }else {
    longout=0; iline=1;
  };
  /* skip spaces: */
  while (line[iline] == ' ' || line[iline] == '\t' ) iline++;
  line[strlen(line)-1]='\0';
  /*printf("dbgprintHelp:=%s=\n",line); */
  if( strncmp(&line[iline], "syntax", 6) == 0) {
    printf("\
Following operands can be used:\n\
\"abc\" 0xabcd 234 val1   -string, hex. const., decimal const., variable\n\
?name                 -print name value\n\
??name                -print info about name\n\
q                     -exit command line interface\n\
funcname(par1,par2)   -function call (possible parameters:\n\
	               variable, *variable, string variable,\n\
		       string constant or constant\n\
                       )\n\
var1+var2             -expression\n\
# comment             -comment up to end of line\n\
var= var1+var3*var1   -assignment expression (ops: +*-/)\n\
loops= 10             -set number of repetitions for any command\n\
logfile=\"name.log\"  -start log (only input statements logged)\n\
macroname(par1,par2,...) -execute macro saved in file macroname.mac\n\
                       $N symbols in macro are replaced with parN during\n\
                       macro execution(pars not implemented yet)\n\
VME operations:\n\
VMEADR=var1           -write to VMEADR (VMEADR was defined in .cf file)\n\
a=vmeoprX(vmeadd)     -vme read (X= 8 | 16 | 32)\n\
vmeopwX(vmeadd,value) -vme write (vmeadd: constant or variable)\n\
"); goto OKRET;
  };
  if( strncmp(&line[iline], "dbg", 3) == 0) {
    int ixn;
    printf("flags     type      symb. name       value(dec)   #pars/vmeadr\n"); 
    for(ixn=0; ixn<nnames; ixn++) {
      printTname(&allnames[ixn], 0);
      printf("\n");
    };
    goto OKRET;
  };
  if( strncmp(&line[iline], "names", 5) == 0) {
    int ixn;
    printf("type      symb. name         value(dec)       note\n"); 
    for(ixn=0; ixn<nnames; ixn++) {
      if(allnames[ixn].name[0]=='\0') continue; /* free item */
      printTname(&allnames[ixn], 1);
      printf("\n");
    };
    goto OKRET;
  } else {
    enum Ttokentype t;
    int ix=iline;
    t= nxtoken(line, namehelp, &ix);
  };
}; 
/* printf("namehelp:%s\n", namehelp); */
nm= findName(namehelp);/* nm= findName(&line[iline]); */
if(nm!= NULL) {
  if(longout==1) {
    printTname(nm,1);
    printf("\n");
  }else{
    w32 val;
    val=getValueName(nm);
    if(getFlags(nm) & flsSTRING) {
      printf("%s\n",(char *)val);
    } else {
      printf("%d\n",val);
    };
  };
} else {
  if(longout==1) {
  printf(
"\
 ? name   -print name attributes (function, vme var, var...)\n\
 ? syntax -command line syntax help\n\
 ? names  -show names\n\
 ? dbg\n"
);
  } else {
    printf("<UNDEFINED>\n");
  };
};
OKRET: return;
}

/*------------------------------------------*/ void prerr(const char *msg) {
printf("%s\n", msg);
}
/*------------------------------------------*/ void errexit(char *msg) {
prerr(msg);
printop(); printarg();
exit(8);
}
/*------------------------------------------*/ void prerr1(char *msg, int i) {
printf("%s %d\n", msg, i);
}
/*----------------------------*/ void prerr2(char *line, int erp, char *msg) {
int i;
printf("%s", line);
for(i=0; i<erp; i++) printf("%c",' ');
printf("A--- %s\n", msg);
}

/*----------------------------------------------*/ void add2allnames(
  const char *name, w32 fls, 
  w32 (*fp)(w32,w32,w32,w32,w32,w32,w32,w32,w32,w32), 
  w32 intvar, 
  Tpardesc *pardesc, w32 vmenp, char *usage) {
Tname *p;
p= addName(name, fls);
p->fp= fp;
p->intvar= intvar; 
p->pardesc= pardesc;
p->vmenp= vmenp;
p->usage= usage;
/*return(p);*/
}

Tpardesc vmeopr8_parameters[1]={{"adr", 0}}; 
Tpardesc vmeopr16_parameters[1]={{"adr", 0}};
Tpardesc vmeopr32_parameters[1]={{"adr", 0}};
Tpardesc vmeopmr32_parameters[1]={{"count", flsVARIABLENP1}};
Tpardesc vmeopw8_parameters[2]={{"adr", 0}, {"val", 0}};
Tpardesc vmeopw16_parameters[2]={{"adr", 0}, {"val", 0}};
Tpardesc vmeopw32_parameters[2]={{"adr", 0}, {"val", 0}};

w8 vmeopr8(w32 adr) {
  w8 v=vmer8(adr);printf("0x%x\n",v);return(v); }
w16 vmeopr16(w32 adr) {
  w16 v=vmer16(adr);printf("0x%x\n",v);return(v); }
w32 vmeopr32(w32 adr) { 
  w32 v=vmer32(adr);printf("0x%x\n",v);return(v); }
w32 hexa(char *hn) {
  int i=0; w32 var4=0;
  char c;
  while( (c=hn[i]) != '\0') {
    w32 ic;
    if( c>='0' && c<='9') {
      ic= c-'0';
    } else if( c>='a' && c<='f') {
      ic= c-'a'+ 10;
    } else if( c>='A' && c<='F') {
      ic= c-'A'+ 10;
    } else {
      char errmsg[100];
      sprintf(errmsg,"wrong hexa-number:%s.",hn);
      prerr(errmsg); break;
    };
    /*var4= (var4<<4) | ic; */
    var4= var4*16 + ic;
    i++;
  };
return(var4);
}
void vmeopmr32(int n) {
int i;
w32 ad;
#define MAXvmeout 12*100+1
char os[MAXvmeout]="";
  char line[33];
for(i=0;i<n; i++) {
  fgets(line,32,stdin);
  if(line[0]!='0' || line[1]!='x') {
    prerr((char *)"Incorrect hexadecimal number");
  };
#ifdef WIN32
  /* in cygwin \r\f instead of \n */
  line[strlen(line)-2]='\0'; 
#else
  line[strlen(line)-1]='\0'; 
#endif
  ad= hexa(&line[2]);
  /*printf("ad:%x",ad);  */
  sprintf(os,"%s0x%x ", os, vmer32(ad)); 
}; 
printf("%s\n",os);
}
/*------------ example of subroutine with variable # of args: 
void vmeopmr32(int n, ...) {
va_list argp;
int i;
#define MAXvmeout 12*MAXFUNPARS+1
char os[MAXvmeout]="";
va_start(argp, n);
for(i=0;i<n; i++) {
  w32 arg;
  arg= va_arg(argp, w32);
  sprintf(os,"%s0x%x ", os, vmer32(arg)); 
}; va_end(argp);
printf("%s\n",os);
}
---------------*/

void vmeopw8(w32 adr, w8 val) { vmew8(adr, val); }
void vmeopw16(w32 adr, w16 val) { vmew16(adr, val); }
void vmeopw32(w32 adr, w32 val) { vmew32(adr, val); }

/*----------------------------------------------*/ void init_allnames() {
add2allnames("vmeopr8", tFUN, (funcall)vmeopr8, 0xdead, vmeopr8_parameters, 1, NULL);
add2allnames("vmeopr16", tFUN, (funcall)vmeopr16, 0xdead, vmeopr16_parameters, 1, NULL);
add2allnames("vmeopr32", tFUN, (funcall)vmeopr32, 0xdead, vmeopr32_parameters, 1, NULL);
add2allnames("vmeopmr32", tFUN, (funcall)vmeopmr32, 0xdead, vmeopmr32_parameters, 1, NULL);
add2allnames("vmeopw8", tFUN, (funcall)vmeopw8, 0xdead, vmeopw8_parameters, 2, NULL);
add2allnames("vmeopw16", tFUN, (funcall)vmeopw16, 0xdead, vmeopw16_parameters, 2, NULL);
add2allnames("vmeopw32", tFUN, (funcall)vmeopw32, 0xdead, vmeopw32_parameters, 2, NULL);
add2allnames("cctopen", tFUN, (funcall)cctopen, 0xdead, NULL, 0, NULL);
add2allnames("cctclose", tFUN, (funcall)cctclose, 0xdead, NULL, 0, NULL);
add2allnames("logfile", tVAR | flsSTRING, NULL, (w32)"", NULL, 0, NULL);
add2allnames("EMPTYSTRING", tVAR | flsSTRING, NULL, (w32)"", NULL, 0, NULL);
/* add2allnames("init", tFUN, NULL, 0xdead, NULL, 0, NULL);*/
add2allnames("init", tVAR, NULL, (w32)"", NULL, 0, NULL);
}

/*======================================== allnames[] operations: */
/*----------------------------------------------*/ Tname *findName(char *name) {
int i;
if(name[0]=='\0') return(NULL);
for(i=0; i<nnames; i++) {
  if( strcmp(name, allnames[i].name) == 0) goto FOUND;
};
return(NULL);
FOUND: return(&allnames[i]);
}
/*----------------------------------------------*/ void delName(Tname *ix) {
if(ix->name[0]=='\0') return;   /* already deleted */
if( typeName(ix) == tSTRING) {
  if((char *)ix->intvar == NULL) {
    errexit((char *)"NULL pointer in tSTRING variable\n");
  };
  if(strlen((char *)ix->intvar)>3) {
    free((char *)ix->intvar);
    /*(char *)ix->intvar=NULL;   -fc5: invalid lvalue */
    ix->intvar=0;
  };
};
ix->name[0]= '\0';
}
/*-----------------------------------*/ void delNamefls(Tname *ix, w32 flg) {
/* delete all the entries with flg set */
if( (ix->fls & (flg & ~flsTTM)) != 0) {
  /*  ix->name[0]= '\0'; */
  delName(ix);
};
}
/*----------------------------------*/ Tname *addName(const char *name, w32 fls) {
int ix;
/* enum Ttokentype tt= fls & flsTTM; */
for(ix=0; ix<nnames; ix++) {
  if( allnames[ix].name[0] == '\0' ) goto EMPTYFOUND;
};
if(nnames<MAXNAMES){
  ix= nnames;
  nnames++;
} else {
  char msg[100];
  sprintf(msg, 
          "Too many symbols (>(MAXNAMES=%d),see MAXNAMES in vmeaistd.h)", 
          MAXNAMES);
  errexit(msg); /*return(NULL); */
};
EMPTYFOUND:
  strncpy(allnames[ix].name, name, MAXNAMELENGTH);
  if(strlen(name) >= MAXNAMELENGTH) {
    char msg[100];
    allnames[nnames].name[MAXNAMELENGTH-1]='\0';
    sprintf(msg,"%s -too long name", allnames[nnames].name);
    prerr(msg);
  };
  allnames[ix].fls= fls;
  allnames[ix].fp= NULL; allnames[ix].intvar= 0; 
  allnames[ix].floatvar= 0.0; 
  allnames[ix].pardesc= NULL;
  allnames[ix].vmenp= 0;
  allnames[ix].usage= NULL;
  return(&allnames[ix]);
}
/*-----------------*/ Tname *addNameIndirect(char *name, w32 fls, void *p) {
Tname *tp;
tp= addName(name, fls);
tp->intvar= (w32)p;
return(tp);
}

/*----------------------------------*/ enum Ttokentype typeName(Tname *ix) {
if( ix == NULL) return(tNONE);
return((enum Ttokentype)(ix->fls & flsTTM));
}
/*----------------------------------*/ w32 getFlags(Tname *ix) {
return(ix->fls);
};
/*----------------------------------*/ void setFlags(Tname *ix, w32 fls) {
ix->fls= fls;
};
/*----------------------------------*/ w32 getValueNameDbg(Tname *ix) {
if( ix->fls & flsINDIRECT ) {
  return(*(w32 *)ix->intvar);
} else {
  return(ix->intvar);
};
}
/*----------------------------------*/ w32 getValueName(Tname *ix) {
if( typeName(ix) == tVMEADR) {
  if( ix->fls & flsINDIRECT ) {
    w32 dummy=0xffffffff;
    prerr("internal error getValueName should not be called");
    ix->intvar= dummy;
    return(ix->intvar);
  } else {
    return(ix->vmenp);
  };
};
if( ix->fls & flsINDIRECT ) {
  return(*(w32 *)ix->intvar);
} else {
  return(ix->intvar);
};
}
/*----------------------------------*/ float getValueNameFloat(Tname *ix) {
return(ix->floatvar);
}
/*----------------------------------*/ char *getStringName(char *nm) {
Tname *ix;
char *ixrc=NULL;
ix=findName(nm);
if(ix==NULL) goto RET;
if( getFlags(ix)& flsSTRING ) {
  ixrc= (char *)ix->intvar;
};
RET:return ixrc;
}
/*----------------------------------*/ void setValueName(Tname *ix, w32 val) {
if( typeName(ix) == tVMEADR) {
  if (ix->fls & flsVME16) {
    w16 val16=val;
    vmew16(ix->vmenp, val16);
  } else if (ix->fls & flsVME8) vmew8(ix->vmenp, val);
  else vmew32(ix->vmenp, val);
};
if( ix->fls & flsINDIRECT ) {
  *(w32 *)ix->intvar= val;
} else {
  ix->intvar= val;
};
}
/*---------------------------*/ void setValueNameFloat(Tname *ix, float val) {
if( ix->fls & flsINDIRECT ) {
  prerr("attempt for indirect addressing with float (internal error)");
} else {
  ix->floatvar= val;
};
}
/*---------------------------*/ void copyValueName(Tname *ix, Tname *ixsrc) {
ix->intvar= ixsrc->intvar;
ix->floatvar= ixsrc->floatvar;
}
/*--------------------*/ void printTname(Tname *arg, int niceprint) {
w32 value;
enum Ttokentype tt;
/*printf("printTname:"); */
tt= (enum Ttokentype) (arg->fls & flsTTM);
if( arg == NULL ) {
  printf("NULL"); 
  return;
};
if( niceprint==1)
  printf("%8s:", tokenSymbol[tt]);
else
  printf("%8x(%8s):", arg->fls, tokenSymbol[tt]);
switch (tt) {
case tSYMNAME:
case tINTNUM:
case tHEXNUM:
case tVAR: case tSTRING: case tCHAR:
case tFUN:
case tVMEADR:
  value= getValueNameDbg(arg);
  if( arg->fls & flsSTRING ) {
    printf("%16s=%8x(%s) %x", arg->name, 
      *(w32 *)value,(char *)value, arg->vmenp);
  } else {
    printf("%16s=%8x(%8d)float:%f", arg->name, value,value, arg->floatvar);
    if(niceprint==1) {
      if(tt== tVMEADR) printf("  adr:%x",  arg->vmenp);
      if(tt== tFUN) printf("  # of pars:%d",  arg->vmenp);
      if(tt== tSYMNAME) printf("  base:%s",  (char *)arg->vmenp);
    } else {
      printf("   %x", arg->vmenp);
    };
  };
  break;
case tFLOAT:
  printf("%16s=%f()", arg->name, arg->floatvar);
  break;
default:
  printf("unknown type (%d), internal error",tt);
};
}

#ifdef MSVISAnotexist
usleep(int micsec) {
int i,j;
for(j=0; j<micsec; j++) {
  /* 100000 -> 1 micsec is cca 1 milisecond on 800MHz PentiumIII
	 i.e. 100 should be 1 micsec */
  for(i=1; i<100; i++) {if (i>0) continue;}; /* 1 micsec ??? */
};
}
#else

void vmeish();
void gotsignal(int signum) {
switch(signum) {
case SIGUSR1:
  signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
  printf("got SIGUSR1 signal:%d\n", signum);
  quit=signum; 
  break;
case SIGBUS:
/*  vmeish(); */
/*  printf("got SIGBUS signal:%d\n", signum); */
  break;
default:
  printf("got unknown signal:%d\n", signum);
};
}
#endif

/*------------------------------------*/ void fillrand(w32* arr, int arrsize){
int i;
w32 r;
for(i=0; i<arrsize; i++) {
  r= rand();
  arr[i]= (r<<16) | r;
};
}

/*-----------------------------------*/ int isop(enum Ttokentype oprtr) {
if( oprtr >= tASSIGN && oprtr < tLEFT )
  return(TRUE);
else
  return(FALSE);
}
/*------------------------------*/ int isunaryop(enum Ttokentype oprtr) {
if( oprtr == tPLUS || oprtr == tMINUS )
  return(TRUE);
else
  return(FALSE);
}
/*--------------------*/ Tname *semantic(enum Ttokentype arg, char *token) {
char c;
Tname *inx=NULL;
/*printf("semantic arg:%x token:%s.\n",arg,token); */
switch (arg) {
case tSTRING:
  if(token[0]=='\0') inx= findName((char *)"EMPTYSTRING");
  else inx= findName(token);
  if( inx == NULL ) {
    char *strname;
    char *strvalue;
    int strlng;
    strlng=strlen(token);
    if(strlng<MAXNAMELENGTH) {
      strname=token;
    } else {
      strname="_intLongString";
    };
    inx= addName(strname, tSTRING | flsCONS | flsSTRING| flsTEMP);  
    if((strlng-2)<4) {
      strvalue= (char *)(&inx->usage);
    }else{
      strvalue= (char *)malloc(strlng-1);
    };
    strncpy(strvalue,&token[1],strlng-2); strvalue[strlng-2]='\0';
    setValueName(inx, (w32)strvalue);
  };
/*  printf("semantic:\n"); printTname(inx,1); printf("\n");  */
  break;
case tSYMNAME:
  inx= findName(token);
  if( inx == NULL ) {
    inx= addName(token, tVAR | flsUNDEF);  
    setValueName(inx, 0xdeadbeaf);
    break;
  };
/*
  if( typeName(inx) == tFUN ) {
    
  };
*/
  break;
case tCHAR:{
  w32 var4=0;
  var4=token[0];
  if( (inx= findName(token)) == NULL) {
    inx= addName(token, tCHAR | flsCONS);
    setValueName(inx, var4);
  }else {
    if( var4 != getValueName(inx) ) {
      prerr("internal error in semantic() for tCHAR");
    };
  };
  break;
}
case tINTNUM:{
  int i=0; w32 var4=0;
  while( (c=token[i]) != '\0') {
    w32 ic;
    if( c>='0' && c<='9') {
      ic= c-'0';
      var4= 10*var4+ic; i++;
    };
  };
  if( (inx= findName(token)) == NULL) {
    inx= addName(token, tINTNUM | flsCONS);
    setValueName(inx, var4);
  }else {
    if( var4 != getValueName(inx) ) {
      prerr("internal error in semantic()");
    };
  };
  break;
}
case tFLOAT:{
  int i=0; w32 var4=0; float fvar4=0.; int id=10;
  c=token[i];
  //while( (c != '\0') && (c != '.') ){
  while( c>='0' && c<='9') { 
    w32 ic;
    ic= c-'0';
    var4= 10*var4+ic; i++; c=token[i];
  };
  if(c=='.') {
    i++; c=token[i];
    while( c>='0' && c<='9') { 
      w32 ic;
      ic= c-'0';
      fvar4= fvar4+ic*(1./id); id=id*10; i++; c=token[i];
    };
    fvar4= fvar4+var4;
  }else {
    char errmsg[100];
    sprintf(errmsg,"semantic(): wrong float-number (. expected):%s.",token);
    prerr(errmsg);
    break;
  };
  if( (inx= findName(token)) == NULL) {
    inx= addName(token, tFLOAT | flsCONS);
    setValueNameFloat(inx, fvar4);
  }else {
    if( fvar4 != getValueNameFloat(inx) ) {
      prerr("internal error in semantic()");
    };
  };
  break;
}
case tHEXNUM:{
  int i=0; w32 var4=0;
  while( (c=token[i]) != '\0') {
    w32 ic;
    if( c>='0' && c<='9') {
      ic= c-'0';
    } else if( c>='a' && c<='f') {
      ic= c-'a'+ 10;
    } else if( c>='A' && c<='F') {
      ic= c-'A'+ 10;
    } else {
      char errmsg[100];
      sprintf(errmsg,"semantic(): wrong hexa-number:%s.",token);
    };
    var4= (var4<<4) | ic;
    i++;
  };
  if( (inx= findName(token)) == NULL) {
    inx= addName(token, tHEXNUM | flsCONS);
    setValueName(inx, var4);
  } else {
    if( var4 != getValueName(inx) ) {
      prerr("internal error in semantic()");
    };
  };
  break;
}
default: prerr1("semantic(): unknown token", arg); break;
};
return(inx);
}
/*============================== operators stack */
/*--------------------*/ void pushop(enum Ttokentype oprtr) {
if( DBG & DBGstacks) {
  printf("DBG pushop():%s\n", tokenSymbol[oprtr]);
};
if( nops >= MAXnops) {
  errexit("Operator stack overflow");
} else {
  opstack[nops]= oprtr;
  nops++;
};
}
/*--------------------*/ enum Ttokentype popop() {
enum Ttokentype resop=tNONE;
if( nops == 0 ) {
  errexit("Operator stack underflow");
} else {
  nops--;
  resop= opstack[nops];
};
if( DBG & DBGstacks) {
  printf("DBG popop():%s\n", tokenSymbol[resop]);
};
return(resop);
}
/*--------------------*/ enum Ttokentype topop() {
if( nops == 0 ) return(tNONE);
return(opstack[nops-1]);
}
/*--------------------*/ int sizeop() {
return(nops);
}
/*--------------------*/ void printop() {
int i;
for(i=0; i<sizeop(); i++) {
  printf("%2d:%8s\n", i, tokenSymbol[opstack[i]]);
};
}
/*--------------------*/ int priority(enum Ttokentype oprtr) {
int respri;
respri= opPriorities[oprtr];
if( respri == 0) {
  prerr1("priority(): bad operator", oprtr);
};
return(respri);
}

/*============================== arguments stack */
/*--------------------*/ void pusharg(Tname *arg) {
if( DBG & DBGstacks) {
  printf("DBG pusharg():");
  printTname(arg,0); printf("\n");
};
if( nargs >= MAXnargs) {
  errexit("Argument stack overflow");
} else {
  argstack[nargs]= arg;
  nargs++;
};
}
/*--------------------*/ Tname *poparg() {
Tname *res=NULL;
if( nargs == 0 ) {
  errexit("Argument stack underflow");
} else {
  nargs--;
  res= argstack[nargs];
};
if( DBG & DBGstacks) {
  printf("DBG poparg():");
  printTname(res,0); printf("\n");
};
return(res);
}
/*--------------------*/ Tname *toparg() {
if( nargs == 0 ) return(NULL);
return(argstack[nargs-1]);
}
/*--------------------*/ int sizearg() {
return(nargs);
}
/*--------------------*/ void printarg() {
int i;
for(i=0; i<sizearg(); i++) {
  printf("%2d:",i);
  printTname(argstack[i],0); printf("\n");
};
}
/*--------------------*/ Tname *apply1(enum Ttokentype oprtr, Tname *arg) {
Tname *ix;
int unop=0;
ix= addName("_intvar1", tVAR | flsTEMP);
if( oprtr == tPLUS) unop= 1;
if( oprtr == tMINUS) unop= -1;
if(unop==0) prerr("unknown unary operator");
else setValueName(ix, unop * arg->intvar);
return(ix);
}
/*------*/ Tname *apply2(enum Ttokentype oprtr, Tname *arg1, Tname *arg2) {
Tname *ixresult= NULL;
enum Ttokentype targ1, targ2;
targ2= typeName(arg2); targ1= typeName(arg1);
switch (oprtr) {
case tASSIGN: 
  if( (getFlags(arg2) & flsSTRING)  && targ1==tVAR) { /* arg2 string */
    if(((getFlags(arg1) & flsSTRING)==0) &&           /* arg1 nostring */
        ( (getFlags(arg1) & flsUNDEF)==0)) {          /*      defined */
        prerr("attempt to assign string to non string variable");
    }else {     /* string-defined or nostring-undefined */
      setValueName(arg1, getValueName(arg2));
      setFlags(arg1, tVAR | flsSTRING);
      /* arg2 string is not temporary because arg1 points to it */
      setFlags(arg2, getFlags(arg2) & (~flsTEMP));
      ixresult= arg1;
    };
    break;
  };
  if( targ2==tVAR || targ2==tHEXNUM  || targ2==tINTNUM || targ2==tVMEADR) {
    if( targ1 == tVMEADR ) {                /* vme write */
      setValueName(arg1, getValueName(arg2));
      ixresult= arg1;
    } else if( targ1 == tVAR ) {
      if(((getFlags(arg1) & flsSTRING)==0) ) {     /* arg1 nostring or */
        setValueName(arg1, getValueName(arg2));
        ixresult= arg1;
      } else if(((getFlags(arg1) & flsSTRING)!=0) &&   /* string1 && string2 */
                ((getFlags(arg2) & flsSTRING)!=0)) {  
        setValueName(arg1, getValueName(arg2));
        /* arg2 string is not temporary because arg1 points to it */
        setFlags(arg2, getFlags(arg2) & (~flsTEMP));
        ixresult= arg1;
      } else {         /* string<-nostring */
	prerr("attempt: nonstring<- string");
      };
    } else {
      prerr("variable or vme address expected on the left side of assignment");
    };
  } else if(targ2==tFLOAT) {
    printf("apply1: arg1/2 fls:%x/%x\n", arg1->fls, arg2->fls);
    if( ((arg1->fls & flsTTM)==tVAR) && 
        ((arg1->fls & flsUNDEF) || (arg1->fls & flsFLOAT) ) ) {
      copyValueName(arg1, arg2);
      arg1->fls = (arg1->fls & (~flsUNDEF)) | flsFLOAT;
    } else {
      prerr("Float expected on the left side of assignment");
    };
  } else {
    prerr("var, const or VME address  expected on the right side of assignment");
  };
  break;
case tPLUS:
  ixresult= addName("_intvar2", tVAR | flsTEMP);
  setValueName(ixresult, getValueName(arg1)+getValueName(arg2));
  break;
case tMINUS:
  ixresult= addName("_intvar2", tVAR | flsTEMP);
  setValueName(ixresult, getValueName(arg1)-getValueName(arg2));
  break;
case tMULT:
  ixresult= addName("_intvar2", tVAR | flsTEMP);
  setValueName(ixresult, getValueName(arg1)*getValueName(arg2));
  break;
case tDIV:
  ixresult= addName("_intvar2", tVAR | flsTEMP);
  setValueName(ixresult, getValueName(arg1)/getValueName(arg2));
  break;
default: prerr1("apply2(): unknown operator", oprtr);
};
return(ixresult);
}

/*----------------------------------*/ int ismacro(Tname *mp) {
FILE *f;
char cmd[MAXLILE];
strcpy(cmd,"CFG/"); strcat(cmd,BoardName); strcat(cmd,"/");
strcat(cmd, mp->name); strcat(cmd,".mac");
f= fopen(cmd,"r");
if( f!= NULL) {
  fclose(f);
  return(1);
} else {
  return(0);
};
}

/*------------*/ void doMacCall(Tname *curmac, char *line, int *iixtoken) {
/*
*/

enum Ttokentype ctoken;
int ixtoken=*iixtoken, curtokpos;
char cmd[MAXLILE];

strcpy(actmac1.macpars[0], curmac->name); actmac1.nmp=1; 
delName(curmac);
while(1) {
  int WASPAR=0;
  curtokpos=ixtoken; ctoken= nxtoken(line, cmd, &ixtoken);
  if( ctoken==tRIGHT ) {
    strcpy(cmd,"CFG/"); strcat(cmd,BoardName); strcat(cmd,"/");
    strcat(cmd, actmac1.macpars[0]); strcat(cmd,".mac");
    actmac1.macfile= fopen(cmd,"r");
    if(actmac1.macfile) {
      actmac= &actmac1;    /* macro is active */
      printf("executing macro:%s\n",cmd); 
    };
    break;
  };
  if( ctoken==tCOMMA ) {
    WASPAR=0;
    continue;
  };
  if( ctoken==tINTNUM || ctoken==tHEXNUM || ctoken==tSYMNAME) {
    if( actmac1.nmp >=MAXFUNPARS ) {
      char msg[100];
      sprintf(msg,"too many macro parameters (max %d)", MAXFUNPARS);
      prerr2(line, curtokpos, msg);
      break;
    };
    strcpy(actmac1.macpars[actmac1.nmp], cmd);
    actmac1.nmp++;
    WASPAR=1;
    continue;
  };
  prerr2(line, curtokpos, "bad parameter in macro call");
  break;
};
*iixtoken=ixtoken;
}

/*------------*/ int doFuncCall(Tname *curfunc, char *line, int *iixtoken) {
/* returns: 0 -OK, 1 -error (function not called)
            iixtoken -token following '...)'
	    result of the function (UNDEF if void) is in the stack
*/
Tname *varconp;
enum Ttokentype ctoken;
int ixtoken=*iixtoken, curtokpos, rcfunc=0;
char cmd[MAXLILE];
w32 nfp; w32 funpars[MAXFUNPARS];
nfp=0; pusharg(curfunc);
while(1) {
  int WASPAR=0;
  curtokpos=ixtoken; ctoken= nxtoken(line, cmd, &ixtoken);
  if( ctoken==tRIGHT ) { /*---------------------------------- tRIGHT */
    Tname *ixres;
    curfunc= poparg();   /* current function */
    if( (curfunc->vmenp == 1) && (curfunc->pardesc[0].fls & flsVARIABLENP1)) {
      /* variable number of arguments, at least 1 par: */
      if( (nfp<1) || (nfp>MAXFUNPARS)) {
        char msg[100];
        sprintf(msg,"wrong # of parameters (1-%d expected)", MAXFUNPARS);
        prerr2(line, curtokpos, msg); rcfunc= 1;
        goto RETDOFC;
      };
    } else {
      if( (curfunc->vmenp != nfp) ||(nfp>MAXFUNPARS) ) {
        char msg[100];
        sprintf(msg,"wrong # of pars (>%d or != %d)", MAXFUNPARS,
	  curfunc->vmenp);
        prerr2(line, curtokpos, msg); rcfunc= 1;
        goto RETDOFC;
      };
    };
    //printf("flsFloatFUN (0xF00):%x fls:%x\n", flsFfloat, curfunc->fls);
    if( (curfunc->fls & flsFMSK) == flsFfloat) {
      setValueNameFloat(curfunc, 
        (float)(*curfunc->fp)(funpars[0], funpars[1], funpars[2], funpars[3],
        funpars[4], funpars[5], funpars[6], funpars[7], 
        funpars[8], funpars[9])   /*MAXFUNPARS*/
      );
    } else {
      setValueName(curfunc, 
        (*curfunc->fp)(funpars[0], funpars[1], funpars[2], funpars[3],
        funpars[4], funpars[5], funpars[6], funpars[7], 
        funpars[8], funpars[9])   /*MAXFUNPARS*/
      );
    };
    /*FC statef= NOFUNC; */
    ixres= addName("_intvar3", tVAR | flsTEMP);
    //setValueName(ixres, getValueName(curfunc));
    copyValueName(ixres, curfunc);
    if( ((curfunc->fls& flsTTM)==flsFfloat )) { 
      ixres->fls = ixres->fls & flsFLOAT;
    };
    //printf("flsFloatFUN2:\n"); printTname(ixres,0); printf("\n");
    pusharg(ixres);
    break;
  };
  if( ctoken==tCOMMA ) {  /*------------------------------ tCOMMA */
    WASPAR=0;
    continue;
  };
  /*printf("DBG VARP cmd:%s len(cmd):%d ixtoken:%d ctoken:%d\n",  
    cmd,strlen(cmd),ixtoken,ctoken); 
  */
  if( ctoken==tINTNUM || ctoken==tHEXNUM || ctoken==tSYMNAME ||
      ctoken==tSTRING || ctoken==tCHAR || ctoken==tFLOAT) {
    /*
    if(ctoken==tSTRING) 
      printf("DBG cmd:%s len(cmd):%d ixtoken:%d\n",cmd,strlen(cmd),ixtoken);
    */
    if( (curfunc->vmenp == 1) && (curfunc->pardesc[0].fls & flsVARIABLENP1)) {
      /* variable number of arguments, at least 1 par: */
    } else {
      if( curfunc->vmenp <= nfp) {
        char msg[100];
        sprintf(msg,"too many parameters (%d expected)", curfunc->vmenp);
        prerr2(line, curtokpos, msg); rcfunc= 1; break;
      };
    };
    varconp= semantic(ctoken, cmd);   
    /*printf("DBGnfp:%d, par.name:%s, par.fls:0x%x\n", 
      nfp,(curfunc->pardesc)[nfp].name,
      (curfunc->pardesc)[nfp].fls); 
    printTname(varconp,1); printf("\n"); */
    if( ((curfunc->pardesc)[nfp].fls & parINDIRECTPAR)!=0 ){ 
      if( ((curfunc->pardesc)[nfp].fls & parMASK)==parCHAR){ 
	/* check if supplied par. is string: */
	if( (getFlags(varconp) & flsSTRING) == 0) {
          prerr2(line, curtokpos, "string parameter expected");
          rcfunc= 1; break;
        };
      };
    };
/*  if( ((curfunc->pardesc)[nfp].fls & parINDIRECTPAR)==0 ){ 
      funpars[nfp]= getValueName(varconp);
    } else {
      funpars[nfp]= (w32)&varconp->intvar;
    }; */
    funpars[nfp]= getValueName(varconp);   // float problem
    /*printf("DBGfunpars:nfp:%d\n",nfp);
    printTname(varconp,1); printf("\n"); */
    if( funpars[nfp] == 0xdeadbeaf ) {
      prerr2(line, curtokpos, "uninitialized parameter");
      rcfunc= 1; break;
    };
    nfp++;
    if( nfp > MAXFUNPARS) {
      prerr2(line, curtokpos, "too many pars (max. 10 allowed)"); /*MAXFUNPARS*/
      rcfunc= 1; break;
    };
    WASPAR=1;
    continue;
  };
  prerr2(line, curtokpos, "bad parameter in function call"); 
  rcfunc= 1; break;
};
RETDOFC:
*iixtoken=ixtoken;
return(rcfunc);
}

/*----------------------------------*/ Tname *doexpr(char *line) {
/* if recursive call, stacks should be mallocated... */
enum Ttokentype ctoken;
/*FC enum {NOFUNC,FUNCNAME,FUNCLEFT,FUNCPAR} statef=NOFUNC; */
int ixtoken=0;   /* process whole line */
int ix;
Tname *varconp;
Tname *lastFunction;
char cmd[MAXLILE];

nops=0; nargs=0;   /* let's start with empty stacks */
while(1) {
  int  curtokpos;
  lastFunction=NULL;
  curtokpos= ixtoken; ctoken= nxtoken(line, cmd, &ixtoken);
  //printf("doexpr:%d\n", ctoken);
/*NOUNARY -commented UNARY ops actions*/
  while( ctoken==tLEFT /*NOUNARY || isunaryop(ctoken)*/ ) {
    pushop(ctoken);
    ctoken= nxtoken(line, cmd, &ixtoken);
  };
  if( isop(ctoken) ) {
    prerr2(line, curtokpos, "operator not expected");
    goto RETDOEXP;
  };
  varconp= semantic(ctoken, cmd);   
  /* varconp: pointer to allnames to:
  */   
  if( varconp==NULL) {
    prerr2(line, curtokpos, "operand expected");
    goto RETDOEXP;
  };
  if( typeName(varconp) == tEOCMD ) {
    goto RETDOEXP;
  };
/*NOUNARY
  while( isunaryop(topop()) ) {
    !!! new var has to be allocated here because equal constants share the same
    place
    varconp= apply1(popop(), varconp);
  };
*/
  curtokpos=ixtoken; ctoken= nxtoken(line, cmd, &ixtoken);
  if( ctoken==tLEFT) {            /* FunName( */
    if( typeName(varconp) == tFUN /*FC && statef==NOFUNC */ ) {
      if( doFuncCall(varconp, line, &ixtoken) != 0) {
	goto RETDOEXP;
      } else {
        lastFunction= varconp;
      };
    } else {
      if(sizearg()==0 && ismacro(varconp)) {
        /* varconp: points to macro name (macname.mac exists) */
/*        printf("macro call:"); printTname(varconp,0); printf("\n"); */
        doMacCall(varconp, line, &ixtoken);
	varconp= NULL;
        goto RETDOEXP;
      } else {
        prerr2(line, curtokpos, "Incorrect use of '(' ");
        goto RETDOEXP;
      };
    };
    curtokpos= ixtoken; ctoken= nxtoken(line, cmd, &ixtoken);
  } else {
    if( typeName(varconp) == tFUN ) {
      prerr2(line, curtokpos, "( expected after function call");
      goto RETDOEXP;
    } else {
      pusharg(varconp);
    };
  };
  while(1) {   /* get operator and execute (according to op-stack) */
    if( !isop(ctoken) && (ctoken!=tEOCMD) ) {
      prerr2(line, curtokpos, "operator expected");
      goto RETDOEXP;
    };
    if( sizeop()==0 ) {
      if( ctoken==tEOCMD ) {
        varconp= poparg(); 
        if(sizearg() != 0) {
          prerr1("arg stack size(0 expected):", sizearg());
        } else {
          if( lastFunction != NULL) { /* print if not void function: */
            /* lastFunction->fls: the result type of the func. */
            // printTname(lastFunction,0);
            if( (getFlags(lastFunction)&flsFMSK)==flsFint) {
              printf("<%d>\n",getValueName(varconp));
            }else if( (getFlags(lastFunction)&flsFMSK)==flsFw81632) {
              printf("<0x%x>\n",getValueName(varconp));
            }else if( (getFlags(lastFunction)&flsFMSK)==flsFfloat) {
              printf("<%f>\n",getValueNameFloat(varconp));
            }else if( (getFlags(lastFunction)&flsFMSK)==flsFchar) {
              printf("<%c>\n",getValueName(varconp));
            };
          };
	};
        goto RETDOEXP;
      } else {
        lastFunction= NULL;
        pushop(ctoken);
        break;
      };
    };
    if( priority(ctoken) > priority(topop()) ) {
      pushop(ctoken);
      break; /*NXARG */
    } else {
      Tname *arg1, *arg2;
      enum Ttokentype oper;
      oper= popop();
      if( oper== tLEFT && ctoken==tRIGHT ) {
  curtokpos= ixtoken;
  ctoken= nxtoken(line, cmd, &ixtoken);
        /*break;*/ continue;
      };
      if( oper== tLEFT ) {
        prerr2(line, curtokpos, "right bracket missing ?");
        goto RETDOEXP;
      };
      arg2= poparg(); arg1= poparg();
      varconp= apply2(oper, arg1, arg2);
      /* release temporary variables: */
      delNamefls(arg1, flsTEMP); delNamefls(arg2, flsTEMP);
      pusharg(varconp);
      if( varconp==NULL) goto RETDOEXP;
      continue; /* pushop(ctoken);break; */
    }; 
   /*NXARG:*/
  };
};
RETDOEXP:
/* release all temporary variables, but the one returned: */
for(ix=0; ix< nnames; ix++) {
  if (varconp != &allnames[ix]) 
    delNamefls(&allnames[ix], flsTEMP);
};
return(varconp);
}

/*---------------------------------------*/ void parseline(char *line) {
int ixtoken=0;
while(1) {
  char cmd[MAXLILE];
  int previxtoken; 
  Tname *ixcmd;
  enum Ttokentype tokentype;
  previxtoken= ixtoken;
  tokentype= nxtoken(line, cmd, &ixtoken); 
  if( tokentype == tEOCMD ) { break;};
  ixcmd= findName(cmd);
  if( DBG & DBGlex) {
    printf("token:%d (%d:%d) cmd:%s\n", 
           tokentype, previxtoken, ixtoken-1, cmd);
  };
};
}

/*----------------------------------------*/ void checkLogFile( FILE **lf) {
static char lfname[MAXNAMELENGTH]="";
if(getStringName("logfile") == NULL) return;
if( strcmp(lfname, getStringName("logfile"))!=0) {
  char lfokname[MAXNAMELENGTH];
  if( *lf !=NULL ) fclose(*lf);
  strcpy(lfname, getStringName("logfile"));
  if(strcmp(lfname, "\"\"")==0) return;
  strcpy(lfokname, &lfname[1]);
  lfokname[strlen(lfokname)-1]='\0';
  *lf= fopen(lfokname,"w");
};
}

/*----------------------------------------*/ int main(int argn, char **argv) {
w32 loops; Tname *p2loops;
int rccret; /* For checking errors */
int noboardInit=0;   /* 0-> boardInit to be called by default*/
int noprompt=0;   /* 0-> prompt by default*/
int noinitmac=0;   /* 0-> init.mac to be called by default*/
int vsp=0;         /* always 0 from cmdbase */
FILE *logfile=NULL;

int i;
char line[MAXLILE];

#if defined(WIN32)
/*fcntl(0, F_SETFL, 0);    0->blocking, FNDELAY->non-bloking */
#endif
/*fcntl(1, F_SETFL, FNDELAY); */

/* #if defined(linux) || defined(AIX) */
setlinebuf(stdout);
signal(SIGUSR1, gotsignal); siginterrupt(SIGUSR1, 0);
signal(SIGBUS, gotsignal); siginterrupt(SIGBUS, 0);
/*intercept(gotsignal);*/
/* #endif */
printf("PID %d", getpid());   /* has to be 1st line written out */
/* boardname.exe [base] -opt1 -opt2
 * base: 0xa000   (hexa) or VXI0::450
*/
line[0]='\0';
for(i=0; i<argn; i++) {
  int wasbase=0;
  //printf("arg%d:%s: ",i, argv[i]); 
  if(i==0) continue;
  printf(" %s", argv[i]);
  if(strcmp(argv[i], "-simvme") == 0){
    printf("-simvme");
    continue;
  };
  if(strcmp(argv[i], "-noboardInit") == 0){
    noboardInit=1;
    continue;
  };
  if(strcmp(argv[i], "-noprompt") == 0){
    noprompt=1;
    continue;
  };
  if(strcmp(argv[i], "-noinitmac") == 0){
    noinitmac=1;
    continue;
  };
  if(wasbase==0) { /* base */
    /*printf("base:%s",argv[i]); */
    if(((strlen(argv[i]) <= 10) && 
        ((strncmp(argv[i],"0x",2)==0) || (strncmp(argv[i],"0X",2)==0))) ||
       (strncmp(argv[i],"VXI0::",6)==0)) {
      strcpy(BoardBaseAddress, argv[i]);
/*      printf("cmdbase.c: BoardBaseAddress:%s\n",BoardBaseAddress); */
      wasbase=1;
      continue;
    } else {
      printf(" base:%s -too long string or not '0x...' ",argv[i]);
    };
  };
  printf(" %s -bad parameter, exiting\n", argv[i]); exit(8);
/* on unix:
  if(strcmp(argv[i], "-nblk") == 0) {
    w32 flags; int rc;
    extern errno;
    printf("-nblk");
    flags= fcntl(0, F_GETFL, 0);
    flags= flags | O_NONBLOCK;
    rc= fcntl(0, F_SETFL, flags);
    if(rc) printf(
      "ERROR fcntl stdin returned: %d %x errno:%d \n",rc, flags, errno);
    rc=setvbuf(stdin, NULL, _IOLBF, 0);
    if(rc) printf(
      "ERROR setvbuf stdin returned: %d %x errno:%d \n",rc, flags, errno);
  };
*/
}; 
printf("\n"); 
micwait(1);   /* calibrate micwait */
rccret= vmxopenam(&vsp, BoardBaseAddress,BoardSpaceLength,BoardSpaceAddmod);
if(rccret!=0) {
  printf("vmeopen rc:%d\n", rccret); exit(8);
};
initmain();
if(noboardInit==0) boardInit();
/* vmeclose();  -necessary with cctopen in use from cmdlin2 */
/* printf("RAND_MAX:%d\n", RAND_MAX); */
/*BoardBaseAddress= (char *)allnames[0].usage; */
/*printf("allnames[0].name:%s initallnames...\n",allnames[0].name);*/
init_allnames();

loops=1;
p2loops= addNameIndirect("loops", tVAR | flsINDIRECT, &loops);
if((noboardInit==0) && (noinitmac==0) && (ismacro(findName("init"))==1)) {
  line[0]=0x1; 
} else {
  line[0]='\0'; 
};
while(1) {
  int iloops, currentloops;
  Tname *result;   /* pointer to allnames[] or temporary Tname item */
  checkLogFile( &logfile );
  if( line[0] == 0x1) {
    /* rc= cctopen(); */
    /* rc= vmeopen(BoardBaseAddress,BoardSpaceLength); */
    strcpy(line,"init()\n");
  } else {
    if( actmac != NULL) {
      fgets(line, MAXLILE, actmac->macfile);
      /*printf("maccalline:%s=\n",line); */
    } else {
      if(noprompt==0) printf(":\n"); 
      #if defined(WIN32)
      fflush(stdout);
      #endif
      fgets(line, MAXLILE, stdin);
    };
    /* for(i=0; i<8; i++) printf("%x:", line[i]); */
    /* printf("line:%s:%x,%x\n", line,line[0],line[1]); */
    if( line[0] == '\0') {
      if(actmac != NULL) {
        /* rc= vmeclose(); with cct */
        fclose(actmac->macfile); actmac= NULL;
        goto NEXTCMD;
      } else {
        printf("Empty input received, quitting main cmd loop...\n");
        break; 
      };
    };
  };
  if( line[0] == '\n' || line[0]=='\r') { goto NEXTCMD; };
  if( logfile != NULL) {
    fprintf(logfile,"%s", line);
  };
  if( line[0] == '#' ) { printf("%s",line); goto NEXTCMD; };
  if( strcmp(line, "q\n") == 0) break;
  if( line[0] == '?' ) {printHelp(line); goto NEXTCMD; };
  if( line[0] == ' ' && line[1]=='?' ) {printHelp(&line[1]); goto NEXTCMD; };
  currentloops=loops;
  for(iloops=1; iloops<=currentloops; iloops++) {
    /*parseline(line);*/
    result=doexpr(line);    /*temporary/named variable or constant */
    /* printTname(result,1); printf("\n"); */
    if( quit != 0 ) break;
  };
  NEXTCMD:
  line[0]='\0'; 
  if( quit != 0 ) {
    /*printf("gotsignl:%d clearing...\n", quit); */
    quit=0;
  };
};
rccret=vmeclose();
if( logfile !=NULL) fclose(logfile);
endmain();
return 0;
}

