/****************************************************************/
/*                                              		*/
/*  file: common.h 	                           		*/
/*                                              		*/
/*  Author: Markus Joos, CERN-ECP               		*/
/*                                              		*/
/*  23.Aug.06  MAJO  created                   		  	*/
/*****C 2006 - The software with that certain something**********/

#ifndef _COMMON_H 
#define _COMMON_H

#include "rcc_error/rcc_error.h"

//error codes
enum
{
  RF2TTC_SUCCESS = 0,
  RF2TTC_NOTKNOWN = (P_ID_RF2TTC << 8) + 1,
  RF2TTC_NOTOPEN,
  RF2TTC_ERROR_FAIL,
  RF2TTC_TS,
  RF2TTC_VME,
  RF2TTC_ISOPEN,
  RF2TTC_CAEN1,
  RF2TTC_CAEN2,
  RF2TTC_CAEN3,
  RF2TTC_CAEN4,
  RF2TTC_NOMAP,
  RF2TTC_COMMON,
  RF2TTC_FIFOOVERFLOW,
  RF2TTC_NOCALIB,
  RF2TTC_ILLVAL,
  RF2TTC_NO_CODE
};


/***************/
/*error strings*/
/***************/
#define RF2TTC_SUCCESS_STR      "Function successfully executed"
#define RF2TTC_NO_CODE_STR      "Unknown error"
#define RF2TTC_NOTOPEN_STR      "The library has not yet been opened"
#define RF2TTC_ERROR_FAIL_STR   "Error from rcc_error library"
#define RF2TTC_TS_STR           "Error from rcc_time_stamp library"
#define RF2TTC_VME_STR          "Error from VMEbus access library"
#define RF2TTC_ISOPEN_STR       "The library has already been opened"
#define RF2TTC_CAEN1_STR        "Error from function CAENVME_Init"
#define RF2TTC_CAEN2_STR        "Error from function CAENVME_End"
#define RF2TTC_CAEN3_STR        "Error from function CAENVME_ReadCycle"
#define RF2TTC_CAEN4_STR        "Error from function CAENVME_WriteCycle"
#define RF2TTC_NOMAP_STR        "No free entry for VMEbus mapping found"
#define RF2TTC_COMMON_STR       "Error from function Common_Open"
#define RF2TTC_FIFOOVERFLOW_STR "Too many words read from a FIFO"
#define RF2TTC_NOCALIB_STR      "The calibration algorithm has failed"
#define RF2TTC_ILLVAL_STR       "Illegal value passed to function"

/**************/
/* Prototypes */
/**************/
u_int Common_Open(void);
u_int rf2ttc_err_get(err_pack err, err_str pid, err_str code);
void setdebug(u_int level, u_int package);


#endif
