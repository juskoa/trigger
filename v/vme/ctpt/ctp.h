/*  ctp.h */
#define CTP_SW_VER "1.0 8.5.2007"

/* CTP boards in ctpboards[] table (ix):
      base        +     code ix I2Caddr
BUSY  0x828000   8000   0x54 0  1
L0    0x829000   9000   0x50 1  2
L1    0x82a000   a000   0x51 2  6
L2    0x82b000   b000   0x52 3  4
INT   0x82c000   c000   0x55 4  5
FO1   0x82*000   *000   0x53 5  *    * = 1-6  (before 19.4.2007: 0-6)
FO2                          6
FO6                          10
... FO boards: ascending order of dials corresponds to
"%x"%(4*0xAAA)
All the boards are accessed through 1 vme space (0x820000)
*/ 
#define BSP 0x1000
#define NCTPBOARDS 11
#define FO1BOARD 5     /* index of first FO board in ctpboards */

#define BUSYcode 0x54
#define L0code   0x50
#define L1code   0x51
#define L2code   0x52
#define INTcode  0x55
#define FOcode   0x53

#define NOTINCRATE 0
#define NICRATE 0x88   /* for Tctpboards.vmever ->board is to be checked */

#define BICfile "/root/NOTES/boardsincrate" 
#define MAXLINE 256
#define ORBITLENGTH 3564

typedef struct {
  char name[8];
  w32  code;       /* 0x50 - 0x55 */
  int dial;
  w32  vmever;     /* NOTINCRATE:  not present in the crate
                      NICRATE   :  after readBICfile: the board should be
                                   configured/checked */
  w32  boardver;   /* 0: not configured, the board can not be used */
  w32  serial;   /* ser. number of the board. 0xff: init. value */
  w32  lastboardver;  /* last board version */
  int  numcnts;  /* number of counters on this board */
  int  memshift;  /* shift of this group of counters in mem. ->readCounters() */
} Tctpboards;

/*REGSTART32 */
/* common VME FPGA addresses: */
#define CODE_ADD      0x4     /* board type (0x56 for LTU) */
#define SERIAL_NUMBER 0x8     /* unique serial number  of the board */
#define VERSION_ADD   0xc     /* VME FPGA firmware version */
#define SOFT_RESET    0x28
/*REGEND */

#define FlashAddClear 0x48           /*  12  Flash memory */ 
#define FlashAccessIncr 0x40         /*  10  */
#define FlashAccessNoIncr 0x44       /*  11  */
#define FlashStatus 0x4c             /*  13  */
#define ConfigStart 0x54             /*  15  */ 
#define ConfigStatus 0x50            /*  0x40 -> FPGA CRC error (SEU)  */

/*REGSTART32 */
#define TEMP_START    0x58   /* LTU temperature: */
#define TEMP_STATUS   0x5c
#define TEMP_READ     0x60

/* common board logic FPGA addresses for all CTP boards: */
#define FPGAVERSION_ADD 0x80 /* board's FPGA version */
/*#define TEST_ADD        0xc0  was till 12.3.2008 */
#define TEST_ADD        0x7e0  /* 0:blink LEDs, 1:VME R/W LEDS are Scope A/B */
#define BC_STATUS       0xc4   /* [2:0] Orbit error, PLL-locked, BC-error */

#define SSMcommand  0x19c  /* 0x0: VMEREAD   0x1: VMEWRITE */
                           /* 0x2 RECAFTER  0x3: RECBEFORE */
#define SSMstart    0x1a0  /* dummy wr */
#define SSMstop     0x1a4  /* dummy wr */
#define SSMaddress  0x1a8  /* w/r */
#define SSMdata     0x1ac  /* w/r */
#define SSMstatus   0x1b0  /* read only. Bits[2:0]: BUSY, OPERATION, MODE */
#define SSMenable   0x1b4  /* 00: normal, 10 in enabled, 01 out enabled */

#define PLLreset   0x1bc  /* dummy write */

/* common for L0/L1/L2/FO (busy,int) boards: */
#define ADC_START       0xcc   /* ADC (not for FO) */
#define ADC_DATA        0xd0 
#define SOFT_LED        0x15c  /* soft LED (busy board too)*/
#define COPYCOUNT      0x1d4   /*dummy wr. copy counters CMD */
#define COPYBUSY       0x1d8   /*ro [0] copy-busy status */
#define COPYCLEARADD   0x1dc   /*dummy wr. clear copy mem. add. */
#define COPYREAD       0x1e0   /*ro copy memory data */
#define CLEARCOUNTER   0x5ac /* clear counters CMD */

