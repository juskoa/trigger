/* generated by comp.py */
#include <stdio.h>
#include "vmewrap.h"
#include "lexan.h"
#include "vmeaistd.h"

char BoardName[]="ttcvi";
char BoardBaseAddress[11]="0x800000";
char BoardSpaceLength[11]="0xCB";
char BoardSpaceAddmod[11]="A24";
char pcsr1_usagehelp[]="print CSR1 ";

Tpardesc setcsr1_parameters[5]={
{"EvOr", 2},
{"Freq", 2},
{"L1AFreset", 2},
{"ExtOrbit0", 2},
{"TrigSrc", 2}};
char getEOcnt_usagehelp[]="get event/orbit counter content\n\
";

Tpardesc settrig_parameters[2]={
{"trigSource", 1},
{"frequency", 1}};
char settrig_usagehelp[]="settrig(int trigSource, int frequency): set trigger Source\n\
and trigger frequency for random trigger.\n\
trigSource: \n\
0=L1A0, 1=L1A1, 2=L1A2, 3=L1A3,\n\
4=VME, 5=random trigger\n\
frequency (valid for random trigger):\n\
0=1Hz, 1=100Hz, 2=1kHz, 3=5kHz, 4=10kHz, \n\
5=25kHz, 6=50kHz, 7=100kHz\n\
";

char InpSelTiming_usagehelp[]="Examine/set CSR1 register (Input selection and timing)\n\
";

Tpardesc setcount_parameters[1]={
{"orbits", 1}};
char setcount_usagehelp[]="Set counting (events/orbits) and clear event/orbit counter:\n\
setcount(0) - count events\n\
setcount(1) - count orbits\n\
";

Tpardesc sendL1_parameters[1]={
{"cnt", 1}};
char sendL1_usagehelp[]="send 1 or more L1 triggers ";

Tpardesc sendL1M_parameters[5]={
{"wo1", 2},
{"wo2", 2},
{"wo3", 2},
{"wo4", 2},
{"wo5", 2}};
char sendL1M_usagehelp[]="send L1 trigger and L1 message\n\
wo1-wo5: 5 12-bits word comprising the L1message ";

Tpardesc sendL2M_parameters[8]={
{"wo1", 2},
{"wo2", 2},
{"wo3", 2},
{"wo4", 2},
{"wo5", 2},
{"wo6", 2},
{"wo7", 2},
{"wo8", 2}};
char sendL2M_usagehelp[]="send L2 message. wo1-wo8: 8 12-bits words comprising the L2 message ";

Tpardesc sendtrigger_parameters[1]={
{"cnt", 1}};
char sendtrigger_usagehelp[]="sendtrigger(int cnt) \n\
  send (L1, last L1 message, last L2 message) in loop (cnt times)\n\
  sendL1M(), sendL2M() should be called before \n\
   to prepare L1,L2 messages\n\
cnt: 0-> endless loop\n\
    >0-> send cnt L1,L2 sequences \n\
";

char sendL1L2_usagehelp[]="send: L1 & L1message or\n\
      L2message or\n\
      L1, L1message, L2message\n\
";

Tpardesc sendBroadcast_parameters[1]={
{"data", 2}};
char sendBroadcast_usagehelp[]="Send broadcast command (short async cycles)\n\
data:\n\
1 bunch counter reset\n\
2 event counter reset\n\
3 both counters reset\n\
";

Tpardesc sendcontrol_parameters[4]={
{"E", 1},
{"subaddr", 2},
{"data", 2},
{"cycles", 1}};
char sendcontrol_usagehelp[]="sendcontrol(int E, w8 subaddr, w8 data, cycles)\n\
send broadcast data\n\
E:\n\
0 access TTCrx internal registers\n\
1 external access\n\
subaddr,data: data to be sent\n\
cycles: # of repetitions\n\
";

Tpardesc senddata_parameters[2]={
{"cycles", 1},
{"data", 2}};
char senddata_usagehelp[]="write 'data (16bits)' through Bgo2 'cycles' times \n\
";

