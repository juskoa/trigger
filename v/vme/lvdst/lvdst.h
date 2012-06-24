#ifndef LTU_H_INCLUDED
#define LTU_H_INCLUDED

/* LTU VME registers.
LTUVERSION_ADD: 0xf3 ltu   -see vme/ltu. Last LTU+TTCVI version.
                0xc1 trigger input generator   -see vme/ltu
                0xe1 lvdst -see vme/lvdst
                0xcd cosmicfo -see vme/cosmicfo
                0xb0 ltuvi
                0xb3 rate limit implemented (TPC)
                0xb4 higher L2_DELAY
trigger input generator:
-----------------------
L0 top connector -CTP L0 trigger input
L0 middle        -CTP L1 trigger input
L0 bottom        -delayed L1 trigger input
All three inputs have a correct, negative polarity
The delay: see PULSE_DELAY register
All three L0 signals toggle if TRG_TOGGLE[0] is set to 1

The L1 input and the delayed L1 input are available on scope: 
chan A bit 8 and 9 (instead of VMEread/write strobe).
The emulated sequence should be L2r, transmission of L1M supressed,
L0 over TTC to be used.

Notes:
------
VME FPGA: 0-3c   -should have the same meaning for any board
         40-7c
LTU FPGA:80-bc   -reserved
         c0-     
"%x"%(4*0x64)    -transl. from Pedja's address (0x64) to VME addr(0x190)-
                  SUBBUSY_TIMER
*/
/* LTU_SW_VER: used for: ltu.exe, ltu_proxy.exe */
//#define LTU_SW_VER "2.3 16.5.2008"  // last F3 version
//#define LTU_SW_VER "3.0 4.6.2008"     // 1st ltuvi version
//#define LTU_SW_VER "3.1 25.6.2008" 
//#define LTU_SW_VER "3.2 23.9.2008"    // RATE_LIMIT, PLL measured
//#define LTU_SW_VER "3.3 16.12.2008"   // BCPHASE measurements
//#define LTU_SW_VER "3.4 11.03.2009"   // web release
//#define LTU_SW_VER "3.5 20.07.2009"   // better RATE_LIMIT
//#define LTU_SW_VER "3.6 08.09.2009"   // better RATE_LIMIT, 0xb4
//#define LTU_SW_VER "3.6 04.02.2010"   // better RATE_LIMIT, 0xb4
//#define LTU_SW_VER "3.7 15.06.2010"   // lnxpool31
//#define LTU_SW_VER "3.8 19.07.2010"   // 0xb5, RATE_LIMIT +3bits
//#define LTU_SW_VER "4.0 19.2.2012"     // 0xb6, +ltu2_b6.rbf
#define LTU_SW_VER "4.1 04.05.2012"     // SYNC SMI cmd added

/*REGSTART32 */
/* VME FPGA: */
#define CODE_ADD      0x4     /* board type (0x56 for LTU) */
#define SERIAL_NUMBER 0x8     /* unique serial number  of the board 
6bits with LTUs before 2012 number: 1..53. 
7bits with new LTUs (1st batch in 2012). Serial number for these: 64.. */
#define VERSION_ADD   0xC     /* VME FPGA firmware version */
#define SOFT_RESET    0x28
/*REGEND */

/* following symb. names are not visible from GUI stdfuncs/vmerw ... */
#define FlashAddClear 0x48           /*  12  Flash memory */ 
#define FlashAccessIncr 0x40         /*  10  */
#define FlashAccessNoIncr 0x44       /*  11  */
#define FlashStatus 0x4c             /*  13  */
#define ConfigStart 0x54             /*  15  */ 
#define ConfigStatus 0x50            /*  14  */

/*REGSTART32 */
#define TEMP_START    0x58   /* LTU temperature: */
#define TEMP_STATUS   0x5c   /* bit0: temp, 
for new LTUs (serial>=64):      bit1: CRCerror 1:error 0:ok */
#define TEMP_READ     0x60 
#define MASTER_MODE   0x64   /* b4=0 ->TTCvi, b4=1 ->test-mode (LTU snapshot)
				b[3..0] dest. LTU dial address in test-mode */

