/* hwreader.c: read continuously CTP counters with given period
SHM: ipcs -m, ipcrm shm SHMID
*/
#include <stdio.h>   /* NULL */
#include <stdlib.h>  /* malloc */
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>    /* nanosleep */

#include "vmewrap.h"
#include "shmaccess.h"

void checkCTP();
void readCounters(w32 *buf, int accruals);

/* hwreader writes after new reading to this fifo if
shm->request is != 0 */
#define FIFONAME "/tmp/dataready"   /* mkfifo /tmp/dataready i.e. it has to be:
prw-r--r--  1 aj alice 0 Sep 20 16:11 /tmp/dataready
*/

int main(int argn, char **argv) {
struct timespec req,rem;
int rc,period,dif=0;
w32 ts2,tu2, ts3,tu3;
int mindif,maxdif;
float sum2=0,sum=0,sigma;
FILE *outfifo;
req.tv_sec=10;
req.tv_nsec=0;
rc= vmeopen("0x820000","0xd000");
checkCTP();
shm= (Tshm *)mallocShared(CTPSHMKEY, sizeof(Tshm), &shmsegid);
if(shm==NULL) exit(4);
shm->active=0;
shm->request=0;  /* nobody subscribed for data */
printf("Opening %s fifo for writing...\n",FIFONAME);
outfifo= fopen(FIFONAME,"w");
setlinebuf(outfifo); 
if(outfifo ==NULL) {
  printf("%s fifo cannot be opened\n",FIFONAME);
  exit(8);
};
printf("%s fifo opened for writing\n",FIFONAME);
printf("Opening VME 0x820000/0xd000\n",FIFONAME);
/*rc= vmeopen("0x820000","0xd000"); */
if(rc!=0) {
  printf("rc from vmeopen:%d\n", rc);
  goto RET;
};
period=0; mindif=10000000; maxdif=0;
GetMicSec(&ts3, &tu3);
while(1) {
  int ix;
  /*
  for(ix=0; ix<NCOUNTERS; ix++) {
    shm->buf[shm->active].cnt[ix]= period+ix;
    //shm->buf[0].cnt[ix]= period+ix;
  };
  shm->buf[0].cnt[0]= dif;
  */
  readCounters(shm->buf[0].cnt,1);
  /* fwrite(&array, sizeof(int), 3, file) will write 3 4-byte integers */
  if(shm->request!=0) {
    fprintf(outfifo, "%d\n",period); fflush(outfifo);
  };
  //nanosleep(&req, &rem);
  usleep(10000000);
  ts2=ts3; tu2=tu3;
  GetMicSec(&ts3, &tu3);
  dif= DiffSecUsec(ts3,tu3,ts2,tu2);
  if(dif<mindif) mindif=dif;
  if(dif>maxdif) maxdif=dif;
  /*if(period>0) {
    sum= sum+dif; sum2=sum2+dif*dif;
    if(period>1) sigma= sqrt((period*sum2 - sum*sum)/(period*period-1));
  };*/
  if((period%10)==0) {
    printf("%d:%d:last min-max sigma:%9d %9d-%9d %e\n", 
      period, shm->request, dif, mindif,maxdif,sigma); 
   /*printf("%d: cnts 13 165: %10d %10d\n", 
      period, shm->buf[0].cnt[13],shm->buf[0].cnt[165]);*/
  };
  period++;
};
vmeclose();
RET:
fclose(outfifo);
}

