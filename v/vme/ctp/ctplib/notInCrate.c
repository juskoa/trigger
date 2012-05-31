#include <stdio.h>
#include "vmewrap.h"
#include "ctp.h"
/* ix: 0..  index into ctpboards[] , i.e. BUSY, L0-2 INT, FO1-6 */
int notInCrate(int ix) {
if(ix>=NCTPBOARDS) {
  //printf("notInCrate called with ix:%d\n",ix); 
  return(1);
};
/* if(ix==1) return(0);    for ctpcfg.py PFboard debug  -L0*/
if(ctpboards[ix].vmever==0) {
  return(1);   // not in crate
} else {
  return(0);   // the board is in the crate
};
}
int l0AB() {
if(ctpboards[1].boardver<=0xab) {
  //return(0);   // forcing AC (for debugging sw with old board)
  return(ctpboards[1].boardver);
} else {
  return(0);
};
}

