/*  ctp.h */
#define CTP_SW_VER "3.0 23.07.2014"

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
Note:
Max.  block size of Interaction record is 458 752 words
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

#define NCLASS 100

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
#define BC_STATUS       0xc4   /* [2:0] Orbit error, PLL-locked, BC-error */
/* Orbit error: set even BUSY board programmed for local orbit*/
#define SSMcommand  0x19c
/* L0 board:
0x0: VMEREAD   0x1: VMEWRITE
0x2 RECAFTER  0x3: RECBEFORE 
LM0 board (from 0xc1):
mask  meaning
0x1   mode 1: continuous (before) mode  0: 1-pass (after mode)
*/
#define SSMstart    0x1a0  /* dummy wr, valid for both L0,LM0 boards */
#define SSMstop     0x1a4  /* dummy wr, L0/LM0 */
#define SSMaddress  0x1a8  /* w/r ,
LM0: -set it to 0 before the start of recording
     -should point after the last word written (when cont. recording stopped)
*/
#define SSMdata     0x1ac  /* w/r */
#define SSMstatus   0x1b0  /* read only. Bits[2:0]: BUSY, OPERATION, MODE */
/* SSMstatus on LM0 notes:
- the same address, valid bits 7.11.2014:
mask meaning
0x1  -copy of mode (mask: 0x1) bit from SSMcommand
0x2  - 1: overflow when recording, can be cleared by writing to SSMaddress
0x4  - n/a
0x8  - n/a
0x100  -1: busy (i.e. recording) 0: ready (i.e. for VME access)
*/

#define SSMenable   0x1b4  /* 00: normal, 10 in enabled, 01 out enabled */

#define PLLreset   0x1bc  /* dummy write */

/* common for L0/L1/L2/FO (busy,int) boards: */
#define ADC_START       0xcc   /* ADC (not for FO) */
#define ADC_DATA        0xd0   /* 0x10: busy flag   0xff: ADC data */
#define SOFT_LED        0x15c  /* soft LED (busy board too)*/
#define COPYCOUNT      0x1d4   /*dummy wr. copy counters CMD */
#define COPYBUSY       0x1d8   /*ro [0] copy-busy status */
#define COPYCLEARADD   0x1dc   /*dummy wr. clear copy mem. add. */
#define COPYREAD       0x1e0   /*ro copy memory data */
/* clear counters: write 1, usleep(4us), write 0 */
#define CLEARCOUNTER   0x5ac /* clear counters CMD NOT FOR LM0!*/
#define CLEARCOUNTER_lm0   0x91f4 /* clear counters CMD ONLY FOR LM!*/

#define SPY_MEMORY     0x400  /* spy memory length: 
L1/2 boards: (0x100 - 0x2ff)*4 from November 2013.
L0 board: increased length (max. adr 0x1ff*4 -> 0x2ff*4) implemented earlier.
          From November 2013 max. addr. in spy mem was increased to 
          0x3ff*4 on L0 board. */

//#define SCOPE_SELECT   0x4f8
#define SCOPE_SELECT   0x5b0  /* For L0/L1/L2 boards
                                  groupB: 0x1e0 groupA: 0x01f */
                               /* 0x17+0x17-not selected                */
                               /* 0x800: enableB, 0x400: enableA        */
