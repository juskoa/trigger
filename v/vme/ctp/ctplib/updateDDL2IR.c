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
    1 error, not updated
*/
int updateDDL2IR(char *line) {
enum Ttokentype token;
int ixt, ix, lineparsix;
w32 inps24_1=0, inps48_25=0;   // 0: input disabled for IR
char value[MAXCTPINPUTLENGTH];
static char archline[250]="";   // archive line here when ctp_proxy started
if(strlen(line)>0){
  strncpy(archline, line, 250); archline[249]='\0';
};
ix=0; token= nxtoken(archline, value, &ix);
if(strcmp(value,"DDL2_IR")!=0) return(1);
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
    continue;
  };
  if(validCTPINPUTs[i48].level!=0) {
    printf("ERROR input %s not L0 level in ctpinputs.cfg\n", value);
    continue;
  };
  if(validCTPINPUTs[i48].inputnum==0) {   // not connected
    printf("ERROR input %s not connected in ctpinputs.cfg\n", value);
    continue;
  };
  swin= validCTPINPUTs[i48].switchn;
  if(swin==0) {   // not connected
    printf("ERROR input %s not wired in ctpinputs.cfg\n", value);
    continue;
  };
  if(((validCTPINPUTs[i48].dimnum>>24)&FLG_FILTEREDOUT) == FLG_FILTEREDOUT) {
    printf("Warning input %s filtered out\n", value);
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
return(0);
}
