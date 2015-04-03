#ifndef _TTCIT_H__
#define _TTCIT_H__

#include "ttcit_conf.h"
#include <time.h>
#include "fm_fpga.h"

/*
                TTCit VME registers
 */


/*
                 VME controller FPGA

                (Some of them should not be visible in the final
                 version to be used by anybody)
*/

/*REGSTART32 */

#define VERSION_NUMBER     0x4  /* (R) Toto by niekedy malo byt board type */ 
#define TTCIT_CONTROL      0x8  /* (R/W)Control register  */
#define TTCIT_STATUS       0xc  /* (R)Status register */
#define RESET_TTCRX        0x10 /* (W) Reset TTCRx chip */
#define VME_CONTROL_VERS   0x3c /* (R) VME controller version */

/*
  Snapshot memory:

  It is not exactly FIFO, but an ordinary RAM. That is why it is easy to
  implement the following reading scheme:

  1) Read the address counter: VMER32( READ_ADDR_COUNT )
     (you get the number of samples in the FIFO)
  
  2) Read all the data backwards, i.e. last dato is read first (now it looks 
     more like LIFO storage). 
 
  3) The address counter is decremented after each read. You should get 0 after
     the last read. 
 */

#define READ_ADDR_COUNT    0x14 /* (R) FIFO/Snapshot address counter */
#define READ_SNAPSHOT      0x18 /* (R/W) Read/Write from/to snapshot memory */
#define RESET              0x28 /* (W) Reset TTCit, before next read */
#define BCNT_TTCRX         0x1c /* (R) Read word from Bunch Xing FIFO */
#define N_BCNT_TTCRX       0x20 /* (R) Number of words in Bunch Xing FIFO */
#define RESET_SNAPSHOT_N   0x8c /* (W) Reset the SSM addr. count to 0 */

/*
  Flash Memory and FPGA
 */

#define CONFIG_START       0x2c   /* (W) Start conf. FPGA from Flash Memory */
#define FLASHADD_CLEAR     0x30   /* (W) Reset the FlashMem address counter */
#define FLASHACCESS_INCR   0x34   /* (R/W) Read/Write FM and Increment */
#define FLASHACCESS_NOINCR 0x38   /* (R/W) Read/Write FM and DON'T increment */

/*
  On-Line Monitor Hardware Registers
 */

#define HW_TIME_L0_L1      0x24
#define HW_RESET_COUNTERS  0x40   /* Reset all FPGA counters */

#define HW_COUNTER_L0      0x44   /* FPGA Counters : TRIGGERS */
#define HW_COUNTER_L1      0x48 
#define HW_COUNTER_L1M     0x4c
#define HW_COUNTER_L2A     0x50
#define HW_COUNTER_L2R     0x54

#define HW_COUNT_ERR_PP    0x58   /* FPGA ERROR Counters */
#define HW_COUNT_ERR_L0S   0x5c
#define HW_COUNT_ERR_L1S   0x60

#define HW_COUNT_ERR_L1MM  0x64
#define HW_COUNT_ERR_L1MS  0x68
#define HW_COUNT_ERR_L1MI  0x6c
#define HW_COUNT_ERR_L1MD  0x70

#define HW_COUNT_ERR_L2MM  0x74
#define HW_COUNT_ERR_L2MS  0x78
#define HW_COUNT_ERR_L2MI  0x7c
#define HW_COUNT_ERR_L2MD  0x80

#define HW_COUNT_ERR_CAL   0x84
#define HW_COUNT_ERR_BCNT  0x88

/*
  Scope signals
 */

#define SCOPE_SELECTED_A   0x90
#define SCOPE_SELECTED_B   0x94

/*
  Board serial number
 */
#define SERIAL_NUMBER     0x98

/*
  Delayed SSM stop after error detected in OnBoard monitor
 */
#define HW_DELAY_SSM_STOP  0x9c
#define HW_BCNT_DIFFERENCE 0xa0

/* PrePulse counter
 */
