/* readctp.c -dim client passing CTP counters to RRD
13.2.2007 I2C,orbit (23 counters-> 1+11*4) added
10.7. LTU volotages added
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h> 
#include <ctype.h> 

#ifdef CPLUSPLUS
extern "C" {
#include <dic.hxx>
} 
#else
#include <dic.h>
#endif
#include "common.h"

/*-------------------------*/ void epoch2date(time_t epoch, char *dmyhms) {
/* Input: epoch
   Output: dmyhms:  string[18] dd.mm.yy hh:mm:ss */
/*int i; */
struct tm *LT;  /*LocalTime */
//T=time(&T); 
LT = localtime(&epoch);
sprintf(dmyhms,"%2.2d.%2.2d.%4.4d %2.2d:%2.2d:%2.2d",
  LT->tm_mday,LT->tm_mon+1,LT->tm_year+1900,
  LT->tm_hour,LT->tm_min,LT->tm_sec);
}

/*------------------------------------------------------------- dodif32()
Substract 2 32 bits values (representing counters)
 -defined in vmeblib, to be removed from here
w32 dodif32(w32 before, w32 now) {
w32 dif;
if(now >= before) dif= now-before;
else dif= now + (0xffffffff-before) +1;
//if(DBGcnts) printf("dodif32:%d\n", dif);
return(dif);
} */
/*--------------------------------*/ void vme2volt(w32 vme, int *volts) {
float volt5,volt3_3,volt1_5,volt5b;
 volt5=(vme & 0xff)*23.725;
 volt5b=((vme & 0xff000000)>>24)*23.725;
 volt3_3=((vme & 0xff00)>>8)*23.725;
 volt1_5=((vme & 0xff0000)>>16)*12.941;
 //volts[0]= volt5; volts[1]= volt3_3; volts[2]= volt1_5; volts[3]= volt5b;
 volts[0]= (int)volt5; volts[1]= (int)volt3_3; volts[2]= (int)volt1_5; volts[3]= (int)volt5b;
 //sprintf(mv4str, "%4.0f %4.0f %4.0f %4.0f",volt5,volt3_3,volt1_5,volt5b);
}

int skipSpaces(char *line, int ix) {
int ixloc=ix;
while(1) {
  if(line[ixloc]!=' ') return(ixloc);
  ixloc++;
}
}
/*-----------------------------*/ int parseLine(char *line, Tsorted *pl) {
/* delimiter: space(s)
rc: 0: ok
1 too long field
2 not enough fields
3 too many fields
*/
int rc=0;
int ix=0, ixstart,ixfield;
ix= skipSpaces(line, ix); ixstart= ix; ixfield=0;
while(1) {
  if((line[ix]==' ') || (line[ix]=='\n')) {
    int lng;
    lng=ix-ixstart;
    if(ixfield==0) {
      if(lng>19) {rc=1; goto RTR; };          // too long field
      strncpy(pl->cname, &line[ixstart], lng); pl->cname[lng]='\0';
    } else if(ixfield==1) {
      if(lng>7) {rc=1; goto RTR; };
      char addr[9];
      strncpy(addr, &line[ixstart], lng); addr[lng]='\0';
      pl->addr= atoi(addr);
    } else if(ixfield==2) {
      if(lng>7) {rc=1; goto RTR; };
      strncpy(pl->board, &line[ixstart], lng); pl->board[lng]='\0';
    } else if(ixfield==3) {
      if(lng>1) {rc=1; goto RTR; }; 
      pl->type= line[ixstart];
    } else if(ixfield==4) {
      if(lng>19) {rc=1; goto RTR; };
      strncpy(pl->ltuname, &line[ixstart], lng); pl->ltuname[lng]='\0';
    } else {
      rc=3; goto RTR;   // too many fields
    };
    ixfield++;
    ix= skipSpaces(line, ix); ixstart=ix;
    if((line[ix]=='\0') || (line[ix]=='\n')) break;
  } else {
    if((line[ix]=='\0') || (line[ix]=='\n')) break;
    ix++;
  };
};
if(ixfield<4) {
  rc=2;   // not enough fields
};
RTR: return(rc);
}

