// $Id: atlice_glue.cpp,v 1.5 2007/03/08 10:40:49 joos Exp $
/****************************************************************/
/*                                              		*/
/*  file: atlice_glue.cpp                            		*/
/*                                              		*/
/*  Glue layer for the ATLAS / ALICE VMEbus access library	*/
/*                                              		*/
/*  Author: Markus Joos, CERN-ECP               		*/
/*                                              		*/
/*  13.Oct.06  MAJO  created                   		  	*/
/*                                              		*/
/********C 2006 - A nickel program worth a dime******************/

#include "DFDebug/DFDebug.h"
#include "rcc_error/rcc_error.h"
#include "vme_rcc/vme_rcc.h"
#include "vme_glue.h"

/*******************/
u_int VME_start(void)
/*******************/
{
  u_int ret;
    
  ret = VME_Open();
  if (ret != VME_SUCCESS)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "VME_start: VME_Open returned " << ret);
    return(RCC_ERROR_RETURN(0, ret)); 
  }  
  
  return(0);
}


/******************/
u_int VME_stop(void)
/******************/
{
  u_int ret;

  ret = VME_Close();
  if (ret != VME_SUCCESS)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "VME_stop: VME_Close returned " << ret);
    return(RCC_ERROR_RETURN(0, ret)); 
  }
  
  return(0);
}


/************************************************************/
u_int VME_map(u_int am, u_int base, u_int size, u_int *handle)
/************************************************************/
{
  VME_MasterMap_t master_map;
  u_int ret;
  int ihandle;
  
  master_map.vmebus_address = base;
  master_map.window_size    = size;
  master_map.options        = 0;
  if (am == MODE_A32)
    master_map.address_modifier = VME_A32;
  else if (am == MODE_A24)
    master_map.address_modifier = VME_A24;
  else 
    return(3);
  
  ret = VME_MasterMap(&master_map, &ihandle);
  if (ret != VME_SUCCESS)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "VME_map: VME_MasterMap returned " << ret);
    return(RCC_ERROR_RETURN(0, ret)); 
  }
  
  DEBUG_TEXT(DFDB_RF2TTC, 20, "VME_map: handle = " << ihandle);
  *handle = (u_int)ihandle;
  return(0);
}


/***************************/
u_int VME_unmap(u_int handle)
/***************************/
{
  u_int ret;

  DEBUG_TEXT(DFDB_RF2TTC, 20, "VME_unmap: handle = " << handle);
  ret = VME_MasterUnmap(handle);
  if (ret != VME_SUCCESS)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "VME_unmap: VME_MasterUnmap returned " << ret);
    return(RCC_ERROR_RETURN(0, ret)); 
  }
  
  return(0);
}


/*****************************************************/
u_int VME_d32r(u_int handle, u_int offset, u_int *data)
/*****************************************************/
{  
  u_int ret;

  ret = VME_ReadSafeUInt(handle, offset, data);
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "VME_d32r: VME_ReadSafeUInt returned " << ret);
    return(RCC_ERROR_RETURN(0, ret)); 
  }
  
  return(0);
}


/****************************************************/
u_int VME_d32w(u_int handle, u_int offset, u_int data)
/****************************************************/
{
  u_int ret;

  ret = VME_WriteSafeUInt(handle, offset, data);
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "VME_d32w: VME_WriteSafeUInt returned " << ret);
    return(RCC_ERROR_RETURN(0, ret)); 
  }
  
  return(0);
}


/*******************************************************/
u_int VME_d16r(u_int handle, u_int offset, u_short *data)
/*******************************************************/
{  
  u_int ret;

  ret = VME_ReadSafeUShort(handle, offset, data);
  if (ret)
  {
    VME_ErrorPrint(ret);  
    return(RCC_ERROR_RETURN(0, ret)); 
  }
  
  return(0);
}


/******************************************************/
u_int VME_d16w(u_int handle, u_int offset, u_short data)
/******************************************************/
{
  u_int ret;

  ret = VME_WriteSafeUShort(handle, offset, data);
  if (ret)
  {
    VME_ErrorPrint(ret);  
    return(RCC_ERROR_RETURN(0, ret)); 
  }
  
  return(0);
}













