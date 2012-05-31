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

#define CORDE_DELREG 7
w32 corde_get(int del);  // 1..7. VME is opened/closed with each call!
void corde_set(int del, w32 val); // detto
w32 corde_shift(int del, int shift, int *origval);  //detto
w32 i2cread_delay(w32 delayadd);
void i2cset_delay(w32 delayadd, int halfns);

int shiftCommentInDAQ(int halfns, int cordeval, 
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
#define DLL_stdout 1
#define DLL_daq 2
#define DLL_info 4
void DLL_RESYNC(int msg);

//readtables.c:
FILE *openFile(char *fname, char *rw);
int readdbfile(char *fname, char *mem, int maxlen);
int writedbfile(char *fname, char *mem);

/*readCounters.c:
customer:
0:proxy 1:dims 2:ctp+ctpt+busytool 3:smaq 4:inputs\n");

*/
void readCounters(w32 *mem, int N, int accrual, int customer);
void getCountersBoard(int board, int reladr,w32 *mem, int customer);
/*FGROUP L012 */
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

// pfp.c
void WritePFcommon(w32 INTa,w32 INTb,w32 Delayed_INT);

// moved from ctp.c:
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

