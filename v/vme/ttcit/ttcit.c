/*BOARD ttcit 0x8a0000 0x300 */
/*

10.3.05 Work started   : 0x8x0000 0x300 should be
*/

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <strings.h>

/*  VME wrapper  */
#include "vmewrap.h"

/* TTCit specific  */
#include "ttcit.h"

/* Software TTCit logic */
#include "ttcit_logic.h"

/* On-Line analysis tools */
#include "ssm_analyz.h"

/* Remove this after test */
#include "ttcit_mm.h"

/* I/O mainly needed for testing */
#include "ttcit_io.h"

#include "ttcit_om.h"

/*
  New SSM format handling
 */
#include "ttcit_n.h"

/*
  GUI static memory allocation
*/
#define TTCIT_COUNTERS_NCOUNTS   18
#define CNT_OM_L0                 0
#define CNT_OM_L1                 1
#define CNT_OM_L1M                2
#define CNT_OM_L2A                3
#define CNT_OM_L2R                4

#define CNT_OM_ERR_L0S            5
#define CNT_OM_ERR_L1S            6

#define CNT_OM_ERR_L1MM           7
#define CNT_OM_ERR_L1MS           8
#define CNT_OM_ERR_L1MI           9
#define CNT_OM_ERR_L1MD          10

#define CNT_OM_ERR_L2MM          11
#define CNT_OM_ERR_L2MS          12
#define CNT_OM_ERR_L2MI          13
#define CNT_OM_ERR_L2MD          14

#define CNT_OM_ERR_CAL           15
#define CNT_OM_ERR_BCNT          16

#define CNT_OM_COUNT_PP          17

#define CNT_SM_L0                17
#define CNT_SM_L1                18
#define CNT_SM_L1M               19
#define CNT_SM_L2A               20
#define CNT_SM_L2R               21
#define CNT_SM_UNK               22
#define CNT_SM_L0_L1_L2A         23
#define CNT_SM_L1_REJ            24
#define CNT_SM_L2_REJ            25
#define CNT_SM_ROI               26
#define CNT_SM_L0L1_FAKES        27

#define CNT_SM_ERR_L0S           28
#define CNT_SM_ERR_L1S           29
#define CNT_SM_ERR_L1T           30
#define CNT_SM_ERR_L1MO          31
#define CNT_SM_ERR_L1F           32
#define CNT_SM_ERR_L2T           33
#define CNT_SM_ERR_L2TS          34
#define CNT_SM_ERR_L2F           35
#define CNT_SM_ERR_BCNT          36
#define CNT_SM_ERR_ROI_F         37
#define CNT_SM_ERR_ROI_S         38
#define CNT_SM_ERR_ROI_T         39

#ifdef TTCIT_SSM_VER_NEW_1_
#define _TEST_AVAIL_ if(FPGAcodeVersion() < 24){    \
    printf("FPGA code version < 24 required to run this function\n");\
    break;}
#else
#define _TEST_AVAIL_ printf("Function not compiled in\n");break
#endif

#define _SET_BOOL_(B,A) B = (A != 0) ? 1 : 0

static struct {

  int nCount;       /* Number of counters */
  int init;        /* if 0 then this is the initial call after clear */

  w32 prev[TTCIT_COUNTERS_NCOUNTS];
  w32 neww[TTCIT_COUNTERS_NCOUNTS];

} TTCit_counters;

extern struct SSMbuffer SSM;
extern struct ttcit1_opt t1o;

#define TCOUNT_ADDC(A,B) case A:\
  TTCit_counters.neww[i] = B;\
  break

