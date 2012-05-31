/* generated by comp.py */
#include <stdio.h>
#include "vmewrap.h"
#include "lexan.h"
#include "vmeaistd.h"

char BoardName[]="ltu";
char BoardBaseAddress[11]="0x810000";
char BoardSpaceLength[11]="0x800";
char BoardSpaceAddmod[11]="A24";
Tpardesc getsigSSM_parameters[4]={
{"board", 1},
{"bit", 1},
{"frombc", 1},
{"bits", 1}};
Tpardesc finddifSSM_parameters[3]={
{"board", 1},
{"bit", 1},
{"frombc", 1}};
char CTP_Emulator_usagehelp[]="CTP emulator:\n\
- start Sequence list editor\n\
- load sequence list\n\
- set signal error rate, enable/disable errors\n\
  ask for 'on demand' error\n\
- START signal generation selection\n\
- start/break/quit Sequence execution\n\
";

Tpardesc prtfnames_parameters[2]={
{"directory", 3| 0x80000000},
{"suffix", 3| 0x80000000}};
char setCurrents_usagehelp[]="Current settings are kept in shared memory and are accessible\n\
by ECS through ltu_proxy or by 'vmecrate -Trigger control client'.\n\
Current settings will be loaded into LTU/TTC boards:\n\
   - at the start of the run through ECS or \n\
   - by LTUinit/TTCinit buttons (TTCinit actions are \n\
     part of LTUinit actions)\n\
\n\
Save/Load memory buttons allows you to modify these current settings.\n\
During editing, the values different from 'default values' are shown\n\
with yellow background color.\n\
\n\
Load defaults/Save as defaults buttons allow the modification of\n\
settings in database. Use 'Save as defaults' button ONLY WHEN\n\
YOU WANT CURRENT SETTINGS TO BE SAVED AND USED AFTER POWERING UP \n\
YOUR PARTITION.\n\
The settings saved in database are loaded AUTOMATICALY\n\
into Current settings and LTU/TTC boards (TTCvi+TTCrx chip) when\n\
the crate is powered ON.\n\
";

Tpardesc setstdalonemode_parameters[1]={
{"b2", 2}};
char setstdalonemode_usagehelp[]="b2: 3 ext. orbit, stdalone\n\
    1 int. orbit, stdalone\n\
    0 global mode\n\
";

Tpardesc getCounters_parameters[3]={
{"Ncounters", 1},
{"accrual", 1},
{"bakery_customer", 1}};
char getSwitchValue_usagehelp[]="rc: Dial switch value X, (char: 0,1,...,f) i.e. it can be used to form\n\
    '0x1X' input string for setDestination() (see ltutmx.c)\n\
";

Tpardesc TTmode_parameters[1]={
{"mode", 1}};
char TTmode_usagehelp[]="Set (mode=1) or clear(mode=0) 'Timing test mode'.\n\
In Timing test mode, L1 Data signal of the emulator\n\
(i.e. valid only in STDALONE) is continuously toggled.\n\
\n\
This mode is used with another (slave) LTU, connected through\n\
special cable connecting the backplane signals of this LTU\n\
to its CTP inputs connector. \n\
Slave LTU, switched to GLOBAL, can measure\n\
(Configuration->ADC_Scan) delay, which should be kept with\n\
current cabling.\n\
";

Tpardesc setDestination_parameters[1]={
{"destination", 2}};
char setDestination_usagehelp[]="Normally, destination VME address for B-channel data is\n\
TTCvi's 'B Channel Data for B-Go<2>' (0x80x0B8). \n\
Instead of TTCvi board, another LTU can become 'the receiver of\n\
data sent over VME by setting destination to 0x1X \n\
where X is dial of receiving LTU.\n\
Receiving LTU must have its snapshot memory\n\
enabled for 'VME write' -> by starting setSSMVMEW() on this LTU.\n\
\n\
Set destination to 0 for normal mode of operation (with TTCvi).\n\
";

char setSSMVMEW_usagehelp[]="Set Snapshot memory of this LTU to 'VME write' mode. This should\n\
be done before starting 'setDestination()' on another LTU board,\n\
redirecting MASTER vme writes to this board.\n\
";

char readCNTS2SHM_usagehelp[]="rc: 0: ok\n\
    1: error reading counters\n\
";

char CheckBusy_usagehelp[]="Shows: status of all the signals contributing to BUSY output\n\
Allows: to enable/disable BUSY1, BUSY2 inputs and set/clear\n\
        SOFTWARE BUSY\n\
";

char TestLEDS_usagehelp[]="Click to Stop/Start blinking\n\
front panel LEDs\n\
  ";

Tpardesc sendB_parameters[3]={
{"data", 2},
{"words", 1},
{"MAXW", 1}};
char sendB_usagehelp[]="send L1h word ower B channel.\n\
data: 12bits data\n\
words: numbe of words to be send as quickly as possible\n\
MAXW: number of words to be send in one batch (after batch is sent,\n\
      the loop testing TTC_STATUS for empty fifo is started)\n\
      FIFO capacity is 128 words.\n\
";

char getSWLEDS_usagehelp[]="Print 1 line string xxxx\n\
where x is the status (0/1) of software LED word\n\
";