#define SCOPE_SELECTbfi 0x4f8  /* BSY,FO,INT: different address */
#define SCOPE_SELECTlm0 0x1f8  /* LM0       : different address */
#define ADC_SELECTlm0  0x1fc /*ADC mode, ADC input selector. 
0x100:ADCmode -seems n/a for lm0 board
*/
#define ADC_SELECT     0x5b4 /*ADC mode, ADC input selector. 0x100:ADCmode */
  /* not FOs, only L0/1/2.      0x100 GND (before A2 version: Orbit (toggle))
                                0x1NN NN:01-18 i: Input1-Input24
                                0x119: Vhigh
                                0x11a: (Vhigh-Vlow)/2 
                                0x11b: ADC test -new way (ver. A2 from 14.12)
                                0x11c: Vlow   
LM0 board:
[5..0] selector code:
1..48  ADC input 1..48
49     Vcc
50     ORBIT
51     ORBIT & !TEST
52     ADC Test input (local phase)  (28 on old L0 board)
53     BUSY/INT clock phase input
*/
#define RND1_EN_FOR_INPUTS 0x93f0 /* only on LM0:
Temporary solutuion for RND1 trigger: it can be connected to any
1..48 input defining following mask in 2 words:
bits 23..0: for first 24 inputs
RND1_EN_FOR_INPUTS+4
bits 23..0  for inputs 48..25
*/
/*#define TEST_ADD        0xc0  was till 12.3.2008 */
/*#define TEST_ADD        0x7e0 was till  6.11.2013 */
#define TEST_ADDr2      0x93f8 /* LM0, L0 */
#define TEST_ADD        0x7e8  /* 0:blink LEDs, 1:VME R/W LEDS are Scope A/B */
/* #define SYNCH_ADD      0x504 */
#define SYNCH_ADDr2    0x340 /*SYNCAL in fw.
Synch/delay adds for LM0 board shifted (i.e. 0x340 for 1st input)
LM0>=0xc5:
 4.. 0   Input delay(0..31) for inputs 1..24
 7       Edge Selector flag inputs 1..24
12.. 8   Input delay for inputs 25..48
15       Edge Selector flag inputs 25..48
21..16   6bits. Selection of the input: 0:not connected 1..48 connected here
26..24   LM input delay (0..7) for first 12, i.e. LM inputs
31..28   4bits. Selection of 12 from first 12

LM0<=0xc4:
 3.. 0   Input delay for inputs 1..24
 7       Edge Selector flag inputs 1..24   (was 4 before 12.2.2015)
11.. 8   Input delay for inputs 25..48
15       Edge Selector flag inputs 25..48  (was 12 before 12.2.2015)
21..16   Selection of the input: 0:not connected 1..48 connected to this one
*/
#define SYNCH_ADD      0x804 /*Synch/delay adds: 0x804-0x860    not FO
0x804:inp1,..., 0x860:inp24
L0, L1, L2:
 8.. 8   Edge Selector flag inputs 1..24
 7.. 4   not used
 3.. 0   Input delay for inputs 1..24 (12 for L2)
*/

//#define PF_COMMON      0x564
#define PF_COMMON      0x864  /* 1 word per L0/L1/L2 board */
//#define PFBLOCK_A      0x568
#define PFBLOCK_A      0x868  /* 5*3 words per L0/L1/L2 board */
			      /* Circuit1. 2,3,4,T      -> +0xc */
#define PFBLOCK_B      0x86c
#define PFLUT          0x870

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
                                 0x1ff bits: TL1-1 -see calcFO_FILTER_L1() */
/*REGEND */

/* SSMsetom() modes:
For LM0 only first 4 modes possible (20.11.2014)
 */
#define SSMomvmer 0
#define SSMomvmew 1
#define SSMomreca 2     // 1-pass
#define SSMomrecb 3     // continuous
#define SSMomgens 4
#define SSMomgenc 5

/*REGSTART32 */
/* BUSY board: */
#define BUSY_DELAY_ADD   0x84c8   /* 5 bits, step 1ns */
#define BUSY_ORBIT_SELECT 0x80d4   /* [11..0] LastBC (3563) if LocalORBIT.
				     [13..12]: 
                                     0 -> ORBIT input, positive BC edge 
                                     1 -> ORBIT input, neg+pos. BC edge 
				     2 -> LocalORBIT  option (i.e. 0x2deb)
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
#define MINIMAX_CLEAR    0x8174  /* dummy w: clear BUSY*_DATA registers */
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
                         default: 52. 60.:run1, 110 from 30.1.2014 */
#define BUSY_OVERLAP     0x8640   /* [20..0] bits.1:overlap 0: not overlap for
                                     12 13 14 15 16 1T 23 24 ... 6T */
/* L0 board: */
#define L0_CLEAR_RND   0x90c8 /*dummywr: clear RND1/2 (according to L0_ENA_CRND)*/
#define L0_TCSTATUS    0x91c0   /*R/O: bits:
0: Test Class Cluster BUSY flag
1: Test Class PP Request flag
2: Test Class L0 Request flag
3: Test Class L0 Acknowledge flag
4: Test Class BUSY flag: goes ON with L0_TCSTART
 */
