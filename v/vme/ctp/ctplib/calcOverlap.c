/* calcullate BUSY_OVERLAP word.
Test: uncomment printf lines and main(), than:
gcc -I$VMECFDIR/ctp_proxy -I$VMEBDIR/vmeblib calcOverlap.c -o calcoverlap
*/
#include <stdio.h>
#include "vmewrap.h"
//#include "Tpartition.h"
#define NCLUST 6
//#include "ctp.h"

/* Calculate 21 bits BUSY_OVERLAP word. BUSY_OVERLAP has to be
always updated, when any BUSY_CLUSTER word changed.
Input: 7 words (test, 1 2 3 4 5 6) BUSY_CLUSTER words
rc: returns 21 bits [20..0] with the meaning:
0:not overlapping clusters 1: overlapping clusters
bit    
20: cluster1 overlaps with cluster2
19:  1 3
18:  1 4
...  15 16 1T 23 24 25 26 2T 34 35 36 3T 45 46 4T 56 5T
0:   6 overlaps with Test cluster
----------------------------------*/ w32 calcOverlap(w32 *busy_clusters) {
int i1, i2, bit=20; w32 overlap=0;
for(i1=1; i1<NCLUST+1; i1++){
  int i2start;
  //if(i1<NCLUST) { i2start= i1+1; } else { i2start=0; }; 
  i2start=i1+1;
  for(i2=i2start; i2<NCLUST+2; i2++){
    int ix;
    if(i2>NCLUST) {ix=0; } else {ix=i2;};
    //printf("%d %d (bit:%d):", i1, ix, bit);
    if( (busy_clusters[i1] & busy_clusters[ix]) != 0) {
      overlap= overlap | (1<<bit);
      //printf("overlap");
    };
    //printf("\n");
    bit--;
  };
};
return(overlap);
}
/*
int main() {
w32 bc[7]= {0x1, 0x2, 0x3, 0x4, 0x5,0x6,0x7};
printf("overlap:0x%x\n", calcOverlap(bc));
return(0);
}*/