Tpardesc RateLimit_parameters[3]={
{"micsecs", 1},
{"maxtrigs", 1},
{"enadis", 1}};
char RateLimit_usagehelp[]="micsecs: time interval (in micsecs). During this time, triggers are counted\n\
         and compared with maxtrigs limit. Options:\n\
         999999    -read current micsecs/maxtrigs settings \n\
         209100/204700 -max. interval \n\
                   (any higher value results in 209100/204700).\n\
         Interval is set in PERIOD_STEP micsecs slots. Minimal value is 2 slots.\n\
maxtrigs: max. number of triggers allowed in any time interval (set by micsecs)\n\
         Allowed values are: 0..63\n\
enadis:  1: enable, 0: disable 'rate limit' option \n\
RATE_LIMIT word:\n\
0x80000000 1:enable rate limit\n\
0x0000ff00 period in 0.82ms steps (255 till ver. 0xb4)\n\
0x0007ff00 period in 0.1ms steps (2047 - VALID from 19.7.2010, LTU ver 0xb5)\n\
0x0000003f limit:max number of triggers/period\n\
";

char printversion_usagehelp[]="print: \n\
CODE_ADD       always 0x56 for LTU\n\
SERIAL_NUMBER  of the LTU board\n\
VERSION_ADD    VME FPGA version firmware\n\
LTUVERSION_ADD LTU FPGA version firmware\n\
BC_STATUS      BC ststus (should be 0x2 == PLL locked, BC ok)\n\
 ";

Tpardesc GenTrigBusy_parameters[3]={
{"busys", 1},
{"busylength", 1},
{"period", 1}};
char GenTrigBusy_usagehelp[]="busys      -number of busy pulses to be generated\n\
busylength -length of the pulses (us)\n\
period     -time interval between pulses (us)\n\
";

Tpardesc SLMcheck_parameters[1]={
{"filen", 3| 0x80000000}};
char SLMcheck_usagehelp[]="filen: see SLMload (e.g. CFG/ltu/SLM/one.seq) ";

char SLMdump_usagehelp[]="read SLM and write its contents to the file WORK/slmasci\n\
";

Tpardesc SLMes_parameters[1]={
{"milsecs", 1}};
char SLMes_usagehelp[]="SLM memory fetch sequence during emulation test: \n\
  execute following steps in the loop:\n\
- clear SLM_ADD_CLEAR\n\
- SLMstart()\n\
- SLMswstart(1,milsecs);\n\
- SLMwaitemuend();\n\
Don't start it from here, instead use:\n\
1.\n\
ltu/ltu.exe -noboardInit    -> the PID appears\n\
SLMes(0)\n\
2. from independent window, issue 'kill -s USR1 PID' if necessary\n\
";

char SLMbreak_usagehelp[]="break emulation. RC: EMU_STATUS immediately after break\n\
";

Tpardesc setBCDOWN_parameters[1]={
{"bcsd", 2}};
char setBCDOWN_usagehelp[]="set BC scaled down START signal:\n\
0: 25ns\n\
0x3fffff:   ~ 0.1 secs\n\
0x3ffffff:  ~ 1.7 secs\n\
0x3fffffff: ~ 26 secs\n\
";

Tpardesc setrate_parameters[1]={
{"rndrate", 2}};
char setrate_usagehelp[]="Set random rate of automatic START signal generation\n\
";

char getERenadis_usagehelp[]="return: 0 = errors disabled, or 1 ->errors enabled ";

char ERgetselector_usagehelp[]="return selector: bits[6:0] ";

Tpardesc ERseterrrate_parameters[1]={
{"errrate", 2}};
char ERseterrrate_usagehelp[]="demand: bits[2:0] valid values: 1-6\n\
";

char Counters_usagehelp[]="Counters monitor ";

char ScopeAB_usagehelp[]="Signal selection for front panel \n\
A,B outputs\n\
";

char LTUinit_usagehelp[]="Initialise LTU board and TTC path (TTCvi and TTCrx).\n\
";

Tpardesc setOption_parameters[2]={
{"name", 3| 0x80000000},
{"value", 3| 0x80000000}};
char setOption_usagehelp[]="Set parameter name to value\n\
";

Tpardesc setTTCint_parameters[1]={
{"ttcint", 2}};
char setTTCint_usagehelp[]="Set 'L0 mode' :\n\
'L0 over fibre'         if ttcint is 8\n\
'L0 over cable'         if ttcint is 0\n\
\n\
This routine sets the TTC_INTERFACE register. It calculates\n\
2 least sifnificant bits (from BC_DELAY_ADD register). Bits 2..3 are\n\
taken from ttcint parameter.\n\
setTTCint() should be invoked ALWAYS AFTER BC_DELAY_ADD change.\n\
The 4 least significant bits of this register are used for setting:\n\
- 12.5ns delay for Orbit and/or L1\n\
- the polarity of L1 output      \n\
- 'L0 over TTC' mode\n\
\n\
TTC_INTERFACE bits:\n\
0  'Delay Orbit' flag. 0: no delay, 1: 12.5ns delay\n\
1  'Delay L1' flag. 0: no delay, 1: 12.5ns delay\n\
   Both bits depends on BC_DELAY_ADD.\n\
2   L1 polarity.    0: positive (TTCex, default)   1: negative (TTCvi)\n\
3  'L0 over TTC' flag. \n\
   0:  normal mode (only L1 as 1 bit over A-TTC channel)\n\
   1:  'L0overTTC' mode. In this mode: \n\
   - L0 is sent over coaxial cable as in normal mode. In addition,\n\
        it is sent over A-TTC channel as 2 bits '10' in 2 BC clocks\n\
   - L1 is sent as 2 bits '11' over A-TTCchannel\n\
";

