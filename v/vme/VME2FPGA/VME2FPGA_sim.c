#include "vmesim.h"
#include <sys/time.h>
#include "vmefpga.h"

#define FmSize 1024*1024
/* Flesh memory in Hardware */
w8 FlashMemHard[FmSize];
int FMindex=0;
/* Time measurements   */
struct timeval tval;
struct timezone tz;
long tsec,tusec;
long BDelay=0;
/* Memory for history */
int ComHis[6];
int CHindex=0;
int erase[6]={0x10,0x55,0xaa,0x80,0x55,0xaa};
/* FPGA status */
long tsecF,tusecF;
long FDelay=0;
/**********************************************************************/
/*  Writing and reading Flash Memory with address Increase*/
int fadr1(w8 *simspace, w32 simlen, w32 offset, w32 rw) {
int rc=8;
w32 *ptr;
w32 value;
/*
printf("fadr1:%x %x %x %x\n", simspace, simlen, offset, rw);
printf("rw&VMEDAW: %x rw: %x VMEDAW: %x \n", rw&VMEDAW,rw,VMEDAW);
*/
ptr= (w32 *)&simspace[offset];
if( (rw&VMEDAW) == VMEDAW){
  value = *(w32 *)&simspace[offset];
  FlashMemHard[FMindex]=value;
  FMindex=FMindex+1;
/*  printf("Writing to FM wincr FMindex=%i value=%i \n",FMindex,value); */
  BDelay=9;
}else{
 *ptr=FlashMemHard[FMindex];
/*  printf("Reading FM %d  at FMindex %i \n",*ptr,FMindex); */
  FMindex=FMindex+1;
  BDelay=1; 

}
vmew32(FlashStatus,0x0);
gettimeofday(&tval,&tz);
tsec=tval.tv_sec;
tusec=tval.tv_usec;

return(rc);
}
/*------------------------------------------------------------------*/
/*  Writing and reading Flash Memory without address Increase*/
void CHindexplus()
{
 if(CHindex == 5) CHindex=0; else CHindex=CHindex+1;
}
void CHindexmin()
{
 if(CHindex == 0) CHindex=5; else CHindex=CHindex-1;
}

