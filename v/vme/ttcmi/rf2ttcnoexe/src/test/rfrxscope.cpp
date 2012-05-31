/****************************************************************/
/*                                              		*/
/*  file: rfrxscope.c                            		*/
/*                                              		*/
/*  rfrxcscope decodes and displays the contents  		*/
/*  of the registers of RF-RX boards  	 			*/
/*                                              		*/
/*  Author: Markus Joos, CERN-ECP               		*/
/*                                              		*/
/*  Sep.06  MAJO  created                   		  	*/
/*                                              		*/
/********C 2008 - A nickel program worth a dime******************/

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
#include "rfrx.h"
#include "vme_glue.h"


// prototypes
void mainhelp(void);
void regdecode(void);
void setreg(void);
void checkstruct(void);
void rawdump(void);
void functions(void);
void testit(void);
void scanit(u_int delay, float flow, float fhigh);

// globals
u_short vmedata;
u_int ret, libhandle, handle, vmeabase = 0xffffff, verbose = 0, decode = 0;
rfrx_regs_t *regs;


/*****************************/
int main(int argc, char **argv)
/*****************************/
{
  u_int ret, fun = 1;
  int c;

  while ((c = getopt(argc, argv, "dvb:")) != -1)
  {
    switch (c) 
    {
      case 'b': vmeabase = strtol(optarg, 0, 16); break;
      case 'v': verbose = 1;               break;
      case 'd': decode = 1;                break;
      default:
	printf("Invalid option %c\n", c);
	printf("Usage: %s  [options]: \n", argv[0]);
	printf("Valid options are ...\n");
	printf("-b <VME A24 base address>: The A24 base address of the RF-RX card\n");
	printf("-d:                        Decode the registers and exit\n");
	printf("-v:                        Verbose mode for debugging\n");
	printf("\n");
	exit(-1);
    }
  }

  printf("\n\n\n");
  printf("This is RFRXSCOPE\n");
  printf("=================\n");
  
  ret = VME_start();
  if (ret)
  {
    printf("VME_start returned %d\n", ret);
    exit(-1);
  }  

  if (vmeabase == 0xffffff)
  {
    printf("Enter the VMEbus A24 base address of the RFRX: ");
    vmeabase = gethexd(0);
  }
 
  ret = VME_map(MODE_A24, vmeabase, 0x100, &handle);
  if (ret)
  {
    printf("VME_map returned %d\n", ret);
    exit(-1);
  }  
 
  ret = RFRX_Open(vmeabase, &libhandle);
  if (ret)
  {
    printf("RFRX_Open returned %d\n", ret);
    exit(-1);
  }
  
  regs = (rfrx_regs_t *) 0;
  
  ret = VME_d16r(handle, (u_int)&regs->card_id, &vmedata); 
  if (ret)
  {
    printf("VME_d16r returned %d\n", ret);
    exit(-1);
  }  
  
  if (decode)
  {
    rawdump();
    regdecode();
  }  
  else
  {
    while(fun != 0)
    {
      printf("\n");
      printf("Select an option:\n");
      printf("   1 Help\n");
      printf("   2 Decode registers\n");
      printf("   3 Dump raw registers\n");
      printf("   4 Modify a register\n");
      printf("   5 Test the library functions\n");
      printf("   6 Test the card\n");
      printf("   7 Print VMEbus register addresses\n");
      printf("   8 Set debugging parameters\n");
      printf("   0 Quit\n");
      printf("Your choice: ");
      fun = getdecd(fun);
      if (fun == 1) mainhelp();
      if (fun == 2) regdecode();
      if (fun == 3) rawdump();
      if (fun == 4) setreg();
      if (fun == 5) functions();
      if (fun == 6) testit();
      if (fun == 7) checkstruct();
      if (fun == 8) setdebug(99, 0);
    }
  }
  
  ret = RFRX_Close(libhandle);
  if (ret)
    printf("RFRX_Close returned %d\n", ret);
  
  ret = VME_unmap(handle);
  if (ret)
    printf("VME_unmap returned %d\n", ret);

  ret = VME_stop();
  if (ret)
    printf("VME_stop returned %d\n", ret);
}


