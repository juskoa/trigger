/* vmesim.c 
13.12.2003 RegAddrs table added for special addresses
27. 1.2004 Shared memory used now (ipcs -m, ipcrm shm SHMID)
*/
#include <stdio.h>   /* NULL */
#include <stdlib.h>  /* malloc */
#include <errno.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include "vmesim.h"
#define MAXVMESPACES 4
#define MAXSPECVMEF 40
typedef struct {
  w32 baseaddr;
  int size;
  w32 am;
  w8 *simspace;
  int shmsegid;
} Tvmespace;
Tvmespace *vsps[MAXVMESPACES]={NULL, NULL, NULL, NULL};

typedef struct {
  int vmespace;
  w32 specaddr;
  TspecVmeF specVmeF;
  char rw[4];   /* should be bits... */
} TRegAddrs;
TRegAddrs RegAddrs[MAXSPECVMEF];

extern int errno;
/*
w8 *mallocShared(w32 shmkey, int size, int *segid) {
int segment_id, segment_size, created=0;
w8 *shared_memory=NULL;
struct shmid_ds shmbuffer;

segment_id = shmget (shmkey, size,
  S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH|S_IRGRP|S_IWGRP);
if( segment_id == -1) {
  printf("shm segment doesn't exist, errno:%d\n", errno);
  printf("creating it...\n");
  segment_id = shmget (shmkey, size,
    IPC_CREAT|IPC_EXCL|S_IRUSR|S_IWUSR|S_IROTH|S_IWOTH|S_IRGRP|S_IWGRP);
  if( segment_id == -1) {
    printf("shmget error:%d(%x)\n", errno, errno);
  } else {
    created=1;
  };
};
*segid= segment_id;
if(segment_id != -1) {
  // Attach the shared memory segment.
  shared_memory = (w8*) shmat (segment_id, 0, 0);
  printf ("shared memory attached at address %p\n", shared_memory);
  // Determine the segment's size.
  shmctl (segment_id, IPC_STAT, &shmbuffer);
  segment_size = shmbuffer.shm_segsz;
  printf ("segment size: %d\n", segment_size);
  if (created==1) {
    int i;
    for(i=0; i<size; i++) shared_memory[i]=0xfe; 
    printf("First %d bytes initialized to 0xfe\n", size);
  };
};
return(shared_memory);
}
int freeShared(Tvmespace *vsp) {
int rc;
// Detach the shared memory segment. 
shmdt (vsp->simspace);
rc= shmctl (vsp->shmsegid, IPC_RMID, 0);
printf("rc.shmctl(IPC_RMID):%d\n", rc);
if( rc != 0) {
  printf("shmctl rc:%d errno:%d(%x)\n", rc, errno, errno);
};
return(rc);
}
*/
int checkaddr(int vmespace, w32 offset, int *exityes) {
/* ret: 0 -address is Ok
        1 -address is Ok, user routine was called, offset registered
	   in exityes is the index into RegAddrs table
 *      10-bad address
 */
int i;
*exityes=-1;
if (offset >= (w32)vsps[vmespace]->size) {
  printf("vmesim(): bad address: %x (max:0x%x)\n",offset,
    vsps[vmespace]->size);
  return(10);
};
for(i=0; i<MAXSPECVMEF; i++) {
  if((RegAddrs[i].specaddr == offset) &&
    (RegAddrs[i].vmespace == vmespace)) {
    /*int rcf;
    printf("checkaddr: vme registered address found for %x\n", offset); */
    *exityes=i;
    return(1);
  };
};
return(0);
}
/* vmesimreg should be called after vmesimOpen to register
 * all the special vme addresses processing
 */