#define SPY_MEMORY     0x400
#define SCOPE_SELECT   0x4f8  /* groupB: 0x1e0 groupA: 0x01f */
                               /* 0x17+0x17-not selected                */
                               /* 0x800: enableB, 0x400: enableA        */
#define ADC_SELECT     0x500 /*ADC mode, ADC input selector. 0x100:ADCmode */
                /* not for FO   0x100 GND (before A2 version: Orbit (toggle))
                                0x1NN NN:01-18 i: Input1-Input24
                                0x119: Vhigh
                                0x11a: (Vhigh-Vlow)/2 
                                0x11b: ADC test -new way (ver. A2 from 14.12)
                                0x11c: Vlow   */
#define SYNCH_ADD      0x504 /*Synch/delay adds: 0x504-0x560    not FO*/

#define PF_COMMON      0x564  /* 1 word per L0/L1/L2 board */
#define PFBLOCK_A      0x568  /* 5*3 words per L0/L1/L2 board */
			      /* Circuit1. 2,3,4,T      -> +0xc */
#define PFBLOCK_B      0x56c
#define PFLUT          0x570

/* FO boards */
/* not valid for A3 (from 2.3.2006) -> use common SCOPE_SELECT
#define FO_SCOPE_SELECT    0x150   groupB: 0x1e0 groupA: 0x01f 
                                  0x17+0x17-not selected
                                  0x800: enableB, 0x400: enableA        */
#define FO_CLUSTER      0x240 /* 0x34333231,  3X -clusters for connectorX */
#define FO_TESTCLUSTER  0x244 /* 31..28 Toggle[4..1], bit20: CALFLAG */
                              /* 19..16 Test Cluster[4..1], 15..0 Roc[4..1] */
#define FO_DELAY_L1CLST 0x248
#define FO_FILTER_L1 0x84     /* 0x1000 1:enable FILTER   0: disable filter
                                 0xfff bits: TL1-1 -see calcFO_FILTER_L1() */
/*REGEND */

/* SSMsetom() modes: */
#define SSMomvmer 0
#define SSMomvmew 1
#define SSMomreca 2
#define SSMomrecb 3
#define SSMomgens 4
#define SSMomgenc 5

/*REGSTART32 */
/* BUSY board: */
#define BUSY_DELAY_ADD   0x84c8   /* 5 bits, step 1ns */
#define BUSY_ORBIT_SELECT 0x80d4   /* [11..0] LastBC (3563) if LocalORBIT.
				     [13..12]: 
                                     0 -> ORBIT input, positive BC edge 
                                     1 -> ORBIT input, neg+pos. BC edge 
				     2 -> LocalORBIT  option
				     3 -> instead ORBIT produce BC/2 to
				          test ADCs on L0/1/2 
				     [14] RO EDGE. 0:negBC   1:posBC */
#define BUSY_DISB_CTP_BUSY 0x80d8 /* 1: disbale CTPbusy input, 
                                     0:normal operation
                               Bit1(ro): reflects status of inp. signal  */
/*#define BUSY_ENABLE_OUT  0x8200 OLD   0x100 -> instead orbit produce BC/2 */
#define BUSYMAX_DATA     0x8168
#define BUSYMINI_DATA    0x816c
#define MINIMAX_SELECT   0x8570
#define MINIMAX_CLEAR    0x8174
#define MINIMAX_LIMIT    0x8578
#define BUSYLAST_SELECT  0x857c
#define BUSY_CLUSTER     0x8600   /* 7 addresses BUSY_CLUSTER +n*4
                                     n=0..6 for clusterT,1-6. Bits 23..00
                                     detectors 24..1 connected to BUSYinp.
				  */
#define BUSY_DAQBUSY     0x861c   /* [5..0] -daqbusy for cluster6..1 */
#define BUSY_L0L1DEADTIME 0x8620  /* [8..0] -L0L1 deadtime is (N+2) BCs.
                                     default: 202 */
#define BUSY_CTPDEADTIME 0x8624   /* [6..0] -ctp deadtime is (N+2) BCs.
                         default: 52. 60 (1.6us) for ctp readout version */
