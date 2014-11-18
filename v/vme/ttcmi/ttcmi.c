/*BOARD ttcmi 0xf00000 0x100000 A32 */
/* 3 boards:
RFRX1  0x300000 A24
RFRX2  0x400000 A24
RF2TTC 0xf00000 A32
CORDE  0x700000 A32

fibres from LHC connected to RFRX boards (base:0x300000 and 0x400000):
RFRX1   RFRX2
BCREF   ---
BC1     BC2
ORB1    ORB2

RF2TTC: BST fibre is marked 41994C XF1346
*/

#include <string.h>
#include <stdio.h>
#include <unistd.h>   //usleep
#include "vmewrap.h"
#include "infolog.h"
#include "vmeblib.h"
//#include "../ctp/ctplib/ctplib.h"
#include "ttcmi.h"

// following: deliberatelly leave NULL -i.e. readCounters crash if called
// todo: why readdCounters linked to ttcmi.exe? If needed, needs to be fixed
//Tctpshm *ctpshmbase=NULL; 
void *ctpshmbase=NULL; 
//Tinput *validCTPINPUTs=NULL;
void *validCTPINPUTs=NULL;
//EXTERN Tdetector *validLTUs;
void *validLTUs;
//EXTERN Hardware HW;
int HW;
//extern int clg_defaults[MAXCLASSGROUPS];
int clg_defaults[1];


/* HIDDEN primitives */

extern int quit;
void writeall();
void printRFRX(char *rfrxbase);

/*FGROUP calib
fifoadr: addr. of ORB1_PERIOD_FIFO_RD
*/
int read1000(w32 fifoadr,int del) {
int ix,not0xDEC=0;
for(ix=0; ix<300; ix++) {
  w32 v;
  v= vmer32(fifoadr);
  if(ix==0) continue;   // 1. reading after reset not valid
  if(v & 0x4000) break;
  if(v!=0xDEC) not0xDEC++;
};
printf("read1000: %4.1fns (%d reads) not0xDEC:%d\n",del/2.,ix, not0xDEC);
return(not0xDEC);
}
/*FGROUP delays
set delayadd with halfns value (in steps of 0.5 ns):
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
*/
void i2cset_delay(w32 delayadd, int halfns);
void DLL_RESYNC(int msg);

/*FGROUP calib
orb:
0: ORB1/BC1 -BPTX monitoring
1: ORB2/BC2 -
2: ORBmain=ORB1 BCmain= BC1   ALICE clock BEAM1
4: ORBmain=ORB2 BCmain= BC2   ALICE clock BEAM2

Set BC_DELAY25_BC1/2/MAIN to their final values before using this!
16.3.2011:
all 3 Set to 29,28,27 - ORB1/BC1 calibration is not working
    for <=26 is OK
Similarly for ORB2/BC2: with BC_DELAY25_BC2>27 calibration is not working
(i.e. all read1000() returns >0 )
Here is the table with 0 in BC_DELAY25_BC1/2/main:
   bad regions[ns]
0:  9-9.5    (# of bad readings in region: <5)
1: 21.5-22    detto
2: 8.5-9.5   <90

i2cset_delay -removed DLL resync, now ok:
Here is the table with 29 in BC_DELAY25_BC1/2/main:
   bad regions[ns]
0:  24        (# of bad readings in region: <=1)
1:  10        16   
2:  24        1  (when BC1corde changed 8500->8650 then 100 bad readings)
4:  10.5      1
*/
void calibrate(int orb, int fromdel, int todel) {  
w32 adr,adrDEL,valDEL;
// BC1_MAN_SELECT=1
// ORB1_MAN_SELECT=1
// 
adrDEL= ORBIN_DELAY25_ORB1 + 4*orb;
adr= ORB1_PERIOD_FIFO_RD - 0x40*orb;
if(orb==2) {
  adrDEL= ORBIN_DELAY25_ORB1;
  adr= ORBmain_PERIOD_FIFO_RD;
} else if(orb==3) {
  printf("bad option (only 0 1 2 4 allowed)\n"); return;
} else if(orb==4) {
  adrDEL= ORBIN_DELAY25_ORB2;
  adr= ORBmain_PERIOD_FIFO_RD;
};
valDEL= fromdel;
while(1) {
  int bad;
  //printf("del:%d not0xDEC:%d\n", valDEL&0x3f, bad);
  if(valDEL<(w32)todel) {
    i2cset_delay(adrDEL, valDEL);   // enable, 0
  } else {
    break;
  };
  vmew32(PERIOD_COUNTER_RESET,0x7);
  usleep(30000);   // wait cca 300 orbits
  bad= read1000(adr, valDEL);
  valDEL++;
};
}

