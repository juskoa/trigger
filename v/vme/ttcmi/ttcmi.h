
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