#define L0_TCSTART     0x91c4   /*dummy wr. */
#define L0_TCCLEAR     0x91c8   /*dummy wr. */
#define RATE_DATA      0x91cc   /*w/r L0 rate data word:
bit31=0: 21bits data pseudo-random pattern-repetition period
         bits 24..21 read 0 if 0 was written
bit31=1: 25bits for L0-class busy time in steps of 10micsecs
         i.e. max. busy time: is  cca 5.58 minutes
LM0: bit25 (not 31) -see RATE_DATABTMr2
 */
#define RATE_CLEARADD  0x91d0   /*dummy wr. clear rate memory add */
#define MASK_DATA      0x91e4   /*wr BC mask data word  4Kwordx4bits */
                                /*   fy>=0xAC           4Kwordxx12bits*/
#define MASK_CLEARADD  0x91e8   /*dummy wr. clear mask mem. add */
#define MASK_MODEr2    0x91ec /* LM0: BCMask memory mode 1:vme 0:normal */
#define L0_BCOFFSETr2  0x91f0 /* BC/Orbit offset data */
#define L0_ENA_CRNDlm0 0x9200 
/* 1..0: enable RND2, RND1 clear In firmware called:  ENABLE_CLEAR */

#define RATE_MODElm0   0x9230 /* Rate mem. mode 1:vme 0:normal */
#define DAQ_LEDlm0     0x9234
#define L0_FUNCTION34r2  0x9240 /* New L0 functions of first 12 inputs*/
#define SCOPE_A_FRONT_PANEL 0x9244  /* LM0 only */
#define SCOPE_B_FRONT_PANEL 0x9248  /* LM0 only */
#define LM_L0_TIME 0x924c           /* 17 BCs ? */
#define LM_RANDOM_1 0x9250
#define LM_RANDOM_2 0x9254
#define LM_SCALED_1 0x9258
#define LM_SCALED_2 0x925c
#define LM_ENABLE_CLEAR  0x9260  /* see L0_ENA_CRNDlm0 */
#define LM_CLEAR_RANDOM  0x9264  /*     L0_CLEAR_RND  */
#define LM_RATE_MODE     0x9268  /*     RATE_MODElm0 */
#define LM_RATE_DATA     0x926c  /*     RATE_DATA    */
#define LM_RATE_CLEARADD 0x9270  /*     RATE_CLEARADD */

/* ddr3 registers on LM0 board 0x280 - 0x2bc (only first 5 used).
Read request:
------------
1. set REG1
2. set REG2  -this second write will trigger the read operation.

Write request:
-------------
16 VME words (32 bits wide) must be written
VME addr of DDR3 data write = hex B0 -BF DDR3_BUFF_DATA
*/
#define DDR3_CONF_REG0      0x9280   /* 31..23 readonly: 
31..28: mem_init,            rdi_fifo_empty,      rdi_fifo_has_space, full_flag, 
27..24: ddr3_ext_rd_itf_rdy, ddr3_ext_wr_itf_rdy, rst_logic,          rd_done, 
    23: wr_done
 2.. 0: writeonly: Errors_reset, Logic_reset, DDR3_reset */
#define DDR3_CONF_REG1      0x9284   /* Start address, read operation */
#define DDR3_CONF_REG2      0x9288   /* Number of readings, read operation */
#define DDR3_CONF_REG3      0x928c   /* Start address, write operation */
#define DDR3_CONF_REG4      0x9290   /* Number of writings, write operation */

#define DDR3_BUFF_DATA      0x92c0   /* read/write 16 regs from here */

#define SEL_SPARE_OUT  0x93e0  
/* 4 registers 0x93e0,4,8,c reserved for 4 output signals -copy
of any 1..48 inputs. Should be programmed with number 1..48.
+0 -spare  see initCTP.c
+4 -spare
+8 -0AMU
+c -LM */
#define L0_TCSET       0x9400   /* 18: P/F 17..14:BCMask[4..1] 13:CAL 12:S/A
                                   11..0: BCnumber (valid for Synch. trigger)*/
#define L0_TCSETr2     0x93fc

