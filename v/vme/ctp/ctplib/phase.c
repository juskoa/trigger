#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    /* usleep */
#include "vmewrap.h"
#include "ctp.h"
#include "ctplib.h"
#include "Tpartition.h"
#define bit0_7 (1+2+4+8+16+32+64+128)
#define stable 10


/* FGROUP ADCtools
Reads ADC checking for busy and timeout.
*/
int readadc(int board) {
#define timeout 1000
 //int i=0;
 int value;
 w32 boardoffset;
 boardoffset=BSP*ctpboards[board].dial;
 //printf("boardoffset=%i \n",boardoffset);
 vmew32(boardoffset+ADC_START,0x0);
 /*while(1) {   // BUSY -should not be longer than 170micsecs
   if(i>timeout) {
   };
   if(((vmer32(ADC_DATA)&bit8) == 0))
   i++;
 }; */ 
 usleep(10000);
  value = (vmer32(boardoffset+ADC_DATA)&bit0_7);
 /*printf("adc= %i\n",value);*/
 return value; 
}
/* FGROUP ADCtools
Reads ADC using readadc and checking that two subsequent values are the same.
*/
int readadc_s_orig(int board)
{
 int value0,value1,i=0;
 value0=readadc(board);
 //return value0;
 if(value0 == -1) return -1;
 while( ( (value1=readadc(board)) != value0) && (i<stable) ){
     if(value1 == -1) return -2;		 
     value0=value1;
     i++;
    }
 /*printf("i=%i value=%i \n",i,value0);*/   
 if(i>=stable) return -3;    
 return value1;
} 
/* FGROUP ADCtools
Reads ADC using readadc:
- read 10x (stable)
- print all values read
- checking that last two values are the same.
rc: -2: error in readadc
    -3: last 2 values not the same
    >0: last value read
*/
int readadc_s(int board)
{
 int rc,value0,value1,ix=0; char line[100]="";
 value0=readadc(board);
 //return value0;
 if(value0 == -1) return -1;
 sprintf(line, "Board %d adcs:%d", board, value0);
 for(ix=0; ix<stable; ix++) {
   value1=readadc(board);
   sprintf(line, "%s %d", line, value1);
   if(value1 == -1) {rc= -2; break;};
   //if(value1 != value0) {rc= -3; } else { rc= value1; };
   rc= value1;
   value0=value1;
 };
 /*printf("i=%i value=%i \n",i,value0);*/   
 printf("%s.\n", line);
 return rc;
} 
int phaseLx(int board){
 w32 offset,value;
 offset=BSP*ctpboards[board].dial;
 if((board==1) && (l0C0())) {
   vmew32(offset+ADC_SELECTlm0,52);   // was 28 on L0 board
 }else {
   vmew32(offset+ADC_SELECT,28);
 };
 value = readadc_s(board);
 //printf("BOARD L%i: adc=%i \n",board-1,value);
 return(value);
}
int phaseBUSY(){
 w32 offset,value;
 // set toggle generation on busy board via test cluster on backplane
 vmew32(BUSY_CLUSTER,1<<24);
 // measure adc on L0 board
 offset=BSP*ctpboards[1].dial;
 if(l0C0()) {
   vmew32(offset+ADC_SELECTlm0,53);   // was 29 on L0 board
 } else {
   vmew32(offset+ADC_SELECT,29);
 };
 value = readadc_s(1);
 //printf("BOARD BUSY: adc=%i \n",value);
 // toggle off
 vmew32(BUSY_CLUSTER,0);
 return(value);
}
int phaseINT(){
 w32 offset,value,valueb;
 // set toggle generation on int board via ctp_busy line
 valueb=vmer32(INT_DISB_CTP_BUSY);
 valueb =  valueb | 0x10;
 vmew32(INT_DISB_CTP_BUSY,valueb);
 // set toggle generation on busy board via test cluster on backplane
 vmew32(BUSY_CLUSTER,2<<24);
 // measure adc on L0 board
 offset=BSP*ctpboards[1].dial;
 if(l0C0()) {
   vmew32(offset+ADC_SELECTlm0,53);
 } else {
   vmew32(offset+ADC_SELECT,29);
 };
 value = readadc_s(1);
 //printf("BOARD INT: adc=%i \n",value);
 // toggle off on busy 
 vmew32(BUSY_CLUSTER,0);
 // toggle off on int
 valueb =  valueb & ~(0x10);
 vmew32(INT_DISB_CTP_BUSY,valueb);
 return(value);
}

