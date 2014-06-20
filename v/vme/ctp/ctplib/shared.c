#include <stdio.h>
#include <string.h>
#include "vmewrap.h"
#include "vmeblib.h"
#include "ctp.h"
#include "ctplib.h"
#include "Tpartition.h"
/*---------------------------------------------------------------- L0-Shared */
/* FGROUP DbgNewFW
void loadRun(w32 runnumber){
  CTPHardware ctp;
  VALIDLTUS ltus;
  ltus.readVALIDLTUS();
  ActiveRun run(runnumber);
  run.PrintActiveRun();
  run.Load2SW();
  CTPHardware::Print();
  run.Load2HW();
} */
/* FGROUP DbgNewFW 
void printHW(){
 CTPHardware::Print();
} */
/* FGROUP DbgNewFW
void unloadRun(w32 run){
 CTPHardware::Unload(run);
} */
/*Shared memory (not yet shared between all tasks running on ctp CPU):
l0f34stat[] -rightmost bit/column (bit 0) corresponds to LUT1
*/
unsigned char l0f34stat[LEN_l0f34] ={0xd,0xe,0xa,0xd};

void getL0FUN34(w32* lut){
 w32 i;
 // toto je zle (write only!, L0FUN34 LUTs cannot be read)
 //for(i=0;i<0xfff+1;i++)lut[i]=vmer32(L0_FUNCTION34+4*i)&0xf000;
 for(i=0;i<LEN_l0f34;i++)lut[i]= l0f34stat[i];
} 
/*FGROUP DbgNewFW    
Function for writing one lut.Other lut is set to 1
This is only for function 3 
void setL0FUN34(w32* lut){
 w32 i;
 if(!lut)printf("setL0FUN34: writing dummy falues.\n");
 for(i=0;i<0xfff+1;i++){
    if(lut){
      vmew32(L0_FUNCTION34,i+lut[i]<<11+1<<12);
    }else{
      vmew32(L0_FUNCTION34,(0x3)<<11+i);
    }
 }
}
*/
/*FGROUP DbgNewFW    */
void printL0FUN34(){
w32 l0ver,i;
l0ver= vmer32(FPGAVERSION_ADD+0x9000); 
//old version <=0xab
if(l0ver>0xab){
  printf("New L0functions 3/4 (incorect -write only, we cannot read): \n");
  w32 lut[0xfff+1];
  getL0FUN34(lut);
  //for(i=0;i<0xfff+1;i++){
  for(i=0;i<0xf+1;i++){
   printf("0x%x \n",lut[i]);
   //if(i+1%64)printf("\n");
  }
  printf("\n");
}else{
  printf("L0FUN34 not available: L0 version=0x%x\n",l0ver);
}
}

