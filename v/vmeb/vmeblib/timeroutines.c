#include <stdio.h>
//#include <unistd.h>   /* usleep */
#include <time.h>   
#include <sys/time.h>   /* gettimeofday */
typedef unsigned int w32;
#include "vmeblib.h"

/*--------------------------------*/ void getdatetime(char *dmyhms) {
/* format date/time for logs
   dmyhms:  string[20] dd.mm.yyyy hh:mm:ss */
/*int i; */
time_t T;
struct tm *LT;  /*LocalTime */
T=time(&T); LT = localtime(&T);
sprintf(dmyhms,"%2.2d.%2.2d.%4.0d %2.2d:%2.2d:%2.2d",
  LT->tm_mday,LT->tm_mon+1,LT->tm_year+1900,
  LT->tm_hour,LT->tm_min,LT->tm_sec);
 /*for(i=0; i<28; i++) { if(dmyhms[i] == ' ') dmyhms[i] = '0'; }; */
}

/* count:1 -> cca 0.5 micsec (for MLTC=15) */
#define MLTC 15
void Delay(int count)
{
        int i,j;
        count = count*MLTC;
        for (i=0; i<count; i++) {
                j=i;
        }
}
/*----------------------------------------------*/
void GetMicSec(w32 *tsec, w32 *tusec) {
int rc;
struct timeval tv; struct timezone tz;
rc=gettimeofday(&tv,&tz);
*tsec=tv.tv_sec; *tusec=tv.tv_usec;
}
/*----------------------------------------------*/
void AddSecUsec(w32 *tsec,w32 *tusec,w32 plussec,w32 plususec) {
w32 usecs, tsecs;
usecs= *tusec + plususec; tsecs= *tsec + plussec;
if(usecs>=1000000) {
  usecs= usecs-1000000;
  tsecs++;
}; *tsec= tsecs; *tusec= usecs;
}
/*----------------------------------------------*/
void SubSecUsec(w32 *tsec,w32 *tusec,w32 plussec,w32 plususec) {
w32 tsecs; int usecs;
usecs= *tusec - plususec; tsecs= *tsec - plussec;
if(usecs<0) {
  usecs= usecs+1000000;
  tsecs--;
}; *tsec= tsecs; *tusec= usecs;
}
/*----------------------------------------------
In: tsec,tusec >= prevtsec,prevtusec
rc: 0: tsec,tusec < prevtsec,prevtusec
    (tsec,tusec - prevtsec,prevtusec) in micsecs
    from 24.11.2011:
    0xffffffff -if (tsec-prevtsec)>2000)  why:0xffffffff/1000000.=4294.967295
*/
w32 DiffSecUsec(w32 tsec,w32 tusec,w32 prevtsec,w32 prevtusec) {
w32 usecdiff;
if(tsec >= prevtsec) {
  if((tsec-prevtsec)<2000) {
    if( tusec >= prevtusec) {
      usecdiff= 1000000*(tsec-prevtsec) + tusec-prevtusec;
    } else {
      if( tsec != prevtsec) {
        usecdiff= 1000000*(tsec-prevtsec-1) + (1000000-prevtusec)+tusec;
      } else {
        usecdiff= 0;
      };
    };
  } else {
    usecdiff= 0xffffffff;
  };
} else {
  usecdiff= 0;
};
/* was incorrect till 12.2.2010
*/
return(usecdiff);   /* in microseconds */
}

int loopspermic=0; /* cca 105 loops for vp110 1.2GHz */
/*----------------------------------------------*/
void micwait(int micsecs) {
int i,j;
if(loopspermic==0) {    /* calibrate */
  w32 sec,usec,prevsec,prevusec,diffusec;
  GetMicSec(&prevsec, &prevusec);
  for(i=1; i<10000; i++) {if (i>0) continue;};
  GetMicSec(&sec, &usec);
  diffusec= DiffSecUsec(sec,usec,prevsec,prevusec);
#ifdef CPLUSPLUS
  loopspermic= int(10000./diffusec);
#else
  loopspermic= 10000./diffusec;
#endif
  /* printf("loopspermic:%d\n",loopspermic);  */
};
for(j=0; j<micsecs; j++) {
  for(i=1; i<loopspermic; i++) {if (i>0) continue;};
};
}
unsigned int profSecs, profUsecs;
/*---------------------------------------------*/ void prtProfTime(char *name) {
unsigned int s_now, us_now, us;
char msg[100];
if(name==NULL) { 
  GetMicSec(&profSecs, &profUsecs);
  sprintf(msg,"prof_ts:cleared 0 us"); prtLog(msg);
} else {
  GetMicSec(&s_now, &us_now);
  us= DiffSecUsec(s_now, us_now, profSecs, profUsecs);
  sprintf(msg,"prof_ts:%s %d us", name, us); prtLog(msg);
};
}

