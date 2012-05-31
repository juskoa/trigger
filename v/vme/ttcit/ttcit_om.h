#ifndef _TTCIT_OM_H__
#define _TTCIT_OM_H__

#include "ttcit_logic.h"

/*
  Macros
 */

#define OM_ADD_TO_ERR_LIST(A,B,C,D) if(A != 0){ B[++C].errcode = D; \
    B[C].flag = 1; }else{ B[++C].errcode = D; B[C].flag = 0;}

#define OM_PRINT_WHAT_ERROR(A,B,C) if((A & B) == B){ \
    printf("    ... Stop on %s    --- %s\n",C,"  SET");}else{ \
    printf("    ... Stop on %s    --- %s\n",C,"  unset");}

#define OM_PRINT_ERR_SEEN(A,B,C) if((A & B) == B){ \
    printf("  ... %s\n",C); }

#define OM_PRINT_COUNTER(A,B) printf("   %s    %d\n",A,B)

#define OM_IS_STOP(A,B) B = \
    ((A & TTCIT_CTRL_STOP_##B) == TTCIT_CTRL_STOP_##B) ? 1 : 0

#define SA_IS_STOP(E,S,H,G) E |= \
  ((s.Stop_at_##S == 1) ? TTCIT_CTRL_STOP_##H | TTCIT_CTRL_STOP_##G : 0x0)

#define OM_IS_1(A) ((A) >= 1) ? 1 : 0
/*
  Structure holding all information about OM internals
 */

enum ttcit_OM_ssm { OM_CONTINUOUS, OM_SCOPE };

enum ttcit_OM_status { OM_STOPPED_ON, OM_INACTIVE, OM_RUNNING };

enum ttcit_OM_RYBY { OM_READY, OM_BUSY };

enum ttcit_OM_PRINT { OM_NO_PRINT, OM_YES_PRINT };

struct OM_internal_state {

  int error_mask;                   /* Currently set error mask */
  enum ttcit_OM_ssm SSM_mode;    /* SSM mode */
  enum ttcit_OM_status status;   /* OM status */
  int any_error_mask;            /* OR'ed all errors */
  int command_mask;
  int time0_1;                   /* Time between L0 - L1 as set last time */
  int ermask_memory;           /* Remembered error mask */

  enum ttcit_OM_PRINT prt;      /* OM print state */
};

enum ttcit_OM_setopt { OM__ON_START, OM__ON_NOSTART, OM__OFF,
		       OM__STOP, OM__START };

struct om_errset_list {
  int errcode;
  int flag;
};

struct om_hw_counters {
  /*
    Trigger counters
   */

  int L0;                 /* Nr. of L0's */
  int L1;                 /* Nr. of L1's */
  int L1m;                /* Nr. of L1m's */
  int L2a;                /* Nr. of L2a messages */
  int L2r;                /* Nr. of L2r messages */

  /*
    Error counters
   */

  int PP;                 /* PP errors */
  int L0S;                /* Spurious L0's */
  int L1S;                /* Spurious L1's */

  int L1MM;               /* Various L1m errors */
  int L1MS;
  int L1MI;
  int L1MD;

  int L2MM;               /* Various L2m errors */
  int L2MS;
  int L2MI;
  int L2MD;

  int CAL;                /* Nr. of CAL errors */
  int BCNT;               /* Nr. of BC mismatches */

  int PP_count;           /* PP counter */
};

/*
  Functions
 */

void OM_init();
void OM_set(enum ttcit_OM_setopt action);
int OM_get_control();
int OM_get_status();
void OM_set_control(int sword);
int OM_get_error_mask(int *command);
void OM_reload_error_mask();
void OM_print_error_mask(int errmask);
void OM_print_error_pattern();
void OM_print_status();
enum ttcit_OM_status OM_get_activity(int *ermask);
void OM_store_errmask(struct om_errset_list *list);
void OM_get_counters(struct om_hw_counters *c);
void OM_set_time_l0_l1(int t);
void OM_reset_counters();
enum ttcit_OM_RYBY OM_status_SSM();
void OM_remember_error_mask();
int OM_wait_till_SSM_full(int verbal, int minutes);
int OM_fetch_SSM();
int OM_fetch_bcnt_fifo();
void OM_fetch_print_SS_bc(int minutes);
int OM_clear_SSM_contents();
int OM_start_at_L0(struct SSMbuffer *ssm);
void OM_soft_monitor_single_ssm(int what, int minutes);
void OM_fetch_print_SS_bc_nowait();
void OM_soft_monitor_single_nowait(int what);
void OM_synchro_settings(int how);
void OM_om_2_soft();
void OM_soft_2_om();
void OM_reset_board();
void OM_noprint();
void OM_yes_print();

#endif  /* !_TTCIT_OM_H__ */
