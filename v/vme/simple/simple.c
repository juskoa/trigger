/*BOARD simple 0x829000 0xd000 A24
*/
/*REGSTART32 */
#define CODE_ADD      0x4     /* board type (0x56 for LTU) */
#define SERIAL_NUMBER 0x8     /* unique serial number  of the board */
#define VMEVERSION_ADD   0xc
#define FPGAVERSION_ADD 0x80 /* board's FPGA version */
/*REGEND */

extern int quit;
#include <string.h>
#include <stdio.h>
#include <unistd.h>   //usleep
#include "vmewrap.h"
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

