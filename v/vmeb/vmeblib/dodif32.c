#include <stdio.h>
#include <math.h>
#include "vmewrap.h"
/*------------------------------------------------------- dodif32()
Substract 2 32 bits values (representing counters)
*/
w32 dodif32(w32 before, w32 now) {
w32 dif;
if(now >= before) dif= now-before;
else dif= now+ (0xffffffff-before) +1;
//if(DBGcnts) printf("dodif32:%d\n", dif);
return(dif);
}
w32 rounddown(float f) {
#ifdef CPLUSPLUS
w32 ret;
//ret= (unsigned int)(f);
ret= floor(f);
//printf("f:%f ret:%d\n",f,ret);
return(ret);
#else
return(f);
#endif
}

int w32toint(w32 w) {
return(w);
}