#define BUSY_OVERLAP     0x8640   /* [20..0] bits.1:overlap 0: not overlap for
                                     12 13 14 15 16 1T 23 24 ... 6T */
/* L0 board: */
#define L0_CLEAR_RND   0x90c8 /*dummywr: clear RND1/2 (according to L0_ENA_CRND)*/
#define L0_TCSTATUS    0x91c0   /*R/O*/
#define L0_TCSTART     0x91c4   /*dummy wr. */
#define L0_TCCLEAR     0x91c8   /*dummy wr. */
#define RATE_DATA      0x91cc   /*w/r L0 rate data word  (21bits data) */
#define RATE_CLEARADD  0x91d0   /*dummy wr. clear rate memory add */
#define MASK_DATA      0x91e4   /*wr BC mask data word  4Kwordx4bits */
#define MASK_CLEARADD  0x91e8   /*dummy wr. clear mask mem. add */
#define L0_TCSET       0x9400   /* 18: P/F 17..14:BCMask[4..1] 13:CAL 12:S/A
                                   11..0: BCnumber (valid for Synch. trigger)*/
#define L0_CONDITION   0x9400    /* +4*n n=1,2,...,50 */
#define L0_VETO        0x9600    /* +4*n n=1,2,...,50 */
#define DAQ_LED        0x9600    /* reserved for SW use */
#define L0_MASK        0x9700    /* +4*n n=1,2,...,50 */
#define L0_INVERT      0x9500    /* +4*n n=45,....,50 */
#define L0_INTERACT1   0x94cc    /* 16 bits thruth table */
#define L0_INTERACT2   0x94d0
#define L0_INTERACTT   0x94d4
#define L0_INTERACTSEL 0x94d8 /* [0..4]->LUT,BC1,BC2,RND1,RND2 for INTERACT1*/
                              /* [5..9]-> ... for INTERACT2 */
#define L0_FUNCTION1   0x94dc
#define L0_FUNCTION2   0x94e0
#define RANDOM_1       0x94e4 /* bit31: 1: Enable filter (for FPGAVER>=A5) */
#define RANDOM_2       0x94e8
#define SCALED_1       0x94ec
#define SCALED_2       0x94f0
#define ALL_RARE_FLAG  0x94f4
#define L0_ENA_CRND    0x94fc /* 1..0: enable RND2, RND1 clear */
/* see PFCOMMON... */
#define MASK_MODE      0x95a4 /* BCMask memory mode 1:vme 0:normal */
#define L0_BCOFFSET    0x95a8 /* BC/Orbit offset data */
#define RATE_MODE      0x9700 /* Rate mem. mode 1:vme 0:normal */
/*   L0_SCOPE_SELECT   0x94f8 see SCOPE_SELECT*/

/* L1 board */
#define L1_TCSTATUS    0xa1c0   /*R/O*/
#define L1_TCCLEAR     0xa1c8   /*dummy wr. */
#define L1_TCSET       0xa400   /* 18: P/F */
#define L1_DEFINITION  0xa400    /* +4*n n=1,2,...,50 */
#define L1_DELAY_L0    0xa4cc
#define L1_INVERT      0xa500    /* +4*n n=45,....,50 */
#define ROIP_BUSY      0xa600 /* RoI Processor BUSY flag */

/* L2 board */
#define L2_DEFINITION  0xb400    /* +4*n n=1,2,...,50 */
#define L2_ORBIT_READ  0xb140    /* synced with INT */
#define L2_ORBIT_CLEAR 0xb144
#define L2_TCSTATUS    0xb1c0   /*R/O*/
#define L2_TCCLEAR     0xb1c8   /*dummy wr. */
#define L2_TCSET       0xb400   /* 24: P/F 23..0:Subdetector [24..1] */
#define L2_DELAY_L1    0xb4cc
#define L2_BCOFFSET    0xb5a8 /* BC/Orbit offset data */

/* INT board */
#define INT_ORBIT_READ 0xc140   /* orbit counter synced with L2 */
#define INT_MAX_CLEAR  0xc144   /* dummy wr: clear INT_FIFO_MAX */
#define INT_FIFO_MAX   0xc148   /* counter read-only */
#define INT_DDL_EMU    0xc14c   /* 6..4 read/only bits:
6: DDL filF   (link full)
5: DDL fiBEN (busy enable negative)
4: DDL fiDIR 
6..4: should be 0b011 in normal operation when DAQ is active
3..0: 3:emu_enable 2:emu_link_full  1:emu_busenable   0:emu_direction 
i.e.: 
set:
     0xb: no DAQ(emulation)   0xf:emulate LinkFull    0:with DAQ    
