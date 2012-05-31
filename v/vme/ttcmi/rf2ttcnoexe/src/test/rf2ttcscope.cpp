// $Id: rf2ttcscope.cpp,v 1.17 2008/05/07 08:13:41 joos Exp $
/****************************************************************/
/*                                              		*/
/*  file: rf2ttcscope.cpp                            		*/
/*                                              		*/
/*  rf2ttcscope decodes and displays the contents  		*/
/*  of the registers of RF2TTC boards   			*/
/*                                              		*/
/*  Author: Markus Joos, CERN-ECP               		*/
/*                                              		*/
/*  28.Jun.06  MAJO  created                   		  	*/
/*                                              		*/
/********C 2007 - A nickel program worth a dime******************/

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
#include "rf2ttc.h"
#include "vme_glue.h"

// prototypes
void mainhelp(void);
void regdecode(void);
void bcdecode(u_int mode, u_int *val);
void orbdecode(u_int mode, u_int *val);
void resetmenu(void);
void testmenu(void);
void commandmenu(void);
void setreg(void);
void checkstruct(void);
void vpeek(void);
void vpoke(void);
void cal_orb(void);
void cal_volt(void);

// Globals
u_int rf2ttchandle, slot = 99, vmea32base = 0xffffffff, verbose = 0;
rf2ttc_regs_t *rf2ttc_regs;


// Macros
#define ISOK  if (ret) {rcc_error_print(stdout, ret); exit(-1);}
#define NREGS 75
#define PRTC  {char cc[9]; printf("Press<return> to continue\n"); fgets(cc, 9, stdin);}


// Constants
#define HELPER_RF2TTC 0x0200000 //VME base address of a second RF2TTC requried for testing a RF2TTC
#define HELPER_TTCVI  0xff0000  //VME base address of a TTCvi requried for testing a RF2TTC


/*****************************/
int main(int argc, char **argv)
/*****************************/
{
  u_int ret, fun = 1, adok = 0;
  int c;

  while ((c = getopt(argc, argv, "vb:s:")) != -1)
  {
    switch (c) 
    {
      case 'b': 
        if (adok)
	{
	  printf("You must not use both -b and -s in the command line\n");
	  exit(-1);	
	}
        if (sscanf(optarg, "%x", &vmea32base) != 1)
        {
	  printf("Failed to decode the base address\n");
	  exit(-1);
	}  
	adok = 1;
	break;

      case 's': 
        if (adok)
	{
	  printf("You must not use both -b and -s in the command line\n");
	  exit(-1);	
	}
        slot = strtol(optarg, 0, 10); 
	adok = 1;
	break;                   

      case 'v': 
        verbose = 1;                     
	break;

      default:
	printf("Invalid option %c\n", c);
	printf("Usage: %s  [options]: \n", argv[0]);
	printf("Valid options are ...\n");
	printf("-b <VME A32 base address>: The hexadecimal A32 base address of the RF2TTC\n");
	printf("-s <slot number>:          The number of the VMEbus slot (1..21) that contains the RF2TTC\n");
	printf("-v:                        Verbose mode for debugging\n");
	printf("\n");
	exit(-1);
    }
  }

  printf("\n\n\n");
  printf("This is RF2TTCSCOPE\n");
  printf("===================\n");

  if (verbose)
    setdebug(99, 0);
  
  if (!adok)
  {
    printf("Enter the VMEbus A32 base address of the RF2TTC or 0xffffffff\n");
    printf("if you want to use geographical addressing: ");
    vmea32base = gethexd(0xffffffff);
  }

  if ((vmea32base & 0xffff) && (vmea32base != 0xffffffff))
  {
    printf("Sorry, the A32 base addess (0x%08x) is not a multiple of 64 kB\n", vmea32base);
    exit(-1);
  }

  if ((vmea32base == 0xffffffff) && (slot == 99))
  {
    printf("Enter the slot number of the RF2TTC: ");
    slot = getdecd(3);
  }
  
  if ((vmea32base == 0xffffffff) && ((slot < 1) || (slot > 21)))
  {
    printf("Sorry, slot number %d does not make sense\n", slot); 
    exit(-1);
  } 
  
  ret = ts_open(1, TS_DUMMY); ISOK
  
  if (slot != 99)
    RF2TTC_Slot2VME(slot, &vmea32base);
  printf("VMEbus base address = 0x%08x\n", vmea32base);
    
  ret = RF2TTC_Open(vmea32base); ISOK
  
  RF2TTC_GetHandle(&rf2ttchandle);  
  if (ret)
  {
    rcc_error_print(stdout, ret);
    exit(-1);  
  }

  rf2ttc_regs = (rf2ttc_regs_t *)0;
  
  while(fun != 0)
  {
    printf("\n");
    printf("Select an option:\n");
    printf("   1 Help\n");
    printf("   2 Raw register dump\n");
    printf("   3 Decode registers\n");
    printf("   4 Resets\n");
    printf("   5 Commands\n");
    printf("   6 Modify a register\n");
    printf("   7 Calibrate the orbit\n");
    printf("   8 Calibrate the voltage threshold\n");
    printf("   9 VME peek\n");
    printf("  10 VME poke\n");
    printf("  11 Check structure\n");
    printf("  12 Test a card\n");
    printf("  13 Set debugging parameters\n");
    printf("   0 Quit\n");
    printf("Your choice: ");
    fun = getdecd(fun);
    if (fun == 1) mainhelp();
    if (fun == 2) 
    {  
      printf("====================================================================\n");
      ret = RF2TTC_Dump(); ISOK
      printf("====================================================================\n");
    }
    if (fun == 3) regdecode();
    if (fun == 4) resetmenu();
    if (fun == 5) commandmenu();
    if (fun == 6) setreg();
    if (fun == 7) cal_orb();
    if (fun == 8) cal_volt();
    if (fun == 9) vpeek();
    if (fun == 10) vpoke();
    if (fun == 11) checkstruct();
    if (fun == 12) testmenu();
    if (fun == 13) setdebug(99, 0);
  }

  ret = RF2TTC_Close(); ISOK
  ret = ts_close(TS_DUMMY); ISOK
}


/****************/
void cal_orb(void)
/****************/
{
  static u_int mode = 0, orbit = 1;
  u_int ret;
    
  printf("Select the target (1=Orbit 1  2=Orbit 2): ");
  orbit = getdecd(orbit);

  printf("Select the mode (0=laboratory test  1=board is initialized): ");
  mode = getdecd(mode);

  ret = RF2TTC_CalOrbit(orbit, mode);
  if (ret)
    rcc_error_print(stdout, ret);
}


/*****************/
void cal_volt(void)
/*****************/
{
  static u_int mode = 0, dac = 1;
  u_int ret;
  
  printf("Select the target (1=DAC 1  2=DAC 2): ");
  dac = getdecd(dac);

  printf("Select the mode (0=laboratory test  1=board is initialized): ");
  mode = getdecd(mode);

  ret = RF2TTC_CalVolt(dac, mode);
  if (ret)
    rcc_error_print(stdout, ret);
}


