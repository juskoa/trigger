/*  ctp.h
CTP boards in ctpboards[] table (ix):
      base        +     code ix I2Caddr
BUSY  0x828000   8000   0x54 0  1
L0    0x829000   9000   0x50 1  2
L1    0x82a000   a000   0x51 2  3
L2    0x82b000   b000   0x52 3  4
INT   0x82c000   c000   0x55 4  5
FO1   0x82*000   *000   0x53 5  *    * = 0-5
FO2                          6
... FO boards: ascending order of dials corresponds to
FOnumber. E.g.: dials: 2,4 corresponds to FO3, FO5
"%x"%(4*0xAAA)
All the boards are accessed through 1 vme space (0x820000)
*/ 

#define ORBITLENGTH 3564

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

typedef struct {
  char name[8];
  w32  code;       /* 0x50 - 0x55 */
  int dial;
  w32  vmever;     /* NOTINCRATE:  not present in the crate*/
  w32  boardver;   /* 0: not configured, should not be used */
  w32  serial;   /* 0xff init. value */
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
#define TEST_ADD        0xc0   /* 0:blink LEDs, 1:VME R/W LEDS are Scope A/B */
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

/* common for L0/L1/L2/FO boards: */
#define ADC_START       0xcc   /* ADC (not for FO) */
#define ADC_DATA        0xd0 

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
#define PFBLOCK_A      0x568  /* 5*3 words per L0/L1/L2 board
			    Circuit1. 2,3,4,T      -> +0xc
			    corresponding BLOCKB   -> +4
			    corresponding LUT word -> +8 */

/* FO boards */
/* not valid for A3 (from 2.3.2006) -> use common SCOPE_SELECT
#define FO_SCOPE_SELECT    0x150   groupB: 0x1e0 groupA: 0x01f 
                                  0x17+0x17-not selected
                                  0x800: enableB, 0x400: enableA        */
#define FO_CLUSTER      0x240 /* 0x34333231,  3X -clusters for connectorX */
#define FO_TESTCLUSTER  0x244 /* 31..28 Toggle[4..1], bit20: Cal. flag */
                              /* 19..16 Test Cluster[4..1], 15..0 Roc[4..1] */
#define FO_DELAY_L1CLST 0x248
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
#define BUSY_DELAY_ADD   0x80c8   /* 5 bits, step 1ns */
#define BUSY_ORBIT_SELECT 0x80d4   /* [11..0] LastBC (3563) if LocalORBIT.
				     [13..12]: 
                                     0 -> ORBIT input, positive BC edge 
                                     1 -> ORBIT input, neg+pos. BC edge 
				     2 -> LocalORBIT  option
				     3 -> instead ORBIT produce BC/2 to
				          test ADCs on L0/1/2 
				     [14] RO EDGE. 0:negBC   1:posBC */
#define BUSY_DISB_CTP_BUSY 0x80d8 /* 1: disbale CTPbusy input, 0:normal operation*/
/*#define BUSY_ENABLE_OUT  0x8200 OLD   0x100 -> instead orbit produce BC/2 */
#define BUSY_CLUSTER     0x8200   /* 7 addresses BUSY_CLUSTER +n*4
                                     n=0..6 for clusterT,1-6. Content:1..24
                                     for 1..24 detectors connected to BUSYinp.
				  */
#define BUSY_DAQBUSY     0x821c   /* [5..0] -daqbusy for cluster6..1 */
#define BUSY_CTPDEADTIME 0x8224   /* [6..0] -ctp deadtime is (N+2) BCs.
                                     default: 52 */
#define BUSY_L0L1DEADTIME 0x8220  /* [8..0] -L0L1 deadtime is (N+2) BCs.
                                     default: 202 */

/* L0 board: */
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
#define RANDOM_1       0x94e4
#define RANDOM_2       0x94e8
#define SCALED_1       0x94ec
#define SCALED_2       0x94f0
#define ALL_RARE_FLAG  0x94f4
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
#define L2_ORBIT_READ  0xb140
#define L2_ORBIT_CLEAR 0xb144
#define L2_TCSTATUS    0xb1c0   /*R/O*/
#define L2_TCCLEAR     0xb1c8   /*dummy wr. */
#define L2_TCSET       0xb400   /* 24: P/F 23..0:Subdetector [24..1] */
#define L2_DELAY_L1    0xb4cc

/* INT board */
#define INT_ORBIT_READ 0xc140   /* orbit counter */
#define INT_MAX_CLEAR  0xc144   /* dummy wr: clear FIFO_MAX */
#define INT_FIFO_MAX   0xc148   /* counter read-only */
#define INT_DDL_EMU    0xc14c   /* 3..0: 3:emu_enable 2:emu_link_full */
                                /* 1: emu_busenable   0:emu_direction */
                                /* should be: 0xb */
#define INT_RC_BLCKMAX 0xc16c   /* counter read only */
#define INT_BLCKMAX_CLEAR 0xc170 /*dummy wr: clear BLCK_MAX */
#define INT_TC_SET     0xc400
#define INT_BC_OFFSET  0xc5a8
#define I2C_SET        0xc164   /* write: 6:always 1  5-3:channel  2-0:branch
                                  read: 12:BUSY 11:Err 10:1 9-7: channel 
                                  Err bit is valid if BUSY==0 */
#define I2C_DATA       0xc168   /* 4 voltages (8 bits each) */
#define I2C_MUXWR      0xc154   /* dummy wr */
#define I2C_MUXRD      0xc158
#define I2C_ADCWR      0xc15c
#define I2C_ADCRD      0xc160
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

#define NCOUNTERS_L0 160
#define NCOUNTERS_L1 160
#define NCOUNTERS_L2 134
#define NCOUNTERS_FO  34
#define NCOUNTERS_BUSY 48
#define NCOUNTERS_INT 0 
#define NCOUNTERS_MAX 160
#define NCOUNTERS NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+6*NCOUNTERS_FO+\
        NCOUNTERS_BUSY+NCOUNTERS_INT

#ifdef CTPMAIN
Tctpboards ctpboards[NCTPBOARDS]={
  /* name code dial vmever    boardver  #of_counters
     memoryshift-(see readCounters) */
  {"busy",0x54,8,NOTINCRATE,0,0xff,NCOUNTERS_BUSY,
  NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+6*NCOUNTERS_FO},
  {"l0",0x50,9,NOTINCRATE,0,0xff,NCOUNTERS_L0,
  0},
  {"l1",0x51,10,NOTINCRATE,0,0xff,NCOUNTERS_L1,
  NCOUNTERS_L0},
  {"l2",0x52,11,NOTINCRATE,0,0xff,NCOUNTERS_L2,
  NCOUNTERS_L0+NCOUNTERS_L1},
  {"int",0x55,12,NOTINCRATE,0,0xff,NCOUNTERS_INT,
  NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+6*NCOUNTERS_FO+NCOUNTERS_BUSY},
  {"fo1",0x53,0,NOTINCRATE,0,0xff,NCOUNTERS_FO,   /* FO dials: 0-5 */
  NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2},
  {"fo2",0x53,1,NOTINCRATE,0,0xff,NCOUNTERS_FO,
  NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+NCOUNTERS_FO},
  {"fo3",0x53,2,NOTINCRATE,0,0xff,NCOUNTERS_FO,
  NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+2*NCOUNTERS_FO},
  {"fo4",0x53,3,NOTINCRATE,0,0xff,NCOUNTERS_FO,
  NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+3*NCOUNTERS_FO},
  {"fo5",0x53,4,NOTINCRATE,0,0xff,NCOUNTERS_FO,
  NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+4*NCOUNTERS_FO},
  {"fo6",0x53,5,NOTINCRATE,0,0xff,NCOUNTERS_FO,
  NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+5*NCOUNTERS_FO}
  };
#else
extern Tctpboards ctpboards[NCTPBOARDS];
#endif

/* libctp.a subroutines (see vme/ctp/ctplib): */
/*FGROUP Common */
int notInCrate(int ix);
Tklas *getpClass(int klas);

void readBICfile();
void checkCTP();

void readCounters(w32 *mem, int accrual);
/*FGROUP L012 */
w32 getCounter(int board, int reladr);
/*FGROUP L012 */
void getCounters(int accrual);
/*FGROUP L012 */
void clearCounters();