/********************/
void checkstruct(void)
/********************/
{
  rfrx_regs_t *rp;
  rp = (rfrx_regs_t *)0;
  
  printf("=====================================================================\n");
  printf("VMEIRQStatID   is at 0x%06x\n", (u_int)&rp->VMEIRQStatID);	  
  printf("VMEIRQLevel    is at 0x%06x\n", (u_int)&rp->VMEIRQLevel);		   
  printf("Status         is at 0x%06x\n", (u_int)&rp->Status);		   
  printf("IdentCode      is at 0x%06x\n", (u_int)&rp->IdentCode);		   
  printf("SIG_DETEC      is at 0x%06x\n", (u_int)&rp->SIG_DETEC);		   
  printf("ReceiverModID  is at 0x%06x\n", (u_int)&rp->ReceiverModID);	  
  printf("ch1_ref        is at 0x%06x\n", (u_int)&rp->ch1_ref);                
  printf("ch2_ref        is at 0x%06x\n", (u_int)&rp->ch2_ref);                
  printf("ch3_ref        is at 0x%06x\n", (u_int)&rp->ch3_ref);                
  printf("ch1_freq_lo    is at 0x%06x\n", (u_int)&rp->ch1_freq_lo);		  
  printf("ch1_freq_hi    is at 0x%06x\n", (u_int)&rp->ch1_freq_hi);		   
  printf("ch2_freq_lo    is at 0x%06x\n", (u_int)&rp->ch2_freq_lo);		   
  printf("ch2_freq_hi    is at 0x%06x\n", (u_int)&rp->ch2_freq_hi);		   
  printf("ch3_freq_lo    is at 0x%06x\n", (u_int)&rp->ch3_freq_lo);		   
  printf("ch3_freq_hi    is at 0x%06x\n", (u_int)&rp->ch3_freq_hi);		   
  printf("card_id        is at 0x%06x\n", (u_int)&rp->card_id);		   
  printf("board_id       is at 0x%06x\n", (u_int)&rp->board_id);		   
  printf("FirmwareVer_lo is at 0x%06x\n", (u_int)&rp->FirmwareVer_lo);         
  printf("FirmwareVer_hi is at 0x%06x\n", (u_int)&rp->FirmwareVer_hi);	  
  printf("=====================================================================\n\n");
}


/***************/
void setreg(void)
/***************/
{
  static u_int offset = 0;
  u_int ret;
  u_short odata, ndata;
  
  printf("=====================================================================\n");
  printf("Enter the VMEbus offset of the register: ");
  offset = gethexd(offset);

  ret = VME_d16r(handle, (u_int)&regs->card_id, &odata); 
  if (ret)
  {
    printf("ERROR: VME_d16r returned %d\n", ret);
    return;
  }  

  printf("Enter the new data: ");
  ndata = gethexd(odata);
  ret = VME_d16w(handle, (u_int)&regs->card_id, ndata); 
  if (ret)
  {
    printf("ERROR: VME_d16w returned %d\n", ret);
    return;
  }  

  printf("=====================================================================\n\n");
}


