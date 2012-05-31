/****************************************************************/
/*                                              		*/
/*  file: rf2ttclib.c 	                           		*/
/*                                              		*/
/*  Author: Markus Joos, CERN-ECP               		*/
/*                                              		*/
/*  Sep.06  MAJO  created                   		  	*/
/*                                              		*/
/***C 2007 - The software with that certain something************/

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
#include "rf2ttc.h"
#include "vme_glue.h"


//Design & implementation issues:
//
//- Multi-process support?
//- SMP support?
//- Multi-card support?


//Globals
u_int isopen = 0, vmeslot, handle;
rf2ttc_regs_t *regs;


/*******************************/
u_int RF2TTC_Open(u_int vmeabase)
/*******************************/
{
  u_int ret;

  DEBUG_TEXT(DFDB_RF2TTC, 15, "RF2TTC_Open: Called");
  if (isopen)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_Open: The library has already been opened");
    return(RCC_ERROR_RETURN(0, RF2TTC_ISOPEN)); 
  }
  
  regs = (rf2ttc_regs_t *)0;

  ret = Common_Open();
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_Open: Common_Open returned " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_COMMON)); 
  }  
  
  ret = ts_open(1, TS_DUMMY);
  if (ret)
  {
    rcc_error_print(stdout, ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_TS)); 
  }
  
  ret = VME_start();
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_Open: VME_start returned " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }  
 
  ret = VME_map(MODE_A32, vmeabase, RF2TTC_MAP_SIZE, &handle);
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_Open: VME_map returned " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }
  
  isopen = 1;
   
  return(0); 
}


/*******************************************/
u_int RF2TTC_Open(u_int vmeabase, u_int slot)
/*******************************************/
{
  u_int ret;

  ret = RF2TTC_Open(vmeabase);
  vmeslot = slot;

  return(ret); 
}


/********************************************/
u_int RF2TTC_Slot2VME(u_int slot, u_int *addr)
/********************************************/
{   
  DEBUG_TEXT(DFDB_RF2TTC, 15, "RF2TTC_Slot2VME: called with slot = " << slot);
  *addr = slot << 20;
  DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_Slot2VME: VMEbus address is = " << *addr);
  return(0);
}


/**********************/
u_int RF2TTC_Close(void)
/**********************/
{
  u_int ret;
  
  if (!isopen)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_Close: The library has not yet been opened");
    return(RCC_ERROR_RETURN(0, RF2TTC_NOTOPEN)); 
  } 
 
  ret = VME_unmap(handle);
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_Close: VME_unmap returned " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }
    
  ret = VME_stop();
  if (ret)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_Close: VME_stop returned " << ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
  }
  
  ret = ts_close(TS_DUMMY);
  if (ret)
  {
    rcc_error_print(stdout, ret);
    return(RCC_ERROR_RETURN(ret, RF2TTC_TS)); 
  }
  
  isopen = 0;
  return(0); 
}


/**************************************/
u_int RF2TTC_GetHandle(u_int *vmehandle)
/**************************************/
{
  if (!isopen)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_GetHandle: The library has not yet been opened");
    return(RCC_ERROR_RETURN(0, RF2TTC_NOTOPEN)); 
  } 
  
  *vmehandle = handle;
  return(0);
}


