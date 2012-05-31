/*
  Hacks for the SIMVME, no need to use it 
 */

#ifdef SIMVME

#include "vmewrap.h"
#include "ttcit.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

#include "ttcit_logic.h"

/*
  If pthreads are possible it we should use them
 */

/*
  Timing constants
*/

#ifdef TTCIT_SIM_USE_PTHREADS__
#define MAX_IN_SS_BC_BUFFER  100
#endif

/*
  Dummy defines, just to keep the code running. All definitions has been
  changed.

  If something is moved back to the ttcit.h, then the same constant
  must be removed from here.

  To make sure that stuff here is not accessed during simulation,
  all constants have the lovest bit set
*/
#ifdef SERIAL_NUMBER
#undef SERIAL_NUMBER
#define SERIAL_NUMBER         0xdb0001
#endif
#define VME_VERSION_ADD       0xdb0011
#define FLASH_STATUS          0xdb0021

/*  #define FLASHACCESS_INCR      0xdb0031  */
/*  #define FLASHACCESS_NOINCR    0xdb0041  */
#define CONFIG_STATUS         0xdb0051
/*  #define FLASHADD_CLEAR        0xdb0061  */
/*  #define CONFIG_START          0xdb0071  */  
#define FPGA_ACTION_STATUS    0xdb0081
#define FPGA_ACTION_LOAD      0xdb0091

enum howtoAccess {incr, noincr};

w32 sim_FM_read(enum howtoAccess how);
void sim_FM_write(enum howtoAccess how, w32 value);
int sim_CMD_comp(unsigned char *cmd, unsigned char *pattern);
int sim_Reg_read32(w32 offset);
void sim_Reg_write32(w32 offset, int dato); 
int sim_FPGA_action(int what);

#define MAX_FLASH_MEMORY 1048576

enum FlashMemState { READY, BUSY, READ, READINGCOMMAND, WRITE };

struct FlashMem {
  unsigned char mem[MAX_FLASH_MEMORY]; /* Simualates Flash memory */
  int AddressCounter; /* Shows actual position of the Address Counter */
  unsigned char command[6]; /* Command (Erase Chip, Write data etc... */
  int cmdByte; /* Which command byte is read */

  unsigned char cmdEraseChip[7];
  unsigned char cmdWrite[7];
  unsigned char cmdReset[7];

  enum FlashMemState state;
};

enum memFPGAstatusFlag {fpgaLoad, fpgaReady, fpgaIntermediate};
struct memFPGA {
  enum memFPGAstatusFlag  status;
  clock_t Timer;
  clock_t ClockPerSec;
  clock_t huMsec;
  int count;
};

static struct FlashMem FM;
static struct memFPGA fpga;

struct VMEcontroller {
  int CodeAdd;
  int SerialNumber;
  int VmeVersionAdd;

  int VersionAdd;
};

static struct VMEcontroller VME;

enum runStatusSim { SIM_RUNNING, SIM_STOPPED, SIM_STOPPING };
enum wireState { ON, OFF };
enum simThreading { NORMAL, DEBUG };
enum triggerCountState { ST_EVENT, ANY_EVENT };

struct FPGAlogicState {

  unsigned long status;      /* Status word */

  enum simThreading thread;  /* Switch for debug or no PTHREAD case */

  enum wireState L0_over_A;  /* ON  if L0 is sent over A channel 
                                OFF no L0 in A channel */

  pthread_t ssmThread;      /* SSM filling thread */
  pthread_t *th;            /* Set to null when no active thread */
  pthread_attr_t ssmThreadAttr; /* SSM thread fillin attributes */

  enum runStatusSim run;           /* SSM filling status */

  unsigned long all_triggers;      /* Number of triggers generated */

  int dt_trigger;     /* Time difference between 2 triggers in 26 ns ticks */
  int dt_l0_l1;       /* Time difference between L0 and L1 pulse */
  int dt_l1_l1m;      /* Time difference between L1 and L1m in 26ns ticks */
  int dt_l1m_data;    /* Time difference between L1M words in 26ns ticks */
  int dt_l1_l2;      /* Time difference between L1 pulse and L2a/L2r */

  int l1_reject_per_l0; /* How many L1 rejects, how many L0-L1 before one 
			   L0 - */
  
  int l2r_per_l2a;    /* How many L2r per 1 L2a */

  int bc_l1_l2;  /* Difference in BC count in BC FIFO and L2a/L2r */

  /*
    This is not an error, just simulation of SSM reading inside the 
    running train of trigger sequences

    This is supposed to simulate the real life

    1) Drop the 1-st L0
   */

  int drop_1st_l0;  /* If TRUE the first L0 is dropped */
  enum triggerCountState state_for_l0;

  /*
    Error settings are given as a number of L0 triggers before one such
    error happens

    0  means no errors of this type are generated
   */

  int err_l0s;     /* > 0, surplus L0 error, L0 arrives during active
		           L0 - L1 decision time */

  int err_l1s;     /* > 0, surplus L1 error, L1 arrives without preceding L0
		      signal */

  int err_l1t;     /* <> 0, L0 - L1 time violation, L1 arrives outside L1
		            decision interval
			    >0 L1 arrives AFTER the decision time
			    <0 L1 arrives BEFORE the decision time */

  int err_l1m;    /* > 0, L1 message error, L1M transmitted without L1 
		     signal */
  int err_l1mo;   /* > 0, L1M transmitted after programmable timeout 
		     interval */

  int err_l1f;   /* <> 0, L1M format error:
		    <0    = missing L1M words (or header) 
		    >0    = surplus L1M words */

  int err_l2t;   /* >0, L1 - L2 time violation, L2a/L2r not transmitted after
		    L1M */
  int err_l2ts;  /* >0, L2a/L2r transmitted without L1 signal */

  int err_l2f;   /* <>0, L2a format error, too many or too few words */

  int err_bcid; /* >0, BC ID of L2a/L2r different from the content of the
		   TTCRx BC counter at the arrival of L1 signal */ 


  unsigned long count_l0;
  unsigned long count_l1;
  unsigned long count_l1m;
  unsigned long count_l2a;
  unsigned long count_l2r;

  unsigned long count_seq;    /* Internal sequence counter */

  unsigned long count_err_l0s;
  unsigned long count_err_l1s;
  unsigned long count_err_l1t;
  unsigned long count_err_l1m;
  unsigned long count_err_l1mo;
  unsigned long count_err_l1f;
  unsigned long count_err_l2t;
  unsigned long count_err_l2ts;
  unsigned long count_err_l2f;
  unsigned long count_err_bcid;

  unsigned long BunchXing;   /* Nr. of the bunch crossing */
  unsigned long Orbit;        /* Nr. of the orbit */

  int flip1;  /* Internal flip-flops */
  int flip2;
  int flip3;
  int flip4;
  int flip5;
  int flip6;
};

static struct FPGAlogicState logicState;

/*
  Simulation of the TTCit logic
*/
static struct SSMbuffer SS;
static struct BCfifo bc;
void logic_INIT();
void random_fill_SS();
void dump_SS_buffer(struct SSMbuffer S, int nw);
void set_logic_mode_sim(int mode);
void trig_sequence_sim(int *ticks, unsigned long *s, 
		       unsigned long *b, int *n);
