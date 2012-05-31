/*BOARD ttcvi 0x800000 0xCB */
/* BOARD ttcvi "VXI0::262" 0xCB */
/* should be 0x80X000 where X is the LTU dial setting */
/* 13.3.2006: ttcinit() -usleeps added (fineDelay2 did not set
correctly without usleeps)
27.6.2006 bgoinit(0 and bgoinit(1 changed (inhibit duration)
7.5.2007:
bgoinit() -moved to ltulib
ttcinit() -obsolete -see ttcInit() in  ltulib/ltuinit.c 
*/
/* HIDDEN primitives */
#include <stdio.h>
#include <unistd.h>   //usleep
#include "vmewrap.h"
#include "ttcvi.h"

extern int quit;
void boardInit();

/*FGROUP primitives
print CSR1 */
void pcsr1() {
w16 csr1, csr2;
csr1= vmer16(CSR1);
csr2= vmer16(CSR2);
/* evorb,frequ,bcdel,vmetp,frese,fempt,ffull,exorb,trsrc */
/*  w     w                 w                 w    w    */
printf("%d,%d,%d,%d,%d,%d,%d,%d,%d\n", 
  (csr1 & 0x8000)>>15, (csr1 & 0x7000)>>12,
  (csr1 & 0x0f00)>>8, (csr1 & 0x0080)>>7, (csr1 & 0x0040)>>6,
  (csr1 & 0x0020)>>5, (csr1 & 0x0010)>>4, (csr1 & 0x0008)>>3,
  (csr1 & 0x0007));
}
/*FGROUP primitives
*/
void setcsr1(w16 EvOr, w16 Freq, w16 L1AFreset, w16 ExtOrbit0, w16 TrigSrc) {
w16 csr=0;
csr= (EvOr<<15) | ((Freq<<12)&0x7000) |
     ((L1AFreset<<6)&0x0040) |
     ((ExtOrbit0<<3)&0x0008) |
     (TrigSrc&0x0007);
vmew16(CSR1, csr);
}

/*FGROUP primitives
get event/orbit counter content
*/
int getEOcnt() {
w16 d2316, d1500;
w32 evcount;
d2316=vmer16(EOcnt1) & 0xff;
micwait(1);
d1500=vmer16(EOcnt2) & 0xffff;
evcount= d2316<<16 | d1500;
return(evcount);
};
/*FGROUP primitives
settrig(int trigSource, int frequency): set trigger Source
and trigger frequency for random trigger.
trigSource: 
0=L1A0, 1=L1A1, 2=L1A2, 3=L1A3,
4=VME, 5=random trigger
frequency (valid for random trigger):
0=1Hz, 1=100Hz, 2=1kHz, 3=5kHz, 4=10kHz, 
5=25kHz, 6=50kHz, 7=100kHz
*/
void settrig(int trigSource, int frequency) {
static w16 csr1=0;   /* the last written value to CSR1 register */
w16 newcsr;
newcsr= csr1 & 0x8fb8; /* mask freq,trigSource bits, don't reset L1A FIFO*/
newcsr= newcsr | (trigSource&0x7) | (frequency & 0x7)<<12;
/*printf("%x -> CSR1", newcsr);*/
vmew16(CSR1, newcsr);
if(trigSource == 5) {
  w32 evcount;
  Delay(4);
  evcount= getEOcnt();
  printf("event counter after 3 secs:%d\n",evcount);
};
}

/*FGROUP TOP GUI InpSelTiming 
Examine/set CSR1 register (Input selection and timing)
*/
/* FGROUP examples GUI guiexample 
example of GUI 
*/
/* FGROUP examples GUI setTrigRadio
sets trigger source and frequency (in case of random trigger)
*/
/* FGROUP examples GUI setTrigMenu
*/
/*FGROUP primitives
Set counting (events/orbits) and clear event/orbit counter:
setcount(0) - count events
setcount(1) - count orbits
*/
void setcount(int orbits) {
static w16 csr1=0;   /* the last written value to CSR1 register */
w16 newcsr;
newcsr= csr1 & 0x7fbf; /* mask OrbitCount bit, don't reset L1A FIFO */
if(orbits!=0) newcsr= newcsr | 0x8000;
vmew16(CSR1, newcsr);
vmew16(EOCreset,1);
/*vmew16(EOcnt1,0); vmew16(EOcnt2,0); */
}

/*FGROUP primitives
send 1 or more L1 triggers */
void sendL1(int cnt) {
int i=0;
while(1) {
  vmew16(L1Agen, 0xaaaa);
  if(cnt!=0) {
    i++;
    if( i>=cnt) break;
  };
};
}

