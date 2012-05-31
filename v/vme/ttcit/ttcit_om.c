/*
  On-Board Monitor (OM) cotrol
 */

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#include "vmewrap.h"
#include "ttcit_conf.h"
#include "ttcit.h"
#include "ttcit_om.h"
#include "ttcit_logic.h"
#include "ttcit_io.h"
#include "ssm_analyz.h"

/*
  OM internal data storage
 */

static struct OM_internal_state OM_guts;
extern struct SSMbuffer SSM;
extern struct BCfifo bcnt;

extern struct ttc_it_event_errors ssm_analyz_errors;

extern struct ttc_it_counters counters;
extern struct ttc_it_errors errores;
extern struct ttc_it_times timing;

extern struct ttc_it_counters sum_count;
extern struct ttc_it_errors sum_err;
extern struct ttc_it_times sum_time;

extern struct logic_StatusInternal StatusInternal;

/*
  FUNCTIONS
 */

void OM_init(){
  int t;

  OM_guts.error_mask = 0x0;
  OM_guts.SSM_mode = OM_SCOPE;
  OM_guts.status = OM_INACTIVE;
  OM_guts.any_error_mask = 0xffffc;  /* Any error bit  sasns 0 and 1 */
  OM_guts.command_mask = 0x3;

  /*
    Setting some default values into the HW registers
   */
  OM_set_time_l0_l1(TTCIT_OM_L0_L1_TIME);
  t = VMER32(HW_TIME_L0_L1);
  if(OM_guts.prt == OM_YES_PRINT){
    printf("OM : L0 - L1 time interval is set to %d\n",t);
    printf("     If needed use OM_SetTime_L0_L1(t) to change it\n");
  }

  OM_SetDelayedSSMstop(0);

  VMEW32(HW_BCNT_DIFFERENCE,0x0);
  t = VMER32(HW_BCNT_DIFFERENCE);
  if(OM_guts.prt == OM_YES_PRINT){
    printf("OM : Default BCNT differene set to %d\n",t);
    printf("     If needed change the value of HW_BCNT_DIFFERENCE register\n");
  }
  /*
    Set the Scope selection A & B to 0x0 i.e. no selection 
   */
  ScopeSelect_AB(100,100);

  if(OM_guts.prt == OM_YES_PRINT){
    printf("On-Board Monitor initialization ........ OK\n");
  }
}

void OM_set(enum ttcit_OM_setopt action){

  int todo = 0x0;
  int cmd;
  /*
    Take the current error mask
   */
  OM_guts.error_mask = OM_get_error_mask(&cmd);

  switch(action){
  case OM__ON_START:
    todo = TTCIT_CTRL_CONTIWRIT | OM_guts.error_mask;
    break;
  case OM__ON_NOSTART:
    todo = TTCIT_CTRL_STOPCWRIT | TTCIT_CTRL_CONTIWRIT | OM_guts.error_mask;
    break;
  case OM__OFF:
    todo = OM_guts.error_mask;
    break;
  case OM__STOP:
    todo = TTCIT_CTRL_STOPCWRIT | cmd | OM_guts.error_mask;
    break;
  case OM__START:
    todo = ~TTCIT_CTRL_STOPCWRIT & ( cmd | OM_guts.error_mask);
    break;
  default:
    todo = 0x0;
    printf("OM_set: Impossible state, resetting OM to idle\n");
  }

  OM_set_control(todo);
  OM_get_status();
}

int OM_get_control(){
  int irc = 0x0;
  irc = VMER32(TTCIT_CONTROL);
  return irc;
}

int OM_get_status(){
  int irc = 0x0;
  irc = VMER32(TTCIT_STATUS);
  return irc;
}

void OM_set_control(int sword){
  VMEW32(TTCIT_CONTROL, sword);
}

int OM_get_error_mask(int *command){
  int irc = 0x0;
  int stt = 0x0;

  stt = OM_get_control();
  irc = stt & OM_guts.any_error_mask;
  *command = stt & OM_guts.command_mask;
  return irc;
}

