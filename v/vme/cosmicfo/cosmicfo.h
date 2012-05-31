/* LTU VME registers.
Notes:
VME FPGA: 0-3c   -should have the same meaning for any board
         40-7c
LTU FPGA:80-bc   -reserved
         c0-     
"%x"%(4*0x64)    -transl. from Pedja's address (0x64) to VME addr(0x190)-
                  SUBBUSY_TIMER
*/
/* LTU_SW_VER: used for: ltu.exe, ltu_proxy.exe */
// #define LTU_SW_VER "1.5.3 19.4.2007"
//#define LTU_SW_VER "1.7 22.5.2007"
//#define LTU_SW_VER "1.8 5.6.2007"
//#define LTU_SW_VER "1.9 12.6.2007"
//#define LTU_SW_VER "2.0 11.7.2007"
#define LTU_SW_VER "2.1 29.10.0007"

/*REGSTART32 */
/* VME FPGA: */
#define CODE_ADD      0x4     /* board type (0x56 for LTU) */
#define SERIAL_NUMBER 0x8     /* unique serial number  of the board */
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
#define TEMP_STATUS   0x5c
#define TEMP_READ     0x60
#define MASTER_MODE   0x64   /* b4=0 ->TTCvi, b4=1 ->test-mode (LTU snapshot)
				b[3..0] dest. LTU dial address in test-mode */

/* LTU FPGA: */
#define LTUVERSION_ADD  0x80   
/* #define REGTEST         0x8c   valid for 'a4' */
/* #define MASTER_START    0x88   valid for 'a4' */

#define TEST_ADD        0xc0   /* (BLINK) w1->blink all LEDs, 0->stop blinking*/
#define BC_STATUS       0xc4   /* [2:0] Orbit-error, PLL-locked, BC-error */
#define L0_CLEAR_RND   0xc8 /*dummywr: clear RND1/2 (according to L0_ENA_CRND)*/
#define BC_DELAY_ADD    0x4c8  /* 5 bits, step:1ns. ! see TTC_INTERFACE !  */
#define ADC_START       0xcc   /* see ADCI */
#define ADC_DATA        0xd0   /* see ADCI */
/* LTU error emulation: */
#define ERROR_ENABLE    0xd4
#define ERROR_STATUS    0xd8
#define ERROR_SELECTOR  0xdc
#define ERROR_DEMAND    0xe0
#define ERROR_RATE      0x4e4  /* RANDOM errors, 0x7fffffff -> always error*/
#define RANDOM_1       0x4e4 /* bit31: 1: Enable filter (for FPGAVER>=A5) */
#define RANDOM_2       0x4e8
#define L0_ENA_CRND    0x4fc /* 1..0: enable RND2, RND1 clear */


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
				bit0=1 -> BC/2 Orbit delay
				bit1=1 -> L1out == L1strobe
				bit1=0 -> L1out is BC/2 earlier
Connected wirh BC_DELAY_ADD!. Last 2 bits are:
00  for BC_DELAY_ADD: 0,1,20-27
01                    15-19
10                    2-8,28-31
11                    9-14
			        bit2=0 -> L1out positive (for TTCex)
			        bit2=1 -> L1out negative (for TTCvi)
			        bit3=1 -> 'L0 over TTC' mode
				*/
#define STDALONE_MODE   0x534  /* GLOBAL:0 STDALONE:1 */
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
#define FIFO_MAX        0x158  /* 15-8:L2max  7-0:L1max */
/* COUNTERS: */
#define SOFT_LED        0x15c  /* soft LED */
/*
define COUNTER_CLEAR   0x160   DUMMYwr -clear counters, set in COUNTER_MASK
define L1FIFO_COUNTER  0x164   r counts cases, when L1,L2 FIFOs were in 
define L2FIFO_COUNTER  0x168   'NEARLY FULL' condition (>128 words) 
define PP_COUNTER      0x16c
define L0_COUNTER      0x170
define L1_COUNTER      0x174
define L1sCOUNTER      0x178
define L2aCOUNTER      0x17c
define L2rCOUNTER      0x180
define START_COUNTER   0x184
define BUSY_COUNTER    0x188
define BUSY_TIMER      0x18c
define SUBBUSY_TIMER   0x190    BUSY0+BUSY1 contributions only 
define COUNTER_MASK    0x194   r/w bits to be set before DUMMYwr to COUNTER_CLEAR 
*/
#define PIPELINE_CLEAR  0x198   /* dummy w sets the EMU_STATUS[1] bit */
/* 0x19c - 0x1b0 reserved for SnapShot memory */
#define SSMcommand  0x19c  /* 0x0: VMEREAD   0x1: VMEWRITE */
                           /* 0x2 RECAFTER  0x3: RECBEFORE */
                           /* 0x4 if set, '7FPsignals->SSM' mode */