Tpardesc send1w_parameters[4]={
{"n", 1},
{"dat", 2},
{"micsecs", 1},
{"fifo", 1}};
char send1w_usagehelp[]="Used for test of TTCvi -Dout strob doubled on TTCrx\n\
(instad of 1 word in B channel we have seen 2 words for fifos 0 and 2\n\
26.10.2006. After replugging the TTCvi board in its VME slot this fault\n\
disappeared). This happened with TTCvi:\n\
CERNID:80030 Serial number:5054543 Board revision:20020415\n\
(used as HMPID in DAQ ref. setup).\n\
All fifos should be set as follows:\n\
vmew16(CSR2,0xff00);\n\
bgoinit(fifoN,    0, 0, 0x03, 0xffffffff); \n\
\n\
n       -# of loops\n\
dat     -data sent in 1 loop\n\
micsecs -wait micsecs between vmew32(...\n\
fifo    - 0,1,2,3\n\
";

char printsernumber_usagehelp[]="Print serial number of TTCvi board\n\
";

char reset_usagehelp[]="the TTCvi board software reset\n\
";

Tpardesc ppdelay_parameters[1]={
{"bcdelay", 2}};
char ppdelay_usagehelp[]="bcdelay: delay in bunch crossings for B-Go1 channel (prepulse)\n\
";

Tpardesc bgoinit_parameters[5]={
{"bgc", 1},
{"del", 2},
{"dur", 2},
{"mode", 2},
{"data", 2}};
char bgoinit_usagehelp[]="This routine should be removed and the one in ltulib/ltuinit.c to be used!\n\
bgc      -Bgo channel (0..3)\n\
del, dur -delay, duration for this channel\n\
mode     - mode of the channel\n\
data     -data to be written into B Channel Data FIFO\n\
Modified registers:\n\
chan delayreg durreg modereg B-go(data)\n\
0    0x92     0x94   0x90    0xb0      Orbit (Bgo driven)\n\
1    0x9a     0x9c   0x98    0xb4      PP (Bgo driven)\n\
2    0xa2     0xa4   0xa0    0xb8      L1/L2 data (FIFO driven)\n\
";

Tpardesc bgodump_parameters[1]={
{"bgc", 1}};
char ttcinit_usagehelp[]="This routine, when called from ltu.exe, is to be replaced be \n\
call to ttcInit(0) -see ltulib/ltuinit.c\n\
Initialize TTCvi, TTCrx. \n\
Has to be called always after power up of TTCvi or TTCrx.\n\
It is called automatically after the start of this software (ttcvi.exe)\n\
";

char ttcdump_usagehelp[]="reads current settings on TTCvi board\n\
";

void pcsr1();
void setcsr1(w16 EvOr, w16 Freq, w16 L1AFreset, w16 ExtOrbit0, w16 TrigSrc);
int getEOcnt();
void settrig(int trigSource, int frequency);
void setcount(int orbits);
void sendL1(int cnt);
void sendL1M(w32 wo1, w32 wo2, w32 wo3, w32 wo4, w32 wo5);
void sendL2M(w32 wo1, w32 wo2, w32 wo3, w32 wo4, w32 wo5, w32 wo6, w32 wo7, w32 wo8);
void sendtrigger(int cnt);
void sendBroadcast(w8 data);
void sendcontrol(int E, w8 subaddr, w8 data, int cycles);
void senddata(int cycles, w16 data);
void send1w(int n, w32 dat, int micsecs, int fifo);
void printsernumber();
void reset();
void ppdelay(w16 bcdelay);
void bgoinit(int bgc, w16 del, w16 dur, w16 mode, w32 data);
void bgodump(int bgc);
void ttcinit();
void ttcdump();