char fpgainit_usagehelp[]="set BC_DELAY_ADD (0..31 in ns). setTTCint is activated after BC_DELAY_ADD\n\
written.\n\
";

char TTCinit_usagehelp[]="- initialise TTCvi for operation in STANDALONE mode\n\
- reset TTCrx over fibre (TTCrxready goes OFF for ~6ms)\n\
- set Control register 3 in TTCrx (over fibre), enabling Dout strobe \n\
- set FineDelay1,FineDelay2 and CoarseDelay TTCrx registers \n\
- send FEEreset (after more than 1 second)\n\
- check BUSY signal and print Warnings and/or Errors:\n\
  Warning: BUSY is ON before TTCrx reset\n\
  Error: BUSY not raised during 'TTCrx READY' off\n\
  Warning: BUSY not raised during 'QPLL LOCKED' off (if QPLL clock is used)\n\
  Error: BUSY ON 10milsecs after FEEreset\n\
";

char printltuDefaults_usagehelp[]="Show shared memory content. Shared memory variables can be \n\
modified by Load from memory/Save buttons in Defaults editor.\n\
shmtemp: 1 shm    0: temp\n\
";

Tpardesc ttcPPdelay_parameters[1]={
{"bc", 1}};
char ttcPPdelay_usagehelp[]="Set Prepulse delay directly on TTCvi board. \n\
Warning: delay is set only on TTCviboard (i.e. it is not changed\n\
in memory, which means, TTCinit will set it back to value\n\
stored in shared memory (see Defaults editor)\n\
";

Tpardesc ttcDelays_parameters[3]={
{"Fine1ps", 1},
{"Fine2ps", 1},
{"Coarse", 1}};
char ttcDelays_usagehelp[]="Set delay registers directly in TTCrx, without resetting TTCrx.\n\
Input:\n\
Fine1ps, Fine2ps: required Fine delays on TTCrx chip in ps\n\
The coreespoding value in TTCrx registers in can be set with 104.17 ps step\n\
(e.g. the value corresponding to 4ps or 104 ps is equal: 0xe ) \n\
\n\
Coarse: the required content of TTCrx Coarse register\n\
";

char ttcFEEreset_usagehelp[]="-send 'Front End Electronics reset command' through TTC B-channel.\n\
 The header '0x8' is reserved for this command (e.g. 0x3 is allocated\n\
 for L2accept header).\n\
";

Tpardesc ttcFEEcmd_parameters[1]={
{"Command", 1}};
char ttcFEEcmd_usagehelp[]="-send 'Front End Electronics user defined command' through TTC B-channel.\n\
Command: number 8-15 \n\
     8 already reserved for FEEreset\n\
 9..11 reserved for CTP\n\
12..15 available for subdetectors\n\
\n\
12 data bits are set to 0.\n\
Note: \n\
   0 not used\n\
1..7 used for L1h, L1data, L2ah, L2adata, L2r, RoIh, RoIdata\n\
";

char ReadTemperature_usagehelp[]="returns temperature on the board in centigrades (-100 if error) ";

char measureBusy100ms_usagehelp[]="Return average busy time (busy/L0) during 100milsecs\n\
";

char measureBusy_usagehelp[]="Measure busy between 2 mouse clicks\n\
";

Tpardesc SLMsetstart_parameters[1]={
{"sel", 2}};
Tpardesc SLMswstart_parameters[2]={
{"n", 1},
{"milsecs", 1}};
Tpardesc ERenadis_parameters[1]={
{"enadis", 1}};
Tpardesc ERsetselector_parameters[1]={
{"selector", 2}};
Tpardesc ERdemand_parameters[1]={
{"demand", 2}};
Tpardesc SLMreadasci_parameters[2]={
{"filen", 3| 0x80000000},
{"slmdata", 2| 0x80000000}};
Tpardesc SLMload_parameters[1]={
{"filen", 3| 0x80000000}};
Tpardesc SLMwaitemuend_parameters[1]={
{"micsec", 1}};
Tpardesc setBUSY_parameters[1]={
{"binputs", 2}};
Tpardesc setAB_parameters[2]={
{"A", 2},
{"B", 2}};
Tpardesc SSMsetom_parameters[1]={
{"opmo", 2}};
Tpardesc SSMstartrec_parameters[1]={
{"mode", 2}};
Tpardesc SSMschedule_parameters[1]={
{"whenmode", 2}};
Tpardesc setbcdelay_parameters[2]={
{"delay", 2},
{"ttcint", 1}};
char setbcdelay_usagehelp[]="Set BC_DELAY_ADD \n\
- WITH TTC_INTERFACE correction (ttcint=0)\n\
- WITHOUT TTC_INTERFACE correction (ttcint=1)\n\
  16.12.2008: measured for both in the lab (for hmpid 0x811000),\n\
              but no difference was seen\n\
";