#define HW_PP_COUNTER 0xa4
    
/*REGEND */

/*
  Control Words for register 0x8 ( OR'ed )
 */

#define TTCIT_CTRL_CONTIWRIT      0x1    /* Enable continuous writing SSM */
#define TTCIT_CTRL_STOPCWRIT      0x2    /* Stop continuous writing */
#define TTCIT_CTRL_STOP_PP        0x4    /* Stop at PP error */
#define TTCIT_CTRL_STOP_L0S       0x8    /* Stop at spurious L0 error L0S */
#define TTCIT_CTRL_STOP_L1S       0x10   /* Stop at spurious L1 error L1S */
#define TTCIT_CTRL_STOP_L1MM      0x20   /* Stop at L1M missing */
#define TTCIT_CTRL_STOP_L1MS      0x40   /* Stop at L1M spurious */
#define TTCIT_CTRL_STOP_L1MI      0x80   /* Stop at L1M incomplete */
#define TTCIT_CTRL_STOP_L1MD      0x100  /* Stop at L1M data error */
#define TTCIT_CTRL_STOP_L2MM      0x200  /* Stop at L2M missing */
#define TTCIT_CTRL_STOP_L2MS      0x400  /* Stop at L2M spurious */
#define TTCIT_CTRL_STOP_L2MI      0x800  /* Stop at L2M incomplete */
#define TTCIT_CTRL_STOP_L2MD      0x1000 /* Stop at L2M data error */
#define TTCIT_CTRL_STOP_CAL       0x2000 /* Stop at Calibration error */
#define TTCIT_CTRL_STOP_BCNT      0x4000 /* Stop at BCNT difference */

/*
  Status words for the Status register 0xc ( OR'ed )

  Nonzero means a stop condition has been reached.
 */

#define TTCIT_STATUS_SSM_ACTIVE  0x1      /* SSM is taking data - b.0 */
#define TTCIT_STATUS_PP          0x2      /* PP error - b.1 */
#define TTCIT_STATUS_L0S         0x4      /* L0S error - b.2 */
#define TTCIT_STATUS_L1S         0x8      /* L1S error - b.3*/
#define TTCIT_STATUS_L1MM        0x10     /* L1M missing error - b.4 */
#define TTCIT_STATUS_L1MS        0x20     /* L1M spurious error -b.5 */
#define TTCIT_STATUS_L1MI        0x40     /* L1M incomplete  -b.6 */
#define TTCIT_STATUS_L1MD        0x80     /* L1M data error - b.7 */
#define TTCIT_STATUS_L2MM        0x100    /* L2M missing error - b.8 */
#define TTCIT_STATUS_L2MS        0x200    /* L2M spurious error -b.9 */
#define TTCIT_STATUS_L2MI        0x400    /* L2M incomplete - b.10 */
#define TTCIT_STATUS_L2MD        0x800    /* L2M data error - b.11 */
#define TTCIT_STATUS_CAL         0x1000   /* Calibration error -b.12 */
#define TTCIT_STATUS_BCNT        0x2000   /* BCNT difference error -b.13 */
#define TTCIT_STATUS_QPLLOK      0x4000   /* QPLL Locked  - b.14 */

/*
  To help with adding scope signals 
 */

#define TTCIT_LAST_SCOPE_BIT    20        /* Bit meaning NOT_SELECTED */

#ifdef SIMVME
#define VMER32 vmer32_sim
#define VMEW32 vmew32_sim
#else
#define VMER32 vmer32
#define VMEW32 vmew32
#endif

#define NO_BOARD_NUMBER 

/*
  Functions in ttcit.c
 */

void initmain();
void endmain();
void boardInit();


/*
            Configuration
 */

void TTCITinit();
void PrintTTCITLogic();

/*
            VMEcontroller  
 */

int GetBoardType();
int GetSerialNumber();
int GetVMEContVersion();
int FPGAcodeVersion();

/*
            Reset
*/

int ResetTTCit();
int TTCRxReset();