void OM_print_error_mask(int errmask){
  int ct = 0x0;

  ct = errmask;

  printf("On-Board Monitor Error Mask:\n");
  printf("---------------------------\n");
  OM_PRINT_WHAT_ERROR(ct,TTCIT_CTRL_STOP_PP,  "Pre-Pulz error    ");
  OM_PRINT_WHAT_ERROR(ct,TTCIT_CTRL_STOP_L0S, "L0 spurious       ");
  OM_PRINT_WHAT_ERROR(ct,TTCIT_CTRL_STOP_L1S, "L1 spurious       ");
  OM_PRINT_WHAT_ERROR(ct,TTCIT_CTRL_STOP_L1MM,"L1m missing       ");
  OM_PRINT_WHAT_ERROR(ct,TTCIT_CTRL_STOP_L1MS,"L1m spurious      ");
  OM_PRINT_WHAT_ERROR(ct,TTCIT_CTRL_STOP_L1MI,"L1m incomplete    ");
  OM_PRINT_WHAT_ERROR(ct,TTCIT_CTRL_STOP_L1MD,"L1m data error    ");
  OM_PRINT_WHAT_ERROR(ct,TTCIT_CTRL_STOP_L2MM,"L2m missing       ");
  OM_PRINT_WHAT_ERROR(ct,TTCIT_CTRL_STOP_L2MS,"L2m spurious      ");
  OM_PRINT_WHAT_ERROR(ct,TTCIT_CTRL_STOP_L2MI,"L2m incomplete    ");
  OM_PRINT_WHAT_ERROR(ct,TTCIT_CTRL_STOP_L2MD,"L2m data error    ");
  OM_PRINT_WHAT_ERROR(ct,TTCIT_CTRL_STOP_CAL, "Calibration error ");
  OM_PRINT_WHAT_ERROR(ct,TTCIT_CTRL_STOP_BCNT,"BC mismatch error ");
  printf("----\n");
}

void OM_print_error_pattern(){
  int er = 0x0;

  er = OM_get_status();
  printf("On-Board Monitor Error Pattern Before Last Stop:\n");
  printf("----------------------------------------------\n");
  OM_PRINT_ERR_SEEN(er,TTCIT_STATUS_PP,"Pre-Pulz error");
  OM_PRINT_ERR_SEEN(er,TTCIT_STATUS_L0S,"L0 spurious");
  OM_PRINT_ERR_SEEN(er,TTCIT_STATUS_L1S,"L1 spurious");
  OM_PRINT_ERR_SEEN(er,TTCIT_STATUS_L1MM,"L1m missing");
  OM_PRINT_ERR_SEEN(er,TTCIT_STATUS_L1MS,"L1m spurious");
  OM_PRINT_ERR_SEEN(er,TTCIT_STATUS_L1MI,"L1m incomplete");
  OM_PRINT_ERR_SEEN(er,TTCIT_STATUS_L1MD,"L1m data error");
  OM_PRINT_ERR_SEEN(er,TTCIT_STATUS_L2MM,"L2m missing");
  OM_PRINT_ERR_SEEN(er,TTCIT_STATUS_L2MS,"L2m spurious");
  OM_PRINT_ERR_SEEN(er,TTCIT_STATUS_L2MI,"L2m incomplete");
  OM_PRINT_ERR_SEEN(er,TTCIT_STATUS_L2MD,"L2m data error");
  OM_PRINT_ERR_SEEN(er,TTCIT_STATUS_CAL,"Calibration error");
  OM_PRINT_ERR_SEEN(er,TTCIT_STATUS_BCNT,"BC mismatch");
  printf("---\n");
}

void OM_print_status(){
  int stp = 0x0;
  int con = 0x0;
  int st = 0x0;
  char *one[3] = { "Continuous SSM Fill OFF",
		   "Continuous SSM Fill ON",
		   "BUG BUG BUG" };
  char *two[3] = { "                    STOPPED",
		   "                    RUNNING",
		   "BUG BUG BUG"};
  char *zero[2] = { "On-Board Monitor Status: ",
		    "                        " };
  enum ttcit_OM_status os;

  os = OM_get_activity(&st);
  switch(os){
  case OM_STOPPED_ON:
    stp = 0;
    con = 1;
    break;
  case OM_RUNNING:
    stp = 1;
    con = 1;
    break;
  case OM_INACTIVE:
    con = 0;
    stp = 0;
    break;
  default:
    con = 2;
    stp = 2;
  }

  printf("%s %s\n%s %s\n",zero[0],one[con],zero[1],two[stp]);
}

