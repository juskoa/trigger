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
/*
unsigned char l0f34stat[LEN_l0f34] ={0xd,0xe,0xa,0xd};

void getL0FUN34(w32* lut){
 w32 i;
 // toto je zle (write only!, L0FUN34 LUTs cannot be read)
 //for(i=0;i<0xfff+1;i++)lut[i]=vmer32(l0_fun34+4*i)&0xf000;
 for(i=0;i<LEN_l0f34;i++)lut[i]= l0f34stat[i];
} 
*/
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
0x3      LM rnd1
0x3            2
0x3      LM scaled1
0x3               2
*/
void getShared() {
w32 w;
if(notInCrate(1)) return;
if(l0C0()<0xc606) {
  printf("Unexpected firmware version. Exiting.\n");
  return;
};
w= vmer32((RANDOM_1)); printf("0x%x\n", w);
w= vmer32((RANDOM_2)); printf("0x%x\n", w);
w= vmer32(LM_RANDOM_1); printf("0x%x\n", w);
w= vmer32(LM_RANDOM_2); printf("0x%x\n", w);
w= vmer32((SCALED_1)); printf("0x%x\n", w);
w= vmer32((SCALED_2)); printf("0x%x\n", w);
w= vmer32(LM_SCALED_1); printf("0x%x\n", w);
w= vmer32(LM_SCALED_2); printf("0x%x\n", w);
//w= vmer32((L0_INTERACT1)); printf("0x%x\n", w);
//w= vmer32((L0_INTERACT2)); printf("0x%x\n", w);
//w= vmer32((L0_INTERACTT)); printf("0x%x\n", w);
w= vmer32((L0_INTERACTSEL)); 
printf("0x%x\n", w&0x1f); printf("0x%x\n", ((w>>5)&0x1f));
w= vmer32((ALL_RARE_FLAG)); printf("0x%x\n", w&0x1);
//w= vmer32((LM_INTERACT1)); printf("0x%x\n", w);
//w= vmer32((LM_INTERACT2)); printf("0x%x\n", w);
//w= vmer32((LM_INTERACTT)); printf("0x%x\n", w);
//w= vmer32((LM_INTERACTSEL)); 
//printf("0x%x\n", w&0x1f); printf("0x%x\n", ((w>>5)&0x1f));
}
void getSharedl0mfs() {
int lutn; char val[LUT8_LEN]; char l0fs[4*LUT8_LEN];
char errmsg[4*100]="";
if(notInCrate(1)) return;
for(lutn=1; lutn<=8; lutn++) {
  /*rc=*/ cshmgetLUT(lutn, val);
  if(lutn<=4) {
    printf("%s\n", val);
    strcpy(&l0fs[(lutn-1)*LUT8_LEN], val);
  } else {
    if(strcmp(val, &l0fs[(lutn-5)*LUT8_LEN])!=0) {
      sprintf(errmsg, "%slmf%d != l0f%d\n", errmsg,
        lutn-4, lutn-4);
    };
  };
}; printf("%s", errmsg);
};
void getSharedintl0mfs() {
int lutn; char val[LUT8_LEN]; char l0fs[3*LUT8_LEN];
char errmsg[3*100]="";
if(notInCrate(1)) return;
for(lutn=1; lutn<=6; lutn++) {
  /*rc=*/ cshmgetintLUT(lutn, val);
  if(lutn<=3) {
    printf("%s\n", val);
    strcpy(&l0fs[(lutn-1)*LUT8_LEN], val);
  } else {
    if(strcmp(val, &l0fs[(lutn-4)*LUT8_LEN])!=0) {
      sprintf(errmsg, "%slmf%d != l0f%d\n", errmsg,
        lutn-3, lutn-3);
    };
  };
}; printf("%s", errmsg);
};
/*FGROUP L0
set rnd1 rnd2 bcsc1 bcsd2 int1 int2 intt L0fun1 L0fun2
*/
void setShared(w32 r1,w32 r2,w32 bs1,w32 bs2) {
if(notInCrate(1)) return;
vmew32((RANDOM_1), r1);
vmew32((RANDOM_2), r2);
vmew32((SCALED_1), bs1);
vmew32((SCALED_2), bs2);
}
void setSharedAll(int shr,w32 value)
{
 switch(shr)
 {
  case 0:
	vmew32((RANDOM_1), value); return;
  case 1:
	vmew32((RANDOM_2), value); return;
  case 2:
	vmew32(LM_RANDOM_1, value); return ;
  case 3:
	vmew32(LM_RANDOM_2, value); return ;
  case 4:
	vmew32((SCALED_1), value); return;
  case 5:
	vmew32((SCALED_2), value); return;
  case 6:
	vmew32(LM_SCALED_1, value); return;
  case 7:
	vmew32(LM_SCALED_2, value); return ;
  case 15:
	vmew32(L0_INTERACTSEL, value);
	vmew32(LM_INTERACTSEL, value);
        return;
  case 16:
	vmew32(L0_INTERACTSEL, value);
	vmew32(LM_INTERACTSEL, value);
        return;
  case 17: return;
	vmew32(ALL_RARE_FLAG , value); return;
  default:
        printf("setSharedAll: resource %i not allowed. \n",shr);
        return ;
 }
 return;
}
void setShared_Old(w32 r1,w32 r2,w32 bs1,w32 bs2,
               w32 int1,w32 int2,w32 intt,w32 l0fun1,w32 l0fun2) {
if(notInCrate(1)) return;
vmew32((RANDOM_1), r1);
vmew32((RANDOM_2), r2);
vmew32((SCALED_1), bs1);
vmew32((SCALED_2), bs2);
vmew32((L0_INTERACT1), int1);
vmew32((L0_INTERACT2), int2);
vmew32((L0_INTERACTT), intt);
if(l0C0()>=0xc606) {
  ;  // l0fun1/2 dummy -see setShared4
} else {
  //vmew32((L0_FUNCTION1), l0fun1);
  //vmew32((L0_FUNCTION2), l0fun2);
  printf("Unexpected firmware version. Exiting.\n");
  return;
};
}
/*FGROUP L0
set INTERACTSEL ALL_RARE_FLAG lmrnd1 lmrnd2 lmbcd1 lmbcd2
*/
void setShared2(w32 intsel, w32 allrare) {
if(notInCrate(1)) return;
vmew32((L0_INTERACTSEL), intsel);
vmew32((LM_INTERACTSEL), intsel);
vmew32((ALL_RARE_FLAG) , allrare);
}
void setShared3(w32 lmrnd1, w32 lmrnd2, w32 lmbcd1, w32 lmbcd2) {
vmew32(LM_RANDOM_1, lmrnd1);
vmew32(LM_RANDOM_2, lmrnd2);
vmew32(LM_SCALED_1, lmbcd1);
vmew32(LM_SCALED_2, lmbcd2);
}