/* LTU FPGA: */
#define LTUVERSION_ADD  0x80   /* see above */
/* #define REGTEST         0x8c   valid for 'a4' */
/* #define MASTER_START    0x88   valid for 'a4' */

#define TEST_ADD        0xc0   /* (BLINK) w1->blink all LEDs, 0->stop blinking*/
#define BC_STATUS       0xc4   /* [2:0] Orbit-error, PLL-locked, BC-error */
//#define ADC_SELECT      0x4c0 
 /* from b2 version (BC clock phase measurement):
bit0: 0: L1_DATA toggle (default) (FO BC phase measured)
      1: LTU BC phase  (PLL phase measured for LTU) */
#define BC_DELAY_ADD    0x4c8  /* 5 bits, step:1ns. ! see TTC_INTERFACE !  */
#define ADC_START       0xcc   /* see ADCI */
#define ADC_DATA        0xd0   /* see ADCI */
/* LTU error emulation: */
#define ERROR_ENABLE    0xd4
//#define ERROR_STATUS    0xd8
#define ERROR_SELECTOR  0xdc
#define ERROR_DEMAND    0xe0
#define ERROR_RATE      0x4e4  /* RANDOM errors, 0x7fffffff -> always error*/
#define ORBIT_CLEAR     0xe8   /* clear emulator's orbit counter */
#define EMULATION_START 0xec   /* Start emulation word addr. */
#define QUIT_SET        0xf0
#define BREAK_SET       0xf4

#define LAST_BC         0x4f8   /* #of Last BC in Orbit def: 3563 */ 
#define PREPULSE_BC     0x4fc   /* BC # of the PP, def: 128 */
#define CALIBRATION_BC  0x500  /* BC of cal. trigger, def: 3556 */
#define GAP_BC          0x504  /* 1st BC of LHC LARGE GAP, def: 3446 */
#define ORBIT_BC        0x508  /* BC of the emulator's orbit, def: 1 */
#define L1_DELAY        0x50c  /* Emulator's L0-L1 (in BCs), def: 224 */
#define L2_DELAY        0x510  /* Emulator's L0-L2 (in BCs), def:3520 */
/*#define PULSE_DELAY     0x5c0   Only for 0xc1 version!. [3:0]: delay of
                                  L1 delayed 0-15 BCs */
#define TRG_TOGGLE      0x5c4  /* Only for 0xc1 version!. 1: toggle L0 outputs*/
#define ORBIT_TIME      0x5c0  /*Ltuvi.Start of ORBIT inhibit interval 0-3563 */
#define PP_TIME         0x5c4  /*Ltuvi.Start of PP inhibit interval 0-3563 */
#define TTC_DATA        0x5c8  /* 30..17: TTCrx addr (0: broadcast)
                                  16:     E (1:external, 0:internal)
                                  15..8:  sub-address   7..0:data */
#define RATE_LIMIT      0x5cc  /* 31: enable ratelimit, 15..8:period in 0.82ms
                                  5..0:limit, i.e. max. # of triggers */
/* LTU emulator: */
#define SLM_ADD_CLEAR   0x114  /* clear both (SLM r/w) add. cnts */
#define SLM_DATA        0x118  
#define EMU_STATUS      0x11c  /* Emulation status word (bit[0]:EMU active) */
                               /* bit1: pipeline busy */
                               /* bit2: TIMING_TEST enabled */
#define START_SET       0x520  /* start signal selector */
                               /* 0x1 pulser input
				* 0x2 Random START
				* 0x3 Scaled-down BC start
				* 0x4 +above bits: rising edge sensitive pulser
				* 0x0 +above bits: level sens. pulser mode
				* 0x8 +above LHC gap veto */
#define SOFT_TRIGGER    0x124  /* Sw generated START (dummy data write) */
#define RANDOM_NUMBER   0x528  /* Rate of random START */
#define COUNT_PERIOD    0x52c  /* Period of scaled-down BC-START (max.0x7fffffff) */
#define TTC_INTERFACE   0x530  /* L1/PP polarity and delay 
bit3=1 -> 'L0 over TTC' mode
LTUvi:
  bit0: Delay CahnnelB flag (+BC/2)
  bit1: Delay CahnnelA flag
  bit2: not used
