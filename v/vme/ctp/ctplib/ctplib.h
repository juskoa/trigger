/* minimal bst message (bobr.c) */
typedef struct Tlhcpp {
  w32 GPSusecs;
  w32 GPSsecs;
  w8 Byte54;
} Tlhcpp;

// updated each turn:
#define NBSTdcsmsg 11
#define iGPSusecs 0
#define iGPSsecs 1
#define  iBSTstatus 2   // 1=BEAM1   2=BEAM2   ok: tested
#define iTurnCount 3
// updated once per second:
#define iLHCFill 4
#define iBeamMode 5
#define  iParticleType1 6
#define  iParticleType2 7
#define iBeamMomentum 8
#define iTotalIntensity1 9
#define iTotalIntensity2 10

int bobrOpen();
void bobrClose(int vspbobr);
int getlhcpp(int vsp, int bstn, int waitforpp, Tlhcpp *lhcpp);
void nextBSTdcs(int vspbobr, w32 *bstmsg, int bstn);
void getlhc2ctpOrbit(int vspbobr, w32 *bst2ctp, w32 *bst3124);

//readtables.c:
int isTrigDet(char *name);
FILE *openFile(char *fname, char *rw);
int readdbfile(char *fname, char *mem, int maxlen);
int writedbfile(char *fname, char *mem);
int findSwitchInput(int swinput);

/*readCounters.c:
customer:
0:proxy 1:dims 2:ctp+ctpt+busytool 3:smaq 4:inputs\n");

*/
void readCounters(w32 *mem, int N, int accrual, int customer);
void getCountersBoard(int board, int reladr,w32 *mem, int customer);
/*FGROUP L012 
I: board: 0(busy),1(L0),2(L1), 3(L2), 4(FO1),...,10(INT)
   reladr: from 0...reladr  (i.e. 3 means reading first 4 counters)
   customer: 2 (for ctp exp. sw)
*/
w32 getCounter(int board, int reladr, int customer);
/*FGROUP L012 */
void getCounters(int N, int accrual, int customer);
/*FGROUP L012 */
void clearCounters(int customer);
void readTVCounters(w32 *mem);

// notInCrate.c
/*FGROUP SimpleTests 
rc: 0: if L0 borad firmware >0xAB
    boardversion if firmware <=0xAB 
*/
int l0AB();
/*FGROUP SimpleTests 
rc: 0: if L0 board
    boardversion if LM0  (i.e. >=0xc0)
*/
int l0C0();

// pfp.c
void WritePFcommon(w32 INTa,w32 INTb,w32 Delayed_INT);

// ctpTools.c, moved from ctp.c:
/*FGROUP INT
daqon:0       ->daq active
daqon:0xb     ->daq off (i.e. produce triggers in spite of DDL red diode 
                on INT board is on 
daqon: other  -> show current status.
NOTE about LEDs on INT board:
DDL interface: 
  green:DDL line ready, data not read out 
  flashing green: DDL line ready, data are read out
  flashing orange: data are read out, backpressure is sometimes active
                   (DAQ is not able to read everything)
upper DDL LED on INT board fron panel:
  red: INT is raising CTPBUSY on backlplane, because of full DDL buffers
INT_DDL_EMU word in normal mode (i.e. DAQ active):
     DDLfiLF  DDLfiBEN  DDLfiDIR
0x20:      0         1         0  data can't be sent (DDL not enabled from DIU)
0x30:      0         1         1  data sent
0x70:      1         1         1  data not sent (backpressure)
*/
void DAQonoff(int daqon);
// classn:1..100  mskbit: 1: disable 0: enable
void setClaMask(int classn, int mskbit);
w32 getRATE_MODE();
/*FGROUP SimpleTests
Input: address (e.g. 0x95bc) from range 0x95bc..0x95e4.
L0_INTERACT1   0x95bc    16 bits thruth table
L0_INTERACT2   0x95c0
L0_INTERACTT   0x95c4
L0_INTERACTSEL 0x95c8    [0..4]->LUT,BC1,BC2,RND1,RND2 for INTERACT1
                         [5..9]-> ... for INTERACT2 
L0_FUNCTION1   0x95cc
L0_FUNCTION2   0x95d0
RANDOM_1       0x95d4    bit31: 1: Enable filter
RANDOM_2       0x95d8
SCALED_1       0x95dc
SCALED_2       0x95e0
ALL_RARE_FLAG  0x95e4

rc: corresponding L0 or LM0 address
*/
w32 getLM0addr(w32 l0addr);
w32 getLM0PFad(w32 l0addr);
w32 getCLAMASK();

/*FGROUP DebCon
Generate n software trigger sequences
Operation:
-check if all detectors are in global (ctpproxy shared memory)
-setswtrig()
-while(n) startswtrig()

Parameters: see setswtrig()
customer: number 0..1
0: SOD/EOD/SYNC generation initiated from ctp_proxy
1: calibration triggers from gcalib task
2: dimservices.c (usually not used) + ctp.exe (expert sw) + ctpt.exe

RC: number of L2a successfully generated, or
    12345678: cal. triggers stopped becasue det. is not in global run
*/
int  GenSwtrg_op(int ntriggers,char trigtype, int roc, w32 BC,w32 detectors);

/*FGROUP DDR3 
read DDR3_CONF_REG0 (and print mnemonic...)
*/
void ddr3_status();
int ddr3_wrdone();
int ddr3_rddone();

/*FGROUP DDR3 
Enable DDR3, i.e.:
vmew32(DDR3_CONF_REG0, 0x7);   // not needed after power-up
vmew32(DDR3_CONF_REG0, 0x4);   // 0x4: Errors_reset
vmew32(DDR3_CONF_REG0, 0x0);   // 0 to: Errors_reset, Logic_reset, DDR3_reset
vmew32(0x9000+SSMaddress, 0);
streg= vmer32(DDR3_CONF_REG0); 
*/
void ddr3_reset();

/* Read DDR3 nws words, reading in 16 32bit-words blocks from ddr3 
ddr3_ad: 0, 16, 32,...   in words (1 word= 32 bits). Has to be N*16 
mem_ad:  pointer to w32[] array
nws:     number of 32bit words to be read from ddr3 
rc:   0: ok
*/
int ddr3_read(w32 ddr3_ad, w32 *mem_ad, int nws);

/* Write nws words to DDR3, writing 16 32bit-words blocks, last one
 * padded by 0s.
ddr3_ad: 0, 16, 32,...   in words (1 word= 32 bits). Has to be N*16 
mem_ad:  pointer to w32[] array
nws:     number of 32bit words to be written
rc:   0: ok
*/
int ddr3_write(w32 ddr3_ad, w32 *mem_ad, int nws);

/* 
 * Is fgroup necessary here ?
Read 4MB of usefull data (64MB from DDR3, i.e. ~23secs), i.e.
store only last 2 words (2x4 bytes) from each 512bits(=64bytes) block
in ssm1, ssm2
Use ssmshow after being read.
ssm1,ssm2: MEGA words  in each (ssm1/2:NULL -do not fill it)
rc: 0 ok
   rc from ddr3_read (stdout printed also)
*/
int ddr3_ssmread(w32 *ssm1, w32 *ssm2);
int ddr3_ssmreadall(w32 *ssms[]);

int ddr3_ssmdump(w32 opmod, FILE *dumpfile);
/*FGROUP DDR3 
secs: >0:continuous  -will return after secs seconds (leaving continuous active)
       0: 1-pass
*/
void ddr3_ssmstart(int secs);
