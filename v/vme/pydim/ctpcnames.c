/* ctpc_ routines supporting CTPRCFG/CNAMES DIM service
debug: see main below
29.4.2015 bug fixed: we work with 48 switch input numbers
          still: where is 'inpupd' command activated (see server.c)
*/
#include <stdio.h>
#include <string.h>
#include "lexan.h"
#include "vmewrap.h"

#ifdef mainp
#define DBMAIN
#endif
#include "Tpartition.h"

#define MAXCLNAMELEN 32
#define MAXNAMELENGTHTRGIN MAXCTPINPUTLENGTH
/*
#define MAXITEMS 500
#define MAXALIASES 300
// average number of aliases per class
#define AVAPC 5
#define MAXLINELENGTH 200

int aliasNamesN;   // number of items in aliasNames
char aliasNames[MAXALIASES][MAXNAMELEN];
// aliases lists, each list finished by -1. Start of the list
// is pointed by array[].value
int cls2aliasesN;   // number of items in cls2aliases
int cls2aliases[MAXITEMS*(AVAPC+1)];   
*/

/* this is quick patch (better: should process cnames.sorted2 file) */
int epoch_positions[2]= {1511, 1512};   // secs, mics
int inp_positions[3]= {119, 306, 606};   // l0inp1, l1inp1, l2inp1
int class_positions[6]= {19, 187, 340, 440, 626, 726};  // l0classB1,l0classA1,l1classB1,...

typedef struct class_item {
  char logname[MAXCLNAMELEN];   // "" not allocated (also runn==0 in this case)
  int runn;   // 0: class not allocated, >1:class allocated for runn
} Tclass_item;
typedef struct ctpin_item {
  // not needed -is in shm-> validCTPINPUTs[ix]
  //char logname[MAXNAMELENTRGIN];   // "" not allocated (i.e. runlist[]=[0,...])
  char logname;   // '\0': not configured, 'Y':
  int runlist[MNPART];   // all 0: input not allocated, >1:input allocated for run(s)
} Tctpin_item;

Tclass_item ctpclasses[NCLASS];   //1..100 -> 0..99
Tctpin_item ctpins[NCTPINPUTS];   //l0inp1..48 l1inp1..24 l2inp1..12 -> 0..83

/*---------------------------------------------------------------------
I: classN:1..100   runn: run number 
*/
void ctpc_clear() {
int ix;
for(ix=0; ix<NCTPINPUTS; ix++) {
  int ixx;
  ctpins[ix].logname= '\0';
  for(ixx=0; ixx<MNPART; ixx++) {
    ctpins[ix].runlist[ixx]= 0;
  };
};
/* now update from validCTPINPUTS: */
for(ix=0; ix<NCTPINPUTS; ix++) {
  int level, inputnum, ixx;
  //if(validCTPINPUTs[ix].configured!=1) continue;
  if(validCTPINPUTs[ix].name[0]=='\0') continue;
  if((validCTPINPUTs[ix].switchn==0) &&
     (validCTPINPUTs[ix].inputnum==0)) continue;
  level= validCTPINPUTs[ix].level;
#ifdef mainp
  if(level<0 || level >2) {
    printf("error: %d %d %s\n", level, validCTPINPUTs[ix].inputnum,
      validCTPINPUTs[ix].name);
  };
#endif
  if(level==0) {
    ixx=0;
    inputnum=  validCTPINPUTs[ix].switchn;
    if(validCTPINPUTs[ix].inputnum==0) continue;
  };
  if(level==1) {
    ixx=48;
    inputnum=  validCTPINPUTs[ix].inputnum;
  };
  if(level==2) {
    ixx=48+24;
    inputnum=  validCTPINPUTs[ix].inputnum;
  };
  ixx= ixx + (inputnum-1);
  ctpins[ixx].logname= 'Y';
};
for(ix=0; ix<NCLASS; ix++) {
  ctpclasses[ix].logname[0]= '\0';
  ctpclasses[ix].runn= 0;
};
}

