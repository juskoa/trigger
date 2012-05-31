#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../../vmeb/vmeblib/lexan.h"
#include "switch.h"
#define ERRMSGL 400

extern char BoardBaseAddress[40]; /* SWITCH VME address */

/*
rc: 0: ok
*/
int switchTables(Tswitch *st, char *fn) {
enum Ttokentype token;
int ix, rc=0;
char emsg[ERRMSGL]="";
char *environ;
FILE *cfgfile;
char fnpath[160];
char line[200], value[200];
environ= getenv("VMECFDIR"); strcpy(fnpath, environ);
strcat(fnpath,"/CFG/ctp/DB/"); strcat(fnpath,fn); 
cfgfile=fopen(fnpath,"r");
if(cfgfile == NULL){
  printf("fnpath:%s:\n", fnpath);
  perror(strerror(errno));
  return(1);
};
for(ix=0; ix<MXOUT; ix++) {
  st[ix].namepp[0]='\0';
  st[ix].namectp[0]='\0';
  st[ix].eq=0;
  st[ix].input=0;   //not connected
  st[ix].ctpinput=ix;
};
while(fgets(line, 199, cfgfile)){
  int swout,swin, ctpin;
  Tswitch nxt;
  //printf("Decoding line:%s ",line);
  if(line[0]=='#') continue;
  if(line[0]=='\n') continue;
  ix=0; token= nxtoken(line, value, &ix);
  if(token==tSYMNAME) {
    strcpy(nxt.namepp, value); 
    token=nxtoken1(line, value, &ix);
    if(token==tSYMNAME) {
      strcpy(nxt.namectp, value); 
      token=nxtoken(line, value, &ix);
    } else {
      sprintf(emsg, "sym. name of CTP L0 signal expected from %s. got:%s", fn, value);
      goto ERR;
    }
    if(token==tINTNUM) {         //eq
      nxt.eq= str2int(value);
      token=nxtoken(line, value, &ix);
      if(token==tINTNUM) {         //input
        swin= str2int(value);
        if((swin<0) || (swin>MXINPUT)) {
          sprintf(emsg, "switch in expected (1..50 or 0) but got:%s",value);
          goto ERR;
        };
        nxt.input= swin;
        token=nxtoken(line, value, &ix);
        if(token==tINTNUM) {         //switch output
          swout= str2int(value);
          if((swout<0) || (swout>MXOUT)) {
            sprintf(emsg, "switch out expected (1..25 or 0) but got:%s",value);
            goto ERR;
          };
          token=nxtoken(line, value, &ix);
          if(token==tINTNUM) {         //ctp input (should be -1)
            ctpin= str2int(value);
          } else if(token!=tEOCMD) {
            sprintf(emsg, "CTP input or nothing expected (1..24 or 0) but got:%s",value);
            goto ERR;
          } else {
            ctpin= swout-1;
          };
        } else {
          sprintf(emsg, "int expected (1..25 or 0) but got:%s",value);
          goto ERR;
        };
      } else {
        sprintf(emsg, "int expected (1..50 or 0) but got:%s",value);
        goto ERR;
      };
    } else {
      sprintf(emsg, "int expected as equaliser but got:%s",value);
      goto ERR;
    };
  } else {
    sprintf(emsg, "symbolic name expected but got:%s",value);
    goto ERR;
  };
  if(swout>0) {
    strcpy(st[swout-1].namepp, nxt.namepp);
    strcpy(st[swout-1].namectp, nxt.namectp);
    st[swout-1].eq= nxt.eq;
    st[swout-1].input= nxt.input;
    st[swout-1].ctpinput= nxt.ctpinput;
  };
};
ERR:
if(emsg[0]!='\0') {
  printf("%s",line);
  printf("Error:%s\n", emsg);
  rc=1;
};
fclose(cfgfile);
/*
for(ix=0; ix<MXOUT; ix++) {
  printf("%s %s: %d -> %d\n", st[ix].namepp, st[ix].namectp, st[ix].input, ix+1);
};*/
return(rc);
}
int loadTable(int ctpltu) {
int ix,rc, retcode=0;
Tswitch st[MXOUT];
char fn[40];
/*
if(strcmp(ctpltu," ")==0) {
  if(strcmp(BoardBaseAddress, "0x140000")==0) {
    strcpy(fn, "CTP.SWITCH");
  } else if(strcmp(BoardBaseAddress, "0x110000")==0) {
    strcpy(fn, "LTU.SWITCH");
  } else {
    printf("loadTable: Unknown base addr: %s swithc not loaded\n",
      BoardBaseAddress);
    retcode=10; goto RET;
  };
} else if(strcmp(ctpltu,"ctp")==0) {
  strcpy(fn, "CTP.SWITCH");
} else if(strcmp(ctpltu,"ltu")==0) {
  strcpy(fn, "LTU.SWITCH");
} else {
  sprintf(fn, "%s.SWITCH", ctpltu);
  //retcode=10; goto RET;
};
*/
if(ctpltu==1) {
  strcpy(fn, "CTP.SWITCH");
} else if(ctpltu==0) {
  strcpy(fn, "LTU.SWITCH");
} else {
    printf("loadTable: ctpltu can be 1:ctp or 0:ltu swith not loaded\n");
    retcode=10; goto RET;
};
rc= switchTables(&st[0], fn);
if(rc==0) {
  int rc, connected=0; int setchans=0;
  for(ix=0; ix<MXOUT; ix++) {
    if(st[ix].input !=0) {
      connected++;
    };
    rc= setchanctpltu(ctpltu, st[ix].input,ix+1);
    if(rc==1) {setchans++;};
  };
  printf("%s: connected:%d changed:%d\n", fn, connected, setchans);
} else {
  printf("loadTable: error in %s file, switch not loaded\n",fn);
  retcode=rc; 
};
RET:
return(retcode);
}