char getbcstatus_usagehelp[]="rc: 2 BC_STATUS low bits: [BC_STATUSpll, BC_STATUSerr] ";

char adcitest_usagehelp[]="Reads adc 300 times as quickly as possible and print it.\n\
";

char readadc_usagehelp[]="Reads ADC checking for busy and timeout.\n\
";

char readadc_s_usagehelp[]="Reads ADC using readadc and checking that two subsequent values are the same.\n\
";

Tpardesc scan_parameters[1]={
{"micseconds", 1}};
char scan_usagehelp[]="Scan of BC delay with going from 0 to 31 \n\
waiting micseconds between measurements.\n\
Do not change TTC_INTERFACE word during scan !\n\
\n\
This subroutine is used with BC phase measurements i.e.\n\
1st line of stdout is relevant.\n\
";

Tpardesc getbcphase_parameters[1]={
{"ltufo", 1}};
char getbcphase_usagehelp[]="Measure BC phase of this FO (ltufo=0) or this LTU (ltufo=1)\n\
FO:\n\
- arrange toggling on FO connector + global mode for this LTU\n\
  before calling this routine\n\
LTU:\n\
- just call this routine\n\
Operation:\n\
- check if global ( the check for toggling to be done)\n\
- store BC_DELAY_ADD, measure\n\
- restore BC_DELAY_ADD\n\
stdout: value of ADC (for BC_DELAY_ADD=0)\n\
        0      may indicate 'not toggling' for FO measurement\n\
        0..128 measured phase\n\
        256    indicates 'not global' for FO measurement (toggling not done yet)\n\
NOTE:\n\
For FO measurement (getbcphase(0)), the change of cable length\n\
leads to change of phase.\n\
";

Tpardesc adctimeconst_parameters[2]={
{"delay0", 2},
{"delay1", 2}};
char adctimeconst_usagehelp[]="Demonstrates time constant parameters of RL element in delay line.\n\
";

char rndtest_usagehelp[]="Generates random delays and measure adc for each of them.\n\
Stdout:\n\
<delay> <adc_value>\n\
...\n\
<0.00> <4.524>\n\
Where:\n\
delay, adc_value: measured pairs as integers 0..31 and 0..127\n\
Last line: avarage wait for PLL locked\n\
";

char ADC_Scan_usagehelp[]="Automatic synchronisation of CTP and LTU signal transitions. \n\
Before this measurement is started,\n\
arrange the following:\n\
- this LTU is in GLOBAL mode\n\
- L1 Data signal (CTP input connector) is continuously toggled\n\
  by BCclock frequency/2\n\
";

void getsigSSM(int board, int bit, int frombc, int bits);
void finddifSSM(int board, int bit, int frombc);
void prtfnames(char *directory, char *suffix);
void waitKB();
void setglobalmode();
int getsgmode();
void setstdalonemode(w32 b2);
void getCounters(int Ncounters, int accrual, int bakery_customer);
void clearCounters();
char getSwitchValue();
void TTmode(int mode);
void setDestination(w32 destination);
void setSSMVMEW();
int readCNTS2SHM();
void TestLEDS();
void sendB(w32 data, int words, int MAXW);
void getSWLEDS();
void RateLimit(int micsecs, int maxtrigs, int enadis);
void printversion();
void GenTrigBusy(int busys, int busylength, int period);
int SLMcheck(char *filen);
int SLMdump();
void SLMes(int milsecs);
int SLMbreak();
void setBCDOWN(w32 bcsd);
void setrate(w32 rndrate);
int getERenadis();
w32 ERgetselector();
void ERseterrrate(w32 errrate);
void LTUinit();
int setOption(char *name, char *value);
void setTTCint(w32 ttcint);
int fpgainit();
int TTCinit();
void printltuDefaults();
int ttcPPdelay(int bc);
int ttcDelays(int Fine1ps, int Fine2ps, int Coarse);
int ttcFEEreset();
int ttcFEEcmd(int Command);
int ReadTemperature();
int getgltuver();
int measureBusy100ms();
int measureBusy();
void SLMsetstart(w32 sel);
int SLMgetstart();
int SLMswstart(int n, int milsecs);
int SLMstart();
void ERenadis(int enadis);
void ERsetselector(w32 selector);
void ERdemand(w32 demand);
int SLMreadasci(char *filen, w32 *slmdata);
int SLMload(char *filen);
int SLMquit();
int SLMwaitemuend(int micsec);
void setBUSY(w32 binputs);
void setAB(w32 A, w32 B);
int SSMsetom(w32 opmo);
void SSMstartrec(w32 mode);
void SSMstoprec();
int SSMdump();
void SSMschedule(w32 whenmode);
int SSMclearac();
void SSMclear();
void setbcdelay(w32 delay, int ttcint);
w32 getbcstatus();
void pllreset();
void adcitest();
int readadc();
int readadc_s();
void scan(int micseconds);
void getbcphase(int ltufo);
void adctimeconst(w32 delay0, w32 delay1);
void rndtest();