void *sim_fpga_logic_loop(void *dummy);

void sim_set_keine_error();
void sim_set_trigger_defaults();
void initTTCit_sim(){

  int i;
  pthread_attr_t *p_attr;
  int irc;

  printf("initTTCit_sim: Setting VME constants for SIMVME\n");

  FM.AddressCounter = 0;
  for(i=0; i<MAX_FLASH_MEMORY; i++){
    FM.mem[i] = 0xff;
  }
  FM.cmdByte = 0;
  FM.state = READY;

  /* ERASE command */
  FM.cmdEraseChip[6] = 6;
  FM.cmdEraseChip[0] = 0xaa;
  FM.cmdEraseChip[1] = 0x55;
  FM.cmdEraseChip[2] = 0x80;
  FM.cmdEraseChip[3] = 0xaa;
  FM.cmdEraseChip[4] = 0x55;
  FM.cmdEraseChip[5] = 0x10;

  /*WRITE command */
  FM.cmdWrite[6] = 3;
  FM.cmdWrite[0] = 0xaa;
  FM.cmdWrite[1] = 0x55;
  FM.cmdWrite[2] = 0xa0;

  /*RESET command */
  FM.cmdReset[6] = 1;
  FM.cmdReset[0] = 0xf0;

  /* VME controller */
  VME.CodeAdd = 0x5a;
  VME.SerialNumber = 0x1;
  VME.VmeVersionAdd = 0x12;

  VME.VersionAdd = 0x23;

  /*
    FPGA simulation is initialized
   */
  fpga.status = fpgaReady;
  fpga.Timer = 0;
  fpga.ClockPerSec = timeInfoPerSec();
  printf("Clocks per second on this platform = %lu\n",fpga.ClockPerSec);
  fpga.huMsec = fpga.ClockPerSec / 10;

  for(i = 0; i < MEGA; i++){
    SS.buf[i] = 0;
  }
  SS.top = 0;

  for(i = 0; i < BCNT_FIFO_SIZ; i++){
    bc.buf[i] = 0;
  }
  bc.top = 0;

  do{
    logicState.status = SSM_SET_MODE_SCOPE;
    logicState.run = SIM_STOPPED;
    logicState.th = NULL;
    p_attr = &logicState.ssmThreadAttr;

    logicState.state_for_l0 = ST_EVENT;

#ifdef TTCIT_SIM_USE_PTHREADS__

    irc = pthread_attr_init(p_attr);
    if(irc != 0){
      print_thread_error_message("pthread_attr_init\0",irc);
      break;
    }

    irc = pthread_attr_setdetachstate(p_attr,PTHREAD_CREATE_DETACHED);
    if(irc != 0){
      print_thread_error_message("pthread_att_setdetachstate\0",irc);
      break;
    }

    logicState.thread = NORMAL;

#else

    logicState.thread = DEBUG;

#endif /* TTCIT_SIM_USE_PTHREADS__ */

    /*
      Set these to rouhly corresponding values. 0's are considered a BUG!
     */
    sim_set_trigger_defaults();


    /*
      This must be set through GUI, by default errors are disabled
     */
    sim_set_keine_error();

    sim_reset_counters();

    logicState.BunchXing = 0;
    logicState.Orbit = 0;

    logicState.flip1 = TRUE;
    logicState.flip2 = TRUE;
    logicState.flip3 = TRUE;
    logicState.flip4 = TRUE;
    logicState.flip5 = TRUE;
    logicState.flip6 = TRUE;

  }while(0);

}

void sim_set_trigger_defaults(){
  logicState.l2r_per_l2a = 0;
  logicState.l1_reject_per_l0 = 0;

  logicState.dt_trigger = 10000;
  logicState.dt_l0_l1 = 224;
  logicState.dt_l1_l1m = 84;
  logicState.dt_l1m_data = 47;
  logicState.dt_l1_l2 = 3101;
  logicState.bc_l1_l2 = 94;

  logicState.L0_over_A = ON;
}

void sim_set_trigger_params(int Tdist, int T_L0_L1, int T_L1_L1M,  
			    int T_L1M_DATA, int T_L1_L2, int BC_L1_L2){
  int dist;
  int l0l1;
  int l1l1m;
  int l1mdata;
  int l1l2;
  int bc;

  int l2rl2a;
  int l1r;

  l2rl2a = logicState.l2r_per_l2a;
  l1r = logicState.l1_reject_per_l0;
  sim_set_trigger_defaults();

  dist = (Tdist > 0) ? Tdist : logicState.dt_trigger;
  l0l1 = (T_L0_L1 > 0) ? T_L0_L1 : logicState.dt_l0_l1;
  l1l1m = (T_L1_L1M > 0) ? T_L1_L1M : logicState.dt_l1_l1m;
  l1mdata = (T_L1M_DATA > 0) ? T_L1M_DATA : logicState.dt_l1m_data;
  l1l2 = (T_L1_L2 > 0) ? T_L1_L2 : logicState.dt_l1_l2;
  bc = (BC_L1_L2 > 0) ? BC_L1_L2 : logicState.bc_l1_l2;

  logicState.dt_trigger = dist;
  logicState.dt_l0_l1 = l0l1;
  logicState.dt_l1_l1m = l1l1m;
  logicState.dt_l1m_data = l1mdata;
  logicState.dt_l1_l2 = l1l2;
  logicState.bc_l1_l2 = bc;

  logicState.l2r_per_l2a = l2rl2a;
  logicState.l1_reject_per_l0 = l1r;
}

void sim_show_trigger_params(){
  printf("SIMULATED TRIGGER PARAMETERS:\n");
  printf(" \n");
  printf("  Distance in 26ns ticks: L0 - L0   : %d\n",logicState.dt_trigger);
  printf("                          L0 - L1   : %d\n",logicState.dt_l0_l1);
  printf("                          L1 - L1M  : %d\n",logicState.dt_l1_l1m);
  printf("                          L1M data  : %d\n",logicState.dt_l1m_data);
  printf("                          L1 - L2   : %d\n",logicState.dt_l1_l2);
  printf("                          BC:L1-L2a : %d\n",logicState.bc_l1_l2);
  switch(logicState.L0_over_A){
  case ON:
    printf("  L0 transmitted over A channel \n");
    break;
  case OFF:
    printf("  L0 NOT transmitted over A channel \n");
    break;
  }
  if(logicState.drop_1st_l0){
    printf("  1st L0 is always dropped \n");
  }else{
    printf("  1st L0 is always present \n");
  }
  printf(" --- \n");
}

void sim_set_trigger_rejects(int L1r, int L2r){
  if(L1r > 0){
    logicState.l1_reject_per_l0 = L1r;
  }
  if(L2r > 0){
    logicState.l2r_per_l2a = L2r;
  }
}

void sim_show_trigger_rejects(){
  printf("SIMULATED TRIGGER REJECT RATES:\n");
  printf(" \n");
  printf(" L1 missing after %d  L0\n",logicState.l1_reject_per_l0);
  printf(" L2r after        %d  L2a messages\n",logicState.l2r_per_l2a);
  printf(" ... \n");
}

