#define MAXNUM_THREADS 6       // ccread used by 6 customers
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
unsigned int lockus;            // usleep(lockus) when a lock is active
} Tbakery;

/*bakery and corresponding customers:
swtriggers -any software trigger (SOD/EOD/SYNC/calibration):
initBakery(swtriggers,4)
0:ctpproxy SOD/EOD/SYNC
1:gcalib 
2:ctp.exe  
3:dims

ccread -counters reading:
initBakery(ccread,6)
0:ctpproxy
1:dims
2:ctp.exe+busytool
3:smaq  -counter read when waiting for trigger
4:inputs (i.e. vmecrate inputs)
5:orbitddl2

ssmcr -ssm control+read. Common locking is used (i.e. for ANY SSM)
initBakery(ssmcr,3)
smaq lock is longer (~20s, while counters read takes ~1s). That's why
lockus introduced -> considerably higher for ssmcr (see bakery.c)
0: smaq -i.e. vme/smaq/smaq2.exe
1: orbitddl2.py -see ctp++/macros/findLMOrbitOff.C
2: ctp.exe
3: inputs -vmecrate inputs when started + check signature

Init: never (incorretly done till aug.2016 in ctpproxy for all resources).
Correct way:
    -init only when shm, allocated (in ctplib/ctpshm.c)
    -in ctpproxy: lock, print info/warning to log, unlock
    -instead of 'init' unlock in time of the customer start all resources used, i.e.:
     - ctpproxy swtriggers(0) ccread(0) ssmcr(4)
     - gcalib   swtriggers(1)
     - ctp      swtriggers(2) ccread(2) ssmcr(2)
     - dims     swtriggers(3) ccread(1)
     - smaq                   ccread(3) ssmcr(0)
     - inputs                 ccread(4) ssmcr(3)
     - orbitddl2              ccread(5) ssmcr(1)

see also DOC/ssmsync

Notes for possible updates (not done yet, seem clgroups not used in run2):
clgroups timer counters: just sending DIM cmd to dims, i.e. the locking managed
                 by dims (still problem with ending  -is it really done before
                 releasing triggers?)

[un]setPartDAQBusy: initBakery(clusterbusys,2)
*/
#define swtriggers_N 4
#define swtriggers_ctpproxy 0
#define swtriggers_gcalib 1
#define swtriggers_ctp 2
#define swtriggers_dims 3   // service CTPDIM/SWTRG used rarely (testing only?)

#define ccread_N 6
#define ccread_ctpproxy 0
#define ccread_dims 1
#define ccread_ctp 2
#define ccread_smaq 3
#define ccread_inputs 4
#define ccread_orbitddl2 5

#define ssmcr_N 5
#define ssmcr_smaq 0
#define ssmcr_orbitddl2 1
#define ssmcr_ctp 2
#define ssmcr_inputs 3
#define ssmcr_ctpproxy 4   // whole ctp(also ssm) is initialised by ctpproxy

void initBakery(Tbakery *bakery, char *name, int maxn);
void lockBakery(Tbakery *bakery, int customer);
int lockBakeryTimeout(Tbakery *bakery, int customer, int timeout);
void unlockBakery(Tbakery *bakery, int customer);
void printBakery(Tbakery *bakery);