#define L0_CONDITION   0x9400    /* +4*n n=1,2,...,100
bits    newMeaning (>=AC)            meaning before AC
----    ----------                   --------------
31..30  Select Scaled-down BC2..1    not used
29..28  Select Random RND2..1        Select Scaled-down BC2..1
27..24  Select L0F4..1               Select Rnd2..1 + L0f2..1
23..0   Select L0 input 24..1        Select L0 input 24..1
*/
#define LM_CONDITION   0x9a00    /* +4*n n=1,2,...,100
bits    Meaning
----    -------
31..20  Select BCMASK 12..1
19..18  Select Scaled-down BC2..1
17..16  Select Random RND2..1
15..12  Select LMF4..1     
11..0   Select L0 input 12..1
*/

/*von #define L0_INVERT      0x9500     old (before AC) +4*n n=45,....,50 
bit23..0: 1: invert L0 input   0: use original polarity
firmAC:
all classes can use inverted inputs, use L0_INVERTac symbol.
*/
/* see PF_COMMON... */
#define MASK_MODE      0x95a4 /* L0: BCMask memory mode 1:vme 0:normal */
//#define L0_INTERACT1   0x94cc (whole block till ALL_RARE_FLAG shifted in 2013)
//
//----------------- L0. The block of LM0 addresses below...
#define L0_INTERACT1   0x95bc    /* 16 bits thruth table */
#define L0_INTERACT2   0x95c0
#define L0_INTERACTT   0x95c4
#define L0_INTERACTSEL 0x95c8 /* [0..4]->LUT,BC1,BC2,RND1,RND2 for INTERACT1*/
                              /* [5..9]-> ... for INTERACT2 */
#define L0_FUNCTION1   0x95cc
#define L0_FUNCTION2   0x95d0
#define RANDOM_1       0x95d4 /* bit31: 1: Enable filter (for FPGAVER>=A5) */
#define RANDOM_2       0x95d8
#define SCALED_1       0x95dc
#define SCALED_2       0x95e0
#define ALL_RARE_FLAG  0x95e4
//----------------- 

/*----------------- LM0. The block of L0 addresses see above...
#define L0_INTERACT1   0x9204    0x95bc-0x9204= 0x3b8 -> see L0LM0DIFF
#define L0_INTERACT2   0x9208
#define L0_INTERACTT   0x920c
#define L0_INTERACTSEL 0x9210
                            
#define L0_FUNCTION1   0x9214
#define L0_FUNCTION2   0x9218
#define RANDOM_1       0x921c
#define RANDOM_2       0x9220
#define SCALED_1       0x9224
#define SCALED_2       0x9228
#define ALL_RARE_FLAG  0x922c WRITE ONLY on LM0 board!
                       1: all (take all classes)
                       0: take only classes without ALL/rare flag set 
                          (=red in ctp)
*/
//----------------- 
//
/*   L0_SCOPE_SELECT   0x94f8 see SCOPE_SELECT*/
#define RATE_MODE      0x95fc /* Rate mem. mode 1:vme 0:normal */
#define DAQ_LED        0x9600    /* reserved for SW use old L0 only*/
#define L0_INVERTac    0x9600    /* +4*n n=1,....,100, 0x9604..0x9790 */ 
/* bit23..0: 1: invert L0 input   0: use original polarity */
#define L0_VETOr2      0x9800    /* +4*n n=1,2,...,100, LM0 board: 0x7f9ffff7
                                LM0 note
31     spare
30..24 DSCG group (7bits)       new
23     class mask (1:disabled)  new
22     spare
21     Select LM-L0 BUSY
20     1:Select All/Rare input  the same
19..8: Select BCmask[12..1]     the same
 7..4: Select PFprot[4..1]      the same
 2..0: Cluster code (1-6)       the same

Note: in ctp.c getClass L0_VETO[bit31] not set for LM0, instead
L0_VETOr2[23] bit is used. L0_MASK is not used in LM0 board!
*/
#define L0_VETO        0x9900    /* +4*n n=1,2,...,100
       fy<0xAC                   fy>=0xAC
bit12: 1:Select All/Rare input   bit20: 1: Select All/Rare input
                                 19..8: BCmask[12..1]
11..8: Select BCmask[4..1]
 7..4: Select PFprot[4..1]        7..4: the same
 2..0: Cluster code (1-6)         2..0: the same