void sim_reset_counters(){
  logicState.count_l0 = 0;
  logicState.count_l1 = 0;
  logicState.count_l1m = 0;
  logicState.count_l2a = 0;
  logicState.count_l2r = 0;

  logicState.count_seq = 0;

  logicState.count_err_l0s = 0;
  logicState.count_err_l1s = 0;
  logicState.count_err_l1t = 0;
  logicState.count_err_l1m = 0;
  logicState.count_err_l1mo = 0;
  logicState.count_err_l1f = 0;
  logicState.count_err_l2t = 0;
  logicState.count_err_l2ts = 0;
  logicState.count_err_l2f = 0;
  logicState.count_err_bcid = 0;

  logicState.all_triggers = 0;
}

void sim_show_counters(){
  printf("SIMULATED TRIGGER AND ERROR COUNTERS\n");
  printf(" \n");
  printf("   Triggers:   L0    %lu\n",logicState.count_l0);
  printf("               L1    %lu\n",logicState.count_l1);
  printf("               L1M   %lu\n",logicState.count_l1m);
  printf("               L2a   %lu\n",logicState.count_l2a);
  printf("               L2r   %lu\n",logicState.count_l2r);
  printf("  \n");
  printf("  Sequence:    SEQ   %lu\n",logicState.count_seq);
  printf("  \n");
  printf("   Errors:     L0S   %lu\n",logicState.count_err_l0s);
  printf("               L1S   %lu\n",logicState.count_err_l1s);
  printf("               L1T   %lu\n",logicState.count_err_l1t);
  printf("               L1M   %lu\n",logicState.count_err_l1m);
  printf("               L1Mo  %lu\n",logicState.count_err_l1mo);
  printf("               L1F   %lu\n",logicState.count_err_l1f);
  printf("               L2T   %lu\n",logicState.count_err_l2t);
  printf("               L2Ts  %lu\n",logicState.count_err_l2ts);
  printf("               L2F   %lu\n",logicState.count_err_l2f);   
  printf("               BCID  %lu\n",logicState.count_err_bcid);
  printf(" ... \n");
}

void sim_set_keine_error(){
  logicState.err_l0s = 0;
  logicState.err_l1s = 0;
  logicState.err_l1t = 0;
  logicState.err_l1m = 0;
  logicState.err_l1mo = 0;
  logicState.err_l1f = 0;
  logicState.err_l2t = 0;
  logicState.err_l2ts = 0;
  logicState.err_bcid = 0;
}

void sim_reset_trigger(){
  sim_set_trigger_defaults();
  sim_set_keine_error();
  sim_reset_counters();
}

void sim_show_trigger_errors(){
  printf("SIMULATED TRIGGER ERROR RATES - Nr of L0 signals before error\n");
  printf(" \n");
  printf("    L0S     : Surplus L0        :%d\n",logicState.err_l0s);
  printf("    L1S     : Surplus L1        :%d\n",logicState.err_l1s);
  printf("    L1T     : L0 - L1 time viol.:%d\n",logicState.err_l1t);
  printf("    L1M     : L1M without L1    :%d\n",logicState.err_l1m);
  printf("    L1Mo    : No L1M after L1   :%d\n",logicState.err_l1mo);
  printf("    L1F     : L1 format error   :%d\n",logicState.err_l1f);
  printf("    L2T     : No L2 after L1    :%d\n",logicState.err_l2t);
  printf("    L2Ts    : L2 without L1     :%d\n",logicState.err_l2ts);
  printf("    L2F     : L2a format error  :%d\n",logicState.err_l2f);
  printf("    BCID    : BC ID no match    :%d\n",logicState.err_bcid);
  printf(" ... \n");
}

void sim_set_trigger_errors(int L0S, int L1S, int L1T, int L1M, int L1Mo, 
			    int L1F, int L2T, int L2Ts, int L2F, int BCID){
  if(L0S > 0){
    logicState.err_l0s = L0S;
  }
  if(L1S > 0){
    logicState.err_l1s = L1S;
  }
  if(L1T != 0){
    logicState.err_l1t = L1T;
  }
  if(L1M != 0){
    logicState.err_l1m = L1M;
  }
  if(L1Mo > 0){
    logicState.err_l1mo = L1Mo;
  }
  if(L1F != 0){
    logicState.err_l1f = L1F;
  }
  if(L2T > 0){
    logicState.err_l2t = L2T;
  }
  if(L2Ts > 0){
    logicState.err_l2ts = L2Ts;
  }
  if(L2F != 0){
    logicState.err_l2f = L2F;
  }
  if(BCID > 0){
    logicState.err_bcid = BCID;
  }
}

void sim_L0_over_A(int mode){
  switch(mode){
  case 1:
    logicState.L0_over_A = ON;
    break;
  case 0:
    logicState.L0_over_A = OFF;
    break;
  default:
    printf("sim_L0_over_A: If you read this, there is a BUG\n");
    break;
  }
}

void sim_drop_1st_l0(int mode){
  switch(mode){
  case 0:
    logicState.drop_1st_l0 = FALSE;
    break;
  default:
    logicState.drop_1st_l0 = TRUE;
  }
}

w32 vmer32_sim(w32 offset){
  w32 retval;
  enum howtoAccess how;
  w32 bit7 = 128;
  w32 addrbits = 0x7f;

  /*
    printf("vmer32_sim is called -SIMULATION, offset = %X\n",offset);
  */

  switch(offset){
  case VERSION_NUMBER:
    retval = VME.CodeAdd;
    break;
  case SERIAL_NUMBER:
    retval = VME.SerialNumber;
    break;
  case VME_VERSION_ADD:
    retval = VME.VmeVersionAdd;
  case FLASH_STATUS:
    if(FM.state != BUSY){
      retval = bit7;
    }else{
      retval = 0;
    }
    retval = retval + ( FM.AddressCounter & addrbits);
    break;
  case FLASHACCESS_INCR:
    how = incr;
    retval = sim_FM_read(how);
    break;
  case FLASHACCESS_NOINCR:
    how = noincr;
    retval = sim_FM_read(how);
    break;
  case CONFIG_STATUS:
    retval = sim_FPGA_action(FPGA_ACTION_STATUS);
    break;
  default:
    retval = sim_Reg_read32(offset);
    break;
  }

  return retval;
}