#define SSMstart    0x1a0  /* dummy wr */
#define SSMstop     0x1a4  /* dummy wr */
#define SSMaddress  0x1a8  /* w/r */
#define SSMdata     0x1ac  /* w/r */
#define SSMstatus   0x1b0  /* read only. Bits[2:0]: BUSY, OPERATION, MODE
			      Bit[4] '7FPsignals->SSM' selection */

/*                      0x1b4   not assigned */
#define TIMING_TEST     0x1b8   /* wr only, bit0: 0->disable, 1->enable */
#define PLL_RESET       0x1bc   /*DUMMY write should always after 
				  BC_DELAY_ADD change*/
#define TIMING_TEST     0x1b8   /* wr only, bit0: 0->disable, 1->enable */

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
#define START_COUNTERrp   8 
#define BUSY_COUNTERrp    9
#define PP_COUNTERrp     15
#define L0_COUNTERrp     16
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

// LTU, ttc parameters from DB.   /*X -used by ltu_proxy
typedef struct {
  int l1format;
  //-------------- see ttcsubs.c ttcDefaults
  int ppdelay;    // default prepulse delay (on TTCvi board)
  int FineDelay1,FineDelay2;  // TTCvi board. In picsecs.
  int CoarseDelay;            // TTCvi. Direct value written in register
  //-------------- see ltuinit.c ltuDefaults
  int ltu_LHCGAPVETO; /*X 0:OFF (default for 2004 beam) 8:ON */
  float ltu_event_rate; /*X ltu event rate in Hertz for BC or random mode */
  int ltu_sodeod_present; /*X EOD/SOD 0:not present  */
  int ltu_autostart_signal; /*X 0:sw 2:random 3:BC 
    5:pulser_edge 1:pulser_level*/
  int busy; /*X 0:not used, 1:busy1 valid, 2:busy2 valid, 3: both busy valid */
  int L0;   /*X 0:over cable 8: over TTC */
  int orbitbc;   /*X ORBIT_BC */
  int dim;       /*X 1:start DIM services   0: do not start DIM services */
  int bc_delay_add;   /*X -temporarily for acorde */
  int ttcrx_reset;   /*X -temporarily for sdd */
  char mainEmulationSequence[64]; /*X name of file defining 
    the main CTP emulation sequence in VMEWORKDIR/CFG/ltu/SLMproxy/ */
} Tltucfg;
/*the structure of shared memory segment shared among:
- ltu_proxy
- ltu.exe (popened from ltu_proxy)
- ltu.exe -started directly from cmd line (ltu/ltu.exe or vmecrate ltu=0x81...)
Shared memory segment is allocated in first instance of ltu.exe
Adding new item:
ltu.h
ltulib/ltuinit.c: ltuDefaults, printltuDefaults, setOption
ltu_proxy/ltu_utils.c:  ltu_configure
- edit $VMEWORKDIR/CFG/ltu/ltuttc.cfg
*/
typedef struct {
  w32 id;    // shmkey (filled when allocated)
  Tltucfg ltucfg;
  w32 ltucnts[LTUNCOUNTERSall];
} Tltushm;

EXTRN Tltushm *ltushm;

/* ltulib functions: */
/*FGROUP "ExpertConf"
Set parameter name to value
*/
int setOption(char *name, char *value);

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

void ltuInit(Tltucfg *ltc);
void ltuDefaults(Tltucfg *ttcpars);

void initStatic();
void ttcDefaults(Tltucfg *ttcpars);
void readltuttcdb(Tltucfg *ttcpars);
/* FGROUP "Configuration" 
- initialise TTCvi 
- reset TTCrx (over fibre)
- set Control register 3 in TTCrx (over fibre), enabling Dout strobe */
int TTCinit();
/*FGROUP "Configuration" 
Show shared memory content. Shared memory variables can be 
modified by Load from memory/Save buttons in Defaults editor.
*/
void printltuDefaults();
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
Coarse: the required content of TTCrx Coarse register
*/
int ttcDelays(int Fine1ps, int Fine2ps, int Coarse);
/*FGROUP "Configuration"
-send 'Front End Electronics reset command' through TTC B-channel.
 The header '0x8' is reserved for this command (e.g. 0x3 is allocated
 for L2accept header).
*/
int ttcFEEreset();
/*FGROUP Configuration GUI setGLOBAL
*/
/*FGROUP Configuration GUI setSTDALONE
*/
/*FGROUP SimpleTests
returns temperature on the board in centigrades (-100 if error) */
int ReadTemperature();

void setglobalmode();
int getsgmode();
void setstdalonemode();
void readCounters(w32 *mem, int N, int accrual);
w32 getCounter(int reladr);
void getCounters(int N, int accrual);
void clearCounters();
/*FGROUP SimpleTests */
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