/*
            SnapShot
 */

int ReadAddressSSM();
int ReadLastSSM();
int FetchSSM();
void DumpSSM(int nw);
void ClearSSM();
void ClearSSMemoryBuffer();
void CombinedFetchAndDump();
int GetNrSSMwords();
int WriteSSMbuffer();
int ThrowSSMdata(int addr);
void SetModeSSM(int mode);
int GetModeSSM();
void RA();
void RawDumpSSM(int begin, int end);
void GetStatusSSM();

/*
            BunchXingFifo
 */

int GetNbcntFIFO();
int FetchBcntFIFO();
void PrintBcntFIFO(int nw);

/*
            SoftMONITOR
*/

int StartMONITOR();
void StopMONITOR();
int StatusMONITOR();
void WriteFile_ON();
void WriteFile_OFF();
void OnlineDiag_ON();
void OnlineDiag_OFF();
void ShowMONITORoptions();
void SetStopCondition(int Stop_at_L0S,
		      int Stop_at_L1S,
		      int Stop_at_L1T,
		      int Stop_at_L1M,
		      int Stop_at_L1Mo,
		      int Stop_at_L1F,
		      int Stop_at_L2T,
		      int Stop_at_L2Ts,
		      int Stop_at_L2F,
		      int Stop_at_BCID_diif);
void SetStopConditionNSEQ(int NSEQ);
void SetStopAnyError();
void SetNEVERstop();
void ShowStopConditions();
void Rescan();
void RescanCont();
void RescanLeave();


/*
            SSM_analyz
 */

void L0signal(int t);
void ResetSSManalyzer();
void ShowCounters();
void ShowErrors();
void ShowTiming();
void SetTimeWindows(int L0_L1_window, int L0_L1_width,
		    int L1_timeout, int L1_L2_timeout,
		    int L1_L2_BCID_window, int L1_L2_BCID_width,
		    int L1_RoI_timeout);
void SetBCconst(int BC_PP, int BC_old_L2, int BC_new_L2);
void ShowTimeWindows();
void DefaultTimeWindows();
void AnalyzeOneSnapShot();
void AnalyzeN(int n);
void BcidMismatch(int n);
void SetL12MesOld();
void SetL12MesNew();
void PrintL12Mes();
void ClearStat();

/*
            OM - On-Board Monitor
 */

void OM_Reset();
void OM_ON_start();
void OM_ON_nostart();
void OM_OFF();
void OM_STOP();
void OM_START();
void OM_Status();
void OM_SetErrorMask_msg(int L1M_missing,
			 int L1M_spurious,
			 int L1M_incomplete,
			 int L1M_data_err,
			 int L2M_missing,
			 int L2M_spurious,
			 int L2M_incomplete,
			 int L2M_data_error
			 );
void OM_SetErrorMask_sig(int PP,
			 int L0S,
			 int L1S,
			 int CAL_error,
			 int BCNT_diff
			 );

void OM_ShowErrorMask();
void OM_Detected_Errors();
void OM_ReloadLastErrorMask();
void OM_SaveErrorMask();
void OM_ClearErrorMask();
void OM_ShowCounters();
void OM_ResetCounters();
void OM_SetTimeL0_L1(int t);
void OM_TriggerCounters();
void OM_ErrorCounters();
void OM_PrintStatusSSM();
void OM_SSM_DumpAfterStop(int minutes);
void OM_SSM_AnalyzAfterStop(int minutes);
void OM_FetchSSM_Dump();
void OM_FetchSMM_Analyze();
void OM_ErrMaskSync(int how);
int OM_GetDelayedSSMstop();
void OM_SetDelayedSSMstop(int dt);

/*
            Debug
 */

void TestFetchSSM();
void LoopTestSMM(int n);
void TestSSMsim(int nw);
int SS_top();
int GetWordBCfifo();
void SetTriggerParams(int Tdist, int T_L0_L1, int T_L1_L1M, int T_L1M_DATA,
		      int T_L1_L2, int BC_L1_L2);