/********************/
void checkstruct(void)
/********************/
{
  rf2ttc_regs_t *rp;
  rp = (rf2ttc_regs_t *) 0;

  printf("manufacturer_id            is at 0x%08x\n", (u_int)&rp->manufacturer_id);
  printf("board_id                   is at 0x%08x\n", (u_int)&rp->board_id);
  printf("revision_id                is at 0x%08x\n", (u_int)&rp->revision_id);
  printf("program_id                 is at 0x%08x\n", (u_int)&rp->program_id);
  printf("bset                       is at 0x%08x\n", (u_int)&rp->bset);
  printf("bclear                     is at 0x%08x\n", (u_int)&rp->bclear);
  printf("bc_delay25_bc1             is at 0x%08x\n", (u_int)&rp->bc_delay25_bc1);
  printf("bc_delay25_bc2             is at 0x%08x\n", (u_int)&rp->bc_delay25_bc2);
  printf("bc_delay25_bcref           is at 0x%08x\n", (u_int)&rp->bc_delay25_bcref);
  printf("bc_delay25_bcmain          is at 0x%08x\n", (u_int)&rp->bc_delay25_bcmain);
  printf("bc_delay25_gcr             is at 0x%08x\n", (u_int)&rp->bc_delay25_gcr);
  printf("orbin_delay25_orb1         is at 0x%08x\n", (u_int)&rp->orbin_delay25_orb1);
  printf("orbin_delay25_orb2         is at 0x%08x\n", (u_int)&rp->orbin_delay25_orb2);
  printf("orbin_delay25_gcr          is at 0x%08x\n", (u_int)&rp->orbin_delay25_gcr);
  printf("orbout_delay25_orb1        is at 0x%08x\n", (u_int)&rp->orbout_delay25_orb1);
  printf("orbout_delay25_orb2        is at 0x%08x\n", (u_int)&rp->orbout_delay25_orb2);
  printf("orbout_delay25_orbmain     is at 0x%08x\n", (u_int)&rp->orbout_delay25_orbmain);
  printf("orbout_delay25_gcr         is at 0x%08x\n", (u_int)&rp->orbout_delay25_gcr);
  printf("delay25_reg                is at 0x%08x\n", (u_int)&rp->delay25_reg);
  printf("ttcrx_pointer              is at 0x%08x\n", (u_int)&rp->ttcrx_pointer);
  printf("ttcrx_data                 is at 0x%08x\n", (u_int)&rp->ttcrx_data);
  printf("ttcrx_reg                  is at 0x%08x\n", (u_int)&rp->ttcrx_reg);
  printf("orb_counter_reset          is at 0x%08x\n", (u_int)&rp->orb_counter_reset);
  printf("period_counter_reset       is at 0x%08x\n", (u_int)&rp->period_counter_reset);
  printf("orb_int_reset              is at 0x%08x\n", (u_int)&rp->orb_int_reset);
  printf("period_counter_enable      is at 0x%08x\n", (u_int)&rp->period_counter_enable);
  printf("orb_counter_enable         is at 0x%08x\n", (u_int)&rp->orb_counter_enable);
  printf("orb_int_enable             is at 0x%08x\n", (u_int)&rp->orb_int_enable);
  printf("working_mode               is at 0x%08x\n", (u_int)&rp->working_mode);
  printf("beam_no_beam_def           is at 0x%08x\n", (u_int)&rp->beam_no_beam_def);
  printf("bst_machine_mode           is at 0x%08x\n", (u_int)&rp->bst_machine_mode);
  printf("ttcrx_status               is at 0x%08x\n", (u_int)&rp->ttcrx_status);
  printf("orbmain_period_fifo_rd     is at 0x%08x\n", (u_int)&rp->orbmain_period_fifo_rd);
  printf("orbmain_period_fifo_status is at 0x%08x\n", (u_int)&rp->orbmain_period_fifo_status);
  printf("orbmain_period_rd          is at 0x%08x\n", (u_int)&rp->orbmain_period_rd);
  printf("orbmain_counter            is at 0x%08x\n", (u_int)&rp->orbmain_counter);
  printf("orbmain_int_period_counter is at 0x%08x\n", (u_int)&rp->orbmain_int_period_counter);
  printf("orbmain_int_period_set     is at 0x%08x\n", (u_int)&rp->orbmain_int_period_set);
  printf("orbmain_length             is at 0x%08x\n", (u_int)&rp->orbmain_length);
  printf("orbmain_course_delay       is at 0x%08x\n", (u_int)&rp->orbmain_course_delay);
  printf("orbmain_polarity           is at 0x%08x\n", (u_int)&rp->orbmain_polarity);
  printf("orbmain_nobeam_select      is at 0x%08x\n", (u_int)&rp->orbmain_nobeam_select);
  printf("orbmain_beam_select        is at 0x%08x\n", (u_int)&rp->orbmain_beam_select);
  printf("orbmain_man_select         is at 0x%08x\n", (u_int)&rp->orbmain_man_select);
  printf("orb2_dac                   is at 0x%08x\n", (u_int)&rp->orb2_dac);
  printf("orb2_period_fifo_rd        is at 0x%08x\n", (u_int)&rp->orb2_period_fifo_rd);
  printf("orb2_period_fifo_status    is at 0x%08x\n", (u_int)&rp->orb2_period_fifo_status);
  printf("orb2_period_rd             is at 0x%08x\n", (u_int)&rp->orb2_period_rd);
  printf("orb2_counter               is at 0x%08x\n", (u_int)&rp->orb2_counter);
  printf("orb2_int_period_counter    is at 0x%08x\n", (u_int)&rp->orb2_int_period_counter);
  printf("orb2_int_period_set        is at 0x%08x\n", (u_int)&rp->orb2_int_period_set);
  printf("orb2_length                is at 0x%08x\n", (u_int)&rp->orb2_length);
  printf("orb2_course_delay          is at 0x%08x\n", (u_int)&rp->orb2_course_delay);
  printf("orb2_polarity              is at 0x%08x\n", (u_int)&rp->orb2_polarity);
  printf("orb2_nobeam_select         is at 0x%08x\n", (u_int)&rp->orb2_nobeam_select);
  printf("orb2_beam_select           is at 0x%08x\n", (u_int)&rp->orb2_beam_select);
  printf("orb2_man_select            is at 0x%08x\n", (u_int)&rp->orb2_man_select);
  printf("orb1_dac                   is at 0x%08x\n", (u_int)&rp->orb1_dac);
  printf("orb1_period_fifo_rd        is at 0x%08x\n", (u_int)&rp->orb1_period_fifo_rd);
  printf("orb1_period_fifo_status    is at 0x%08x\n", (u_int)&rp->orb1_period_fifo_status);
  printf("orb1_period_rd             is at 0x%08x\n", (u_int)&rp->orb1_period_rd);
  printf("orb1_counter               is at 0x%08x\n", (u_int)&rp->orb1_counter);
  printf("orb1_int_period_counter    is at 0x%08x\n", (u_int)&rp->orb1_int_period_counter);
  printf("orb1_int_period_set        is at 0x%08x\n", (u_int)&rp->orb1_int_period_set);
  printf("orb1_length                is at 0x%08x\n", (u_int)&rp->orb1_length);
  printf("orb1_course_delay          is at 0x%08x\n", (u_int)&rp->orb1_course_delay);
  printf("orb1_polarity              is at 0x%08x\n", (u_int)&rp->orb1_polarity);
  printf("orb1_nobeam_select         is at 0x%08x\n", (u_int)&rp->orb1_nobeam_select);
  printf("orb1_beam_select           is at 0x%08x\n", (u_int)&rp->orb1_beam_select);
  printf("orb1_man_select            is at 0x%08x\n", (u_int)&rp->orb1_man_select);
  printf("bcmain_qpll_status         is at 0x%08x\n", (u_int)&rp->bcmain_qpll_status);
  printf("bcmain_qpll_mode           is at 0x%08x\n", (u_int)&rp->bcmain_qpll_mode);
  printf("bcmain_nobeam_select       is at 0x%08x\n", (u_int)&rp->bcmain_nobeam_select);
  printf("bcmain_beam_select         is at 0x%08x\n", (u_int)&rp->bcmain_beam_select);
  printf("bcmain_man_select          is at 0x%08x\n", (u_int)&rp->bcmain_man_select);
  printf("bcref_qpll_status          is at 0x%08x\n", (u_int)&rp->bcref_qpll_status);
  printf("bcref_daq                  is at 0x%08x\n", (u_int)&rp->bcref_daq);
  printf("bcref_qpll_mode            is at 0x%08x\n", (u_int)&rp->bcref_qpll_mode);
  printf("bcref_nobeam_select        is at 0x%08x\n", (u_int)&rp->bcref_nobeam_select);
  printf("bcref_beam_select          is at 0x%08x\n", (u_int)&rp->bcref_beam_select);
  printf("bcref_man_select           is at 0x%08x\n", (u_int)&rp->bcref_man_select);
  printf("bc2_qpll_status            is at 0x%08x\n", (u_int)&rp->bc2_qpll_status);
  printf("bc2_daq                    is at 0x%08x\n", (u_int)&rp->bc2_daq);
  printf("bc2_qpll_mode              is at 0x%08x\n", (u_int)&rp->bc2_qpll_mode);
  printf("bc2_nobeam_select          is at 0x%08x\n", (u_int)&rp->bc2_nobeam_select);
  printf("bc2_beam_select            is at 0x%08x\n", (u_int)&rp->bc2_beam_select);
  printf("bc2_man_select             is at 0x%08x\n", (u_int)&rp->bc2_man_select);
  printf("bc1_qpll_status            is at 0x%08x\n", (u_int)&rp->bc1_qpll_status);
  printf("bc1_daq                    is at 0x%08x\n", (u_int)&rp->bc1_daq);
  printf("bc1_qpll_mode              is at 0x%08x\n", (u_int)&rp->bc1_qpll_mode);
  printf("bc1_nobeam_select          is at 0x%08x\n", (u_int)&rp->bc1_nobeam_select);
  printf("bc1_beam_select            is at 0x%08x\n", (u_int)&rp->bc1_beam_select);
  printf("bc1_man_select             is at 0x%08x\n", (u_int)&rp->bc1_man_select);
 
}


/***************/
void setreg(void)
/***************/
{
  static u_int offset = 0, rtype = 0;
  u_int ret, odata, ndata;
  
  printf("Enter the register type (0=VME  1=I2C): ");
  rtype = getdecd(rtype);
  
  if (rtype)
  {
    printf("Possible offsets for the delay registers: 0x7D000 to 0x7D054\n");
    printf("Possible offsets for the TTC registers:   0 to 28\n");
    printf("Enter the offset of the register: ");
  }
  else
    printf("Enter the VMEbus address offset of the register: ");

  offset = gethexd(offset);

  if (rtype)
  {
    ret = RF2TTC_I2C_Read(offset, &odata); 
    ISOK
  } 
  else
  {
    ret = VME_d32r(rf2ttchandle, offset, &odata); 
    ISOK
  }
  
  printf("The register currently contains 0x%08x\n", odata);
  printf("Enter the new data: ");
  ndata = gethexd(odata);
  
  if (rtype)
  {
    ret = RF2TTC_I2C_Write(offset, ndata); 
    ISOK
  }
  else
  {
    ret = VME_d32w(rf2ttchandle, offset, ndata); 
    ISOK
  }
}


/**************/
void vpeek(void)
/**************/
{
  static u_int offset = 0;
  u_int ret, odata;
  
  printf("Enter the VMEbus offset of the register: ");
  offset = gethexd(offset);

  ret = VME_d32r(rf2ttchandle, offset, &odata); ISOK
  printf("The register contains 0x%08x\n", odata);
}