int vmesimreg(TspecVmeF pf, int vmespace, w32 vmeaddr, w32 rw) {
int i,rc=0;
for(i=0; i<MAXSPECVMEF; i++) {
  if((RegAddrs[i].specVmeF != NULL) &&
     (RegAddrs[i].specaddr == vmeaddr) &&
     (RegAddrs[i].vmespace == vmespace)) {
    rc=1;
    printf("vmesimreg: vme address registered more times\n");
    RegAddrs[i].specVmeF= NULL;
  };
};
for(i=0; i<MAXSPECVMEF; i++) {
  if(RegAddrs[i].specVmeF == NULL) {
    RegAddrs[i].specaddr= vmeaddr;
    RegAddrs[i].specVmeF= pf;
    RegAddrs[i].vmespace= vmespace;
    RegAddrs[i].rw[0]= rw;   /* not used yet */
    goto ERR;
  };
};
rc=2;
printf("vmesimreg: too many vme addresses registered\n");
ERR:
printf("vmesimreg: rc:%x vmespace:%d vmeaddr:%x rw:%x\n",rc,
  vmespace,vmeaddr,rw);
return(rc);
}
int vmesimOpen(int vmespace, w32 baseaddr, w32 size, w32 am) {
int i,ret=0;
if( (vmespace >=0) && (vmespace <MAXVMESPACES) && 
  (vsps[vmespace]==NULL) ) {
  int shmsegid;
  vsps[vmespace]= (Tvmespace *)malloc(sizeof(Tvmespace));
  if (vsps[vmespace] == NULL) {ret=1; goto ERR; };
  vsps[vmespace]->simspace= (w8 *)malloc(size); 
  /*vsps[vmespace]->simspace= mallocShared(baseaddr, size, &shmsegid); */
  if (vsps[vmespace]->simspace == NULL) {
    free(vsps[vmespace]);
    vsps[vmespace]= NULL; 
    ret=2; goto ERR;
  };
/*  for(i=0; i<size; i++) vsps[vmespace]->simspace[i]=0xfe; */
  vsps[vmespace]->baseaddr= baseaddr;
  vsps[vmespace]->size=size;
  vsps[vmespace]->am=am;
  vsps[vmespace]->shmsegid=shmsegid;
} else {
  ret=3;
};
for(i=0; i<MAXSPECVMEF; i++) {   /* no spec. vme funcs registered */
  RegAddrs[i].specVmeF= NULL;
};
ERR: return(ret);
}
int vmesimClose(int vmespace) {
int ret=0;
if( (vmespace >=0) && (vmespace <MAXVMESPACES) && 
  (vsps[vmespace]!=NULL) ) {
  if (vsps[vmespace] == NULL) {ret=1; goto ERR; };
  free(vsps[vmespace]->simspace); 
  /* ret= freeShared(vsps[vmespace]);*/
  free(vsps[vmespace]);
  vsps[vmespace]= NULL;
} else {
  ret=3;
};
ERR:return(ret);
}
w8  vmesimr8(int vmespace,w32 offset){
w8 rv=0xf2; int exityes;
if (checkaddr(vmespace,offset, &exityes) <= 1) {
  if( exityes>=0 ) {
    int rcf;
    rcf= RegAddrs[exityes].specVmeF(vsps[vmespace]->simspace,
	 vsps[vmespace]->size, offset, VMEDAR|VMEDA8);
  };
  rv= vsps[vmespace]->simspace[offset];
};
return(rv);
}
w16 vmesimr16(int vmespace,w32 offset){
w16 rv=0xf1f1;
w16 *ptr; int exityes;
if (checkaddr(vmespace,offset, &exityes) <= 1) {
  if( exityes>=0 ) {
    int rcf;
    rcf= RegAddrs[exityes].specVmeF(vsps[vmespace]->simspace,
	 vsps[vmespace]->size, offset, VMEDAR|VMEDA16);
  };
  ptr= (w16 *)&vsps[vmespace]->simspace[offset];
  rv= *ptr;
/*  rv= *(w16 *)(&vsps[vmespace]->simspace)[offset/2]; */
};
return(rv);
}
w32 vmesimr32(int vmespace,w32 offset){
w32 rv=0xf0f0f0f0; int exityes;
if (checkaddr(vmespace,offset, &exityes) <= 1) {
  if( exityes>=0 ) {
    int rcf;
    rcf= RegAddrs[exityes].specVmeF(vsps[vmespace]->simspace,
	 vsps[vmespace]->size, offset, VMEDAR|VMEDA32);
  };
  rv= *(w32 *)&vsps[vmespace]->simspace[offset];
};
/*printf("vmesimr32 from %x value:%x space:%d\n", offset,rv, vmespace); */
return(rv);
}
void vmesimw8(int vmespace,w32 offset, w8 value){
int crc,exityes;
printf("vmew8 %x to %x\n", value, offset);
crc= checkaddr(vmespace,offset, &exityes);
if (crc <= 1) {
  vsps[vmespace]->simspace[offset]=value;
  if( exityes>=0 ) {
    int rcf;
    rcf= RegAddrs[exityes].specVmeF(vsps[vmespace]->simspace,
	 vsps[vmespace]->size, offset, VMEDAW|VMEDA8);
  };
};
}
void vmesimw16(int vmespace,w32 offset, w16 value){
w16 *ptr;
int crc; int exityes;
printf("vmew16 %x to %x\n", value, offset);
crc= checkaddr(vmespace,offset, &exityes);
if (crc <= 1) {
/*  (w16 *)vsps[vmespace]->simspace[offset]= value; */
  ptr= (w16 *)&vsps[vmespace]->simspace[offset];
  *ptr= value;
  if( exityes>=0 ) {
    int rcf;
    rcf= RegAddrs[exityes].specVmeF(vsps[vmespace]->simspace,
	 vsps[vmespace]->size, offset, VMEDAW|VMEDA16);
  };
};
}
void vmesimw32(int vmespace,w32 offset, w32 value){
w32 *ptr;
int crc; int exityes;
/*printf("vmesimw32 %x to %x space:%d\n", value, offset,vmespace); */
crc= checkaddr(vmespace,offset, &exityes);
if (crc <= 1) {
  ptr= (w32 *)&vsps[vmespace]->simspace[offset];
  *ptr= value;
  if( exityes>=0 ) {
    int rcf;
    rcf= RegAddrs[exityes].specVmeF(vsps[vmespace]->simspace,
	 vsps[vmespace]->size, offset, VMEDAW|VMEDA32);
  };
};
}