void vmew32_sim(w32 offset, w32 value){
  enum howtoAccess how;
  int irc;
  w32 mask = 0xff;
  int i;
  enum {accessFM, accessReg, accessFPGA} accessDest;

  while(1){
    switch(offset){
    case FLASHACCESS_INCR:
      how = incr;
      irc = 0;
      accessDest = accessFM;
      break;
    case FLASHACCESS_NOINCR:
      how = noincr;
      irc = 0;
      accessDest = accessFM;
      break;
    case FLASHADD_CLEAR:
      irc = 0;
      accessDest = accessReg;
      break;
    case CONFIG_START:
      irc = 0;
      accessDest = accessFPGA;
      irc = sim_FPGA_action(FPGA_ACTION_LOAD);
      if(irc != 0){
	printf("Bug in sim_FPGA_action : You should not see this!!! \n");
	irc = 999;
      }
      break;
    case RESET:
      accessDest = accessReg;
      irc = 0;
      break;
    case TTCIT_CONTROL:
      accessDest = accessReg;
      irc = 0;
      break;
    default:
      irc = 1;
    }
    if(irc != 0){
      FM.state = BUSY;
      break;
    }
    if(accessDest == accessReg){
      sim_Reg_write32(offset, value);
      break;
    }
    if(accessDest == accessFPGA){
      break;
    }
    switch(FM.state){
    case READY:             /* READY to start reading a new command */
      FM.cmdByte = 0;
      FM.command[6] = 0;
    case READINGCOMMAND:    /* READING COMMAND, i.e. value is a command */
      FM.state = READINGCOMMAND;
      if(how == incr){
	FM.AddressCounter = (FM.AddressCounter + 1) % MAX_FLASH_MEMORY;
      }
      FM.command[FM.cmdByte++] = (value & mask); /* only 8bits relevant */
      FM.command[6] = FM.cmdByte;
      if(FM.cmdByte == FM.cmdEraseChip[6]){
	irc = sim_CMD_comp(&(FM.command[0]),&(FM.cmdEraseChip[0]));
	if(irc){
	  printf("Erase Chip command simulated\n");
	  for(i=0; i<MAX_FLASH_MEMORY; i++){
	    FM.mem[i] = 0xff;                /* Erasing puts all 1's */
	  }
	  FM.state = READY;
	  break;
	}
      }
      if(FM.cmdByte == FM.cmdWrite[6]){
	irc = sim_CMD_comp(&(FM.command[0]),&(FM.cmdWrite[0]));
	if(irc){
	 /*  printf("Write Command simulated\n"); */
	  FM.state = WRITE;
	  break;
	}
      }
      if(FM.cmdByte == FM.cmdReset[6]){
	irc = sim_CMD_comp(&(FM.command[0]),&(FM.cmdReset[0]));
	if(irc){
	  /* printf("Reset command simulated\n"); */
	  FM.state = READY;
	  break;
	}
      }
      break;
    case WRITE:             /* WRITES - value is stored into Flash memory */
      sim_FM_write(how, value);
      FM.state = READY;
      break;
    case BUSY:            
      FM.state = BUSY;
      if(value == 0xf0){  /* Reset command */
	FM.state = READY;
      }
      if(how == incr){
	FM.AddressCounter = (FM.AddressCounter + 1) % MAX_FLASH_MEMORY;
      }
    default:                /* Unexpected state, something wrong */
      FM.state = BUSY;
      break;
    }
    break;
  }
}

w32 sim_FM_read(enum howtoAccess how){
  w32 retval = 0xbadbad;

  while(1){
    if(FM.state != READY){
      FM.state = BUSY;
      break;
    }else{
      FM.state = READ;
    }
    retval = FM.mem[FM.AddressCounter];
    if(how == incr){
      FM.AddressCounter = (FM.AddressCounter + 1) % MAX_FLASH_MEMORY;
    }
    FM.state = READY;
    break;
  }
  return retval;
} 

void sim_FM_write(enum howtoAccess how, w32 value){
  w32 mask = 0xff;
  while(1){
    if(FM.state != WRITE){
      FM.state = BUSY;
      break;
    }
    FM.mem[FM.AddressCounter] = ( value & mask );
    if(how == incr){
      FM.AddressCounter = (FM.AddressCounter + 1) % MAX_FLASH_MEMORY;
    }
    FM.state = READY;
    break;
  }
}

int sim_CMD_comp(unsigned char *cmd, unsigned char *pattern){
  int irc;
  int i;
  int nr_bytes_cmd, nr_bytes_pattern;

  nr_bytes_cmd = cmd[6];
  nr_bytes_pattern = pattern[6];

  while(1){
    irc = (nr_bytes_cmd == nr_bytes_pattern);
    if( !irc ){
      break;
    }
    for(i=0; i<nr_bytes_pattern; i++){
      irc = (cmd[i] == pattern[i]);
      if( !irc ){
	break;
      }
    }
    break;
  }

  return irc;
}

int sim_Reg_read32(w32 offset){
  int retval;
#ifndef TTCIT_SIM_USE_PTHREADS__
  int i;
#endif
  /*
    printf("sim_Reg_read32, offset = %X\n",offset);
  */

  switch(offset){
  case READ_ADDR_COUNT:
    retval = SS.top;
    break;
  case READ_SNAPSHOT:
    if(SS.top <= 0){
      printf("READ_SNAPSHOT: Reding beyond the 1-st item in SSM\n");
      retval = 0xdeaddead;
    }else{
      retval = SS.buf[--SS.top];
    }
    break;
  case N_BCNT_TTCRX:

#ifdef TTCIT_SIM_USE_PTHREADS__

    retval = bc.top;     /* This should be updated by logic thread */

#else
    if(bc.top <= 0){
      printf("Regenerating BC fifo\n");
      retval = 1536;
      bc.top = retval;
      for(i = 0; i < retval; i++){
	if(i == 17){
	  /*	  printf("Setting marker\n"); */
	  bc.buf[i] = 0xadadead;
	}else{
	  bc.buf[i] = i;
	}
      }
    }else{
      retval = bc.top;
    }

#endif /* TTCIT_SIM_USE_PTHREADS__ */

    break;
  case BCNT_TTCRX:
    switch(bc.top){
    case 0:
      printf("BC fifo empty\n");
      retval = 0;
      break;
    default:
      retval = bc.buf[--bc.top];
      break;
    }
    break;
  default:
    retval = 0xbadbad;
  }

  return retval;
}

void sim_Reg_write32(w32 offset, int dato){

  switch(offset){
  case FLASHADD_CLEAR:
    FM.AddressCounter = 0;
    break;
  case RESET:
    logic_INIT();
    break;
  case TTCIT_CONTROL:
    set_logic_mode_sim(dato);
    break;
  default:
    printf("sim_Reg_write32(a: %i, d %x ): Cannot handle\n",offset,dato);
    break;
  }
}

int sim_FPGA_action(int what){
  int irc = 0;
  clock_t now;
  clock_t elapsed;
  clock_t busytime = 3;  /* Loading time in 100 * msec */
  int count2worry = 27; /* Nr. of counts before we start to worry */

  switch(what){
  case FPGA_ACTION_STATUS:
    switch(fpga.status){
    case fpgaIntermediate:
    case fpgaLoad:
      now = clock();
      elapsed = (now - fpga.Timer) / fpga.huMsec;
      fpga.count++;
      if(elapsed >= busytime){
	irc = READYFP;
	fpga.status = fpgaReady;
      }else{
	if((elapsed == 0) && (fpga.count > count2worry)){ /* Clock ??? */
	  irc = READYFP;
	  fpga.status = fpgaReady;
	  fpga.count = 0;
	}else{
	  irc = BUSYFP;
	}
      }
      break;
    case fpgaReady:
      irc = READYFP;
      break;
    default:
      irc = BUSYFP;
    }
    break;
  case FPGA_ACTION_LOAD:
    fpga.status = fpgaLoad;
    fpga.Timer = clock();
    fpga.count = 0;
    break;
  default:
    printf("Unknown action requested from simulated FPGA, nothing done\n");
    irc = 10;
  }

  return irc;
}