#define IF_SCOPE(A,B) if(A == 100){	  \
    printf("Scope Signal %s not set\n",##B); \
  }else{ \
    printf("Scope Signal %s bit set %d\n",##B,A)	\
      }

/*   Stuff needed by the overall logic */

void initmain(){
  /*                Called once at the very beginning 
   */
  initCounters();
}

void endmain(){
  /*                This is probably called at the end of session
   */

  /*
    Free all allocated FIFO's in SSM analysis
   */
  ttc_it_free_fifos();
}

void boardInit(){
  /*                Called once, after initmain()
                    if -noboardInit parameter is given at ttcvi.exe call
                    this function is not called

            ACTIONS:

       - sets the default values of registers
       - prints the default values from selected set of registers
       - prints the initial default configuration of the board
       - prints the : BOARD version - from some register
                      FPGA  version - from some register
       - clears counters (and prints a message about it)
   */

  printf("TTCit Initialization\n");

#ifdef SIMVME
  initTTCit_sim();
#endif

  /*
         Check the TTCit board and FPGA version.

         IF something funny is detected -> Reload the FPGA code
   */

  {
    int status = 0;
    int bt;
    while(1){
      bt = GetBoardType();
#ifndef NO_BOARD_NUMBER
      if(bt != 0x5a){
	printf("The board %X seems to be of the wrong type\n",bt);
	status = 1;
	break;
      }else{
	printf("Board correctly identified as %X\n",bt);
      }
#else
      printf("No Board number identification expected\n");
      printf("TTCit logic FPGA version = %d\n",bt);
#endif

#ifdef NOT_YET_DONE
      printf("Resetting Flash Memory...\n");
      bt = ResetFM();
      if(bt != 0){
	printf("FM cannot be reset - returned IRC = %d\n",bt);
	status = 2;
	break;
      }
      printf("Setting the FM address counter to 0\n");
      ClearAddressFM();
      printf("FM address counter set to 0\n");

/*
      bt = InfoCodeFPGA();
      if(bt != 0){
	printf("Cannot get valid FPGA code information, IRC = %d\n",bt);
	status = 3;
	break;
      }
*/

      printf("Loading FPGA logic\n");
      bt = LoadFPGA();
      if(bt != 0){
	printf("Canna load the code from Flash Memory to FPGA, IRC = %d\n",bt);
	break;
      }

#endif

      /*
	Reset the TTCrx chip - this should NOT be done
      

      printf("Resetting TTCrx chip\n");
      bt = TTCRxReset();
      if(bt != 0){
	printf("Canna reset the TTCRx chip IRC = %d\n",bt);
	break;
      }
      */

      /*
	Reset the TTCit logic
       */

      printf("Initialization of the softeare TTCIT logic\n");
      bt = init_TTCIT_logic();
      if(bt != 0){
	printf("Canna intialize TTCIT logic, IRC = %d\n",bt);
	break;
      }
      print_TTCIT_logic_state();
      printf("Resetting TTCIT\n");
      bt = ResetTTCit();
      OM_yes_print();
      if(bt != 0){
	printf("Canna reset TTCIT logic, IRC = %d\n",bt);
	break;
      }
      ttcit1_init();
      /*
	Set Snap Shot memory analyzer into default state
       */
      printf("Setting Snap Shot memory analyzer to default state\n");
      bt = ttc_it_alloc_fifos();
      if(bt != 0){
	printf("Canna allocate memory for SSM analysis, IRC = %d\n",bt);
	break;
      }
      ttc_it_reset_options();
      printf("Snap Shot Memory analyzer set up - O.K.\n");

      break;
    }
    if(status == 0){
      printf("TTCIT board configuration successfull\n");
    }else{
      printf("TTCIT board configuration FAILED!!!\n\n");
      printf(" \n");
      printf("TRY TO RELOAD FPGA LOGIC:  LoadFPGA() \n");
      printf("(may be needed after power up\n");
      printf(" \n");
      printf("If this does not help,\n");
      printf("STOP AND CONTACT EXPERTS !!!\n");
    }
  }

  /*
    Initialize On-Line Monitor OM
   */
  OM_init();

  /*
    Initialize GUI Counters
   */
  initCounters();

  /*
         The FPGA default configuration must be loded here
   */
}

/*FGROUP Configuration
Initialize TTCit board
*/
void TTCITinit(){
  boardInit();           /* Basically call boardInit once more */
}

/*FGROUP VMEcontroller
  Get the version of the FPGA code
 */
int FPGAcodeVersion(){
  int r;
  r = VMER32(VERSION_NUMBER) & 0xff;
  printf("FPGA code Version = %d\n",r);
  return r;
}

/*FGROUP VMEcontroller
Get Board Serial Number 
*/
int GetSerialNumber(){

  int sernum;
  w32 wrdSerialNumber;
  w32 serialMask = 0xff;

  wrdSerialNumber = VMER32(SERIAL_NUMBER);
  sernum = wrdSerialNumber & serialMask;

  printf("Detected Board serial number = %X\n",sernum);

  return sernum;
}

/*FGROUP Reset
Reset TTCRx chip
*/
int TTCRxReset(){
  int irc = 0;

  VMEW32(RESET_TTCRX,0);

  return irc;
}

/*FGROUP Reset
Reset TTCIT logic
*/
int ResetTTCit(){
  int irc = 0;

  reset_TTCit_logic();
  OM_init();

  return irc;
}

/*FGROUP FrontPanel
  Clear all counters:   Reset all memory counters to 0
                        Reset all OM counters
                        SM counters are not resettable
 */
void clearCounters(){
  int i;

  /*
    Clear counters in memory
   */
  TTCit_counters.init = 0;
  for(i = 0; i < TTCit_counters.nCount; i++){
    TTCit_counters.prev[i] = 0;
    TTCit_counters.neww[i] = 0;
  }

  /*
    Clear Counters of the On-Line Monitor
   */
  OM_ResetCounters();
}


/*FGROUP FrontPanel
  Sets the scope signals for A, B oscilloscope channels
 */
void ScopeSelect_AB(int A, int B){
  //int aa, bb;
  int j = 0;

  if(A == TTCIT_LAST_SCOPE_BIT){
    A = 100;
  }
  if(B == TTCIT_LAST_SCOPE_BIT){
    B = 100;
  }

  switch(A){
  case 100:                              /* Set 0x0 */
    VMEW32(SCOPE_SELECTED_A,0x0);
    break;
  case 99:                               /* Keep old, do nothing */
    break;
  default:
    j = 0x1 << A;
    VMEW32(SCOPE_SELECTED_A,j);
    break;
  }

  switch(B){
  case 100:
    VMEW32(SCOPE_SELECTED_B,0x0);
    break;
  case 99:
    break;
  default:
    j = 0x1 << B;
    VMEW32(SCOPE_SELECTED_B,j);
    break;
  }

}

/*FGROUP FrontPanel
  Gets the current settings for A and B scope signals

  how =  0  Return 100*A + B
         1  Return
	 2  Print 100*A + B
 */
void ScopeGet_AB(int how){
  int ab;
  int a = 0;
  int b = 0;
  int bita, bitb;
  int ba, bb;
  a = VMER32(SCOPE_SELECTED_A);
  b = VMER32(SCOPE_SELECTED_B);
  bita = ffs(a);
  bitb = ffs(b);
  ba = (bita != 0) ? bita - 1 : 100;
  bb = (bitb != 0) ? bitb - 1 : 100;
  ab = 1000 * ba + bb;
  switch(how){
  case 0:
    break;
  case 1:
    if(ba == 100){
      printf("Scope signal A not set\n");
    }else{
      printf("Scope signal A set bit %d\n",ba);
    }
    if(bb == 100){
      printf("Scope signal B not set\n");
    }else{
      printf("Scope signal B set bit %d\n",bb);
    }
    break;
  case 2:
    printf("%d\n",ab);
    break;
  default:
    break;
  }
  /*
  return ab;
  */
}

/*FGROUP OnBoard_Monitor
  Resets the TTCit board, keeps all OM setting intact
 */
void OM_Reset(){
  OM_reset_board();
}

/*FGROUP OnBoard_Monitor
  Activate OM and start collecting data
*/
void OM_ON_Start(){
  OM_Reset();
  OM_set(OM__ON_START);
}

/*FGROUP OnBoard_Monitor
  Deactivate OM, stop it if collecting data
*/
void OM_OFF(){
  OM_set(OM__OFF);
}

/*xxxFGROUP OnBoard_Monitor
  Stop the OM collecting data (not effect otherwise)
*/
void OM_STOP(){
  OM_set(OM__STOP);
}

/*aaFGROUP OnBoard_Monitor
  Start the OM if enabled and not collecting data (no effect otherwise)
*/
void OM_START(){
  OM_Reset();
  OM_set(OM__START);
}

/*FGROUP OnBoard_Monitor
  Show the status of the OM
*/
void OM_Status(){
  OM_print_status();
  OM_PrintStatusSSM();
}

/*FGROUP OnBoard_Monitor
  Prints the status of the SSM, whether it is reading data or stopped and 
  available for read. To be used only with the other functions of the Online
  Monitor.
*/
void OM_PrintStatusSSM(){
  enum ttcit_OM_RYBY st;

  st = OM_status_SSM();
  printf("SSM found in state ");
  if(st == OM_READY){
    printf("  STOPPED, available for read");
  }else{
    printf("  READING, busy. Stop it before reading attempt.");
  }
  printf("\n");
}

/*FGROUP OnBoard_Monitor
  Sets the error mask for the OM. Set nonzero value for each error
  that is to be watched. When this error is detected the data collection
  stops and the SSM can be fetched and its contents analyzed.

  Error conditions related to messages: L1m, L2m
*/
void OM_SetErrorMask_msg(int L1M_missing,
			 int L1M_spurious,
			 int L1M_incomplete,
			 int L1M_data_error,
			 int L2M_missing,
			 int L2M_spurious,
			 int L2M_incomplete,
			 int L2M_data_error
			 ){
  struct om_errset_list erma_list[12];
  int i = 0;
  erma_list[0].errcode = i;
  erma_list[0].flag = 0;
  OM_ADD_TO_ERR_LIST(L1M_missing,erma_list,i,TTCIT_CTRL_STOP_L1MM);
  OM_ADD_TO_ERR_LIST(L1M_spurious,erma_list,i,TTCIT_CTRL_STOP_L1MS);
  OM_ADD_TO_ERR_LIST(L1M_incomplete,erma_list,i,TTCIT_CTRL_STOP_L1MI);
  OM_ADD_TO_ERR_LIST(L1M_data_error,erma_list,i,TTCIT_CTRL_STOP_L1MD);
  OM_ADD_TO_ERR_LIST(L2M_missing,erma_list,i,TTCIT_CTRL_STOP_L2MM);
  OM_ADD_TO_ERR_LIST(L2M_spurious,erma_list,i,TTCIT_CTRL_STOP_L2MS);
  OM_ADD_TO_ERR_LIST(L2M_incomplete,erma_list,i,TTCIT_CTRL_STOP_L2MI);
  OM_ADD_TO_ERR_LIST(L2M_data_error,erma_list,i,TTCIT_CTRL_STOP_L2MD);
  erma_list[0].errcode = i;
  OM_store_errmask(&erma_list[0]);
  OM_ShowErrorMask();
}

/*FGROUP OnBoard_Monitor
  Sets the error mask for the OM. Set nonzero value for each error
  that is to be watched. When this error is detected the data collection
  stops and the SSM can be fetched and its contents analyzed.

  Error conditions related to signals: L0, L1, Calibration, BC
*/
void OM_SetErrorMask_sig(int PP,
			 int L0S,
			 int L1S,
			 int CAL_error,
			 int BCNT_diff
			 ){
  struct om_errset_list erma_list[12];
  int i = 0;
  erma_list[0].errcode = i;
  erma_list[0].flag = 0;
  OM_ADD_TO_ERR_LIST(PP,erma_list,i,TTCIT_CTRL_STOP_PP);
  OM_ADD_TO_ERR_LIST(L0S,erma_list,i,TTCIT_CTRL_STOP_L0S);
  OM_ADD_TO_ERR_LIST(L1S,erma_list,i,TTCIT_CTRL_STOP_L1S);
  OM_ADD_TO_ERR_LIST(CAL_error,erma_list,i,TTCIT_CTRL_STOP_CAL);
  OM_ADD_TO_ERR_LIST(BCNT_diff,erma_list,i,TTCIT_CTRL_STOP_BCNT);
  erma_list[0].errcode = i;
  OM_store_errmask(&erma_list[0]);
  OM_ShowErrorMask();
}


/*FGROUP OnBoard_Monitor
  Show the Error mask set
*/
void OM_ShowErrorMask(){
  int a = 0x0;
  int dummy;
  a = OM_get_error_mask(&dummy);
  OM_print_error_mask(a);
}

/*FGROUP OnBoard_Monitor
  Clear the Error mask, i.e. unset all error stop conditions.
*/
void OM_ClearErrorMask(){
  OM_SetErrorMask_msg(0,0,0,0,0,0,0,0);
  OM_SetErrorMask_sig(0,0,0,0,0);
}

/*FGROUP OnBoard_Monitor
  Save the currently set error mask for later use. The current error mask 
  is stored only in memory.
*/
void OM_SaveErrorMask(){
  OM_remember_error_mask();
}

/*FGROUP OnBoard_Monitor
 Set the saved error mask (useful after TTCit Reset). The saved Error mask
 is kept in the computer memory.
*/
void OM_ReloadLastErrorMask(){
  OM_reload_error_mask();
}

/*FGROUP OnBoard_Monitor
  Set time between L0 - L1 in BC clocks
*/
void OM_SetTimeL0_L1(int t){
  /*
    The number to be placed in the register must be decremeneted

    the internal counter in the FPGA logic starts from 0 where humans start
    from 1
   */
  OM_set_time_l0_l1(t - TTCIT_L0_L1_HW_SW_KONST);
}

/*FGROUP OnBoard_Monitor
  Print the contents of all hardware counters - trigger and errors.
*/
void OM_ShowCounters(){
  OM_TriggerCounters();
  printf("--- --- --- --- --- --- --- --- ---\n");
  OM_ErrorCounters();
}

/*FGROUP OnBoard_Monitor
  Reset all hardware counters.
*/
void OM_ResetCounters(){
  OM_reset_counters();
}

/*FGROUP OnBoard_Monitor
 Print the contents of the TRIGGER counters (L0, L1, L1m, L2m)
*/
void OM_TriggerCounters(){
  struct om_hw_counters ct;

  OM_get_counters(&ct);

  printf(" OM TRIGGER COUNTERS:\n");
  printf(" ------------------------\n");
  OM_PRINT_COUNTER("L0  :",ct.L0);
  OM_PRINT_COUNTER("L1  :",ct.L1);
  OM_PRINT_COUNTER("L1m :",ct.L1m);
  OM_PRINT_COUNTER("L2a :",ct.L2a);
  OM_PRINT_COUNTER("L2r :",ct.L2r);
  printf("\n");
}

/*FGROUP OnBoard_Monitor
  Print the contents of the ERROR counters
*/
void OM_ErrorCounters(){
  struct om_hw_counters ct;

  OM_get_counters(&ct);

  printf(" OM ERROR COUNTERS\n");
  printf(" --------------------------\n");
  OM_PRINT_COUNTER("Pre-Pulz error     ",ct.PP);
  OM_PRINT_COUNTER("L0 spurious        ",ct.L0S);
  OM_PRINT_COUNTER("L1 spurious        ",ct.L1S);
  printf("\n");
  OM_PRINT_COUNTER("L1m missing        ",ct.L1MM);
  OM_PRINT_COUNTER("L1m spurious       ",ct.L1MS);
  OM_PRINT_COUNTER("L1m incomplete     ",ct.L1MI);
  OM_PRINT_COUNTER("L1m data error     ",ct.L1MD);
  printf("\n");
  OM_PRINT_COUNTER("L2m missing        ",ct.L2MM);
  OM_PRINT_COUNTER("L2m spurious       ",ct.L2MS);
  OM_PRINT_COUNTER("L2m incomplete     ",ct.L2MI);
  OM_PRINT_COUNTER("L2m data error     ",ct.L2MD);
  printf("\n");
  OM_PRINT_COUNTER("CALIBRATION error  ",ct.CAL);
  OM_PRINT_COUNTER("BC mismatch        ",ct.BCNT);
  printf("\n");
}

/*FGROUP OnBoard_Monitor
  Show the errors detected by the OM after STOP
*/
void OM_Detected_Errors(){
  OM_print_error_pattern();
}

/*FGROUP OnBoard_Monitor
  After stopping of the OM the contents of the SSM is fetched and dumped.
*/
void OM_FetchSSM_Dump(){
  OM_fetch_print_SS_bc_nowait();
  OM_Detected_Errors();
}

/*FGROUP OnBoard_Monitor
  After stopping of the OM the contents of the SSM is fetched and analyzed.
 */
void OM_FetchSSM_Analyze(){
  OM_soft_monitor_single_nowait(TRUE);
  OM_Detected_Errors();
}

/*FGROUP OnBoard_Monitor
  Makes error masks and settings of the OM as equivalent to those of the 
  SSM analyzer and SSM Software monitor as possible.
  Saves the need to set basicaly the same constants  twice.

  how  = 0 copy the settings from OM to SSM analyz/Soft Monitor
         1 copy the settings from SSM analyz/Soft Monitor to OM
 */
void OM_ErrMaskSync(int how){
  OM_synchro_settings(how);
}

/*FGROUP OnBoard_Monitor
  Dump the contents of the SSM after On-Line Monitor stopped after an error
  condition. Does CombinedFetchAndDump() when the OM is not running.

  Its argument specifies how much minutes we are wanting to wait before 
  forcing the OM to stop accumulating data.
*/
void OM_SSM_DumpAfterStop(int minutes){
  OM_fetch_print_SS_bc(minutes);
  OM_Detected_Errors();
}

/*FGROUP OnBoard_Monitor
  Applies software SSM analyzer to the contents of the SSM before the occurence
  of the error condition leading to a stop.

  Its argument specifies how much minutes we are wanting to wait before
  forcing OM to stop accumulating data.
*/
void OM_SSM_AnalyzeAfterStop(int minutes){
  OM_soft_monitor_single_ssm(TRUE,minutes);
  OM_Detected_Errors();
}

/*FGROUP OnBoard_Monitor
  Sets the number of BCs during which the SSM is written into after
  an error detection (0 means stop on error)
 */
void OM_SetDelayedSSMstop(int dt){
  VMEW32(HW_DELAY_SSM_STOP, dt);
}

/*FGROUP OnBoard_Monitor
  Gets the value of the delay for SSM write after error detection
 */
int OM_GetDelayedSSMstop(){
  int dt = 0;
  dt = VMER32(HW_DELAY_SSM_STOP);
  printf("Delay for SSM stop on error = %d\n",dt);
  return dt;
}

/*FGROUP SnapShot
  Reads address counter - nr. of words in Snap Shot memory
 */
int ReadAddressSSM(){
  int retval;
  retval = read_SSM_address();
  return retval;
}

/*FGROUP SnapShot
  Reads and prints SSM address counter and Nr. of words in BC fifo
*/
void RA(){
  int s = 0;
  int b = 0;

  s = ReadAddressSSM();
  b = GetNbcntFIFO();
  printf("SSM address counter = %d       BC FIFO level = %d\n",s,b);
}

/*FGROUP SSM_analyz
  Specify, how the L0 signal is sent to TTCit board:

  t == 0   Not present (L0 is not sent to TTCit at all)
       
       1   L0 is sent over A channel

       2   L0 is sent over wire

       3   L0 is sent over wire and A channel (if possible)

       4   Unknown. We shall try to read a couple of snapshots and determine
           absence/presence of L0. Its absence will NOT be considered an
	   error!
 */
void L0signal(int t){
  ttc_it_set_l0_signal(t);
}

/*FGROUP SoftMONITOR
  Start software monitor:  

  A soft Monitor run in split in Loops in which SSM and BC fifo is read.
  During one loop the SSM is set to SCOPE mode and read nSCOPE times. 
  Then it is set to SEQ mode and read nSEQ times. 
  The whole process repeats nLoops times (or indefinitely to be stopped
  from StopMONITOR function. 

  use SetStopConditionNSEQ(...) if you want to loop only limited nr. of times
      SetStopConditions(...)    if you want to stop on selected errors
      SetNEVERstop()            if you want to loop indefinitely

  use StopMONITOR()             to stop looping at any moment 

 */
int StartMONITOR(){
  int retval = 0;
  int stat;
  int nSEQignore = 0;
  int nLoops = 0;
  int nSCOPE = 1;
  do{
    stat = StatusMONITOR();
    if(stat != MON_STOPPED){
      printf("Soft MONITOR is already running, stop it first\n");
      retval = -1;
      break;
    }
    retval = soft_monitor_start(nSCOPE, nSEQignore, nLoops);
  }while(0);
  return retval;
}

/*FGROUP FrontPanel
  Reads all Counters (Soft Monitor and On-Line monitor) and prints them
  in a standard way: 

  ordering of counters must correspond with that in ttcit_u.py
  TTCITcnts = {...}

  NCNTS  = Number of counters to be read + 1
  accrual == 1 return differences (actual - previos value)
          != 1 return actual values

 */
void getCounters(int NCNTS, int difval){
  w32 buffer[TTCIT_COUNTERS_NCOUNTS];
  int i;
  readCounters(&buffer[0], NCNTS, difval);
  for(i = 0; i < NCNTS; i++){
    printf("0x%x\n",buffer[i]);
  }
}

/*FGROUP Debug
Read the last Snap Shot memory word
*/
int ReadLastSSM(){
  int retval;
  int stat;
  do{
    stat = StatusMONITOR();
    if(stat != MON_STOPPED){
      printf("Action interferes with the Soft MONITOR\n");
      printf("Stop it first, then resubmit your command\n");
      retval = 0;
      break;
    }
    retval = read_last_SSM_word();
  }while(0);
  return retval;
}

/*FGROUP SnapShot
  Fetch the contents of the SSM into a software buffer
*/
int FetchSSM(){
  int retval;
  int stat;
  do{
    stat = StatusMONITOR();
    if(stat != MON_STOPPED){
      printf("Action interferes with running Soft MONITOR\n");
      printf("Stop it and resubmit your command\n");
      retval = 0;
      break;
    }
    retval = fetch_SSM();
  }while(0);
  return retval;
}

/*FGROUP SnapShot
  Dump the Snap Shot memory buffer (after being read from the VME)
  nw = nr. of words to be dumped (0 == all)
 */
void DumpSSM(int nw){
  dump_SSM(nw);
}

/*xxxFGROUP SnapShot
  Write the software buffer onto the disk file 'ssm_dump.dat'
 */
int xxxWriteSSMbuffer(){
  int retval = 0;
  retval = write_SS_buffer();
  return retval;
}

/*FGROUP SnapShot
  Combnation of several actions for faster inspection of SSM and BC FIFO
  
  Performs the following actions: 1) ResetTTCit
                                  2) FetchSSM
                                  3) FetchBcntFIFO
                                  4) Combined print of seen events

  It provides a quick look at the TTC signals in A and B channels 
 */
void CombinedFetchAndDump(){
  fetch_print_SS_bc();
}

/*FGROUP Debug
  Print contents of the SSM from address 'begin' till address 'end'
  with zero suppression (only words that are 0x0 are not printed

  Since there is 1 MB the full SSM dump is to be avoided
 */
void RawDumpSSM(int begin, int end){
  dump_raw_SSM_buffer(begin, end);
}

/*FGROUP SnapShot
  Clears the ON-BOARD Snap Shot Memory (SSM) i.e. write Zeroes to the SSM.
 */
void ClearSSM(){
  clear_SS_memory_on_board();
}

/*FGROUP SnapShot 
Clear the SSM buffer in MEMORY
*/
void ClearSSMemoryBuffer(){
  clear_SS_memory_buffer();
}

/*FGROUP SnapShot
Get the number of words in the SSM software buffer
 */
int GetNrSSMwords(){
  int retval = 0;
  retval = get_SS_top_mem_buffer();
  return retval;
}

/*xxxFGROUP SnapShot
  Throw away some data from SSM, set the address counter to given address
  addr = address from which you would like to start reading SSM again
 */
int ThrowSSMdata(int addr){
  int retval = 0;
  int stat;
  do{
    stat = StatusMONITOR();
    if(stat != MON_STOPPED){
      printf("Action interferes with running Soft MONITOR\n");
      printf("Stop it and resubmit your command\n");
      break;
    }

    throw_SS_data(addr);
    retval = read_SSM_address();
  }while(0);
  return retval;
}

/*xxxFGROUP SnapShot
  Set the Snap Shot memory mode:  0    = SSM in SCOPE mode
                                  1    = SSM in SEQ mode
 */
void SetModeSSM(int mode){
  int irc;
  do{
    irc = StatusMONITOR();
    if(irc != MON_STOPPED){
      printf("Action interferes with running Soft MONITOR\n");
      printf("Stop it and resubmit your command\n");
      break;
    }
    set_SS_mode(mode);
  }while(0);
}

/*xxxFGROUP SnapShot
  Get the Snap Shot memory mode
*/
int GetModeSSM(){
  int retval = 0;
  retval = get_SS_mode();
  switch(retval){
  case SSM_SCOPE:
    printf("SSM mode is SCOPE\n");
    break;
  case SSM_SEQ:
    printf("SSM mode is SEQ\n");
    break;
  default:
    printf("SSM undefined, probably BUG in the code, contact experts\n");
    break;
  }
  return retval;
}

/*xxxFGROUP SelectDump
  Hexadecimal and binary dump of the SSM contents. Starts from the 1-st
  recorded event and dumps as many events as set by SelDumpN(n)
 */
void HexBinDump(){
  int i;
  do{
    _TEST_AVAIL_;
    i = ttcit1_hexbin_prt(&SSM,0,0);
    if(i < 0){
      printf("NO MORE DATA IN SSM BUFFER!\n");
    }
  }while(0);
}

/*xxxFGROUP SelectDump
  Hexadecimal and binary dumo of the SSM contents. Starts where the last
  dump via HexBinDump() or HexBinDumpNext() ended. Continuation print.
 */
void HexBinDumpNext(){
  int i;
  do{
    _TEST_AVAIL_;
    i = ttcit1_hexbin_prt(&SSM,-1,0);
    if(i < 0){
      printf("NO MORE DATA IN SSM BUFFER\n");
    }
  }while(0);

}

/*xxxFGROUP SelectDump
  Human readable dump (works only with firmware version >= 24). What to
  print can be selected using SelDumpOptions(...), how many events to print
  is set by SelDumpN(n).
 */
int SelDump(){
  int retval = 0;
  do{
    _TEST_AVAIL_;
    retval = ttcit1_ssm_prt(&SSM,0,0);
  }while(0);
  return retval;
}

/*xxxFGROUP SelectDump
  Resets TTCit, waits till SSM is full, fetches data and prints 
  the first N words in hex/binary form
 */
int SelFetchHexBinDump(){
  int retval = 0;
  do{
    _TEST_AVAIL_;
    retval = ttcit1_fetch_dump(TTCIT1_WHAT_BINHEX_);
  }while(0);
  return retval;
}

/*xxxFGROUP SelectDump
  Resets TTCit, waits until the SSM is full, fetches SSM from TTCit board,
  and prints the first N recorded events (selected print available using
  SelDumOptions(...)).
 */
int SelFetchDump(){
  int retval = 0;
  do{
    _TEST_AVAIL_;
    retval = ttcit1_fetch_dump(TTCIT1_WHAT_HUMAN_);
  }while(0);
  return retval;
}

/*xxxFGROUP SelectDump
  Continuation dump. Prints next N (set by SelDumpN()) events from SSM,
  starting where the last print using SelDump(), SelDumpNext(), 
  or SelFetchDump() stopped.
 */
int SelDumpNext(){
  int retval = 0;
  do{
    _TEST_AVAIL_;
    retval = ttcit1_ssm_prt(&SSM,-1,0);
  }while(0);
  return retval;
}

/*xxxFGROUP SelectDump
  Set SSM dumping options. 0 = disable option
                           1 = enable it
  Orbit     : print Orbit reset commands
  TTC_A     : print all traffic from the TTC A channel (L0, L1 accept)
  TTC_B     : print all traffic from the TTC B channel (L1M, L2a, L2r...)
  L0_LVDS   : print L0 signals sent over LVDS cable
  PP        : print PP signals
  TTCrx     : print information/data from the TTCrx chip
  EventType : print exact signature of SSM event
 */
void SelDumpOptions(int Orbit, int TTC_A, 
		    int TTC_B, int L0_LVDS, int PP, int TTCrx,
		    int EventType){

  do{
    _TEST_AVAIL_;
    _SET_BOOL_(t1o.pTick,1);
    _SET_BOOL_(t1o.pOrbit,Orbit);
    _SET_BOOL_(t1o.pBC,1);
    _SET_BOOL_(t1o.pTTC_A,TTC_A);
    _SET_BOOL_(t1o.pTTC_B,TTC_B);
    _SET_BOOL_(t1o.pL0_LVDS,L0_LVDS);
    _SET_BOOL_(t1o.pPP,PP);
    _SET_BOOL_(t1o.pTTCrx,TTCrx);
    _SET_BOOL_(t1o.pSig,EventType);
  }while(0);
}

/*xxxFGROUP SelectDump
  Prints options for the SelDump, SelDumpNext, SelFetchDump, HexBinDump,
  HexBinDumpNext.
 */
void SelPrintDumpOpt(){
  char *ans[2] = { "NO", "YES"};
  char *ms[2] = { "UNSET", "SET"};
  char c32[33];
  struct ttcit1_1st_mask *msk;
  do{
#ifdef TTCIT_SSM_VER_NEW_1_
    printf("Options for SelectDump module:\n");
    printf("==============================\n");
    printf("How many events/words to print        %d\n",t1o.N);
    printf("Position of the 1-st printed event    %d\n",t1o.pos_1st);
    printf("Print Tick?                           %s\n",ans[t1o.pTick]);
    printf("Print Orbit?                          %s\n",ans[t1o.pOrbit]);
    printf("Print BC number?                      %s\n",ans[t1o.pBC]);
    printf("Print TTC A channel traffic?          %s\n",ans[t1o.pTTC_A]);
    printf("Print TTC B channel traffic?          %s\n",ans[t1o.pTTC_B]);
    printf("Print L0 over LVDS cable?             %s\n",ans[t1o.pL0_LVDS]);
    printf("Print PP?                             %s\n",ans[t1o.pPP]);
    printf("Print TTC Rx data/commands?           %s\n",ans[t1o.pTTCrx]);
    printf("Print event signatures?               %s\n",ans[t1o.pSig]);
    printf("\n");
    printf("Max timeout = %d seconds\n",t1o.t_out_s);
    printf("\n");
    ttcit1_int2bin(t1o.sel1st,&c32[0]);
    printf("1st dumped event mask            %s\n",&c32[0]);
    msk = (struct ttcit1_1st_mask *)(&t1o.sel1st);
    printf("                      L0         %s\n",ms[msk->L0]);
    printf("                      L0 wire    %s\n",ms[msk->L0_wire]);
    printf("                      L1         %s\n",ms[msk->L1]);
    printf("                      Orbit      %s\n",ms[msk->Orbit]);
    printf("                      L1m        %s\n",ms[msk->L1m]);
    printf("                      L2a        %s\n",ms[msk->L2a]);
    printf("                      L2r        %s\n",ms[msk->L2r]);
    printf("                      PP         %s\n",ms[msk->PP]);
    printf("-------------------------------------------------------\n");
#endif
#ifdef TTCIT_SSM_VER_OLD_
    printf("Max timeout = %d seconds\n",t1o.t_out_s);
#endif
  }while(0);
}

/*xxxFGROUP SelectDump
  Set how many events stored in the SSM are to be printed at the same time
  (the number of SSM events is quite large so set this number not too high,
  otherwise the output became unmanageable)

  0 == print ALL SSM events (not recommended, use at your own risk)
 */
int SelDumpN(int howmany){
  int retval = 0;
  do{
    _TEST_AVAIL_;
    t1o.N = howmany;
  }while(0);
  return retval;
}

/*xxxFGROUP SelectDump
  Set the max allowed timeout in seconds for combined fetch_and_dumps

  Time_s   = max allowed timeout in seconds
 */
void SelDumpTimeouts(int Time_s){
  do{
    t1o.t_out_s = Time_s;
  }while(0);
}

/*xxxFGROUP SelectDump
  Allows to set the first event in the dump, where you want to start your 
  print. The Sel mask is to be given as OR for all events that are to be used
  as a starting point.
  This selection affects ALL dumping routines, even  SelDumpNext(), if the 
  dump should continue where the last print stopped, you must deselect
  1-st event option:

  Sel = 0x1      L0
	0x2      L0 over wire
	0x4      L1
	0x8      Orbit
	0x10     L1m
	0x20     L2a
	0x40     L2r
	0x80     PP
*/
void SelDump1stEvent(int Sel){
  do{
    _TEST_AVAIL_;
    t1o.sel1st = Sel;
  }while(0);
}

/*xxxFGROUP SelectDump
  Set the first dumped event type mask in a more user friendly way.

  Nonzero value = select
  Zero          = do not select

  If you want to disable the 1-st dumped event selection
  use SelDump1stEvent(0xff);
 */
void SelDump1stMenu(int L0, int L0_LVDS, int L1, int Orbit, int L1m,
		    int L2a, int L2r, int PP){
  struct ttcit1_1st_mask *msk;
  do{
    _TEST_AVAIL_;
    msk = (struct ttcit1_1st_mask *)(&t1o.sel1st);
    t1o.sel1st = 0x0;
    _SET_BOOL_(msk->L0,L0);
    _SET_BOOL_(msk->L0_wire,L0_LVDS);
    _SET_BOOL_(msk->L1,L1);
    _SET_BOOL_(msk->Orbit,Orbit);
    _SET_BOOL_(msk->L1m,L1m);
    _SET_BOOL_(msk->L2a,L2a);
    _SET_BOOL_(msk->L2r,L2r);
    _SET_BOOL_(msk->PP,PP);
  }while(0);
}

/*xxxFGROUP SelectDump
  Sets L0 signal source - cable or fibre. Proper knowledge of this
  is needed to interpret the signals in TTC A channel (L0/L1)

  OverFibre  = 1 if L0 is sent over fibre or over both wire and fibre
               0 if L0 is not sent over TTC A channel (fibre)
 */
void SelDumpL0(int OverFibre){
  do{
    _TEST_AVAIL_;
    _SET_BOOL_(t1o.L0_over_fibre,OverFibre);
  }while(0);
}

/*xxxFGROUP SelectDump
  Resests the default values (Print all, start at any event
 */
void SelDumpDefaults(){
  ttcit1_init();
}

/*xxxFGROUP SelectDump
 */
void SelDumpCountSSM(){
  int irc;
  do{
    _TEST_AVAIL_;
    irc = ttcit1_counters();
    if(irc != 0){
      printf("SSM scan failed, IRC = %d\n",irc);
      break;
    }
    /*
      Print the counters
     */
    printf("SSM event counters:\n");
    printf("===================\n");
    printf("\n");
    printf("L0 sent over TTC         %d\n",t1o.l0f);
    printf("L0 sent over LVDS (wire) %d\n",t1o.l0w);
    printf("L1                       %d\n",t1o.l1);
    printf("L1M headers              %d\n",t1o.l1m);
    printf("L1M words                %d\n",t1o.l1mw);
    printf("L2a headers              %d\n",t1o.l2a);
    printf("L2a words                %d\n",t1o.l2aw);
    printf("L2r messages             %d\n",t1o.l2r);
    printf("Pre Pulses               %d\n",t1o.pp);
    printf("RoI headers              %d\n",t1o.RoIh);
    printf("RoI words                %d\n",t1o.RoIw);
    printf("TTC Rx data/commands     %d\n",t1o.rx);
    printf("Orbits                   %d\n",t1o.orb);
    printf("TTC B undefined words    %d\n",t1o.B_undef);
    printf("Unknowns                 %d\n",t1o.unknown);
    printf("===================\n");
     /*
      We must reset the event counters, so that the new SelDumpNext will
      start from the beginning
     */
    ttcit1_evdec_reset();
  }while(0);
}

/*xxxFGROUP SelectDump
  Select the position of the 1-st event to be printed

  The N-th event matching the 1-st event mask wil be printed in the 
  following call to HexBinDumpNext(), SelDumpNext().

  This is meant as a help in browsing the large Snap Shot.

  N = 0           the 1-st event 
      1 .. MAX    i-th event (events are numbered from 1 to max)
                  if i > Max nr. of events stored in SSM then the last
                  event matching 1-st event criteria is chosen
      If you waht to see tha last occurence of event accorfing to the
      1-st event mask, run SelDumpCountSSM(), see how many of them is
      in the snapshot and select some bigger number.

    return: 0    on success
           -1    if no event satisfy the 1-st event selection criteria
 */
int SelDumpSetEvent(int N){
  int irc = 0;
  do{
    _TEST_AVAIL_;
    t1o.pos_1st = N;
    irc = ttcit1_reposit();
    if(irc < 0){
      printf("No event matches the 1-st event selection criteria\n");
    }else{
      printf("SSM buffer index of event found = %d\n",irc);
      irc = 0;
    }
  }while(0);
  return irc;
}

/*FGROUP Debug
Get number of words stored in Bunch Crossing TTCRx FIFO
 */
int GetNbcntFIFO(){
  int retval = 0;
  retval = get_nr_words_bcnt_fifo();
  return retval;
}

/*FGROUP Debug
Read the whole bunch crossing FIFO, stora data in memory
 */
int FetchBcntFIFO(){
  int retval = 0;
  int stat;
  do{
    /*
      Check that the soft MONITOR is not running
     */
    stat = StatusMONITOR();
    if(stat != MON_STOPPED){
      printf("Action inteferes with running Soft MINITOR\n");
      printf("Stop it and resubmit your action\n");
      break;
    }
    retval = fetch_bcnt_fifo();
  }while(0);
  return retval;
}

/*FGROUP SoftMONITOR
Stops the running soft MONITOR loop. Does nothing if nothing is looping.
 */
void StopMONITOR(){
  soft_monitor_stop();
}

/*FGROUP SoftMONITOR
  Shows status of the software monitor (running status, nr. of loops 
  completed etc...)

  Returns:    0xA     MONITOR is RUNNING
              0xB                STOPPING, transition to stopped state
              0xC                STOPPED

              0xBAD  is a BUG, call experts
 */
int StatusMONITOR(){
  int retval = 0;
  unsigned long shots = 0;
  retval = soft_monitor_status();
  switch(retval){
  case MON_RUNNING:
    shots = soft_monitor_snapshots();
    printf("MONITOR is  RUNNIG:  ... read %lu snapshots\n",shots);
    break;
  case MON_STOPPING:
    printf("MONITOR is STOPPING, transition to stop\n");
    break;
  case MON_STOPPED:
    printf("MONITOR is STOPPED\n");
    break;
  default:
    printf("This is a BUG, call experts\n");
  }
  return retval;
}

/*FGROUP SoftMONITOR
  Read the stored file (created during a soft MONITOR run) and reanalyze it.
  To be used when one first looks at the errors that happen (maybe not very
  often) and then wants to investigate them one by one. 

  Opens the file, reads, reached a stop condition-stops, closes file

  To use this function, you must first run: WriteFile_ON()
                                            StartMONITOR()
*/
void Rescan(){
  soft_monitor_rescan(0);
}

/*FGROUP SoftMONITOR 
  Read the stored file (created during a soft MONITOR run) and reanalyze it.
  To be used when one first looks at the errors that happen (maybe not very
  often) and then wants to investigate them one by one. 

  Opens the file, reads, reached a stop condition-stops, leaves the file opened
  for subsequent reads

  To use this function, you must first run: WriteFile_ON()
                                            StartMONITOR()
 */
void RescanLeave(){
  soft_monitor_rescan(1);
}

/*FGROUP SoftMONITOR
  Read the stored file (created during a soft MONITOR run) and reanalyze it.
  To be used when one first looks at the errors that happen (maybe not very
  often) and then wants to investigate them one by one. 

  Reads already opened file, reached a stop condition-stops, leaves the file
  opened

  To use this function, you must first run: WriteFile_ON()
                                            StartMONITOR()
 */
void RecsanCont(){
  soft_monitor_rescan(2);
}


/*FGROUP SoftMONITOR
  Ask MONITOR to write a file to be inspected later
 */
void WriteFile_ON(){
  soft_monitor_file(1);
}

/*FGROUP SoftMONITOR
  Ask MONITOR not to write any file
 */
void WriteFile_OFF(){
  soft_monitor_file(0);
}

/*FGROUP SoftMONITOR
  Ask MONITOR to perform on-line diagnostics
 */
void OnlineDiag_ON(){
  soft_monitor_online_diag(1);
}

/*FGROUP SoftMONITOR
  Ask MONITOR not to perform on-line diagnostics (if a file is written the
  same diagnostics can be performed off-line later)
 */
void OnlineDiag_OFF(){
  soft_monitor_online_diag(0);
}

/*FGROUP SoftMONITOR
  Show the current MONITOR options 
 */
void ShowMONITORoptions(){
  soft_monitor_file(2);
  soft_monitor_online_diag(2);
}

/*FGROUP SoftMONITOR
  Set stopping conditions for the Sotfware MONITOR :  ERRORS 

  If the MONITOR is to be stopped ad given error set argument to 1
  if the error is to be not used as stopping point set 0
 */
void SetStopCondition(int Stop_at_L0S,
		      int Stop_at_L1S,
		      int Stop_at_L1T,
		      int Stop_at_L1M,
		      int Stop_at_L1Mo,
		      int Stop_at_L1F,
		      int Stop_at_L2T,
		      int Stop_at_L2Ts,
		      int Stop_at_L2F,
		      int Stop_at_BCID_diff){
  struct MONITOR_stops s;

  get_TTCIT_stop_conds(&s);

  s.Stop_at_L0S = Stop_at_L0S;
  s.Stop_at_L1S = Stop_at_L1S;
  s.Stop_at_L1T = Stop_at_L1T;
  s.Stop_at_L1M = Stop_at_L1M;
  s.Stop_at_L1Mo = Stop_at_L1Mo;
  s.Stop_at_L1F = Stop_at_L1F;
  s.Stop_at_L2T = Stop_at_L2T;
  s.Stop_at_L2Ts = Stop_at_L2Ts;
  s.Stop_at_L2F = Stop_at_L2F;
  s.Stop_at_BCID_diff = Stop_at_BCID_diff;

  set_TTCIT_stop_conds(&s);
}

/*FGROUP SoftMONITOR
  Stop Software MONITOR at ANY error
 */
void SetStopAnyError(){
  struct MONITOR_stops s;
  get_TTCIT_stop_conds(&s);
  s.ANY_ERROR = TRUE;
  set_TTCIT_stop_conds(&s);
}

/*FGROUP SoftMONITOR
Stop Software monitor after reading NSEQ sequences (Snap shots)
 */
void SetStopConditionNSEQ(int NSEQ){
  struct MONITOR_stops s;
  get_TTCIT_stop_conds(&s);
  s.NSEQ = NSEQ;
  set_TTCIT_stop_conds(&s);
}

/*FGROUP SoftMONITOR
  Clear STOP conditions, after this the Software MONITOR never stops and
  must be stopped manually
 */
void SetNEVERstop(){
  set_TTCIT_default_stops();
}

/*FGROUP SoftMONITOR
  Prints the actual Stopping conditions for Software MONITOR
 */
void ShowStopConditions(){
  print_TTCIT_stop_conds();
}

/*FGROUP SSM_analyz
  Resets the SSM analyzer to default values, clears all counters
 */
void ResetSSManalyzer(){
  ttc_it_start_analyz();
  ttc_it_reset_options();
}

/*FGROUP SSM_analyz
  Sets the L1M, L2A messages to old format (short messages)
 */
void SetL12MesOld(){
  ttc_it_clear_oldmes();
  ttc_it_clear_printmes();
}

/*FGROUP SSM_analyz
  Sets the L1M, L2A messages to new format (long messages)
 */
void SetL12MesNew(){
  ttc_it_clear_newmes();
  ttc_it_clear_printmes();
}

/*FGROUP SSM_analyz
  Clears all counters, timings etc...
 */
void ClearStat(){
  ttc_it_clear_zeroos();
}

/*FGROUP SSM_analyz
  Prints the numbers of words for L1M, L2A, L2R, RoI and Res messages
  expected
 */
void PrintL12Mes(){
  ttc_it_clear_printmes();
}

/*FGROUP SSM_analyz
  Shows the contents of the trigger counters (as seen by SSM analyzer)
 */
void ShowCounters(){
  ttc_it_print_last_counters();
}

/*FGROUP SSM_analyz
  Show the contents of the error counters (as seen by SSM analyzer)
 */
void ShowErrors(){
  ttc_it_print_last_errors();
}

/*FGROUP SSM_analyz
  Show the timing infor as calculated by SSM analyzer
 */
void ShowTiming(){
  ttc_it_print_last_timing();
}

/*FGROUP SSM_analyz
  Defines the software time windows used in definition of some trigger 
  errors

  All time value are given in 26ns ticks

  Setting half width to 0 turns intervals into single fixed numbers

  L0_L1_window      = centre of the L0-L1 decision interval

  L0_L1_width       = half width of the L0-L1 decision interval 

  L1_timeout        = timout for L1m from L1 signal, after this time 
                      the L1m is assumed to be missing

  L1_L2_timeout     = timout for L2a header or L2r, after this time the 
                      L2a/L2r is assumed to be missing

  L1_L2_BCID_window = centre of the window in which the difference in 
                      BCID from TTC Rx at the arrival of L1 signal and the 
		      BCID value found in L2a/L2r must be found. If not, 
		      an error is signalled

  L1_L2_BCID_width  = half width of the BCID window.
 */
void SetTimeWindows(int L0_L1_window, int L0_L1_width,
		    int L1m_timeout, int L1_L2_timeout,
		    int L1_L2_BCID_window, int L1_L2_BCID_width,
		    int L1_RoI_timeout){
  ttc_it_set_opt_wins(L0_L1_window, L0_L1_width, L1m_timeout, L1_L2_timeout,
		      L1_L2_BCID_window, L1_L2_BCID_width,
		      L1_RoI_timeout);
}

/*FGROUP SSM_analyz
  Set the Bunch crossing during which the PrePulse is sent and set limiting
  BC that can be used to mark old L2 i.e. those L2 which follow L0-L1-L1m
  that has not been recorded in SSM (because of ResetTTCit came after sending
  the L0-L1-L1m) when those originated in the orbit preceding the one 
  recorded in SSM

  BC_PP        = bunch crossing during which the PrePulse is sent
  BC_old_L2    = old L2 bunch crossing boundary (this value must be tuned)
  BC_new_L2    = new L0-L1 orbit last BC boundary
 */
void SetBCconst(int BC_PP, int BC_old_L2, int BC_new_L2){
  ttc_it_set_opt_const(BC_PP, BC_old_L2, BC_new_L2);
}

/*FGROUP SSM_analyz
  Set the default values of the time windows used in definitions of some
  trigger errors
 */
void DefaultTimeWindows(){
  ttc_it_reset_options();
}

/*FGROUP SSM_analyz
  Shows the actual values of the time windows used in definitions of some 
  trigger errors.
 */
void ShowTimeWindows(){
  ttc_it_print_opt_windows();
}

/*FGROUP SSM_analyz
  Quick look at the activity on the optical line. Combines the following 
  actions:

  1) ResetTTCit                 : Start reading one SSM
  2) wail till SSM gets filled 
  3) FetchSSM
  4) FetchBcntFIFO
  5) perform analyzis on one Snap Shot
  6) print error count
  7) dump the SSM with indication of error occurence
 */
void AnalyzeOneSnapShot(){
  soft_monitor_single_ssm(TRUE);
}

/*xxxFGROUP SSM_analyz
  Analyze N snap shots, stop at 1-st error
 */
void AnalyzeN(int n){
  soft_monitor_N_ssm(n);
}

/*xxxFGROUP SSM_analyz
  Tries to analyze SSM snapshots and in those where NO ERRORS are present
  the BCID differences are evaluated and histogrammed. Snapshots with errors
  are ignored.

  This is supposed to work independently of bugs in TTCit soft logic
 */
void BcidMismatch(int n){
  //int irc = 0;
  //int samples, maxrpt;

#ifdef _MONITOR_SCAN_BCID_DIFF__
  samples = n;
  maxrpt = 3;
  irc = BCID_mismatch_finder(samples, maxrpt);
  if(irc != 0){
    printf("BCID Mismatch Analysis FAILED, IRC = %d\n", irc);
  }
#else
  printf("This function has not been compiled in\n");
#endif
}

/*FGROUP Debug
  Print the contents of the BC Fifo (only differing words are printed
  nw = number of words you want to print (0 == all)
 */
void PrintBcntFIFO(int nw){
  print_bcnt_fifo_buf(nw);
}

/*FGROUP TOP GUI TTCitCounters
  Counters for On-Line Monitor (OM) and Software Snap-shot Monitor (SM)
 */

/*FGROUP TOP GUI Scope_Signals
  Scope signal selection A/B
 */

/*FGROUP Debug
  Test fetching SSM 
 */
void TestFetchSSM(){
  test_fetch_SSM(0);
}

/*FGROUP Debug
Get board type
*/
int GetBoardType(){

  int btype;
  w32 wrdBoardType;
  w32 typeMask = 0xff;

  wrdBoardType = VMER32(VERSION_NUMBER);
  btype = wrdBoardType & typeMask;

  /*
    printf("Detected board type = %X\n",btype); 
  */

  return btype;
}

/*FGROUP Debug
Get VME comtroller version
*/
int GetVMEContVersion(){

  int vmeVersion;
  w32 wrdVMEversion;
  w32 vmeVersMask = 0xff;

#if NOT_YET_DONE
  wrdVMEversion = VMER32(VME_VERSION_ADD);
#else
  wrdVMEversion = 0xdead;
#endif

  vmeVersion = wrdVMEversion & vmeVersMask;

  printf("Detected VME Controller version = %X\n",vmeVersion);

  return vmeVersion;
}

/*FGROUP Debug
  n        = number of SSM fetches
  nretries = number of dummy loops before each retry (D=100)
 */
void LoopTestSSM(int n, int nretries){
  loop_test_fetch_SSM(n, nretries);
}

/*xxFGROUP Debug
  Test what is written into the SSM in Simulator
  nw = number of words to be printed ( 0 == all)
 */
void TestSSMsim(int nw){
#ifdef SIMVME
  dump_SS(nw);
#else
  printf("Function implemented only for SIMVME driver\n");
#endif
}

/*FGROUP Debug 
Returns SS.top
 */
int SS_top(){
  int retval = 0;
#ifdef SIMVME
  retval = get_SS_top();
#else
  printf("Function implemented only for SIMVME driver\n");
#endif
  return retval;
}

/*FGROUP Debug
  Get next word from the BC fifo
 */
int GetWordBCfifo(){
  int retval = 0;
  retval = VMER32(BCNT_TTCRX);
  return retval;
}

/*FGROUP Debug
  Print Internal state of the TTCIT software logic
 */
void PrintTTCITLogic(){
  print_TTCIT_logic_state();
}

/*xxFGROUP Debug
  Set parameters for simlated triggers, usable only with SIMVME driver

  Setting -1 in any of these parameters reverts to the default value

  Tdist      = distance between 2 L0 triggers (in 26ns ticks)
  T_L0_L1    = distance between L0 - L1 in 26ns ticks
  T_L1_L1M   = distance between L1 and L1M in 26ns ticks
  T_L1_L2    = distance between L1 and L2a / L2r in 26ns ticks

 */
void SetTriggerParams(int Tdist, int T_L0_L1, int T_L1_L1M, int T_L1M_DATA,
		      int T_L1_L2, int BC_L1_L2){
#ifdef SIMVME
  sim_set_trigger_params(Tdist, T_L0_L1, T_L1_L1M,T_L1M_DATA,
			 T_L1_L2, BC_L1_L2);
#else
  printf("Function available only with SIMVME driver - software simulation\n");
#endif
}

/*xxFGROUP Debug
  Shows the trigger parameters
 */
void ShowTriggerParams(){
#ifdef SIMVME
  sim_show_trigger_params();
#else
  printf("Function available only with SIMVME driver\n");
#endif
}

/*xxFGROUP Debug
  Set the rate of rejected triggers

  L1r       = Nr. of L0 signals before one L1 signal missing (i.e. no L1M, L2)
              0    = all L1 signals arrive
             -1    = do not change this value

  L2r       = Nr. of L2a messages before one L1r message arrives
              0    = all L2 messages are L2a
             -1    = do not change this value
 */
void SetTriggerRejects(int L1r, int L2r){
#ifdef SIMVME
  sim_set_trigger_rejects(L1r, L2r);
#else
  printf("Function available only with SIMVME driver\n");
#endif
}

/*xxFGROUP Debug
  Shows rate of rejected triggers
 */
void ShowTriggerRejects(){
#ifdef SIMVME
  sim_show_trigger_rejects();
#else
  printf("Function available only with SIMVME driver\n");
#endif
}

/*xxFGROUP Debug
  Sets the rate of simulated trigger errors

  ABS(val) is the number of L0 triggers that must pass before an error is
  generated - the whole process is deterministic in order that I might be
  able to trace the errors and debug the TTCit logic code.

  L0S       >0   Surplus L0, L0 arrives during active L0 - L1 decistion time
  L1S       >0   Surplus L1, L1 arrives without preceding L0 signal
  L1T      <>0   L0 - L1 time violation, L1 arrives sooner (<0) or later (>0)
                 than the decition interval
  L1M       >0   L1M transmitted without L1 signal
  L1Mo     <>0   L1M tnasmitted after programmable interval (>0)
                 or not transmitted at all (<0)
  L1F      <>0   L1M format error, Either missing header or words (<0) 
                 or surplus words (>0) in the sequence
  L2T       >0   L1 - L2 time violation, L2a/L2r not transmitted after L1
  L2Ts      >0   L2a/L2r transmitted without L1 signal
  L2F      <>0   L2a format error, too many (>0) or too few (<0) words 
                 in the L2a message
  BCID      >0   BC ID of L2a/L2r different from the content of the 
                 TTCRx BC counter at the arrival of L1 signal

  It looks like the GUI/UI cannot handle negative numbers, 
  so the <>0 alternatives will switch between >0 and <0 chices. Use only
  positive values.

 */
void SetTriggerErrors(int L0S, int L1S, int L1T, int L1M, int L1Mo, int L1F,
		      int L2T, int L2Ts, int L2F, int BCID){
#ifdef SIMVME
  sim_set_trigger_errors(L0S, L1S, L1T, L1M, L1Mo, L1F, L2T, L2Ts, L2F, BCID);
#else
  printf("Function available only with SIMVME driver\n");
#endif
}

/*xxFGROUP Debug
  Shows the rate of simulated trigger errors
 */
void ShowTriggerErrors(){
#ifdef SIMVME
  sim_show_trigger_errors();
#else
  printf("Function available only with SIMVME driver\n");
#endif
}

/*xxFGROUP Debug
  Resets simualated trigger constants
 */
void ResetSimTriggerDefaults(){
#ifdef SIMVME
  sim_reset_trigger();
#else
  printf("Function available only for SIMVME driver\n");
#endif
}

/*xxFGROUP Debug
  Show contents of the simulated trigger counters
*/
void ShowSimTriggerCounters(){
#ifdef SIMVME
  sim_show_counters();
#else
  printf("Function available only for SIMVME driver\n");
#endif
}

/*xxFGROUP Debug
  Clears simulated Trigger counters
 */
void ClearSimTriggerCounters(){
#ifdef SIMVME
  sim_reset_counters();
#else
  printf("Function available only with SIMVME trigger\n");
#endif
}

/*xxFGROUP Debug
  Simulate L0 triggers sent over A channel: ON
 */
void SimL0overAchannelON(){
#ifdef SIMVME
  sim_L0_over_A(1);
#else
  printf("Function available only with SIMVME driver\n");
#endif
}

/*xxFGROUP Debug
  Stop simulating L0 triggers over A channel: OFF
 */
void SimL0overAchannelOFF(){
#ifdef SIMVME
  sim_L0_over_A(0);
#else
  printf("Function available only with SIMVME driver\n");
#endif
}

/*xxFGROUP Debug
 */
void SimDrop1stL0(){
#ifdef SIMVME
  sim_drop_1st_l0(1);
#else
  printf("Function available only with SIMVME driver\n");
#endif
}

/*xxFGROUP Debug
 */
void SimUndrop1stL0(){
#ifdef SIMVME
  sim_drop_1st_l0(0);
#else
  printf("Function available only with SIMVME driver\n");
#endif
}

/*xxFGROUP Debug
  Try the trigger sequence generator
 */
void SimTryTriggerSequence(){
#ifdef SIMVME
  sim_try_trig_seq();
#else
  printf("This function is available only for SIMVME driver\n");
#endif
}

/*  XXFGROUP Debug
  Test the Simulated FPGA filling loop
 */
void SimDebugFPGAfill(){
#ifdef SIMVME
  int i;
  ClearSSMemoryBuffer();
  sim_debug_fpga_fill();
  i = ReadAddressSSM();
  if(i > 10){
    FetchSSM();
    DumpSSM(0);
  }else{
    printf("It looks like too few items in SSM\n");
  }
  i = GetNbcntFIFO();
  if(i <= 0){
    printf("No data in BC FIFO\n");
  }else{
    FetchBcntFIFO();
    PrintBcntFIFO(0);
  }
#else
  printf("Function available only for SIMVME driver\n");
#endif
}

/*xxFGROUP Debug
  Remove this!!!!!
 */
void TestMemManagment(){
  ttcit_mm_test();
}

/*FGROUP Debug
  Read a part of the MONITOR file and dump its contents as HEX and DEC
  It is to be used for debug only

  nwords   = number of words to be read and dumped
 */
void DumpWrittenData(int nwords){

  int irc = 0;

  char file[] = TTCIT_MONITOR_FILE;
  int fd;

  unsigned long *buf = NULL;
  size_t nmemb = 0;
  size_t size = 0;

  do{
    fd = ttcit_io_open(&file[0], TTCIT_IO_READ);
    if(fd == -1){
      printf("DumpWrittenData: Canna open monitor file\n");
      break;
    }

    if(nwords <= 0){
      printf("DumpWrittenData: No data requested, nwords = %d\n",nwords);
      break;
    }

    size = sizeof(unsigned long);
    nmemb = (size_t)nwords;
    buf = (unsigned long *)calloc(nmemb, size);
    if(buf == NULL){
      printf("Canna allocate memory buffer\n");
      break;
    }

    irc = ttcit_io_read_dump(fd, nwords, buf);

  }while(0);

  if(buf != NULL){
    free(buf);
  }
}

/*FGROUP Debug
  Dump the contents on the ttcit_monitor.dat as TTCit events. It is used
  for debugging the I/O operations. This call may produce HUGE output.
 */
void DumpFileEvents(){
  soft_monitor_dumpfile();
}

/*FGROUP Debug
  Debugging tool for manual command clearCounters()
 */
void GUI_ClearCounters(){
  clearCounters();
}

/*FGROUP Debug
  Debugging tool for manual command getCounters(N,difval)
 */
void GUI_GetCounters(int difval){
  getCounters(TTCIT_COUNTERS_NCOUNTS,difval);
}

/*FGROUP Debug
  Test of the ClearSSM function
 */
void DBG_TestClearSSM(){
  test_clear_SSM_on_board();
}

/*FGROUP Debug
  Activate OM but do not start collecting data
*/
void OM_ON_nostart(){
  OM_set(OM__ON_NOSTART);
}

/*ENDOFCF */

clock_t timeInfoPerSec(){
  long tpc = 0;
  tpc = sysconf(_SC_CLK_TCK);
  return (clock_t)tpc;
}

/*
  GUI functions
 */


/*
  Read all counters and fills mem 
 */
void readCounters(w32 *mem, int NCNTS, int difval){
  int i;
  struct om_hw_counters ct;
  /*struct ttc_it_counters smc;
  struct ttc_it_errors sme;*/
  w32 old;
  w32 neww;
  w32 dif;

  /*
    If init == 1 then copy old neww to prev at THIS point
   */
  if(TTCit_counters.init != 0){
    swapCounters();
  }

  /*
    1) Get all NEW values
   */ 

  /*
        1a) Get all OM counters
   */
  OM_get_counters(&ct);

  /*
        1b) Get all SM counters - not at this moment 
   */

  /*
  ttc_it_collect_last_counterr(&smc, &sme);
  */

  /*
    2) Fill in new values
   */
  for(i = 0; i < NCNTS; i++){
    switch(i){
      TCOUNT_ADDC(CNT_OM_L0,ct.L0);
      TCOUNT_ADDC(CNT_OM_L1,ct.L1);
      TCOUNT_ADDC(CNT_OM_L1M,ct.L1m);
      TCOUNT_ADDC(CNT_OM_L2A,ct.L2a);
      TCOUNT_ADDC(CNT_OM_L2R,ct.L2r);

      TCOUNT_ADDC(CNT_OM_ERR_L0S,ct.L0S);
      TCOUNT_ADDC(CNT_OM_ERR_L1S,ct.L1S);

      TCOUNT_ADDC(CNT_OM_ERR_L1MM,ct.L1MM);
      TCOUNT_ADDC(CNT_OM_ERR_L1MS,ct.L1MS);
      TCOUNT_ADDC(CNT_OM_ERR_L1MI,ct.L1MI);
      TCOUNT_ADDC(CNT_OM_ERR_L1MD,ct.L1MD);

      TCOUNT_ADDC(CNT_OM_ERR_L2MM,ct.L2MM);
      TCOUNT_ADDC(CNT_OM_ERR_L2MS,ct.L2MS);
      TCOUNT_ADDC(CNT_OM_ERR_L2MI,ct.L2MI);
      TCOUNT_ADDC(CNT_OM_ERR_L2MD,ct.L2MD);

      TCOUNT_ADDC(CNT_OM_ERR_CAL,ct.CAL);
      TCOUNT_ADDC(CNT_OM_ERR_BCNT,ct.BCNT);
      TCOUNT_ADDC(CNT_OM_COUNT_PP,ct.PP_count);


    default:                       /* We should not get there */
      break;
    }
  }
  /*
    If init == 0 then copy new to prev at THIS point
   */
  if(TTCit_counters.init == 0){
    swapCounters();
  }

  /*
    Fill the output memory buffer mem
   */
  for(i = 0; i < NCNTS; i++){
    if(difval == 1){                             /* Differences */
      old = (w32)TTCit_counters.prev[i];
      neww = (w32)TTCit_counters.neww[i];
      if(neww >= old){
	dif = neww - old;
      }else{
	dif = (0xffffffff - old) + neww + 1;
      }
    }else{                                       /* Actual values */
      dif = (w32)TTCit_counters.neww[i];
    }
    mem[i] = dif;
  }

}

/*
  Initialization routine - filling all counters with 0's initially
 */
void initCounters(){
  TTCit_counters.nCount = TTCIT_COUNTERS_NCOUNTS;
  clearCounters();
}

void swapCounters(){
  int i;
  for(i = 0; i < TTCit_counters.nCount; i++){
    TTCit_counters.prev[i] = TTCit_counters.neww[i];
  }
  TTCit_counters.init = 1;
}