Note: in ctp.c getClass L0_VETO[bit31] is set according to L0_MASK[0] bit
*/
#define L0_MASK        0x9b00    /* +4*n n=1,2,...,100   NOT used in LM0
bit0: 1: the class is disabled 
LM0: this word dos not exist (bit is in L0_VETOr2 now) 
*/
#define LM_INVERT      0x9c00
/*
11..0  Invert LM input
*/
//#define L0_SDSCG        0x98c8    /* +4*n n=1,....,50, 0x98cc..0x9990*/ 
#define L0_SDSCG        0x9d00    /* +4*n n=1,....,100, 0x9d04..0x9e90
LM0: does not exist (is in L0_VETOr2)
*/
#define LM_VETO        0x9e00
/*
31     spare
30..24 LM down scaling Class group (DSCG) 7bits
23..17 spare
16     class mask
15..14 spare veto bits
13..10 Select PFprot[4..1]      the same
 9     All/Rare input
 8     LM deadtime
 7..0  Cluster BUSY enabled 8..1
*/

/* L1 board */
#define L1_TCSTATUS    0xa1c0   /*R/O*/
#define L1_TCCLEAR     0xa1c8   /*dummy wr. */
#define L1_TCSET       0xa400   /* 18: P/F */
#define L1_DEFINITION  0xa400    /* +4*n n=1,2,...,100 */
/* bits:
31 RoI Veto Flag. 1: segmented readout (RoI) is suspended if class triggered
30..28 Cluster code
27..24 Select P/F[4..1] protection veto
23.. 0 Select L1 triggr input
*/
//#define L1_DELAY_L0    0xa4cc
#define L1_DELAY_L0    0xa5b8
//#define L1_INVERT      0xa500    /* +4*n n=45,....,50 */
#define L1_INVERT      0xa600    /* +4*n n=1,...,100 */
/* bits: 23..0: 0: original polarity, 1: corresponding input is inverted
*/
//#define ROIP_BUSY      0xa600
#define ROIP_BUSY      0xa5bc /* RoI Processor BUSY flag. Should be set to 1,
  unless RoI is implemented */

/* L2 board */
#define L2_DEFINITION  0xb400    /* +4*n n=1,2,...,50 */
/* bits:
30..28   Cluster code
27..24 Select P/F[4..1] protection veto
23..12 Invert L1 trigger input (0: original polarity, 1: inverted)
11.. 0 Select L2 trigger input
*/
#define L2_ORBIT_READ  0xb140    /* synced with INT */
#define L2_ORBIT_CLEAR 0xb144
#define L2_TCSTATUS    0xb1c0   /*R/O*/
#define L2_TCCLEAR     0xb1c8   /*dummy wr. */
#define L2_TCSET       0xb400   /* 24: P/F 23..0:Subdetector [24..1] */
#define L2_BCOFFSET    0xb5a8 /* BC/Orbit offset data */
//#define L2_DELAY_L1    0xb4cc
#define L2_DELAY_L1    0xb5b8

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

#define INT_DISB_CTP_BUSY 0xc150 /* 
bit0:
1: SW generated CTPbusy output, 0:normal operation.
bit1(ro): reflects CTPreadout NEARLYFULL
bit4: phase enable
*/
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
/* deliberately after REGEND becasue it is different for L0/LM0*/ 
#define L0_BCOFFSET    0x95a8 /* BC/Orbit offset data */
#define L0_ENA_CRND    0x95b8 /* 1..0: enable RND2, RND1 clear */
#define L0_FUNCTION34    0x97ec

#define L0LM0DIFF   0x3b8     // 0x95bc-0x9204= 0x3b8 -> L0LM0DIFF
#define L0LM0PFDIFF 0x4c4     // 0x864-0x3a0= 0x4c4 
#define DUMMYVAL 0xffffffff   /* recommended for DUMMY writes */
#define RATE_MASK 0x81ffffff   /* firmware AF: 6bits [30..25] are downscaling group, default: 0..49 */
#define RATE_MASKr2 0x03ffffff   /* firmware C0: bit25:0 rnddownscale */
#define RATE_DATABTM    0x80000000   // where class mask is 
#define RATE_DATABTMr2  0x2000000

#define DDR3_mem_init 0x80000000
#define DDR3_rdi_fifo_empty 0x40000000
#define DDR3_rdi_fifo_has_space 0x20000000
#define DDR3_full_flag 0x10000000
#define DDR3_rd_itf_rdy 0x8000000
#define DDR3_wr_itf_rdy 0x4000000
#define DDR3_rst_logic 0x2000000
#define DDR3_rd_done 0x1000000
#define DDR3_wr_done 0x0800000

