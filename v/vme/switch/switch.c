/*BOARD switch 0x110000 0x30068 */
/* has to be always 0x110000 0x30068  (i.e. switch counters reading)
*/
/* NOTES:
25.5.
24ns delay/channel (96ns measured for 4 connections daisy chained)
27.5.
Configuration files:
there are 2 config files in CTP database directory:
LTU.SWITCH -LTU (left one) TX board config
CTP.SWITCH -CTP (rght one)...
The format is:
namepatch namectp swin swout ctpin

swin: switch input (1-50)
swout: switch output (1-25)
ctpin: CTP L0 input (1-24)
*/
#include <stdio.h>
#include <unistd.h>
//#include <stdlib.h>
#include <string.h>
/*
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
*/
#include "vmewrap.h"
#include "switch.h"

extern int quit;
//extern char BoardBaseAddress[];

/*FGROUP
*/
void reset() {
w32 shift; w32 r;
for(shift=0; shift<=TXCTPSHIFT; shift=shift+0x10000) {
  vmew32(shift+CSR, 1); r=vmer32(shift+CSR);
  if(r&0x1) {
    printf("%x: OK (0x1 bit in CSR set with 1st reading after reset\n", shift);
  } else {
    printf("%x: Error (0x1 bit in CSR no set to 1 after reset\n", shift);
  };
};
}

void checkboard(w32 basehex) {
int rc,vsp=-1;
char base[12];
sprintf(base,"0x%x", basehex);
rc= vmxopenam(&vsp, base, "0x68", "A24");
if(rc!=0) {
  printf("rc: %d from vmxopen(%s...\n", rc, base);
} else {
  w32 csr;
  csr= vmxr32(vsp, CSR);
  printf("%s CSR:0x%x\n", base, csr);
  rc= vmxclose(vsp);
  if(rc!=0) {
    printf("rc: %d from vmxclose(%d)\n", rc, vsp);
  };
};
}
/*FGROUP
read CSR from all switch boards
*/
void check() {
checkboard(TXLTUBASE);
checkboard(RXLOWBASE);
checkboard(RXHIGHBASE);
checkboard(TXCTPBASE);
}
/*FGROUP
ctpltu: 1:ctp   0:ltu
input: 0 disable channel output
       1..50 connect output to input
output: 1..25   
RC: 0: already set or error (no vme write)
    1: set (vme write)
*/
int setchanctpltu(int ctpltu, int input, int output) {
w32 adr, shift, inputr; int rc;
char ctpltutx[4];
if(ctpltu==0) {
  shift= TXLTUSHIFT;
  strcpy(ctpltutx,"LTU");
} else if(ctpltu==1) {
  shift= TXCTPSHIFT;
  strcpy(ctpltutx,"CTP");
} else {
  printf("Bad ctpltu (1:ctp or 0:ltu allowed)\n"); return(0);
};
if((output>MAXOUTPUTS) || (output<1)) {
  printf("Bad output (1..%d allowed)\n", MAXOUTPUTS); return(0);
};
if((input>50) || (input<0)) {
  printf("Bad input (0..50 allowed)\n"); return(0);
};
adr= shift + TXCFGSTART + (output-1)*4;
if((inputr=vmer32(adr)) == (w32)input) {
  //printf("ok not written:%s: setchan: %x: %d -> %d\n", ctpltutx, adr, input, output) ;
  rc=0;
} else {
  vmew32(adr, input);
  printf("%s: setchan: %x: %d -> %d\n", ctpltutx, adr, input, output);
  rc=1;
};
return(rc);
}
/*FGROUP
ctpltu: 1:ctp   0:ltu
Print current configuration (only connected outputs)
inp -> ctpinp   where:
inp -the nmber of CTPSWITCH input (1..50)
ctpinp -the number of CTP L0 input +1  (2..25)
*/
void showcfgCPU(int ctpltu) {
int ix, active=0;
w32 shift;
char ctpltutx[4];
if(ctpltu==0) {
  shift= TXLTUSHIFT;
  strcpy(ctpltutx,"LTU");
} else if(ctpltu==1) {
  shift= TXCTPSHIFT;
  strcpy(ctpltutx,"CTP");
} else {
  printf("Bad ctpltu (1:ctp or 0:ltu allowed)\n"); return;
};
printf("%s:\n",ctpltutx);
for(ix=0; ix<MAXOUTPUTS; ix++) {
  w32 inp, adr;
  adr= shift+TXCFGSTART+ ix*4;
  inp= vmer32(adr);
  if(inp != 0) {
    printf("%x: %2d->%2d ", adr, inp, ix+1);
    active++;
    if((active%2)==0) printf("\n");
  };
};
if((active%2)!=0) printf("\n");
printf("Connected outputs:%d\n", active);
}
/*FGROUP
set type of pulses to be counted:
0: counting off (default)
5: <  1 period
6: >= 1 period
7: == 1 period
0xa: >= 2 periods
0xb: == 2 periods
0xe: >= 3 periods
0xf level ON (probably means: strobing with clock? 
*/
void setCounters(int type) {
w32 shift; int ix;
for(shift=RXLOWSHIFT; shift<=RXHIGHSHIFT; shift=shift+0x10000) {
  for(ix=0; ix<25; ix++) {
    vmew32(shift+RXCNTSTART+ix*4, type<<28);
  };
};
}
/*FGROUP
Print all input counters (both RX boards)
*/
void printCounters() {
int ix, inputn=0; w32 shift, cnt;
for(shift=RXLOWSHIFT; shift<=RXHIGHSHIFT; shift=shift+0x10000) {
  for(ix=0; ix<25; ix++) {
    inputn++;
    cnt=vmer32(shift+RXCNTSTART+ix*4) & 0x3ffffff;
    printf(" %d:%d=0x%x ", inputn, cnt, cnt);
    if((inputn%3)==0) printf("\n");   // DOES NOT WORK WITH 3 !
  };
};
printf("\n");   //has to be here (always \n at the end of stdout)
}

void initmain() {   /* called once, at the very beginning */
//printf("do not forget setCounters()! -necessary for printCounters()\n");
}
void endmain() {   /* called once, at the very end */
}
void boardInit() {   /* called once, after initmain, if -noboardInit */
loadTable(1); loadTable(0);      /*  load CTP.SWITCH and LTU.SWITCH   */
setCounters(0xf);   // strobe with clock?
}
/*ENDOFCF
*/