/**************/
void vpoke(void)
/**************/
{
  static u_int offset = 0, ndata = 0;
  u_int ret;
  
  printf("Enter the VMEbus offset of the register: ");
  offset = gethexd(offset);

  printf("Enter the data to write: ");
  ndata = gethexd(ndata);
  
  ret = VME_d32w(rf2ttchandle, offset, ndata); ISOK
  printf("The new data has been written.\n");
}


/******************/
void resetmenu(void)
/******************/
{
  u_int ret, fun = 1;
  
  while(fun != 0)
  {
    printf("\n");
    printf("Select an option:\n");
    printf("   1  Reset the board\n");
    printf("   2  Reset the BC and Orbit delay25 chips\n");
    printf("   3  Reset the QPLL\n");
    printf("   4  Reset the orbit pulse counters\n");
    printf("   5  Reset the orbit period counters\n");
    printf("   6  Reset the internal orbit counters\n");
    printf("   7  Reset the TTCrx\n");
    printf("   0  Quit\n");
    printf("Your choice: ");
    fun = getdecd(fun);

    if (fun == 1) 
    {
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bset, 0x8); ISOK          
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bclear, 0x8); ISOK        
      printf("Reset done\n");
    }     

    if (fun == 2) 
    {
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bset, 0x1); ISOK          
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bclear, 0x1); ISOK        
      printf("Reset done\n");
    }

    if (fun == 3) 
    {
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bset, 0x2); ISOK          
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bclear, 0x2); ISOK        
      printf("Reset done\n");
    }

    if (fun == 4) 
    {
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb_counter_reset, 1); ISOK     
      printf("Reset done\n");
    }

    if (fun == 5) 
    {
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->period_counter_reset, 1); ISOK 
      printf("Reset done\n");
    }

    if (fun == 6) 
    {
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb_int_reset, 1); ISOK        
      printf("Reset done\n");
    }
    
    if (fun == 7) 
    {
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bset, 0x4); ISOK          
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bclear, 0x4); ISOK        
      printf("Reset done\n");
    }
  }
}


/********************/
void commandmenu(void)
/********************/
{
  static u_int data = 0;
  u_int ret, fun = 1;
  
  while(fun != 0)
  {
    printf("\n");
    printf("Select an option:\n");
    printf("   1  Enable or disable orbit period counters\n");
    printf("   2  Enable or disable orbit pulse counters\n");
    printf("   3  Enable or disable the initernal orbit counters\n");
    printf("   0  Quit\n");
    printf("Your choice: ");
    fun = getdecd(fun);
    if (fun == 1) 
    {
      printf("Bit 0: Enable Orbit 1 period counter\n");
      printf("Bit 1: Enable Orbit 2 period counter\n");
      printf("Bit 2: Enable Main orbit period counter\n");
      printf("Enter the data (0..7): ");
      data = getdecd(data);
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->period_counter_enable, data); ISOK
      printf("Done!\n");
    }
    if (fun == 2) 
    {
      printf("Bit 0: Enable Orbit 1 pulse counter\n");
      printf("Bit 1: Enable Orbit 2 pulse counter\n");
      printf("Bit 2: Enable Main orbit pulse counter\n");
      printf("Enter the data (0..7): ");
      data = getdecd(data);
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb_counter_enable, data); ISOK
      printf("Done!\n");
    }
    if (fun == 3) 
    {
      printf("Bit 0: Enable Orbit 1 internal counter\n");
      printf("Bit 1: Enable Orbit 2 internal counter\n");
      printf("Bit 2: Enable Main orbit internal counter\n");
      printf("Enter the data (0..7): ");
      data = getdecd(data);
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb_int_enable, data); ISOK
      printf("Done!\n");
    }
  }
}


/******************/
void regdecode(void)
/******************/
{
  u_int ret, vdata, val[14];

  printf("====================================================================\n");

  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->manufacturer_id, &vdata); ISOK
  printf("Manufacturer ID:                        0x%08x\n", vdata);

  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->board_id, &vdata); ISOK
  printf("Board ID:                               0x%08x\n", vdata);
  
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->revision_id, &vdata); ISOK
  printf("Revision ID:                            0x%08x\n", vdata);
  
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->program_id, &vdata); ISOK
  if (ret) {rcc_error_print(stdout, ret); exit(-1);}  
  printf("Program ID:                             0x%08x\n", vdata);

  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_qpll_status, &vdata);   val[1] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_daq, &vdata);           val[2] = vdata & 0xff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_qpll_mode, &vdata);     val[3] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_nobeam_select, &vdata); val[4] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_beam_select, &vdata);   val[5] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_man_select, &vdata);    val[6] = vdata & 0x1; ISOK
  bcdecode(1, val);

  
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_qpll_status, &vdata);   val[1] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_daq, &vdata);           val[2] = vdata & 0xff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_qpll_mode, &vdata);     val[3] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_nobeam_select, &vdata); val[4] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_beam_select, &vdata);   val[5] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_man_select, &vdata);    val[6] = vdata & 0x1; ISOK
  bcdecode(2, val);

  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_qpll_status, &vdata);   val[1] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_daq, &vdata);           val[2] = vdata & 0xff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_qpll_mode, &vdata);     val[3] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_nobeam_select, &vdata); val[4] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_beam_select, &vdata);   val[5] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_man_select, &vdata);    val[6] = vdata & 0x1; ISOK
  bcdecode(3, val);

  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_qpll_status, &vdata);   val[1] = vdata & 0x1; ISOK
  val[2] = 0;
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_qpll_mode, &vdata);     val[3] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_nobeam_select, &vdata); val[4] = vdata & 0x2; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_beam_select, &vdata);   val[5] = vdata & 0x2; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_man_select, &vdata);    val[6] = vdata & 0x2; ISOK
  bcdecode(4, val);
  
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_dac, &vdata);                val[1]  = vdata & 0xff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_period_fifo_rd, &vdata);     val[2]  = vdata & 0xfff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_period_fifo_status, &vdata); val[3]  = vdata & 0x3; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_period_rd, &vdata);          val[4]  = vdata & 0xfff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_counter, &vdata);            val[5]  = vdata; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_int_period_counter, &vdata); val[6]  = vdata & 0xfff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_int_period_set, &vdata);     val[7]  = vdata & 0xfff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_length, &vdata);             val[8]  = vdata & 0xff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_course_delay, &vdata);       val[9]  = vdata & 0xfff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_polarity, &vdata);           val[10] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_nobeam_select, &vdata);      val[11] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_beam_select, &vdata);        val[12] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_man_select, &vdata);         val[13] = vdata & 0x1; ISOK
  orbdecode(1, val);
  
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_dac, &vdata);                val[1]  = vdata & 0xff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_period_fifo_rd, &vdata);     val[2]  = vdata & 0xfff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_period_fifo_status, &vdata); val[3]  = vdata & 0x3; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_counter, &vdata);            val[5]  = vdata; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_int_period_counter, &vdata); val[6]  = vdata & 0xfff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_int_period_set, &vdata);     val[7]  = vdata & 0xfff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_length, &vdata);             val[8]  = vdata & 0xff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_course_delay, &vdata);       val[9]  = vdata & 0xfff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_polarity, &vdata);           val[10] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_nobeam_select, &vdata);      val[11] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_beam_select, &vdata);        val[12] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_man_select, &vdata);         val[13] = vdata & 0x1; ISOK
  orbdecode(2, val);  
  
  val[1] = 0;
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_period_fifo_rd, &vdata);     val[2]  = vdata & 0xfff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_period_fifo_status, &vdata); val[3]  = vdata & 0x3; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_period_rd, &vdata);          val[4]  = vdata & 0xfff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_counter, &vdata);            val[5]  = vdata; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_int_period_counter, &vdata); val[6]  = vdata & 0xfff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_int_period_set, &vdata);     val[7]  = vdata & 0xfff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_length, &vdata);             val[8]  = vdata & 0xff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_course_delay, &vdata);       val[9]  = vdata & 0xfff; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_polarity, &vdata);           val[10] = vdata & 0x1; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_nobeam_select, &vdata);      val[11] = vdata & 0x3; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_beam_select, &vdata);        val[12] = vdata & 0x3; ISOK
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_man_select, &vdata);         val[13] = vdata & 0x3; ISOK
  orbdecode(3, val);

  printf("\nDuming the delay registers\n");
  
  ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->bc_delay25_bc1, &vdata); ISOK
  printf("BC delay 25 BC1:                        0x%02x\n", vdata & 0xff);

  ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->bc_delay25_bc2, &vdata); ISOK
  printf("BC delay 25 BC2:                        0x%02x\n", vdata & 0xff);
 
  ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->bc_delay25_bcref, &vdata); ISOK
  printf("BC delay 25 BCref:                      0x%02x\n", vdata & 0xff);
 
  ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->bc_delay25_bcmain, &vdata); ISOK
  printf("BC delay 25 BCmain:                     0x%02x\n", vdata & 0xff);
 
  ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->bc_delay25_gcr, &vdata); ISOK
  printf("BC delay 25 GCR:                        0x%02x\n", vdata & 0xff);
 
  ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbin_delay25_orb1, &vdata); ISOK
  printf("Orbit in delay 25 ORB1:                 0x%02x\n", vdata & 0xff);

  ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbin_delay25_orb2, &vdata); ISOK
  printf("Orbit in delay 25 ORB2:                 0x%02x\n", vdata & 0xff);
 
  ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbin_delay25_gcr, &vdata); ISOK
  printf("Orbit in delay 25 GCR:                  0x%02x\n", vdata & 0xff);
 
  ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbout_delay25_orb1, &vdata); ISOK
  printf("Orbit out delay 25 ORB1:                0x%02x\n", vdata & 0xff);
 
  ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbout_delay25_orb2, &vdata); ISOK
  printf("Orbit out delay 25 ORB2:                0x%02x\n", vdata & 0xff);

  ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbout_delay25_orbmain, &vdata); ISOK
  printf("Orbit out delay 25 ORBmain:             0x%02x\n", vdata & 0xff);

  ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbout_delay25_gcr, &vdata); ISOK
  printf("Orbit out delay 25 GCR:                 0x%02x\n", vdata & 0xff);

  printf("\nDecoding the Working Modes registers\n");
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->working_mode, &val[0]); ISOK
  printf("Output BC1 :                 %s\n", (val[0] & 0x1)?"Automatic":"Manual");
  printf("Output BC2:                  %s\n", (val[0] & 0x2)?"Automatic":"Manual");
  printf("Output BCref:                %s\n", (val[0] & 0x4)?"Automatic":"Manual");
  printf("Output BCmain:               %s\n", (val[0] & 0x8)?"Automatic":"Manual");
  printf("Output ORB1:                 %s\n", (val[0] & 0x10)?"Automatic":"Manual");
  printf("Output ORB2:                 %s\n", (val[0] & 0x20)?"Automatic":"Manual");
  printf("Output ORBmain:              %s\n", (val[0] & 0x40)?"Automatic":"Manual");

  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->beam_no_beam_def, &val[0]); ISOK 
  printf("The beam_no_beam_def register contains: 0x%08x\n", val[0]);
  
  printf("\nDecoding the BST registers\n");
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bst_machine_mode, &val[0]); ISOK  
  printf("The bst_machine_mode register contains: 0x%08x\n", val[0]);
  
  ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->ttcrx_status, &val[0]); ISOK  
  printf("The ttcrx_status register contains: 0x%08x\n", val[0]);
  printf("====================================================================\n");
}


