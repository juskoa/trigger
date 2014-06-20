#include <stdio.h>
#include "vmewrap.h"
#include "ctp.h"
#include "ctplib.h"

/*---------------------------------------------------------------- scope  */
w32 getSCOPE_SEL(int ix) {
w32 adr;
if((ix==0) || (ix==4) || ((ix>=5) && (ix<=10))) {
  adr= SCOPE_SELECTbfi;   // BUSY, INT, FO1-6
} else {
  if((ix==1) && (l0C0())) {
    adr= SCOPE_SELECTlm0;
  } else {
    adr= SCOPE_SELECT;
  };
}; return(adr);
}
/*FGROUP DbgScopeCalls
rc: the number of the board (0-10) which has its ab output enabled
    -1 if there is no enabled board
Note: by running this subroutine, all boards are checked, and
      if there is more 'enabled' boards then 1, they are disabled
      (but the first enabled one)
*/
int checkScopeBoard(char ab) {
int ix, rc=-1;
w32 status,enmask=0;
if(ab=='A') enmask=0x400;
if(ab=='B') enmask=0x800;
if(enmask==0) {
  printf("internal error in checkScopeBoard\n");
  return(rc);
};
for(ix=0; ix<NCTPBOARDS; ix++) {
  if(notInCrate(ix)) continue;
  status=vmer32(getSCOPE_SEL(ix)+BSP*ctpboards[ix].dial);
  if(status & enmask) {
    if(rc==-1) {
      rc=ix;   /* this board enabled */ 
      printf("checkScopeBoard%c:enabled:%d\n",ab,ix);
      continue;
    } else {   /* more than 1 board enabled, disable it: */
      w32 disst;
      printf("checkScopeBoard%c:enabled->disable:%d\n",ab,ix);
      disst= status & ~enmask;
      vmew32(getSCOPE_SEL(ix)+BSP*ctpboards[ix].dial, disst);
    };
  };
};
return(rc);
}
/*FGROUP DbgScopeCalls
Enable 1 board scope output.
rc: the number of the board (0-10) which has its ab output enabled
    -1 if there is no enabled board
Note: by running this subroutine, all boards are checked, and
      if there is more 'enabled' boards then 1, they are disabled
      (but the first enabled one)
*/
int setScopeBoard(char ab, int board) {
int ix, rc=-1;
w32 status,enmask=0;
if(ab=='A') enmask=0x400;
if(ab=='B') enmask=0x800;
if(enmask==0) {
  printf("internal error in setScopeBoard\n");
  return(rc);
};
for(ix=0; ix<NCTPBOARDS; ix++) {
  if(notInCrate(ix)) continue;
  status=vmer32(getSCOPE_SEL(ix)+BSP*ctpboards[ix].dial);
  if(ix==board) {
    status= status | enmask;
    rc=ix;
  } else {
    status= status & ~enmask;
  };
  vmew32(getSCOPE_SEL(ix)+BSP*ctpboards[ix].dial, status);
};
return(rc);
}
/*FGROUP DbgScopeCalls
rc: the number of the signal choosen for this board's ab output
    0x1000+0..31  -> Abis signals
    0x100 +0..31  -> Bbis signals
*/
int getScopeSignal(int board, char ab) {
int rc=-2;
w32 status,smask=0;
if(notInCrate(board)) {
  printf("%d not in crate\n",board);
  return(-1);
};
status=vmer32(getSCOPE_SEL(board)+BSP*ctpboards[board].dial);
if(ab=='A') {
/*  smask=0x01f; */
  smask=0x101f;            /* 0x1000 -Abis bit */
  rc= status&smask;
}  else if(ab=='B') {
/*  smask=0x1e0; */
  smask=0x23e0;            /* 0x2000 -Bbis bit */
  rc= (status&smask) >> 5;
}  else {
  printf("internal error in getScopeSignal\n");
  return(rc);
};
/*if(rc==23) rc=-1; */
return(rc);
}
/*FGROUP DbgScopeCalls
signal: 0x1000+0..23  -> Abis signals
        0x100 +0..23  -> Bbis signals
rc: the number of the signal choosen for this board's ab output
    -1 in case of error
*/
int setScopeSignal(int board, char ab, int signal) {
int rc=-1;
w32 newstatus,status,smask=0;
if(notInCrate(board)) return(-1);
status=vmer32(getSCOPE_SEL(board)+BSP*ctpboards[board].dial);
if(ab=='A') {
  smask=0x101f;
  newstatus= (status & ~smask) | (signal &smask);
  rc= signal&smask;
}  else if(ab=='B') {
  smask=0x23e0;
  newstatus= (status & ~smask) | ((signal<<5) &smask);
  rc= signal&(smask>>5);
}  else {
  printf("internal error in getScopeSignal\n");
  return(rc);
};
vmew32(getSCOPE_SEL(board)+BSP*ctpboards[board].dial, newstatus);
return(rc);
}

/*FGROUP DbgScopeCalls
rc: word with the bits corresponding to CTP boards in VMERW-Scope mode
    0 -no board in VMERW-Scope mode
    0x7ff -all boards in VMERW-Scope mode
*/
int getVMERWScope() {
int ix;
w32 status,rc=0, va;
for(ix=0; ix<NCTPBOARDS; ix++) {
  if(notInCrate(ix)) continue;
  va= TEST_ADD+BSP*ctpboards[ix].dial;
  if(ix==0) va= va + 0x400;  // busy board is special (1nn)
  status=vmer32(va) & 0x2;   /* VMERW-Scope bit */
  if(status!=0) {
    rc= rc | (1<ix);
  };
};
return(rc);
}

/*FGROUP DbgScopeCalls
newv: new settings     oldv: old settings
*/
void setVMERWScope(w32 newv, w32 oldv) {
int ix,bits;
w32 msk, newm, oldm, newbit, va;
for(ix=0; ix<NCTPBOARDS; ix++) {
  if(notInCrate(ix)) continue;
  msk= 1<<ix; newm= newv&msk; oldm= oldv&msk;
  if( newm ^ oldm) {   /* different setting */
   if(newm != 0) newbit=0x2;
   else newbit=0;
   va= TEST_ADD+BSP*ctpboards[ix].dial;
   if(ix==0) va= va + 0x400;  // busy board is special (1nn)
   vmew32(va, newbit);
   bits++;
  };
};
if(bits!=1) {
  printf("setVMERWScope: internal error %d >1\n", bits);
};
}
w32 slbin=0;
/*FGROUP ConfiguratioH 
Print 1 line string xxxx
where x is the status (0/1) of software LED word
*/
void getSWLEDS(int ixboard) {
int ix; w32 va;
char sl[5]="0000";
if(notInCrate(ixboard)) goto RET;
va= SOFT_LED+BSP*ctpboards[ixboard].dial;
if(softLEDimplemented(ixboard)) {
  slbin= vmer32(va);
  //           slbin=slbin+1;
  for(ix=0; ix<4; ix++) {
    if(slbin &(1<<ix)) sl[ix]='1';
  };
};
RET:printf("%s\n", sl);
}