static w32 memL1L2m[13];
void sendlastL1M() {
int i; w32 dat;
/* vmew16(BCLFACttcrxadr, 0x8001);   directly */
for(i=0; i<=4; i++) {
  dat= memL1L2m[i]; 
  /* vmew16(BCLFACdata, dat);         directly */
  vmew32(BCDBG2, 0x80010000|dat);  /* through FIFO2 */
};
}
void sendlastL2M() {
int i;
w16 dat;
/* vmew16(BCLFACttcrxadr, 0x8001);  directly */
for(i=5; i<=12; i++) {
  /*vmew32(BCDBG3, memL1L2m[i]);  20.11(following 2 lines instead of this one*/
  dat= memL1L2m[i]; 
  /* vmew16(BCLFACdata, dat);      directly */
  vmew32(BCDBG2, 0x80010000|dat);  /* through FIFO2 */
};
}

/*FGROUP primitives
send L1 trigger and L1 message
wo1-wo5: 5 12-bits word comprising the L1message */
void sendL1M(w32 wo1, w32 wo2, w32 wo3, w32 wo4, w32 wo5 ) {
memL1L2m[0]= TTCBrExRe | TTCAL1h | (TTCDATAMask &wo1);
memL1L2m[1]= TTCBrExRe | TTCAL1d | (TTCDATAMask &wo2);
memL1L2m[2]= TTCBrExRe | TTCAL1d | (TTCDATAMask &wo3);
memL1L2m[3]= TTCBrExRe | TTCAL1d | (TTCDATAMask &wo4);
memL1L2m[4]= TTCBrExRe | TTCAL1d | (TTCDATAMask &wo5);
/* vmew16(L1Agen, 0xaaaa);  */ sendL1(1);
Delay(10);
sendlastL1M();
}

/*FGROUP primitives
send L2 message. wo1-wo8: 8 12-bits words comprising the L2 message */
void sendL2M(w32 wo1, w32 wo2, w32 wo3, w32 wo4, w32 wo5,
        w32 wo6, w32 wo7, w32 wo8 ) {
memL1L2m[5]= TTCBrExRe | TTCAL2ah | (TTCDATAMask & wo1);
memL1L2m[6]= TTCBrExRe | TTCAL2ad | (TTCDATAMask & wo2);
memL1L2m[7]= TTCBrExRe | TTCAL2ad | (TTCDATAMask & wo3);
memL1L2m[8]= TTCBrExRe | TTCAL2ad | (TTCDATAMask & wo4);
memL1L2m[9]= TTCBrExRe | TTCAL2ad | (TTCDATAMask & wo5);
memL1L2m[10]= TTCBrExRe | TTCAL2ad | (TTCDATAMask & wo6);
memL1L2m[11]= TTCBrExRe | TTCAL2ad | (TTCDATAMask & wo7);
memL1L2m[12]= TTCBrExRe | TTCAL2ad | (TTCDATAMask & wo8);
sendlastL2M();
}

/*FGROUP primitives 
sendtrigger(int cnt) 
  send (L1, last L1 message, last L2 message) in loop (cnt times)
  sendL1M(), sendL2M() should be called before 
   to prepare L1,L2 messages
cnt: 0-> endless loop
    >0-> send cnt L1,L2 sequences 
*/
void sendtrigger(int cnt) {
int i=cnt,k=0;
while(1) {
  if(i>0) {
    i--;
    if(k>100000 ) {k=0;printf(" i je :%d\n",i);};
    k++;
  };
  /*vmew16(L1Agen, 0xaaaa)*/ sendL1(1); 
  Delay(60);
  sendlastL1M();
  Delay(800);
  sendlastL2M();
  if(quit !=0) {
    printf("sendtrigger: SIGUSR1 received, exiting \n");
    return;
  };
  if(i==0) break;
};
}

/*FGROUP TOP GUI sendL1L2  
send: L1 & L1message or
      L2message or
      L1, L1message, L2message
*/
/*FGROUP primitives
Send broadcast command (short async cycles)
data:
1 bunch counter reset
2 event counter reset
3 both counters reset
*/
void sendBroadcast(w8 data) {
vmew8(BCSFACdata, data);
}