void logic_INIT(){
  int i;
  int lupmax;

#ifdef TTCIT_SIM_USE_PTHREADS__

  unsigned long msec26 = 26000;
  unsigned long maxSleep = 1000000;
  unsigned long totSleep;
  float seconds;

#endif

  /*
    For time being let us fill the SS here with random values
  */

#ifdef TTCIT_SIM_USE_PTHREADS__

  do{

    logicState.state_for_l0 = ST_EVENT;
    /*
      If Logic sim is running, stop it first, wait for successfull 
      completion 
    */

    if(logicState.run != SIM_STOPPED){   /* Not running, no need to stop */
      logicState.run = SIM_STOPPING;
    }

    totSleep = 0;
    seconds = 0.;
    do{
      if(logicState.run == SIM_STOPPED){  /* Stopped, no need to loop */
	break;
      }
      usleep(msec26);
      totSleep += msec26;
      if(totSleep >= maxSleep){
	seconds += ((float)totSleep / 1000000. );
	printf("Slept %f seconds, waiting\n",seconds);
      }
    }while(logicState.run != SIM_STOPPED);

    /*
      SSM simulation stopped, we may clear the SSM and FIFO buffers
    */
    SS.top = 0;
    bc.top = 0;
    lupmax = (MEGA > BCNT_FIFO_SIZ) ? MEGA : BCNT_FIFO_SIZ;
    for(i = 0; i < lupmax; i++){
      if(i < MEGA){
	SS.buf[i] = 0;
      }
      if(i < BCNT_FIFO_SIZ){
	bc.buf[i] = 0;
      }
    }

    if(logicState.dt_trigger <= 0){
      printf("You must set-up the trigger timing constants\n");
      printf("It seems that you put there some funky numbers \n");
      printf("Or the simulation code has not been properly written\n");
      printf("NO ACTION TAKEN - TTCIT LOGIC INIT FAILED\n");
      break;
    }

    /*
      Now we can start SSM / BC FIFO simulation
    */

    if(logicState.thread == NORMAL){

      i = pthread_create(&logicState.ssmThread,
			 &logicState.ssmThreadAttr, 
			 sim_fpga_logic_loop, NULL);
      if(i != 0){
	printf("Cannot start SSM/FIFO simulation thread IRC = %d\n",i);
	break;
      }

    }else{
      sim_fpga_logic_loop(NULL);
    }
 
  }while(0);

#else

  SS.top = 0;
  for(i = 0; i < MEGA; i++){
    SS.buf[i] = 0;
  }

  random_fill_SS();

#endif
}

/*
  One fill of the SSM and BC FIFO with timed sleeps. Trigger sequences should
  have the proper structure. Timing simulates filling times.

  Routine returns when SSM is full (if BC FIFO is full, its filling is stopped
  but SSM is filled till the end.
 */
void *sim_fpga_logic_loop(void *dummy){

  int terminate = 0;   /* If == 1 then the main loop terminates */
  int nTrigger = 0;    /* Trigger counter */

  int tick_26ns[MAX_IN_SS_BC_BUFFER];
  unsigned long ssm_data[MAX_IN_SS_BC_BUFFER];
  unsigned long bc_data[MAX_IN_SS_BC_BUFFER];
  unsigned long swapper[TTCIT_MAX_ADDR_BCFIFO];
  int nItems[2];

  int i, j, k;
  unsigned long last_time = 0;
  unsigned long time_2_sleep;
  unsigned long div_26ns = 60;

  int SS_mode;
  int first_entry;
  int dif;
  int wanna_sleep;

  logicState.run = SIM_RUNNING;

  logicState.count_seq++;   /* Mark another sequence */

  /*
    Repeat the basic sequence, Fill SSM and BC FIFO until some. Each is filled
    untill its counter is at max.
   */

  terminate = FALSE;
  first_entry = TRUE;

  do{
    /*
      Generate a trigger sequence according to internal settings
    */
    trig_sequence_sim(&tick_26ns[0], &ssm_data[0], &bc_data[0], &nItems[0]);

    if(logicState.run != SIM_RUNNING){
      terminate = 1;
      break;
    }else{
      nTrigger++;
    }
    
    /*
      Still running, time it and store in the BC and SSM

      Watch the logicState.status for SSM mode SCOPE/SEQ
     */
    j = 0;
    for(i = 0; i <= nItems[0]; i++){
      
      if(first_entry){  /* If SS is empty, skip the 1st L0 */
	last_time = 0;
	first_entry = FALSE;
      }else{
	last_time += tick_26ns[i];
	time_2_sleep = (tick_26ns[i]>0) ? ( tick_26ns[i] / div_26ns ) + 1 : 1;
	/*
	  Sleep before filling. Sleep must be at least 1 us long, 
	  it is longer than the real thing, but we are not pressed by time
	 */

	/* 
	   We must somehow speed up the simulation in SEQ mode, so we should
	   sleep once for every 1000 triggers
	*/
	SS_mode = get_SS_mode();

	if(SS_mode == SSM_SCOPE){
	  wanna_sleep = TRUE;
	}else{
	  /*	  wanna_sleep = ((nTrigger % 100000) == 0);  */
	  wanna_sleep = FALSE;
	}
	if(wanna_sleep){
	  usleep(time_2_sleep);
	}

	switch(SS_mode){
	case SSM_SCOPE:
	  /*
	    We fill all the empty places of the SSM while watching
	    its size
	   */
	  dif = (i > 0) ? tick_26ns[i] - tick_26ns[i-1] : 0;
	  for(k = 1; k < dif; k++){ /* tick_26ns[i] - 1 0's */
	    SS.buf[SS.top++] = 0;
	    terminate = (SS.top >= TTCIT_MAX_ADDR_SSM);
	    if(terminate){
	      break;
	    }
	  }
	  break;
	case SSM_SEQ:
	  /*
	    No empty spaces need filling 
	   */
	  terminate = (SS.top >= TTCIT_MAX_ADDR_SSM); 
	  break;
	default:
	  printf("sim_fpga_logic_loop: I should not be there, BUG!\n");
	  terminate = TRUE;
	  break;
	}
	
	/* */

	if(terminate){
	  break;
	}
	SS.buf[SS.top++] = ssm_data[i];
	terminate = (SS.top >= TTCIT_MAX_ADDR_SSM);
	if(terminate){
	  break;
	}
      }
      /*
	If A channel fires, store dato BC FIFO if available, or 0 if not

	Do not fill if FIFO is already full
       */
      if(bc.top < TTCIT_MAX_ADDR_BCFIFO){
	if(((ssm_data[i] & MASK_L1ACCEPT) != 0) && (j <= nItems[1])){
	  bc.buf[bc.top++] = bc_data[j++];
	}
      }
      terminate = (SS.top >= TTCIT_MAX_ADDR_SSM);
      if(terminate){
	break;
      }
    }
    terminate = (SS.top >= TTCIT_MAX_ADDR_SSM);
    if(terminate){
      break;
    }

    /*
      If there is still free place in SSM start a new trigger
     */

    last_time += logicState.dt_trigger;
    time_2_sleep = (logicState.dt_trigger / div_26ns) + 1;
    usleep(time_2_sleep);
    SS_mode = get_SS_mode();
    switch(SS_mode){
    case SSM_SCOPE:
      dif = logicState.dt_trigger;
      for(k = 0; k < dif; k++){
	SS.buf[SS.top++] = 0;
	terminate = (SS.top >= TTCIT_MAX_ADDR_SSM);
	if(terminate){
	  break;
	}
      }
      break;
    case SSM_SEQ:
      terminate = (SS.top >= TTCIT_MAX_ADDR_SSM);
      break;
    default:
      printf("I should not be there:\n");
      terminate = TRUE;
      break;
    }

  }while((!terminate) && (logicState.run == SIM_RUNNING));

  /*
    BC FIFO is organized as FIFO, i.e. first in - first out, but
    the FIRST IN/OUT is the one with the biggest index!!!

    We must swap all items in bc.buf
   */
  j = bc.top;
  for(i=0; i<bc.top; i++){
    swapper[--j] = bc.buf[i];
  }
  for(i=0; i<bc.top; i++){
    bc.buf[i] = swapper[i];
  }

  logicState.run = SIM_STOPPED;   /* Work finished, we should stop */

  return NULL;
}

