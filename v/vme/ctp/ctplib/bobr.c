#include <stdio.h>
#include "vmewrap.h"
#include "../../bobr/bobr.h"
#include "ctp.h"
#include "vmeblib.h"
#include "ctplib.h"
/* rc: >=0: ok, vme space rc opened
       -1: error opening vme space  
*/
int bobrOpen() {
int rc, vspbobr=-1;
char mibase[]= "0xb00000";
rc= vmxopenam(&vspbobr, mibase,"0x14000","A24");
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
w32 bstread2(int vsp, w32 adr) {
int ix; w32 rc=0;
for(ix=0; ix<2; ix++) {
  rc= rc | ((vmxr32(vsp, adr+4*ix)&0xff)<<(8*ix));
};
return(rc);
}

/* ask for next BST message and wait util it is stored in auxRAM
*/
void waitaux(w32 mb) {
int ix;
vmew32(mb+BlockStatus, EnableDPRam); vmew32(mb+BlockStatus, 0x80 | EnableDPRam);
for(ix=0; ix<1000; ix++) {
  w32 stat;
  stat= vmer32(mb+BlockStatus);
  if(stat&0x8000) break;
};
// usual value of ix is ~110 (1000 corresponds certainly to timeout)
//printf("waitaux:%d vmer32-cycles\n", ix);
}
void readBSTbix(int vsp, w32 *bstmsg, w32 MessageBase, int bix) {
// bix not used now (might be if we want to read both messages simultaneously).
bix=bix;
// updated each turn:
bstmsg[iGPSusecs]= bstread4(vsp, MessageBase);
//printf("readBSTbix:GPSusecs: %x \n",bstmsg[iGPSusecs]); 
bstmsg[iGPSsecs]= bstread4(vsp, MessageBase+4*4);
//printf("readBSTbix:GPSsecs: %x \n",bstmsg[iGPSsecs]); 
bstmsg[iBSTstatus]= vmxr32(vsp, MessageBase+17*4)&0xff;
bstmsg[iTurnCount]= bstread4(vsp, MessageBase+18*4);
// updated once per second:
bstmsg[iLHCFill]= bstread4(vsp, MessageBase+22*4);
bstmsg[iBeamMode]= bstread2(vsp, MessageBase+26*4);
bstmsg[iParticleType1]= vmxr32(vsp, MessageBase+28*4)&0xff;
bstmsg[iParticleType2]= vmxr32(vsp, MessageBase+29*4)&0xff;
bstmsg[iBeamMomentum]= bstread2(vsp, MessageBase+30*4);
bstmsg[iTotalIntensity1]= bstread4(vsp, MessageBase+32*4);
bstmsg[iTotalIntensity2]= bstread4(vsp, MessageBase+36*4);
//printf("readBSTbix:BeammMode: %x \n",bstmsg[iBeamMode]); 
}
/*bstn: 1: BST1     2: BST2
        3: auxbst1  4: auxbst2
1,2: just read BST message
3,4: request next BST message, wait until it is stored in AuxDP Ram, read
*/
void nextBSTdcs(int vspbobr, w32 *bstmsg, int bstn) {
w32 MessageBase; int bix;
if(bstn==1) {
  MessageBase= MessageInput; bix=0;
} else if(bstn==2) {
  MessageBase= MessageInput+Message2shift; bix=1;
} else if(bstn==3) {
  MessageBase= AuxMessageInput; bix=2; waitaux(0);
} else if(bstn==4) {
  MessageBase= AuxMessageInput+Message2shift; bix=3; waitaux(Message2shift);
} else { return;} ;
readBSTbix(vspbobr, bstmsg, MessageBase, bix);
}

/*In:
vsp: opened BOBR vme space
bstn: 1 or 2    (bst message)
waitforpp: 0: do not wait for pp
           >0: wait number of seconds (timed out)
Out:
rc: 0: ok, BST message read into lhcpp, and LHCpp detected in (if requested)
  !=0: error: 1:bad bstn   
              2:timeout: BST not coming
              3:LHCpp timeout (didn't come before waitforpp secs)
*/
int getlhcpp(int vsp, int bstn, int waitforpp, Tlhcpp *lhcpp) {
w32 MessageBase, sec,usec; int ix, difsec; w32 shift;
if(bstn==1) {
  MessageBase= AuxMessageInput; shift=0; //waitaux(0);
} else if(bstn==2) {
  MessageBase= AuxMessageInput+Message2shift;
  shift= Message2shift; //shift= waitaux(Message2shift);
} else { return(1);} ;
/* ask for next BST message and wait util it is stored in auxRAM */
lhcpp->Byte54=0;
GetMicSec(&sec,&usec); difsec=0;
vmxw32(vsp, shift+BlockStatus, EnableDPRam); 
vmxw32(vsp, shift+BlockStatus, 0x80 | EnableDPRam);
while(difsec<waitforpp) {
  w32 sec2, usec2;
  for(ix=0; ix<1000; ix++) {
    w32 stat;
    stat= vmxr32(vsp, shift+BlockStatus);
    //if((stat&0x8000) && (waitforpp==0)) goto OK;
    if(stat&0x8000) {   // bst received
      lhcpp->Byte54= vmxr32(vsp, MessageBase+54*4)&0xff;
      if(waitforpp>0)  {   // wait for LHCpp
        if((lhcpp->Byte54 & 0x1)== 0x1) {     // ok LHCpp
          goto OK;
        } else {   // in this BST, LHCpp not arrived yeat
          GetMicSec(&sec2,&usec2);
          difsec= DiffSecUsec(sec2,usec2,sec,usec);
          if(difsec>=waitforpp) {
            return(3);   // LHCpp timeout
          };
        };
      } else {
        goto OK;   // the wait for LHCpp not requested
      };
    };
  };
  return(2);
};
OK:
lhcpp->GPSusecs= bstread4(vsp, MessageBase);
lhcpp->GPSsecs= bstread4(vsp, MessageBase+4*4);
//printf("bobr.c:Byte54:%x\n", lhcpp->Byte54);
return(0);
}
/*------------------------------------------------- w32 findorbit_ctp()
Operation:
1. read orbit until 2 equal readings
2. read orbit until changed (+1) between 2 readings
3. save orbit, time
RC:
0-0xffffff -current orbit, just started
*/
w32 findorbit_ctp() {
w32 o1,o2,orbit2,o1p1;
FINDNEXT: while(1) {
  o1= vmer32(L2_ORBIT_READ); o2= vmer32(L2_ORBIT_READ);
  if(o1==o2) break;
}; o1p1=o1+1;
while(1) {
  orbit2= vmer32(L2_ORBIT_READ);
  if(orbit2 == o1) continue;
  if(orbit2 == o1p1) {
    break;
  } else {
    //printf("findorbit:%u %u\n", o1, orbit2);  //quite often (>1/s rate)
    goto FINDNEXT;
  };
};
return(orbit2);
}
/*-------------------------------------------------
BSTturn= (CTPorbit - bst2ctp) | bst3124  was before 26.4.
BSTturn= CTPorbit + bst2ctp
example:
ctp:b95da4 bst:22f860f3 ctp-bst orbit: -4129615=ffc0fcb1 bst31..24:22000000
ctp:c3b4a0 bst:2302b7e7 ctp-bst orbit: 12647609=c0fcb9 bst31..24:23000000
*/
void getlhc2ctpOrbit(int vspbobr, w32 *bst2ctp, w32 *bst3124) {
w32 l2orbit,bstorbit; w32 diff32;
l2orbit= findorbit_ctp();   // find change
bstorbit= bstread4(vspbobr, MessageInput+18*4);
//diff32= dodif32(l2orbit, bstorbit);
//diff32= (bstorbit&0xffffff) - l2orbit;
//diff32= l2orbit - (bstorbit&0xffffff);
diff32= bstorbit-l2orbit;
/*printf("ctp:%x bst:%x bst-ctp orbit: %d=%x bst31..24:%x\n", 
  l2orbit,bstorbit, diff32,diff32, bstorbit&0xff000000);  */
*bst2ctp= diff32; *bst3124= bstorbit&0xff000000;
}