void ShowTriggerParams();
void SetTriggerRejects(int L1r, int L2r);
void ShowTriggerRejects();
void SetTriggerErrors(int L0S, int L1S, int L1T, int L1M, int L1Mo, int L1F,
		      int L2T, int L2Ts, int L2F, int BCIID);
void ShowTriggerErrors();
void ResetSimTriggerDefaults();
void ShowSimTriggerCounters();
void ClearSimTriggerCounters();
void SimL0overAchannelON();
void SimL0overAchannelOFF();
void SimTryTriggerSequence();
void SimDrop1stL0();
void SimUndrop1stL0();
void SimDebugFPGAfill();

void TestMemManagment();

void DumpWrittenData(int nwords);
void DumpFileEvents();

void GUI_ClearCounters();
void GUI_GetCounters(int difval);

void DBG_TestClearSSM();

/* Scope Signals 
 */
void ScopeSelect_AB(int A, int B);
void ScopeGet_AB(int how);

/*
  SelDump 
 */
void HexBinDump();
void HexBinDumpNext();
int SelDump();
int SelFetchHexBinDump();
int SelFetchDump();
int SelDumpNext();
void SelDumpOptions(int Orbit, int TTC_A, 
		    int TTC_B, int L0_LVDS, int PP, int TTCrx,
		    int EventType);
void SelPrintDumpOpt();
int SelDumpN(int howmany);
void SelDump1stEvent(int Sel);
void SelDump1stMenu(int L0, int L0_LVDS, int L1, int Orbit, int L1m,
		    int L2a, int L2r, int PP);
void SelDumpL0(int OverFibre);
void SelDumpTimeouts(int Time_s);
void SelDumpDefaults();
void SelDumpCountSSM();
int SelDumpSetEvent(int N);

/*
            Simulation only (if SIMVME driver is used)
 */

#ifdef SIMVME
w32 vmer32_sim(w32 offset);
void vmew32_sim(w32 offset, w32 value);

void initTTCit_sim(); /* Initialization of memory map for simulation */

void dump_SS(int nw);
int get_SS_top();
void sim_set_trigger_params(int Tdist, int T_L0_L1, int T_L1_L1M, 
			    int T_L1M_DATA, int T_L1_L2, int BC_L1_L2);
void sim_show_trigger_params();
void sim_set_trigger_rejects(int L1r, int L2r);
void sim_show_trigger_rejects();
void sim_set_trigger_errors(int L0S, int L1S, int L1T, int L1M, int L1Mo, 
			    int L1F, int L2T, int L2Ts,int L2F, int BCID);
void sim_show_trigger_errors();
void sim_reset_trigger();
void sim_show_counters();
void sim_L0_over_A(int mode);
void sim_drop_1st_l0(int mode);
void sim_try_trig_seq();
void sim_debug_fpga_fill();
void sim_reset_counters();
#endif

/*
  Functions in fm_fpga.c
 */

int GetStatusFPGA();                              /* X */
int GetStatusFM();                                /* X */
int ResetFM();                                    /* X */
int EraseFM();                                    /* X */
void ClearAddressFM();                            /* X */
int WriteCodeFM();                                /* X */
int LoadFPGA();


int GetStatFPGA();   /* X */
int GetStatFM();     /* X */

/*
  Functions to for the Flash Memory manipulation
 */

int writeFM(w32 address, int dato);                       /* X */
int blockWriteFM(int nbytes);                            /* X */
int readFM(w32 address, int* dato);                      /* X */
int blockReadFM(int nbytes);                             /* X */
int eraseFM();                                       /* X */
int resetFM();                                       /* X */

/*
  GUI functions
 */
void clearCounters();
void getCounters(int NCNTS, int difval);
void readCounters(w32 *mem, int NCNTS, int difval);
void initCounters();
void swapCounters();

/*
  Helpers
 */

clock_t timeInfoPerSec();

#endif /* !_TTCIT_H__ */
