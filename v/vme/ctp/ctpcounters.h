/*  ctpcounters.h */
/* the number of counters per 1 CTP board: */
#define NCOUNTERS_L0 300      //  3 spares. run1:160 run2:262
#define NCOUNTERS_L0_SP1 17   // run1:15 run2: 17..18 (2 spares)
#define NCOUNTERS_L0_SP2 178  // run2: 178..186. not defined in run1.
#define NCOUNTERS_L0_SP3 298  // run2: 298..299 run1: 159

#define NCOUNTERS_L1 300      // run1: 160,14 spares  run2: 300
#define NCOUNTERS_L1_SP1 38   // run1/2: 2 spares
#define NCOUNTERS_L1_SP2 250  // run1: 148, 12spares run2: 250,50spares

#define NCOUNTERS_L2 300      //  run1: 134   run2:300
#define NCOUNTERS_L2_SP1 25
#define NCOUNTERS_L2_SP2 236  // run2 only: 236/64 spares

#define NCOUNTERS_FO  72    // FO1-FO6. run1: 48(+4) run2: 72= 63+9spares
/* not used even in run1:
#define NCOUNTERS_FOae  52   + L2r counter for each connector -stored in spare871..894
*/
#define NCOUNTERS_BUSY 160  // run1:105 +49spares + 6runx  run2: 300, but we read only 160
#define NCOUNTERS_BUSY_SP1 113 //run1:105 run2:113 (187 spares, we read only 47 of them)
//#define NCOUNTERS_BUSY_L2RS 129  // run1 only:129. 6*4 L2r FO counters (FO1, FO2,...)
#define NCOUNTERS_BUSY_TSGROUP 153  // active Time sharing class group
#define NCOUNTERS_BUSY_RUNX1 154  // 6 RUNX counters from here (see dimcoff,dims)
#define BYTIMERSCOUNTN 44      // 43: run2-bytime  39:run1-bytime see busyTools.c
#define NCOUNTERS_BUSY_BYTIME 43

#define NCOUNTERS_INT 19 
#define NCOUNTERS_SPEC 49   /* 2 + 1 + 22 + 24
 0: 2 words:server unix time in seconds + micseconds
 2: 1 word: Orbit number
 3: 2 words: temperature, volts for BUSY board
 5: 2 words: temperature, volts for L0   board
 7: 2 words: temperature, volts for L1   board
 9: 2 words: temperature, volts for L2   board
11: 2 words: temperature, volts for INT  board
13: 2x6 words: temperature, volts for FO1-6  boards
          temperature: in centigrades (0: if not valid or error)
          volts: 4 values in 4 bytes. See
          'LTU software model', chapter: LTU DC Voltage monitoring
          on ALICE-trigger web page
25: 24 words: volts for 24 LTUs
together: 49 WORDS
*/
#define NCOUNTERS_MAX 300   // max. number of counters on 1 board (run1: 160)

#define NCOUNTERS (NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+6*NCOUNTERS_FO+\
        NCOUNTERS_BUSY+NCOUNTERS_INT+NCOUNTERS_SPEC)
// i.e.: 320+134+6*34+48+19+2+23+24=774
//       320+134+6*34+160+19+2+23+24=886
//       320+134+6*48+160+19+2+23+24=970     11.11.2008
#define CSTART_L0   0
#define CSTART_L1   NCOUNTERS_L0
#define CSTART_L2   (NCOUNTERS_L0+NCOUNTERS_L1)
#define CSTART_FO   (NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2)
#define CSTART_BUSY (NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+6*NCOUNTERS_FO)
#define CSTART_INT  (NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+6*NCOUNTERS_FO+\
        NCOUNTERS_BUSY)
#define CSTART_SPEC  (NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+6*NCOUNTERS_FO+\
        NCOUNTERS_BUSY+NCOUNTERS_INT)
#define CSTART_LTUVOLTS CSTART_SPEC+25
#define CSTART_RUNX (CSTART_BUSY+NCOUNTERS_BUSY_RUNX1)
#define CSTART_TSGROUP (CSTART_BUSY+NCOUNTERS_BUSY_TSGROUP)

/* following defined for ctp_proxy.c:
#define SODEODfol0out1 36
#define SODEODfol1out1 40
#define SODEODfol2stro1 44 */
// run2:
#define SODEODfol0out1 47
#define SODEODfol1out1 51
#define SODEODfol2stro1 55