enum ttcit_OM_status OM_get_activity(int *ermask){
  enum ttcit_OM_status retval;
  int st = 0x0;
  int con = 0x0;
  int stp = 0x0;

  st = OM_get_control();
  *ermask = st & OM_guts.any_error_mask;
  con = ((st & TTCIT_CTRL_CONTIWRIT) == TTCIT_CTRL_CONTIWRIT); 
  stp = ((st & TTCIT_CTRL_STOPCWRIT) == TTCIT_CTRL_STOPCWRIT);
  if(con){
    if(stp){
      retval = OM_STOPPED_ON;
    }else{
      retval = OM_RUNNING;
    }
  }else{
    retval = OM_INACTIVE;
  }
  return retval;
}

void OM_store_errmask(struct om_errset_list *list){
  int n = 0;
  int i;
  int mask = 0x0;
  int control;
  int oldmask = 0x0;
  int flag;

  oldmask = OM_get_error_mask(&control);

  n = (*list).errcode;
  for(i = 1; i <= n; i++){
    mask = (*(list + i)).errcode;  /* New mask */
    oldmask = ~mask & oldmask;             /* Set a bit to 0 in oldmask */
    flag = (*(list + i)).flag;
    if(flag){
      oldmask = mask | oldmask;
    }
  }
  OM_guts.error_mask = oldmask;
  oldmask = oldmask | control;
  OM_set_control(oldmask);
}

void OM_get_counters(struct om_hw_counters *c){

  c->L0 = VMER32(HW_COUNTER_L0);
  c->L1 = VMER32(HW_COUNTER_L1);
  c->L1m = VMER32(HW_COUNTER_L1M);
  c->L2a = VMER32(HW_COUNTER_L2A);
  c->L2r = VMER32(HW_COUNTER_L2R);

  c->PP = VMER32(HW_COUNT_ERR_PP);
  c->L0S =  VMER32(HW_COUNT_ERR_L0S);
  c->L1S =  VMER32(HW_COUNT_ERR_L1S);

  c->L1MM =  VMER32(HW_COUNT_ERR_L1MM);
  c->L1MS =  VMER32(HW_COUNT_ERR_L1MS);
  c->L1MI =  VMER32(HW_COUNT_ERR_L1MI);
  c->L1MD =  VMER32(HW_COUNT_ERR_L1MD);

  c->L2MM =  VMER32(HW_COUNT_ERR_L2MM);
  c->L2MS =  VMER32(HW_COUNT_ERR_L2MS);
  c->L2MI =  VMER32(HW_COUNT_ERR_L2MI);
  c->L2MD =  VMER32(HW_COUNT_ERR_L2MD);

  c->CAL =  VMER32(HW_COUNT_ERR_CAL);
  c->BCNT =  VMER32(HW_COUNT_ERR_BCNT);

  c->PP_count = VMER32(HW_PP_COUNTER);
}

void OM_reload_error_mask(){
  int old = 0x0;
  int control = 0x0;
  int mask;

  old = OM_get_error_mask(&control);
  mask = OM_guts.ermask_memory | control;
  OM_set_control(mask);
}

void OM_set_time_l0_l1(int t){
  VMEW32(HW_TIME_L0_L1,t);
}

void OM_reset_counters(){
  VMEW32(HW_RESET_COUNTERS,0x1);
}

enum ttcit_OM_RYBY OM_status_SSM(){
  int n;
  enum ttcit_OM_RYBY retval = OM_BUSY;

  n = OM_get_status() & TTCIT_STATUS_SSM_ACTIVE;
  if(n == TTCIT_STATUS_SSM_ACTIVE){
    retval = OM_BUSY;
  }else{
    retval = OM_READY;
  }
  return retval;
}

void OM_remember_error_mask(){
  int n;
  int comm;

  n = OM_get_error_mask(&comm);
  OM_guts.ermask_memory = n;
}

