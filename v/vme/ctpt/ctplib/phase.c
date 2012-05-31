#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    /* usleep */
#include "vmewrap.h"
#include "ctp.h"
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
 /*while(1) {
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
int readadc_s(int board)
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
void phaseLx(int board){
 w32 offset;
 offset=BSP*ctpboards[board].dial;
 vmew32(offset+ADC_SELECT,28);
 readadc_s(board);
}
/*FGROUP Simple Tests
*/
void checkPhases(){
 phaseLx(1);
 phaseLx(2);
 phaseLx(3);
}