void checkPhases(char *line){
 w32 value; int ival;
 line[0]='\0';
 value=vmer32(BUSY_DELAY_ADD);
 vmew32(BUSY_DELAY_ADD,0);
 ival=phaseLx(1); sprintf(line,"%d", ival);
 ival= phaseLx(2); sprintf(line,"%s %d",line, ival);
 ival= phaseLx(3); sprintf(line,"%s %d",line, ival);
 ival= phaseBUSY(); sprintf(line,"%s %d",line, ival);
 ival= phaseINT(); sprintf(line,"%s %d",line, ival);
 vmew32(BUSY_DELAY_ADD,value);
}
void checkPhasesPrint(){
char line[80];
checkPhases(line); printf("L0 L1 L2 BUSY INT: %s\n",line);
}
void resetPLLS(){
 int i;
 for(i=0;i<NCTPBOARDS;i++){
   if(notInCrate(i)) continue;
   vmew32(BSP*ctpboards[i].dial+PLLreset, DUMMYVAL);
    //printf("PLL reset: board %i 0x%x \n",i,BSP*ctpboards[i].dial+PLLreset);
 }
}

void findToggle(char *toggling_dets) {
//void findToggle() {
int ix,ibit; w32 FOtestcluster, ssmenable;
//char toggling_dets[MAXDETNAME*NDETEC+1];

toggling_dets[0]='\0';
for(ix=FO1BOARD; ix<=FO1BOARD+5; ix++) {
  if(notInCrate(ix)) continue;
  //FOcluster= vmer32(FO_CLUSTER+BSP*ctpboards[ix].dial);
  FOtestcluster= vmer32(FO_TESTCLUSTER+BSP*ctpboards[ix].dial);
  ssmenable= vmer32(SSMenable+BSP*ctpboards[ix].dial);
  //X   : bits[31..28]: toggle signal on connector 4..1
  if((ssmenable& 0x1)==1) {
    printf("fo: SSMenable[0]:1\n"); continue;
  };
  for(ibit=28; ibit<=31; ibit++) {
    if(FOtestcluster & (1<<ibit)) {
      int rc, detix; char name[MAXDETNAME];
      printf("Toggle on fo:%d con:%d\n", ix, ibit-27);
      rc= Connector2Detector((ix-FO1BOARD)+1, ibit-27, &detix);
      if(rc==0) {
        strcpy(name, validLTUs[detix].name);
      } else {
        strcpy(name, "INTERROR");
      };
      sprintf(toggling_dets,"%s %s", name, toggling_dets); 
    };
  };
};
//printf("Toggling dets:%s:\n", toggling_dets);
}
void printToggle() {
char toggling_dets[MAXDETNAME*NDETEC+1];
findToggle(toggling_dets);
printf("%s\n", toggling_dets); 
}
/* Make det's L1 input toggling.
RC: 0:ok 1:not existing LTU or bad onoff   2: SSM not in right state 
*/
int Toggle(char *det, int onoff) {
int fo, focon;
w32 foword, ssmenable;
Tdetector *detvl;
detvl= findLTU(det);
if(detvl == NULL) {
  return(1);
};
fo= detvl->fo+FO1BOARD-1;
focon= detvl->foc; // 1..4
foword= vmer32(FO_TESTCLUSTER+BSP*ctpboards[fo].dial);
ssmenable= vmer32(SSMenable+BSP*ctpboards[fo].dial);
if((ssmenable& 0x1)==1) {
  return(2);
};
if(onoff==1) {
  foword= foword | (1<<(27+focon));
} else if(onoff==0) {
  foword= foword & (~(1<<(27+focon)));
} else {return(1);};
vmew32(FO_TESTCLUSTER+BSP*ctpboards[fo].dial, foword);
return(0);
}

