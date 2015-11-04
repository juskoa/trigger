
/*REGSTART32 */
#define BSET 0x10
#define BCLEAR 0x14
#define PROGRAM_ID 0xc
#define WORKING_MODE 0x7fa78
#define BST_Machine_Mode 0x7fa9c
#define TTCrx_status 0x7faa0   // bit0:0 TTCrx not ready 1:40MHz clock present

/* *QPLL_STATUS  (page 10)
bit0: 0:QPLL not locked   1: QPLL locked
bit1: 0:QPLL OK           1: QPLL error
 it remembers state of error until read
*/
#define BC1_QPLL_STATUS 0x7fbe8
#define BC2_QPLL_STATUS 0x7fbb8
#define BCref_QPLL_STATUS 0x7fb98
#define BCmain_QPLL_STATUS 0x7fb7c

/* 0: relock only after reset   1: relock automatically */
#define BC1_QPLL_MODE 0x7fbf0
#define BC2_QPLL_MODE 0x7fbc0
#define BCref_QPLL_MODE 0x7fba0
#define BCmain_QPLL_MODE 0x7fb80

/* MAIN ORB2 ORB1: 0 +0x40 +0x80 32bit */
#define ORBX_COUNTER 0x7facc
/* MAIN ORB2 ORB1: 0 +0x40 +0x80 */
#define ORBX_POLARITY 0x7fae0
/* MAIN ORB2 ORB1: 0 +0x40 +0x80 */
#define ORBX_LENGTH 0x7fad8 
/* MAIN ORB2 ORB1: 0 +0x40 +0x80 */
#define ORBX_COARSE_DELAY 0x7fadc 

#define BC1_MAN_SELECT     0x7fbfc   /* 0: int 1:ext */ 
#define BC2_MAN_SELECT     0x7fbcc   /* 0: int 1:ext */ 
#define BCref_MAN_SELECT     0x7fbac   /* 0: int 1:ext */ 
#define BCmain_MAN_SELECT 0x7fb8c  // 0:int 1:BCref 2:BC2 3:BC1

#define ORBmain_MAN_SELECT 0x7faec
#define ORB1_MAN_SELECT    0x7fb6c   /* 1: int 0:ext */ 
#define ORB2_MAN_SELECT    0x7fb2c   /* 1: int 0:ext */ 

/* I2C registers (special READ access): */
#define BC_DELAY25_BC1             0x7D000
#define BC_DELAY25_BC2             0x7D004
#define BC_DELAY25_BCREF           0x7D008
#define BC_DELAY25_BCMAIN          0x7D00C
#define BC_DELAY25_GCR             0x7D014
#define ORBIN_DELAY25_ORB1         0x7D020
#define ORBIN_DELAY25_ORB2         0x7D024
#define ORBIN_DELAY25_GCR          0x7D034
#define ORBOUT_DELAY25_ORB1        0x7D040
#define ORBOUT_DELAY25_ORB2        0x7D044
#define ORBOUT_DELAY25_ORBMAIN     0x7D048
#define ORBOUT_DELAY25_GCR         0x7D054
/* end of I2C regs */
#define DELAY25_REG                0x7D200

/* reset/enable 3 counters  (3 bits: 012 ->Orbit1,Orbit2,OrbitMain )*/
#define PERIOD_COUNTER_RESET       0x7FA48
#define PERIOD_COUNTER_ENABLE      0x7FA64
#define ORB2_DAC                   0x7FAFC
#define ORB1_DAC                   0x7FB3C
/* FIFOs for 'orbit periods -bits 0..13'. bit14==1: fifo empy */
#define ORB1_PERIOD_FIFO_RD        0x7FB40
#define ORB2_PERIOD_FIFO_RD        0x7FB00
#define ORBmain_PERIOD_FIFO_RD     0x7FAC0

#define ORB1_COARSE_DELAY          0x7FB5C
#define ORB2_COARSE_DELAY          0x7FB1C
#define ORBmain_COARSE_DELAY       0x7FADC
/*REGEND */
//
//RFRX board:
#define ch1_ref 0x12
#define ch2_ref 0x14
#define ch3_ref 0x16
#define ch1_freq_low 0x18
#define ch1_freq_high 0x1a
//#define ch2_freq_low 0x1c
//#define ch2_freq_high 0x1e
//#define ch3_freq_low 0x20
//#define ch3_freq_high 0x22
#define ident_id 0x8
#define card_id 0x24
#define board_id 0x3a

typedef struct Tchan {
  float freq;   //MHz
  w16 ref;  
} Tchan;

//----------------------------------------- corde board (also vme/corde dir):
#define CORDE_RESET 0x24
#define CORDE_ORBMAIN 0x7fbb4
// following part moved from ctplib.h
#define CORDE_DELREG 7
w32 corde_get(int del);  // 1..7. VME is opened/closed with each call!
void corde_set(int del, w32 val); // detto
w32 corde_shift(int del, int shift, int *origval);  //detto
w32 i2cread_delay(w32 delayadd);

int openrfrxs();
void i2cset_delay(w32 delayadd, int halfns);
void micrate(int present);
int micratepresent();

void shiftCommentInDAQ(int halfns, int cordeval, 
  int dbhalfns, int dbcordeval, char *fineshift);

/* FGROUP
Input: maino: 
1 -> BC1/Orbit1
2 -> BC2/Orbit2
3 -> BCref/int BCmain synch. orbit generator
4 -> internal 40.078MHz/int BCmain synch. orbit generator
Operastion:
- compare 2 regs on ttcmi and corde board with $dbctp/clockshift
- set new values in these regs if different
- 'Clock shift' comment to daqlogbook
- change clock 
- 'CLOCK' comment written into DAQ logbook
Note (todo?):
It seems, for
A. BC2/ORB2 or BC1/ORB1 we should set ORB1_MAN_SELECT=ORB2_MAN_SELECT=0
B. BCREF/
C. localBC/
*/
void setbcorbitMain(int maino);
#define REF_MASK 0x0c
/*FGROUP
read QPLL* and TTCrx status bits.
RC: 0xTAB
T: bit 8. 1: TTCrx ok
A: [7..6] BC1 error,locked (i.e. 01 correct)
   [5..4] BC2
B: [3..2] BCref
   [1..0] BCmain
I.e. 0x155 is correct status of all 9 bits
     0x1aa error in both BC, was not locked. NEXT READING is 0x155 !
*/
w32 readstatus();

int readclockshift(char *mem, int maxlen);

#define DLL_stdout 1
#define DLL_daq 2
#define DLL_info 4
void DLL_RESYNC(int msg);
void micrate(int present);
int micratepresent();