#define MAXL0REGS 7
typedef struct{
  w32 regs[MAXL0REGS];   /* 7 regs: condition invert veto prescaler
			            L1definition L1invert L2definition */
 /* L0: veto[16/31] -> bit0 copied from L0_MASK word
   LM0: veto[23] is classmask, L0_MASK word not used */
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
  {"busy",0x54, 8,NOTINCRATE,0,0xff,0xaa,NCOUNTERS_BUSY, CSTART_BUSY},
  {"l0",  0x50, 9,NOTINCRATE,0,0xff,0xc5,NCOUNTERS_L0, CSTART_L0},
  {"l1",  0x51,10,NOTINCRATE,0,0xff,0xa9,NCOUNTERS_L1, CSTART_L1},
  {"l2",  0x52,11,NOTINCRATE,0,0xff,0xa9,NCOUNTERS_L2, CSTART_L2},
  {"int", 0x55,12,NOTINCRATE,0,0xff,0xae,NCOUNTERS_INT, CSTART_INT},
  {"fo1", 0x53, 1,NOTINCRATE,0,0xff,0xb4,NCOUNTERS_FO, CSTART_FO},  /* FO dials: 0-5 */
  {"fo2", 0x53, 2,NOTINCRATE,0,0xff,0xb4,NCOUNTERS_FO, CSTART_FO+ 1*NCOUNTERS_FO},
  {"fo3", 0x53, 3,NOTINCRATE,0,0xff,0xb4,NCOUNTERS_FO, CSTART_FO+ 2*NCOUNTERS_FO},
  {"fo4", 0x53, 4,NOTINCRATE,0,0xff,0xb4,NCOUNTERS_FO, CSTART_FO+ 3*NCOUNTERS_FO},
  {"fo5", 0x53, 5,NOTINCRATE,0,0xff,0xb4,NCOUNTERS_FO, CSTART_FO+ 4*NCOUNTERS_FO},
  {"fo6", 0x53, 6,NOTINCRATE,0,0xff,0xb4,NCOUNTERS_FO, CSTART_FO+ 5*NCOUNTERS_FO}
  };

#else
extern Tctpboards ctpboards[NCTPBOARDS];
#endif

/*--------------------------- libctp.a subroutines (see vme/ctp/ctplib): */
int setL0f34c(int lutn, char *m4);
void combine34(w8 *lut34, char *m4);
Tklas *getpClass(int klas);

int getTL1();
int getTL2();
int calcPFablut(int deltathalf, w32* ablut);

void setTimeParsDB(int TL1, int TL2, int TBCL0, int CALIBRATION_BC, int orbl);
w32 calcFO_DELAY_L1CLST();
w32 calcFO_FILTER_L1();
w32 calcBUSY_L0L1DEADTIME();
w32 calcPFisd(int level);
w32 calcL1_DELAY_L0();
w32 calcL2_DELAY_L1();
w32 calcL0_BCOFFSET();
w32 calcL2_BCOFFSET();
w32 calcINT_BCOFFSET();

void readBICfile();
void checkCTP();   // configure
void initCTP();    // initialise system parameters
int softLEDimplemented(int board);
w32 dodif32(w32 before, w32 now);    // substract 2 counters

//FILE *openFile(char *fname); is in ctplib.h
int getINT12fromcfg(char *int1, char *int2, int max12);
void readTables();
int loadcheckctpcfg();
int Detector2Connector(int idet,int *ifoM1,int *iconnectorM1);

void printLTUname(int fo, int foc);
w32 calcOverlap(w32 *bsyclusts);

int ReadTemp(int ix);
void vme2volt(w32 vme );
w32 i2cread(int channel, int branch);
int i2cgetaddr(int board0_34, int *channel, int *branch);

int getEdgeDelayDB(int level, int input, int *edge, int *delay);
int getSwnDB(int input);
int getedge(int board,w32 input,w32 *del);
int getedgerun1(int board,w32 input,w32 *del);
int getedgerun2(int board,w32 input,w32 *del);

void loadBCmasks(w16 *bcmasks);

// Phase measurement tools 
int readadc(int board);
int readadc_s(int board);
void checkPhases(char *line);

