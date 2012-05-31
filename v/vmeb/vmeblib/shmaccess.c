/* shmaccess.c
mallocShared: allocate 
SHM: ipcs -m, ipcrm shm SHMID
*/
#include <stdio.h>   /* NULL */
#include <stdlib.h>  /* malloc */
#include <errno.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "vmewrapdefs.h"
/*#define SHMACCESS
#include "shmaccess.h" */

extern int errno;

/* Input:
shmkey: 0x820000 (see shmaccess.h)
size:   0 -> only attch
       >0 -> attch or create& attach & clear (fill with 0s)
Output:
segid:  to be used with freeShared()
rc:  pointer to shared memory or 
     NULL in case of:
     - size==0 and shared memory was not allocated
Typical usage:
Master: shmbase= mallocShared(IDKEY, size, &segid)
        freeShared(shmbase, segid)  -remove it from the system
Client:
        #include <sys/shm.h>
        mallocShared(IDKEY, 0, &segid)
        shmdt(shmbase);   -release (not used by client more)
*/
w8 *mallocShared(w32 shmkey, int size, int *segid) {
int segment_id, segment_size, created=0;
w8 *shared_memory=NULL;
struct shmid_ds shmbuffer;
  
segment_id = shmget (shmkey, size,
  S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH|S_IRGRP|S_IWGRP);
if( segment_id == -1) {
  if(size==0) {
    printf("shm segment 0x%x doesn't exist, cannot be attached\n", shmkey);
    goto RET;
  };
  /* errno is:
  2 -if segment does not exist at all
  22 -if segment of different size exists
  */
  if(errno==22) {
    printf("Shared memory 0x%x of different size exist, errno:%d requested:%d bytes\n", 
      shmkey, errno, size);
    printf("Try: ipcs -m, ipcrm shm SHMID to remove it\n");
    goto RET;
  };
  printf("shm segment 0x%x doesn't exist, errno:%d\n", shmkey, errno);
  printf("creating it...\n");
  segment_id = shmget (shmkey, size,
    IPC_CREAT|IPC_EXCL|S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH|S_IRGRP|S_IWGRP);
  if( segment_id == -1) {
    printf("shmget error:%d(0x%x)\n", errno, errno);
  } else {
    created=1;
  };
};
*segid= segment_id;
if(segment_id != -1) {
  /* Attach the shared memory segment.  */
  shared_memory = (w8*) shmat (segment_id, 0, 0);
  printf ("shared memory attached at address %p\n", shared_memory);
  /* Determine the segment's size.  */
  shmctl (segment_id, IPC_STAT, &shmbuffer);
  segment_size = shmbuffer.shm_segsz;
  printf ("segment size: %d\n", segment_size);
  if (created==1) {
    int i;
    for(i=0; i<size; i++) shared_memory[i]=0;
    printf("First %d bytes initialized to 0\n", size);
  };
};
RET: fflush(stdout); return(shared_memory);
}

int freeShared(w8 *shmbase, int shmsegid) {
int rc;
/* Detach the shared memory segment.  */
shmdt(shmbase);
rc= shmctl(shmsegid, IPC_RMID, 0);
printf("rc.shmctl(IPC_RMID):%d\n", rc);
if( rc != 0) {
  printf("shmctl rc:%d errno:%d(%x)\n", rc, errno, errno);
};
return(rc);
}
void detachShared(w8 *shmbase) {
shmdt(shmbase);
}
/*
w32 getcnt1(int cntix) {
w32 val;
val= shm->buf[0].cnt[cntix];
//val= shm->buf[shm->active].cnt[cntix];
return(val);
}

int main() {
shm= (Tshm *)mallocShared(CTPSHMKEY, 0, &shmsegid);
}
*/