/*FGROUP delays
read BC_DEALY25_* registers
*/
void printBC_DELAYS() {
int ix;
char *names[40]={
"BC_DELAY25_BC1",
"BC_DELAY25_BC2",
"BC_DELAY25_BCREF",
"BC_DELAY25_BCMAIN",
"BC_DELAY25_GCR",
"ORBIN_DELAY25_ORB1",
"ORBIN_DELAY25_ORB2",
"ORBIN_DELAY25_GCR",
"ORBOUT_DELAY25_ORB1",
"ORBOUT_DELAY25_ORB2",
"ORBOUT_DELAY25_ORBMAIN",
"ORBOUT_DELAY25_GCR",
""};
w32 adds[]={
0x7D000,
0x7D004,
0x7D008,
0x7D00C,
0x7D014,
0x7D020,
0x7D024,
0x7D034,
0x7D040,
0x7D044,
0x7D048,
0x7D054,
0};
ix=0;
while(1) {
  w32 d, addr;
  addr= adds[ix];
  if(addr==0 || names[ix][0]=='\0') break;
  d= i2cread_delay(addr);
  printf("%24s 0x%x:%x\n", names[ix], addr, d);
  ix++;
};
}
/*FGROUP delays
#define ORB1_COARSE_DELAY          0x7FB5C
#define ORB2_COARSE_DELAY          0x7FB1C
#define ORBmain_COARSE_DELAY       0x7FADC
*/
void setOrbitCoarse(w32 addr, int coarse) {
vmew32(addr, coarse);
}
void setbcorbitBO1(int);
/*FGROUP
Input: maino: 
1 -> BC1/Orbit1
2 -> BC2/Orbit2
3 -> BCref/int BCmain synch. orbit generator
4 -> internal 40.078MHz/int BCmain synch. orbit generator
Output:
- clock is changed + log messages written into DAQ infoLogger
Note (todo?):
BCREF/
localBC/
*/
void setbcorbitMain(int maino);
/*FGROUP Resets
chip:
0: Delay25 chips
1: no efect
2: BC1-QPLL chip
2: BC2-QPLL chip
2: BCref-QPLL chip
2: BCmain-QPLL chip
6: TTCrx chips
7: whole board
Operation:
1. read+print BSET word
set BSET bit
2. read+print BSET word
set BCLEAR bit
3. read+print BSET word
*/
void resetQPLL(int chip) {
w32 v,setbit;
if(chip >7) {
  printf("chip:\n\
0: Delay25 chips\n\
1: no efect\n\
2: BC1-QPLL chip\n\
2: BC2-QPLL chip\n\
2: BCref-QPLL chip\n\
2: BCmain-QPLL chip\n\
6: TTCrx chips\n\
7: whole board\n"); return;
};
v= vmer32(BSET); printf("1. BSET:%x bit%d:%d\n", v, chip, (v>>chip)&1);
setbit= 1<<chip;
vmew32(BSET, setbit); printf("BSET<- %x\n", setbit);
v= vmer32(BSET); printf("2. BSET:%x bit%d:%d\n", v, chip, (v>>chip)&1);
vmew32(BSET, setbit); printf("BCEAR<- %x\n", setbit);
v= vmer32(BSET); printf("3. BSET:%x bit%d:%d\n", v, chip, (v>>chip)&1);
}
/*FGROUP Resets
resync DEALY25 chip (write 0x40 -> BC_DELAY25_GCR)
*/
void resyncDLL() {
DLL_RESYNC(DLL_stdout);
}
/*FGROUP
Set orbit delay in BCs in 3 ORBX_COARSE_DELAY registers.
0    - minimum shift
3563 -maximum allowed shift
4000 - show current setting
*/
void setorbitdelay(int bcs) {
int ix;
if(bcs==4000) {
  for(ix=0; ix<3; ix++) {
    w32 adr, val;
    adr= ORBX_COARSE_DELAY+ 0x40*ix;
    val=vmer32(adr);
    printf("0x%x: %d(0x%x)\n", adr, val, val);
  };
} else if(bcs<=3563) {
  for(ix=0; ix<3; ix++) {
    w32 adr;
    adr= ORBX_COARSE_DELAY+ 0x40*ix;
    vmew32(adr, bcs);
  };
  printf("all ORBX_COARSE_DELAYs changed\n");
} else {
  printf("Bad bcs:%d (has to be <=3563 or 4000)\n", bcs);
};
//printf("ORBmain_COARSE_DELAY changed only\n");
}
/*FGROUP
Read and print ORBX_COUNTER (X: main 2 1) registers */
void readorbitcnts() {
int ix;
for(ix=0; ix<3; ix++) {
  w32 adr, val;
  adr= ORBX_COUNTER+ 0x40*ix;
  val=vmer32(adr);
  printf("%u: %d(0x%x)\n", adr, val, val);
};
}
/*FGROUP
PROGRAM_ID:8102008
ch1_ref:0x5
ch2_ref:0x5
ch3_ref:0x70
rc:0 vsp:1
ch1_ref:0x5
ch2_ref:0x5
ch3_ref:0x70
polarity0: 0x7fae0:0x1   0x7fad8 0x26 delay:0
polarity1: 0x7fb20:0x1   0x7fb18 0x26 delay:0
polarity2: 0x7fb60:0x1   0x7fb58 0x26 delay:0
WORKING_MODE (bits 0..6 (0:man,1:auto)for: BC1/2/r/main Orb1/2/m:0 
BST_Machine_Mode:7
BC 1/2/ref selection (Int/Ext):EEE
BC/ORB main_MAN_SELECT: 0 2
BCmain_MAN_SELECT:
0:40.078MHz int clock, 1: BCref, 2: BC2in, 3: BC1in
ORBmain_MAN_SELECT:
0:Orbit1, 1:Orbit2, 2:int BCmain sync. orbit generator
*/
void readall() {
int ix;
char bc12rmansel[4]="III";
w32 adrpol, adrlen, pol, len, bc1man, orb1man, wwm, wmm, pol1;
printf("PROGRAM_ID:%x\n", vmer32(PROGRAM_ID));
printf("ref bc1 orbit1\n");
printRFRX("0x300000");
printf("--- bc2 orbit2\n");
printRFRX("0x400000");
/*von
vsp=-1; rc= vmxopenam(&vsp, "0x400000", "0x100", "A24");
printf("rc:%d vsp:%d\n", rc, vsp);
printf("ch1/2/3_ref:0x%x 0x%x 0x%x\n", vmxr16(vsp, ch1_ref), 
  vmxr16(vsp, ch2_ref), vmxr16(vsp, ch3_ref));
rc= vmxclose(vsp);
*/
/*vsp=-1; rc= vmxopenam(&vsp, "0x0f00000", "0x100000", "A32");
printf("rf2ttc rc:%d vsp:%d\n", rc, vsp); */
for(ix=0; ix<3; ix++) {
  w32 adrdelay, delay;
  adrpol= ORBX_POLARITY+ 0x40*ix;
  adrlen= ORBX_LENGTH+ 0x40*ix;
  adrdelay= ORBX_COARSE_DELAY+ 0x40*ix;
  pol= vmer32(adrpol); len= vmer32(adrlen); delay= vmer32(adrdelay);
  printf("polarity%d: 0x%x:0x%x   0x%x 0x%x delay:%x\n", 
    ix, adrpol, pol, adrlen, len, delay);
};
wmm= vmer32(BST_Machine_Mode); wwm= vmer32(WORKING_MODE);
printf("WORKING_MODE (bits 0..6 (0:man,1:auto)for: BC1/2/r/main Orb1/2/m:%x \n\
BST_Machine_Mode:%x\n", 
wwm, wmm);
/*
bc1man= vmer32(BC1_MAN_SELECT); orb1man= vmer32(ORB1_MAN_SELECT);
printf(
"BC1_MAN_SELECT:%d ORB1_MAN_SELECT:%d       -> set now. Possible values:\n\
               0                 1       -> LOCAL CLOCK+ORBIT\n\
               1                 0       -> LHC CLOCK+ORBIT\n\
", bc1man, orb1man);
*/
pol= i2cread_delay(BC_DELAY25_BCMAIN);
pol1= i2cread_delay(BC_DELAY25_BC1);
printf("%24s 0x%x=%d/0x%x=%d halfsns \n\n", "BC_DELAY25_BCMAIN/BC1", 
  pol, pol-0x140, pol1, pol1-0x140);
pol= vmer32(ORBmain_COARSE_DELAY);
pol1= vmer32(ORB1_COARSE_DELAY);
printf("%24s %d/%d bcs \n\n", "ORBmain_COARSE_DELAY/ORB1_", 
  pol,pol1);

if(vmer32(BC1_MAN_SELECT)) bc12rmansel[0]='E';
if(vmer32(BC2_MAN_SELECT)) bc12rmansel[1]='E';
if(vmer32(BCref_MAN_SELECT)) bc12rmansel[2]='E';
printf("BC 1/2/ref selection (Int/Ext):%s\n", bc12rmansel);
bc1man= vmer32(BCmain_MAN_SELECT); orb1man= vmer32(ORBmain_MAN_SELECT);
printf("BC/ORB main_MAN_SELECT: %x %x\n\
BCmain_MAN_SELECT:\n\
0:40.078MHz int clock, 1: BCref, 2: BC2in, 3: BC1in\n\
ORBmain_MAN_SELECT:\n\
0:Orbit1, 1:Orbit2, 2:int BCmain sync. orbit generator\n",
bc1man, orb1man);
pol= corde_get(CORDE_DELREG);
printf("CORDE reg%d: %d(0x%x)\n",CORDE_DELREG,pol,pol);
}