/**********************************************/
u_int RF2TTC_I2C_Read(u_int offset, u_int *data)
/**********************************************/
{  
  //Rule:
  //Offset in the range       0..28      -> Read TTC register
  //Offset in the range 0x7D000..0x7D054 -> Read delay register
  //Other offsets: error  
  
  u_int ret, dummy;
 
  if (!isopen)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_I2C_Read: The library has not yet been opened");
    return(RCC_ERROR_RETURN(0, RF2TTC_NOTOPEN)); 
  } 
    
  if (offset <= 28)
  {    
    DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_I2C_Read: reading TTC register");

    ret = VME_d32w(handle, (u_int)&regs->ttcrx_pointer, offset);
    if (ret)
    {
      DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_I2C_Read: First call to VME_d32w returned " << ret);
      *data = 0;
      return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
    }
    ts_delay(TWO_MILLISECONDS);

    ret = VME_d32r(handle, (u_int)&regs->ttcrx_pointer, &dummy);
    if (ret)
    {
      DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_I2C_Read: First call to VME_d32r returned " << ret);
      *data = 0;
      return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
    }
    ts_delay(TWO_MILLISECONDS);

    ret = VME_d32r(handle, (u_int)&regs->ttcrx_reg, data);
    if (ret)
    {
      DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_I2C_Read: Second call to VME_d32r returned " << ret);
      *data = 0;
      return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
    }
  }

  else if ((offset >= 0x7D000) && (offset <= 0x7D054))
  {
    DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_I2C_Read: reading delay register");
    ret = VME_d32r(handle, offset, &dummy);
    if (ret)
    {
      DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_I2C_Read: first call to VME_d32r returned " << ret);
      *data = 0;
      return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
    }
    ts_delay(TWO_MILLISECONDS);
    ret = VME_d32r(handle, (u_int)&regs->delay25_reg, data);
    if (ret)
    {
      DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_I2C_Read: second call to VME_d32r returned " << ret);
      *data = 0;
      return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
    }
  }

  else
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_I2C_Read: The offset parameter is out of range");
    return(RCC_ERROR_RETURN(ret, RF2TTC_ILLVAL)); 
  }

  return(0);
}


/**********************************************/
u_int RF2TTC_I2C_Write(u_int offset, u_int data)
/**********************************************/
{  
  //Rule:
  //Offset in the range       0..28      -> Read TTC register
  //Offset in the range 0x7D000..0x7D054 -> Read delay register
  //Other offsets: error
  
  u_int ret;
 
  if (!isopen)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_I2C_Write: The library has not yet been opened");
    return(RCC_ERROR_RETURN(0, RF2TTC_NOTOPEN)); 
  } 

  if (offset <= 28)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_I2C_Write: Writing TTC register");
    
    ret = VME_d32w(handle, (u_int)&regs->ttcrx_pointer, offset);
    if (ret)
    {
      DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_I2C_Write: First call to VME_d32w returned " << ret);
      return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
    }
    ts_delay(TWO_MILLISECONDS);  

    ret = VME_d32w(handle, (u_int)&regs->ttcrx_data, data);
    if (ret)
    {
      DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_I2C_Write: Second call to VME_d32w returned " << ret);
      return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
    }
  }
  
  else if ((offset >= 0x7D000) && (offset <= 0x7D054))
  {
    DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_I2C_Write: Writing delay register");
    ret = VME_d32w(handle, offset, data);
    if (ret)
    {
      DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_I2C_Write: call to VME_d32w returned " << ret);
      return(RCC_ERROR_RETURN(ret, RF2TTC_VME)); 
    }

    ts_delay(TWO_MILLISECONDS);  
  }
    
  else
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_I2C_Write: The offset parameter is out of range");
    return(RCC_ERROR_RETURN(ret, RF2TTC_ILLVAL)); 
  }

  return(0);
}


