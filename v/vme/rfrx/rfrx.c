/*BOARD rfrx 0x400000 0x100 A24 */
/* This sw is used to setup 1 RFRX board in alidcsvme056 crate (Antonello):
RFRX  0x300000 A24

Note: rfrx boards in ttcmi crate are setup directly in ttcmi/ttcmi.c

fibres from LHC connected to RFRX board in alidcsvme056 (base:0x300000):
RFRX
?BCREF
?BC1  
?ORB1 
*/

/*REGSTART16 */
//RFRX board:
#define ch1_ref 0x12
#define ch2_ref 0x14
#define ch3_ref 0x16
#define ch1_freq_low 0x18
#define ch1_freq_high 0x1a
//#define ch2_freq_low 0x1c
//#define ch2_freq_high 0x1e
//#define ch3_freq_low 0x20
//#define ch3_freq_high 0x22
#define ident_id 0x8
#define card_id 0x24
#define board_id 0x3a
/*REGEND */

#include <string.h>
#include <stdio.h>
#include <unistd.h>   //usleep
#include "vmewrap.h"
//#include "vmeblib.h"
//#include "../ctp/ctplib/ctplib.h"
//#include "ctplib.h"
//#include "ttcmi.h"

/* HIDDEN primitives */

extern int quit;

/*FGROUP
- set RF board (0x5 for BC and 0x70 for Orbit)
*/
void writeall() {
int ix,rc1,rc2,vsp3,vsp4;
w32 adrpol, adrlen;
w16 refsBC[3]={0x5, 0x5, 0x70};
w16 refsOrbit[3]={0x5, 0x5, 0x70};
/*vsp3=-1; rc1= vmxopenam(&vsp3, "0x300000", "0x100", "A24");
printf("vmxopenam 0x300000 rc:%d 0x400000\n", rc1); */
for(ix=0; ix<3; ix++) {
  adrpol= ch1_ref + ix*0x2;
  //vmex16(vsp3, adrpol, refsBC[ix]);
  vmew16(adrpol, refsBC[ix]);
};
/*rc1= vmxclose(vsp3);
printf("vmxclose 0x300000 rc:%d 0x400000 rc:%d\n", rc1, rc2); */
}
void printRFRX() {
//void printRFRX(char *rfrxbase) {
int ix,rc,vsp;
float frekvs[3];
//vsp=-1; rc= vmxopenam(&vsp, rfrxbase, "0x100", "A24");
printf("RFRX: ch1/2/3_ref:0x%x 0x%x 0x%x\n",
  vmer16(ch1_ref), vmer16(ch2_ref), vmer16(ch3_ref));
//  vmxr16(vsp, ch1_ref), vmxr16(vsp, ch2_ref), vmxr16(vsp, ch3_ref));
printf("    frekvch1/2/3: ");
for(ix=0; ix<3; ix++) {
  w32 fhl;
  w16 flow, fhigh;
  flow= vmer16(ch1_freq_low+(ix*4));
  fhigh= vmer16(ch1_freq_high+(ix*4));
  fhl= flow | (fhigh <<16);
  frekvs[ix]= (80*16*22)/(flow+ (fhigh*65536.));
  printf("%d=%fMHz ", fhl, frekvs[ix]);
}; printf("\n");
//rc= vmxclose(vsp);
}
/*FGROUP
ch1_ref:0x5
ch2_ref:0x5
ch3_ref:0x70
*/
void readall() {
printRFRX();
}
void initmain() {
printf("RFRX initmain called...\n");
}
void boardInit() {
writeall();
printf("boardInit:\n");
printRFRX();
}
void endmain() {
}