read:
     0x30: ok, DAQ active, link not full */

#define INT_DISB_CTP_BUSY 0xc150 /* 1: SW generated CTPbusy output, 
                                    0:normal operation.
                               Bit1(ro): reflects CTPreadout NEARLYFULL */
#define I2C_MUXWR      0xc154   /* dummy wr */
#define I2C_MUXRD      0xc158
#define I2C_ADCWR      0xc174  /* 0xc15c from version A5 */
#define I2C_ADCRD      0xc160
#define I2C_SET        0xc164   /* write: 6:always 1  5-3:channel  2-0:branch
                                  read: 12:BUSY 11:Err 10:1 9-7: channel 
                                  Err bit is valid if BUSY==0 */
#define I2C_DATA       0xc168   /* 4 voltages (8 bits each) */
#define INT_RC_BLCKMAX 0xc16c   /* counter read only */
#define INT_BLCKMAX_CLEAR 0xc170 /*dummy wr: clear BLCK_MAX */
#define INT_TCSET      0xc400   /* [4:1]:RoC, [0]:CalFlag for TestClass CTPreadout*/
#define INT_TEST_COUNT 0xc404   /* 14:8 EnaCIT,CIT, EnaRoC,RoC for TestCnt2 */
                                /* 06:r0EnaCIT,CIT, EnaRoC,RoC for TestCntr1*/
#define INT_BCOFFSET  0xc5a8
/*REGEND */
#define DUMMYVAL 0xffffffff   /* recommended for DUMMY writes */

#define MAXL0REGS 7
typedef struct{
  w32 regs[MAXL0REGS];   /* 7 regs: condition invert veto prescaler
			            L1definition L1invert L2definition */
                         /* veto[16] -> bit0 copied from L0_MASK word */
} Tklas;
typedef struct{
  w32 cluster;
  w32 tcluster;
} Tfanout;

#include "ctpcounters.h"
// CTPMAIN is defined in ctplib/readBICfile.c: i.e. readBICfile
// has to be recompiled when following definition is changed
#ifdef CTPMAIN
Tctpboards ctpboards[NCTPBOARDS]={
  /* name code dial vmever    boardver serial lastboardver 
     #of_counters memoryshift-(see readCounters) */
  {"busy",0x54, 8,NOTINCRATE,0,0xff,0xa7,NCOUNTERS_BUSY, CSTART_BUSY},
  {"l0",  0x50, 9,NOTINCRATE,0,0xff,0xa7,NCOUNTERS_L0, CSTART_L0},
  {"l1",  0x51,10,NOTINCRATE,0,0xff,0xa3,NCOUNTERS_L1, CSTART_L1},
  {"l2",  0x52,11,NOTINCRATE,0,0xff,0xa5,NCOUNTERS_L2, CSTART_L2},
  {"int", 0x55,12,NOTINCRATE,0,0xff,0xa7,NCOUNTERS_INT, CSTART_INT},
  {"fo1", 0x53, 1,NOTINCRATE,0,0xff,0xaa,NCOUNTERS_FO, CSTART_FO},  /* FO dials: 0-5 */
  {"fo2", 0x53, 2,NOTINCRATE,0,0xff,0xaa,NCOUNTERS_FO, CSTART_FO+ 1*NCOUNTERS_FO},
  {"fo3", 0x53, 3,NOTINCRATE,0,0xff,0xaa,NCOUNTERS_FO, CSTART_FO+ 2*NCOUNTERS_FO},
  {"fo4", 0x53, 4,NOTINCRATE,0,0xff,0xaa,NCOUNTERS_FO, CSTART_FO+ 3*NCOUNTERS_FO},
  {"fo5", 0x53, 5,NOTINCRATE,0,0xff,0xaa,NCOUNTERS_FO, CSTART_FO+ 4*NCOUNTERS_FO},
  {"fo6", 0x53, 6,NOTINCRATE,0,0xff,0xaa,NCOUNTERS_FO, CSTART_FO+ 5*NCOUNTERS_FO}
  };