/*********************/
u_int RF2TTC_Dump(void)
/*********************/
{
  u_int ret, v1, v2, v3, v4, v5, v6, v7;
 
  printf("IDs  | Manufacturer |      Board |   Revision |    Program\n");
  ret = VME_d32r(handle, (u_int)&regs->manufacturer_id, &v1); 
  if (ret) return(ret);
  ret = VME_d32r(handle, (u_int)&regs->board_id, &v2); 
  if (ret) return(ret);
  ret = VME_d32r(handle, (u_int)&regs->revision_id, &v3); 
  if (ret) return(ret);
  ret = VME_d32r(handle, (u_int)&regs->program_id, &v4); 
  if (ret) return(ret);  
  printf("     |   0x%08x | 0x%08x | 0x%08x | 0x%08x\n", v1, v2, v3, v4);

  printf("                |  BC1 |  BC2 | BCref | BCmain |  GCR \n");
  printf("BC delay        | ");

  ret = RF2TTC_I2C_Read((u_int)&regs->bc_delay25_bc1, &v1); v1 &= 0xff;
  if (ret) return(ret);  
  ret = RF2TTC_I2C_Read((u_int)&regs->bc_delay25_bc2, &v2); v2 &= 0xff;
  if (ret) return(ret);  
  ret = RF2TTC_I2C_Read((u_int)&regs->bc_delay25_bcref, &v3); v3 &= 0xff;
  if (ret) return(ret);  
  ret = RF2TTC_I2C_Read((u_int)&regs->bc_delay25_bcmain, &v4); v4 &= 0xff;
  if (ret) return(ret);  
  ret = RF2TTC_I2C_Read((u_int)&regs->bc_delay25_gcr, &v5); v5 &= 0xff;
  if (ret) return(ret);  
  printf("0x%02x | 0x%02x |  0x%02x |   0x%02x | 0x%02x\n\n", v1, v2, v3, v4, v5);

  printf("                | ORB1 | ORB2 | ORBmain |  GCR\n");
  printf("Orbit in delay  | ");
  ret = RF2TTC_I2C_Read((u_int)&regs->orbin_delay25_orb1, &v1); v1 &= 0xff;
  if (ret) return(ret);  
  ret = RF2TTC_I2C_Read((u_int)&regs->orbin_delay25_orb2, &v2); v2 &= 0xff;
  if (ret) return(ret);  
  ret = RF2TTC_I2C_Read((u_int)&regs->orbin_delay25_gcr, &v3); v3 &= 0xff;
  if (ret) return(ret);  
  printf("0x%02x | 0x%02x | ------- | 0x%02x\n", v1, v2, v3);

  printf("Orbit out delay | ");
  ret = RF2TTC_I2C_Read((u_int)&regs->orbout_delay25_orb1, &v1); v1 &= 0xff;
  if (ret) return(ret);  
  ret = RF2TTC_I2C_Read((u_int)&regs->orbout_delay25_orb2, &v2); v2 &= 0xff;
  if (ret) return(ret);  
  ret = ret = RF2TTC_I2C_Read((u_int)&regs->orbout_delay25_orbmain, &v3); v3 &= 0xff;
  if (ret) return(ret);  
  RF2TTC_I2C_Read((u_int)&regs->orbout_delay25_gcr, &v4); v4 &= 0xff;
  printf("0x%02x | 0x%02x |    0x%02x | 0x%02x\n\n", v1, v2, v3, v4);

  printf("                | Period counter enable | Orbit counter enable | Orbit internal enable \n");
  printf("Command space   | ");
  ret = VME_d32r(handle, (u_int)&regs->period_counter_enable, &v1); v1 &= 0x7;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb_counter_enable, &v2);    v2 &= 0x7;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb_int_enable, &v3);        v3 &= 0x7;
  if (ret) return(ret);  
  printf("                 0x%02x |                 0x%02x |                  0x%02x\n\n", v1, v2, v3);

  printf("                | Working Mode | Beam-No-Beam-Def \n");
  printf("Working Modes   | ");
  ret = VME_d32r(handle, (u_int)&regs->working_mode, &v1);     v1 &= 0x7f;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->beam_no_beam_def, &v2);
  if (ret) return(ret);  
  printf("        0x%02x |             0x%02x\n\n", v1, v2);
  
  printf("                | BST Machine Mode | TTCrx status \n");
  printf("BST registers   | ");
  ret = VME_d32r(handle, (u_int)&regs->bst_machine_mode, &v1);
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->ttcrx_status, &v2); v2 &= 0x3;
  if (ret) return(ret);  
  printf("      0x%08x |         0x%02x\n\n", v1, v2 & 0xff);

  printf("                |   DAC |  PERIOD_FIFO_RD | PERIOD_FIFO_STATUS | PERIOD_RD |    COUNTER | PERIOD_COUNTER | PERIOD_SET\n");
  printf("Orbit 1         | ");
  ret = VME_d32r(handle, (u_int)&regs->orb1_dac, &v1);                v1 &= 0xff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb1_period_fifo_rd, &v2);     v2 &= 0xfff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb1_period_fifo_status, &v3); v3 &= 0x3;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb1_period_rd, &v4);          v4 &= 0xfff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb1_counter, &v5);
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb1_int_period_counter, &v6); v6 &= 0xfff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb1_int_period_set, &v7);     v7 &= 0xfff;
  if (ret) return(ret);  
  printf("0x%03x |           0x%03x |              0x%03x |     0x%03x | 0x%08x |          0x%03x |      0x%03x\n", v1, v2, v3, v4, v5, v6, v7); 

  printf("Orbit 2         | ");
  ret = VME_d32r(handle, (u_int)&regs->orb2_dac, &v1);                v1 &= 0xff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb2_period_fifo_rd, &v2);     v2 &= 0xfff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb2_period_fifo_status, &v3); v3 &= 0x3;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb2_period_rd, &v4);          v4 &= 0xfff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb2_counter, &v5);
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb2_int_period_counter, &v6); v6 &= 0xfff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb2_int_period_set, &v7);     v7 &= 0xfff;
  if (ret) return(ret);  
  printf("0x%03x |           0x%03x |              0x%03x |     0x%03x | 0x%08x |          0x%03x |      0x%03x\n", v1, v2, v3, v4, v5, v6, v7); 

  printf("Main orbit      | ");
  ret = VME_d32r(handle, (u_int)&regs->orbmain_period_fifo_rd, &v2);     v2 &= 0xfff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orbmain_period_fifo_status, &v3); v3 &= 0x3;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orbmain_period_rd, &v4);          v4 &= 0xfff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orbmain_counter, &v5);
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orbmain_int_period_counter, &v6); v6 &= 0xfff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orbmain_int_period_set, &v7);     v7 &= 0xfff;
  if (ret) return(ret);  
  printf("  n/a |           0x%03x |              0x%03x |     0x%03x | 0x%08x |          0x%03x |      0x%03x\n", v2, v3, v4, v5, v6, v7); 
 
  printf("                | LENGTH | COARSE_DELAY | POLARITY | NOBEAM_SELECT | BEAM_SELECT | MAN_SLECT\n");
  printf("Orbit 1         | ");
  ret = VME_d32r(handle, (u_int)&regs->orb1_length, &v1);        v1 &= 0xff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb1_course_delay, &v2);  v2 &= 0xfff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb1_polarity, &v3);      v3 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb1_nobeam_select, &v4); v4 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb1_beam_select, &v5);   v5 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb1_man_select, &v6);    v6 &= 0x1;
  if (ret) return(ret);  
  printf(" 0x%03x |        0x%03x |    0x%03x |    0x%08x |       0x%03x |     0x%03x\n", v1, v2, v3, v4, v5, v6); 

  printf("Orbit 2         | ");
  ret = VME_d32r(handle, (u_int)&regs->orb2_length, &v1);        v1 &= 0xff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb2_course_delay, &v2);  v2 &= 0xfff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb2_polarity, &v3);      v3 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb2_nobeam_select, &v4); v4 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb2_beam_select, &v5);   v5 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orb2_man_select, &v6);    v6 &= 0x1;
  if (ret) return(ret);  
  printf(" 0x%03x |        0x%03x |    0x%03x |    0x%08x |       0x%03x |     0x%03x\n", v1, v2, v3, v4, v5, v6); 

  printf("Main orbit      | ");
  ret = VME_d32r(handle, (u_int)&regs->orbmain_length, &v1);        v1 &= 0xff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orbmain_course_delay, &v2);  v2 &= 0xfff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orbmain_polarity, &v3);      v3 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orbmain_nobeam_select, &v4); v4 &= 0x3;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orbmain_beam_select, &v5);   v5 &= 0x3;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->orbmain_man_select, &v6);    v6 &= 0x3;
  if (ret) return(ret);  
  printf(" 0x%03x |        0x%03x |    0x%03x |    0x%08x |       0x%03x |     0x%03x\n\n", v1, v2, v3, v4, v5, v6); 

  printf("                | QPLL_STATUS |   DAC | QPLL_MODE | NOBEAM_SELECT | BEAM_SELECT | MAN_SLECT\n");
  printf("BC 1            | ");
  ret = VME_d32r(handle, (u_int)&regs->bc1_qpll_status, &v1);   v1 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bc1_daq, &v2);           v2 &= 0xff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bc1_qpll_mode, &v3);     v3 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bc1_nobeam_select, &v4); v4 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bc1_beam_select, &v5);   v5 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bc1_man_select, &v6);    v6 &= 0x1;
  if (ret) return(ret);  
  printf("      0x%03x | 0x%03x |     0x%03x |         0x%03x |       0x%03x |     0x%03x\n", v1, v2, v3, v4, v5, v6); 

  printf("BC 2            | ");
  ret = VME_d32r(handle, (u_int)&regs->bc2_qpll_status, &v1);   v1 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bc2_daq, &v2);           v2 &= 0xff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bc2_qpll_mode, &v3);     v3 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bc2_nobeam_select, &v4); v4 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bc2_beam_select, &v5);   v5 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bc2_man_select, &v6);    v6 &= 0x1;
  if (ret) return(ret);  
  printf("      0x%03x | 0x%03x |     0x%03x |         0x%03x |       0x%03x |     0x%03x\n", v1, v2, v3, v4, v5, v6); 

  printf("BC ref          | ");
  ret = VME_d32r(handle, (u_int)&regs->bcref_qpll_status, &v1);   v1 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bcref_daq, &v2);           v2 &= 0xff;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bcref_qpll_mode, &v3);     v3 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bcref_nobeam_select, &v4); v4 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bcref_beam_select, &v5);   v5 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bcref_man_select, &v6);    v6 &= 0x1;
  if (ret) return(ret);  
  printf("      0x%03x | 0x%03x |     0x%03x |         0x%03x |       0x%03x |     0x%03x\n", v1, v2, v3, v4, v5, v6); 

  printf("BC main         | ");
  ret = VME_d32r(handle, (u_int)&regs->bcmain_qpll_status, &v1);   v1 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bcmain_qpll_mode, &v3);     v3 &= 0x1;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bcmain_nobeam_select, &v4); v4 &= 0x3;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bcmain_beam_select, &v5);   v5 &= 0x3;
  if (ret) return(ret);  
  ret = VME_d32r(handle, (u_int)&regs->bcmain_man_select, &v6);    v6 &= 0x3;
  if (ret) return(ret);  
  printf("      0x%03x |   n/a |     0x%03x |         0x%03x |       0x%03x |     0x%03x\n", v1, v3, v4, v5, v6); 
  return(0);
}


