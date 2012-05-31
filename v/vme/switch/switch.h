/*REGSTART32 */
#define CSR 0x0  /* Control/Status register on all 4 boards
w: 
bit0: sw reset= clear all TXCHAN registers, counters disabled, clock phase to 0
bits 6..5:
  00 phase 0
  01       90 degrees
  10       180 degrees
  11       270 deg.

r: bit0: 1 if last sw reset,   0: after read 
      1: 1 if TxMux card   
      2: 1 if RxLow card
      3: 1 if RxHigh card
      4: 1 if LHC clock is active
     6,5: clock phase -see above 
*/
#define TXCFGSTART 0x4   /* 0x4: chan1,...  0x64:chan25. 
Value: 1..50  -input channel selected (1-25 from RX1, 26-50 from RX2)
       0      -this output is disabled
*/
#define RXCNTSTART 0x4   /* from here 25 counters (i.e. 50 on 2 boards) */
/*
bits31..28: w/r counting type: 
  0: counting off
  5: counting pulses of width  < 1 period
  6: counting pulses of width >= 1 period
  7: counting pulses of width == 1 period
  A:                          >= 2 periods
  B:                          == 2 periods
  E:                          >= 3 periods
  F: counting pulses while level is ON

25..0: read only: counter value updated every second
*/
/*REGEND */
#define DUMMY 0xff
#define MAXOUTPUTS 25
#define TXLTUBASE   0x00110000
#define RXLOWBASE   0x00120000
#define RXHIGHBASE  0x00130000
#define TXCTPBASE   0x00140000
/* Channel 42:
>>> hex(0x20000+(4*(42-25)))
'0x20044'
/* Channel 22:
>>> hex(0x10000+(4*(22-0)))
'0x10058'
Reads RXCNTSTART content for given channel.
*/

#define TXLTUSHIFT  0x0
#define RXLOWSHIFT  0x00010000
#define RXHIGHSHIFT 0x00020000
#define TXCTPSHIFT  0x00030000

#define MXINPUT 50   // number of input channels
#define MXOUT 25   // number of output channels
#define MXNAME 40
typedef struct {
  char namepp[MXNAME];
  char namectp[MXNAME];
  int eq;   // 0: no eq
  int input; // 1..50
  int ctpinput;  // 1..24
} Tswitch;

int setchanctpltu(int ctpltu, int input, int output);
/*FGROUP
ctpltu:   1 for ctp   or 0 for ltu. Load config file:
CTP.SWITCH ctp switch 
LTU.SWITCH ltu switch 
Return:
rc: 0: ok, loaded
*/
int loadTable(int ctpltu);
