/* SSM routines -used by SSMbrowser */

#include <stdio.h>
#include <string.h>
#include "vmewrap.h"

#include "ctp.h"
#include "ssmctp.h"

/*------------------------------------------ ssmbrowser.c routines: */
/*FGROUP DbgSSMBROWSERcalls
return the names+modes of SSMs for present boards:
stdout:
name1 mode1 
name2 mode2 
...
mode -mode of the ssm or:
      _nomode if sms[ix].mode is epmty string
      notin  board is not in the crate
      nossm  if board or sms[ix].sm==NULL
      flag = 1 include 4 ssm for browser,exclude lm0
      flag = 0 only lm0 for ssmcontrol
*/
void gettableSSM(int flag) {
int ix; 
char mode[MAXSSMMODE];
for(ix=0; ix<NSSMBOARDS; ix++) {
/*  printf("ix:%d\n",ix); */
  if((ix<NCTPBOARDS) && (ctpboards[ix].vmever==NOTINCRATE)) {
      strcpy(mode, "notin");
  } else {   /* CTPboard in crate or LTU ot (test/none) or lm0_**/
     /*  printf(ol,"%s %s_%s\n",ol, sms[ix].name, sms[ix].modeSW); */
    // LTU 
    //printf("ix %i %p %c\n",ix,sms[ix].sm,sms[ix].mode[0]);
    if(sms[ix].mode[0]=='\0') {
      strcpy(mode, "_nomode");
    } else {
      strncpy(mode, sms[ix].mode, MAXSSMMODE-1); mode[MAXSSMMODE-1]='\0';
    };
    if(sms[ix].sm==NULL) {
      strcpy(mode, "nossm");
    };
    //if((ix>=NCTPBOARDS) && (sms[ix].ltubase[0]!='0') && (ix != (NCTPBOARDS+4))) {
    //  strcpy(mode, "notin");
    //};
    if((ix>=NCTPBOARDS) && (ix<(NCTPBOARDS+4)) && (sms[ix].ltubase[0] != '0')) strcpy(mode,"notin");
    if(ix==(NCTPBOARDS+4))strcpy(mode,"notin");
    if(ix==(NCTPBOARDS+5))strcpy(mode,"notin");
  };
  bool lm0=strcmp(sms[ix].name,"lm0")==0;
  bool lm0_x=strncmp(sms[ix].name,"lm0_",4)==0;
  if((flag==0) && lm0_x) continue;  
  if((flag==1) && lm0  ) strcpy(mode,"nossm");
  printf("%s %s\n", sms[ix].name, mode);
};
}