int OM_wait_till_SSM_full(int verbal, int minutes){
  int irc = 0;
  unsigned int slept = 0;
  unsigned int tosleep;
  unsigned int off;
  int forsd = 0;
  enum ttcit_OM_RYBY ssm_st;
#ifdef TTCIT_NEVER_EVER_USE__
  int nloop = 100;
#endif
  int loop = 0;
  unsigned int tinter = 1; /* Check every 10 seconds */

  off = (unsigned int)minutes * 60; /* sleep is in seconds */

  /*
    Set the SSM to ZERO
   */
  
  loop = 0;
#ifdef TTCIT_NEVER_EVER_USE__
  do{
    OM_set(OM__STOP);
    tosleep = my_funky_sleep(1);         /* 1 sec is more than enough */
    ssm_st = OM_status_SSM();
    if(++loop > nloop){
      irc = -1;
      goto last_one;
    }
  }while(ssm_st == OM_BUSY);
#endif

  OM_Reset();
  OM_clear_SSM_contents();   /* We need clean SSM at the start */

  /*
    Start the reading loop, assume we WANT to do it
   */
  OM_set(OM__ON_START);

  /*
    Check every tinter seconds, accumulate slept time until minutes reached,
    then force STOP.
   */
  do{
    /* tosleep = my_funky_sleep(tinter) */
    tosleep = sleep(tinter);
    slept += tinter;
    ssm_st = OM_status_SSM();
    if(ssm_st == OM_READY){        /* Stopped, we can leave */
      irc = 0;
      goto last_one;
    }
  }while(slept < off);
  /*
    Still there, it means we are still taking data, forcing STOP
   */
  forsd = 1;
  OM_set(OM__STOP);
  /*  tosleep = my_funky_sleep(1); */
  tosleep = sleep(2);
  ssm_st = OM_status_SSM();
  if(ssm_st == OM_BUSY){
    irc = -2;
  }

 last_one:
  if(verbal){
    slept = slept / 60;
    printf("OM_SSM_WAIT: slept %u minutes, IRC = %d\n",slept,irc);
    if(forsd){
      printf("   Data collection was forced to stop\n");
    }
  }

  return irc;
}

int OM_fetch_SSM(){
  int retval = 0;
  enum ttcit_OM_RYBY ssm_st;
  int dummy;
  int maxloop = 10;
  int nloops = 100;
  int N1;                  /* The last address in SSM */
  int N2;                  /* The first address in SSM */
  int nw;
  int i;
  int ssm_val;
  int nread_ssm = 0;

  do{
    /*
      Make sure the SSM is not BUSY
     */
    ssm_st = OM_status_SSM();
    if(ssm_st == OM_BUSY){
      retval = -1;
      break;
    }

    /*
      Get the starting address
     */
    dummy = 0;
  once_more:
    N1 = read_SSM_address();
    if(N1 < 0){
      if(dummy++ > maxloop){
	retval = -2;
	break;
      }
      my_dummy_loop(nloops);
      goto once_more;
    }
    N2 = (N1 == (MEGA - 1)) ? 0 : N1 + 1;

    /*
      We must read the whole SSM, since we do no know its beginning
     */
    nw = MEGA;
    for(i = nw-1; i >= 0; i--){         /* This must be MEGA reads */
      ssm_val = read_last_SSM_word();
      SSM.buf[i] = ssm_val;
      nread_ssm++;
    }
    SSM.top = nw;

    /*
      Address check:
     */
    dummy = 0;
  once_more_2:
    nw = read_SSM_address();
    if(nw < 0){
      if(dummy++ > maxloop){
	retval = -3;
	break;
      }
      my_dummy_loop(nloops);
      goto once_more_2;
    }
    if(nw != N1){          /* After the last read the next item is the 1-st */
      retval = -5;
      SSM.top = -SSM.top;
      break;
    }
    /*
      Remove everything before the 1-st L0
     */
    nw = OM_start_at_L0(&SSM);
    if(nw < 0){
      retval = -6 + 100 * nw;
      break;
    }
    retval = SSM.top;
  }while(0);

  return retval;
}

int OM_fetch_bcnt_fifo(){
  /*
    At this stage the BC fifo is of no use, just fill it full with 0's
   */
  int i;
  bcnt.top = BCNT_FIFO_SIZ;
  for(i = 0; i < bcnt.top; i++){
    bcnt.buf[i] = 0x0;
  }
  return BCNT_FIFO_SIZ;
}

void OM_fetch_print_SS_bc(int minutes){
#define TTCIT_CLONE_FETCH_PRINT_SS__
#define TTCIT_CLONE_FETCH_PRINT_SS_2__
#include "ttcit_replicas.h"
#undef TTCIT_CLONE_FETCH_PRINT_SS_2__
}

