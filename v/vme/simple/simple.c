/*BOARD simple 0x829000 0xd000 A24
*/
/*REGSTART32 */
#define CODE_ADD      0x4     /* board type (0x56 for LTU) */
#define SERIAL_NUMBER 0x8     /* unique serial number  of the board */
#define VMEVERSION_ADD   0xc
#define FPGAVERSION_ADD 0x80 /* board's FPGA version */
#define L0_CONDITIONr2 0x400
#define L0_INVERTr2 0x600
#define L0_VETOr2 0x800
#define SCOPE_A_FRONT_PANEL 0x244  /* LM0 only */
#define SCOPE_B_FRONT_PANEL 0x248  /* LM0 only */

/*REGEND */

extern int quit;
#include <string.h>
#include <stdio.h>
#include <unistd.h>   //usleep
#include "vmewrap.h"
#include "vmeblib.h"
//#include "vmeblib.h"
extern char BoardName[];
extern char BoardBaseAddress[];
extern char BoardSpaceLength[];
extern char BoardSpaceAddmod[];

/* FGROUP
int example(int n, char *string, char c)
int: only >=0
string: \"abc\"
char: 'ch'
float: not supported as parameter
*/
int example(int n, char *strg, char ch) {
//printf("number:%d fpn:%f strg:%s c:%c\n", n,fpn, strg, c);
printf("number:%d strg:%s c:%c\n", n, strg, ch);
return(n);
}
/* FGROUP
float not suported neither in function result
*/
float fexa(int ifpn) {
float rcf;
printf("fexa.ifpn:%d returning float (not supported!)...\n",ifpn); rcf=ifpn;
return(rcf);
}
/*FGROUP
address: rel. adress (4, 8, 12,... for 32 bits words readings)
loops: 0: endless loop
value: 0: read + print
       1: read only
      >1: write, no print
mics: mics between vme reads/writes. 0: do not wait between vme r/w
*/
void vmeloop(w32 address, int loops, w32 value, int mics) {
int todoloops= loops;
while(1) {
  if(loops>0) {
    if(todoloops>0) {
      todoloops--;
    } else {
      break;
    };
  };
  if(value<=1) {
    w32 val;
    val= vmer32(address);
    if(value==1) {
      printf("read:0x%x %u\n", val, val); 
      fflush(stdout);   //nebavi
    };
  } else {   
    vmew32(address, value);
  };
  if(mics>0) micwait(mics);
};
}
/*FGROUP
Write/read random value into array of registers

fromaddr: first address to be tested
Nregs:    number of consecutive addresses to be tested
bitmask:  e.g. 0xff -> test only bits 7..0
loops:    number of loops over the array of registers
*/
void rndtest(w32 fromaddr, int Nregs, w32 bitmask, int loops) {
int nn=0, nerrors=0;
setseeds(7,3);
while(1) {
  int ixreg; w32 address, val, val2;
  for(ixreg=0; ixreg<Nregs; ixreg++) {
    val= rounddown(bitmask * rnlx()); val= bitmask&val;
    address= fromaddr+(ixreg*4);
    vmew32(address, val);
    val2= vmer32(address)&bitmask;
    if((val!= val2) || (loops==(nn+1))) { // print last loop
      printf("Addr:0x%x Written:0x%x Read:0x%x\n", address, val, val2);
      if(val!=val2) nerrors++;
    }; 
    if(nerrors>10) goto STOPTEST;
  };
  nn++;
  if(nn>=loops) break;
};
STOPTEST: 
printf("Loops:%d errors:%d\n", nn, nerrors);
return;
}
void initmain() {
printf("Here initmain. BoardName:%s\n\
BoardBaseAddress:%s BoardSpaceLength%s BoardSpaceAddmod:%s\n",
  BoardName, BoardBaseAddress, BoardSpaceLength, BoardSpaceAddmod);
micwait(1);   // calibrate micwait
}
void boardInit() {
printf("boardInit called...\n");
}
void endmain() {
printf("endmain called...\n");
}

