#include <stdio.h>
#include "vmewrap.h"
#include "ctp.h"
/* ix: 0..  index into ctpboards[] */
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
