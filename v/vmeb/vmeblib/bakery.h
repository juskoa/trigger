#define MAXNUM_THREADS 5    
#define true 1    
#define false 0    
typedef struct {
int Maxn;
int Entering[MAXNUM_THREADS]; // T: entering phase F: not entering phase
int Number[MAXNUM_THREADS];   // 0: this one is not locked
char name[16];   // ccread, swtriggers or ssmcr
unsigned int lastsecs,lastusecs;  // last usage
unsigned int minUsecs;            // minimum between 2 locks
unsigned int Nlocks;            // +1 when lock
unsigned int Nunlocks;          // -1 when unlock
unsigned int lockus;            // usleep(lockus) when lock required
} Tbakery;

/*bakery and corresponding customers:
swtriggers -any software trigger (SOD/EOD/SYNC/calibration):
initBakery(swtriggers,4)
0:ctpproxy SOD/EOD/SYNC
1:gcalib 
2:ctp.exe  
3:dims

ccread -counters reading:
initBakery(ccread,5)
0:ctpproxy
1:dims
2:ctp.exe+busytool
3:smaq  -counter read when waiting for trigger
4:inputs (i.e. vmecrate inputs)

ssmcr -ssm control+read. Common locking is used (i.e. for ANY SSM)
initBakery(ssmcr,3)
smaq lock is longer (~20s, while counters read takes ~1s). That's why
lockus introduced -> considerably higher for ssmcr
0: smaq -i.e. vme/smaq/smaq2.exe
1: orbitddl2.py -see ctp++/macros/findLMOrbitOff.C
2: ctp.exe
3: inputs -vmecrate inputs when started + check signature

Notes for possible updates (not done yet):
clgroups timer counters: just sending DIM cmd to dims, i.e. the locking managed
                 by dims (still problem with ending  -is it really done before
                 releasing triggers?)

[un]setPartDAQBusy: initBakery(clusterbusys,2)
*/
#define ssmcr_smaq 0
#define ssmcr_orbiddl2 1
#define ssmcr_ctp 2
#define ssmcr_inputs 3

void initBakery(Tbakery *bakery, char *name, int maxn);
void lockBakery(Tbakery *bakery, int customer);
void unlockBakery(Tbakery *bakery, int customer);
void printBakery(Tbakery *bakery);