LTUf3:
  bits[1..0]
  00  for BC_DELAY_ADD: 0,1,20-27
  01                    15-19
  10                    2-8,28-31
  11                    9-14
  bit2=0 -> L1out positive (for TTCex)
  bit2=1 -> L1out negative (for TTCvi)
				*/
#define STDALONE_MODE   0x534  /* bit0: GLOBAL:0 STDALONE:1 
  for ltuver>=0xb6 also bit1: ext. orbit:1 int. orbit:0 */
/* BUSY configuration: */
#define BUSY_ENABLE     0x138  /* w 0x3 enables BUSY1,2 */
#define BUSY_STATUS     0x13c
#define SW_BUSY         0x140  /* w 1: set SWBUSY, w 0:clear SWBUSY */
#define L1_FORMAT       0x544  /* bit0=1:L1enable, bit1=0: L1header only */
#define L1MAX_CLEAR     0x148  
#define L2MAX_CLEAR     0x14c  /* dummy write: clears the part of FIFO_MAX counter */
#define SCOPE_SELECT    0x550  /* groupB: 0x1e0 groupA: 0x1f. 0x17+0x17-not selected */
#define BACKPLANE_EN    0x554  /* bit0-7:BC,ORBIT,L2D,L2S,L1D,L1/L1S,L0,PP */
                               /* bit8: enable logic analyser oututs */
#define MINIMAX_SELECT  0x570
#define MINIMAX_LIMIT   0x578
#define FIFO_MAX        0x158  /* 15-8:L2max  7-0:L1max */
/* COUNTERS: */
#define SOFT_LED        0x15c  /* soft LED */
#define BUSYMAX_DATA    0x168
#define BUSYMINI_DATA   0x16c
#define MINIMAX_CLEAR   0x174
#define PIPELINE_CLEAR  0x198   /* dummy w sets the EMU_STATUS[1] bit */
/* 0x19c - 0x1b0 reserved for SnapShot memory */
#define SSMcommand  0x19c  /* bit0:mode: 0:VMEREAD   1:VMEWRITE */
                           /* bit2:operation: 0:RECAFTER  0x3:RECBEFORE */
                           /* bit4:CTPflag: '7FPsignals->SSM' mode */
#define SSMstart    0x1a0  /* dummy wr */
#define SSMstop     0x1a4  /* dummy wr */
#define SSMaddress  0x1a8  /* w/r */
#define SSMdata     0x1ac  /* w/r */
#define SSMstatus   0x1b0  /* read only. Bits[2:0]: BUSY, OPERATION, MODE
			      Bit[4] '7FPsignals->SSM' i.e.CTP flag selected */

/*                      0x1b4   not assigned */
#define TIMING_TEST     0x1b8   /* wr only, bit0: 0->disable, 1->enable */
#define PLL_RESET       0x1bc   /*DUMMY write should always after 
				  BC_DELAY_ADD change*/
#define TIMING_TEST     0x1b8   /* wr only, bit0: 0->disable, 1->enable */
#define TTC_STATUS     0x1c8   /* bit0: TTCfifo empty   bit1:TTCfifo full */
#define COPYCOUNT      0x1d4   /*dummy wr. copy counters CMD */
#define COPYBUSY       0x1d8   /*ro [0] copy-busy status */
#define COPYCLEARADD   0x1dc   /*dummy wr. clear copy mem. add. */
#define COPYREAD       0x1e0   /*ro copy memory data */
#define CLEARCOUNTER   0x5ac /* clear counters CMD */
/*REGEND */

#define LTUNCOUNTERSall   26 
#define LTUNCOUNTERS   21
#define LTU_TIMErp        0
#define SUBBUSY1_TIMERrp  1
#define SUBBUSY2_TIMERrp  2
#define SUBBUSY_TIMERrp   3
#define BUSY_TIMERrp      4
#define L1FIFO_NFrp       5 
#define L2FIFO_NFrp       6 
#define BC_ERRORrp        7 
#define START_COUNTERrp   8 
#define BUSY_COUNTERrp    9
#define L1FIFO_NF_TSrp   10
#define L2FIFO_NF_TSrp   11 
#define ANYERRORrp       12 
#define BC_ERROR_TSrp    13 
#define LONGBUSYrp       14 
#define PP_COUNTERrp     15
#define L0_COUNTERrp     16
#define L1_ONLYrp        17
#define L1_STROBErp      18
#define L2a_COUNTERrp    19
#define L2r_COUNTERrp    20
#define temperaturerp    23
#define epochsecsrp      24
#define epochmicsrp      25