/******************/
void regdecode(void)
/******************/
{
  u_short vd1, vd2, vd3, vd4;
  u_int v1, freq;
  float v2, v3;
  
  printf("=====================================================================\n");
  VME_d16r(handle, (u_int)&regs->card_id, &vd1); 
  printf("CARD ID: 0x%04x\n\n", vd1);
  
  VME_d16r(handle, (u_int)&regs->board_id, &vd1); 
  printf("BOARD ID: 0x%04x\n\n", vd1);  
  
  VME_d16r(handle, (u_int)&regs->FirmwareVer_lo, &vd1);
  VME_d16r(handle, (u_int)&regs->FirmwareVer_hi, &vd2);
  printf("Firmware: 0x%08x\n\n", (vd2 << 16) | vd1);  

  printf("          | Signal detected |     Frequency | Comparator voltage\n");
  printf("Channel 1 |");  
  VME_d16r(handle, (u_int)&regs->SIG_DETEC, &vd1); 
  VME_d16r(handle, (u_int)&regs->ch1_freq_lo, &vd2); 
  VME_d16r(handle, (u_int)&regs->ch1_freq_hi, &vd4); 
  VME_d16r(handle, (u_int)&regs->ch1_ref, &vd3); 
  freq = (vd4 << 16) | vd2;
  v1 = vd1 & 0x2;
  v3 = (float)(vd3 & 0xff) * 1.2 / 255.0;
  if (freq == 0xffffffff)
    printf("             %s |     undefined |            %4.3f V\n", v1?"Yes":" No", v3);
  else  
  {
    v2 = (float)FREQ_BASE / (float)freq;
    printf("             %s | %9.5f MHz |            %4.3f V\n", v1?"Yes":" No", v2, v3);
  }  
  
  printf("Channel 2 |");
  VME_d16r(handle, (u_int)&regs->ch2_freq_lo, &vd2); 
  VME_d16r(handle, (u_int)&regs->ch2_freq_hi, &vd4); 
  VME_d16r(handle, (u_int)&regs->ch2_ref, &vd3);   
  freq = (vd4 << 16) | vd2;
  v1 = vd1 & 0x4;
  v3 = (float)(vd3 & 0xff) * 1.2 / 255.0;
  if (freq == 0xffffffff)
    printf("             %s |     undefined |            %4.3f V\n", v1?"Yes":" No", v3);
  else  
  {
    v2 = (float)FREQ_BASE / (float)freq;
    printf("             %s | %9.5f MHz |            %4.3f V\n", v1?"Yes":" No", v2, v3);
  }  

  printf("Channel 3 |");
  VME_d16r(handle, (u_int)&regs->ch3_freq_lo, &vd2); 
  VME_d16r(handle, (u_int)&regs->ch3_freq_hi, &vd4); 
  VME_d16r(handle, (u_int)&regs->ch3_ref, &vd3); 
  freq = (vd4 << 16) | vd2;
  v1 = vd1 & 0x8;
  v3 = (float)(vd3 & 0xff) * 1.2 / 255.0;
  if (freq == 0xffffffff)
    printf("             %s |     undefined |            %4.3f V\n", v1?"Yes":" No", v3);
  else  
  {
    v2 = (float)FREQ_BASE / (float)freq;
    printf("             %s | %9.5f MHz |            %4.3f V\n", v1?"Yes":" No", v2, v3);
  }  
  printf("=====================================================================\n\n");
}


/****************/
void rawdump(void)
/****************/
{  
  printf("=====================================================================\n");
  ret = RFRX_Dump(libhandle);
  if (ret)
  {
    printf("RFRX_Dump returned %d\n", ret);
    exit(-1);
  }  
  printf("=====================================================================\n\n");
}


/*****************/
void mainhelp(void)
/*****************/
{
  printf("\n=================================================\n");
  printf("Call Markus Joos, 72364, 160663, if you need help\n");
  printf("=================================================\n\n");
}


/******************/
void functions(void)
/******************/
{
  u_int vmeabase = 0, handle, ret, fun = 1;
  rfrx_init_t initdata;
  
  printf("\n=========================================================================\n");
  while (fun != 0)  
  {
    printf("\n");
    printf("Select a function of the API:\n");
    printf("   1 RFRX_Open\n");
    printf("   2 RFRX_Close\n");
    printf("   3 RFRX_Init\n");
    printf("   4 RFRX_Dump\n");
    printf("   0 Exit\n");
    printf("Your choice ");
    fun = getdecd(fun);  
    
    if(fun == 1)
    {
      printf("Enter the VMEbus A24 base address: ");
      vmeabase = gethexd(vmeabase);
      ret = RFRX_Open(vmeabase, &handle);
      if (ret)
	printf("RFRX_Open returned error code %d\n", ret);
      else
        printf("handle = %d\n", handle);	
    }

    if(fun == 2)
    {
      printf("Enter the handle: ");
      handle = getdecd(handle);
      
      ret = RFRX_Close(handle);
      if (ret)
	printf("RFRX_Close returned error code %d\n", ret);
      else
        printf("RFRX_Close OK\n");
    }

    if(fun == 3)
    {
      printf("Enter the handle: ");
      handle = getdecd(handle);
 
      printf("Enter the value for the ch1_ref register: ");
      initdata.ch1_ref = gethexd(0);      

      printf("Enter the value for the ch2_ref register: ");
      initdata.ch2_ref = gethexd(0);      

      printf("Enter the value for the ch3_ref register: ");
      initdata.ch3_ref = gethexd(0);      
      
      ret = RFRX_Init(handle, initdata);
      if (ret)
	printf("RFRX_Init returned error code %d\n", ret);

    }

    if(fun == 4)
    {
      printf("Enter the handle: ");
      handle = getdecd(handle);
      
      ret = RFRX_Dump(handle);
      if (ret)
	printf("RFRX_Dump returned error code %d\n", ret);

    }
  }
}  