/*
  Simulates one trigger sequence with timing (sleeps) and fills SSM and
  FIFO storage.

  Timing information is written but the real time timing is left to the 
  caller

  ticks[]    = tick values for which SSM values are stored (can be ignored
               if SEQ mode of SSM is used

  s[]        = data for SSM, each happend at corresponding ticks[i] time
  b[]        = BC fifo values, not times

  n[0]       = number of data in s[] buffer -1 (Max index filled in s[])
  n[1]       = number of data in b[] buffer -1 (Max index filled in b[])

  1 tick at position -1 (the first L0 is assumed): It should not be stored
  for the 1-st sequence, byt must be present for all others.

  BC FIFO stores BC count any time a Channel A is in state 1, i.e. 
  1 value is stored when L0 arrives (the 1-st L0 value is stored), and
  2 consecutive values are stored when L1 signal arrives. 

  L2a , L2r cotain BC count shifted by some value, that should be fixed.
*/
void trig_sequence_sim(int *ticks, unsigned long *s,
		       unsigned long *b, int *n){

  int item = -1;
  int bc_item = -1;
  int do_error;
  unsigned long word;
  int time_last = 0;
  int diff;
  int terminate = 0;

  unsigned long x;
  unsigned long dato[25], t[25];
  int i;
  unsigned long strobe = MASK_DOUTSTR;

  unsigned long last_bc; 
  int add_remove, ar_nwords, header_affected;
  int l2_reject;
  int n_w, i1;

  do{

    if((ticks == NULL) || (s == NULL) || (b == NULL) || (n == NULL)){
      printf("trig_sequence_sim: Some arguments are NULL\n");
      break;
    }

    n[0] = 0;    /* We have no data in buffers */
    n[1] = 0;

    logicState.all_triggers++;

    logicState.BunchXing += (unsigned long)rand();
    if(logicState.BunchXing > TTCIT_MAX_BUNCH_XING){
      logicState.Orbit++;
    }
    logicState.BunchXing %= TTCIT_MAX_PHYS_BC;
    logicState.BunchXing++;

    x = logicState.BunchXing;

    last_bc = logicState.BunchXing;

    terminate = FALSE;

    /*  Generate trigger sequence */

    /*
      L0 - may be missing is L1S errror is triggered, 
           the rest of sequence follows 

      L0 - must not be present if L0 is not sent over A channel
     */
    do{
      do_error = (logicState.err_l1s != 0);
      if(do_error){
	do_error = ((logicState.all_triggers % logicState.err_l1s) == 0);
      }
      if(do_error){    /* If L1S error, no L0 is sent */
	logicState.count_err_l1s++;
	break;
      }
      if(logicState.drop_1st_l0){  /* Drop 1-st L0 */
	if(logicState.state_for_l0 == ST_EVENT){
	  logicState.state_for_l0 = ANY_EVENT;
	  break;
	}
      }
      /*
	If L0 is not sent over A channel, L0 is not sent and BC FIFO is
	not filled
       */
      if(logicState.L0_over_A == ON){
	word = MASK_L1ACCEPT;    /* 1 tick in A channel, */
	time_last += -1;
	ticks[++item] = time_last;
	s[item] = word;
	logicState.count_l0++;
	if(logicState.L0_over_A == ON){  /* BC FIFO stores only when L0 sent */
	  b[++bc_item] = logicState.BunchXing;
	  last_bc = x;
	}
      }
    }while(0);

    /*
      If L0S is to happen, generate one surplus L0 inside L0 - L1 decision
      interval - for simplicity half of the L0 - L1 distance
     */
    do{
      /*
	If L0 is not sent over A channel, this error just cannot happen
       */
      if(logicState.L0_over_A == OFF){
	break;
      }
      do_error = (logicState.err_l0s == 0);
      if(do_error){
	break;
      }
      do_error = ((logicState.all_triggers % logicState.err_l0s) != 0);
      if(do_error){
	break;
      }
      word = MASK_L1ACCEPT;
      diff = (logicState.dt_l0_l1 / 2); 
      ticks[++item] = time_last + diff; /* Don't mess with L0 - L1 time */
      logicState.count_l0++;
      logicState.count_err_l0s++;
      if(logicState.L0_over_A == ON){ /* If L0 sent over A, store BC */
	logicState.BunchXing += diff;
	if(logicState.BunchXing > TTCIT_MAX_BUNCH_XING){
	  logicState.Orbit++;
	}
	logicState.BunchXing %= (TTCIT_MAX_BUNCH_XING + 1);
	b[++bc_item] = logicState.BunchXing;
	last_bc = logicState.BunchXing;
      }
    }while(0);

    /*
      L1 : may be missing if:  a) L1 reject (it stops the sequence)
                               b) L1M, L1 message arrives without L1
     */
    do{
      /*
	Check for L1 rejects, if event is rejected at L1 level, 
	no L1, L1M, L2a/L2r are present and nothing else is stored into
	BC FIFO, the sequence generation is finished.
       */
      do_error = (logicState.l1_reject_per_l0 != 0);
      if(do_error){
	do_error = ((logicState.count_l0 % logicState.l1_reject_per_l0) == 0);
	if(do_error){
	  terminate = TRUE;
	  break;
	}
      }
      /*
	L1M error, i.e. no L1 is sent, but the sequence goes on
       */
      do_error = (logicState.err_l1m != 0);
      if(do_error){
	do_error = ((logicState.all_triggers % logicState.err_l1m) == 0);
	if(do_error){
	  logicState.count_err_l1m++;
	  break;
	}
      }
      /*
	Generate L1 signal
       */
      word = MASK_L1ACCEPT;
      diff = logicState.dt_l0_l1;
      /*
	L1T simulation: L1 outside the L0-L1 time decision window
       */
      do_error = (logicState.err_l1t != 0);
      if(do_error){
	do_error = ((logicState.all_triggers % logicState.err_l1t) == 0);
	if(do_error){
	  if((logicState.all_triggers % 4) == 0){
	    diff = 2 * diff;
	  }else{
	    diff = diff / 2;
	  }
	}
      }
      time_last += diff;
      ticks[++item] = time_last++;  /* L1 lasts 2 ticks */
      s[item] = word;
      ticks[++item] = time_last;
      s[item] = word;
      logicState.BunchXing += diff;
      if(logicState.BunchXing > TTCIT_MAX_BUNCH_XING){
	logicState.Orbit++;
      }
      logicState.BunchXing %= (TTCIT_MAX_BUNCH_XING + 1);
      b[++bc_item] = logicState.BunchXing;;   /* BC FIFO always gets 2 words */
      x = logicState.BunchXing + 1;
      x %= (TTCIT_MAX_BUNCH_XING + 1);
      b[++bc_item] = x;
      last_bc = logicState.BunchXing;;  /* we want to mark the 1-st one */
      logicState.count_l1++;
    }while(0);
    /*
      After L1 reject the sequence is finished
     */
    if(terminate){
      break;
    }

    /*
      Generate L1M:   DoutStr is 1 tick long, Data stay for 2 ticks

      Errors: L1Mo  L1M sent after programmable time interval
              L1F   missing header or words, surplus header or words
     */
    do{
      /*
	For time being all L1M words and Header will contain 0's

	if needed, change it to something reasonable
       */
      dato[0] = 0;
      dato[1] = 0;
      dato[2] = 0;
      dato[3] = 0;
      dato[4] = 0;
      /*
	L1M header
       */
      dato[0] = dato[0] | TTC_ADDR_L1_H;
      dato[1] = dato[1] | TTC_ADDR_L1_D;
      dato[2] = dato[2] | TTC_ADDR_L1_D;
      dato[3] = dato[3] | TTC_ADDR_L1_D;
      dato[4] = dato[4] | TTC_ADDR_L1_D;
      /*
	L1M time after L1
       */
      diff = logicState.dt_l1_l1m;
      do_error = (logicState.err_l1mo != 0);
      if(do_error){
	do_error = ((logicState.all_triggers % logicState.err_l1mo) == 0);
	if(do_error){
	  diff *= 2;
	  logicState.count_err_l1mo++;
	}
      }
      t[0] = diff;
      t[1] = logicState.dt_l1m_data;
      t[2] = logicState.dt_l1m_data;
      t[3] = logicState.dt_l1m_data;
      t[4] = logicState.dt_l1m_data;
      n_w = 5;
      i1 = 0;
      /*
	Produce the L1M sequence,

	L1F:     More words if : logicState.count_l0 % 2 == 0
                 Less words if : logicState.count_l0 % 2 == 1

               +/- words = logicState.count_l0 % 3
	       add/remove L1M header if logisState.count_l0 % 7 > 4
       */
      do_error = (logicState.err_l1f != 0);
      if(do_error){
	do_error = ((logicState.all_triggers % logicState.err_l1f) == 0);
	if(do_error){
	  add_remove = logicState.flip1;
	  logicState.flip1 = (!logicState.flip1);
	  ar_nwords = (logicState.count_l0 % 2) + 1;
	  if(add_remove){  /* Add */
	    for(i=1; i<=ar_nwords; i++){
	      dato[4+i] = dato[4];
	      t[4+i] = t[4];
	    }
	    n_w += ar_nwords;
	  }else{  /* Remove */
	    header_affected = logicState.flip3;
	    logicState.flip3 = (!logicState.flip3);
	    if(header_affected){
	      i1 = ar_nwords;
	    }else{
	      if(logicState.flip5){
		n_w -= ar_nwords;
	      }else{
		n_w = 1;
	      }
	      logicState.flip5 = (!logicState.flip5);
	    }
	  }
	  t[i1] = diff;
	  logicState.count_err_l1f++;
	}
      }
      /*
	Fill the L1M sequence (good or bad)
       */
      for(i=i1; i<n_w; i++){
	time_last += t[i];
	ticks[++item] = time_last++;    /* 1 tick = strobe + data */
	s[item] = strobe | dato[i];
	ticks[++item] = time_last;      /* 1 tick = data only */
	s[item] = dato[i];
      }
      logicState.count_l1m++;
    }while(0);

    /*
      After L0 - L1 - L1M must be L2r or L2a. They are quite similar

      Errors: L2T     L2a/L2r are not sent, but L1 was
              L2TS    L2a/L2r sent, but no L1 (this is simulated by L1M)
              L2F     L2a/L2r format errors, too many (L2a, L2r)
	                                     or too few (L2a)

     */
    do{
      terminate = FALSE;
      /*
	L2T : Missing L2a/L2r
       */
      do_error = (logicState.err_l2t != 0);
      if(do_error){
	do_error = ((logicState.all_triggers % logicState.err_l2t) == 0);
	if(do_error){
	  logicState.count_err_l2t++;
	  terminate = TRUE;
	  break;
	}
      }
      /*
	L2a/L2r are to be sent, we must decide which one
       */
      l2_reject = (logicState.l2r_per_l2a != 0);
      if(l2_reject){
	l2_reject = ((logicState.all_triggers % logicState.l2r_per_l2a) == 0);
      }
      /*
	Header in L2a and L2r is basically the same
       */
      t[0] = logicState.dt_l1_l2;

      if(logicState.bc_l1_l2 == 0){
	dato[0] = last_bc;
      }else{
	/*
	  a - b mod N = a + N - b mod N
	 */
	dato[0] = (last_bc + TTCIT_MAX_BUNCH_XING + 1 - logicState.bc_l1_l2) %
	  (TTCIT_MAX_BUNCH_XING + 1);
      }

      dato[0] &= TTC_DATA_MASK;
      if(l2_reject){
	dato[0] |= TTC_ADDR_L2R;
	i1 = 0;
	n_w = 1;
      }else{
	dato[0] |= TTC_ADDR_L2A_H;
	/*
	  Add the rest of the L2A message (7 words)
	 */
	dato[1] = TTC_ADDR_L2A_D | 
	  ((logicState.Orbit >> TTC_ORBID_RSH) & TTC_DATA_MASK);
	dato[2] = TTC_ADDR_L2A_D |
	  (logicState.Orbit & TTC_DATA_MASK);
	dato[3] = TTC_ADDR_L2A_D | 0;
	dato[4] = TTC_ADDR_L2A_D | 0;
	dato[5] = TTC_ADDR_L2A_D | 0;
	dato[6] = TTC_ADDR_L2A_D | 0;
	dato[7] = TTC_ADDR_L2A_D | 0;

	t[1] = logicState.dt_l1m_data;
	t[2] = t[1];
	t[3] = t[1];
	t[4] = t[1];
	t[5] = t[1];
	t[6] = t[1];
	t[7] = t[1];

	i1 = 0;
	n_w = 8;
      }
      /*
	Simulate L2F error: Too few or too many words in message:

        More words if : logicState.count_l0 % 2 == 0
                        Less words if : logicState.count_l0 % 2 == 1
			( less words applicable only for L2a)

                        +/- words = logicState.count_l0 % 6
	                add/remove L2a header if logisState.count_l0 % 7 > 4

       */
      do_error = (logicState.err_l2f != 0);
      if(do_error){
	do_error = ((logicState.all_triggers % logicState.err_l2f) == 0);
	if(do_error){
	  if(l2_reject){
	    add_remove = TRUE;
	    header_affected = TRUE;
	  }else{
	    add_remove = logicState.flip2;
	    logicState.flip2 = (!logicState.flip2);
	  }
	  ar_nwords = (logicState.count_l0 % 5) + 1;
	  if(add_remove){
	    for(i=1; i<=ar_nwords; i++){
	      dato[7 + i] = dato[7];
	      t[7 + i] = logicState.dt_l1m_data;
	    }
	    n_w += ar_nwords;
	    i1 = 0;
	  }else{
	    header_affected = logicState.flip4;
	    logicState.flip4 = (!logicState.flip4);
	    if(header_affected){
	      i1 = ar_nwords;
	    }else{
	      if(logicState.flip6){
		n_w -= ar_nwords;
	      }else{
		n_w = 1;
	      }
	      logicState.flip6 = (!logicState.flip6);
	    }
	  }
	  t[i1] = logicState.dt_l1_l1m;
	  logicState.count_err_l2f++;
	}
      }
      /*
	Fill the L2a/L2r sequence
       */
      for(i=i1; i<n_w; i++){
	time_last += t[i];
	ticks[++item] = time_last++;
	s[item] = strobe | dato[i];
	ticks[++item] = time_last;
	s[item] = dato[i];
      }
      if(l2_reject){
	logicState.count_l2r++;
      }else{
	logicState.count_l2a++;
      }
    }while(0);
    
    /*
      Here we have the whole L0
                             L0 - L1 - L1M - L2a/L2r   sequence

      or the sequence with predictable errors
     */

  }while(0);

  n[0] = item;           /* Fill counters with correct values */
  n[1] = bc_item;

  logicState.BunchXing += time_last;
  if(logicState.BunchXing > TTCIT_MAX_BUNCH_XING){
    logicState.Orbit++;
  }
  logicState.BunchXing %= (TTCIT_MAX_BUNCH_XING + 1);

  /*
    If L0 has not been sent over channel A we must shift ticks to
    have the 1-st L1 tick at -1:
   */
  if(ticks[0] >= 0){
    x = ticks[0] + 1;
    for(i=0; i<= n[0]; i++){
      ticks[i] -= x;
    }
  }
}