/*---------------------------------------------------------------------
I: classN:1..100   runn: run number 
*/
void ctpc_addclass(int classN, char *clname, int runn) {
strcpy(ctpclasses[classN-1].logname, clname);
if(ctpclasses[classN-1].runn != 0) {
  printf("ERROR addclass %d %s run:%d reallocated for run:%d\n", classN,clname,
    ctpclasses[classN-1].runn, runn);
};
ctpclasses[classN-1].runn= runn;
}

/*---------------------------------------------------------------------
I: inn:1..60   runn: run number 
   i.e. L0: 1..24 (ctp inputs, NOT switch inputs)
        L1: 25.. 48
        L2: 49.. 60
*/
void ctpc_addinp(int inn60, int runn) {
int ix,ixfree,allocated=0,inn;
// find corresponding index (1..84) in ctpins from inn60 (1..60)
if(inn60<=24) { // L0
  int ix;
  ix= findInput(0, inn60);
  if(ix==-1) {
    printf("ERROR ctpc_addinp: unconnected L0 ctp inp. number(1..24):%d run:%d\n", inn60, runn);
    goto RET;
  };
  inn=  validCTPINPUTs[ix].switchn;
  //printf("inn60:%d inn(switch):%d\n",inn60, inn);
} else if(inn60<=48) { // L1
  inn= inn60+24;
  //printf("inn60:%d inn:%d\n",inn60, inn);
} else if(inn60<=60) { // L2
  inn= inn60+24;
  //printf("inn60:%d inn:%d\n",inn60, inn);
} else {
  printf("ERROR ctpc_addinp: bad inp. number:%d run:%d\n", inn60, runn);
  goto RET;
};
ixfree=-1;
for(ix=0; ix<MNPART; ix++) {
  if(ctpins[inn-1].runlist[ix] == 0) {
    if(ixfree== -1) ixfree= ix;
  } else {
    allocated++;
  };
};
if(ixfree==-1) {
  printf("ERROR ctpc_addinp: input %d fully allocated (6/8 runs)\n", inn);
} else {
  ctpins[inn-1].runlist[ixfree]= runn;
  //ctpins[inn-1].logname= 'Y';   // is there for all connected inputs anyhow
  allocated++;
};
RET: ;
#ifdef mainp
printf("INFO ctpc_addinp: %d runn:%d in position:%d allocated inps:%d\n", inn, runn, ixfree, allocated);
#endif
}
/*---------------------------------------------------------------------
*/
void ctpc_delrun(int runn) {
int ix;
// over ctpclasses
for(ix=0; ix<NCLASS; ix++) {
  if(ctpclasses[ix].runn== runn) {
    ctpclasses[ix].runn= 0;
    ctpclasses[ix].logname[0]= '\0';
  };
};
// over ctpins
for(ix=0; ix<NCTPINPUTS; ix++) {
  int ixx, nfree=0;
  for(ixx=0; ixx<MNPART; ixx++) {
    if(ctpins[ix].runlist[ixx]==0) nfree++;
    if(ctpins[ix].runlist[ixx]==runn) {
      ctpins[ix].runlist[ixx]= 0;
      nfree++;
    };
  };
  /* input is still configured, athough not used by run(s):
  if( nfree==MNPART ) { ctpins[ix].logname= '\0'; }; */
};
}
/*---------------------------------------------------------------------
*/
int gethw_inp(int level, int ixvci, char *hwname) {
int icnames;
if(level==0) {
  icnames=  validCTPINPUTs[ixvci].switchn;
} else {
  icnames=  validCTPINPUTs[ixvci].inputnum;
};
sprintf(hwname, "l%dinp%d", level, icnames);
return(inp_positions[level] + (icnames-1));
}
/*---------------------------------------------------------------------
levelba: 0..5 corresponding to 0b 0a 1b 1a 2b 2a
classn: 1..100
*/
int gethw_class(int levelba, int classn, char *hwname, char *ctype) {
int level; char ba;
level= levelba/2;
if((levelba==0) || (levelba==2) || (levelba==4)) {
  ba='B';
  sprintf(ctype, "class%db", level);
} else {
  ba='A';
  sprintf(ctype, "class%da", level);
};
sprintf(hwname, "l%dclass%c%d", level, ba, classn);
return(class_positions[levelba]+ (classn-1));
}
/*---------------------------------------------------------------------
print following lines:
index type logname hwname run_number_list
index type logname hwname
            -

*/
void ctpc_print(char *dimpublication) {
int ix, position;
char line[200]; char hwname[16]; char logname[32]; char runlist[100];
dimpublication[0]= '\0';
sprintf(line, "%d epoch epochsecs epochsecs\n",epoch_positions[0]);
strcat(dimpublication, line);
sprintf(line, "%d epoch epochmics epochmics\n",epoch_positions[1]);
strcat(dimpublication, line);
// ctpins part. Go ovver configured validCTPINPUTs and look into ctpins:
for(ix=0; ix<NCTPINPUTS; ix++) {
  int level, inputnum, insix, ix2;
  //if(validCTPINPUTs[ix].configured!=1) continue;
  if(validCTPINPUTs[ix].name=='\0') continue;
  level= validCTPINPUTs[ix].level;
  if(level==0) {
    inputnum= validCTPINPUTs[ix].switchn;
  } else {
    inputnum= validCTPINPUTs[ix].inputnum;
  };
  if((inputnum==0) || (inputnum==-1)) continue;   // L0/1/2 not connected
  //ix= findInput(0, inn60);
  position= gethw_inp(level, ix, hwname);
  if(level==0) {
    insix=0;
  } else if(level==1) {
    insix=48;
  } else if(level==2) {
    insix=72;
  };
  insix= insix + (inputnum-1);
  if(ctpins[insix].logname=='\0') {
    strcpy(logname, "-");   // should not happen! (i.e. error should be issued!)
  } else if(ctpins[insix].logname=='Y') {
    //strcpy(logname, ctpins[insix].logname);
    strcpy(logname, validCTPINPUTs[ix].name);
  } else {
    printf("ctpcnames: interr\n"); continue;
  };
  runlist[0]= '\0';
  for(ix2=0; ix2<MNPART; ix2++) {
    int runn;
    runn= ctpins[insix].runlist[ix2];
    if(runn>0) {
      sprintf(runlist, "%s%d ", runlist, runn); };
  };
  if(strlen(runlist)>0) {
    //printf("ctpc_print: runlist:%s:\n", runlist);
    runlist[strlen(runlist)-1]= '\0';   // strip off last ' '
    sprintf(line, "%d input %s %s %s\n", position, logname, hwname, runlist);
  } else {
    sprintf(line, "%d input %s %s\n", position, logname, hwname);
  };
  strcat(dimpublication, line);
};
// ctpclass part.
for(ix=0; ix<NCLASS; ix++) {
  int runn, ix2;
  runn= ctpclasses[ix].runn;
  if(runn==0) continue;
  strcpy(logname, ctpclasses[ix].logname);
  for(ix2=0; ix2<6; ix2++) {   // 0b 0a 1b 1a 2b 2a
    char ctype[16];
    position= gethw_class(ix2, ix+1, hwname, ctype);
    sprintf(line, "%d %s %s %s %d\n", position, ctype, logname, hwname, runn);
    strcat(dimpublication, line);
  };
};
}
/* 
g++ -g -Wall -Dmainp -DCPLUSPLUS -I$VMEBDIR/vmeblib -I$VMECFDIR/ctp/ctplib ctpcnames.c -lpthread -L$VMECFDIR/ctp/ctplib/linux_s -lctp -L$VMEBDIR/vmeblib/linux_s -lvmeb -o linux_s/ctpcnames.exe
*/
#ifdef mainp
#define MAXCNAMESDIM (100*6 + 60 + 5)*80  // (100 classes, 60 inps, 1: epoch)
char dimpublication[MAXCNAMESDIM];
int main() {
cshmInit();
readTables(); // should not be here for debugging with running ctpproxy!
ctpc_clear();
ctpc_addinp(49, 188);   // l2inp1
ctpc_addinp(24, 188);   // l0inp24
ctpc_addinp(25, 189);   // l1inp1
ctpc_addclass(10, (char *)"class-10-ALL-BLA", 188);
ctpc_addclass(11, (char *)"class-10-ALL-BLA", 189);
ctpc_print(dimpublication); printf(":%s:", dimpublication);
ctpc_delrun(188);
ctpc_print(dimpublication);

cshmDetach(); printf("INFO shm detached.\n");
//printf(":%s:", dimpublication);
} 
#endif