#define BC_STATUSorbiterr 0x4  /* valid only in GLOBAL mode */
#define BC_STATUSpll 0x2
#define BC_STATUSerr 0x1

/* BUSY_STATUS bits: */
#define BUSY_ENABLE0 0x0
#define BUSY_ENABLE1 0x1
#define BUSY_ACTIVE  0x80

/*Master interface test the following is hardwired in ltu
 * master interface:
#define TTCVIBASE 0x800100
#define TTCVI_FIFO2 0xb8
*/

/* LVDST specific: */
#define ADC_SELECT    0x480  /* ADC chan selection: 1/2:chan /2 0/3:pattern */
#define PATTERN_SEL   0x484  /* 0:None (ground) 1:Pattern+Period, 2:Random 3:Toggle */
#define SEQ_PERIOD    0x488  /* 12bits in BCs */
#define SEQ_DATA      0x48c  /* 24bits data */
#define RANDOM_RATE   0x490  /* 31 bits */
#define SYN_EDGE      0x494  /* clock polarity */
#define DELAY_1       0x498  /* 5bits in BCs */
#define DELAY_2       0x49c  /* 5bits */
#define CLEAR_ERR1    0x4a0  /* clear error latch1 */
#define CLEAR_ERR2    0x4a4  /* clear error latch2 */
#define ERROR_STATUS  0x200  /* RO, status of attached errors */



#define DUMMYVAL 0xffffffff   /* recommended for DUMMY writes */
/* wait GLBSTDDELAY before START emulation, GLB/STDALONE switch */
#define GLBSTDDELAY 120

#define MAXSLMW 256    /* length of the SLM memory */

/*LTUMAIN id defined ONLY in: 
ltu/ltu.c
ltu_proxy/ltu_proxy.c
*/
#ifdef LTUMAIN
#define EXTRN
#else
#define EXTRN extern
#endif

EXTRN w32 Gltuver;  // has to be 0xf3 or 0xbX