/*FGROUP SimpleTests
Dump CTP configuration.
L0 BOARD CLASSES section:
class: L0_CONDITION L0_VETO L0_RATE L0_MASK [L0_INVERT for classes45-50 or all]
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
like getPF
*/
void getprtPF(int ix);
/*FGROUP L0
like getPFc
*/
void getprtPFc(int ix, int circ);

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
/*FGROUP L0
get rnd1 rnd2 bcsc1 bcsd2 int1 int2 intt L0fun1 L0fun2 INTSEL1 INTSEL2 allrare
*/
void getShared();
/*FGROUP L0
get 4096 hexa chars each containing i-bit of LUT4-1. i:0..4095
*/
void getSharedL0f34(int lutout);
/*FGROUP L0
4096 hexa chars from stdin will be loaded to LUT31 32 41 42*/
void setSharedL0f34();

/*FGROUP L0
set rnd1 rnd2 bcsc1 bcsd2 int1 int2 intt L0fun1 L0fun2
*/
void setShared(w32 r1,w32 r2,w32 bs1,w32 bs2, w32 int1,w32 int2,w32 intt,w32 l0fun1,w32 l0fun2);
/*FGROUP L0
set INTERACTSEL ALL_RARE_FLAG
*/
void setShared2(w32 intsel, w32 allrare);
/*----------------------------libctp.a subroutines for new firmware  */
/* FGROUP DbgNewFW 
Load run reading RCFG file in WORK directory 

void loadRun(w32 runnumber); */
/* FGROUP DbgNewFW 
Prints static class CTPHardware.
void printHW(); */
/* FGROUP DbgNewFW 
void unloadRun(w32 runnumber); */
/*FGROUP DbgNewFW */
void printL0FUN34();
/*--------------------------- libctp.a subroutines (see vme/ctp/ctplib): */
/*FGROUP Common */
int notInCrate(int ix);

/*FGROUP Common */
int findBUSYINP(int fo, int foc);

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
Input: dets is pattern of detectors to be checked.  0xffffff: all dets
Operation:
- read busy timers
- sleep 100ms
- read busy timers 
- calculate difference between 2 measurements and compare 
  with busy_timer
rc: busy pattern: [0..23] bits set to 1 correspond to Dead busy inputs
*/
w32 findDeadBusys(w32 dets);

/*FGROUP DebCon */
int  GenSwtrg(int n,char trigtype, int roc, w32 BC,w32 detectors, 
  int customer, w32 *orbitn);
/*FGROUP DebCon */
int getCALIBBC2(w32 ctprodets);
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
input: no sense for busy board.For L0/1/2 boards: L0,L1:1..24   L2:1..12
edge: 0:Positive 1:Negative

Edge: choose negative (for delay:0) if unstability is found around delay 0.
*/
void setEdge(int board,w32 input,w32 edge);
/*FGROUP inputsTools
set Edge/Delay 
Inputs:
board: 1:L0 2:L1 3:L2
input: 1..24 (1..12 for L2)   1..48 for LM0
edge:  0:positive 1:negative
delay: 0..15*/
void setEdgeDelay(int board, int input, int edge, int delay);
/*FGROUP inputsTools
* i48->i24 */
void setSwitch(int i48, int i24);
/*FGROUP inputsTools
*/
void printSwitch();

/*FGROUP inputsTools
Read edge/delay info from hw for all the inputs (clk edge for ORbit
in case of busy board).
Inputs:
  board: 0:busy (the CLK edge for INPUT ORBIT signal)
         1..3: L0/1/2
  input: 1..24 (for L0/1 boards) 1..12 for L2 board

Edge: choose negative (for delay:0) if unstability is found around delay 0.

*/
void printEdgeDelay(int board);
void printEdgeDelayrun1(int board);
//void printEdgeDelayrun2(int board);