/* FGROUP L0  
An example of output:
0x20c49b RND1/2
0x0
0x7cf    BC1/2
0x0
0x8080   INTfun1/2/T
0xc0c0
0x0
0x0      L0fun1/2
0x0
0x8      L0_INTERACTSEL & 0x1f
0x1      (L0_INTERACTSEL >>5) & 0x1f
0x1      All/Rare flag
*/
void getShared() {
w32 w;
if(notInCrate(1)) return;
w= vmer32(getLM0addr(RANDOM_1)); printf("0x%x\n", w);
w= vmer32(getLM0addr(RANDOM_2)); printf("0x%x\n", w);
w= vmer32(getLM0addr(SCALED_1)); printf("0x%x\n", w);
w= vmer32(getLM0addr(SCALED_2)); printf("0x%x\n", w);
w= vmer32(getLM0addr(L0_INTERACT1)); printf("0x%x\n", w);
w= vmer32(getLM0addr(L0_INTERACT2)); printf("0x%x\n", w);
w= vmer32(getLM0addr(L0_INTERACTT)); printf("0x%x\n", w);
w= vmer32(getLM0addr(L0_FUNCTION1)); printf("0x%x\n", w);
w= vmer32(getLM0addr(L0_FUNCTION2)); printf("0x%x\n", w);
w= vmer32(getLM0addr(L0_INTERACTSEL)); 
printf("0x%x\n", w&0x1f); printf("0x%x\n", ((w>>5)&0x1f));
w= vmer32(getLM0addr(ALL_RARE_FLAG)); printf("0x%x\n", w&0x1);
}
/*
Read 4 LUTs LUT31/2 LUT41/2 from shared memory (they are not accessible
from hw).
Stdout:
lutout=1: and shared memory found: 2+4096 characters string +\n:
  0xfa1e...\n
  i.e.:
          LUTn
  1111    4321
  1010    4321
  0001    4321
  1110    4321
the ame returned in 4 strings:
lutout=4: and shared memory found: four (2+1024 characters+\n) strings:
  0xa... =1010 LUT1 = LUT31
  0xd... =1101 LUT2 = LUT32
  0x9... =1001 LUT3 = LUT41
  0xd... =1101 LUT4 = LUT42

Errors: printed to stdout:
Error error-text
*/
void getSharedL0f34(int lutout) {
int is; unsigned char *l0f34;
/* simulate hw:
  init only if first 4 are "dead"
if( (l0f34stat[0]==0xd) &&
    (l0f34stat[1]==0xe) &&
    (l0f34stat[2]==0xa) &&
    (l0f34stat[3]==0xd)) {
  unsigned char lis=0;
  for(is=0;is<LEN_l0f34;is++){
    if(is>=(LEN_l0f34-2)) {
      l0f34stat[is]= 0xa;   // last 2 are aa
    } else {
      l0f34stat[is]= lis;
    };
    lis++; if(lis>15) lis=0;
  };
}; */
//read hw: no way (cannot be read out).
//for(is=0;is<LEN_l0f34;is++)l0f34stat[i]= (vmer32(L0_FUNCTION34)&0xf000)>>12;
l0f34= l0f34stat;
if(lutout==1) {
  printf("0x");
  for(is=0;is<LEN_l0f34;is++){
    printf("%x",l0f34[is]); 
  }; printf("\n");
} else if(lutout==4) {
  int lutix=0; w8 lutc[4];
  w8 lut[4][LEN_l0f34/4];
  for(is=0;is<LEN_l0f34;is= is+4){
    int lix;
    //printf("l0f34[%d]: %x %x %x %x\n", is, l0f34[is], l0f34[is+1], l0f34[is+2], l0f34[is+3] );
    for(lix=0; lix<4; lix++) {
      lutc[lix]= 
        (((l0f34[is+0]>>lix)&0x1)<<3) | (((l0f34[is+1]>>lix)&0x1)<<2) | 
        (((l0f34[is+2]>>lix)&0x1)<<1) | (((l0f34[is+3]>>lix)&0x1)<<0) ;
      //printf(" lutc[%d]:0x%x", lix, lutc[lix]);
    };//printf("\n");
    for(lix=0; lix<4; lix++) {
      lut[lix][is/4]= lutc[lix];
    };
  };
  for(lutix=0; lutix<4; lutix++) {
    printf("0x");
    for(is=0; is<LEN_l0f34/4; is++) {
      printf("%x", lut[lutix][is]);
    }; printf("\n");
  };
} else {
  printf("Error bad input:%d\n", lutout);
};
}
/*FGROUP L0
set rnd1 rnd2 bcsc1 bcsd2 int1 int2 intt L0fun1 L0fun2
*/
void setShared(w32 r1,w32 r2,w32 bs1,w32 bs2,
               w32 int1,w32 int2,w32 intt,w32 l0fun1,w32 l0fun2) {
if(notInCrate(1)) return;
vmew32(getLM0addr(RANDOM_1), r1);
vmew32(getLM0addr(RANDOM_2), r2);
vmew32(getLM0addr(SCALED_1), bs1);
vmew32(getLM0addr(SCALED_2), bs2);
vmew32(getLM0addr(L0_INTERACT1), int1);
vmew32(getLM0addr(L0_INTERACT2), int2);
vmew32(getLM0addr(L0_INTERACTT), intt);
vmew32(getLM0addr(L0_FUNCTION1), l0fun1);
vmew32(getLM0addr(L0_FUNCTION2), l0fun2);
}
/*FGROUP L0
set INTERACTSEL ALL_RARE_FLAG
*/
void setShared2(w32 intsel, w32 allrare) {
if(notInCrate(1)) return;
vmew32(getLM0addr(L0_INTERACTSEL), intsel);
vmew32(getLM0addr(ALL_RARE_FLAG) , allrare);
}
/* set L0f34 in hw. 
Input:
1. set/reset alll bits for all 4 LUTs (lutn:0):
setL0f34c(0, "0") -set all bits to 0   (i.e. when 1 char string on input)
setL0f34c(0, "f") -set all bits to 1

2. set all 4 LUTs bits (lutn:0) in shared mem +hw:
0, "abcdef..." LEN_l0f34=4096 hexa digits, each hexa digit represents LUT4..1 

3. set LUT bits for 1 LUT (lutn=1,2,3,4 -> LUT31 LUT32 LUT41 LUT42):
in shared memory +hw
1,"abcd..." LEN_l0f34/4=1024 hexa digits

rc:0 ok, rc>0: error: bad string on input, or bad lutn input */
int setL0f34c(int lutn, char *m4) {
int is;
if(lutn==0) {   // all 4 LUTs operations:
  if((strcmp(m4,"0")==0) || (strcmp(m4,"f")==0)) {
    w8 bits;
    bits=  hex12int(m4[0]);
    for(is=0;is<LEN_l0f34;is++){
      l0f34stat[is]= bits;
      vmew32(L0_FUNCTION34, ((bits<<12) | is));
      //vmew32(L0_FUNCTION3, (((bits&0x3)<<12) | is));
      //vmew32(L0_FUNCTION4, (((bits&0xc)<<10) | is));
    };
  } else {  // should be 4096 chars string
    if(strlen(m4)!=LEN_l0f34) return(254);
    for(is=0;is<LEN_l0f34;is++){
      w8 bits;
      bits= hex12int(m4[is]);
      if(bits>0xf) return(255);
      l0f34stat[is]= bits;
      vmew32(L0_FUNCTION34, ((bits<<12) | is));
      //vmew32(L0_FUNCTION3, (((bits&0x3)<<12) | is));
      //vmew32(L0_FUNCTION4, (((bits&0xc)<<10) | is));
    };
  };
} else if((lutn>=1) and (lutn<=4)) {   // one LUT operations:
  if(strlen(m4)!=LEN_l0f34/4) return(254);
  return(250); // not implemented yet
  // put m4 -> 4096 shm-bits in lutn check if at least 1 change:
  for(is=0;is<LEN_l0f34/4;is++){
    w8 bits; int isxbase,isx;
    bits= hex12int(m4[is]);   // 0xf..0x0
    if(bits>0xf) return(255);
    isxbase= 4*is;                // check/update 4 rows from here
    for(isx=0;isx<4;isx++){
      w8 rowbit,shmbit,shmval,rowbitmask,bitval;
      rowbitmask= 1<<isx;
      bitval= bits & (1<<isx);
      rowbit= bitval >> isx;   // bit 0,1,2,3
      shmval= l0f34stat[isxbase+isx];
      shmbit= (shmval & rowbitmask) >> isx;
      // now in bit 0 of rowbit/shmbit is new/old value
      if(rowbit!=shmbit) {
        w32 newval;
        //change=1;
        newval= (shmval & (~rowbitmask)) | bitval;
        l0f34stat[isxbase+isx]= newval;
        vmew32(L0_FUNCTION34, (((isxbase+isx)<<12) | newval));
        //vmew32(L0_FUNCTION3, (((bits&0x3)<<12) | is));
        //vmew32(L0_FUNCTION4, (((bits&0xc)<<10) | is));
      };
    };
  };
} else {
  return(253);
};
return(0);
};
void setSharedL0f34() {
int rc,strl; char m4[LEN_l0f34+3];  //enough LEN_l0f34+1 because
// this routine is casued with (hexnum without '0x' +NL) on stdin
fgets(m4,LEN_l0f34+1,stdin); //MUST BE LIKE THIS (i.e.4096+1)
strl= strlen(m4);
m4[strl]='\0';
//printf("setSharedL0f34:%d\n",strl);   //64 for dbg version with 64 bits
rc=setL0f34c(0,m4);
}
/* Combine luts (from .pcfg file) into 4096 chars (input for setL0f34c(0,str4096))
See:
- ctpcfg.py Ctpconfig.writeShared()  writing 4x4096 bits
- savepcfg() 'L0F34 ' in parted.py
I: lut34[0 or 1]: 2x1024 4bits (each in unsigned char), see Tpartition.h TRBIF.lut34
O: m4: LEN_l0f34+1 chars array
*/
void combine34(w8 *lut34, char *m4) {
int ix,ixa,ixd;
//for(ix=0; ix<LEN_l0f34/4; ix++) {
for(ix=(LEN_l0f34/4-1); ix>=0; ix--) {   // ix: 1023..0
  int base,bases[4],lutn;
  int iy;
  ixd= ix*4+3;                      // descending: 4095, 4091...
  ixa= ((LEN_l0f34/4-1) - ix)*4;  // ascending   0,4..4095
  //for(base=0; base<LEN_l0f34; base=base+LEN_l0f34/4) {
  base=0;
  for(lutn=0; lutn<4; lutn++) {
    bases[lutn]= base+ix;  // lut1234 positions in rbif.lut34
    base= base+ LEN_l0f34/4;
  };
  for(iy=0; iy<4; iy++) {
    int sh,ii; unsigned char l4bits; char l41;
    sh= 1<<iy;
    l4bits=0;
    for(ii=0; ii<4; ii++) {   
    //for(ii=3; ii>=0; ii--) {   // the same
      l4bits= l4bits | (((lut34[bases[ii]] & sh) >>iy ) << ii);
    };
    if(l4bits<0xa) {
      l41= l4bits+'0';
    } else {
      l41= l4bits-0xa+'a';
    }; 
    //m4[ixa]= l41; ixa++ ; //printf("ixa:%d %x=%c\n", ixa-1, l4bits, l41);
    m4[ixd]= l41; ixd-- ; //printf("ixa:%d %x=%c\n", ixd+1, l4bits, l41);
  };
}; m4[LEN_l0f34]='\0';
printf("combine34:len:%d:%s\n", strlen(m4), m4);
}