/********************************************/
u_int RF2TTC_CalOrbit(u_int orbit, u_int mode)
/********************************************/
{
  u_int fifowasfull, bestfit, reg1, reg2, reg3, min, max, value, sample, delay, sync[ORBRANGE];
  
  DEBUG_TEXT(DFDB_RF2TTC, 15, "RF2TTC_CalOrbit: Called");

  //introduce "orbit" here    
  if (mode == LAB_MODE)
  {
    VME_d32w(handle, (u_int)&regs->bc_delay25_bc1,         0x40);
    VME_d32w(handle, (u_int)&regs->bc_delay25_bc2,         0x40);
    VME_d32w(handle, (u_int)&regs->bc_delay25_bcref,       0x40);
    VME_d32w(handle, (u_int)&regs->bc_delay25_bcmain,      0x40);
    VME_d32w(handle, (u_int)&regs->orbin_delay25_orb1,     0x40);
    VME_d32w(handle, (u_int)&regs->orbin_delay25_orb2,     0x40);
    VME_d32w(handle, (u_int)&regs->orbout_delay25_orb1,    0x40);
    VME_d32w(handle, (u_int)&regs->orbout_delay25_orb2,    0x40);
    VME_d32w(handle, (u_int)&regs->orbout_delay25_orbmain, 0x40);
    VME_d32w(handle, (u_int)&regs->working_mode,           0);
    VME_d32w(handle, (u_int)&regs->bc1_man_select,         1);
    VME_d32w(handle, (u_int)&regs->bc2_man_select,         1);
  }
  
  //introduce "orbit" here    
  VME_d32w(handle, (u_int)&regs->orb1_man_select,       0);
  VME_d32w(handle, (u_int)&regs->orb2_man_select,       0);
  VME_d32w(handle, (u_int)&regs->orb1_dac,              0xaa);
  VME_d32w(handle, (u_int)&regs->orb2_dac,              0xaa);
  VME_d32w(handle, (u_int)&regs->period_counter_enable, 7);
  
  if (orbit == 1)
  {
    reg1 = (u_int)&regs->orbin_delay25_orb1;
    reg2 = (u_int)&regs->orb1_period_fifo_rd;
    reg3 = (u_int)&regs->orb1_period_fifo_status;
  }
  else
  {
    reg1 = (u_int)&regs->orbin_delay25_orb2;
    reg2 = (u_int)&regs->orb2_period_fifo_rd;
    reg3 = (u_int)&regs->orb2_period_fifo_status;
  }
  
  for (delay = 0; delay < 0x40; delay++)
  {    
    VME_d32w(handle, reg1,   (0x40 | delay));
    VME_d32w(handle, (u_int)&regs->period_counter_reset, 3);
    
    ts_delay(30000); //delay 30ms
    
    min = 0xffff;
    max = 0;
    sample = 0;
    while (1) 
    {      
      if (sample == 0)
      {
        VME_d32r(handle, reg3, &value);
        if (value != 2)
        {
          DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_CalOrbit: WARNING: FIFO not full for delay " << delay);
          DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_CalOrbit: WARNING: orb_period_fifo_status contains " << value);
          fifowasfull = 0;
        }
	else
          fifowasfull = 1;	 
      }	

      VME_d32r(handle, reg2, &value);

      if (delay == 0)
        DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_CalOrbit: Delay = 0: FIFO value[" << sample << "] = " << HEX(value));
      
      if (value & 0x4000)
      {
        DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_CalOrbit: INFO: FIFO empty after " << sample << " samples for delay " << delay);
        break;
      }
      
      if (sample != 0)  //The first sample contains invalid data
      {
        value &= 0x3fff;
        if (value > max) max = value;
        if (value < min) min = value;
      }	

      sample++;
    }

    sync[delay] = 0;
    if (sample > 1) 
      DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_CalOrbit: Orbit " << orbit << ": min=" << min << ", max=" << max << " for delay = " << delay << " (" << sample << " samples)");

    if (sample > FIFOSIZE)
    {
      DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_CalOrbit: ERROR: The FIFO contained " << sample << " samples");
      return(RCC_ERROR_RETURN(0, RF2TTC_FIFOOVERFLOW)); 
    }  
    
    if ((max == min) && fifowasfull)
    {
      DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_CalOrbit: Orbit " << orbit << " is synchronized for delay = " << delay);
      sync[delay] = 1;
    }
  }
  
  bestfit = RF2TTC_FindCalib(sync, ORBRANGE, mode);
  
  if (bestfit == INVALID)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_CalOrbit: ERROR: Calibration failed");
    DEBUG_TEXT(DFDB_RF2TTC, 15, "RF2TTC_CalOrbit: Done");
    return(RCC_ERROR_RETURN(0, RF2TTC_NOCALIB)); 
  }

  DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_CalOrbit: Best Orbin_delay25: " << bestfit);
  if (mode == TEST_MODE)
    printf("Best Orbin_delay25: 0x%08x\n", bestfit);
  else
    VME_d32w(handle, reg1,   (0x40 | bestfit));
  
  DEBUG_TEXT(DFDB_RF2TTC, 15, "RF2TTC_CalOrbit: Done");
  return(0); 
}