/************************************/
void orbdecode(u_int mode, u_int *val)
/************************************/
{
  if (mode == 1)
    printf("\nDecoding ORB1 registers\n");
  else if (mode == 2)
    printf("\nDecoding ORB2 registers\n");
  else
    printf("\nDecoding Mainorb registers\n");
    
  if (mode != 3)  
    printf("Input comparator threshold voltage:                  %f V\n", -1.25 + val[1] * 2.5 / 256.0);
    
  printf("Oldest entry in the period FIFO:                     %d\n", val[2]);
  printf("Status of period FIFO:                               %d\n", val[3]); 
  printf("Period between last 2 orbit pulses:                  %d\n", val[4]);
  printf("Number of orbit signals:                             %d\n", val[5]);
  printf("Value of BC counter used to generate internal orbit: %d\n", val[6]);   
  printf("Period of internal orbit generator:                  %d\n", val[7]); 
  printf("Orbit pulse stretching:                              %d ns\n", 25 * val[8]); 
  if (val[9] < 3564)
    printf("Orbit pulse shift:                                   %d ns\n", 25 * val[9]);
  else  
    printf("Orbit pulse shift:                                   Out of range\n");
  printf("Orbit polarity:                                      %s\n", val[10]?"Inverted":"Not inverted");

  if (mode < 3)  
    printf("Orbit output source in no beam mode:                 %s\n", val[11]?"Internal orbit":"External orbit");
  else
  {
    printf("Orbit output source in no beam mode:                 ");
    if (val[11] == 0)
      printf("Orbit 1\n");
    else if (val[11] == 1)
      printf("Orbit 2\n");
    else 
      printf("Internal orbit\n");
  }

  if (mode < 3)  
    printf("Orbit output source in beam mode:                    %s\n", val[12]?"Internal orbit":"External orbit");
  else
  {
    printf("Orbit output source in beam mode:                    ");
    if (val[12] == 0)
      printf("Orbit 1\n");
    else if (val[12] == 1)
      printf("Orbit 2\n");
    else 
      printf("Internal orbit\n");
  }

  if (mode < 3)  
    printf("Orbit output source in manual mode:                  %s\n", val[13]?"Internal orbit":"External orbit");
  else
  {
    printf("Orbit output source in manual mode:                  ");
    if (val[13] == 0)
      printf("Orbit 1\n");
    else if (val[13] == 1)
      printf("Orbit 2\n");
    else 
      printf("Internal orbit\n");
  }
}


/***********************************/
void bcdecode(u_int mode, u_int *val)
/***********************************/
{
  if (mode == 1)
    printf("\nDecoding BC1 registers\n");
  else if (mode == 2)
    printf("\nDecoding BC2 registers\n");
  else if (mode == 3)
    printf("\nDecoding BCref registers\n");
  else
    printf("\nDecoding mainBC registers\n");
  
  if (mode < 4)  
    printf("BC source in manual mode:     %s\n", val[6]?"LHC BC":"Internal 40.078 MHz");
  else
  {
    printf("BC source in manual mode:     ");

    if (val[6] == 0)
      printf("Internal 40.078 MHz\n");
    else if (val[6] == 1)
      printf("LHC BCref\n");
    else if (val[6] == 2)
      printf("LHC BC2\n");
    else 
      printf("LHC BC1\n");
  }
  
  if (mode < 4)  
    printf("BC source in beam mode:       %s\n", val[5]?"LHC BC":"Internal 40.078 MHz");
  else
  {
    printf("BC source in beam mode:       ");

    if (val[5] == 0)
      printf("Internal 40.078 MHz\n");
    else if (val[5] == 1)
      printf("LHC BCref\n");
    else if (val[5] == 2)
      printf("LHC BC2\n");
    else 
      printf("LHC BC1\n");
  }

  if (mode < 4)  
    printf("BC source in no beam mode:    %s\n", val[4]?"LHC BC":"Internal 40.078 MHz");
  else
  {
    printf("BC source in no beam mode:    ");
    if (val[4] == 0)
      printf("Internal 40.078 MHz\n");
    else if (val[4] == 1)
      printf("LHC BCref\n");
    else if (val[4] == 2)
      printf("LHC BC2\n");
    else 
      printf("LHC BC1\n");
  }  
  
  printf("QPLL locking mode:            %s\n", val[3]?"relock if lock is lost":"relock after reset");
  if (mode != 4)
    printf("Input comparator threshold:   %f V\n", -1.25 + val[2] * 2.5 / 256.0);
  printf("QPLL status:                  %s\n", val[1]?"locked":"error");
}


/*****************/
void mainhelp(void)
/*****************/
{
  printf("\n=================================================\n");
  printf("Call Markus Joos, 72364, 160663, if you need help\n");
  printf("=================================================\n\n");
}


