/****************************************************************/
/*                                              		*/
/*  file: commob_lib.c 	                           		*/
/*                                              		*/
/*  Author: Markus Joos, CERN-ECP               		*/
/*                                              		*/
/*  Oct.06  MAJO  created                   		  	*/
/*                                              		*/
/***C 2006 - The software with that certain something************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include "rcc_error/rcc_error.h"
#include "DFDebug/DFDebug.h"
#include "ROSGetInput/get_input.h"
#include "common.h"

/*********************/
u_int Common_Open(void)
/*********************/
{
  u_int ret;
  
  ret = rcc_error_init(P_ID_RF2TTC, rf2ttc_err_get);
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"Common_Open: Error from rcc_error_init");
    return(RCC_ERROR_RETURN(0, RF2TTC_ERROR_FAIL)); 
  }
  return(0);
}  


/***************************************/
void setdebug(u_int level, u_int package)
/***************************************/
{
  static u_int dblevel = 0, dbpackage = DFDB_RF2TTC;
  
  if (level == 99)
  {
    printf("Enter the debug level: ");
    dblevel = getdecd(dblevel);
    printf("Enter the debug package: ");
    dbpackage = getdecd(dbpackage);
    DF::GlobalDebugSettings::setup(dblevel, dbpackage);
  }
  else
    DF::GlobalDebugSettings::setup(level, DFDB_RF2TTC);
}


/***********************************************************/
u_int rf2ttc_err_get(err_pack err, err_str pid, err_str code)
/***********************************************************/
{ 
  strcpy(pid, P_ID_RF2TTC_STR);

  switch (RCC_ERROR_MINOR(err))
  {  
    case RF2TTC_SUCCESS:      strcpy(code, RF2TTC_SUCCESS_STR); break;   
    case RF2TTC_NOTOPEN:      strcpy(code, RF2TTC_NOTOPEN_STR); break; 
    case RF2TTC_ERROR_FAIL:   strcpy(code, RF2TTC_ERROR_FAIL_STR); break; 
    case RF2TTC_TS:           strcpy(code, RF2TTC_TS_STR); break; 
    case RF2TTC_VME:          strcpy(code, RF2TTC_VME_STR); break; 
    case RF2TTC_ISOPEN:       strcpy(code, RF2TTC_ISOPEN_STR); break; 
    case RF2TTC_CAEN1:        strcpy(code, RF2TTC_CAEN1_STR); break; 
    case RF2TTC_CAEN2:        strcpy(code, RF2TTC_CAEN2_STR); break; 
    case RF2TTC_CAEN3:        strcpy(code, RF2TTC_CAEN3_STR); break; 
    case RF2TTC_CAEN4:        strcpy(code, RF2TTC_CAEN4_STR); break; 
    case RF2TTC_NOMAP:        strcpy(code, RF2TTC_NOMAP_STR); break; 
    case RF2TTC_COMMON:       strcpy(code, RF2TTC_COMMON_STR); break; 
    case RF2TTC_FIFOOVERFLOW: strcpy(code, RF2TTC_FIFOOVERFLOW_STR); break; 
    case RF2TTC_NOCALIB:      strcpy(code, RF2TTC_NOCALIB_STR); break;
    case RF2TTC_ILLVAL:       strcpy(code, RF2TTC_ILLVAL_STR); break;
    default:                  strcpy(code, RF2TTC_NO_CODE_STR); return(RCC_ERROR_RETURN(0, RF2TTC_NO_CODE)); break;
  }
  return(RCC_ERROR_RETURN(0, RF2TTC_SUCCESS));
}
