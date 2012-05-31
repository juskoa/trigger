#include <unistd.h>
#include "vmewrap.h"
#include "ltu.h"
w32 loadFPGA(int board);

void setglobalmode() {
/*printf("setting global mode\n"); */
usleep(GLBSTDDELAY);
vmew32(STDALONE_MODE,0);
}
int getsgmode() {
/*return:0: global mode
         1: stdalone mode 
        -1: error (not LTU board, or LTU is not in the crate) */
int rc; w32 code;
code=vmer32(CODE_ADD)&0xff;
if(code!=0x56) {
  rc= -1;
} else {
  w32 ltuver;
  ltuver= 0xff&vmer32(LTUVERSION_ADD);
  if(ltuver==0xff) {
    loadFPGA(0);          // is in vmeblib
  };
  ltuver= 0xff&vmer32(LTUVERSION_ADD);
  // allow only F3 (TTC) 0x? (ltuvi) or 0xc1: trigger input generator
  if((ltuver != 0xf3) && (ltuver!=0xc1) && ((ltuver&0xf0)!=0xb0)) {
    rc= -1;   // bad LTU version, perhaps ACORDE-FANOUT (bsy1->PP), or LVDST?
  } else {
    rc= vmer32(STDALONE_MODE)&1;
  };
};
return(rc);
}
void setstdalonemode(w32 b2) { // 3: ext.orbit   1: int. orbit
usleep(GLBSTDDELAY);
vmew32(STDALONE_MODE,b2);
}
