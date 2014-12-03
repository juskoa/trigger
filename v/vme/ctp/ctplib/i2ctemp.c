#include <stdio.h>
#include <unistd.h>    /* usleep() */
#include "vmewrap.h"
#include "ctp.h"
#include "Tpartition.h"

int prterror=1;   // 0: do not print error messages (too many of them)
w32 hourcnt=0;    // only first MAXERRSPRINT and then each 1000th error message
#define MAXERRSPRINT 40
/*-------------------------------------------------------- errifallowed */
void errifallowed(char *msg) {
hourcnt++;
if(hourcnt>MAXERRSPRINT) {   // print first MAXERRSPRINT messages
  prterror=0;
  if((hourcnt%1000)==0) {   // and then only once per 1000
    prterror=1;
  };
};
if(prterror==1) { printf("Error:%s\n",msg); };
}
/*-------------------------------------------------------- all the boards */
int ReadTemp(int ix) {   /* ix: the board (pointer to ctpboards[]) */
int i; w32 status, temp2;
vmew32(TEMP_START+BSP*ctpboards[ix].dial, DUMMYVAL);
for(i=0; i<3; i++) {
  usleep(300);
  status=vmer32(TEMP_STATUS+BSP*ctpboards[ix].dial);
  if( (status & 0x1) == 0) goto TEMPOK;
};
temp2=0; return(temp2);
TEMPOK:
temp2=vmer32(TEMP_READ+BSP*ctpboards[ix].dial)&0xff;
return(temp2);
}
/*---------------------------------------------------------------- I2C  */
#define MICSEC1 100
/* ret: 1 if timeout  0: not busy */
/*--------------------------------------------------*/ int i2cwait(int phase) {
w32 itcr; int busytime=0;
while(1) {
  itcr= vmer32(I2C_SET);
  if((itcr & 0x2000) == 0) break;
  usleep(MICSEC1);
  busytime= busytime+MICSEC1;
  if(busytime>3000) {
    char msg[300];
    sprintf(msg, "i2cwait: timeout (%d >  3000 micsecs) phase:%d",
      busytime, phase);
    errifallowed(msg);
    return(1);
  };
};
if((itcr & 0x1000)==0x1000) {
  char msg[300];
  sprintf(msg, "Err bit on found in i2cwait(%d)",phase);
  errifallowed(msg);
  return 2;
};
//printf("i2cwait:%d micsecs in phase:%d\n",busytime,phase);
return(0);
}
/*-------------------------------------------------*/ void vme2volt(w32 vme ){
float volt5,volt3_3,volt1_5,volt5b;
 volt5=(vme & 0xff)*23.725;
 volt5b=((vme & 0xff000000)>>24)*23.725;
 volt3_3=((vme & 0xff00)>>8)*23.725;
 volt1_5=((vme & 0xff0000)>>16)*12.941;
 printf("%4.0f %4.0f %4.0f %4.0f [mV]\n",volt5,volt3_3,volt1_5,volt5b);
}
/*FGROUP INT
read voltages from 1 board.
Cabling in the lab:
-------------------
channel: 0-7. 0-5: for LTU crates, 
              6:BUSY/1 L0/2 L1/6 L2/4
              7:FO/0-6 (according to FO dial), INT/7
branch:  0-7  -see above (BUSY/1 -> BUSY board is channel6, branch 1
Cabling in the pit:
-------------------
channel: 0: F1-FO6 + INT
         1: BUSY, L0, L1, L2
         2-7: branches: 0,1,2 [3]
         2: crate1 alidcsvme006 sdd muon_trk muon_trg daq
         3: crate2 alidcsvme007 spd tof v0
         4: crate3 alidcsvme004 trd zdc emc
         5: crate4 alidcsvme005 tpc pmd acorde
         6: crate5 alidcsvme002 ssd fmd t0
         7: crate6 alidcsvme003 hmpid phos cpv ad
return: 4 channels packed in 1 32bit word
*/
/*------------------------------------*/ w32 i2cread(int channel, int branch) {
w32 cb, i2crd; char msg[300]; //i2csetrd; int i;
cb= 0x80 | (channel<<4) | branch;
//printf("i2cread: cb=0x%x \n",cb);
//if(i2cwait(1)) goto RET; because I2C_SET remembers last error 
vmew32(I2C_SET, cb); 
//if(i2cwait(2)) goto RET; because I2C_SET remembers last error
vmew32(I2C_MUXWR, DUMMYVAL); if(i2cwait(3)) goto RET;
vmew32(I2C_MUXRD, DUMMYVAL); if(i2cwait(4)) goto RET;
  //i2csetrd= vmer32(I2C_SET); if(i2cwait(5)) goto RET;   //only for test
  //printf("   I2C_SET reading:%x\n", i2csetrd);
vmew32(I2C_ADCWR, DUMMYVAL); if(i2cwait(6)) goto RET;
//for(i=0; i<100000; i++) {
vmew32(I2C_ADCRD, DUMMYVAL); if(i2cwait(7)) goto RET;
//};
i2crd= vmer32(I2C_DATA); if(i2cwait(8)) goto RET;
//printf("channel:%d branch:%d  reading:%x: ", channel, branch,i2crd);
//vme2volt(i2crd);
return(i2crd);
RET:
sprintf(msg, "i2cread:channel:%d branch:%d  device not responding.", 
  channel, branch);
errifallowed(msg);
return(i2crd);
}
/*----------------------*/ int i2cgetaddr(int ix, int *channel, int *bra) {
/* Input:
ix: 0-NCTPBOARDS: CTP board, i.e.: busy, L0/1/2, INT, FO1/2/3/4/5/6
    11: ltu0
    12: ltu1  ...
    34: ltu23   0-23: DAQ/ECS det numbers
Output:
VALID.LTUS is consulted.
rc: 0: ok, channel, bra were set
   >0: not assigned
*/
int rc=0; int chan, branch;
#ifdef CAVERN_SETUP
int daqdet=-1; // DAQ/ECS detector number
  if( (ix>=5) && (ix<NCTPBOARDS) ) {   //fo
    chan= 0; branch= ctpboards[ix].dial; 
  } else if(ix==0) {chan=1; branch=1;   // busy
  } else if(ix==1) {chan=1; branch=2;   // L0
  } else if(ix==2) {chan=1; branch=6;   // L1
  } else if(ix==3) {chan=1; branch=4;   // L2
  } else if(ix==4) {chan=0; branch=7;   // INT
  } else if(ix<(NCTPBOARDS+24))    {
    Tdetector *vlp;
    daqdet= ix-NCTPBOARDS;
    vlp= findLTUdaqdet(daqdet);   
    if(vlp==NULL) { rc=1; 
    } else {
      chan= vlp->i2cchan; branch= vlp->i2cbran;
    };
  } else {printf("internal error in i2cgetaddr\n"); rc=2;
  };
  /*
    chan=2; branch=0;   // sdd
  validLTUs[ixtab].i2cchan=a3[4];
  validLTUs[ixtab].i2cbran=a3[5];
  } else if(ix==NCTPBOARDS+1)  {chan=2; branch=1;   // muon_trk
  } else if(ix==NCTPBOARDS+2)  {chan=2; branch=2;   // muon_trg
  } else if(ix==NCTPBOARDS+3)  {chan=2; branch=3;   // daq
  } else if(ix==NCTPBOARDS+4)  {chan=3; branch=0;   // spd
  } else if(ix==NCTPBOARDS+5)  {chan=3; branch=1;   // tof
  } else if(ix==NCTPBOARDS+6)  {chan=3; branch=2;   // v0
  } else if(ix==NCTPBOARDS+7)  {rc=1;           // --
  } else if(ix==NCTPBOARDS+8)  {chan=4; branch=0;   //  
  } else if(ix==NCTPBOARDS+9)  {chan=4; branch=1;   // 
  } else if(ix==NCTPBOARDS+10) {chan=4; branch=2;   //
  } else if(ix==NCTPBOARDS+11) {rc=1;           // --
  } else if(ix==NCTPBOARDS+12) {chan=5; branch=0;   //
  } else if(ix==NCTPBOARDS+13) {chan=5; branch=1;   //
  } else if(ix==NCTPBOARDS+14) {chan=5; branch=2;   //
  } else if(ix==NCTPBOARDS+15) {rc=1;           // --
  } else if(ix==NCTPBOARDS+16) {chan=6; branch=0;   //
  } else if(ix==NCTPBOARDS+17) {chan=6; branch=1;   //
  } else if(ix==NCTPBOARDS+18) {chan=6; branch=2;   //
  } else if(ix==NCTPBOARDS+19) {rc=1;           // --
  } else if(ix==NCTPBOARDS+20) {chan=7; branch=0;   //
  } else if(ix==NCTPBOARDS+21) {chan=7; branch=1;   //
  } else if(ix==NCTPBOARDS+22) {chan=7; branch=2;   //
  } else if(ix==NCTPBOARDS+23) {rc=1;           // --
  } else {printf("internal error in i2cgetaddr\n"); rc=2;
  };
*/
#endif
#ifdef CERNLAB_SETUP
  if(ix==0) {chan=6; branch=1;   // busy
  } else if(ix==1) {chan=6; branch=2;   // L0
  } else if(ix==2) {chan=6; branch=6;   // L1
  } else if(ix==3) {chan=6; branch=4;   // L2
  } else if(ix==4) {chan=7; branch=7;   // INT
  } else if(ix<NCTPBOARDS) {   //fo
    chan= 7; branch= ctpboards[ix].dial; 
  } else {
    //printf("internal error in i2cgetaddr\n"); 
    rc =2;
  };
#endif
*channel= chan; *bra= branch;
//printf("i2cgetaddr rc:%d ix/daqd:%d/%d chan bra: %d %d\n", rc, ix, daqdet, chan, branch);
return(rc);
}

