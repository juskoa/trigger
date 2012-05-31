/* LVDST VME registers.
Notes:
VME FPGA: 0-3c   -should have the same meaning for any board
         40-7c
LTU FPGA:80-bc   -reserved
         c0-     
"%x"%(4*0x64)    -transl. from Pedja's address (0x64) to VME addr(0x190)-
                  SUBBUSY_TIMER
*/
/*REGSTART32 */
/* VME FPGA: */
#define CODE_ADD      0x4     /* board type (0x56 for LTU or LVDST) */
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

/* LVDST FPGA: */
#define LTUVERSION_ADD  0x80   /* E0 for first LVDST version */

#define TEST_ADD        0xc0   /* (BLINK) w1->blink all LEDs, 0->stop blinking*/
#define BC_STATUS       0xc4   /* [2:0] Orbit-error, PLL-locked, BC-error */
#define BC_DELAY_ADD    0x4c8  /* 5 bits, step: 1ns  */
#define ADC_START       0xcc   /* see ADCI */
#define ADC_DATA        0xd0   /* see ADCI */

#define SCOPE_SELECT    0x550  /* groupB: 0x1e0 groupA: 0x1f. 0x17+0x17-not selected */
#define SOFT_LED        0x15c  /* soft LED */

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
//#define TIMING_TEST     0x1b8   /* wr only, bit0: 0->disable, 1->enable */
#define PLL_RESET       0x1bc   /*DUMMY write should always after 
				  BC_DELAY_ADD change*/

#define COPYCOUNT      0x1d4   /*dummy wr. copy counters CMD */
#define COPYBUSY       0x1d8   /*ro [0] copy-busy status */
#define COPYCLEARADD   0x1dc   /*dummy wr. clear copy mem. add. */
#define COPYREAD       0x1e0   /*ro copy memory data */
#define CLEARCOUNTER   0x5ac /* clear counters CMD */

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

/*REGEND */

#define BC_STATUSorbiterr 0x4  /* valid only in GLOBAL mode */
#define BC_STATUSpll 0x2
#define BC_STATUSerr 0x1

#define DUMMYVAL 0xffffffff   /* recommended for DUMMY writes */

/* ltulib functions:
int ttcreset();
void ltuinit();
void setglobalmode();
int getsgmode();
void setstdalonemode();
w32 getCounter(int reladr);
void getCounters(int accrual);
void clearCounters();
*/
void readCounters(w32 *mem, int N, int accrual);

// LTU, ttc parameters from DB.   /*X -used by ltu_proxy
typedef struct {
  int l1format;
  int ppdelay;    // default prepulse delay (on TTCvi board)
  int ltu_LHCGAPVETO; /*X 0:OFF (default for 2004 beam) 8:ON */
  float ltu_event_rate; /*X ltu event rate in Hertz for BC or random mode */
  int ltu_sodeod_present; /*X EOD/SOD 0:not present  */
  int ltu_autostart_signal; /*X 0:sw 2:random 3:BC 
    5:pulser_edge 1:pulser_level*/
  int busy; /*X 0:not used, 1:busy1 valid, 2:busy2 valid, 3: both busy valid */
  int L0;   /*X 0:over cable 8: over TTC */
  int orbitbc;   /*X ORBIT_BC */
  int dim;       /*X 1:start DIM services   0: do not start DIM services */
  char mainEmulationSequence[64]; /*X name of file defining 
    the main CTP emulation sequence in VMEWORKDIR/CFG/ltu/SLMproxy/ */
} Tltucfg;

Tltucfg ltucfg;

void ltuDefaults(Tltucfg *ttcpars);