void sim_try_trig_seq(){
  int tick_26ns[MAX_IN_SS_BC_BUFFER];
  unsigned long ssm_data[MAX_IN_SS_BC_BUFFER];
  unsigned long bc_data[MAX_IN_SS_BC_BUFFER];
  int n[2];

  int i;

  int tick, A, Strobe, addr, data;

  logicState.count_seq++;
  trig_sequence_sim(&tick_26ns[0], &ssm_data[0], &bc_data[0], &n[0]);

  printf("SIMULATED TRIGGER SEQUENCE TEST\n");
  printf("-------------------------------\n");
  printf(" \n");
  printf("Tick         A       Data      TTC       TTC\n");
  printf("          Channel   Strobe    Address   data\n");

  for(i=0; i<=n[0]; i++){
    tick = tick_26ns[i];
    A = ((ssm_data[i] & MASK_L1ACCEPT) >> RSH_L1ACCEPT);
    Strobe = ((ssm_data[i] & MASK_DOUTSTR) >> RSH_DOUTSTR);
    addr = ((ssm_data[i] & TTC_ADDR_MASK) >> TTC_ADDR_RSH);
    data = (ssm_data[i] & TTC_DATA_MASK);
    printf("%07d       %1X         %1X        %1X     %3X\n",
	   tick,A,Strobe,addr,data);
  }
  printf(" ---------- \n");
  printf(" \n");
  printf("BC FIFO DUMP:\n");
  for(i=0; i<=n[1]; i++){
    printf("%07d      %lX\n",i,bc_data[i]);
  }
  printf(" \n");
  printf(" ----- END OF SIMULATED TRIGGER SEQUENCE TEST\n");
}