void OM_fetch_print_SS_bc_nowait(){
#define TTCIT_CLONE_FETCH_PRINT_SS__
#define TTCIT_CLONE_FETCH_PRINT_SS_3__
#include "ttcit_replicas.h"
#undef TTCIT_CLONE_FETCH_PRINT_SS_3__
}


void OM_soft_monitor_single_ssm(int what, int minutes){
#define TTCIT_CLONE_SM_SINGLE_SSM__
#define TTCIT_CLONE_SM_SINGLE_SSM_2__
#include "ttcit_replicas.h"
#undef TTCIT_CLONE_SM_SINGLE_SSM_2__
}

void OM_soft_monitor_single_nowait(int what){
#define TTCIT_CLONE_SM_SINGLE_SSM__
#define TTCIT_CLONE_SM_SINGLE_SSM_3__
#include "ttcit_replicas.h"
#undef TTCIT_CLONE_SM_SINGLE_SSM_3__
}

int OM_clear_SSM_contents(){
  int irc = 0;
  clear_SS_memory_on_board();
  return irc;
}

int OM_start_at_L0(struct SSMbuffer *ssm){
  /* 
     We must find the first bit set in A channel and start SSM
     buffer from this position
   */
  int nw = -7;
  int first_L0 = -1;
  int i;
  int nfound = 0;
  unsigned long wrd, mrd;
  int j;

  do{
    if(ssm->top <= 0){
      nw = -2;
      break;
    }
    if(ssm->top > MEGA){
      nw = -3;
      break;
    }
    nfound = ssm->top;
    for(i = 0; i < (nfound - 1); i++){
      wrd = ssm->buf[i];
      mrd = ssm->buf[i+1];
      if( ((wrd & MASK_L1ACCEPT) == MASK_L1ACCEPT) && 
	  ((mrd & MASK_L1ACCEPT) != MASK_L1ACCEPT)){
	first_L0 = i;
	goto get_out;
      }
    }
  get_out:
    if(first_L0 < 0){
      nw = -4;
      break;
    }
    /*
      It looks like we are skipping the first L0 -> remove if not
     */
    first_L0++;

    j = 0;
    for(i = first_L0; i < nfound; i++){
      ssm->buf[j++] = ssm->buf[i];
    }
    ssm->top = (j == 0) ? -1 : j;
    nw = ssm->top;

  }while(0);

  return nw;
}

void OM_synchro_settings(int how){

  switch(how){
  case 0:         /* OM -> soft */
    printf("Copying Error Mask from On-Board monitor to software\n");
    OM_om_2_soft();
    break;
  case 1:         /* Soft -> OM */
    printf("Copying the Error Mask from software to On-Board monitor\n");
    OM_soft_2_om();
    break;
  default:
    printf("@ OM_synchro_settings: Unknown request, no action\n");
    break;
  }
}

void OM_om_2_soft(){
  int em = 0x0;
  int com = 0x0;
  int t01 = 0;
  int tbc = 0;

#define TTCIT_OM_SYNC_DECL__
#define TTCIT_OM_SYNC_DECL_S__
#define TTCIT_OM_SYNC_DECL_H__
#include "ttcit_replicas.h"
#undef TTCIT_OM_SYNC_DECL_S__
#undef TTCIT_OM_SYNC_DECL_H__

  /* Get whetewer cen be got from OM settings */

  em = OM_get_error_mask(&com);  /* error mask */

  t01 = VMER32(HW_TIME_L0_L1);  /* time L0_L1 */
  t01 += TTCIT_L0_L1_HW_SW_KONST;

  /*
    Extract individua stopping conditions from the error mask
   */
  OM_IS_STOP(em,L1MM);
  OM_IS_STOP(em,L1MS);
  OM_IS_STOP(em,L1MI);
  OM_IS_STOP(em,L1MD);
  OM_IS_STOP(em,L2MM);
  OM_IS_STOP(em,L2MS);
  OM_IS_STOP(em,L2MI);
  OM_IS_STOP(em,L2MD);
  OM_IS_STOP(em,PP);
  OM_IS_STOP(em,L0S);
  OM_IS_STOP(em,L1S);
  OM_IS_STOP(em,CAL);
  OM_IS_STOP(em,BCNT);

  /*
    Convert OM error mask conditions into soft stopping conditions
   */
  Stop_at_L0S = OM_IS_1(L0S);
  Stop_at_L1S = OM_IS_1(L1S);
  Stop_at_L1T = OM_IS_1(L1S);
  Stop_at_L1M = OM_IS_1(L1MS);
  Stop_at_L1Mo = OM_IS_1(L1MM);
  Stop_at_L1F = OM_IS_1((L1MD + L1MI));
  Stop_at_L2T = OM_IS_1(L2MM);
  Stop_at_L2Ts = OM_IS_1(L2MS);
  Stop_at_L2F = OM_IS_1((L2MD + L2MI));
  Stop_at_BCID_diff = OM_IS_1(BCNT);

  SetStopCondition(Stop_at_L0S, Stop_at_L1S, Stop_at_L1T, Stop_at_L1M,
		   Stop_at_L1Mo, Stop_at_L1F, Stop_at_L2T, Stop_at_L2Ts,
		   Stop_at_L2F, Stop_at_BCID_diff);

  /*
    Move the L0-L1 time interval definition from HW
   */
  tbc = VMER32(HW_BCNT_DIFFERENCE);
  DefaultTimeWindows();
  SetTimeWindows(t01,0,-1,-1,tbc,0,-1);
}

