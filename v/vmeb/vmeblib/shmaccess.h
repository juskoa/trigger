/*
#define NCOUNTERS 320+134+6*34+48+19+2
#define NPERIODSKEPT 1

typedef struct {   // not used
  w32 cix;    // counter index (0..NCOUNTERS-1)
  w32 caddr;  // vme address for vmer32(vmeaddr...
} Tcntaddrs;

typedef struct {
  char fifoname[20];   // fifoname where to send sync signal
} Trequests;

typedef struct {
  w32 cnt[NCOUNTERS];
} Tbuf1;

typedef struct {
  int active;   // index to buf to last read set of counters
  int request;  // 0: nobody is receiving data
  Tbuf1 buf[NPERIODSKEPT];
} Tshm;

#ifdef SHMACCESS
#define EXT
EXT Tshm *shm=NULL;
#else
#define EXT extern
EXT Tshm *shm;
#endif

EXT int shmsegid;
*/
#define CTPSHMKEY 0x820000
void *mallocShared(w32 shmkey, int size, int *segid);
int freeShared(void *shmbase, int shmsegid);
void detachShared(void *shmbase);
/*
w32 getcnt1(int cntix);
*/