/*****************************************/
u_int RF2TTC_CalVolt(u_int dac, u_int mode)
/*****************************************/
{
  u_int fifowasfull, bestfit, reg1, reg2, reg3, min, max, value, sample, vthr, sync[DACRANGE];

  DEBUG_TEXT(DFDB_RF2TTC, 15, "RF2TTC_CalVolt: Called");

  //introduce "dac" here    
  if (mode == LAB_MODE)
  {
    VME_d32w(handle, (u_int)&regs->bc_delay25_bc1,         0x40);
    VME_d32w(handle, (u_int)&regs->bc_delay25_bc2,         0x40);
    VME_d32w(handle, (u_int)&regs->bc_delay25_bcref,       0x40);
    VME_d32w(handle, (u_int)&regs->bc_delay25_bcmain,      0x40);
    VME_d32w(handle, (u_int)&regs->orbin_delay25_orb1,     0x40);
    VME_d32w(handle, (u_int)&regs->orbin_delay25_orb2,     0x40);
    VME_d32w(handle, (u_int)&regs->orbout_delay25_orb1,    0x40);
    VME_d32w(handle, (u_int)&regs->orbout_delay25_orb2,    0x40);
    VME_d32w(handle, (u_int)&regs->orbout_delay25_orbmain, 0x40);
    VME_d32w(handle, (u_int)&regs->working_mode,           0);
    VME_d32w(handle, (u_int)&regs->bc1_man_select,         0);
    VME_d32w(handle, (u_int)&regs->bc2_man_select,         0);
  }
  
  //introduce "dac" here    
  VME_d32w(handle, (u_int)&regs->orb1_man_select,       0);
  VME_d32w(handle, (u_int)&regs->orb2_man_select,       0);
  VME_d32w(handle, (u_int)&regs->period_counter_enable, 7);
  
  if (dac == 1)
  {
    reg1 = (u_int)&regs->orb1_dac;
    reg2 = (u_int)&regs->orb1_period_fifo_rd;
    reg3 = (u_int)&regs->orb1_period_fifo_status;
  }
  else
  {
    reg1 = (u_int)&regs->orb2_dac;
    reg2 = (u_int)&regs->orb2_period_fifo_rd;
    reg3 = (u_int)&regs->orb2_period_fifo_status;
  }
 
  for (vthr = 0x80; vthr < 0x100; vthr++)
  {    
    VME_d32w(handle, reg1, vthr);
    VME_d32w(handle, (u_int)&regs->period_counter_reset, 3);

    ts_delay(80000); //delay 80ms
    
    min = 0xffff;
    max = 0;
    sample = 0;
    while (1)
    {
      if (sample == 0)
      {
        VME_d32r(handle, reg3, &value);
        if (value != 2)
        {
          DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_CalVolt: WARNING: FIFO not full for voltage threshold " << vthr);
          DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_CalVolt: WARNING: orb_period_fifo_status contains " << value);
	  fifowasfull = 0;
        } 
	else
	  fifowasfull = 1;	
      }	
       
      VME_d32r(handle, reg2, &value);
	     
      if (value & 0x4000)
      {
        DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_CalVolt: INFO: FIFO empty after " << sample << " samples for voltage threshold " << vthr);
        break;
      }
      
      if (sample != 0)  //The first sample contains invalid data
      {
        value &= 0x3fff;
        if (value > max) max = value;
        if (value < min) min = value;
      }	
      sample++;
    }

    sync[vthr] = 0;
    if (sample > 1) 
      DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_CalVolt: DAC " << dac << ": min=" << min << ", max=" << max << " for voltage threshold = " << vthr << " (" << sample << " samples)");
    
    if (sample > FIFOSIZE)
    {
      DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_CalVolt: ERROR: The FIFO contained " << sample << " samples");
      return(RCC_ERROR_RETURN(0, RF2TTC_FIFOOVERFLOW)); 
    }
    
    if ((max == min) && fifowasfull)
    {
      DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_CalVolt: DAC " << dac << " is synchronized for voltage threshold = " << vthr);
      sync[vthr] = 1;
    }
  }
  
  bestfit = RF2TTC_FindCalib(sync, DACRANGE, mode);
  
  if (bestfit == INVALID)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_CalVolt: ERROR: Calibration failed");
    DEBUG_TEXT(DFDB_RF2TTC, 15, "RF2TTC_CalVolt: Done");
    return(RCC_ERROR_RETURN(0, RF2TTC_NOCALIB)); 
  }

  DEBUG_TEXT(DFDB_RF2TTC, 5, "RF2TTC_CalVolt: Best voltage threshold: " << bestfit);

  if (mode == TEST_MODE)
    printf("Best voltage threshold: 0x%08x\n", bestfit);
  else
    VME_d32w(handle, reg1, bestfit);
  
  DEBUG_TEXT(DFDB_RF2TTC, 15, "RF2TTC_CalVolt: Done");
  return(0); 
}


