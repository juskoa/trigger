/*  ctpcounters.h */
/* the number of counters per 1 CTP board: */
#define NCOUNTERS_L0 160    //  3 spares
#define NCOUNTERS_L1 160    // 14 spares
#define NCOUNTERS_L2 134    //  1 spare
#define NCOUNTERS_FO  34    // there are 6 FO boards (FO1-FO6)
#define NCOUNTERS_BUSY 160  // was 48 till 28.9.2007. 55 spares now
#define NCOUNTERS_INT 19 
#define NCOUNTERS_SPEC 49   /* 2 + 1 + 22 + 24
 2 words:server unix time in seconds + micseconds
 1 word: Orbit number
 2 words: temperature, volts for BUSY board
 2 words: temperature, volts for L0   board
 2 words: temperature, volts for L1   board
 2 words: temperature, volts for L2   board
 2 words: temperature, volts for INT  board
 2 words: temperature, volts for FO1  board
 2 words: temperature, volts for FO2  board
 2 words: temperature, volts for FO3  board
 2 words: temperature, volts for FO4  board
 2 words: temperature, volts for FO5  board
 2 words: temperature, volts for FO6  board
          temperature: in centigrades (0: if not valid or error)
          volts: 4 values in 4 bytes. See
          'LTU software model', chapter: LTU DC Voltage monitoring
          on ALICE-trigger web page
24 words: volts for 24 LTUs
together: 49 WORDS
*/
#define NCOUNTERS_MAX 160   // max. number of counters on 1 board

#define NCOUNTERS (NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+6*NCOUNTERS_FO+\
        NCOUNTERS_BUSY+NCOUNTERS_INT+NCOUNTERS_SPEC)
// i.e.: 320+134+6*34+48+19+2+23+24=774
//       320+134+6*34+160+19+2+23+24=886
#define CSTART_L0   0
#define CSTART_L1   NCOUNTERS_L0
#define CSTART_L2   (NCOUNTERS_L0+NCOUNTERS_L1)
#define CSTART_FO   (NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2)
#define CSTART_BUSY (NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+6*NCOUNTERS_FO)
#define CSTART_INT  (NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+6*NCOUNTERS_FO+\
        NCOUNTERS_BUSY)
#define CSTART_SPEC  (NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+6*NCOUNTERS_FO+\
        NCOUNTERS_BUSY+NCOUNTERS_INT)