int fadr4(w8 *simspace, w32 simlen, w32 offset, w32 rw) {
int rc=8,i,chind,ok;
w32 *ptr;
w32 value;
/*
printf("fadr1:%x %x %x %x\n", simspace, simlen, offset, rw);
printf("rw&VMEDAW: %x rw: %x VMEDAW: %x \n", rw&VMEDAW,rw,VMEDAW);
*/
ptr= (w32 *)&simspace[offset];
if( (rw&VMEDAW) == VMEDAW){
  value = *(w32 *)&simspace[offset];
/*  printf("Writing to FM woincr FMindex=%i value=%i \n",FMindex,value);*/
  ComHis[CHindex]=value;
  CHindexplus();
  chind=CHindex;
  ok=1;
  for(i=0;i<6;i++){
   CHindexmin();
   ok=ok*(ComHis[CHindex] == erase[i]);
/*   printf("ok=%i\n",ok); */
  }
  CHindex=chind;
  if(ok == 1){ 
    BDelay=10000000;
    for(i=0;i<FmSize;i++)FlashMemHard[i]=0xff;
    }else BDelay=10;
}else{
 *ptr=FlashMemHard[FMindex];
  BDelay=5; 
/*  printf("Reading FM %d \n",*ptr); */
}
vmew32(FlashStatus,0x0);
gettimeofday(&tval,&tz);
tsec=tval.tv_sec;
tusec=tval.tv_usec;
return(rc);
}
/*-------------------------------------------------------------------------*/
/*     Clearing FM address                                        */
int fadr2(w8 *simspace, w32 simlen, w32 offset, w32 rw) {
int rc=8;
if( (rw&VMEDAW) == VMEDAW){
 FMindex=0;
/*  printf("FMindex=%i \n",FMindex); */
}
return(rc);
}
/*-------------------------------------------------------------------------*/
/*     FlashStatus                                        */
int fadr3(w8 *simspace, w32 simlen, w32 offset, w32 rw) {
int rc=8;
w32 *ptr;
long tt;

if( (rw&VMEDAR) == VMEDAR){
  if (BDelay == 0) return 0;
  rc= gettimeofday(&tval,&tz);
  tt=(tval.tv_sec-tsec);
  if(tt < 0) {
    printf("Problem with time in simulation. Exiting. \n");
    exit(1);
  }
  tt=tt*1000000+(tval.tv_usec-tusec);
/*  printf("GetStatFM; tt=%d BDelay=%d ",tt,BDelay); */
  if(tt > BDelay){
/*    printf(" BDELAY UPDATED \n"); */
    BDelay=0;
    ptr= (w32 *)&simspace[offset];
    *ptr=0xffff;
  }
}
return(rc);
}
/*-------------------------------------------------------------------------*/
/*     FPGA Status                                        */
int fadr5(w8 *simspace, w32 simlen, w32 offset, w32 rw) {
int rc=8;
w32 *ptr;
long tt;

if( (rw&VMEDAR) == VMEDAR){
  if (FDelay == 0) return 0;
  rc= gettimeofday(&tval,&tz);
  tt=(tval.tv_sec-tsecF);
  if(tt < 0) {
    printf("Problem with time in simulation. Exiting. \n");
    exit(1);
  }
  tt=tt*1000000+(tval.tv_usec-tusecF); 
/*  printf("GetStatFPGA; tt=%d FDelay=%d ",tt,FDelay); */
  if(tt > FDelay){
/*    printf(" FDELAY UPDATED \n"); */
    FDelay=0;
    ptr= (w32 *)&simspace[offset];
    *ptr=0xffff;
  }
}
return(rc);
}
/*-------------------------------------------------------------------------*/
/*     FPGA Config Start                                       */
int fadr6(w8 *simspace, w32 simlen, w32 offset, w32 rw) {
int rc=8;
if( (rw&VMEDAW) == VMEDAW){
  rc= gettimeofday(&tval,&tz);
  tsecF=tval.tv_sec;
  tusecF=tval.tv_usec;
  FDelay=300000;
  vmew32(ConfigStatus,0x0);
}
return(rc);
}

/*************************************************************************/
void regfuns() {
int rc,i;
char c;
printf("VME2FPGA: regfuns called\n");
/* problem: we can't use symbolic VME regs names (they are in .cf file) */
/*rc= vmesimreg((int (*)())fadr1, 0, 0x80, "rw");- compiles without warning*/

c='x';
for(i=0;i<FmSize;i++)FlashMemHard[i]=0xff;
rc=gettimeofday(&tval,&tz);
tsec=tval.tv_sec;
tusec=tval.tv_usec;
printf("TIME rc=%i sec=%i usec=%i \n",rc,tsec,tusec);

rc= vmesimreg((TspecVmeF)fadr1, 0, FlashAccessIncr, 0); 
printf("VME2FPGA_sim: fadr1() registered, rc:%d\n");

rc= vmesimreg((TspecVmeF)fadr2, 0, FlashAddClear, 0);
printf("VME2FPGA_sim: fadr2() registered, rc:%d\n");

rc= vmesimreg((TspecVmeF)fadr3, 0, FlashStatus, 0);
printf("VME2FPGA_sim: fadr3() registered, rc:%d\n");

rc= vmesimreg((TspecVmeF)fadr4, 0, FlashAccessNoIncr, 0);
printf("VME2FPGA_sim: fadr4() registered, rc:%d\n");

rc= vmesimreg((TspecVmeF)fadr5, 0, ConfigStatus, 0);
printf("VME2FPGA_sim: fadr5() registered, rc:%d\n");

rc= vmesimreg((TspecVmeF)fadr6, 0, ConfigStart, 0);
printf("VME2FPGA_sim: fadr6() registered, rc:%d\n");

}

