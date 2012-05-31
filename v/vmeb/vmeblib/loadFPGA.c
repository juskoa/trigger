/* loadFPGA(ix): load CTP/LTU/LVDST boards.
Input:
- VME space already opened
- ix: 0: for LTU/LVDST/FO0 boards.
      0x1000-0x6000 for FO1-6 board
      0x8000 busy   0x9000 L0   0xa000 L1   0xb000 L2   0xc000 INT
*/
#include <stdio.h>
#include <unistd.h>
#include "vmewrap.h"
//#include "ctp.h"
//#define FlashAddClear 0x48           /*  12  Flash memory */ 
//#define FlashAccessIncr 0x40         /*  10  */
#define FlashAccessNoIncr 0x44       /*  11  */
#define FlashStatus 0x4c             /*  13  */
#define ConfigStart 0x54             /*  15  */ 
#define ConfigStatus 0x50            /*  0x40 -> FPGA CRC error (SEU)  */
#define FPGAVERSION_ADD 0x80 /* board's FPGA version */

/*-------------------------*/ int writeFM(w32 reladdr, w32 address, w32 data) {
#define wAttempts 4000
int rc,attempts=0;
vmew32(reladdr+address,data);
/* usleep(4000); */
while (1) {
  w32 sfm;
  sfm= vmer32(reladdr+FlashStatus) & 0x80;
  if( (sfm == 0) && (attempts < wAttempts) ){
    attempts=attempts+1;
  } else {
    break;
  };
};
/*printf("writeFM, attempts:%d\n",attempts); fflush(stdout); */
if(attempts >= wAttempts){
  printf("Unable to write (a:%x data:%x)in Flash Memory after %i attempts \n",
    address,data,wAttempts);
  rc=1;
}else{
  rc=0;
};
fflush(stdout);
return(rc);
}

/*----------------------------------------------*/ int resetFM(w32 reladdr) {
if(writeFM(reladdr, FlashAccessNoIncr,0xF0) !=0 )
  return 1; else return 0;
}

/*--------------------------------------------------*/ w32 loadFPGA(int ix) {
/* configure FPGA on CTP/LTU/LVDST board.
Input:
- vmespace 0 is opened, i.e. from vmebase:
  from 0x820000 for CTP boards or 
  from 0x8x0000 for LTU-x (x is LTU dial)
ix: ix + vmebase -base address of the CTP/LTU board
Output:
rc: fpga version or >0x100 ->unsuccessfull configuration */
 int i=0,stat;
 w32 reladdr, sfpga;
 reladdr= ix;
 if(resetFM(reladdr) == 0){
   sfpga= 0x01 & vmer32(reladdr+ConfigStatus);
   if(sfpga == 0 ){
     vmew32(reladdr+ConfigStart,0x0);
     printf("FPGA configuration started. \n");
     fflush(stdout);
     while( (i<40) && (stat=0x01&vmer32(reladdr+ConfigStatus)) ){
       printf("loadFPGA:FPGA stat = %x \n",stat);
       usleep(10000);
       /* MonitorStatusFPGA(10,10000); */
       i++;
     }
     /* MonitorStatus(ConfigStatus,&bit); 
     if( (bit[2] == 0) || (bit[3]== 0)) return 4; */
     usleep(10000);
     sfpga= vmer32(reladdr+ConfigStatus);
     if( (sfpga & 0xc) != 0xc ) {
       printf("loadFPGA:FPGA ConfigStatus = %x \n",sfpga);
       return 0x104;
     };
     printf("loadFPGA:FPGA status = %x \n",sfpga);
     if( stat==0) {
       sfpga= 0xff & vmer32(reladdr+ FPGAVERSION_ADD);
       return sfpga;
     } else return 0x101;
   }else return 0x103;
 }else return 0x102;
}