/*FGROUP primitives
sendcontrol(int E, w8 subaddr, w8 data, cycles)
send broadcast data
E:
0 access TTCrx internal registers
1 external access
subaddr,data: data to be sent
cycles: # of repetitions
*/
void sendcontrol(int E, w8 subaddr, w8 data, int cycles) {
w16 adr,dat;
if( E == 0) adr= 0x8000;
else        adr= 0x8001;
{
int i=0;
while(i<cycles) {
/*  usleep(1000000);*/
     sendL1(1); /*VON to be removed */
  vmew16(BCLFACttcrxadr, adr);
  dat= subaddr<<8 || data; vmew16(BCLFACdata, dat);
  i++;
};
};
}
/* FGROUP primitives
sendbg012(w8 subaddr, w8 data, int cycles) {
w32 dat;
int i=0;
while(i<cycles) {
  vmew32(BCDBG0, adr);
  dat= 0xsubaddr<<8 || data; vmew16(BCLFACdata, dat);
  i++;
};
}
*/

/*FGROUP primitives
write 'data (16bits)' through Bgo2 'cycles' times 
*/
void senddata(int cycles, w16 data) {
int i=0;
while(i<cycles) {
  Delay(20);
  if(quit !=0) {
    printf("SIGUSR1 received, exiting \n");
    return;
  };
  vmew32(BCDBG2, 0x80010000|data);
  i++;
};
}

w32 r16x4(w32 fromadr) {
w32 i,rc=0;
for(i=0; i<=12; i=i+4) {
  rc= rc<<8;
  rc= rc | (vmer16(fromadr+i) & 0xff);
};
return(rc);
}
/*FGROUP  TOP 
Used for test of TTCvi -Dout strob doubled on TTCrx
(instad of 1 word in B channel we have seen 2 words for fifos 0 and 2
26.10.2006. After replugging the TTCvi board in its VME slot this fault
disappeared). This happened with TTCvi:
CERNID:80030 Serial number:5054543 Board revision:20020415
(used as HMPID in DAQ ref. setup).
All fifos should be set as follows:
vmew16(CSR2,0xff00);
bgoinit(fifoN,    0, 0, 0x03, 0xffffffff); 

n       -# of loops
dat     -data sent in 1 loop
micsecs -wait micsecs between vmew32(...
fifo    - 0,1,2,3
*/
void send1w(int n, w32 dat, int micsecs, int fifo) {
int i; w32 va;
/* vmew16(BCLFACttcrxadr, 0x8001);   directly */
va= BCDBG0+4*fifo;
for(i=0; i<n; i++) {
  /* vmew16(BCLFACdata, dat);         directly */
  usleep(micsecs);
  vmew32(va, 0x80010000|dat);  /* through FIFO 0-3 */
};
}
/*FGROUP TTCconfig
Print serial number of TTCvi board
*/
void printsernumber() {
printf("CERNID:%x Serial number:%x Board revision:%x\n",
	r16x4(0x22), r16x4(0x32), r16x4(0x42));
}
/*FGROUP TTCconfig
the TTCvi board software reset
*/
void reset() {
vmew16(BOARDreset, 0xff);
}

