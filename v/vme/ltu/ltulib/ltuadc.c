/* ADCI.c
5.7.2004
from version >ac.rbf works only with front panel cabel connected with
BC comming through L1 data wire. With version <=ac.rbf should work
autonomously
27.10.2005
rndtest(): now PLL_RESET done after each change of BC_DELAY_ADD and
           average wait for end of PLL_RESET is printed
*/
#include <unistd.h>    /* usleep */
#include <stdio.h>
#include <sys/time.h>
#include "vmewrap.h"
#include "ltu.h"

#define nread 300
#define bit8 256
#define bit0_7 (1+2+4+8+16+32+64+128)
#define stable 10

double rnlx();
void setseeds(long,int);
/**-------------------------------------------------------*/
void setbcdelay(w32 delay, int ttcint) {
/* ttcint:  1: just set BC_DELAY_ADD (to be used for scan)
            0: set BC_DEALY_ADD taking into account correction
               for the clock when LTU is in global! 
*/
 //vmew32(BC_DELAY_ADD ,delay);
if(ttcint==1) {
  vmew32(BC_DELAY_ADD,delay);
} else {
  setBC_DELAY_ADD(delay);
};
}
w32 getbcstatus() {
 return 3&vmer32(BC_STATUS);
}
void pllreset() {
/*w32 seconds1,micseconds1, seconds2,micseconds2;
GetMicSec(&seconds1, &micseconds1);*/
vmew32(PLL_RESET, DUMMYVAL);
/*while(1) {
  w32 bcs;
  usleep(1);
  bcs= vmer32(BC_STATUS);
  if(bcs & BC_STATUSpll) break;
};
GetMicSec(&seconds2, &micseconds2);
diff=DiffSecUsec(seconds2, micseconds2, seconds1, micseconds1);*/
//return(diff); //micsecs spent during waiting for PLL lock (average: 5 micsecs)
}
void adcitest()
{
 struct timeval tval;
 struct timezone tz;
 long tsec0,tusec0;
 int i;
 int busy,value;
 w32 buffer[nread];
 long tt,time[nread];
 
 
 gettimeofday(&tval,&tz);
 tsec0=tval.tv_sec;
 tusec0=tval.tv_usec;
 
 for(i=0;i<nread;i++){
   if( (i % 10000) == 0) vmew32(ADC_START,0x0);
   buffer[i]=vmer32(ADC_DATA);
   gettimeofday(&tval,&tz);
   tt=(tval.tv_sec-tsec0)*1000000+(tval.tv_usec-tusec0);
   time[i]=tt;
 }
 for(i=0;i<nread;i++){
  busy= ((buffer[i]&bit8) == bit8);  /* bit 8 of buffer[i] */
  value=buffer[i]&bit0_7;
  if( (i % 20) == 0 ) printf(" SEQ TIME BUSY VALUE (Time in microsec) \n");
  printf("%4i %4d   %d   %i \n",i,(int)time[i],busy,value);
 }
}
/*----------------------------------------------------------------*/
int readadc() {
#define timeout 1000
 //int i=0;
 int value;
 vmew32(ADC_START,0x0);
 /*while(1) {
   if(i>timeout) {
   };
   if(((vmer32(ADC_DATA)&bit8) == 0))
   i++;
 }; */ usleep(10000);
  value = (vmer32(ADC_DATA)&bit0_7);
 /*printf("adc= %i\n",value);*/
 return value; 
}
/*---------------------------------------------------------------------------*/
int readadc_s()
{
 int value0,value1,i=0;
 value0=readadc();
 if(value0 == -1) return -1;
 while( ( (value1=readadc()) != value0) && (i<stable) ){
     if(value1 == -1) return -2;		 
     value0=value1;
     i++;
    }
 /*printf("i=%i value=%i \n",i,value0);*/   
 if(i>=stable) return -3;    
 return value1;
} 
void scan(int micseconds)
{
 int i,buf[32];
 int imax=0,imin=0;
 int val;
 for(i=0;i<32;i++){
  setbcdelay(i, 1); //Do not change TTC_INTERFACE word during scan !
  vmew32(PLL_RESET, DUMMYVAL);
   //usleep(10000);
  if(micseconds>0) usleep(micseconds);
  while(1) {
    w32 bcs;
    bcs= vmer32(BC_STATUS);
    if(bcs & BC_STATUSpll) break;
  };
   val=readadc_s();
   buf[i]=val;
   if(val>buf[imax]) imax=i;
   if(val<buf[imin]) imin=i;
   //printf("i=%i, val=%i \n",i,buf[i]);
 }; 
 for(i=0;i<32;i++) printf("%d ", buf[i]);
 printf("\n");
 //printf("max=%i %i    min=%i %i \n",imax,buf[imax],imin,buf[imin]); 
}
void getbcphase(int ltufo) {
w32 origbcdel,adcval;
// check global/toggling, return 256 if not (not done yet)...
if(ltufo==0) {
  origbcdel= vmer32(STDALONE_MODE)&1;
  if(origbcdel==1) {adcval=256; goto RET; };
};
origbcdel= vmer32(BC_DELAY_ADD);
vmew32(BC_DELAY_ADD, 0);
vmew32(ADC_SELECT, ltufo);
adcval= readadc_s();
vmew32(BC_DELAY_ADD, origbcdel);
RET:printf("%d\n", adcval); 
}
/*----------------------------------------------------------------------*/
void adctimeconst(w32 delay0,w32 delay1)
{
 int i,val;
 setbcdelay(delay0,0);
 usleep(100000);
 setbcdelay(delay1,0);
 for(i=0;i<10;i++){

  val=readadc();
  printf(" %i adc=%i \n",i,val);
  usleep(10000);
 }
 return ;
}
void rndtest()
{
#ifdef SIMVME
#define NMEASUREMENTS 10
#else
#define NMEASUREMENTS 101
#endif
 int i,value, all0mics,allmics;
 w32 delay;
 /* FILE *prn=fopen("WORK/bc.txt","w"); */
 w8 delays[NMEASUREMENTS];
 w8 vals[NMEASUREMENTS];
 setseeds(7,7); allmics=0; all0mics=0;
 for(i=0;i<NMEASUREMENTS;i++){
  w32 seconds1,micseconds1, seconds2,micseconds2,diff;
  //delay=32*rnlx();
  delay=(i+7)%32;
  setbcdelay(delay, 1); //Do not change TTC_INTERFACE word during scan !
  /* PLL_RESET always after dealy change then wait for PLL_LOCK: */
  GetMicSec(&seconds1, &micseconds1);
  vmew32(PLL_RESET, DUMMYVAL);
  /* first wait for 'unlocked' 
  while(1) {
    w32 bcs;
    bcs= vmer32(BC_STATUS);
    GetMicSec(&seconds2, &micseconds2);
    diff=DiffSecUsec(seconds2, micseconds2, seconds1, micseconds1);
    if( diff > 200000) break;
    if((bcs & BC_STATUSpll)==0) break;
  };
  all0mics=all0mics+diff;   */
  while(1) {
    w32 bcs;
    bcs= vmer32(BC_STATUS);
    if(bcs & BC_STATUSpll) break;
  };
  GetMicSec(&seconds2, &micseconds2);
  diff=DiffSecUsec(seconds2, micseconds2, seconds1, micseconds1);
  allmics=allmics+diff;
  value=readadc_s();
  /*  fprintf(prn,"%i %i \n",delay,value); */
  /*if(value < 0)  printf("%i \n",delay);
   if( (i % 10) == 0)printf("%i\n",i);*/
  delays[i]= delay; vals[i]= value;
  /*  printf("<%i> <%i> \n",delay,value); */
 };
 /* fclose(prn); */
 for(i=0;i<NMEASUREMENTS;i++){
   printf("<%i> <%i> \n",delays[i],vals[i]);
 };
 /* rndtest finished. 
    Average wait for PLL unlocked   
    Average wait for PLL  usual values are: <3.415842> <5.950495> 
    for both: sequential change (dealy=dealy+1) 
        or    random change (delay=32*rnlx() ) */
 printf("<%f> <%f>\n", 1.0*all0mics/NMEASUREMENTS,
   1.0*allmics/NMEASUREMENTS); 
}
/*********************************************************
void initmain() {
printf("initmain called.\n");
setseeds(3,3);
#ifdef SIMVME
printf("SIMVME active. Calling regfuns. \n");
regfuns();
#else
#ifdef VMERCC
printf("VMERCC active. \n");
#else
printf("OS not defined !. Exiting. \n");
exit(8);
#endif
#endif
}
*/
