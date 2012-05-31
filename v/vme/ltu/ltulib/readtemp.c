/* readtemp.c */
#include "stdio.h"
#include "unistd.h"
#include "vmewrap.h"
#include "ltu.h"

int ReadTemperature() {
w32 temp2,status;
int i;
vmew32(TEMP_START, DUMMYVAL);
for(i=0; i<3; i++) {
  usleep(300);
  status=vmer32(TEMP_STATUS);
  if( (status & 0x1) == 0) goto TEMPOK;
};
printf("ReadTemperature, TEMP_STATUS.BUSY timeout:\n");
return(-100);
TEMPOK:
temp2=vmer32(TEMP_READ)&0xff;
// do not print here! (the call from ltu_proxy) printf("ReadTemperature TEMP_READ:%x\n",temp2);
/* do the conversion from binary 2's complement */
return(temp2);
}
