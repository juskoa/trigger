/* VME FPGA registers
Notes:
VME FPGA: 0-3c   -should have the same meaning for any board
         40-7c
LTU FPGA:80-bc   -reserved
         c0-     
"%x"%(4*0x64)    -transl. from Pedja's address (0x64) to VME addr(0x190)-
                  SUBBUSY_TIMER
*/
/*REGSTART32 */
/* VME FPGA: */
#define CODE_ADD      0x4     /* board type (0x56 for LTU) */
#define SERIAL_NUMBER 0x8     /* unique serial number  of the board */
#define VERSION_ADD   0xC     /* VME FPGA firmware version */
#define SOFT_RESET    0x28
/*REGEND */

/* following symb. names are not visible from GUI stdfuncs/vmerw ... */
#define FlashAddClear 0x48           /*  12  Flash memory */ 
#define FlashAccessIncr 0x40         /*  10  */
#define FlashAccessNoIncr 0x44       /*  11  */
#define FlashStatus 0x4c             /*  13  */
#define ConfigStart 0x54             /*  15  */ 
#define ConfigStatus 0x50            /*  0x40 -> FPGA CRC error (SEU)  */

/*REGSTART32 */
#define TEMP_START    0x58   /* LTU temperature: */
#define TEMP_STATUS   0x5c
#define TEMP_READ     0x60

/*REGEND */

#define DUMMYVAL 0xffffffff   /* recommended for DUMMY writes */