/*****************/
void testmenu(void)
/*****************/
{
  // Action bits:
  // 1: 0->r/w       1->r/o
  // 2: 0->standard  1->I2C
  // 3: special 1 (Read twice)
  // 4: special 2 (check for data > 0)
  // 5: add 200 ms delay before the test
  // Actions:
  // 0 = r/w
  // 1 = r/o
  // 2 = r/w (I2C)
  // 5 = special
  // 9 = special

  typedef struct
  {
    u_int offset;
    u_int nbits;
    u_int action;
    u_int read_default;
    u_int write;
    u_int read_back;
  } one_reg_t;

  u_int reg, action, loop, bmask, vdata, vdata2, ret, fun = 1, helper_handle, vi_handle;

  one_reg_t all_regs[NREGS] = 
  {  
    {0x7FBFC,  1,  0,     0,      1,      1}, // BC1_MAN_SELECT
    {0x7FBF8,  1,  0,     1,      1,      1}, // BC1_BEAM_SELECT
    {0x7FBF4,  1,  0,     0,      1,      1}, // BC1_NOBEAM_SELECT
    {0x7FBF0,  1,  0,     1,      0,      0}, // BC1_QPLL_MODE
    {0x7FBE8,  2, 17,     1,      0,      0}, // BC1_QPLL_STATUS
    {0x7FBCC,  1,  0,     0,      1,      1}, // BC2_MAN_SELECT
    {0x7FBC8,  1,  0,     1,      1,      1}, // BC2_BEAM_SELECT
    {0x7FBC4,  1,  0,     0,      1,      1}, // BC2_NOBEAM_SELECT
    {0x7FBC0,  1,  0,     1,      0,      0}, // BC2_QPLL_MODE
    {0x7FBB8,  2, 17,     1,      0,      0}, // BC2_QPLL_STATUS
    {0x7FBAC,  1,  0,     0,      1,      1}, // BCref_MAN_SELECT
    {0x7FBA8,  1,  0,     1,      1,      1}, // BCref_BEAM_SELECT
    {0x7FBA4,  1,  0,     0,      1,      1}, // BCref_NOBEAM_SELECT
    {0x7FBA0,  1,  0,     1,      0,      0}, // BCref_QPLL_MODE
    {0x7FB98,  2, 17,     1,      0,      0}, // BCref_QPLL_STATUS
    {0x7FB8C,  2,  0,     0,      1,      1}, // BCmain_MAN_SELECT
    {0x7FB88,  2,  0,     1,      1,      1}, // BCmain_BEAM_SELECT
    {0x7FB84,  2,  0,     0,      1,      1}, // BCmain_NOBEAM_SELECT
    {0x7FB80,  1,  0,     1,      0,      0}, // BCmain_QPLL_MODE
    {0x7FB7C,  2,  1,     1,      0,      0}, // BCmain_QPLL_STATUS
    {0x7FB6C,  1,  0,     1,      0,      0}, // ORB1_MAN_SELECT
    {0x7FB68,  1,  0,     0,      1,      1}, // ORB1_BEAM_SELECT
    {0x7FB64,  1,  0,     1,      0,      0}, // ORB1_NOBEAM_SELECT
    {0x7FB60,  1,  0,     0,      1,      1}, // ORB1_POLARITY
    {0x7FB5C, 12,  0,     0,   0x10,   0x10}, // ORB1_COARSE_DELAY
    {0x7FB58,  8,  0,     0,    0x5,    0x5}, // ORB1_LENGTH
    {0x7FB54, 12,  0, 0xdec,   0x20,   0x20}, // ORB1_INT_PERIOD_SET
    {0x7FB50, 12, 25,     0,      0,      0}, // ORB1_INT_PERIOD_COUNTER
    {0x7FB4C, 32,  9,     0,      0,      0}, // ORB1_COUNTER
    {0x7FB48, 12,  1, 0xded,      0,      0}, // ORB1_PERIOD_RD
    {0x7FB44,  2,  1,     2,      0,      0}, // ORB1_PERIOD_FIFO_STATUS
    {0x7FB40, 16,  5, 0xded,      0,      0}, // ORB1_PERIOD_FIFO_RD
    {0x7FB3C,  8,  0,  0xAA,   0x22,   0x22}, // ORB1_DAC
    {0x7FB2C,  1,  0,     1,      0,      0}, // ORB2_MAN_SELECT
    {0x7FB28,  1,  0,     0,      1,      1}, // ORB2_BEAM_SELECT
    {0x7FB24,  1,  0,     1,      0,      0}, // ORB2_NOBEAM_SELECT
    {0x7FB20,  1,  0,     0,      1,      1}, // ORB2_POLARITY
    {0x7FB1C, 12,  0,     0,   0x10,   0x10}, // ORB2_COARSE_DELAY
    {0x7FB18,  8,  0,     0,    0x5,    0x5}, // ORB2_LENGTH
    {0x7FB14, 12,  0, 0xdec,   0x20,   0x20}, // ORB2_INT_PERIOD_SET
    {0x7FB10, 12, 25,     0,      0,      0}, // ORB2_INT_PERIOD_COUNTER
    {0x7FB0C, 32,  9,     0,      0,      0}, // ORB2_COUNTER
    {0x7FB08, 12,  1, 0xded,      0,      0}, // ORB2_PERIOD_RD
    {0x7FB04,  2,  1,     2,      0,      0}, // ORB2_PERIOD_FIFO_STATUS
    {0x7FB00, 16,  5, 0xded,      0,      0}, // ORB2_PERIOD_FIFO_RD
    {0x7FAFC,  8,  0,  0xAA,   0x22,   0x22}, // ORB2_DAC
    {0x7FAEC,  2,  0,     2,      0,      0}, // ORBmain_MAN_SELECT
    {0x7FAE8,  2,  0,     0,      1,      1}, // ORBmain_BEAM_SELECT
    {0x7FAE4,  2,  0,     2,      0,      0}, // ORBmain_NOBEAM_SELECT
    {0x7FAE0,  1,  0,     0,      1,      1}, // ORBmain_POLARITY
    {0x7FADC, 12,  0,     0,   0x10,   0x10}, // ORBmain_COARSE_DELAY
    {0x7FAD8,  8,  0,     0,    0x5,    0x5}, // ORBmain_LENGTH
    {0x7FAD4, 12,  0, 0xdec,   0x20,   0x20}, // ORBmain_INT_PERIOD_SET
    {0x7FAD0, 12, 25,     0,      0,      0}, // ORBmain_INT_PERIOD_COUNTER
    {0x7FACC, 32,  9,     0,      0,      0}, // ORBmain_COUNTER
    {0x7FAC8, 12,  1, 0xded,      0,      0}, // ORBmain_PERIOD_RD
    {0x7FAC4,  2,  1,     2,      0,      0}, // ORBmain_PERIOD_FIFO_STATUS
    {0x7FAC0, 16,  5, 0xded,      0,      0}, // ORBmain_PERIOD_FIFO_RD
    {0x7FAA0,  1,  1,     1,      0,      0}, // TTCrx_status
    {0x7FA9C, 32,  1,     0,      0,      0}, // BST_Machine_Mode
    {0x7FA7C, 32,  0,   0x8, 0xCAFE, 0xCAFE}, // BEAM_NO_BEAM_DEF
    {0x7FA78,  7,  0,     0,   0x7F,   0x7F}, // WORKING_MODE
    {0x7FA6C,  3,  0,   0x7,    0x0,    0x0}, // ORB_INT_ENABLE
    {0x7FA68,  3,  0,   0x7,    0x0,    0x0}, // ORB_COUNTER_ENABLE
    {0x7FA64,  3,  0,   0x7,    0x0,    0x0}, // PERIOD_COUNTER_ENABLE
    {0x00003,  9,  2, 0x1ff,   0xFF,  0x1FF}, // *TTCrx_CONTROL REGISTER
    {0x7D048,  9,  2, 0x140,   0x7F,  0x17F}, // *ORBOUT_DELAY25_ORBmain
    {0x7D044,  9,  2, 0x140,   0x7E,  0x17E}, // *ORBOUT_DELAY25_ORB2
    {0x7D040,  9,  2, 0x140,   0x7C,  0x17C}, // *ORBOUT_DELAY25_ORB1
    {0x7D024,  9,  2, 0x140,   0x7B,  0x17B}, // *ORBIN_DELAY25_ORB2
    {0x7D020,  9,  2, 0x140,   0x7A,  0x17A}, // *ORBIN_DELAY25_ORB1
    {0x7D00c,  9,  2, 0x140,   0x79,  0x179}, // *BC_DELAY25_BCmain
    {0x7D008,  9,  2, 0x140,   0x78,  0x178}, // *BC_DELAY25_BCref
    {0x7D004,  9,  2, 0x140,   0x77,  0x177}, // *BC_DELAY25_BC2
    {0x7D000,  9,  2, 0x140,   0x76,  0x176}  // *BC_DELAY25_BC1
  }; 
  
  //Set debug level to 5 to get error from the libraries
  setdebug(5, 309);
    
  //For some tests we need a second RF2TTC as a data generator
  //As the RF2TTC_Open() call only supports one card we map the helper 
  //card at a lower level.
  ret = VME_map(MODE_A32, HELPER_RF2TTC, RF2TTC_MAP_SIZE, &helper_handle); ISOK
  
  //We also have to control a TTCvi card
  ret = VME_map(MODE_A24, HELPER_TTCVI, 0x2000, &vi_handle); ISOK

  while(1)
  {
    printf("\n");
    printf("=============================================\n");
    printf("Select a test:\n");
    printf("   1  Geographical / manual addressing   2  Bord identification\n");
    printf("   3  Register access                    4  Reset commands\n");
    printf("   5  Bunch clock                        6  External bunch clock\n");
    printf("   7  BCmain selection                   8  BC delays\n");
    printf("   9  External orbit                    10  Orbit polarity\n");
    printf("  11  Orbit coarse delay                12  Orbit length adjustment\n");
    printf("  13  Orbit input threshold             14  Orbit input fine shift\n");
    printf("  15  Orbit output fine delay           16  Internal orbit\n");
    printf("  17  BST/TTC set-up                    18  No beam machine mode\n");
    printf("  19  Filling machine mode              20  Ramping machine mode\n");
    printf("  21  Physics machine mode\n");
    printf("------Tools--------------------------------------------------------\n");
    printf("  22  Raw register dump                23  Set debugging parameters\n");
    printf("   0  Quit\n");
    printf("Your choice: ");
    fun = getdecd(fun);
    if (!fun)
      break;
      
    if (fun == 1) 
    {
      printf("Running test 1.......\n");
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->board_id, &vdata); ISOK
      if(vdata != 0x16B)
      {
        printf("The Board ID register contains 0x%08x instead of 0x0000016B\n", vdata);
	exit(-1);
      }
      printf(".....OK\n\n");
    }
    
    if (fun == 2) 
    {
      printf("Running test 2.......\n");
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->program_id, &vdata); ISOK
      printf("The Program ID is 0x%08x\n", vdata);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->revision_id, &vdata); ISOK
      printf("The Revision ID is 0x%08x\n", vdata);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->board_id, &vdata); ISOK
      if(vdata != 0x16B)
      {
        printf("The Board ID is 0x%08x instead of 0x0000016B\n", vdata);
	exit(-1);
      }
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->manufacturer_id, &vdata); ISOK
      if(vdata != 0x80030)
      {
        printf("The Manufacturer ID is 0x%08x instead of 0x00080030\n", vdata);
	exit(-1);
      }      
      printf(".....OK\n\n");
    }
    
    if (fun == 3) 
    {
      printf("Running test 3.......\n");
      for (reg = 0; reg < NREGS; reg++)
      {
        if (all_regs[reg].action & 0x10)
	  ts_delay(500000);
        action = all_regs[reg].action & 0xf;
	
        bmask = 0;
        for (loop = 0; loop < all_regs[reg].nbits; loop++)
	  bmask |= (1 << loop);

        //printf("Offset=0x%08x, bmask=0x%08x\n", all_regs[reg].offset, bmask);
        if (action == 9) //Just check if value is > 0
	{
	  ret = VME_d32r(rf2ttchandle, all_regs[reg].offset, &vdata); ISOK
	  if (!(vdata & bmask))
	  {
	    printf("The register at offset 0x%08x contains 0 instead of any other value\n", all_regs[reg].offset);
	    continue;
	  }  
	}
        
	else if (action == 5) //Read twice
	{
	  ret = VME_d32r(rf2ttchandle, all_regs[reg].offset, &vdata); ISOK
	  ret = VME_d32r(rf2ttchandle, all_regs[reg].offset, &vdata); ISOK
	  if ((vdata & bmask) != all_regs[reg].read_default)
	  {
	    printf("The register at offset 0x%08x contains 0x%08x instead of 0x%08x\n", all_regs[reg].offset, vdata & bmask, all_regs[reg].read_default);
	    continue;
	  }  
	}

 	else if (action == 2) //I2C r/w test
        {
          ret = RF2TTC_I2C_Read(all_regs[reg].offset, &vdata); ISOK	
	  if ((vdata & bmask) != all_regs[reg].read_default)
	    printf("The register at offset 0x%08x contains 0x%08x instead of 0x%08x\n", all_regs[reg].offset, vdata & bmask, all_regs[reg].read_default);

          ret = RF2TTC_I2C_Write(all_regs[reg].offset, all_regs[reg].write); ISOK	
          ret = RF2TTC_I2C_Read(all_regs[reg].offset, &vdata); ISOK	
	  if ((vdata & bmask) != all_regs[reg].read_back)
	    printf("Register at offset 0x%08x: written=0x%08x  read=0x%08x\n", all_regs[reg].offset, all_regs[reg].write, vdata & bmask);
          ret = RF2TTC_I2C_Write(all_regs[reg].offset, all_regs[reg].read_default); ISOK	
	}
	
        else //Standard register r/o and r/w
	{
	  ret = VME_d32r(rf2ttchandle, all_regs[reg].offset, &vdata); ISOK
	  if ((vdata & bmask) != all_regs[reg].read_default)
	    printf("The register at offset 0x%08x contains 0x%08x instead of 0x%08x\n", all_regs[reg].offset, vdata & bmask, all_regs[reg].read_default);
          if(action == 0)
	  {
  	    ret = VME_d32w(rf2ttchandle, all_regs[reg].offset, all_regs[reg].write); ISOK	    
	    ret = VME_d32r(rf2ttchandle, all_regs[reg].offset, &vdata); ISOK
	    if ((vdata & bmask) != all_regs[reg].read_back)
	      printf("Register at offset 0x%08x: written=0x%08x  read=0x%08x\n", all_regs[reg].offset, all_regs[reg].write, vdata & bmask);
  	    ret = VME_d32w(rf2ttchandle, all_regs[reg].offset, all_regs[reg].read_default); ISOK	    
	  }	
	}
      }
      printf(".....OK\n\n");
    }

    if (fun == 4) 
    {    
      printf("Running test 4.......\n");
      //test delay_25 reset
      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->bc_delay25_bc1, &vdata); ISOK
      if (vdata != 0x140)
       printf("The register BC_DELAY25_BC1 contains 0x%08x instead of 0x140\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->bc_delay25_bc2, &vdata); ISOK
      if (vdata != 0x140)
       printf("The register BC_DELAY25_BC2 contains 0x%08x instead of 0x140\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->bc_delay25_bcref, &vdata); ISOK
      if (vdata != 0x140)
       printf("The register BC_DELAY25_BCREF contains 0x%08x instead of 0x140\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->bc_delay25_bcmain, &vdata); ISOK
      if (vdata != 0x140)
       printf("The register BC_DELAY25_BCMAIN contains 0x%08x instead of 0x140\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbin_delay25_orb1, &vdata); ISOK
      if (vdata != 0x140)
       printf("The register ORBIN_DELAY25_ORB1 contains 0x%08x instead of 0x140\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbin_delay25_orb2, &vdata); ISOK
      if (vdata != 0x140)
       printf("The register ORBIN_DELAY25_ORB2 contains 0x%08x instead of 0x140\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbout_delay25_orb1, &vdata); ISOK
      if (vdata != 0x140)
       printf("The register ORBOUT_DELAY25_ORB1 contains 0x%08x instead of 0x140\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbout_delay25_orb2, &vdata); ISOK
      if (vdata != 0x140)
       printf("The register ORBOUT_DELAY25_ORB2 contains 0x%08x instead of 0x140\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbout_delay25_orbmain, &vdata); ISOK
      if (vdata != 0x140)
       printf("The register ORBOUT_DELAY25_ORBMAIN contains 0x%08x instead of 0x140\n", vdata);

      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bset, 0x1); ISOK
      printf("setting delay_25 reset\n");          
      ts_delay(200);
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bclear, 0x1); ISOK        
      printf("clearing delay_25 reset\n");          
      
      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->bc_delay25_bc1, &vdata); ISOK
      if ((vdata & 0xff) != 0x0)
       printf("The register BC_DELAY25_BC1 contains 0x%08x instead of 0x0\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->bc_delay25_bc2, &vdata); ISOK
      if ((vdata & 0xff) != 0x0)
       printf("The register BC_DELAY25_BC2 contains 0x%08x instead of 0x0\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->bc_delay25_bcref, &vdata); ISOK
      if ((vdata & 0xff) != 0x0)
       printf("The register BC_DELAY25_BCREF contains 0x%08x instead of 0x0\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->bc_delay25_bcmain, &vdata); ISOK
      if ((vdata & 0xff) != 0x0)
       printf("The register BC_DELAY25_BCMAIN contains 0x%08x instead of 0x0\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbin_delay25_orb1, &vdata); ISOK
      if ((vdata & 0xff) != 0x0)
       printf("The register ORBIN_DELAY25_ORB1 contains 0x%08x instead of 0x0\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbin_delay25_orb2, &vdata); ISOK
      if ((vdata & 0xff) != 0x0)
       printf("The register ORBIN_DELAY25_ORB2 contains 0x%08x instead of 0x0\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbout_delay25_orb1, &vdata); ISOK
      if ((vdata & 0xff) != 0x0)
       printf("The register ORBOUT_DELAY25_ORB1 contains 0x%08x instead of 0x0\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbout_delay25_orb2, &vdata); ISOK
      if ((vdata & 0xff) != 0x0)
       printf("The register ORBOUT_DELAY25_ORB2 contains 0x%08x instead of 0x0\n", vdata);

      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->orbout_delay25_orbmain, &vdata); ISOK
      if ((vdata & 0xff) != 0x0)
       printf("The register ORBOUT_DELAY25_ORBMAIN contains 0x%08x instead of 0x0\n", vdata);

      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->bc_delay25_bc1, 0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->bc_delay25_bc2, 0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->bc_delay25_bcref, 0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->bc_delay25_bcmain, 0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbin_delay25_orb1, 0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbin_delay25_orb2, 0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbout_delay25_orb1, 0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbout_delay25_orb2, 0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbout_delay25_orbmain, 0x40); ISOK
      ts_delay(500000);
      printf("delay_25 reset OK\n");
      
      //test QPLL reset
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_qpll_status, &vdata); ISOK
      if (vdata != 0x1)
       printf("The register BC1_QPLL_STATUS contains 0x%08x instead of 0x1\n", vdata);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_qpll_status, &vdata); ISOK
      if (vdata != 0x1)
       printf("The register BC2_QPLL_STATUS contains 0x%08x instead of 0x1\n", vdata);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_qpll_status, &vdata); ISOK
      if (vdata != 0x1)
       printf("The register BCREF_QPLL_STATUS contains 0x%08x instead of 0x1\n", vdata);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_qpll_status, &vdata); ISOK
      if (vdata != 0x1)
       printf("The register BCMAIN_QPLL_STATUS contains 0x%08x instead of 0x1\n", vdata);

      printf("Check if the LEDs are on\n");
      PRTC

      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bset, 0x2); ISOK          
      printf("setting QPLL reset\n");          

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_qpll_status, &vdata); ISOK
      if (vdata != 0x0)
       printf("The register BC1_QPLL_STATUS contains 0x%08x instead of 0x\n", vdata);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_qpll_status, &vdata); ISOK
      if (vdata != 0x0)
       printf("The register BC2_QPLL_STATUS contains 0x%08x instead of 0x0\n", vdata);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_qpll_status, &vdata); ISOK
      if (vdata != 0x0)
       printf("The register BCREF_QPLL_STATUS contains 0x%08x instead of 0x0\n", vdata);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_qpll_status, &vdata); ISOK
      if (vdata != 0x0)
       printf("The register BCMAIN_QPLL_STATUS contains 0x%08x instead of 0x0\n", vdata);

      printf("Check if the LEDs are off\n");
      PRTC

      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bclear, 0x2); ISOK 
      printf("clearing QPLL reset\n");          
      ts_delay(500000);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_qpll_status, &vdata); ISOK
      if (vdata != 0x1)
       printf("The register BC1_QPLL_STATUS contains 0x%08x instead of 0x1\n", vdata);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_qpll_status, &vdata); ISOK
      if (vdata != 0x1)
       printf("The register BC2_QPLL_STATUS contains 0x%08x instead of 0x1\n", vdata);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_qpll_status, &vdata); ISOK
      if (vdata != 0x1)
       printf("The register BCREF_QPLL_STATUS contains 0x%08x instead of 0x1\n", vdata);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_qpll_status, &vdata); ISOK
      if (vdata != 0x1)
       printf("The register BCMAIN_QPLL_STATUS contains 0x%08x instead of 0x1\n", vdata);

      printf("Check if the LEDs are on\n");
      PRTC
      printf("QPLL reset OK\n");
      
      //test TTCrx reset
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->ttcrx_status, &vdata); ISOK
      if (vdata != 0x1)
       printf("The register TTCRX_STATUS contains 0x%08x instead of 0x1\n", vdata);

      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bset, 0x4); ISOK          
      printf("setting TTC reset\n");          
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->ttcrx_status, &vdata); ISOK
      if (vdata != 0x0)
       printf("The register TTCRX_STATUS contains 0x%08x instead of 0x0\n", vdata);
 
      printf("Check if the BST realdy LED is off\n");
      PRTC    
      
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bclear, 0x4); ISOK 
      printf("clearing TTC reset\n");          
      ts_delay(2000);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->ttcrx_status, &vdata); ISOK
      if (vdata != 0x1)
       printf("The register TTCRX_STATUS contains 0x%08x instead of 0x1\n", vdata);
      printf("TTCrx reset OK\n");

      //test global reset
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bset, 0x8); ISOK
      printf("setting global reset\n");          
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_qpll_status, &vdata); ISOK
      if (vdata != 0x0)
       printf("The register BC1_QPLL_STATUS contains 0x%08x instead of 0x0\n", vdata);
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->ttcrx_status, &vdata); ISOK
      if (vdata != 0x0)
       printf("The register TTCRX_STATUS contains 0x%08x instead of 0x0\n", vdata);
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_counter, &vdata); ISOK
      if (vdata != 0x0)
       printf("The register ORB1_COUNTER contains 0x%08x instead of 0x0\n", vdata);
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_period_rd, &vdata); ISOK
      if (vdata != 0x0)
       printf("The register ORB1_PERIOD_RD contains 0x%08x instead of 0x0\n", vdata);
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_period_fifo_status, &vdata); ISOK
      if (vdata != 0x1)
       printf("The register ORB1_PERIOD_FIFO_STATUS contains 0x%08x instead of 0x1\n", vdata);
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_period_fifo_rd, &vdata); ISOK
      if (vdata != 0x4000)
       printf("The register ORB1_PERIOD_FIFO_RD contains 0x%08x instead of 0x4000\n", vdata);
                
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bclear, 0x8); ISOK 
      printf("clearing clobal reset\n");          
      ts_delay(500000);
                    
      ret = RF2TTC_I2C_Read((u_int)&rf2ttc_regs->bc_delay25_bc1, &vdata); ISOK
      if ((vdata & 0xff) != 0x0)
       printf("The register BC_DELAY25_BC1 contains 0x%08x instead of 0x0\n", vdata);
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_qpll_status, &vdata); ISOK
      if (vdata != 0x1)
       printf("The register BC1_QPLL_STATUS contains 0x%08x instead of 0x1\n", vdata);
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->ttcrx_status, &vdata); ISOK
      if (vdata != 0x1)
       printf("The register TTCRX_STATUS contains 0x%08x instead of 0x1\n", vdata);
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_counter, &vdata); ISOK
      if (!vdata)
       printf("The register ORB1_COUNTER contains 0 instead of a positive value\n");
 
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_period_rd, &vdata); ISOK
      if (vdata != 0xded)
       printf("The register ORB1_PERIOD_RD contains 0x%08x instead of 0xded\n", vdata);
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_period_fifo_status, &vdata); ISOK
      if (vdata != 0x2)
       printf("The register ORB1_PERIOD_FIFO_STATUS contains 0x%08x instead of 0x2\n", vdata);
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_period_fifo_rd, &vdata); ISOK
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_period_fifo_rd, &vdata); ISOK
      if (!vdata)
       printf("The register ORB1_PERIOD_FIFO_RD contains 0 instead of a positive value\n");
      printf("global reset OK\n");
      
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->bc_delay25_bc1,         0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->bc_delay25_bc2,         0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->bc_delay25_bcref,       0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->bc_delay25_bcmain,      0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbin_delay25_orb1,     0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbin_delay25_orb2,     0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbout_delay25_orb1,    0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbout_delay25_orb2,    0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbout_delay25_orbmain, 0x40); ISOK
      ts_delay(500000);
           
      printf(".....OK\n\n");
    }    

    if (fun == 5) 
    {  
      printf("Running test 5.......\n");
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->working_mode, 0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_man_select, 0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_man_select, 0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_man_select, 0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_man_select, 0); ISOK
      printf("The RF2TTC is ready for the test. Check now if the 4 signals on the scope\n");
      printf("are synchronized with respect to each other. Then check the jitter and the\n");
      printf("signal amplitude as described in the test documentation.\n");
      PRTC
      printf(".....OK\n\n");
    }    

    if (fun == 6) 
    { 
      printf("Running test 6.......\n");
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->working_mode, 0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_man_select, 1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_man_select, 1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_man_select, 1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_man_select, 1); ISOK
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_qpll_status, &vdata); ISOK
      if (vdata)
        printf("The QPLL_1 status register is 0x%08x instead of 0\n", vdata);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_qpll_status, &vdata); ISOK
      if (vdata)
        printf("The QPLL_2 status register is 0x%08x instead of 0\n", vdata);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_qpll_status, &vdata); ISOK
      if (vdata)
        printf("The QPLL_ref status register is 0x%08x instead of 0\n", vdata);

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_qpll_status, &vdata); ISOK
      if (vdata)
        printf("The QPLL_main status register is 0x%08x instead of 0\n", vdata);

      printf("Check if the QPLLs loose the lock and that the LEDs are off.\n");
      PRTC       
      printf(".....OK\n\n");
    }    

    if (fun == 7) 
    {    
      printf("Running test 7.......\n");
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_man_select,    0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_man_select,    1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_man_select,  1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_man_select, 3); ISOK
      printf("You should now see:\n");
      printf("  BC1 - BCmain locked\n");
      printf("  BC2 - BCref locked\n");
      PRTC
    
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_man_select,    1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_man_select,    0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_man_select,  1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_man_select, 2); ISOK
      printf("You should now see:\n");
      printf("  BC2 - BCmain locked\n");
      printf("  BC1 - BCref locked\n");
      PRTC
    
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_man_select,    1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_man_select,    1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_man_select,  0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_man_select, 1); ISOK
      printf("You should now see:\n");
      printf("  BCref - BCmain locked\n");
      printf("  BC2 - BC1 locked\n");
      PRTC
    
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_man_select,    1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_man_select,    0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_man_select,  0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_man_select, 3); ISOK
      printf("You should now see:\n");
      printf("  BC1 - BCmain locked\n");
      printf("  BC2 - BCref locked\n");
      PRTC
    
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_man_select,    0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_man_select,    1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_man_select,  0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_man_select, 2); ISOK
      printf("You should now see:\n");
      printf("  BC2 - BCmain locked\n");
      printf("  BC1 - BCref locked\n");
      PRTC
    
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_man_select,    0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_man_select,    0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_man_select,  1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_man_select, 1); ISOK
      printf("You should now see:\n");
      printf("  BCref - BCmain locked\n");
      printf("  BC2 - BC1 locked\n");
      PRTC
    
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_man_select,    1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_man_select,    1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_man_select,  1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_man_select, 3); ISOK
      printf("You should now see:\n");
      printf("  All locked\n");
      PRTC    
      printf(".....OK\n\n");
    }    

    if (fun == 8) 
    { 
      printf("Running test 8.......\n");
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->bc_delay25_bcmain, 0x4a); ISOK
      printf("Check if the rising edge of BCmain moves from 5 ns\n");
      PRTC
      
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->bc_delay25_bcref, 0x4a); ISOK
      printf("Check if the rising edge of BCref moves from 5 ns\n");
      PRTC
      
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->bc_delay25_bc2, 0x4a); ISOK
      printf("Check if the rising edge of BC2 moves from 5 ns\n");
      PRTC
      
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->bc_delay25_bc1, 0x4a); ISOK
      printf("Check if the rising edge of BC1 moves from 5 ns\n");
      PRTC
      printf(".....OK\n\n");
    } 

    if (fun == 9) 
    {    
      printf("Running test 9.......\n");
      ret = VME_d32w(helper_handle, (u_int)&rf2ttc_regs->working_mode,           0); ISOK
      ret = VME_d32w(helper_handle, (u_int)&rf2ttc_regs->bc1_man_select,         0); ISOK
      ret = VME_d32w(helper_handle, (u_int)&rf2ttc_regs->bc2_man_select,         0); ISOK
      ret = VME_d32w(helper_handle, (u_int)&rf2ttc_regs->bcref_man_select,       0); ISOK
      ret = VME_d32w(helper_handle, (u_int)&rf2ttc_regs->bcmain_man_select,      0); ISOK
      ret = VME_d32w(helper_handle, (u_int)&rf2ttc_regs->orb1_man_select,        1); ISOK
      ret = VME_d32w(helper_handle, (u_int)&rf2ttc_regs->orb2_man_select,        1); ISOK
      ret = VME_d32w(helper_handle, (u_int)&rf2ttc_regs->orbmain_man_select,     2); ISOK
      ret = VME_d32w(helper_handle, (u_int)&rf2ttc_regs->orb1_int_period_set,    0xabc); ISOK
      ret = VME_d32w(helper_handle, (u_int)&rf2ttc_regs->orb2_int_period_set,    0x123); ISOK
      ret = VME_d32w(helper_handle, (u_int)&rf2ttc_regs->orbmain_int_period_set, 0x456); ISOK

      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->working_mode,            0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_man_select,          1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_man_select,          1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_man_select,        1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_man_select,       1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_man_select,         0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_man_select,         0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_man_select,      0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_int_period_set,     0xdec); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_int_period_set,     0xdec); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_int_period_set,  0xdec); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_dac,                0xaa); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_dac,                0xaa); ISOK      
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbin_delay25_orb1,            0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbin_delay25_orb2,            0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbout_delay25_orb1,           0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbout_delay25_orb2,           0x40); ISOK
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbout_delay25_orbmain,        0x40); ISOK
      ts_delay(5000); //wait 5 ms

      printf("Check if the orbit LEDs are on\n");
      PRTC
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_counter, &vdata); ISOK
      ts_delay(1000); //wait 1 ms
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_counter, &vdata2); ISOK
      if (vdata == vdata2)
        printf("The ORB1 counter does not increase\n"); 
	
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_counter, &vdata); ISOK
      ts_delay(1000); //wait 1 ms
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_counter, &vdata2); ISOK
      if (vdata == vdata2)
        printf("The ORB2 counter does not increase\n"); 
	
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_counter, &vdata); ISOK
      ts_delay(1000); //wait 1 ms
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_counter, &vdata2); ISOK
      if (vdata == vdata2)
        printf("The ORBmain counter does not increase\n"); 
      
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->period_counter_reset, 0xf); ISOK
      ts_delay(5000); //wait 5 ms

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_period_rd, &vdata); ISOK
      if (vdata != 0xabd)
        printf("The ORB1_PERIOD_RD register contains 0x%08x instead of 0xabd\n", vdata); 
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_period_fifo_rd, &vdata); ISOK
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_period_fifo_rd, &vdata); ISOK
      if (vdata != 0xabd)
        printf("The ORB1_PERIOD_FIFO_RD register contains 0x%08x instead of 0xabd\n", vdata); 
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_period_rd, &vdata); ISOK
      if (vdata != 0x124)
        printf("The ORB2_PERIOD_RD register contains 0x%08x instead of 0x124\n", vdata); 
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_period_fifo_rd, &vdata); ISOK
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_period_fifo_rd, &vdata); ISOK
      if (vdata != 0x124)
        printf("The ORB2_PERIOD_FIFO_RD register contains 0x%08x instead of 0x124\n", vdata); 

      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_period_rd, &vdata); ISOK
      if (vdata != 0xabd)
        printf("The ORBmain_PERIOD_RD register contains 0x%08x instead of 0xabd\n", vdata); 
      
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_period_fifo_rd, &vdata); ISOK
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_period_fifo_rd, &vdata); ISOK
      if (vdata != 0xabd)
        printf("The ORBmain_PERIOD_FIFO_RD register contains 0x%08x instead of 0xabd\n", vdata); 

      ret = VME_d32w(helper_handle, (u_int)&rf2ttc_regs->orb1_int_period_set,    0x100); ISOK
      ret = VME_d32w(helper_handle, (u_int)&rf2ttc_regs->orb2_int_period_set,    0x100); ISOK
      ret = VME_d32w(helper_handle, (u_int)&rf2ttc_regs->orbmain_int_period_set, 0x100); ISOK
      printf("Check jitter amplitude and NIM output\n");
      PRTC

      printf(".....OK\n\n");
    } 

    if (fun == 10) 
    {    
      printf("Running test 10.......\n");
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_polarity, 1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_polarity, 1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_polarity, 1); ISOK
      printf("Check jitter amplitude and NIM output\n");
      PRTC
      printf(".....OK\n\n");
    } 

    if (fun == 11) 
    { 
      printf("Running test 11.......\n");
      printf("Set the cursors of the scope as described in the test procedure");
      PRTC
      
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_man_select, 1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_course_delay, 0xf); ISOK      
      printf("Check if the rising edge of the orbit 2 is delayed by 375 ns\n");
      PRTC
      
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_course_delay, 0xf); ISOK
      printf("Check if the rising edge of the orbit main is delayed by 375 ns\n");
      PRTC

      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_course_delay, 0xf); ISOK
      printf("Check if all orbits are back to the initial position\n");
      PRTC      
      printf(".....OK\n\n");
    } 

    if (fun == 12) 
    {   
      printf("Running test 12.......\n");
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_length, 0xf); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_length, 0xf); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_length, 0xf); ISOK
      printf("Check if the length of the orbit pulse is 425 ns\n");
      PRTC
      printf(".....OK\n\n");
    } 

    if (fun == 13) 
    { 
      printf("Running test 13.......\n");
      ret = RF2TTC_CalVolt(1, TEST_MODE); ISOK
      ret = RF2TTC_CalVolt(2, TEST_MODE); ISOK
      printf(".....OK\n\n");
    } 

    if (fun == 14) 
    {    
      printf("Running test 14.......\n");
      ret = RF2TTC_CalOrbit(1, TEST_MODE); ISOK
      ret = RF2TTC_CalOrbit(2, TEST_MODE); ISOK
      printf(".....OK\n\n");
    } 

    if (fun == 15) 
    {    
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_course_delay,    0x80); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_course_delay, 0x80); ISOK
      
      printf("Running test 15.......\n");
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_man_select, 1); ISOK
      printf("Increase the scope resolution to 25 ns / division\n");
      printf("Position the cursor on the scope\n");
      PRTC
      
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbout_delay25_orbmain, 0x4a); ISOK
      printf("Check if ORBmain is shifting by 5 ns to the right\n");
      PRTC  
          
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbout_delay25_orb2, 0x4a); ISOK
      printf("Check if ORB2 is shifting by 5 ns to the right\n");
      printf("Check ORB2\n");
      PRTC  
          
      ret = RF2TTC_I2C_Write((u_int)&rf2ttc_regs->orbout_delay25_orb1, 0x4a); ISOK
      printf("Check if the orbit signal are back to the innitial positions\n");
      printf("Check ORB1\n");
      PRTC  
      printf(".....OK\n\n");          
    }

    if (fun == 16) 
    {   
      printf("Running test 16.......\n");
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->working_mode,           0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_man_select,         1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_man_select,         1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_man_select,       1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_man_select,      1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_man_select,        1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_man_select,        1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_man_select,     2); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_int_period_set,    0xdec); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_int_period_set,    0xdec); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_int_period_set, 0xdec); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_dac,               0xaa); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_dac,               0xaa); ISOK
      printf("Check the scope\n");
      PRTC
      printf(".....OK\n\n");
    }
    
    if (fun == 17) 
    {   
      printf("Running test 17.......\n");
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_beam_select,       1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc1_nobeam_select,     0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_beam_select,       1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bc2_nobeam_select,     0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_beam_select,     1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcref_nobeam_select,   0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_beam_select,    1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->bcmain_nobeam_select,  0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_beam_select,      0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb1_nobeam_select,    1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_beam_select,      0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orb2_nobeam_select,    1); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_beam_select,   0); ISOK
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->orbmain_nobeam_select, 2); ISOK
      ret = RF2TTC_I2C_Write(3, 0xff); ISOK      
      ret = VME_d32w(rf2ttchandle, (u_int)&rf2ttc_regs->working_mode,          1); ISOK
      printf(".....OK\n\n");
    }
    
    if (fun == 18) 
    {   
      printf("Running test 18.......\n");
      ret = VME_d16w(vi_handle, 0x10C0, 0x8001); ISOK
      ret = VME_d16w(vi_handle, 0x10C2, 0x1B00); ISOK
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bst_machine_mode, &vdata); ISOK
      if(vdata != 0)
        printf("The BST_MACHINE_MODE register contains 0x%08x instead of 0\n", vdata);
      PRTC
      printf(".....OK\n\n");
    }
    
    if (fun == 19) 
    {   
      printf("Running test 19.......\n");
      ret = VME_d16w(vi_handle, 0x10C0, 0x8001); ISOK
      ret = VME_d16w(vi_handle, 0x10C2, 0x1B01); ISOK
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bst_machine_mode, &vdata); ISOK
      if(vdata != 1)
        printf("The BST_MACHINE_MODE register contains 0x%08x instead of 1\n", vdata);
      PRTC
      printf(".....OK\n\n");
    }
    
    if (fun == 20) 
    {   
      printf("Running test 20.......\n");
      ret = VME_d16w(vi_handle, 0x10C0, 0x8001); ISOK
      ret = VME_d16w(vi_handle, 0x10C2, 0x1B02); ISOK
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bst_machine_mode, &vdata); ISOK
      if(vdata != 2)
        printf("The BST_MACHINE_MODE register contains 0x%08x instead of 2\n", vdata);
      PRTC
      printf(".....OK\n\n");
    }
    
    if (fun == 21) 
    {   
      printf("Running test 21.......\n");
      ret = VME_d16w(vi_handle, 0x10C0, 0x8001); ISOK
      ret = VME_d16w(vi_handle, 0x10C2, 0x1B03); ISOK
      ret = VME_d32r(rf2ttchandle, (u_int)&rf2ttc_regs->bst_machine_mode, &vdata); ISOK
      if(vdata != 3)
        printf("The BST_MACHINE_MODE register contains 0x%08x instead of 3\n", vdata);
      PRTC
      printf(".....OK\n\n");
    }
    
    if (fun == 22) 
    {  
      printf("====================================================================\n");
      ret = RF2TTC_Dump(); ISOK
      printf("====================================================================\n");
    }
    
    if (fun == 23) 
      setdebug(99, 0);
        
    if (fun < 22)
      fun++;
    if (fun == 22)
      fun = 0;    
  }
  
  ret = VME_unmap(helper_handle); ISOK         
  ret = VME_unmap(vi_handle); ISOK         
}