/*********************************************************/
u_int RF2TTC_FindCalib(u_int *data, u_int size, u_int mode)
/*********************************************************/
{
  u_int index, first = INVALID, bfirst = INVALID, cont = 0, contmax = 0;
  
  DEBUG_TEXT(DFDB_RF2TTC, 15, "RF2TTC_FindCalib: Called");
  for (index = 0; index < size; index++)
  {
    DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_FindCalib: data[" << index << "] = " << data[index]);
    if (data[index])
    {
      if (cont == 0)
      {
        first = index;
        DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_FindCalib: Synchronized zone starts at " << first);
	cont = 1;
      }
      else
      {
        cont++;
        DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_FindCalib: Synchronized zone continues");
      }	
      if (cont >= contmax)
      {
        contmax = cont;
        DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_FindCalib: New longest zone ends at " << index);
	bfirst = first;
      }
    }
    else
      cont = 0;
  }
  
  DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_FindCalib: bfirst = " << bfirst);
  DEBUG_TEXT(DFDB_RF2TTC, 20, "RF2TTC_FindCalib: contmax = " << contmax);
  if (mode == TEST_MODE)
    printf("The calibrated zone starts for delay = 0x%08x and continues for %d steps\n", bfirst, contmax);

  return(bfirst + (contmax >> 1));
}














