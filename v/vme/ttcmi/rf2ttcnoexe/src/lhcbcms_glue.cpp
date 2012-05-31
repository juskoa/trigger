// $Id: lhcbcms_glue.cpp,v 1.3 2007/03/01 10:01:34 joos Exp $
/****************************************************************/
/*                                              		*/
/*  file: lhcbcms_glue.cpp                            		*/
/*                                              		*/
/*  Glue layer for theLHC-b / CMS VMEbus access library		*/
/*                                              		*/
/*  Author: Markus Joos, CERN-ECP               		*/
/*                                              		*/
/*  13.Oct.06  MAJO  created                   		  	*/
/*                                              		*/
/********C 2006 - A nickel program worth a dime******************/

#include "rcc_error/rcc_error.h"
#include "DFDebug/DFDebug.h"
#include "CAENVMElib.h"  
#include "CAENVMEoslib.h" 
#include "CAENVMEtypes.h"
#include "common.h"
#include "vme_glue.h"


#define MAX_MMAP 30


typedef struct
{
  u_int free;
  u_int base;
  u_int am;
} mastermap_t;


//Globals
u_int glue_isopen = 0, base;
long init_handle;
mastermap_t mmap[MAX_MMAP];


/*******************/
u_int VME_start(void)
/*******************/
{ 
  CAENVME_API ret;
  u_int loop;
  
  if (!glue_isopen)
  {
#ifdef LHCB
    DEBUG_TEXT(DFDB_RF2TTC, 20, "VME_start: Opening USB to VMEbus link");
    ret = CAENVME_Init(cvV1718, 0, 0, &init_handle);
#else    
    DEBUG_TEXT(DFDB_RF2TTC, 20, "VME_start: Opening PCI to VMEbus link");
    ret = CAENVME_Init(cvV2718, 0, 0, &init_handle);
#endif

    if (ret)
    {    
      DEBUG_TEXT(DFDB_RF2TTC, 5, "VME_start: CAENVME_Init returned " << ret);
      return(RCC_ERROR_RETURN(0, RF2TTC_CAEN1)); 
    }
    for (loop = 0; loop < MAX_MMAP; loop++)
      mmap[loop].free = 1;
      
    glue_isopen = 1; 
  }
  
  return(0);
}


/******************/
u_int VME_stop(void)
/******************/
{
  CAENVME_API ret;

  if (glue_isopen)
  {
    glue_isopen = 0; 
    ret = CAENVME_End(init_handle);
    if (ret)
    {
      DEBUG_TEXT(DFDB_RF2TTC, 5, "VME_stop: CAENVME_End returned " << ret);
      return(RCC_ERROR_RETURN(0, RF2TTC_CAEN2)); 
    }
  }
  
  return(0);
}


/************************************************************/
u_int VME_map(u_int am, u_int base, u_int size, u_int *handle)
/************************************************************/
{
  u_int loop, ok = 0;
  
  for (loop = 0; loop < MAX_MMAP; loop++)
    if (mmap[loop].free == 1)
    {
      ok = 1;
      break;
    }
  
  if (!ok)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "VME_map: No free mapping found");
    return(RCC_ERROR_RETURN(0, RF2TTC_NOMAP)); 
  }
  printf("Using mapping %d\n", loop);
  mmap[loop].free = 0;  
  mmap[loop].base = base;  
  mmap[loop].am = am;  
  *handle = loop;
  return(0);
}


/***************************/
u_int VME_unmap(u_int handle)
/***************************/
{
  mmap[handle].free = 1;  
  return(0);
}



/*****************************************************/
u_int VME_d32r(u_int handle, u_int offset, u_int *data)
/*****************************************************/
{  
  CAENVME_API ret;  

  if (mmap[handle].am == MODE_A32)
    ret = CAENVME_ReadCycle(init_handle, mmap[handle].base + offset, data, cvA32_U_DATA, cvD32); 
  else
    ret = CAENVME_ReadCycle(init_handle, mmap[handle].base + offset, data, cvA24_U_DATA, cvD32); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "VME_d32r: CAENVME_ReadCycle returned " << ret);
    return(RCC_ERROR_RETURN(0, RF2TTC_CAEN3)); 
  }
  return(0);
}


/****************************************************/
u_int VME_d32w(u_int handle, u_int offset, u_int data)
/****************************************************/
{
  CAENVME_API ret;  

  if (mmap[handle].am == MODE_A32)
    ret = CAENVME_WriteCycle(init_handle, mmap[handle].base + offset, (void *)&data, cvA32_U_DATA, cvD32); 
  else
    ret = CAENVME_WriteCycle(init_handle, mmap[handle].base + offset, (void *)&data, cvA24_U_DATA, cvD32); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "VME_d32w: CAENVME_WriteCycle returned " << ret);
    return(RCC_ERROR_RETURN(0, RF2TTC_CAEN4)); 
  }
  return(0);
}


/*******************************************************/
u_int VME_d16r(u_int handle, u_int offset, u_short *data)
/*******************************************************/
{  
  CAENVME_API ret;  

  if (mmap[handle].am == MODE_A32)
    ret = CAENVME_ReadCycle(init_handle, mmap[handle].base + offset, data, cvA32_U_DATA, cvD16); 
  else
    ret = CAENVME_ReadCycle(init_handle, mmap[handle].base + offset, data, cvA24_U_DATA, cvD16); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "VME_d16r: CAENVME_ReadCycle returned " << ret);
    return(RCC_ERROR_RETURN(0, RF2TTC_CAEN3)); 
  }
  return(0);
}


/******************************************************/
u_int VME_d16w(u_int handle, u_int offset, u_short data)
/******************************************************/
{
  CAENVME_API ret;  

  if (mmap[handle].am == MODE_A32)
    ret = CAENVME_WriteCycle(init_handle, mmap[handle].base + offset, (void *)&data, cvA32_U_DATA, cvD16); 
  else
    ret = CAENVME_WriteCycle(init_handle, mmap[handle].base + offset, (void *)&data, cvA24_U_DATA, cvD16); 
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "VME_d16w: CAENVME_WriteCycle returned " << ret);
    return(RCC_ERROR_RETURN(0, RF2TTC_CAEN4)); 
  }
  return(0);
}