#else
extern Tctpboards ctpboards[NCTPBOARDS];
#endif
/*FGROUP SimpleTests
*/
void dumpCTP();
/*FGROUP L0
get PF parameters for 1 board (L0, L1, or L2 -> ix= 1, 2 or 3)
*/
void getPF(int ix);
/*FGROUP L0
get PF parameters for 1 circuit 
I:
L0, L1, or L2 -> ix= 1, 2 or 3
circ -> 1..5
O: on stdout: 3 hexadecimal numbers: PFBLOCK_A, PFBLOCK_B, PFLUT
*/
void getPFc(int ix, int circ);

/*FGROUP L0
set PF parameters for 1 board (L0, L1, or L2 -> ix= 1, 2 or 3)
pfc: PF_COMMON word
*/
void setPF(int ix, w32 pfc);

/*FGROUP L0
set PF parameters for 1 circuit 
I:
L0, L1, or L2 -> ix= 1, 2 or 3
circ -> 1..5
A, B, LUT -3 words to be written
*/
void setPFc(int ix, int circ, w32 A, w32 B, w32 LUT);

/*FGROUP L0
Print setings (12 hexa numbers) of PFcircuit for deltat in BCs
First hexa number is 0xffffffff if error
*/
void printPFwc(int deltat);

/*--------------------------- libctp.a subroutines (see vme/ctp/ctplib): */
/*FGROUP Common */
int notInCrate(int ix);
Tklas *getpClass(int klas);

int getTL1();
int getTL2();
int calcPFablut(int deltathalf, w32* ablut);
void readBICfile();
void checkCTP();   // configure
void initCTP();    // initialise system parameters
int softLEDimplemented(int board);
w32 dodif32(w32 before, w32 now);    // substract 2 counters

void readTables();
int Detector2Connector(int idet,int *ifoM1,int *iconnectorM1);

/*FGROUP Common */
int findBUSYINP(int fo, int foc);
void printLTUname(int fo, int foc);

void readCounters(w32 *mem, int N, int accrual);
/*FGROUP L012 */
w32 getCounter(int board, int reladr);
/*FGROUP L012 */
void getCounters(int N, int accrual);
/*FGROUP L012 */
void clearCounters();
void readTVCounters(w32 *mem);

w32 calcOverlap(w32 *bsyclusts);
/*FGROUP busy
 Input: time in milisecs
 For detectors in clusters as defined on busy board 
 it calculates and prints average: 
  -fraction of the time detectors is busy 
  -average deadtime
*/
w32 findDeadBusysRuns(int time);
/*FGROUP busy
 For detectors in clusters as defined on busy board
 it reads LasBusy counter.
 Last busy counts number of cases when detector is releasing busy LAST
 in given cluster.
*/
void printLastDetectors(w32 cluster); 
/*FGROUP busy
Busy probe option - minimax select word
Select the object you want to study:
0- CTP BUSY
1-24 : detectors
25-30 : clusters
31 test cluster
*/
void busyprobe(char *det);
/*FGROUP busy
Operation:
- read busy timers
- sleep 100ms
- read busy timers 
- calculate difference between 2 measurements and compare 
  with busy_timer
rc: busy pattern: [0..23] bits set to 1 correspond to Dead busy inputs
*/
w32 findDeadBusys();

int ReadTemp(int ix);
void vme2volt(w32 vme );
w32 i2cread(int channel, int branch);
int i2cgetaddr(int board0_34, int *channel, int *branch);

/*FGROUP DebCon */
int  GenSwtrg(int n,char trigtype, int roc, w32 BC,w32 detectors);
void clearflags();

/*FGROUP DbgScopeCalls */
int checkScopeBoard(char ab);
/*FGROUP DbgScopeCalls */
int setScopeBoard(char ab, int board) ;
/*FGROUP DbgScopeCalls */
int getScopeSignal(int board, char ab) ;
/*FGROUP DbgScopeCalls */
int setScopeSignal(int board, char ab, int signal) ;
/*FGROUP DbgScopeCalls */
int getVMERWScope() ;
/*FGROUP DbgScopeCalls */
void setVMERWScope(w32 newv, w32 oldv) ;
/*FGROUP ConfiguratioH 
Print 1 line string xxxx
where x is the status (0/1) of software LED word
*/
void getSWLEDS(int ixboard);
/*FGROUP inputsTools
board:0:busy (the CLK edge for input ORBIT signal) 
      1..3:L0/1/2  
input: busy: no sense,  L0,L1:1..24   L2:1..12
edge: 0:Positive 1:Negative
*/
void setEdge(int board,w32 input,w32 edge);
 

