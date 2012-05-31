/*BOARD bobr 0xb00000 0x14000 A24 
  BOARD bobr 0xb00000 0x4000 A24 
                      0x14000 -for simultaneous access to both BST? (tbtested)
 see aj@11:bobr/reame
 see vmeb/atlas/make_mf.py,
               /BOBR/src
               /BOBR/i686-slc4-gcc34-dbg/testBOBRTimestamp
[alidcscom026] /data/ClientCommonRootFs/usr/local/trigger/v/vmeb/atlas/BOBR/i686-slc4-gcc34-dbg > export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../installed
./testBOBRTimestamp -1 CTRL C
Compile link:
[alidcscom026] /data/ClientCommonRootFs/usr/local/trigger/v/vmeb/atlas/BOBR/cmt > ./mf_testBOBRTimestamp 

0xb000   BLOCK0 A16/D8  0x100
0xb00000 BLOCK1 A24/D32 0x4000
0xb10000 BLOCK2

Notes:
readings for version 0135 (in the pit not modified for LHC pp):
readBST1raw()
 0:ffffff00
 4:ffffff00
 8:ffffff00
12:ffffff00
16:ffffff00
20:ffffff00
24:ffffff00
28:ffffff00
32:ffffff00
36:ffffff00
readBSTprint( 1)
UTC: -256us -256s   BSTstatus:ff   TurnCount:4278255615 0xff00ffff
LHCFill:-16711681   BeamMode:65535   ParticleType1/2:0/255 
Momentum:65535   Intensity1/2:-256/-256
readBSTprint( 2)
UTC: -256us -256s   BSTstatus:ff   TurnCount:4278255615 0xff00ffff
LHCFill:-16711681   BeamMode:65535   ParticleType1/2:0/255 
Momentum:65535   Intensity1/2:-256/-256

*/
#include "bobr.h"

/* HIDDEN primitives */
#include <string.h>
#include <stdio.h>
#include <unistd.h>   //usleep
#include "vmewrap.h"
#include "vmeblib.h"

extern int quit;

typedef struct Tbstmsg {
// updated each turn:
  w32 GPSusecs;
  w32 GPSsecs;
  w8  BSTstatus;    // 1=BEAM1   2=BEAM2   ok: tested
  w32 TurnCount;
// updated once per second:
  w32 LHCFill;
  w16 BeamMode;
  w8  ParticleType1;
  w8  ParticleType2;
  w16 BeamMomentum;
  w32 TotalIntensity1;
  w32 TotalIntensity2;
  w8  Byte54;   // bit 0x1 is LHC prepulse
} Tbstmsg;
Tbstmsg bstmsg[4];

