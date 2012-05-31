/* bcmask.c */
#include <string.h>
//#include <unistd.h>    /* usleep() */
#include <stdio.h>
#include "vmewrap.h"
#include "lexan.h"
#include "ctp.h"
#include "ctplib.h"
#include "Tpartition.h"
#include "infolog.h"
/* read BC masks from HW and print out 3564 4 (or 12)bits words
*/
void getBCmasks() {
int ix, hchars; w32 m4_12; char m4[3*ORBITLENGTH+1];
vmew32(MASK_MODE,1);   /* vme mode */
vmew32(MASK_CLEARADD,DUMMYVAL);
if(l0AB()==0) {   //firmAC
  m4_12= 0xfff; hchars= 3*ORBITLENGTH;
} else {
  m4_12= 0xf; hchars= ORBITLENGTH;
};
if(l0AB()==0) {   //firmAC
  for(ix=0; ix<ORBITLENGTH; ix++) {
    w32 c12,c;
    c12=vmer32(MASK_DATA)&m4_12;
    c= (c12 & 0xf00)>>8;
    if(c>=10) m4[3*ix+0]= c-10+'a'; else m4[3*ix+0]= c+'0';
    c= (c12 & 0x0f0)>>4;
    if(c>=10) m4[3*ix+1]= c-10+'a'; else m4[3*ix+1]= c+'0';
    c= (c12 & 0x00f);
    if(c>=10) m4[3*ix+2]= c-10+'a'; else m4[3*ix+2]= c+'0';
    /*  printf("%x",vmer32(MASK_DATA)&0xf); */
  };
} else {
  for(ix=0; ix<ORBITLENGTH; ix++) {
    w32 c;
    c=vmer32(MASK_DATA)&m4_12;
    if(c>=10) m4[ix]= c-10+'a'; else m4[ix]= c+'0';
  };
};
vmew32(MASK_MODE,0);   /* normal mode */
m4[hchars]='\0';
printf("%s\n",m4);
}
/*--------------------------------------------------------- loadBCmasks()
bcmasks: w16 array of length: ORBITLENGTH.
stdout: message about the number of bytes (and possible errors) written
*/
void loadBCmasks(w16 *bcmasks) {
int ix;
/* abandoned (not char * but w16 *)
int hchars, strl;
if(l0AB()==0) {   //firmAC
  hchars= 3*ORBITLENGTH;
} else {
  hchars= ORBITLENGTH;
};
strl= strlen(bcmasks);
if(strl!= hchars) {
  if(strl==0) {
    printf("loadBCmasks: length of the input string is 0, no action\n");
    return;
  } else {
    int oldstrl= strl;
    char emsg[300];
    if(strl>ORBITLENGTH) strl=ORBITLENGTH;
    sprintf(emsg,"loadBCmasks: bad length:%d, anyhow, the %di[/3 for AC) words will be written to hw",oldstrl, strl);
    infolog_trgboth(LOG_ERROR, emsg);
  };
}; */
vmew32(MASK_MODE,1);   /* vme mode */
vmew32(MASK_CLEARADD,DUMMYVAL);
/* this is shifted by 2. Correct programming of BCmask memory is:
3562, 3563, 0, 1,...
*/
for(ix=0; ix<ORBITLENGTH; ix++) {
  w32 val; w16 c;
  c=bcmasks[ix]; val=c;
  /*von
  if((c>='a') && (c<='f')) val= c - 'a' +10; else val= c-'0';
  if(l0AB()==0) {
    if((ix%3)==0) {val3= val;
    } else if((ix%3)==1) {val3= (val3<<4) | val;
    } else if((ix%3)==2) {val3= (val3<<4) | val; 
      vmew32(MASK_DATA,val3);
    };
  } else {
    //  if(ix<80) printf("%x",val);
    vmew32(MASK_DATA,val);
    //  printf("%x",vmer32(MASK_DATA)&0xf);
  };*/
  vmew32(MASK_DATA,val);
};
vmew32(MASK_MODE,0);   /* normal mode */
//if(DBGmask) printf("loadBCmasks:written to hw:%d words\n",ORBITLENGTH);
if(DBGmask) {
  printf("loadBCmasks:written to hw:%d words\n",ORBITLENGTH);
  for(ix=0; ix<8; ix++) {
  };
};
}

/* set BC masks in HW from input line containing:
 3564   (1char per masks 1-4) hexa-chars.
 3564*3 (3chars per masks 1-12) hexa-chars.
*/
void setBCmasks() {
int ix,c3, il,strl,hchars; char m4[3*ORBITLENGTH+3]; w16 bytes[ORBITLENGTH];
if(l0AB()==0) {   //firmAC
  hchars= 3*ORBITLENGTH;
  c3=3;
} else {
  hchars= ORBITLENGTH;
  c3=1;
};
fgets(m4,hchars+2,stdin);
strl= strlen(m4)-1;   // peel off NL
if(strl!= hchars) {
  printf("loadBCmasks: length of the input string is %d, no action\n", strl);
  return;
};
m4[strl]='\0'; il=0;
for(ix=0; ix<ORBITLENGTH; ix++) {
  w16 hx; char hx3[4];
  strncpy(hx3, &m4[il],c3); hx=hex2int(hx3);
  bytes[ix]= hx; il=il+c3;
};
loadBCmasks(bytes);
}

/* set/read/check ntimes
words: if 0 than checj whole BCmask memory (3564)
*/
void checkBCmasks(int ntimes, int words) {
w32 val,m4_12;
int ix,strl, cycles; //char m4[ORBITLENGTH+3];
if(words==0)
  strl=ORBITLENGTH;
else
  strl=words;
if(l0AB()==0) {   //firmAC
  m4_12=0xfff;
} else {
  m4_12=0xf;
};
for(cycles=0; cycles<ntimes; cycles++) {
  vmew32(MASK_MODE,1);   /* vme mode */
  vmew32(MASK_CLEARADD,DUMMYVAL);
  for(ix=0; ix<strl; ix++) {
    val=ix&m4_12;
    vmew32(MASK_DATA,val);
    /*  printf("%x",vmer32(MASK_DATA)&0xf); */
  };
  vmew32(MASK_MODE,0);   /* normal mode */
  /*  printf("written to hw:%d\n",strl); */
  /* read and check; */
  vmew32(MASK_MODE,1);   /* vme mode */
  vmew32(MASK_CLEARADD,DUMMYVAL);
  for(ix=0; ix<strl; ix++) {
    w32 valr;
    val=ix&m4_12;
    valr= vmer32(MASK_DATA)&m4_12;
    if(val != valr) {
      printf("Error at %d exp:%x got:%x\n",ix,val,valr);
    };
  };
  vmew32(MASK_MODE,0);   /* normal mode */
  printf("read/checked:%d\n",strl);
};
}