/***************/
void testit(void)
/***************/
{
  u_short v1, v2;
  char dummy[5];
  
  
  ret = VME_d16r(handle, (u_int)&regs->card_id, &v1); 
  if (ret)
    printf("VME_d16r returned %d\n", ret);
  if (v1 != CARD_ID)
    printf("ERROR: The card does not have the CARD ID of a RF-RX\n");
 
  ret = VME_d16r(handle, (u_int)&regs->board_id, &v1); 
  if (ret)
    printf("VME_d16r returned %d\n", ret);
  if (v1 != BOARD_ID)
    printf("ERROR: The card does not have the BOARD ID of a RF-RX\n");

  VME_d16r(handle, (u_int)&regs->FirmwareVer_lo, &v1);
  VME_d16r(handle, (u_int)&regs->FirmwareVer_hi, &v2);
  printf("Firmware version (You decide if it is OK): 0x%08x\n\n", (v2 << 16) | v1);
  
  ret = VME_d16r(handle, (u_int)&regs->ReceiverModID, &v1); 
  if (ret)
    printf("VME_d16r returned %d\n", ret);
  printf("Position of SW3:\n");
  if ((v1 & 0x3) == 0x0)
    printf("  0: no module on channel 1\n");    
  if ((v1 & 0x3) == 0x1)
    printf("  1: OPC SRX3 mounted on channel 1\n");    
  if ((v1 & 0x3) == 0x2)
    printf("  2: OPC SRX24 mounted on channel 1\n");    
  if ((v1 & 0x3) == 0x3)
    printf("  3: TRR-1B43 mounted on channel 1\n");    

  printf("Position of SW4:\n");
  if (((v1 >> 2) & 0x3) == 0x0)
    printf("  0: no module on channel 1\n");    
  if (((v1 >> 2) & 0x3) == 0x1)
    printf("  1: OPC SRX3 mounted on channel 1\n");    
  if (((v1 >> 2) & 0x3) == 0x2)
    printf("  2: OPC SRX24 mounted on channel 1\n");    
  if (((v1 >> 2) & 0x3) == 0x3)
    printf("  3: TRR-1B43 mounted on channel 1\n");    

  printf("Position of SW5:\n");
  if (((v1 >> 4) & 0x3) == 0x0)
    printf("  0: no module on channel 1\n");    
  if (((v1 >> 4) & 0x3) == 0x1)
    printf("  1: OPC SRX3 mounted on channel 1\n");    
  if (((v1 >> 4) & 0x3) == 0x2)
    printf("  2: OPC SRX24 mounted on channel 1\n");    
  if (((v1 >> 4) & 0x3) == 0x3)
    printf("  3: TRR-1B43 mounted on channel 1\n");    
    
  if (v1 != 0x3f)
    printf("ERROR: All channels should have a TRR-1B43 mounted\n");
    
  ret = VME_d16r(handle, (u_int)&regs->ch1_ref, &v1); 
  if (ret)
    printf("VME_d16r returned %d\n", ret);
  if(v1 != 0xa)
    printf("ERROR: ch1_ref value is 0x%04x instead of 0xA\n", v1);
    
  ret = VME_d16r(handle, (u_int)&regs->ch2_ref, &v1); 
  if (ret)
    printf("VME_d16r returned %d\n", ret);
  if(v1 != 0xa)
    printf("ERROR: ch2_ref value is 0x%04x instead of 0xA\n", v1);

  ret = VME_d16r(handle, (u_int)&regs->ch3_ref, &v1); 
  if (ret)
    printf("VME_d16r returned %d\n", ret);
  if(v1 != 0xa)
    printf("ERROR: ch3_ref value is 0x%04x instead of 0xA\n", v1);
    
  printf("Install the cables for the first scan and press return\n");
  gets(dummy);
  scanit(10, 40.05, 40.15);

  printf("Install the cables for the second scan and press return\n");
  gets(dummy);
  scanit(1000, 0.0111, 0.0113);
}