// LTU, ttc parameters from DB.   /*X -used by ltu_proxy
#define MAXPARLIST 20
#define FLGG_calibration 0  // bit position in flags
#define FLGlogtimestamp 1   // 1: time stamps for ltu log (todo)
#define FLGfecmd12 2        // 1: hmpid (fecmd12 to be sent) 0: not hmpid
#define FLGextorbit 0x4     // 1: external 0: internal orbit
#define IXG_calibration_roc 0 
#define IXltuver 1 
#define IXGpp_time 2 
#define IXSpp_time 3 
#define IXGorbit_time 4 
#define IXSorbit_time 5 
#define IXrate_limit 6 
#define IXdefedit_id 7 
//#define IXbc_offset 6 
typedef struct {
  int l1format; // "L1_FORMAT"
  //-------------- see ttcsubs.c ttcDefaults
  int ppdelay;    // default prepulse delay (on TTCvi board)
  int FineDelay1,FineDelay2;  // TTCvi board. In picsecs.
  int CoarseDelay;            // TTCvi. Direct value written in register
  //-------------- see ltuinit.c ltuDefaults
  int ltu_LHCGAPVETO; /*X 0:OFF (default for 2004 beam) 8:ON */
  float ltu_event_rate; /*"BCRATE" X ltu event rate in Hertz for BC or random mode */
  int ltu_sodeod_present; /*X "SODEOD" 0:not present  */
  int ltu_autostart_signal; /* "MODE" X 0:sw 2:random 3:BC 
    5:pulser_edge 1:pulser_level*/
  int busy; /* "BUSY" X 0:not used, 1:busy1 valid, 2:busy2 valid, 3: both busy valid */
  int L0;   /*"L0" X 0:over cable 8: over TTC */
  int orbitbc;   /*X "ORBIT_BC"  on LTU*/
  int dim;       /*X 1:start DIM services   0: do not start DIM services */
  int bc_delay_add;   /*X -temporarily for acorde */
  int ttcrx_reset;   /* YES:1 NO:0 INIT:2 */
  char mainEmulationSequence[64]; /*X name of file defining 
    the main CTP emulation sequence in VMEWORKDIR/CFG/ltu/SLMproxy/ */
  // added 21.2.
  int Sbgo0delay;    // default Bgo0 delay stdalone (on TTCvi board)
  int Gbgo0delay;    // default Bgo0 delay global (on TTCvi board)
  int calibbc;   /*X CALIBRATION_BC  on LTU*/
  int Gppdelay;    // default prepulse delay (on TTCvi board) in global
  w32 flags;       // see FLGxxx above
  float global_calibration_rate;   //valid if FLGglobal_calibration 
  w32 plist[MAXPARLIST];
} Tltucfg;
/*the structure of shared memory segment shared among:
- ltu_proxy
- ltu.exe (popened from ltu_proxy)
- ltu.exe -started directly from cmd line (ltu/ltu.exe or vmecrate ltu=0x81...)
Shared memory segment is allocated in first instance of ltu.exe
Adding new item:
ltu.h
ltulib/ltuinit.c: 
  ltuDefaults()      -initial values in shm (valid if not in DB)
  printltuDefaults() -print values from shared mamory, 
  setOption()        -convert string->shm or shm->stdout
ltu_proxy/ltu_utils.c:  
  ltu_configure()    -shm->LTUhw init
- edit $VMECFDIR/CFG/ltu/ltuttc.cfgall    (the list of all posible pars)
- edit $VMEWORKDIR/CFG/ltu/ltuttc.cfg     (private pars for particular LTU)
*/
typedef struct {
  w32 id;    // shmkey (filled when allocated)
  Tltucfg ltucfg;
  w32 ltucnts[LTUNCOUNTERSall];
} Tltushm;
/*
typedef union a{
  struct {
    int min,max;
  } inttype;
  struct {
    float fmin,fmax;
  };
  struct {
    int flagindex;
  };
  struct {
    int index;
  };
} Tpartypes;

typedef struct {
  char name[40];
  int partype;
  void *addr;
  Tpartypes parspec;
} Tpardesc;
*/

EXTRN Tltushm *ltushm;

#define ltuvino ((Gltuver&0xf0)!=0xb0)
#define ltuviyes ((Gltuver&0xf0)==0xb0)

/* ltulib functions: */
/*FGROUP "ExpertConf"
Set parameter name to value
*/
int setOption(char *name, char *value);
int setOptionMem(char *name, char *value, Tltucfg *ltc);
void copyltucfg(Tltucfg *dest, Tltucfg *src);

w32 readValFile();

/*FGROUP Configuration
Set 'L0 mode' :
'L0 over fibre'         if ttcint is 8
'L0 over cable'         if ttcint is 0

This routine sets the TTC_INTERFACE register. It calculates
2 least sifnificant bits (from BC_DELAY_ADD register). Bits 2..3 are
taken from ttcint parameter.
setTTCint() should be invoked ALWAYS AFTER BC_DELAY_ADD change.
The 4 least significant bits of this register are used for setting:
- 12.5ns delay for Orbit and/or L1
- the polarity of L1 output      
- 'L0 over TTC' mode

TTC_INTERFACE bits:
0  'Delay Orbit' flag. 0: no delay, 1: 12.5ns delay
1  'Delay L1' flag. 0: no delay, 1: 12.5ns delay
   Both bits depends on BC_DELAY_ADD.
2   L1 polarity.    0: positive (TTCex, default)   1: negative (TTCvi)
3  'L0 over TTC' flag. 
   0:  normal mode (only L1 as 1 bit over A-TTC channel)
   1:  'L0overTTC' mode. In this mode: 
   - L0 is sent over coaxial cable as in normal mode. In addition,
        it is sent over A-TTC channel as 2 bits '10' in 2 BC clocks
   - L1 is sent as 2 bits '11' over A-TTCchannel
*/
void setTTCint(w32 ttcint);
/*FGROUP Configuration 
set BC_DELAY_ADD (0..31 in ns). setTTCint is activated after BC_DELAY_ADD
written.
*/
int fpgainit();
void setBC_DELAY_ADD(int delay);