void random_fill_SS(){

  int L0time = 2;
  int L1time = 1;
  int DoutTime = 5;

  int DoutData = 0xad;
  int SubadrData = 0xde;

  int nseq = 5;
  int L0_L1 = 100;
  int L0_Doutstr_1 = 150;
  int L0_Doutstr_2 = 1000;
  int last_L0 = 500;

  int nData_1 = 3;
  int nData_2 = 4;

  int d1, d2;

  int iseq;
  int timer;
  unsigned long word;
  int t;
  int dato;

  SS.top = 0;
  for(iseq = 0; iseq < nseq; iseq++){   /* Loop over all requested sequences */

    /*
      Generate one sequence : L0 - L1 - Doutstr - Doutstr
                                        Data_1    Data_2
     */

    word = MASK_L1ACCEPT;             /* L0 */
    for(t = 0; t < L0time; t++){
      SS.buf[SS.top++] = word;
    }
    word = 0;                         /* L0 - L1 gap */
    for(t = 0; t < L0_L1; t++){
      SS.buf[SS.top++] = word;
    }
    word = MASK_L1ACCEPT;           /* L1 */
    for(t = 0; t < L1time; t++){
      SS.buf[SS.top++] = word;
    }
    word = 0;                        /* L1 Doutstr gap */
    timer = L0_Doutstr_1 - L0_L1;
    for(t = 0; t < timer; t++){
      SS.buf[SS.top++] = word;
    }
    /*
      1-st data sequence: Doutstr is 1 when data are active
     */
    for(dato = 0; dato < nData_1; dato++){
      d1 = DoutData & MASK_DOUT;
      d2 = (SubadrData << RSH_SUBADR) & MASK_SUBADR;
      word = MASK_DOUTSTR | d1 | d2;
      for(t = 0; t < DoutTime; t++){
	SS.buf[SS.top++] = word;
      }
      word = 0;
      for(t = 0; t < DoutTime; t++){
	SS.buf[SS.top++] = word;
      }
    }
    word = 0;
    timer = L0_Doutstr_2 - L0_Doutstr_1;
    for(t = 0; t < timer; t++){
      SS.buf[SS.top++] = word;
    }
    /*
      2-nd data sequence: Doutstr is 1 when data are active
     */
    for(dato = 0; dato < nData_2; dato++){
      d1 = DoutData & MASK_DOUT;
      d2 = (SubadrData << RSH_SUBADR) & MASK_SUBADR;
      word = MASK_DOUTSTR | d1 | d2;
      for(t = 0; t < DoutTime; t++){
	SS.buf[SS.top++] = word;
      }
    }
    word = 0;
    for(t = 0; t < DoutTime; t++){
      SS.buf[SS.top++] = word;
    }
    /*
      Insert a gap between the seqence and the next one
     */
    word = 0;
    for(t = 0; t < last_L0; t++){
      SS.buf[SS.top++] = word;
    }
  }
}

void dump_SS(int nw){

  printf("Simulated Snap Shot memory\n");
  dump_SS_buffer(SS, nw);
}

int get_SS_top(){
  return SS.top;
}

void set_logic_mode_sim(int mode){
  switch(mode){
  case SSM_SET_MODE_SCOPE:
    logicState.status |= mode;
    break;
  case SSM_SET_MODE_SEQ:
    logicState.status |= mode;
    break;
  default:
    printf("No mode associated with %X\n",mode);
    break;
  }
}

void sim_debug_fpga_fill(){
  enum simThreading th;

  th = logicState.thread;
  logicState.thread = DEBUG;
  logic_INIT();
  logicState.thread = th;
}

#endif