void OM_soft_2_om(){
  int t01;
  struct MONITOR_stops s;
  struct ttc_it_options o;
  int irc;
  w32 er = 0x0;
  int tbc;

#define TTCIT_CTRL_STOP_NIKDY 0x0

  do{
    get_TTCIT_stop_conds(&s);
    irc = get_SSM_ANALYZ_options(&o);
    if(irc != 0){
      printf("OM_soft_2_om: Cannot fetch options from SSM_analyz IRC = %d\n",
	     irc);
      break;
    }
    t01 = o.l0_l1_win - TTCIT_L0_L1_HW_SW_KONST;
    VMEW32(HW_TIME_L0_L1,t01);
    tbc = o.bc_l1_l2_win;
    VMEW32(HW_BCNT_DIFFERENCE,tbc);

    SA_IS_STOP(er,L0S,L0S,NIKDY);
    SA_IS_STOP(er,L1S,L1S,NIKDY);
    SA_IS_STOP(er,L1T,L1S,NIKDY);
    SA_IS_STOP(er,L1M,L1MS,NIKDY);
    SA_IS_STOP(er,L1Mo,L1MM,NIKDY);
    SA_IS_STOP(er,L1F,L1MD,L1MI);
    SA_IS_STOP(er,L2T,L2MM,NIKDY);
    SA_IS_STOP(er,L2Ts,L2MS,NIKDY);
    SA_IS_STOP(er,L2F,L2MD,L2MI);
    SA_IS_STOP(er,BCID_diff,BCNT,NIKDY);
    VMEW32(TTCIT_CONTROL,er);

  }while(0);

#undef TTCIT_CTRL_STOP_NIKDY 

}

void OM_reset_board(){
  w32 control = 0x0;
  w32 time = 0x0;
  w32 delay = 0x0;
  w32 bcdif = 0x0;
  /*
    Fetch values of the Error mask, L0-L1 time and SSM top delay
   */
  control = VMER32(TTCIT_CONTROL) & OM_guts.any_error_mask;
  time = VMER32(HW_TIME_L0_L1);
  delay = VMER32(HW_DELAY_SSM_STOP);
  bcdif = VMER32(HW_BCNT_DIFFERENCE);
  /*
    Reset the TTCit board
   */
  ResetTTCit();
  /*
    Return all fetched values to where they belong
   */
  VMEW32(TTCIT_CONTROL,control);
  VMEW32(HW_TIME_L0_L1,time);
  VMEW32(HW_DELAY_SSM_STOP,delay);
  VMEW32(HW_BCNT_DIFFERENCE,bcdif);

  if(OM_guts.prt == OM_YES_PRINT){
    printf("OM_Reset: Control word   restored %X\n",control);
    printf("OM_Reset: L0 - L1 time   restored %d\n",time);
    printf("OM_Reset: SSM stop delay restored %d\n",delay);
    printf("OM_Reset: BCNT diff.     restored %d\n",bcdif);
  }
  
}

void OM_noprint(){
  OM_guts.prt = OM_NO_PRINT;
}

void OM_yes_print(){
  OM_guts.prt = OM_YES_PRINT;
}