void ltuInit(Tltucfg *ltc, int stdalone, int secs);
void ltuDefaults(Tltucfg *ttcpars);

void initStatic();
void ttcDefaults(Tltucfg *ttcpars);
void readltuttcdb(Tltucfg *ttcpars);
/*FGROUP "Configuration" 
- initialise TTCvi for operation in STANDALONE mode
- reset TTCrx over fibre (TTCrxready goes OFF for ~6ms)
- set Control register 3 in TTCrx (over fibre), enabling Dout strobe 
- set FineDelay1,FineDelay2 and CoarseDelay TTCrx registers 
- send FEEreset (after more than 1 second)
- check BUSY signal and print Warnings and/or Errors:
  Warning: BUSY is ON before TTCrx reset
  Error: BUSY not raised during 'TTCrx READY' off
  Warning: BUSY not raised during 'QPLL LOCKED' off (if QPLL clock is used)
  Error: BUSY ON 10milsecs after FEEreset
*/
int TTCinit();
void TTCrxregs(Tltucfg *ltc);
void TTCrxreset();
/* FGROUP "ConfiguratioH" 
- initialise TTCvi 
- reset TTCrx (over fibre)
- set Control register 3 in TTCrx (over fibre), enabling Dout strobe
Input: stdalone=1 if values valid for standalone are to be loaded
                0 if global configuration is required 
       secs: number of seconds to wait till FEE becomes ready
*/
int TTCinitgs(int stdalone, int secs, Tltucfg *ltc);
/*FGROUP "Configuration" 
Show shared memory content. Shared memory variables can be 
modified by Load from memory/Save buttons in Defaults editor.
shmtemp: 1 shm    0: temp
*/
void printltuDefaults(); //int shmtemp);
void printltuDefaultsMem(Tltucfg *ltc);
/*FGROUP "Configuration"
Set Prepulse delay directly on TTCvi board. 
Warning: delay is set only on TTCviboard (i.e. it is not changed
in memory, which means, TTCinit will set it back to value
stored in shared memory (see Defaults editor)
*/
int ttcPPdelay(int bc);
/*FGROUP "Configuration"
Set delay registers directly in TTCrx, without resetting TTCrx.
Input:
Fine1ps, Fine2ps: required Fine delays on TTCrx chip in ps
The coreespoding value in TTCrx registers in can be set with 104.17 ps step
(e.g. the value corresponding to 4ps or 104 ps is equal: 0xe ) 

Coarse: the required content of TTCrx Coarse register
*/
int ttcDelays(int Fine1ps, int Fine2ps, int Coarse);
/*FGROUP "Configuration"
-send 'Front End Electronics reset command' through TTC B-channel.
 The header '0x8' is reserved for this command (e.g. 0x3 is allocated
 for L2accept header).
*/
int ttcFEEreset();
/*FGROUP "Configuration"
-send 'Front End Electronics user defined command' through TTC B-channel.
Command: number 8-15 
     8 already reserved for FEEreset
 9..11 reserved for CTP
12..15 available for subdetectors

12 data bits are set to 0.
Note: 
   0 not used
1..7 used for L1h, L1data, L2ah, L2adata, L2r, RoIh, RoIdata
*/
int ttcFEEcmd(int Command);
/*FGROUP Configuration GUI setGLOBAL
*/
/*FGROUP Configuration GUI setSTDALONE
*/
/*FGROUP SimpleTests
returns temperature on the board in centigrades (-100 if error) */
int ReadTemperature();

