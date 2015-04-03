#ifndef _TTCIT_CONF_H__
#define _TTCIT_CONF_H__

/*
  TTCit logic 
 */


#define SSM_SCOPE   0
#define SSM_SEQ     1

#define MON_RUNNING    0xa
#define MON_STOPPING   0xb
#define MON_STOPPED    0xc

#define SSM_SET_MODE_SCOPE 0x1
#define SSM_SET_MODE_SEQ   0x0

#define MEGA 1048576
#define BFSIZ 4096
#define BCNT_FIFO_SIZ 2048
#define MEGA_PLUS_ONE 1048577

#define EVENT_BUFFER 2 * MEGA + 500

/*
  Code flow flags
 */

#ifdef SIMVME
#define TTCIT_SIM_USE_PTHREADS__
#define TTCIT_SIM_26_MSEC 26000     /* 26 miliseconds in microsec */
/* #define TTCIT_SMALL_SSM__ */        /* Decrease the size of SSM for Debug */
/*
                  Extra debug print at some special places needed for debug
		  If not needed, undef them
 */
#define TTCIT_EXTRA_DBG_PRINT_1__  
#endif

#define TTCIT_TRIGGER_SEQ_FORMAT__  /* Print better stuff */

/*
  Dimensions 
 */

#ifndef TTCIT_SMALL_SSM__
#define TTCIT_MAX_ADDR_SSM     1048573   /* Max address of SSM */
#else
#define TTCIT_MAX_ADDR_SSM     102400     /* Reduce the SSM address range */
#endif
#define TTCIT_MAX_ADDR_BCFIFO  2048    /* Max address of BC FIFO */

#ifdef TTCIT_SSM_VER_NEW_1_
#undef TTCIT_MAX_ADDR_SSM
#define TTCIT_MAX_ADDR_SSM 1048573 /* Different with every firmware version */
#endif

/*
  Trigger words format is put here
 */

#define TTC_ADDR_MASK   0xF000   /* Mask of the TTC address */
#define TTC_ADDR_RSH    12       /* Right shift to get the TTC address */
#define TTC_DATA_MASK   0xFFF    /* Mask of the TTC data */
#define TTC_ORBID_RSH   12       /* Orbit ID shift */

#define TTC_ADDR_L1_H   0x1000   /* L1 header */
#define TTC_ADDR_L1_D   0x2000   /* L1 data */
#define TTC_ADDR_L2A_H  0x3000   /* L2a header */
#define TTC_ADDR_L2A_D  0x4000   /* L2a data */
#define TTC_ADDR_L2R    0x5000   /* L2r message */
#define TTC_ADDR_ROI_H  0x6000   /* RoI header */
#define TTC_ADDR_ROI_D  0x7000   /* RoI data */

/*        L1M Header */

#define TTC_L1H_CIT_MASK      0x100
#define TTC_L1H_CIT_RSH       8
#define TTC_L1H_ROC_MASK      0xf0
#define TTC_L1H_ROC_RSH       4
#define TTC_L1H_ESR_MASK      0x8
#define TTC_L1H_ESR_RSH       3
#define TTC_L1H_L1SWC_MASK    0x4
#define TTC_L1H_L1SWC_RSH     2
#define TTC_L1H_L1CLASS_MASK  0x3

/*       L1M words  */

#define TTC_L1W_L1CLASS_MASK  0xFFF

/*      L2A Header */

#define TTC_L2AH_BCID_MASK    0xFFF


/*      L2A Words */

#define TTC_L2AW_ORBID_MASK   0xFFF
#define TTC_L2AW_ORBID_LSH    12

#define TTC_L2AW_CIT_MASK       0x200
#define TTC_L2AW_CIT_RSH        9
#define TTC_L2AW_L2SW_MASK      0x100
#define TTC_L2AW_L2SW_RSH       8
#define TTC_L2AW_L2CLUST_MASK   0xfc
#define TTC_L2AW_L2CLUST_RSH    2
#define TTC_L2AW_L2CLASS_MASK   0x3

/*
  Number of words for each group of the B channel data
 */

