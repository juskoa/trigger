#include <stdio.h>
#include "vmewrap.h"
#include "ctplib.h"
/* rc: >=0: ok, vme space rc opened
       -1: error opening vme space  
*/
int bobrOpen() {
int rc, vspbobr=-1;
char mibase[]= "0xb00000";
rc= vmxopenam(&vspbobr, mibase,"0x4000","A24");
if(rc!=0) { 
  printf("bobr vmeopen rc:%d\n", rc); rc= -1;
} else {
  rc= vspbobr;
};
return(rc);
}
void bobrClose(int vspbobr) {
  vmxclose(vspbobr);
}
#define AuxMessageInput 0x0c00
#define Message2shift 0x10000
#define BlockStatus 0x10
#define EnableDPRam 0x4

w32 bstread4(int vsp, w32 adr) {
int ix; w32 rc=0;
for(ix=0; ix<4; ix++) {
  rc= rc | ((vmxr32(vsp, adr+4*ix)&0xff)<<(8*ix));
};
return(rc);
}

/* vsp: opened BOBR vme space
   bstn: 1 or 2    (bst message)
   waitforpp: 0: do not wait for pp
             >0: wait number of seconds
rc: 0: ok
     !=0: error: 1:bad bstn   2:timeout
*/
int getlhcpp(int vsp, int bstn, Tlhcpp *lhcpp) {
//int getlhcpp(int vsp, int bstn, Tlhcpp *lhcpp, int waitforpp) {
w32 MessageBase; int ix; w32 shift;
if(bstn==1) {
  MessageBase= AuxMessageInput; shift=0; //waitaux(0);
} else if(bstn==2) {
  MessageBase= AuxMessageInput+Message2shift;
  shift= Message2shift; //shift= waitaux(Message2shift);
} else { return(1);} ;
/* ask for next BST message and wait util it is stored in auxRAM */
lhcpp->Byte54=0;
//GetMicSec(&sec,&usec);
vmxw32(vsp, shift+BlockStatus, EnableDPRam); 
vmxw32(vsp, shift+BlockStatus, 0x80 | EnableDPRam);
for(ix=0; ix<1000; ix++) {
  w32 stat;
  stat= vmxr32(vsp, shift+BlockStatus);
  if(stat&0x8000) goto OK;
};
return(2);
OK:
lhcpp->GPSusecs= bstread4(vsp, MessageBase);
lhcpp->GPSsecs= bstread4(vsp, MessageBase+4*4);
lhcpp->Byte54= vmxr32(vsp, MessageBase+54*4)&0xff;
//printf("bobr.c:Byte54:%x\n", lhcpp->Byte54);
return(0);
}
