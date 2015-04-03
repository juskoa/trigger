#include <stdio.h>
#include <string.h>
#include "vmewrap.h"
#define CTPMAIN
#include "ctp.h"
#include "vmeblib.h"

/*---------------------------------------------------------- readBICfile
op:
- read ~/root/NOTES/boardsincrate file
- fill in ctpboards[ix].vmever (indicating the presence of the board)
- check versions of CTP boards
*/
void readBICfile() {
FILE *con;
int nctp;
int ix; w32 code, vmever,sernum,boardver,adshift;
char line[MAXLINE];
if( ctpboards[1].vmever!=NOTINCRATE) {
  if( ctpboards[1].vmever!=0xa0) {
    printf("ERROR:readBICfile:bad L0 board. vme ver:0x%x (a0 expected)\n",
      ctpboards[1].vmever);
  };
  return;
};
printf("readBICfile: checking CTP boards in crate...\n");
if((con=fopen(BICfile,"r")) == NULL){
  printf("Cannot read %s file. \n", BICfile);
  return;
};
while(fgets(line,MAXLINE,con)!=NULL) {
  /* printf("readBICfiledbg: line:%s\n",line); */
  if(line[0]=='\n') continue;
  if(strncmp(line,"ltu",3)==0) {
    // for ltu do nothing (see initSSM())
    continue;
  } else if(strncmp(line,"fo",2)==0) {
    /* in BICfile, FO boards are placed in ascending order of their
     * base address. They have to have dials: 0-5 (for FO1-FO6).
       From 19.4.2007 we use dials 1-6 for FO1-FO6 */
    nctp= FO1BOARD + (line[7]-'0')-1;
    ctpboards[nctp].vmever=NICRATE;
  } else if(strncmp(line,"busy",4)==0) {
    ctpboards[0].vmever=NICRATE;       /* real one will be read later */
  } else if(strncmp(line,"l0",2)==0) {
    ctpboards[1].vmever=NICRATE;
  } else if(strncmp(line,"l1",2)==0) {
    ctpboards[2].vmever=NICRATE;
  } else if(strncmp(line,"l2",2)==0) {
    ctpboards[3].vmever=NICRATE;
  } else if(strncmp(line,"int",3)==0) {
    ctpboards[4].vmever=NICRATE;
  } else {
    printf("readBICfile: unknown line:%s\n",line);
  };
};
fclose(con);
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
    if((code==0x50) && (boardver>=0xc0)) {
      vmever=0xa0;   // LM0 board, force vmever to the standard one
      strcpy(ctpboards[ix].name, "lm0");
    };
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
      ctpboards[ix].name, ctpboards[ix].dial);
    ctpboards[ix].vmever= 0;
  };
};
printf("CTP boards in the crate (counters:%d):\n", NCOUNTERS);
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
    char msg1[40]="";
    if(((bcst&0x4)==0x4) && (ix==0)) {   
      //busy: always external orbit/BC signals checked?
      w32 lom;
      lom= vmer32(BUSY_ORBIT_SELECT)&0x2000;
      if(lom==0x2000) {
        strcpy(msg1,"Bit 0x4:ON,OK -local orbit used");
      } else {
        strcpy(msg1,"Bit 0x4:bad external Orbit.");
      };
    };
    sprintf(errnote,"%s BCstatus: 0x2 expected. %s", errnote, msg1);
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