// bcmask.c
/*FGROUP L0
read BC masks from HW and print out 3564 4bits words
*/
void getBCmasks();
/*FGROUP L0
set BC masks in HW from input line containing 3564 hexa-chars.
*/
void setBCmasks();
/*FGROUP L0
set/read/check ntimes
words: if 0 than check whole BCmask memory (3564)
*/
void checkBCmasks(int ntimes, int words);
/*FGROUP SimpleTests
return 5 integers in 1 line corresponding to clcock phase on L0/1/2 BUSY INT
9.9.2014 in lab: 
38 113 117 123 126   -with LM0 board in altri1 crate, also in P2
114 114 116 120 123  -with L0  board in altri2 crate

*/
void checkPhasesPrint();
/* Find out toggling FO connectors.
Return the list of names (space separated) of toggled detectors.
"" (empty string) returned if there are not such detectors
INTERROR is returned as one of toggling detectors in case of internal error
*/
void findToggle(char *toggling_dets);
/*FGROUP SimpleTests */
void printToggle();
/*FGROUP SimpleTests */
int Toggle(char *det, int onoff);
/*FGROUP SimpleTests
Resets PLL clock on all boards
*/
void resetPLLS();

/* PF in pfp.c: */
/*FGROUP PF 
Read the recent PF setting
circuit: 1..5. 0: read all 5 circuits
*/
void ReadPF(int circuit);
/*FGROUP PF 
Read the recent PF setting from hw and print string appropriate
for TRIGGER.PFS file.
circuit: 1..5
*/
void ReadPF2str(int circuit);
/*FGROUP PF 
Decode the string defining PF in TRIGGER.PFS 
pfstr: PF definition in format given in TRIGGER.PFS, i.e. 12 0xN in one line:
     L0_PFBLOCK_A L0_PFBLOCK_B L0_PFLUT
     L1_PFBLOCK_A L1_PFBLOCK_B L1_PFLUT
     L2_PFBLOCK_A L2_PFBLOCK_B L2_PFLUT
     L0_PF_COMMON L1_PF_COMMON L2_PF_COMMON
or 1-character string 1..4 (enclosed in double quotation marks):
1 -PF definition taken from PF1 circuit from CTP L0/1/2 boards

Output:
PF_COMMON
intA/B    IR1 (0xa) or IR2(0xc)
                             L0       L1           L2
delLT     Delayed INT LUT  ignored
delSD     signal delay     ignored    max.512    4095

PF_BLOCK+PFLUT
th1/2     0..63
dT        0->257BCs, n->n+1 BCs. Protection interval width
del       On L0: del and f ALWAYS IGNORED
          f=0: Protection end: (del+ PFinterval) before T0
          f=1: del ignored. 
               Protection end: PFinterval before T0
f         Delay flag, see above
dsf       Downscale factor 0..31 in hw shown, corresponding to 1..32 BCs
PLUT      ?
*/
void DecodePF(char *pfstr);
/*FGROUP PF 
Set the PFCOMMON word
*/
void WritePFcommon(w32 INTa,w32 INTb,w32 Delayed_INT);
/*FGROUP PF 
Set the PFBLOCK A and B and PFLUT
dTa and dTb should be in BC
*/
void WritePF(w32 icircuit,w32 THa1,w32 THa2,w32 THb1,w32 THb2,int dTa,int dTb,w32 P_signal);
/*FGROUP PF 
Set PF circuit for INT1 only (for INT1/2 combinations, another
function should be prepared). Note that INT1 should be defined in Shared Resources!
Examples of INT1 definition: it can be BC1,BC2,RND1,RND2 or any logical combination of 
first 4 L0 inputs (INTfun1). For example if we want to define INT1 as L01 input then INTfun1 
will be 0xaaaa, L02 only -> INTfun1=0xcccc, L03 only -> INTfun1=0xf0f0, L04 only -> INTfun1=0xff00,
L01.or.L02.or.L03.or.L04 -> INTfun1=0xfffe, L01.and.L02.and.L03.and.L04 -> INTfun1=0x8000

icircuit: 1..4 - circuit number - there can be 4 PF protections in parallel

bcs: 1..4096 - protected interval in BCs. 
For example: 10mus = 400 BC.

threshold: 0..63 - number of allowed interactions in protected interval 
For PF protection activation N(dT) > Threshold (interaction in question included in N)

For example: 0: kill this event  1: only this event   2: max. 1 additional event
*/
void WritePFuser(w32 icircuit, w32 threshold, w32 bcs);
/*FGROUP PF
Setting for PF on all trigger levels. 
Ncoll - number of collisions
dT1 - protection time interval before interaction
dT2 - protection time interval after interactions
ipf - index to pf {1,2,3,4} for all boards where pf is set
*/
int WritePFuserII(w32 Ncoll,w32 dT1,w32 dT2,w32 icircuit,w32 plut);
