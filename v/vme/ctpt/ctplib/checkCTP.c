#include <stdio.h>
//#include <string.h>
#include "vmewrap.h"
#include "ctp.h"

w32 loadFPGA(int board);   //is in vmeblib

/* check/configure/print FPGAs versions for all the ctp boards */
void checkCTP() {
int ix; w32 code, vmever,sernum,boardver,adshift;
readBICfile();
readTables();
for(ix=0; ix<NCTPBOARDS; ix++) {
/*  if(notInCrate(ix)) continue;  -not here*/
  if(ctpboards[ix].vmever!=NICRATE) continue; 
  adshift=BSP*ctpboards[ix].dial;
#ifdef SIMVME
  ctpboards[ix].boardver= ctpboards[ix].lastboardver;
  ctpboards[ix].vmever= 0xa3; 
  code=ctpboards[ix].code;
  continue; 
#else
  code= 0xff&vmer32(CODE_ADD+adshift);
#endif
  /*printf("code:%x adshift:%x\n",code,adshift); fflush(stdout);  */
  if(code==ctpboards[ix].code) {
    vmever= 0xff&vmer32(VERSION_ADD+adshift);
    sernum= 0xff&vmer32(SERIAL_NUMBER+adshift);
    boardver= 0xff&vmer32(FPGAVERSION_ADD+adshift);
    ctpboards[ix].vmever= vmever;
    /*
      printf("--->%s (code:0x%x base:0x82%1x000) vmeFPGA:0x%x boardFPGA:0x%x SN:0x%x\n",
        ctpboards[ix].name,ctpboards[ix].code, ctpboards[ix].dial, 
	vmever, boardver, sernum);
      fflush(stdout);
    */
    if(boardver==0xff) {
      boardver= loadFPGA(BSP*ctpboards[ix].dial);
    };
    if(boardver>0x100) {
      printf("Board %s (base:0x82%1x000) not configured. Error:0x%x\n",
        ctpboards[ix].name,ctpboards[ix].dial, boardver);
    } else {
      ctpboards[ix].boardver= boardver;
      ctpboards[ix].serial= sernum;
    };
  } else {
    printf("Board %s (base:0x82%1x000) missing\n",
      ctpboards[ix].name,ix);
    ctpboards[ix].vmever= 0;
  };
};
printf("CTP boards in the crate:\n");
printf("   board code ser# base     vmeV boardV BCstatus\n");
for(ix=0; ix<NCTPBOARDS; ix++) {
  w32 bcst;
  char errnote[80];
  if(notInCrate(ix)) continue;
  adshift=BSP*ctpboards[ix].dial;
  bcst= vmer32(adshift+BC_STATUS)&0x7;
  errnote[0]='\0';
  if(ctpboards[ix].boardver != ctpboards[ix].lastboardver) {
    sprintf(errnote,"Bad boardV (%x expected).", ctpboards[ix].lastboardver);
  };
  if(bcst!= 2) {
    sprintf(errnote,"%s Bad BCstatus (0x2 expected)", errnote);
  };
  /*
  printf("%d:%s \t(code:0x%x serial:0x%x base:0x82%1x000) vmeFPGA:0x%x boardFPGA:0x%x\n",
    ix, ctpboards[ix].name,ctpboards[ix].code, ctpboards[ix].serial, 
    ctpboards[ix].dial, ctpboards[ix].vmever, ctpboards[ix].boardver);
    */
  printf("%2d:%5s 0x%x %4d 0x82%1x000 0x%x 0x%x   %x     %s\n",
    ix, ctpboards[ix].name,ctpboards[ix].code, ctpboards[ix].serial, 
    ctpboards[ix].dial, ctpboards[ix].vmever, ctpboards[ix].boardver,bcst,
    errnote);
};
}

