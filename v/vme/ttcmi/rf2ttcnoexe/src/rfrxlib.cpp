/****************************************************************/
/*                                              		*/
/*  file: rfrxlib.c 	                           		*/
/*                                              		*/
/*  Author: Markus Joos, CERN-ECP               		*/
/*                                              		*/
/*  Sep.06  MAJO  created                   		  	*/
/*                                              		*/
/***C 2008 - The software with that certain something************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include "rcc_error/rcc_error.h"
#include "rcc_time_stamp/tstamp.h"
#include "ROSGetInput/get_input.h"
#include "DFDebug/DFDebug.h"
#include "rfrx.h"
#include "vme_glue.h"

//Globals


/********************************************/
u_int RFRX_Open(u_int vmeabase, u_int *handle)
/********************************************/
{
  u_int ret;

  ret = VME_start();
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"RFRX_Open: VME_start returned " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
 
  ret = VME_map(MODE_A24, vmeabase, MAP_SIZE, handle);
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"RFRX_Open: VME_map returned " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  } 
  return(0); 
}


/****************************/
u_int RFRX_Close(u_int handle)
/****************************/
{
  u_int ret;
  
  ret = VME_unmap(handle);
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"RFRX_Close: VME_unmap returned " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }
    
  ret = VME_stop();
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"RFRX_Close: VME_stop returned " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }
  
  return(0); 
}


/*********************************************/
u_int RFRX_Init(u_int handle, rfrx_init_t data)
/*********************************************/
{
  rfrx_regs_t *rp;
  u_int ret;
 
  rp = (rfrx_regs_t *)0;
  
  ret = VME_d16w(handle, (u_int)&rp->ch1_ref, data.ch1_ref); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"RFRX_Init: VME_d16w returned error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  

  ret = VME_d16w(handle, (u_int)&rp->ch2_ref, data.ch2_ref); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"RFRX_Init: VME_d16w returned error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  

  ret = VME_d16w(handle, (u_int)&rp->ch3_ref, data.ch3_ref); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"RFRX_Init: VME_d16w returned error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  

  return(0); 
}


/***************************/
u_int RFRX_Dump(u_int handle)
/***************************/
{
  rfrx_regs_t *rp;
  u_int ret;
  u_short vd;
  
  rp = (rfrx_regs_t *)0;

  ret = VME_d16r(handle, (u_int)&rp->card_id, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register card_id         contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->ch1_freq_lo, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register ch1_freq_lo     contains 0x%04x\n", vd);
  
  ret = VME_d16r(handle, (u_int)&rp->ch1_freq_hi, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register ch1_freq_hi     contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->ch2_freq_lo, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register ch2_freq_lo     contains 0x%04x\n", vd);
  
  ret = VME_d16r(handle, (u_int)&rp->ch2_freq_hi, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register ch2_freq_hi     contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->ch3_freq_lo, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register ch3_freq_lo     contains 0x%04x\n", vd);
  
  ret = VME_d16r(handle, (u_int)&rp->ch3_freq_hi, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register ch3_freq_hi     contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->ch1_ref, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register ch1_ref         contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->ch2_ref, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register ch2_ref         contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->ch3_ref, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register ch3_ref         contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->VMEIRQStatID, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register VMEIRQStatID    contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->VMEIRQLevel, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register VMEIRQLevel     contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->Status, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register Status          contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->IdentCode, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register IdentCode       contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->SIG_DETEC, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register SIG_DETEC       contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->ReceiverModID, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register ReceiverModID   contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->board_id, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register board_id        contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->FirmwareVer_lo, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register FirmwareVer_lo  contains 0x%04x\n", vd);

  ret = VME_d16r(handle, (u_int)&rp->FirmwareVer_hi, &vd); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5 ,"VME_d16w returns error " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
  printf("Register FirmwareVer_hi  contains 0x%04x\n", vd);

  return(0);  
}
