/* arguments: if no arguments, all the CTP/LTU boards searched
 *            (see bnames in main())
 * stdout: ltu=0x81x000\nfo=0x82x000\nttcvi=0x80x000\n
7.6.2005
The findboards.exe is called in boot time to create file
/root/NOTES/boardsincrate (see /etc/rc.d/rc.local)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "vmewrap.h"
//#include "../CTPcommon/vmefpga.h"

#define CODE_ADD      0x4     /* board type (0x56 for LTU) */
#define SERIAL_NUMBER 0x8     /* unique serial number  of the board */
#define VERSION_ADD   0xC     /* VME FPGA firmware version */

int quit=0;
int buserr=0;

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
  buserr=1;
  break;
default:
  printf("got unknown signal:%d\n", signum);
};
}

w32 r16x4(w32 fromadr) {
w32 i,rc=0;
for(i=0; i<=12; i=i+4) {
  rc= rc<<8;
  rc= rc | (vmer16(fromadr+i) & 0xff);
  if(buserr!=0) {rc=0; break; };
};
return(rc);
}

/*-------------------------------------------------------- findboards()
boardtype: ltu, fo, ttcvi
foundboard: output string
*/
w32 findboards(char *boardtype, char *foundboards) {
int ix,rco=0,rcc=0,mindial,maxdial;
w32 code, boardcode;
char base[20],blength[20];
/*printf("findboards:%s\n", boardtype); */
foundboards[0]='\0'; base[0]='\0';
if(strcmp(boardtype,"ltu")==0) {
  strcpy(base,"0x81");mindial=0;maxdial=15; boardcode=0x56;
  strcpy(blength,"0x300");
};	
if(strcmp(boardtype,"fo")==0) {
  strcpy(base,"0x82");mindial=0;maxdial=7; boardcode=0x53;
  strcpy(blength,"0x300");
};	
if(strcmp(boardtype,"busy")==0) {
  strcpy(base,"0x82");mindial=8;maxdial=8; boardcode=0x54;
  strcpy(blength,"0x300");
};	
if(strcmp(boardtype,"l0")==0) {
  strcpy(base,"0x82");mindial=9;maxdial=9; boardcode=0x50;
  strcpy(blength,"0x300");
};	
if(strcmp(boardtype,"l1")==0) {
  strcpy(base,"0x82");mindial=10;maxdial=10; boardcode=0x51;
  strcpy(blength,"0x300");
};	
if(strcmp(boardtype,"l2")==0) {
  strcpy(base,"0x82");mindial=11;maxdial=11; boardcode=0x52;
  strcpy(blength,"0x300");
};	
if(strcmp(boardtype,"int")==0) {
  strcpy(base,"0x82");mindial=12;maxdial=12; boardcode=0x55;
  strcpy(blength,"0x300");
};	
if(strcmp(boardtype,"ttcvi")==0) {
  strcpy(base,"0x80");mindial=0;maxdial=7; boardcode=0x80030;
  strcpy(blength,"0x300");
};
if(base[0]=='\0') {
  sprintf(foundboards,"%s=unknown board\n", boardtype);
  return(0);   /* unknown request */
};
for(ix=mindial; ix<=maxdial; ix++) {
  int rco, rcc;
  char b1[20];
  sprintf(b1,"%s%1x000",base,ix);
  rco= vmeopen(b1, blength);
  if(rco>0) break;
  if(strcmp(boardtype,"ttcvi")==0) {
    code=r16x4(0x22);   /* CERNID of TTCvi */
  } else {
    code= 0xff&vmer32(CODE_ADD); 
  };
  usleep(1000);
  if(buserr==0) {
    char b2[30];
    if(code==boardcode) {
      sprintf(b2,"%s=%s", boardtype, b1);
    } else {
      sprintf(b2,"%s=%s", "UNKNOWN", b1);
    };
    if(foundboards[0]=='\0') {
      strcpy(foundboards, b2);
    } else {
      strcat(foundboards, "\n"); strcat(foundboards, b2);
    };
    /*LoadFPGA(); */
  } else {
    /* printf("Was buserr\n"); */
    buserr=0;
  };
  rcc= vmeclose();
/*  printf("%s %s %d 0x%x %d:%s\n",b1,blength,rco,code,rcc,foundboards); */
};
/*printf("vmecrate %s\n", foundboards); */
return(rco+rcc);
}
int main(int argn, char **argv) {
int rc,i,ifrom,nboards;
char **wanted;
#define NBOARDS 8
char *bnames[NBOARDS]={"ltu","fo","busy","l0","l1","l2","int", "ttcvi"};
char foundboards[400];
char allboards[400];
signal(SIGBUS, gotsignal); siginterrupt(SIGBUS, 0);
allboards[0]='\0';
if(argn > 1) {
  ifrom=1;
  nboards=argn;
  wanted=argv;
} else {
  ifrom=0;
  nboards=NBOARDS;
  wanted=bnames;
};
for(i=ifrom; i<nboards; i++) {
  rc= findboards(wanted[i], foundboards);
  if(rc != 0) {
    printf("findboards error:%d\n",rc);
  } else {
    if(foundboards[0]!='\0') {
      strcat(allboards, foundboards);
      strcat(allboards, "\n"); 
    };
  };
}
/*rc= findboards("ttcvi", foundboards);
strcat(allboards, " "); strcat(allboards, foundboards)*/
/*printf("vmecrate%s\n",allboards); */
printf("%s",allboards);
}