/***********************************************/
void scanit(u_int delay, float flow, float fhigh)
/***********************************************/
{
  u_int channel, in_ok, r1, r2, r3, r4, mask, loop, test_ref, vin, vout;
  u_short v1, v2, v3;
  float freq;
  ret = ts_open(1, TS_DUMMY);
  if (ret)
    rcc_error_print(stdout, ret);
      
  for (channel = 0; channel < 3; channel++)
  {
    if (channel == 0)
    {
      r1 = (u_int)&regs->ch1_ref;    
      r2 = (u_int)&regs->ch1_freq_lo;
      r3 = (u_int)&regs->ch1_freq_hi;
    }
    if (channel == 1)
    {
      r1 = (u_int)&regs->ch2_ref;    
      r2 = (u_int)&regs->ch2_freq_lo;
      r3 = (u_int)&regs->ch2_freq_hi;
    }
    if (channel == 2)
    {
      r1 = (u_int)&regs->ch3_ref;    
      r2 = (u_int)&regs->ch3_freq_lo;
      r3 = (u_int)&regs->ch3_freq_hi;
    }
    r4 = (u_int)&regs->SIG_DETEC;
    mask = 1 << (channel + 1);
    vin = 0xffffffff;
    vout = 0xffffffff;
  
    for (test_ref = 0; test_ref < 256; test_ref ++)
    {
      VME_d16w(handle, r1, test_ref); 
      ts_delay(delay);

      in_ok = 0;
      for (loop = 0; loop < 200; loop ++)
      {
	VME_d16r(handle, r2, &v1); 
	VME_d16r(handle, r3, &v2);
	VME_d16r(handle, r4, &v3); 
	freq = float(FREQ_BASE)/((v2 << 16) | v1);

	if ((freq > flow) && (freq < fhigh) && (v3 & mask) && (vin == 0xffffffff)) 
          in_ok++;

	if ((vin != 0xffffffff) && ((freq < flow) || (freq > fhigh) || !(v3 & mask)))  
	{
	  vout = test_ref;
	  break;
	}

	if (verbose)
	  printf("Channel %d: test_ref = %d, freq = %f, v3 = 0x%1x, in_ok = %d\n", channel + 1, test_ref, freq, v3 & 0xe, in_ok);

	ts_delay(10);
      }
      
      if (in_ok == 200) 
        vin = test_ref;

      if (vout != 0xffffffff)  
	break;
    }
    if ((vin != 0xffffffff) && (vout != 0xffffffff))
    {
      printf("Channel %d: offset voltage min = %d, offset voltage max = %d\n", channel + 1, vin, vout);
      v3 = (vin + vout) / 2; 
      printf("Channel %d: Writing %d to ch_ref\n", channel + 1, v3);
      VME_d16w(handle, r1, v3); 
    } 
    else if ((vin != 0xffffffff) && (vout == 0xffffffff))
      printf("Channel %d: offset voltage min = %d, ERROR: No offset voltage max found\n", channel + 1, vin);
    else if ((vin == 0xffffffff) && (vout != 0xffffffff))
      printf("Channel %d: ERROR: No offset voltage min found, offset voltage max = %d\n", channel + 1, vout);
    else
      printf("Channel %d: ERROR: No stable region found\n", channel + 1);
  }

  ret = ts_close(TS_DUMMY);
  if (ret)
    rcc_error_print(stdout, ret);
}