/*FGROUP DbgSSMBROWSERcalls
return line:
highest_syncflag n1 n2...
n1,n2 -numbers of items (indexes into sms[])
*/
void getsfSSM(int board) {
printf("%d\n", sms[board].syncflag);
};
/*FGROUP DbgSSMBROWSERcalls
return line:
highest_syncflag n1 n2...
n1,n2 -numbers of items (indexes into sms[])
*/
void getsyncedSSM() {
int ix,maxsf;
int ilm0=0;
char ol[18*NSSMBOARDS]="";
/* find highest syncflag: */
maxsf=0;
for(ix=0; ix<NSSMBOARDS; ix++) {
  if((strcmp(sms[ix].name,"lm0")==0))ilm0=ix;
  if(sms[ix].syncflag>maxsf) {
    maxsf= sms[ix].syncflag;
  };
};
sprintf(ol,"%d",maxsf);
for(ix=0; ix<NSSMBOARDS; ix++) {
  //printf("get synced: %i %i \n",ix,sms[ix].syncflag);
  if(sms[ix].syncflag==maxsf) {
    //if(ix<ilm0)sprintf(ol,"%s %d",ol, ix);
    //else if(ix>ilm0)sprintf(ol,"%s %d",ol, ix-1);
    sprintf(ol,"%s %d",ol, ix);
  };
  if(strlen(ol)>18*(NSSMBOARDS-2)) {
    printf("getsyncedSSM internal ERROR: short ol\n"); 
    break;
  };
};
printf("%s\n",ol);
}
/*FGROUP DbgSSMBROWSERcalls
Extract 1 signal to stdout:
Input:
board:   (0...) according to sms global array
bit:     SSM bit (0-31)
frombc: bc number. 
         0 corresponds to word with address sms[board].offset
bits:    number of bits to be examined (but don't print more then
         102 lines)
Output:
value_of_the_1st_bit      or <0 if error
bit_number_for_which_value_changed
bit_number_for_which_value_changed
...
Errors:
-1 -> required SSM not read
*/
void getsigSSM(int board, int bit, int frombc, int bits) {
int adr,adr1,adr2,adrprint,rc=0; w32 curvalbin;
w32 *ssmbase;
char curval;
adr1=frombc+sms[board].offset; 
/*adr2= adr1+ bits -1; */
adr2=Mega;
ssmbase= sms[board].sm;
//printf("board=%i, ssmbas=%p \n",board,ssmbase);
if(ssmbase==NULL) {
  printf("-1\n"); return;
};
if(ssmbase[adr1] & (1<<bit)) {
  curval='1'; curvalbin=1;
} else {
  curval='0'; curvalbin=0;
};
printf("%c\n", curval); rc++;
adrprint=1;
for(adr=adr1+1; adr<=adr2; adr++) {
  if(((ssmbase[adr]>>bit) & 1) != curvalbin) {
    printf("%d\n", adrprint); rc++;
    /*    if(rc>bits+1) break; */
    if(rc>102) break;
    if(curvalbin) {
      curval='0'; curvalbin=0;
    } else {
      curval='1'; curvalbin=1;
    };
  };
  adrprint++;
};
/*printf("\n"); rc++; */
/* rc: number of lines printed to stdout */
}
/*FGROUP DbgSSMBROWSERcalls
Find signal change.
Input:
board,bit,frombc: as in getsigSSM()
Output (on stdout):
-1 -signal does not change (or memory not accessible)
n  - pointing to the last bit with the same value, next bit
     is different
*/
void finddifSSM(int board, int bit, int frombc) {
int adr,adr1,adr2,adrprint; w32 curvalbin;
w32 *ssmbase;
char rctxt[12]="-1";
ssmbase= sms[board].sm;
if((ssmbase==NULL) || (frombc>=(Mega-1))) {
  printf("-1\n"); return;
};
adr1=frombc+sms[board].offset; adr2= Mega -1;
if(ssmbase[adr1] & (1<<bit)) {
  curvalbin=1;
} else {
  curvalbin=0;
};
adrprint=frombc+1;
for(adr=adr1+1; adr<=adr2; adr++) {
  if(((ssmbase[adr]>>bit) & 1) != curvalbin) {
    sprintf(rctxt, "%d", adrprint-1);
    break;
  };
  adrprint++;
};
printf("%s\n", rctxt);
}
/*FGROUP DbgSSMBROWSERcalls
print sms[board].offset
*/
void getoffsetSSM(int board) {
printf("%d\n",sms[board].offset);
}
/*FGROUP DbgSSMBROWSERcalls
set sms[board].offset
*/
void setoffsetSSM(int board, int newoffset) {
sms[board].offset= newoffset;
}
/*FGROUP DbgSSMBROWSERcalls
set sms[board].mode, ltubase
board: 0..  index into sms[]
newmode: file name in CFG/ctp/ssmsigs without .sig suffix
ltubase: valid only for ltu (board>10)
*/
void setmodeSSM(int board, char *newmode, char *ltubase) {
strcpy(sms[board].mode, newmode);
if(board>10) {
  strcpy(sms[board].ltubase, ltubase);
};
}
/*FGROUP DbgSSMBROWSERcalls
set sms[board].offset
*/
void printsms() {
int i; char ssm[5];
printf("        name         mode SSM ltubase       offset   synf\n");
for(i=0; i<NSSMBOARDS; i++) {
  if(sms[i].sm==NULL) {
    strcpy(ssm,"NULL");
  } else {
    strcpy(ssm,"--->");
  };
  printf("%2d: %10s %10s %4s %9s %10d %4d\n",i,
    sms[i].name,
    sms[i].mode,
    ssm,
    sms[i].ltubase,
    sms[i].offset,
    sms[i].syncflag
    );
};
}