/*FGROUP TTCconfig
bcdelay: delay in bunch crossings for B-Go1 channel (prepulse)
*/
void ppdelay(w16 bcdelay) {
vmew16(IDel1, bcdelay);
}
/*FGROUP TTCconfig
This routine should be removed and the one in ltulib/ltuinit.c to be used!
bgc      -Bgo channel (0..3)
del, dur -delay, duration for this channel
mode     - mode of the channel
data     -data to be written into B Channel Data FIFO
Modified registers:
chan delayreg durreg modereg B-go(data)
0    0x92     0x94   0x90    0xb0      Orbit (Bgo driven)
1    0x9a     0x9c   0x98    0xb4      PP (Bgo driven)
2    0xa2     0xa4   0xa0    0xb8      L1/L2 data (FIFO driven)
*/
void bgoinit(int bgc, w16 del, w16 dur, w16 mode, w32 data) {
w32 delreg, durreg,modereg,bcdbgoreg;
w32 bgc8;
bgc8=bgc*8;
delreg= IDel0+ bgc8;
durreg= IDur0+ bgc8;
modereg= BGo0mode+ bgc8;
bcdbgoreg= BCDBG0+ bgc*4;
vmew16(delreg, del);
vmew16(durreg, dur);
vmew16(modereg, mode);
if(data != 0xffffffff) {
  vmew32(bcdbgoreg, data);
  printf("Bgo%d  delay:%d duration:%d mode:0x%x data:%x\n",
    bgc, del, dur, mode, data);
} else {
  printf("Bgo%d  delay:%d duration:%d mode:0x%x data:not set/sent\n",
    bgc, del, dur, mode);
};
}
/*FGROUP TTCconfig
*/
void bgodump(int bgc) {
w32 delreg, durreg,modereg,bcdbgoreg;
w32 bgc8;
w16 del; w16 dur; w16 mode; //w32 data;
bgc8=bgc*8;
delreg= IDel0+ bgc8;
durreg= IDur0+ bgc8;
modereg= BGo0mode+ bgc8;
bcdbgoreg= BCDBG0+ bgc*4;
del= vmer16(delreg)& 0xfff;
dur= vmer16(durreg)&0xff;
mode= vmer16(modereg)&0xf;
//data= vmer32(bcdbgoreg);   only W
printf("Bgo%d  delay:%d duration:%d mode:0x%x \n",
  bgc, del, dur, mode);
}
/*FGROUP TTCconfig
This routine, when called from ltu.exe, is to be replaced be 
call to ttcInit(0) -see ltulib/ltuinit.c
Initialize TTCvi, TTCrx. 
Has to be called always after power up of TTCvi or TTCrx.
It is called automatically after the start of this software (ttcvi.exe)
*/
void ttcinit() {
w16 csr1,csr2;
csr1=0x8047; csr2= 0xfc00;
vmew16(CSR2,csr2);
usleep(1000);
printf("CSR1:0x%x   CSR2:0x%x\n", csr1, csr2);
/*vmew16(CSR1,0x8044);  Orbit Counting, L1AFIFO reset, VME trigger */
vmew16(CSR1,csr1);  /* Orbit Counting, L1AFIFO reset, disable L1 */
usleep(1000);
vmew16(BCLFACdata, 0x06ff); usleep(100000);  // TTCrx reset(p.21) 
/* 1000 us is short! Let's use 100000 from 20.3.2007
After TTCrx reset, TTCrx goes to not ready state
for quite long time (400micsecs but possibly much more -according
to the quality of the clock received over fibre), which can prevent
the execution of following instructions 

SSD: 
After FE off/on it is necessary, before programming TTCrx control reg.3:
- to do hw-TTCrx reset (pin on TTCrx) or
- to disconnect/connect fibre */

/* set TTCrx internal control register - enable all the outputs (3f3) */
vmew16(BCLFACttcrxadr, 0x8000);   /* access internal regs */
usleep(1000);
/*25.10: last 2 bits of Control register have to be '01' -the
trigger mode on TTCrx as used in ALICE experiment */
vmew16(BCLFACdata, 0x03f9);    /* TTCrx control reg3, f9 -data for cont. reg */
usleep(1000);
vmew16(BCLFACdata, 0x0000);       /* Fine Delay1  */
usleep(1000);
vmew16(BCLFACdata, 0x0180);       /* Fine Delay2  */
usleep(1000);
vmew16(BCLFACdata, 0x0200);       /* Coarse delay*/
usleep(1000);
/* following usleeps necessary for correct FineDelay2 setting */
/* BGo-0: Orbit -> BCntRes signal on TTCrx.
26.6.06: BCntRes delayed to LTU.orbit by: 53+48.
   44 leads to missing BCntRes signal
15.9.06
   49 -leads to the occassional shift of BCntRes signal. Set to 54
*/
bgoinit(0,  53, 54, 0x0c, 0x00800000);  /* BCreset */
usleep(1000);
/* BGo-1: Prepulse. */
bgoinit(1, TTCppdelay, 54, 0x08, 0x7e000000); /*Sys+User message bits: all on*/
usleep(1000);
/* BGo-2: L1/2 messages, 0x3: front panel input disabled, async cycles */
bgoinit(2,    0, 0, 0x03, 0xffffffff); 
/* usleep(1000); 
bgoinit(2,    0, 0, 0x03, 0x800003f3); */
/*bgoinit(3, 0, 0, 0x01, 0x800103f3); ?? */
}
/*FGROUP TTCconfig
reads current settings on TTCvi board
*/
void ttcdump() {
int i;
w16 csr1,csr2;
csr1= vmer16(CSR1);
csr2= vmer16(CSR2) & 0xfff;
printf("CSR1:0x%x   CSR2:0x%x\n", csr1, csr2);
for(i=0; i<=2; i++) bgodump(i);
}

void initmain() {
printf("initmain called...\n");
#ifdef SIMVME
/* commented because we do not register any 'simulation functions' for ttcvi
printf("initmain: now calling regfuns() (SIMVME mode)...\n");
regfuns();
*/
#endif
}
void boardInit() {
printsernumber();
ttcinit();
}
void endmain() {
}

