/*
g++ -g -c -Wall -I/local/trigger/v/vmeb/vmeblib -I/usr/local/include -I$VMECFDIR/ttcmi rf2ttc.c
g++ rf2ttc.o -L/local/trigger/v/vmeb/vmeblib/linux_c -lvmeb -lpthread -L/lib/modules/daq -lvme_rcc -lrcc_error -lio_rcc -lcmem_rcc -lDFDebug -lm -o rf2ttc
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>     // nanosleep
#include "vmewrap.h"
#include "ttcmi.h"

void prtHelp() {
printf("Usage:\n\
rf2ttc.exe local | beam1 | ref | switch | status | clock\n\
where:\n\
local|beam1|ref   -switch the clock (to local, BC1 or ref)\n\
                     -wait 1sec and read LOL status on all boards.\n\
                      ok or lol is shown (i.e. if there was a loss of lock\n\
                      after the switch to the new clock)\n\
switch              - switch local/ref-> beam1   or    beam1 ->local\n\
status              -check if there was a LOL from last reading\n\
clock               -show current clock (local, beam1 or ref)\n\
\n\
TTCvx/beam1: 40.07931 MHz  (LHC freq. for lab precise clock source: 40.0785MHz)\n\
local:      40.07808 MHz\n\
TICG/ref:   LHC ramp: 40.07833 - 40.07887 MHz\n\
\n\
Seems the output is 40.0810 MHz (regardless of input source) when RF2TTC is not initialised properly \n");

}
void setbcorbitMain(int maino) {
w32 bcmain,orbmain;
char msg[300]; char daqlog[90];
bcmain= vmer32(BCmain_MAN_SELECT); orbmain= vmer32(ORBmain_MAN_SELECT);
/* if((maino==1) || (maino==2) || (maino==3)) {   // when LHC clock 
setCordeshift();
}; */
if(maino==1) {
  vmew32(BCmain_MAN_SELECT, 3);  /* 3: BC1 input */
  vmew32(ORBmain_MAN_SELECT, 0); /* 0: ORB1 input */
  sprintf(msg, "Global clock: %x/%x -> BEAM1/ORB1",bcmain,orbmain);
  strcpy(daqlog,"BEAM1");
} else if(maino==2) {
  vmew32(BCmain_MAN_SELECT, 2);  /* 2: BC2 input */
  vmew32(ORBmain_MAN_SELECT, 1); /* 1: ORB2 input */
  sprintf(msg, "Global clock: %x/%x -> BEAM2/ORB2",bcmain,orbmain);
  strcpy(daqlog,"BEAM2");
} else if(maino==3) {
  vmew32(BCmain_MAN_SELECT, 1);  /* 1: BCref input */
  vmew32(ORBmain_MAN_SELECT, 2); /* 2: internal BCmain synchronized orbit gen*/
  sprintf(msg, "Global clock: %x/%x -> BCREF/ORBlocal",bcmain,orbmain);
  strcpy(daqlog,"REF");
} else if(maino==4) {
  vmew32(BCmain_MAN_SELECT, 0);  /* 1: internal 40.078 MHz clock */
  vmew32(ORBmain_MAN_SELECT, 2); /* 2: internal BCmain synchronized orbit gen*/
  sprintf(msg, "Global clock: %x/%x -> BClocal/ORBlocal",bcmain,orbmain);
  strcpy(daqlog,"LOCAL");
} else if(maino==12) {
  vmew32(BCmain_MAN_SELECT, 3);  /* 3: BC1 input */
  vmew32(ORBmain_MAN_SELECT, 1); /* 1: ORB2 input */
  sprintf(msg, "Global clock: %x/%x -> BEAM1/ORB2",bcmain,orbmain);
  strcpy(daqlog,"BEAM1/ORB2");
} else {
  printf("Bad maino input. No action\n"); return;
};
// always, after clock change resynchronize DLL on RF2TTC:
//sprintf(msg, "%s + DLL resync", msg);
//sprintf(msg, "%s note: DLL NOT resynchronised", msg);
//DLL_RESYNC(0);        //vmew32(BC_DELAY25_GCR, 0x40);
}
/* T1122RRMM hexa (qpllstat binary)
T: TTCrx ready   i.e. 1 ok
BC1/2/Ref/main:  Error Locked, i.e. 01 ok

*/
w32 readstatus() {
w16 rc; w32 s1,s2,s3,s4,s5;
s1= vmer32(BC1_QPLL_STATUS);
s2= vmer32(BC2_QPLL_STATUS);
s3= vmer32(BCref_QPLL_STATUS);
s4= vmer32(BCmain_QPLL_STATUS);
s5= vmer32(TTCrx_status);
rc= ((s5&0x1)<<8) | ((s1&0x3)<<6) | ((s2&0x3)<<4) | ((s3&0x3)<<2) | (s4&0x3);
return(rc);
}
int main(int argc, char **argv)  {
int rc, vsp;
w32 rc32;
vsp=0;
//rc= vmxopenam(&vsp, "0x7000000", "0x7fc00", "A32"); corde
rc= vmxopenam(&vsp, (char *)"0xf00000", (char *)"0x100000", (char *)"A32");   // rf2ttc
if(rc!=0) exit(8);
if( (argc < 2) || (argc>2) ) {
  prtHelp(); exit(8);
};
if(strcmp(argv[1], "local")==0) {
  setbcorbitMain(4);
} else if(strcmp(argv[1], "beam1")==0) {
  setbcorbitMain(1);
} else if(strcmp(argv[1], "ref")==0) {
  setbcorbitMain(3);
} else if((strcmp(argv[1], "switch")==0) ||
    (strcmp(argv[1], "clock")==0)) {
  w32 mbc,mo;
  mbc= vmer32(BCmain_MAN_SELECT); 
  mo= vmer32(ORBmain_MAN_SELECT); /* 0,1,2: ORB1/ORB2/internal_orbit input */
  //printf("mbc mo:%d %d\n", mbc, mo);
  if( mbc==3 ) {          // beam1
    if(strcmp(argv[1], "switch")==0) {
      setbcorbitMain(4);   // beam1 -> local
    } else {
      printf("beam1\n");
    };
  } else if( mbc==1 ) {   // ref
    if(strcmp(argv[1], "switch")==0) {
      setbcorbitMain(1);   // ref-> beam1
    } else {
      printf("ref\n");
    };
  } else {
    if(strcmp(argv[1], "switch")==0) {
      setbcorbitMain(1);   // local-> beam1
    } else {
      printf("local\n");
    };
  };  
} else if(strcmp(argv[1], "status")==0) {
  ;
} else {
  prtHelp(); exit(8);
};
sleep(1);
rc32= readstatus()&0x3;   // BCmain: b'01' ok
//printf("status: 0x%x\n",rc32);
if( rc32 == 0x1 ) {
  printf("ok\n");
} else {
  printf("%d\n",rc32);
};
//printf("vmxclose rc:%d\n",vmxclose(vsp));
}