int nnames=160;
Tname allnames[MAXNAMES]={
{"ltu", tSYMNAME, NULL, (w32)BoardSpaceLength, 0.0, NULL, (w32)BoardBaseAddress, NULL},
{"CODE_ADD", tVMEADR, NULL, 0, 0.0, NULL, 0x4, NULL},
{"SERIAL_NUMBER", tVMEADR, NULL, 0, 0.0, NULL, 0x8, NULL},
{"VERSION_ADD", tVMEADR, NULL, 0, 0.0, NULL, 0xC, NULL},
{"SOFT_RESET", tVMEADR, NULL, 0, 0.0, NULL, 0x28, NULL},
{"TEMP_START", tVMEADR, NULL, 0, 0.0, NULL, 0x58, NULL},
{"TEMP_STATUS", tVMEADR, NULL, 0, 0.0, NULL, 0x5c, NULL},
{"TEMP_READ", tVMEADR, NULL, 0, 0.0, NULL, 0x60, NULL},
{"MASTER_MODE", tVMEADR, NULL, 0, 0.0, NULL, 0x64, NULL},
{"LTUVERSION_ADD", tVMEADR, NULL, 0, 0.0, NULL, 0x80, NULL},
{"TEST_ADD", tVMEADR, NULL, 0, 0.0, NULL, 0xc0, NULL},
{"BC_STATUS", tVMEADR, NULL, 0, 0.0, NULL, 0xc4, NULL},
{"ADC_SELECT", tVMEADR, NULL, 0, 0.0, NULL, 0x4c0, NULL},
{"BC_DELAY_ADD", tVMEADR, NULL, 0, 0.0, NULL, 0x4c8, NULL},
{"ADC_START", tVMEADR, NULL, 0, 0.0, NULL, 0xcc, NULL},
{"ADC_DATA", tVMEADR, NULL, 0, 0.0, NULL, 0xd0, NULL},
{"ERROR_ENABLE", tVMEADR, NULL, 0, 0.0, NULL, 0xd4, NULL},
{"ERROR_STATUS", tVMEADR, NULL, 0, 0.0, NULL, 0xd8, NULL},
{"ERROR_SELECTOR", tVMEADR, NULL, 0, 0.0, NULL, 0xdc, NULL},
{"ERROR_DEMAND", tVMEADR, NULL, 0, 0.0, NULL, 0xe0, NULL},
{"ERROR_RATE", tVMEADR, NULL, 0, 0.0, NULL, 0x4e4, NULL},
{"ORBIT_CLEAR", tVMEADR, NULL, 0, 0.0, NULL, 0xe8, NULL},
{"EMULATION_START", tVMEADR, NULL, 0, 0.0, NULL, 0xec, NULL},
{"QUIT_SET", tVMEADR, NULL, 0, 0.0, NULL, 0xf0, NULL},
{"BREAK_SET", tVMEADR, NULL, 0, 0.0, NULL, 0xf4, NULL},
{"LAST_BC", tVMEADR, NULL, 0, 0.0, NULL, 0x4f8, NULL},
{"PREPULSE_BC", tVMEADR, NULL, 0, 0.0, NULL, 0x4fc, NULL},
{"CALIBRATION_BC", tVMEADR, NULL, 0, 0.0, NULL, 0x500, NULL},
{"GAP_BC", tVMEADR, NULL, 0, 0.0, NULL, 0x504, NULL},
{"ORBIT_BC", tVMEADR, NULL, 0, 0.0, NULL, 0x508, NULL},
{"L1_DELAY", tVMEADR, NULL, 0, 0.0, NULL, 0x50c, NULL},
{"L2_DELAY", tVMEADR, NULL, 0, 0.0, NULL, 0x510, NULL},
{"TRG_TOGGLE", tVMEADR, NULL, 0, 0.0, NULL, 0x5c4, NULL},
{"ORBIT_TIME", tVMEADR, NULL, 0, 0.0, NULL, 0x5c0, NULL},
{"PP_TIME", tVMEADR, NULL, 0, 0.0, NULL, 0x5c4, NULL},
{"TTC_DATA", tVMEADR, NULL, 0, 0.0, NULL, 0x5c8, NULL},
{"RATE_LIMIT", tVMEADR, NULL, 0, 0.0, NULL, 0x5cc, NULL},
{"SLM_ADD_CLEAR", tVMEADR, NULL, 0, 0.0, NULL, 0x114, NULL},
{"SLM_DATA", tVMEADR, NULL, 0, 0.0, NULL, 0x118, NULL},
{"EMU_STATUS", tVMEADR, NULL, 0, 0.0, NULL, 0x11c, NULL},
{"START_SET", tVMEADR, NULL, 0, 0.0, NULL, 0x520, NULL},
{"SOFT_TRIGGER", tVMEADR, NULL, 0, 0.0, NULL, 0x124, NULL},
{"RANDOM_NUMBER", tVMEADR, NULL, 0, 0.0, NULL, 0x528, NULL},
{"COUNT_PERIOD", tVMEADR, NULL, 0, 0.0, NULL, 0x52c, NULL},
{"TTC_INTERFACE", tVMEADR, NULL, 0, 0.0, NULL, 0x530, NULL},
{"STDALONE_MODE", tVMEADR, NULL, 0, 0.0, NULL, 0x534, NULL},
{"BUSY_ENABLE", tVMEADR, NULL, 0, 0.0, NULL, 0x138, NULL},
{"BUSY_STATUS", tVMEADR, NULL, 0, 0.0, NULL, 0x13c, NULL},
{"SW_BUSY", tVMEADR, NULL, 0, 0.0, NULL, 0x140, NULL},
{"L1_FORMAT", tVMEADR, NULL, 0, 0.0, NULL, 0x544, NULL},
{"L1MAX_CLEAR", tVMEADR, NULL, 0, 0.0, NULL, 0x148, NULL},
{"L2MAX_CLEAR", tVMEADR, NULL, 0, 0.0, NULL, 0x14c, NULL},
{"SCOPE_SELECT", tVMEADR, NULL, 0, 0.0, NULL, 0x550, NULL},
{"BACKPLANE_EN", tVMEADR, NULL, 0, 0.0, NULL, 0x554, NULL},
{"MINIMAX_SELECT", tVMEADR, NULL, 0, 0.0, NULL, 0x570, NULL},
{"MINIMAX_LIMIT", tVMEADR, NULL, 0, 0.0, NULL, 0x578, NULL},
{"FIFO_MAX", tVMEADR, NULL, 0, 0.0, NULL, 0x158, NULL},
{"SOFT_LED", tVMEADR, NULL, 0, 0.0, NULL, 0x15c, NULL},
{"BUSYMAX_DATA", tVMEADR, NULL, 0, 0.0, NULL, 0x168, NULL},
{"BUSYMINI_DATA", tVMEADR, NULL, 0, 0.0, NULL, 0x16c, NULL},
{"MINIMAX_CLEAR", tVMEADR, NULL, 0, 0.0, NULL, 0x174, NULL},
{"PIPELINE_CLEAR", tVMEADR, NULL, 0, 0.0, NULL, 0x198, NULL},
{"SSMcommand", tVMEADR, NULL, 0, 0.0, NULL, 0x19c, NULL},
{"SSMstart", tVMEADR, NULL, 0, 0.0, NULL, 0x1a0, NULL},
{"SSMstop", tVMEADR, NULL, 0, 0.0, NULL, 0x1a4, NULL},
{"SSMaddress", tVMEADR, NULL, 0, 0.0, NULL, 0x1a8, NULL},
{"SSMdata", tVMEADR, NULL, 0, 0.0, NULL, 0x1ac, NULL},
{"SSMstatus", tVMEADR, NULL, 0, 0.0, NULL, 0x1b0, NULL},
{"TIMING_TEST", tVMEADR, NULL, 0, 0.0, NULL, 0x1b8, NULL},
{"PLL_RESET", tVMEADR, NULL, 0, 0.0, NULL, 0x1bc, NULL},
{"TIMING_TEST", tVMEADR, NULL, 0, 0.0, NULL, 0x1b8, NULL},
{"TTC_STATUS", tVMEADR, NULL, 0, 0.0, NULL, 0x1c8, NULL},
{"COPYCOUNT", tVMEADR, NULL, 0, 0.0, NULL, 0x1d4, NULL},
{"COPYBUSY", tVMEADR, NULL, 0, 0.0, NULL, 0x1d8, NULL},
{"COPYCLEARADD", tVMEADR, NULL, 0, 0.0, NULL, 0x1dc, NULL},
{"COPYREAD", tVMEADR, NULL, 0, 0.0, NULL, 0x1e0, NULL},
{"CLEARCOUNTER", tVMEADR, NULL, 0, 0.0, NULL, 0x5ac, NULL},
{"getsigSSM", tFUN+0x400, (funcall)getsigSSM, 0xdead, 0.0, getsigSSM_parameters, 4, NULL},
{"finddifSSM", tFUN+0x400, (funcall)finddifSSM, 0xdead, 0.0, finddifSSM_parameters, 3, NULL},
{"CTP_Emulator", tFUN, NULL, 0xdead, 0.0, NULL, 0, CTP_Emulator_usagehelp},
{"prtfnames", tFUN+0x400, (funcall)prtfnames, 0xdead, 0.0, prtfnames_parameters, 2, NULL},
{"waitKB", tFUN+0x400, (funcall)waitKB, 0xdead, 0.0, NULL, 0, NULL},
{"setglobalmode", tFUN+0x400, (funcall)setglobalmode, 0xdead, 0.0, NULL, 0, NULL},
{"setCurrents", tFUN, NULL, 0xdead, 0.0, NULL, 0, setCurrents_usagehelp},
{"CheckRateLimit", tFUN, NULL, 0xdead, 0.0, NULL, 0, NULL},
{"getsgmode", tFUN+0x200, (funcall)getsgmode, 0xdead, 0.0, NULL, 0, NULL},
{"setstdalonemode", tFUN+0x400, (funcall)setstdalonemode, 0xdead, 0.0, setstdalonemode_parameters, 1, setstdalonemode_usagehelp},
{"getCounters", tFUN+0x400, (funcall)getCounters, 0xdead, 0.0, getCounters_parameters, 3, NULL},
{"clearCounters", tFUN+0x400, (funcall)clearCounters, 0xdead, 0.0, NULL, 0, NULL},
{"getSwitchValue", tFUN+0x500, (funcall)getSwitchValue, 0xdead, 0.0, NULL, 0, getSwitchValue_usagehelp},
{"TTmode", tFUN+0x400, (funcall)TTmode, 0xdead, 0.0, TTmode_parameters, 1, TTmode_usagehelp},
{"setDestination", tFUN+0x400, (funcall)setDestination, 0xdead, 0.0, setDestination_parameters, 1, setDestination_usagehelp},
{"setSSMVMEW", tFUN+0x400, (funcall)setSSMVMEW, 0xdead, 0.0, NULL, 0, setSSMVMEW_usagehelp},
{"readCNTS2SHM", tFUN+0x200, (funcall)readCNTS2SHM, 0xdead, 0.0, NULL, 0, readCNTS2SHM_usagehelp},
{"CheckBusy", tFUN, NULL, 0xdead, 0.0, NULL, 0, CheckBusy_usagehelp},
{"TestLEDS", tFUN+0x400, (funcall)TestLEDS, 0xdead, 0.0, NULL, 0, TestLEDS_usagehelp},
{"sendB", tFUN+0x400, (funcall)sendB, 0xdead, 0.0, sendB_parameters, 3, sendB_usagehelp},
{"getSWLEDS", tFUN+0x400, (funcall)getSWLEDS, 0xdead, 0.0, NULL, 0, getSWLEDS_usagehelp},
{"RateLimit", tFUN+0x400, (funcall)RateLimit, 0xdead, 0.0, RateLimit_parameters, 3, RateLimit_usagehelp},
{"printversion", tFUN+0x400, (funcall)printversion, 0xdead, 0.0, NULL, 0, printversion_usagehelp},
{"GenTrigBusy", tFUN+0x400, (funcall)GenTrigBusy, 0xdead, 0.0, GenTrigBusy_parameters, 3, GenTrigBusy_usagehelp},
{"SLMcheck", tFUN+0x200, (funcall)SLMcheck, 0xdead, 0.0, SLMcheck_parameters, 1, SLMcheck_usagehelp},
{"SLMdump", tFUN+0x200, (funcall)SLMdump, 0xdead, 0.0, NULL, 0, SLMdump_usagehelp},
{"SLMes", tFUN+0x400, (funcall)SLMes, 0xdead, 0.0, SLMes_parameters, 1, SLMes_usagehelp},
{"SLMbreak", tFUN+0x200, (funcall)SLMbreak, 0xdead, 0.0, NULL, 0, SLMbreak_usagehelp},
{"setBCDOWN", tFUN+0x400, (funcall)setBCDOWN, 0xdead, 0.0, setBCDOWN_parameters, 1, setBCDOWN_usagehelp},
{"setrate", tFUN+0x400, (funcall)setrate, 0xdead, 0.0, setrate_parameters, 1, setrate_usagehelp},
{"getERenadis", tFUN+0x200, (funcall)getERenadis, 0xdead, 0.0, NULL, 0, getERenadis_usagehelp},
{"ERgetselector", tFUN+0x100, (funcall)ERgetselector, 0xdead, 0.0, NULL, 0, ERgetselector_usagehelp},
{"ERseterrrate", tFUN+0x400, (funcall)ERseterrrate, 0xdead, 0.0, ERseterrrate_parameters, 1, ERseterrrate_usagehelp},
{"Counters", tFUN, NULL, 0xdead, 0.0, NULL, 0, Counters_usagehelp},
{"Snapshot_memory", tFUN, NULL, 0xdead, 0.0, NULL, 0, NULL},
{"ScopeAB", tFUN, NULL, 0xdead, 0.0, NULL, 0, ScopeAB_usagehelp},
{"LTUinit", tFUN+0x400, (funcall)LTUinit, 0xdead, 0.0, NULL, 0, LTUinit_usagehelp},
{"setOption", tFUN+0x200, (funcall)setOption, 0xdead, 0.0, setOption_parameters, 2, setOption_usagehelp},
{"setTTCint", tFUN+0x400, (funcall)setTTCint, 0xdead, 0.0, setTTCint_parameters, 1, setTTCint_usagehelp},
{"fpgainit", tFUN+0x200, (funcall)fpgainit, 0xdead, 0.0, NULL, 0, fpgainit_usagehelp},
{"TTCinit", tFUN+0x200, (funcall)TTCinit, 0xdead, 0.0, NULL, 0, TTCinit_usagehelp},
{"printltuDefaults", tFUN+0x400, (funcall)printltuDefaults, 0xdead, 0.0, NULL, 0, printltuDefaults_usagehelp},
{"ttcPPdelay", tFUN+0x200, (funcall)ttcPPdelay, 0xdead, 0.0, ttcPPdelay_parameters, 1, ttcPPdelay_usagehelp},
{"ttcDelays", tFUN+0x200, (funcall)ttcDelays, 0xdead, 0.0, ttcDelays_parameters, 3, ttcDelays_usagehelp},
{"ttcFEEreset", tFUN+0x200, (funcall)ttcFEEreset, 0xdead, 0.0, NULL, 0, ttcFEEreset_usagehelp},
{"ttcFEEcmd", tFUN+0x200, (funcall)ttcFEEcmd, 0xdead, 0.0, ttcFEEcmd_parameters, 1, ttcFEEcmd_usagehelp},
{"setGLOBAL", tFUN, NULL, 0xdead, 0.0, NULL, 0, NULL},
{"setSTDALONE", tFUN, NULL, 0xdead, 0.0, NULL, 0, NULL},
{"ReadTemperature", tFUN+0x200, (funcall)ReadTemperature, 0xdead, 0.0, NULL, 0, ReadTemperature_usagehelp},
{"getgltuver", tFUN+0x200, (funcall)getgltuver, 0xdead, 0.0, NULL, 0, NULL},
{"measureBusy100ms", tFUN+0x200, (funcall)measureBusy100ms, 0xdead, 0.0, NULL, 0, measureBusy100ms_usagehelp},
{"measureBusy", tFUN+0x200, (funcall)measureBusy, 0xdead, 0.0, NULL, 0, measureBusy_usagehelp},
{"SLMsetstart", tFUN+0x400, (funcall)SLMsetstart, 0xdead, 0.0, SLMsetstart_parameters, 1, NULL},
{"SLMgetstart", tFUN+0x200, (funcall)SLMgetstart, 0xdead, 0.0, NULL, 0, NULL},
{"SLMswstart", tFUN+0x200, (funcall)SLMswstart, 0xdead, 0.0, SLMswstart_parameters, 2, NULL},
{"SLMstart", tFUN+0x200, (funcall)SLMstart, 0xdead, 0.0, NULL, 0, NULL},
{"ERenadis", tFUN+0x400, (funcall)ERenadis, 0xdead, 0.0, ERenadis_parameters, 1, NULL},
{"ERsetselector", tFUN+0x400, (funcall)ERsetselector, 0xdead, 0.0, ERsetselector_parameters, 1, NULL},
{"ERdemand", tFUN+0x400, (funcall)ERdemand, 0xdead, 0.0, ERdemand_parameters, 1, NULL},
{"SLMreadasci", tFUN+0x200, (funcall)SLMreadasci, 0xdead, 0.0, SLMreadasci_parameters, 2, NULL},
{"SLMload", tFUN+0x200, (funcall)SLMload, 0xdead, 0.0, SLMload_parameters, 1, NULL},
{"SLMquit", tFUN+0x200, (funcall)SLMquit, 0xdead, 0.0, NULL, 0, NULL},
{"SLMwaitemuend", tFUN+0x200, (funcall)SLMwaitemuend, 0xdead, 0.0, SLMwaitemuend_parameters, 1, NULL},
{"setBUSY", tFUN+0x400, (funcall)setBUSY, 0xdead, 0.0, setBUSY_parameters, 1, NULL},
{"setAB", tFUN+0x400, (funcall)setAB, 0xdead, 0.0, setAB_parameters, 2, NULL},
{"SSMsetom", tFUN+0x200, (funcall)SSMsetom, 0xdead, 0.0, SSMsetom_parameters, 1, NULL},
{"SSMstartrec", tFUN+0x400, (funcall)SSMstartrec, 0xdead, 0.0, SSMstartrec_parameters, 1, NULL},
{"SSMstoprec", tFUN+0x400, (funcall)SSMstoprec, 0xdead, 0.0, NULL, 0, NULL},
{"SSMdump", tFUN+0x200, (funcall)SSMdump, 0xdead, 0.0, NULL, 0, NULL},
{"SSMschedule", tFUN+0x400, (funcall)SSMschedule, 0xdead, 0.0, SSMschedule_parameters, 1, NULL},
{"SSMclearac", tFUN+0x200, (funcall)SSMclearac, 0xdead, 0.0, NULL, 0, NULL},
{"SSMclear", tFUN+0x400, (funcall)SSMclear, 0xdead, 0.0, NULL, 0, NULL},
{"setbcdelay", tFUN+0x400, (funcall)setbcdelay, 0xdead, 0.0, setbcdelay_parameters, 2, setbcdelay_usagehelp},
{"getbcstatus", tFUN+0x100, (funcall)getbcstatus, 0xdead, 0.0, NULL, 0, getbcstatus_usagehelp},
{"pllreset", tFUN+0x400, (funcall)pllreset, 0xdead, 0.0, NULL, 0, NULL},
{"adcitest", tFUN+0x400, (funcall)adcitest, 0xdead, 0.0, NULL, 0, adcitest_usagehelp},
{"readadc", tFUN+0x200, (funcall)readadc, 0xdead, 0.0, NULL, 0, readadc_usagehelp},
{"readadc_s", tFUN+0x200, (funcall)readadc_s, 0xdead, 0.0, NULL, 0, readadc_s_usagehelp},
{"scan", tFUN+0x400, (funcall)scan, 0xdead, 0.0, scan_parameters, 1, scan_usagehelp},
{"getbcphase", tFUN+0x400, (funcall)getbcphase, 0xdead, 0.0, getbcphase_parameters, 1, getbcphase_usagehelp},
{"adctimeconst", tFUN+0x400, (funcall)adctimeconst, 0xdead, 0.0, adctimeconst_parameters, 2, adctimeconst_usagehelp},
{"rndtest", tFUN+0x400, (funcall)rndtest, 0xdead, 0.0, NULL, 0, rndtest_usagehelp},
{"ADC_Scan", tFUN, NULL, 0xdead, 0.0, NULL, 0, ADC_Scan_usagehelp}};
