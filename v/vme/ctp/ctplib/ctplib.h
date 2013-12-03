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

/*FGROUP DebCon
Generate n software trigger sequences
Operation:
-check if all detectors are in global (ctpproxy shared memory)
-setswtrig()
-while(n) startswtrig()

Parameters: see setswtrig()
customer: number 0..1
0: SOD/EOD/SYNC generation initiated from ctp_proxy
1: calibration triggers from gcalib task
2: dimservices.c (usually not used) + ctp.exe (expert sw) + ctpt.exe

RC: number of L2a successfully generated, or
    12345678: cal. triggers stopped becasue det. is not in global run
*/
int  GenSwtrg_op(int ntriggers,char trigtype, int roc, w32 BC,w32 detectors);