void setglobalmode();
int getsgmode();
void setstdalonemode(w32 b2);
/*FGROUP ConfiguratioH */
int getgltuver();
void readCounters(w32 *mem, int N, int accrual);
w32 getCounter(int reladr);
void getCounters(int N, int accrual, int bakery_customer);
void clearCounters();
/*FGROUP SimpleTests 
Return average busy time (busy/L0) during 100milsecs
*/
int measureBusy100ms();
/*FGROUP SimpleTests 
Measure busy between 2 mouse clicks
*/
int measureBusy();
/*FGROUP SLM */
void SLMsetstart(w32 sel);
/*FGROUP SLM */
int SLMgetstart();
/*FGROUP SLM */
int SLMswstart(int n, int milsecs);
/*FGROUP SLM */
int SLMstart();
/*FGROUP SLM */
void ERenadis(int enadis);
/*FGROUP SLM */
void ERsetselector(w32 selector);
/*FGROUP SLM */
void ERdemand(w32 demand);

/*FGROUP SLM */
int SLMreadasci(char *filen, w32 *slmdata);
/*FGROUP SLM */
int SLMload(char *filen);
/*FGROUP SLM */
int SLMquit();
/*FGROUP SLM */
int SLMwaitemuend(int micsec);

/*FGROUP ConfiguratioH */
void setBUSY(w32 binputs);
/*FGROUP FrontPanel */
void setAB(w32 A, w32 B);

/*FGROUP SSM */
int SSMsetom(w32 opmo);
/*FGROUP SSM */
void SSMstartrec(w32 mode);
/*FGROUP SSM */
void SSMstoprec();
/*FGROUP SSM */
int SSMdump();
/*FGROUP SSM */
void SSMschedule(w32 whenmode);
/*FGROUP SSM */
int SSMclearac();
/*FGROUP SSM */
void SSMclear();
int readSSM(w32 *sm);
int checkSignature(w32 *sm,int *channels,int offset); 
char *getAB(w32 opmo);

/*FGROUP ADC
Set BC_DELAY_ADD 
- WITH TTC_INTERFACE correction (ttcint=0)
- WITHOUT TTC_INTERFACE correction (ttcint=1)
  16.12.2008: measured for both in the lab (for hmpid 0x811000),
              but no difference was seen
*/
void setbcdelay(w32 delay, int ttcint);
/*FGROUP ADC
rc: 2 BC_STATUS low bits: [BC_STATUSpll, BC_STATUSerr] */
w32 getbcstatus();
/*FGROUP ADC
*/
void pllreset();
/*FGROUP ADC
Reads adc 300 times as quickly as possible and print it.
*/
void adcitest();
/*FGROUP ADC
Reads ADC checking for busy and timeout.
*/
int readadc();
/*FGROUP ADC
Reads ADC using readadc and checking that two subsequent values are the same.
*/
int readadc_s();
/*FGROUP ADC
Scan of BC delay with going from 0 to 31 
waiting micseconds between measurements.
Do not change TTC_INTERFACE word during scan !

This subroutine is used with BC phase measurements i.e.
1st line of stdout is relevant.
*/
void scan(int micseconds);
/*FGROUP ADC
Measure BC phase of this FO (ltufo=0) or this LTU (ltufo=1)
FO:
- arrange toggling on FO connector + global mode for this LTU
  before calling this routine
LTU:
- just call this routine
Operation:
- check if global ( the check for toggling to be done)
- store BC_DELAY_ADD, measure
- restore BC_DELAY_ADD
stdout: value of ADC (for BC_DELAY_ADD=0)
        0      may indicate 'not toggling' for FO measurement
        0..128 measured phase
        256    indicates 'not global' for FO measurement (toggling not done yet)
NOTE:
For FO measurement (getbcphase(0)), the change of cable length
leads to change of phase.
*/
void getbcphase(int ltufo);
/*FGROUP ADC
Demonstrates time constant parameters of RL element in delay line.
*/
void adctimeconst(w32 delay0,w32 delay1);

/*FGROUP ADC
Generates random delays and measure adc for each of them.
Stdout:
<delay> <adc_value>
...
<0.00> <4.524>
Where:
delay, adc_value: measured pairs as integers 0..31 and 0..127
Last line: avarage wait for PLL locked
*/
void rndtest();

/*FGROUP Configuration GUI ADC_Scan
Automatic synchronisation of CTP and LTU signal transitions. 
Before this measurement is started,
arrange the following:
- this LTU is in GLOBAL mode
- L1 Data signal (CTP input connector) is continuously toggled
  by BCclock frequency/2
*/

#endif

