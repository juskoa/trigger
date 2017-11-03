#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vmewrap.h"
#include "infolog.h"
#include "ctp.h"
#include "ctplib.h"

#include "Tpartition.h"
#include "lexan.h"
#include "vmeblib.h"
/* line: take it + archive or (if "") use archived value
rc: 0 ok, updated
    1 errors, updated
    2 serious error, not updated
*/
int updateDDL2IR(char *line) {
enum Ttokentype token;
int ixt, ix, lineparsix, rcc=0;;
w32 inps24_1=0, inps48_25=0;   // 0: input disabled for IR
char value[MAXCTPINPUTLENGTH];
static char archline[250];   // see reatTables.c
if(strlen(line)>0){
  // when ctpproxy restarted (ctp.cfg was read and DDL2_IR stored in archline):
  strncpy(archline, line, 250); archline[249]='\0';
} else {
  ; // just go over the same DDL2_IR line (read earlier):
};
ix=0; token= nxtoken(archline, value, &ix);
if(strcmp(value,"DDL2_IR")!=0) return(2);
lineparsix= ix;
for(ixt=0; ixt<48; ixt++) {
  int i48,swin;
  token=nxtoken1(archline, value, &ix);
  if(token==tEOCMD) break;
  if(strcmp(value,"ALL")==0) {
    inps24_1=0xffffff; inps48_25=0xffffff;
    break;
  };
  i48= findInputName(value);
  if(i48==-1) {   // not found
    printf("ERROR input %s not found in ctpinputs.cfg\n", value);
    rcc=1;
    continue;
  };
  if(validCTPINPUTs[i48].level!=0) {
    printf("ERROR input %s not L0 level in ctpinputs.cfg\n", value);
    rcc=1;
    continue;
  };
  if(validCTPINPUTs[i48].inputnum==0) {   // not connected
    printf("ERROR input %s not connected in ctpinputs.cfg\n", value);
    rcc=1;
    continue;
  };
  swin= validCTPINPUTs[i48].switchn;
  if(swin==0) {   // not connected
    printf("ERROR input %s not wired in ctpinputs.cfg\n", value);
    rcc=1;
    continue;
  };
  if(((validCTPINPUTs[i48].dimnum>>24)&FLG_FILTEREDOUT) == FLG_FILTEREDOUT) {
    printf("Warning input %s filtered\n", value);
    continue;
  };
  if(swin>24) {
    inps48_25= inps48_25 | (1<<(swin-25));
  } else {
    inps24_1= inps24_1 | (1<<(swin-1));
  };
};
printf("DDL2_IR 48..25 24..1: 0x%x 0x%x %s", inps48_25, inps24_1,
  &archline[lineparsix]);
vmew32(INT_MASK_FOR_INPUTS_1_24, inps24_1);
vmew32(INT_MASK_FOR_INPUTS_1_24+4, inps48_25);
return(rcc);
}