#define TTC_WORDS_L1M_O   4    /* Nr of words after L1m header */
#define TTC_WORDS_L1M_N   8    /* New value, longer message */
#define TTC_WORDS_L2A_O   7    /* Nr of words after L2a header */
#define TTC_WORDS_L2A_N   12   /* Longer message */
#define TTC_WORDS_L2R_O   0    /* Nr of words after L2r */
#define TTC_WORDS_L2R_N   0    /* same */
#define TTC_WORDS_ROI_O   3    /* Nr of words after RoI Header */
#define TTC_WORDS_ROI_N   3    /* same ??? */
#define TTC_WORDS_RES_O   0    /* Nr of words for reserved cases, irrelevant */
#define TTC_WORDS_RES_N   0    /* same ??? */

/*
  Max number of words that can be inna trigger message (unknown type)
 */

#define TTC_MAX_MESSAGE_LEN    1024

/*
  Definitions of 18 bits in SnapShot Memory

  0  -  7 : Cout(0:7)        Data from TTCrx
  8  - 15 : Sub_Addr(0:7)    Subaddress from the TTCrx
  16      : Dout Str
  17      : L1 accept
 */

#define MASK_DOUT       0xFF
#define RSH_DOUT        0
#define MASK_SUBADR     0xFF00
#define RSH_SUBADR      8
#define MASK_DOUTSTR    0x10000
#define RSH_DOUTSTR     16
#define MASK_L1ACCEPT   0x20000
#define RSH_L1ACCEPT    17

/*
  How many triggers can be pending during off-line analysis
 */

#define ANALYZ_MAX_PEND_TRIG 1536 
#define ANALYZ_MAX_ERR_LIST  2048

/*
  Bunch Crossing structure
 */

#define TTCIT_MAX_BUNCH_XING      3563   /* Max nr. of bunches in LHC cycle */
#define TTCIT_MIN_PHYS_BC         1      /* Minimum BC for physics */
#define TTCIT_MAX_PHYS_BC         3445   /* Max BC for physics */
#define TTCIT_CALIB_CH_INJ        3508   /* Calibration charge injection */
#define TTCIT_PRE_PULSE_IN_GAP    50     /* Pre Pulse BC in LHC gap */
#define TTCIT_PRE_PULSE_LHC       3495   /* Pre Pulse BC in LHC cycle */

/*
  Default timing
 */

#define TTCIT_OM_L0_L1_TIME       224    /* Default L0-L1 distance in BC's */

/*
  Filenames used by this program
 */

#define TTCIT_MONITOR_FILE "ttcit_monitor.dat\0"
#define TTCIT_SSDUMP_FILE  "ssm_dump.dat\0"

/*
  Some helpers
 */

#ifndef FALSE
#define FALSE (0==1)
#endif
#ifndef TRUE
#define TRUE  (1==1)
#endif

/*
  If L0 signal is available on the wire undef this 
*/
#define _TTCIT_L0_A_CHANNEL_ONLY__

/*
  Debug switches, to add or eliminate code that is needed only during 
  debugging process. Keep it at the end of file

  UNDEFINE AFTER SUCCESSFULL DEBUG
 */

#define _LOGIC_COMBINED_1_RADDR__

/*
  If you are 100% sure that SSM data cannot be 0xFFFFFFFF i.e. all 1's,
  i.e. (int)(-1)

  If you are not sure, or  if you KNOW that such a value is normal for the 
  SSM data, undef it, otherwise you will get funky error reports
 */
#undef _LOGIC_SSM_CANNOT_BE_FFFFFF__   

/*
  Undef if you do not need a quick and dirty look at the BCID differences
  (L1 arrival v.s. L2a Header)

  This function should work even when the TTCit soft-logic contains some bugs,
  since it ignores all SSM's where at least one error has been detected
 */
#undef _MONITOR_SCAN_BCID_DIFF__

/*
  Activate when the writing into the SSM becomes possible. This hack is needed
  as an inter versional development trick.
 */
#define _SSM_WRITING_POSSIBLE__

/*
  The L0 - L1 window defined in hardware and software differ by some constant
  (basically 1)
 */

#define TTCIT_L0_L1_HW_SW_KONST      1

/*
  To keep two versions of the SSM in one source, we have to introduce 
  SSM version switch

  Uncomment one of the 

  #define TTCIT_SSM_VER_OLD_         SSM up to the firmware version 23
  #define TTCIT_SSM_VER_NEW_1_       SSM introduced in the firmware version 24
 */

/* #define TTCIT_SSM_VER_OLD_ */
#define TTCIT_SSM_VER_NEW_1_

/*
  Do not use my dummy loops, replace 1000 with 30 sec sleep
 */
#define TTCIT_USE_SLEEPS_

#endif /* _TTCIT_CONF_H__ */