#define LEN_LUT8 256   // bits
void setLUT1(int lutn, char *m4) {
w32 lutadr; int is;
lutadr= getLM0_F8ad(lutn);
/* put if m4 taken as 16x16bits words, put them into LUT this way:
m4[15]
m4[14]
...
m4[0]  -i.e. last 4 hex-digits from m4
*/
if((strcmp(m4,"0")==0) || (strcmp(m4,"1")==0) || (strcmp(m4,"0x0")==0)) {
  w32 lutw;
  if((strcmp(m4,"0")==0) || (strcmp(m4,"0x0")==0)) { lutw=0;
    cshmsetLUT(lutn, "0x0000000000000000000000000000000000000000000000000000000000000000");
  } else { lutw=0xffff0000;
    cshmsetLUT(lutn, "0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
  };
  for(is=LEN_LUT8/16; is>=1; is--){   // 16 bits/word in LUT, LUT=16words
    //int isx;
    // isx: pointing to char in m4 (60,56,52,48,44,...,8,4,0)
    //sx= (is-1)*4;
    vmew32(lutadr, lutw|(16-is));
  };
} else {
  for(is=LEN_LUT8/16; is>=1; is--){   // 16 bits/word in LUT, LUT=16words
    w32 lutw; int isx;
    // isx: pointing to char in m4 (60,56,52,48,44,...,8,4,0)
    isx= (is-1)*4;
    lutw= hex4(&m4[isx+2]);
    lutw= lutw<<16 | (16-is);
    vmew32(lutadr, lutw);
  };
  /* update shared memory */
  cshmsetLUT(lutn, m4);
};
}
/*set LUT1..8 in hw. 
Input:
1. set/reset alll bits for all 8 LUTs (lutn:0):
setLUT(0, "0") -set all bits to 0   (i.e. when 1 char string on input)
setLUT(0, "f") -set all bits to 1

2. set all 8 LUTs (l0f1..4+ lmf1..4) bits (lutn:0) in shared mem +hw:
0, "0xabcdef..." each hexa digit represents LUT4..1 

3. set LUT bits for 1 LUT (lutn=1,2,3,4 -> LUT31 LUT32 LUT41 LUT42):
in shared memory +hw
1,"0xabcd..." LEN_LUT8/4=64 hexa digits

rc:0 ok, rc>0: error: bad string on input, or bad lutn input */
int setLUT(int lutn, char *m4) {
if((strlen(m4)!=(LEN_LUT8/4+2)) &&
   (strcmp(m4,"0")!=0) && (strcmp(m4,"1")!=0) && (strcmp(m4,"0x0"))) {
  printf("ERROR setLUT:=%s= len:%d\n", m4, (int)strlen(m4));
  return(254);
};
if(lutn==0) {   // all 8 LUTs operations:
  int lutn1;
  for(lutn1=LM0_F8_MIN;lutn1<=LM0_F8_MAX;lutn1++){
    setLUT1(lutn1, m4);
  };
} else if((lutn>=LM0_F8_MIN) and (lutn<=LM0_F8_MAX)) {   // one LUT operations:
  setLUT1(lutn, m4);
} else {
  return(253);
};
return(0);
};
void setShared4(int X, char *lut) {
int rc;
rc= setLUT(X, lut);
if(rc!=0) {
  printf("Error: LUT%d cannot be loaded, setLUT() rc:%d\n", X, rc);
} else {
  rc= setLUT(X+4, lut);
  if(rc!=0) {
    printf("Error: LUT%d cannot be loaded, setLUT() rc:%d\n", X+4, rc);
  };
};
if(rc==0)printf("LUT%d (l0f%d + lmf%d) loaded\n", X, X, X+4);
//printf("LUT%d (l0f%d + lmf%d) loaded\n", X,X,X);
}
//////////////////////////////////////////////////////////////////
// INT functions
//////////////////////////////////////////////////////////////////
/*
 * setINTLUT1 = setLUT1 but:
 * - different lut address   
 */ 
void setINTLUT1(int lutn, char *m4) {
w32 lutadr; int is;
lutadr= getLM0_INTad(lutn);
//lutadr= getLM0_F8ad(lutn);
/* put if m4 taken as 16x16bits words, put them into LUT this way:
m4[15]
m4[14]
...
m4[0]  -i.e. last 4 hex-digits from m4
*/
if((strcmp(m4,"0")==0) || (strcmp(m4,"1")==0) || (strcmp(m4,"0x0")==0)) {
  w32 lutw;
  if((strcmp(m4,"0")==0) || (strcmp(m4,"0x0")==0)) { lutw=0;
    cshmsetintLUT(lutn, "0x0000000000000000000000000000000000000000000000000000000000000000");
  } else { lutw=0xffff0000;
    cshmsetintLUT(lutn, "0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
  };
  for(is=LEN_LUT8/16; is>=1; is--){   // 16 bits/word in LUT, LUT=16words
    //int isx;
    // isx: pointing to char in m4 (60,56,52,48,44,...,8,4,0)
    //sx= (is-1)*4;
    vmew32(lutadr, lutw|(16-is));
  };
} else {
  for(is=LEN_LUT8/16; is>=1; is--){   // 16 bits/word in LUT, LUT=16words
    w32 lutw; int isx;
    // isx: pointing to char in m4 (60,56,52,48,44,...,8,4,0)
    isx= (is-1)*4;
    lutw= hex4(&m4[isx+2]);
    lutw= lutw<<16 | (16-is);
    //printf("lutn: %i lutadr: 0x%x , 0x%x \n",lutn,lutadr,lutw);
    vmew32(lutadr, lutw);
  };
  /* update shared memory */
  cshmsetintLUT(lutn, m4);
};
}
int setINTLUT(int lutn, char *m4) {
//if((strlen(m4)!=(LEN_LUT8/4+2)) &&
//   (strcmp(m4,"0x0")!=0)) {
   //(strcmp(m4,"0")!=0) && (strcmp(m4,"1")!=0)) {
//  printf("ERROR setINTLUT:=%s= len:%d\n", m4, (int)strlen(m4));
//  return(254);
//};
if((strlen(m4)!=(LEN_LUT8/4+2)) &&
   (strcmp(m4,"0")!=0) && (strcmp(m4,"1")!=0) && (strcmp(m4,"0x0"))) {
  printf("ERROR setINTLUT:=%s= len:%d\n", m4, (int)strlen(m4));
  return(254);
};
if(lutn==0) {   // all 8 LUTs operations:
  int lutn1;
  for(lutn1=1;lutn1<=6;lutn1++){
    setINTLUT1(lutn1, m4);
  };
} else if((lutn>=1) and (lutn<=6)) {   // one LUT operations:
  setINTLUT1(lutn, m4);
} else {
  return(253);
};
return(0);
};
void setSharedINT3(int X, char *lut) {
int rc;
//printf("setSharedINT called \n");
rc= setINTLUT(X, lut);
if(rc!=0) {
  printf("Error: INTLUT%d cannot be loaded, setINTLUT() rc:%d\n", X, rc);
} else {
  // to be changed
  rc= setINTLUT(X-3, lut);
  if(rc!=0) {
    printf("Error: INTLUT%d cannot be loaded, setINTLUT() rc:%d\n", X+4, rc);
  };
};
if(rc==0)printf("INTLUT%d (L0INT%d + lmINT%d) loaded\n", X, X, X+4);
}



