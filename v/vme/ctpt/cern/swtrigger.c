#include <stdio.h>
#include "vmewrap.h"
#include "ctp.h"
#define TIMEOUT 10000
/*---------------------------------------------------------------CountTime()
  Count time from the last function call.
*/
w32 CountTime(){
 static w32 seconds=0;
 static w32 micsec=0;
 w32 sec,mic,time;
 GetMicSec(&sec,&mic);
 time=(sec-seconds)*1000000+(mic-micsec);
 seconds=sec;
 micsec=mic;
 return time;
}
/*FGROUP DebCon
*/
void mysleep(w32 delta){
 w32 seconds,micsec,sec,mic,time;
 GetMicSec(&seconds,&micsec);
 time=0;
 while(time<delta){
   GetMicSec(&sec,&mic);
   time=(sec-seconds)*1000000+(mic-micsec);
 }
}

/*FGROUP DebCon---------------------------------------------------setswtrig
 Set asynchr. sw trigger
 trigtype: 'a' - asynchronous
           's' - synchronous, noncalibration
           'c' - callibration
 BC : BC for synchronous trigger
*/
int setswtrig(char trigtype,w32 BC,w32 detectors){
 w32 word;
 if(BC>3563){
  printf("setswtrig: BC>3563 %i \n",BC);
  return 1;
 }
 // L0 board
 //     P/F      BCM4   BCM3    BCM2    BCM1
 word=(1<<18)+(1<<17)+(1<<16)+(1<<15)+(1<<14);
 switch(trigtype){
  case 'a':
       word=word+0;
       //printf("setswtrig: asynchr trigger 0x%x \n",word);
       break;
  case 's':
       word=word+(1<<12)+BC;
       //printf("setswtrig: synchr trigger 0x%x \n",word);
       break;
  case 'c':
       word=word+(1<<12)+(1<<13)+BC;
       //printf("setswtrig: calib trigger 0x%x \n",word);
       break;
  default:
       printf("setswtrig: unknown type of trigger %c \n",trigtype);
       return 1;
 }
 vmew32(L0_TCSET,word);
 // L1 board
 word=(1<<18);
 vmew32(L1_TCSET,word);
 // L2 board
 //word=(1<<24)+0xaaaaaa;
 word=(1<<24)+detectors;
 vmew32(L2_TCSET,word);
 return 0;
}
/*FGROUP DebCon---------------------------------------------clearflags()
 * Clear flags on all levels
*/
void clearflags(){
 vmew32(L0_TCCLEAR,0x0);
 vmew32(L1_TCCLEAR,0x0);
 vmew32(L2_TCCLEAR,0x0);
}
/*---------------------------------------------------------getlxackn
*/
w32 getl0ackn(){
 return (vmer32(L0_TCSTATUS)&0x8)/0x8;
}
w32 getl1ackn(){
 return vmer32(L1_TCSTATUS)/0x8;
}
w32 getl2ackn(){
 return vmer32(L2_TCSTATUS);
}
w32 getPPrqst(){
 return (vmer32(L0_TCSTATUS)&0x2)/0x2;
}
w32 getL0rqst(){
 return (vmer32(L0_TCSTATUS)&0x4)/0x4;
}
/*---------------------------------------------------------startswtrig
 * - start sw trig
 * - follows acknowledgement on L0,L1,L2 levels
 * return 0 = fail
          1 = killed at l0
          2 = killed at l1
          3 = l2r
          4 = l2a
          5 = PP timeout
          6 = L0 timeout
*/
int startswtrig(char trigtype){
 w32 flag;
 int ret,i;
 //w32 itime=0,time[20];
 vmew32(L0_TCSTART,0);
 //time[itime++]=CountTime();
 // 
 if(trigtype == 'c'){
  i=0;
  while(getPPrqst() && i<TIMEOUT)i++;
  if(i>=TIMEOUT){
      clearflags(); 
      ret=5;
      goto RET;
  };
 }
 if(trigtype == 's' || trigtype == 'c'){
  i=0;
  while(getL0rqst() && i<TIMEOUT)i++;
  if(i>=TIMEOUT){
    clearflags(); 
    ret = 6;
    goto RET;
  } 
 }
 ret=3;
 //time[itime++]=CountTime();
 //printf("l0ackn: %i \n",getl0ackn());
 i=0;
 while(!getl0ackn() && (i<TIMEOUT))i++;
 //time[itime++]=CountTime();
 if(i>=TIMEOUT){
   clearflags();
   ret = 1;
   goto RET;
 }
 //mysleep(10); // wait for L1 trigger  
 //printf("l1ackn: %i \n",getl1ackn());
 i=0;
 while(!getl1ackn() && i<TIMEOUT)i++;
 //time[itime++]=CountTime();
 if(i>=TIMEOUT){
   clearflags();
   ret=2;
   goto RET;
 }
 flag=getl2ackn();
 mysleep(120);
 i=0;
 while((flag != 8) && (flag != 4) && i<TIMEOUT){
   flag=getl2ackn();
   i++;
 }
 //printf("l2ackn: %i \n",flag);
 if(i >= TIMEOUT){
  printf("startswtrig: Timeout at l2ackn. \n");
  clearflags();
  ret = 0;
  goto RET;
 }
 if(flag == 4){ 
   clearflags();
   ret = 3;
 }else if(flag == 8){
  clearflags();
  ret = 4;
 }else{
    printf("startswtrig: FAIL, flag=%i %i I should never be here.\n",flag,i);
    return 0; // there should by l2a or l2r 
 }
 RET: 
 // Time
 /* time[itime++]=CountTime();
 printf("startswtrig: TIMES ");
 for(i=0;i<itime;i++)printf(" %i ",time[i]);
 printf("ret= %i \n",ret);
 */ 
 return ret;
}
/*---------------------------------------------------------getstatswtrig
 *  get test class busy flag 
*/
w32 getstatswtrig(){
 w32 busy;
 busy=vmer32(L0_TCSTATUS);
 printf("swtrig: status=%x \n",busy);
 busy=busy&0x10;
 return busy;
}
/*FGROUP DebCon
   Generate infinite loop of software triggers
   Code same as swtrg
   trigtype:
             'c' - calibration
             's' - synchronous
             'a' - asynchronous
   BC : bunch crossing for the case of 'c' and 's'
   detectors: integer for detectorcode=list of detectors in CTPreadout
*/
int swtrginf(char trigtype, w32 BC,w32 detectors){
 int flag,itr=0;
 int l0=0,l1=0,l2a=0,l2r=0;
 if(getstatswtrig()){
  printf("swtrg: TC busy ?? \n");
  return 0;
 }
 setswtrig(trigtype,BC,detectors);
 while((flag=startswtrig(trigtype))){
    if(flag == 1)l0++;
    else if(flag == 2)l1++;
    else if(flag == 3)l2r++;
    else if(flag == 4)l2a++;
    else {
      printf("swtrg: unexpected flag %i \n",flag);
      return 0;
    }
    itr++;
    mysleep(1000);
    if(!(itr % 10000))printf("l0,l1,l2r,l2a: %i %i %i %i \n",l0,l1,l2r,l2a);
 };
 printf("swtrg: %i %c triggers generated. \n",itr,trigtype);
 printf("l0,l1,l2r,l2a: %i %i %i %i \n",l0,l1,l2r,l2a);
 // return i;
 return l2a;
}
/*FGROUP DebCon
   Generate n swtrig sequence
   if n=0 -> infinite loop (dont run in graph mode)
   trigtype:
             'c' - calibration
             's' - synchronous
             'a' - asynchronous
   BC : bunch crossing for the case of 'c' and 's'
   detectors: integer for detectorcode=list of detectors in CTPreadout
*/
int swtrg(int n,char trigtype, w32 BC,w32 detectors){
 int flag,itr=0;
 int l0=0,l1=0,l2a=0,l2r=0;
 if(getstatswtrig()){
  printf("swtrg: TC busy ?? \n");
  return 0;
 }
 setswtrig(trigtype,BC,detectors);
 while(((itr<n) && ((flag=startswtrig(trigtype))))){
      if(flag == 1)l0++;
      else if(flag == 2)l1++;
      else if(flag == 3)l2r++;
      else if(flag == 4)l2a++;
      else {
        printf("swtrg: unexpected flag %i \n",flag);
        return 0;
      }
      itr++;
 };
 printf("swtrg: %i %c triggers generated. \n",itr,trigtype);
 printf("l0,l1,l2r,l2a: %i %i %i %i \n",l0,l1,l2r,l2a);
 // return i;
 return l2a;
}