w32 bstread4(w32 adr) {
int ix; w32 rc=0;
for(ix=0; ix<4; ix++) {
  rc= rc | ((vmer32(adr+4*ix)&0xff)<<(8*ix));
};
return(rc);
}
w32 bstread2(w32 adr) {
int ix; w32 rc=0;
for(ix=0; ix<2; ix++) {
  rc= rc | ((vmer32(adr+4*ix)&0xff)<<(8*ix));
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
void readBSTbix(w32 MessageBase, int bix) {
// updated each turn:
bstmsg[bix].GPSusecs= bstread4(MessageBase);
bstmsg[bix].GPSsecs= bstread4(MessageBase+4*4);
bstmsg[bix].BSTstatus= vmer32(MessageBase+17*4)&0xff;
bstmsg[bix].TurnCount= bstread4(MessageBase+18*4);
// updated once per second:
bstmsg[bix].LHCFill= bstread4(MessageBase+22*4);
bstmsg[bix].BeamMode= bstread2(MessageBase+26*4);
bstmsg[bix].ParticleType1= vmer32(MessageBase+28*4)&0xff;
bstmsg[bix].ParticleType2= vmer32(MessageBase+29*4)&0xff;
bstmsg[bix].BeamMomentum= bstread2(MessageBase+30*4);
bstmsg[bix].TotalIntensity1= bstread4(MessageBase+32*4);
bstmsg[bix].TotalIntensity2= bstread4(MessageBase+36*4);
bstmsg[bix].Byte54= vmer32(MessageBase+54*4)&0xff;
}
/*bstn: 1: BST1     2: BST2
        3: auxbst1  4: auxbst2
1,2: just read BST message
3,4: request next BST message, wait until it is stored in AuxDP Ram, read
*/
void readBST(int bstn) {
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
readBSTbix(MessageBase, bix);
}
/* this one was reading BST message correctly with BOBR version 013a
   -lhc prepulse connected to frnt panel 
void readBST_013a(int bstn) {
w32 dw1,dw2,dw3,MessageBase; int bix;
if(bstn==1) {
  MessageBase= MessageInput; bix=0;
} else if(bstn==2) {
  MessageBase= MessageInput+Message2shift; bix=1;
} else { return;} ;
// updated each turn:
bstmsg[bix].GPSusecs= vmer32(MessageBase);
bstmsg[bix].GPSsecs= vmer32(MessageBase+4);
dw1= vmer32(MessageBase+16); dw2= vmer32(MessageBase+20);
bstmsg[bix].BSTstatus= (dw1>>8) & 0xff;
bstmsg[bix].TurnCount= (dw1>>16) | (dw2<<16);
// updated once per second:
dw3= vmer32(MessageBase+24);
bstmsg[bix].LHCFill= (dw2>>16) | (dw3<<16);
bstmsg[bix].BeamMode= dw3>>16;
dw3= vmer32(MessageBase+28);
bstmsg[bix].ParticleType1= dw3 & 0xff;
bstmsg[bix].ParticleType2= (dw3>>8) & 0xff;
bstmsg[bix].BeamMomentum= dw3>>16;
bstmsg[bix].TotalIntensity1= vmer32(MessageBase+32);
bstmsg[bix].TotalIntensity2= vmer32(MessageBase+36);
} */
/*
read BST message. bstn: 1:BST1   2:BST2
*/
void printBST(int bix) {
printf("UTC: %dus %us   BSTstatus:%x   TurnCount:%u 0x%x\n\
LHCFill:%d   BeamMode:%d   ParticleType1/2:%d/%d \n\
Momentum:%d   Intensity1/2:%d/%d Byte54(LHCpp:0):0x%x\n",
bstmsg[bix].GPSusecs, bstmsg[bix].GPSsecs, bstmsg[bix].BSTstatus, 
bstmsg[bix].TurnCount, bstmsg[bix].TurnCount,
bstmsg[bix].LHCFill, bstmsg[bix].BeamMode, 
bstmsg[bix].ParticleType1, bstmsg[bix].ParticleType2,
bstmsg[bix].BeamMomentum, bstmsg[bix].TotalIntensity1, 
bstmsg[bix].TotalIntensity2, bstmsg[bix].Byte54);
}
/*FGROUP
wait for LHCpp + read BST message. 
bstn: 1:BST1   2:BST2
nppmax: max. number of PP to wait for
*/
void readBSTpp(int bstn, int nppmax) {
w32 MessageBase; int bix,npp=0, loops=0; w32 shift;
if(bstn==1) {
  MessageBase= AuxMessageInput; bix=2; shift=0; //waitaux(0);
} else if(bstn==2) {
  MessageBase= AuxMessageInput+Message2shift; bix=3; 
  shift= Message2shift; //shift= waitaux(Message2shift);
} else { return;} ;
while(loops<10000*1000) {   // ~1000 secs
  waitaux(shift);
  bstmsg[bix].Byte54= vmer32(MessageBase+54*4)&0xff;
  if(((bstmsg[bix].Byte54)&0x1) !=0) {
    readBSTbix(MessageBase, bix);
    printBST(bix); 
    npp++;
    if(npp>=nppmax) break;
  };
  loops++;
};
}
/*FGROUP
*/
void readBSTprint(int bstn) {
int bix=bstn-1;
readBST(bstn); printBST(bix);
}

/*FGROUP
*/
void readBST1raw() {
int i;
for(i=0; i<60; i++) {
  w32 dbyte; //w8 dw;
  //dw= vmer32(MessageInput+i*4);
  dbyte= vmer32(MessageInput+i*4);
  printf("%2d:0x%4x:0x%x\n", i, i*4, dbyte);
};
}
void readBST1raw_013a() {
int i;
for(i=0; i<10; i++) {
  w32 dw;
  dw= vmer32(MessageInput+i*4);
  printf("%2d:%x\n", i*4, dw);
};
}

/*FGROUP
Input:
  ut: 'u' ->detect 'UTC change'
  ut: 't' ->detect 'Turn change'
Operation (CPU/UTC times given in 'secs micsecs'):
read UTC1 and CPU1 time, addyTurnCount, BlockTurnCount
while 1:
  read UTC2 time, BlockTurnCount2
  if (UTC2!=UTC1) or (BlockTurnCount2!=BlockTurnCount): 
    read CPU2; break
  ix++;
print CPU/UTC 1, addyTurnCount, BlockTurnCount, ut
      CPU/UTC 2, addyTurnCount, BlockTurnCount2, ix
*/
void readBSTutcturn(char ut) {
w32 secscpu1, micscpu1;
w32 secscpu2, micscpu2;
w32 least4, most4, secsutc1, micsutc1, secsutc2, 
  micsutc2,turns, turns_2, turns24, turns24_2;
int ix;
//unsigned long long usecs;
//vmew32(BlockStatus, 0x84);
micsutc1= vmer32(addyTime0); secsutc1= vmer32(addyTime1);
GetMicSec(&secscpu1, &micscpu1);
turns= vmer32(addyTurnCount); turns24= vmer32(BlockTurnCount);
ix=0;
while(1) {
  int change;
  micsutc2= vmer32(addyTime0); secsutc2= vmer32(addyTime1);
  turns_2= vmer32(addyTurnCount); turns24_2= vmer32(BlockTurnCount);
  change=0;
  if(ut=='u') {
    if(micsutc2 != micsutc1) change=1;
  } else if(ut=='t') {
    if(turns24_2 != turns24) change=1; 
  } else {
    printf("bad ut (give 'u' for UTC change, 't' for Turn change)\n");
    break;
  };
  if(change==1) {
    GetMicSec(&secscpu2, &micscpu2);
    break;
  };
  ix++;
};
//usecs= most4*1000000000L + least4*1000;
printf("cpu1:%d %d utc1:%d %d turns:%d %d ut:%c\n",
  secscpu1, micscpu1, secsutc1, micsutc1, turns, turns24, ut);
printf("cpu2:%d %d utc2:%d %d turn2:%d %d ix:%d\n",
  secscpu2, micscpu2, secsutc2, micsutc2, turns_2, turns24_2, ix);
}
/*FGROUP
*/
void readall() {
w32 cs,ident;
cs= vmer32(BlockStatus); ident= vmer32(BlockIdentificator);
printf("BlockIdentificator:0x%x BlockStatus:0x%x (bit 0x4 shoudl be 1)\n", ident, cs);
}
void initmain() {
printf("initmain called...\n");
}
void boardInit() {
//writeall();
/* 0x4 -> BlockStatus (for both BST1/2) */
vmew32(BlockStatus, EnableDPRam);
vmew32(Message2shift+BlockStatus, EnableDPRam);
}
void endmain() {
}