#define MAXHIST 100     // max. # of the status changes stored in 1 fast loop
#define LOOPSINSEC 1000 // number of 'fast loops' (cca 2milsecs) in 'slow loop'
#define INFOLOOPS 60    // info message printed after INFOLOOPS 'slow loops'
/*FGROUP
used for measurement at the end of 2008.
*/
void monitorstatus() {
int secs,readsinsec,ixhistory;
w32 prevst=0x12345678;
w32 prevmachmode=0x12345678;
FILE *f;
char dt[20];
char fname[20];
char fpath[40];
char line[2000];   // MAXHIST* len(999:155) = cca 700
#define MAXSECS 10
typedef struct Thistpoint {
w16 ix;   // 0.. MAXHIST-1
w16 status;
} Thistpoint;
Thistpoint history[MAXHIST];
getdatetime(dt); strcpy(fname, dt); fname[10]='-';
sprintf(fpath,"WORK/%s", fname);
f= fopen(fpath,"w");
secs=0;
printf("Loop reading QPLL*, BST_* for %d secs\n", MAXSECS);
while(1) {
  w32 newmachmode;
  getdatetime(dt);
  readsinsec=0; ixhistory=0;
  while(1) {
    w32 newst; int urc;
    newst= readstatus();
    if(newst != prevst) {
      if(ixhistory<MAXHIST) {
        history[ixhistory].ix= readsinsec;
        history[ixhistory].status= newst; 
        ixhistory++;
      };
      prevst= newst;
    };
    readsinsec++;
    if(readsinsec>=LOOPSINSEC) {
      if(ixhistory>0) {
        int i;
        sprintf(line, "%s %d", dt, ixhistory);
        for(i=0; i<ixhistory; i++) {
          sprintf(line,"%s %d:%x", line, history[i].ix, history[i].status);
        };
        fprintf(f,"%s\n", line);
        fflush(f);
      };
      break;
    };
    urc=usleep(500);
    //if(urc!=0) printf("usleep rc:%d\n",urc);
  };
  newmachmode= vmer32(BST_Machine_Mode);
  if(newmachmode != prevmachmode) {
    prevmachmode= newmachmode;
    printf("%s Mode:%d\n", dt, newmachmode);
    fprintf(f,"%s Mode:%d\n", dt, newmachmode); fflush(f);
  } else {
    if((secs % INFOLOOPS)==0) {
      printf("%s INFO secs:%d\n", dt, secs);
    };
  };
  secs++;
  if(secs>MAXSECS) break;
  if(quit!=0) break;
  //if(secs>9) break;
};
fprintf(f,"%s closing\n", dt); fclose(f);
}
void initmain() {
printf("initmain called...\n");
}
void boardInit() {
writeall();
}
void endmain() {
}