int nnames=49;
Tname allnames[MAXNAMES]={
{"ttcvi", tSYMNAME, NULL, (w32)BoardSpaceLength, 0.0, NULL, (w32)BoardBaseAddress, NULL},
{"CSR1", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0x80, NULL},
{"CSR2", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0x82, NULL},
{"BOARDreset", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0x84, NULL},
{"L1Agen", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0x86, NULL},
{"EOCreset", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0x8c, NULL},
{"BGo0mode", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0x90, NULL},
{"BGo1mode", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0x98, NULL},
{"BGo2mode", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0xa0, NULL},
{"BGo3mode", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0xa8, NULL},
{"IDel0", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0x92, NULL},
{"IDur0", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0x94, NULL},
{"IDel1", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0x9a, NULL},
{"IDur1", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0x9c, NULL},
{"IDel2", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0xa2, NULL},
{"IDur2", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0xa4, NULL},
{"IDel3", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0xaa, NULL},
{"IDur3", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0xac, NULL},
{"EOcnt1", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0x88, NULL},
{"EOcnt2", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0x8a, NULL},
{"BCLFACttcrxadr", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0xC0, NULL},
{"BCLFACdata", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0xC2, NULL},
{"BCSFACdata", tVMEADR|0x04000000, NULL, 0, 0.0, NULL, 0xC4, NULL},
{"BCDBG0", tVMEADR, NULL, 0, 0.0, NULL, 0xb0, NULL},
{"BCDBG1", tVMEADR, NULL, 0, 0.0, NULL, 0xb4, NULL},
{"BCDBG2", tVMEADR, NULL, 0, 0.0, NULL, 0xb8, NULL},
{"BCDBG3", tVMEADR, NULL, 0, 0.0, NULL, 0xbc, NULL},
{"pcsr1", tFUN+0x400, (w32 (*)())pcsr1, 0xdead, 0.0, NULL, 0, pcsr1_usagehelp},
{"setcsr1", tFUN+0x400, (w32 (*)())setcsr1, 0xdead, 0.0, setcsr1_parameters, 5, NULL},
{"getEOcnt", tFUN+0x200, (w32 (*)())getEOcnt, 0xdead, 0.0, NULL, 0, getEOcnt_usagehelp},
{"settrig", tFUN+0x400, (w32 (*)())settrig, 0xdead, 0.0, settrig_parameters, 2, settrig_usagehelp},
{"InpSelTiming", tFUN, NULL, 0xdead, 0.0, NULL, 0, InpSelTiming_usagehelp},
{"setcount", tFUN+0x400, (w32 (*)())setcount, 0xdead, 0.0, setcount_parameters, 1, setcount_usagehelp},
{"sendL1", tFUN+0x400, (w32 (*)())sendL1, 0xdead, 0.0, sendL1_parameters, 1, sendL1_usagehelp},
{"sendL1M", tFUN+0x400, (w32 (*)())sendL1M, 0xdead, 0.0, sendL1M_parameters, 5, sendL1M_usagehelp},
{"sendL2M", tFUN+0x400, (w32 (*)())sendL2M, 0xdead, 0.0, sendL2M_parameters, 8, sendL2M_usagehelp},
{"sendtrigger", tFUN+0x400, (w32 (*)())sendtrigger, 0xdead, 0.0, sendtrigger_parameters, 1, sendtrigger_usagehelp},
{"sendL1L2", tFUN, NULL, 0xdead, 0.0, NULL, 0, sendL1L2_usagehelp},
{"sendBroadcast", tFUN+0x400, (w32 (*)())sendBroadcast, 0xdead, 0.0, sendBroadcast_parameters, 1, sendBroadcast_usagehelp},
{"sendcontrol", tFUN+0x400, (w32 (*)())sendcontrol, 0xdead, 0.0, sendcontrol_parameters, 4, sendcontrol_usagehelp},
{"senddata", tFUN+0x400, (w32 (*)())senddata, 0xdead, 0.0, senddata_parameters, 2, senddata_usagehelp},
{"send1w", tFUN+0x400, (w32 (*)())send1w, 0xdead, 0.0, send1w_parameters, 4, send1w_usagehelp},
{"printsernumber", tFUN+0x400, (w32 (*)())printsernumber, 0xdead, 0.0, NULL, 0, printsernumber_usagehelp},
{"reset", tFUN+0x400, (w32 (*)())reset, 0xdead, 0.0, NULL, 0, reset_usagehelp},
{"ppdelay", tFUN+0x400, (w32 (*)())ppdelay, 0xdead, 0.0, ppdelay_parameters, 1, ppdelay_usagehelp},
{"bgoinit", tFUN+0x400, (w32 (*)())bgoinit, 0xdead, 0.0, bgoinit_parameters, 5, bgoinit_usagehelp},
{"bgodump", tFUN+0x400, (w32 (*)())bgodump, 0xdead, 0.0, bgodump_parameters, 1, NULL},
{"ttcinit", tFUN+0x400, (w32 (*)())ttcinit, 0xdead, 0.0, NULL, 0, ttcinit_usagehelp},
{"ttcdump", tFUN+0x400, (w32 (*)())ttcdump, 0xdead, 0.0, NULL, 0, ttcdump_usagehelp}};
