/*
  Analysis of SSM and BC FIFO.

  Common stuff for ON-LINE and OFF-LINE analysis
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "ttcit_conf.h"
#include "ssm_analyz.h"
#include "ttcit_mm.h"
#include "ttcit_io.h"

/*
  Communication with the outside world
 */

struct ttc_it_event_errors ssm_analyz_errors;

static struct ttc_it_options options;

struct ttc_it_counters counters;
struct ttc_it_errors errores;
struct ttc_it_times timing;

static struct ttc_it_internal guts;

struct ttc_it_counters sum_count;
struct ttc_it_errors sum_err;
struct ttc_it_times sum_time;

static DECL_FIFO(AB_fifo);
static DECL_FIFO(BC_fifo);

static DECL_FIFO(L0L1_fifo);
static DECL_FIFO(L0L1L1m_fifo);
static DECL_FIFO(L0L1L1mL2_fifo);

static DECL_FIFO(L1_fifo);
static DECL_FIFO(RoI_fifo);

static DECL_FIFO(error_list);

#ifdef _MONITOR_SCAN_BCID_DIFF__
static DECL_FIFO(DirtyBCID_fifo);

#define TTCIT_MAX_DIRTY_BCID      10240

struct ttc_it_dirty_BCID {
  int tick;                       /* tick at the time of sequence assembly */
  int bc_l1;                      /* BCID at the L1 arrival */
  int bc_l2a;                     /* BCID from the L2a Header */
  int dbc;                        /* Difference from SSM analyzer */
};

struct ttcit_mm_I1Di1 hist_dirty_dbc;

#endif

/*
  This is the TTCit analyzer state machine main data structure
 */

enum ttc_it_A_states { EMPTY, L0 };
struct ttc_it_A_machine {
  enum ttc_it_A_states state;   /* State in which A machine is in */

  int virgin;   /* A channel state machine in Virgin state TRUE/FALSE */
  int empty;    /* A channel FIFO is empty, i.e. missing L1's are not an
		   error */

  int L0_time;  /* Time of arrival of L0 signal */
  int L1_time;  /* Time of arrival of L1 signal */

  int BCID_L0;  /* BCID from TTC Rx at the arrival of L0 signal */
  int BCID_L1;  /* BCID from TTC Rx at the arrival of L1 signal */ 

  int L1_rising; /* Set to TRUE if the first L1 hit identified
                    When true, skip the processing of hit (it has already
		    been processed, when the first L1 hit arrived.
                    FALSE after skipping the 2-nd L1 hit
		    or when L0 detected */ 

  int bcid_found; /* Number of BCID's found in BC ID FIFO. */

  int tick_last_l0;  /* Tick of the last L0 seen */
  int tick_last_l1;  /* Tick of the last L1 seen */
  int bcid_last_l0;
  int bcid_last_l1;
};

struct ttc_it_B_machine {

  enum data_strobe_machine_states strobe_state; /* Data Strobe machine state */

  enum ttc_it_B_states state;  /* State the B machine is in */

  enum ttc_it_B_groups group;  /* Group of the data type being processed */

  int n_words;                 /* Number of words after Header for current 
				  group */
  int n_words_received;        /* Number of words in given group received */

  int hdr;           /* Copy of header */
  int words[20];     /* Copy of data words */

  int l1f;          /* TRUE if L1F error detected */
  int l2f;          /* TRUE if L2F error detected */
  int roif;         /* TRUE if ROIF error detected */
  int format_error; /* TRUE if format error has been detected */

  int tick_start;    /* Start of the sequence timing in ticks */
  int tick_end;      /* End of the sequence timing in ticks */
  int swap;

};


struct ttc_it_state_machine {

  int virgin;    /* Virgin state TRUE/FALSE, we must ignore tails of 
		    trigger sequences that started sooner than we switched
		    the Snap Shot memory ON */

  int empty;     /* TRUE/FALSE is the AB FIFO is empty. We must not count
		    missin tails of trigger sequences that has not been 
		    stored in snapshot memory */

  int L0;       /* TRUE/FALSE : Is L0 present ? */

  int SSM_mode;  /* SSM mode of operation, some errors cannot be detected
		    in SEQ mode */

  int tick;     /* Actual value of tick */

  int l0_seen;  /* Nr. of L0's seen in the current Snap Shot */
  int l1_seen;  /* Nr. of L1's seen in the current Snap Shot */

  /*
    A channel
   */

  struct ttc_it_A_machine A;

  /*
    B channel
   */

  struct ttc_it_B_machine B;

};

static struct ttc_it_state_machine maquina;

#ifdef TTCIT_EXTRA_DBG_PRINT_1__
static int ttc_it_extra_print_1_want__;
static int ttc_it_extra_print_2_want__;

void ttc_it_set_noprint(int what){
  ttc_it_extra_print_1_want__ = (!what);
  ttc_it_extra_print_2_want__ = FALSE;
}

void ttc_it_set_prt_suppress(){
  ttc_it_extra_print_2_want__ = TRUE;
}
#endif

void ttc_it_clear_counters(struct ttc_it_counters *c){

  c->l0 = 0;
  c->l1 = 0;
  c->l1m = 0;
  c->l2a = 0;
  c->l2r = 0;

  c->roi = 0;

  c->lxx = 0;

  c->accepted = 0;
  c->l1_reject = 0;
  c->l2_reject = 0;

  c->l0l1_fakes = 0;
}

void ttc_it_clear_errors(struct ttc_it_errors *e){

  e->l0s = 0;
  e->l1s = 0;
  e->l1t = 0;
  e->l1m = 0;
  e->l1mo = 0;
  e->l1f = 0;
  e->l2t = 0;
  e->l2ts = 0;
  e->l2f = 0;
  e->bcid = 0;
  e->roif = 0;
  e->rois = 0;
  e->roit = 0;
}

void ttc_it_clear_timing(struct ttc_it_times *t){

  t->l0_l0_mean = 0.;
  t->l0_l0_n = 0.;
  t->l0_l0_min = TTC_IT_BIG_U_NR;
  t->l0_l0_max = TTC_IT_SMALL_U_NR;

  t->l0_l1_mean = 0.;
  t->l0_l1_n = 0.;
  t->l0_l1_min = TTC_IT_BIG_U_NR;
  t->l0_l1_max = TTC_IT_SMALL_U_NR;

  t->l1_l2_mean = 0.;
  t->l1_l2_n = 0;
  t->l1_l2_min = TTC_IT_BIG_U_NR;
  t->l1_l2_max = TTC_IT_SMALL_U_NR;

  t->l1_l1m_mean = 0.;
  t->l1_l1m_n = 0;
  t->l1_l1m_min = TTC_IT_BIG_U_NR;
  t->l1_l1m_max = TTC_IT_SMALL_U_NR;

  t->bc_l1_l2_mean = 0;
  t->bc_l1_l2_n = 0;
  t->bc_l1_l2_min = TTC_IT_BIG_U_NR;
  t->bc_l1_l2_max = TTC_IT_SMALL_U_NR;

  t->l1_roi_mean = 0.;
  t->l1_roi_n = 0.;
  t->l1_roi_min = TTC_IT_BIG_U_NR;
  t->l1_roi_max = TTC_IT_SMALL_U_NR;

  t->bc_l1_roi_mean = 0.;
  t->bc_l1_roi_n = 0.;
  t->bc_l1_roi_min = TTC_IT_BIG_U_NR;
  t->bc_l1_roi_max = TTC_IT_SMALL_U_NR;
}

void ttc_it_print_last_counters(){
  ttc_it_print_counters(&sum_count);
}

void ttc_it_print_last_errors(){
  ttc_it_print_errors(&sum_err);
}

void ttc_it_print_last_timing(){
  ttc_it_print_timing(&sum_time);
}

void ttc_it_print_counters(struct ttc_it_counters *c){
  int i;
  int finish;
  char *name;
  unsigned long val;

  do{
    printf("SSM ANALYSIS:    COUNTERS\n");
    printf("-------------------------\n");
    printf(" \n");

    finish = FALSE;
    for(i = 1; i <= 11; i++){
      switch(i){
      case 1:
	name = "L0";
	val = c->l0;
	break;
      case 2:
	name = "L1";
	val = c->l1;
	break;
      case 3:
	name = "L1m";
	val = c->l1m;
	break;
      case 4:
	name = "L2a";
	val = c->l2a;
	break;
      case 5:
	name = "L2r";
	val = c->l2r;
	break;
      case 6:
	name = "Unknown message";
	val = c->lxx;
	break;
      case 7:
	name = "L0-L1-L2 accepted";
	val = c->accepted;
	break;
      case 8:
	name = "L1 rejected";
	val = c->l1_reject;
	break;
      case 9:
	name = "L2 rejected";
	val = c->l2_reject;
	break;
      case 10:
	name = "RoI";
	val = c->roi;
	break;
      case 11:
	name = "L0L1 fakes";
	val = c->l0l1_fakes;
	break;
      default:
	finish = TRUE;
	break;
      }
      if(finish){
	break;
      }
      printf("Number of %25s   : %lu\n",name,val);
    }

    printf(" --- \n");

  }while(0);
}

void ttc_it_print_errors(struct ttc_it_errors *e){
  int i;
  char *name;
  unsigned long val;
  int finish;

  do{

    printf("SSM ANALYSIS:    DETECTED ERRORS\n");
    printf("--------------------------------\n");
    printf(" \n");

    finish = FALSE;

    for(i=1; i <= 13; i++){
      switch(i){
      case 1:
	name = "L0S";
	val = e->l0s;
	break;
      case 2:
	name = "L1S";
	val = e->l1s;
	break;
      case 3:
	name = "L1T";
	val = e->l1t;
	break;
      case 4:
	name = "L1M";
	val = e->l1m;
	break;
      case 5:
	name = "L1Mo";
	val = e->l1mo;
	break;
      case 6:
	name = "L1F";
	val = e->l1f;
	break;
      case 7:
	name = "L2T";
	val = e->l2t;
	break;
      case 8:
	name = "L2Ts";
	val = e->l2ts;
	break;
      case 9:
	name = "L2F";
	val = e->l2f;
	break;
      case 10:
	name = "L1-L2 BCID diff";
	val = e->bcid;
	break;
      case 11:
	name = "RoI-F";
	val = e->roif;
	break;
      case 12:
	name = "RoI-S";
	val = e->rois;
	break;
      case 13:
	name = "RoI-T";
	val = e->roit;
	break;
      default:
	finish = TRUE;
      }
      if(finish){
	break;
      }
      printf("Number of errors %20s   : %lu\n",name,val);
    }
    printf(" ---\n");
  }while(0);
}

void ttc_it_print_timing(struct ttc_it_times *t){
  int i;
  char *name;
  double dval;
  unsigned long n, x;
  int finish;

  do{

    printf("SSM ANALYSIS:    TIMING INFO\n");
    printf("----------------------------\n");
    printf(" \n");

    finish = FALSE;

    for(i = 0; i <= 7; i++){
      switch(i){
      case 0:
	name = "L0 - L0 interval";
	dval = t->l0_l0_mean;
	n = (unsigned long)t->l0_l0_min;
	x = (unsigned long)t->l0_l0_max;
	break;
      case 1:
	name = "L0 - L1 interval";
	dval = t->l0_l1_mean;
	n = t->l0_l1_min;
	x = t->l0_l1_max;
	break;
      case 2:
	name = "Empty";
	dval = 0.;
	n = 0;
	x = 0;
	continue;
	break;
      case 3:
	name = "L1 - L2a/L2r interval";
	dval = t->l1_l2_mean;
	n = t->l1_l2_min;
	x = t->l1_l2_max;
	break;
      case 4:
	name = "L1 - L1m interval";
	dval = t->l1_l1m_mean;
	n = t->l1_l1m_min;
	x = t->l1_l1m_max;
	break;
      case 5:
	name = "L1 - L2a/L2r in BC ID";
	dval = t->bc_l1_l2_mean;
	n = t->bc_l1_l2_min;
	x = t->bc_l1_l2_max;
	break;
      case 6:
	name = "L1 - RoI time interval";
	dval = t->l1_roi_mean;
	n = t->l1_roi_min;
	x = t->l1_roi_max;
	break;
      case 7:
	name = "L1 - RoI BCID difference";
	dval = t->bc_l1_roi_mean;
	n = t->bc_l1_roi_min;
	x = t->bc_l1_roi_max;
	break;
      default:
	finish = TRUE;
	break;
      }
      if(finish){
	break;
      }
      printf("%25s  : MEAN = %10.2f : MIN = %6lu : MAX = %6lu\n",
	     name,dval,n,x);
    }
    printf(" ---\n");

  }while(0);

}

void ttc_it_collect_last_counterr(struct ttc_it_counters *cn,
				  struct ttc_it_errors *er){
  size_t Ncount;
  size_t Nerr;
  void *dumptr;

  Ncount = sizeof(struct ttc_it_counters);
  Nerr = sizeof(struct ttc_it_errors);
  do{
    if(cn == NULL){
      break;
    }
    dumptr = memcpy(cn, &sum_count, Ncount);

    if(er == NULL){
      break;
    }
    dumptr = memcpy(er, &sum_err, Nerr);

  }while(0);
}

void ttc_it_add_counters(struct ttc_it_counters *src,
			 struct ttc_it_counters *dest){

  do{

    if((src == NULL) || (dest == NULL)){
      break;
    }

    TTCIT_ADDTO(l0);
    TTCIT_ADDTO(l1);
    TTCIT_ADDTO(l1m);
    TTCIT_ADDTO(l2a);
    TTCIT_ADDTO(l2r);
    TTCIT_ADDTO(roi);
    TTCIT_ADDTO(lxx);
    TTCIT_ADDTO(accepted);
    TTCIT_ADDTO(l1_reject);
    TTCIT_ADDTO(l2_reject);
    TTCIT_ADDTO(l0l1_fakes);

  }while(0);
}

void ttc_it_add_errors(struct ttc_it_errors *src,
		       struct ttc_it_errors *dest){

  do{
    if((src == NULL) || (dest == NULL)){
      break;
    }

    TTCIT_ADDTO(l0s);
    TTCIT_ADDTO(l1s);
    TTCIT_ADDTO(l1t);
    TTCIT_ADDTO(l1m);
    TTCIT_ADDTO(l1mo);
    TTCIT_ADDTO(l1f);
    TTCIT_ADDTO(l2t);
    TTCIT_ADDTO(l2ts);
    TTCIT_ADDTO(l2f);
    TTCIT_ADDTO(bcid);
    TTCIT_ADDTO(roif);
    TTCIT_ADDTO(rois);
    TTCIT_ADDTO(roit);

  }while(0);
}

void ttc_it_add_timing(struct ttc_it_times *src,
		       struct ttc_it_times *dest){

  do{
    if((src == NULL) || (dest == NULL)){
      break;
    }

    TTCIT_ADDT(l0_l0);
    TTCIT_ADDT(l0_l1);
    TTCIT_ADDT(l1_l2);
    TTCIT_ADDT(l1_l1m);
    TTCIT_ADDT(bc_l1_l2);
    TTCIT_ADDT(l1_roi);
    TTCIT_ADDT(bc_l1_roi);

  }while(0);
}

void ttc_it_reset_options(){

  options.l0_l1_win = TTC_IT_L0_L1_WIN;
  options.l0_l1_wid = TTC_IT_L0_L1_WID;

  options.l1_l1m_timeout = TTC_IT_L1_L1M_TIMEOUT;

  options.l1_l2_timeout = TTC_IT_L1_L2_TIMEOUT;

  options.bc_l1_l2_win = TTC_IT_BC_L1_L2_WIN;
  options.bc_l1_l2_wid = TTC_IT_BC_L1_L2_WID;

  options.l1_roi_timeout = TTC_IT_L1_ROI_TIMEOUT;
  options.bc_l1_roi_win = 0;
  options.bc_l1_roi_wid = 0;

  options.bc_min = 0;
  options.bc_max = 3563;
  options.bc_fiz_min = 1;
  options.bc_fiz_max = 3445;
  options.bc_pp = 46 + 3445;   /* 1.2 us after end of physics */

  options.bc_l2_filter = 2000; /* "End of orbit", must be tuned */
  options.bc_l2_fil2 = 2000;   /* "End of beginning of orbit - tune it */

  ttc_it_clear_counters(&counters);
  ttc_it_clear_counters(&sum_count);

  ttc_it_clear_errors(&errores);
  ttc_it_clear_errors(&sum_err);

  ttc_it_clear_timing(&timing);
  ttc_it_clear_timing(&sum_time);
}

void ttc_it_set_opt_wins(int l0l1wind, int l0l1width,
			 int l1l1mtimeout, int l1l2timeout,
			 int bcwin, int bcwidth,
			 int l1roitimeout){

  options.l0_l1_win = (l0l1wind == -1) ? options.l0_l1_win : l0l1wind;
  options.l0_l1_wid = (l0l1width == -1) ? options.l0_l1_wid : l0l1width;

  options.l1_l1m_timeout = (l1l1mtimeout == -1) ? options.l1_l1m_timeout :
    l1l1mtimeout;

  options.l1_l2_timeout = (l1l2timeout == -1) ? options.l1_l2_timeout :
    l1l2timeout;

  options.bc_l1_l2_win = (bcwin == -1) ? options.bc_l1_l2_win : bcwin;
  options.bc_l1_l2_wid = (bcwidth == -1) ? options.bc_l1_l2_wid : bcwidth;

  options.l1_roi_timeout = (l1roitimeout == -1) ? options.l1_roi_timeout :
    l1roitimeout;
  options.bc_l1_roi_win = 1;
  options.bc_l1_roi_wid = 1;
}

void ttc_it_set_opt_const(int bcpp, int l2filter, int l2fil2){

  options.bc_pp = (unsigned long)bcpp;
  options.bc_l2_filter = (unsigned long)l2filter;
  options.bc_l2_fil2 = (unsigned long)l2fil2;
}

void ttc_it_print_opt_windows(){
  int ticks, twid;
  float time26ns = 0.026;  /* 26ns in microseconds */
  float c, w;
  char *text;
  char *txtw;
  int interval;
  int i;
  int stop_it;

  printf("TIME WINDOWS for trigger error definitions\n");
  printf(" ------------------------------------------\n");
  
  stop_it = FALSE;
  for(i=1; i<= 5; i++){
    switch(i){
    case 1:
      ticks = options.l0_l1_win;
      twid = options.l0_l1_wid;
      text = "L0 - L1 decision interval: Centre = %4d (%6.2f microsec)\n";
      txtw = "                       Half Width = %4d (%6.2f microsec)\n";
      interval = TRUE;
      break;
    case 2:
      ticks = options.l1_l1m_timeout;
      twid = 0;
      text = "L1 - L1m timeout        : Tiemout = %4d (%6.2f microsec)\n";
      txtw = NULL;
      interval = FALSE;
      break;
    case 3:
      ticks = options.l1_l2_timeout;
      twid = 0;
      text = "L1 - L2 timeout         : Timeout = %4d (%6.2f microsec)\n";
      txtw = NULL;
      interval = FALSE;
      break;
    case 4:
      ticks = options.bc_l1_l2_win;
      twid = options.bc_l1_l2_wid;
      text = "BC ID difference L1-L2 :   Centre = %4d (%6.2f microsec)\n";
      txtw = "                       Half Width = %4d (%6.2f microsec)\n"; 
      interval = TRUE;
      break;
    case 5:
      ticks = options.l1_roi_timeout;
      twid = 0;
      text = "L1 - RoI timeout        : Timeout = %4d (%6.2f microsec)\n";
      txtw = NULL;
      interval = FALSE; 
      break;
    case 6:
      ticks = options.bc_l1_roi_win;
      twid = options.bc_l1_roi_wid;
      text = "BC ID difference L1-RoI:   Centre = %4d (%6.2f microsec)\n";
      txtw = "                       Half width = %4d (%6.2f microsec)\n";
      interval = TRUE;
      break;
    default:
      stop_it = TRUE;
      break;
    }
    if(stop_it){
      break;
    }
    c = ((float)ticks) * time26ns;
    w = ((float)twid) * time26ns;
    if(interval){
      printf(text,ticks,c);
      printf(txtw,twid,w);
    }else{
      printf(text,ticks,c);
    }
  }

  printf(" \n");
  printf("Physics BC\'s    : %5lu   - %5lu \n",options.bc_fiz_min, 
	 options.bc_fiz_max);
  printf("Pre pulse BC     : %5lu \n",options.bc_pp);
  printf("Old L2 filter BC : %5lu  END OF\n",options.bc_l2_filter);
  printf("New L2 filter BC : %5lu  BEGINNING OF\n",options.bc_l2_fil2);

  printf(" ------------------------- \n");
}

void ttc_it_set_l0_signal(int type){

  switch(type){
  case 0:
    options.l0 = ABSENT;
    break;
  case 1:
    options.l0 = A_CHANNEL;
    break;
  case 2:
    options.l0 = WIRE;
    break;
  case 3:
#ifdef _TTCIT_L0_A_CHANNEL_ONLY__
    options.l0 = A_CHANNEL;           /* No wire L0 present */
    printf("There is no L0 wire present in the TTCit Logic FPGA\n");
#else
    options.l0 = A_WIRE;
#endif
    break;
  case 4:
    options.l0 = UNKNOWN;
  default:
    printf("If yu read this, you know there is a BUG!\n");
    break;
  }
}

/*
  This starts the on-line analysis tools
 */
int ttc_it_start_analyz(){
  int irc = 0;

  ttc_it_clear_counters(&counters);
  ttc_it_clear_errors(&errores);
  ttc_it_clear_timing(&timing);

  guts.start_time = time(NULL);
  guts.uptime = 0;
  guts.l0_seen = FALSE;
  guts.nBuffs = 0;

  options.low_l0l1 = options.l0_l1_win - options.l0_l1_wid;
  options.up_l0l1 = options.l0_l1_win + options.l0_l1_wid;

  options.low_bc = options.bc_l1_l2_win - options.bc_l1_l2_wid;
  options.up_bc = options.bc_l1_l2_win + options.bc_l1_l2_wid;

  options.low_bc_roi = options.bc_l1_roi_win - options.bc_l1_roi_wid;
  options.up_bc_roi = options.bc_l1_roi_win + options.bc_l1_roi_wid;

  return irc;
}

void ttc_it_finish_analyz(){

  /*
    Accumulate statistics
   */
  ttc_it_add_counters(&counters, &sum_count);
  ttc_it_add_errors(&errores, &sum_err);
  ttc_it_add_timing(&timing, &sum_time);

  /*
    Calculate mean values from accumulated data
   */
  timing.l0_l0_mean = ttc_it_mean_val(timing.l0_l0_mean, timing.l0_l0_n);
  timing.l0_l1_mean = ttc_it_mean_val(timing.l0_l1_mean, timing.l0_l1_n);
  timing.l1_l2_mean = ttc_it_mean_val(timing.l1_l2_mean, timing.l1_l2_n);
  timing.l1_l1m_mean = ttc_it_mean_val(timing.l1_l1m_mean, timing.l1_l1m_n);
  timing.bc_l1_l2_mean = ttc_it_mean_val(timing.bc_l1_l2_mean,
					 timing.bc_l1_l2_n);
  timing.l1_roi_mean = ttc_it_mean_val(timing.l1_roi_mean, timing.l1_roi_n);
  timing.bc_l1_roi_mean = ttc_it_mean_val(timing.bc_l1_roi_mean, 
					  timing.bc_l1_roi_n);

}

double ttc_it_mean_val(double val, double n){
  double retval;

  retval = (n > 0.) ? val/n : 0.;

  return retval;
}

/*
  Allocates all FIFO's
 */
int ttc_it_alloc_fifos(){
  int irc = 0;

  size_t siz;
  size_t nrObj;

  do{

    /*
      AB FIFO
     */
    siz = sizeof(struct ttc_it_A_B_channel);
    nrObj = MEGA_PLUS_ONE;
    irc = ttcit_mm_FIFO_create(&AB_fifo, siz, nrObj);
    CANNA_ALLOC_FIFO(AB_fifo,-1);

    /*
      BC FIFO:
    */

    siz = sizeof(unsigned long);
    nrObj = TTCIT_MAX_ADDR_BCFIFO + 1;
    irc = ttcit_mm_FIFO_create(&BC_fifo, siz, nrObj);
    CANNA_ALLOC_FIFO(BC_fifo,-2);

    /*
      L0-L1 FIFO:
     */
    siz = sizeof(struct ttc_it_seq_L0_L1);
    nrObj = ANALYZ_MAX_PEND_TRIG;
    irc = ttcit_mm_FIFO_create(&L0L1_fifo, siz, nrObj);
    CANNA_ALLOC_FIFO(L0L1_fifo,-3);

    /*
      L0-L1-L1m FIFO
     */
    siz = sizeof(struct ttc_it_seq_L0_L1_L1M);
    nrObj = ANALYZ_MAX_PEND_TRIG;
    irc = ttcit_mm_FIFO_create(&L0L1L1m_fifo, siz, nrObj);
    CANNA_ALLOC_FIFO(L0L1L1m_fifo,-4);

    /*
      L0-L1-L1m-L2 FIFO
     */
    siz = sizeof(struct ttc_it_seq_L0_L1_L1M_L2);
    nrObj = ANALYZ_MAX_PEND_TRIG;
    irc = ttcit_mm_FIFO_create(&L0L1L1mL2_fifo, siz, nrObj);
    CANNA_ALLOC_FIFO(L0L1L1mL2_fifo,-5);

    /*
      Error list
     */
    siz = sizeof(struct error_collection);
    nrObj = ANALYZ_MAX_ERR_LIST;
    irc = ttcit_mm_FIFO_create(&error_list, siz, nrObj);
    CANNA_ALLOC_FIFO(error_list,-6);

    /*
      L1 FIFO
     */
    siz = sizeof(struct ttc_it_seq_L1);
    nrObj = ANALYZ_MAX_PEND_TRIG;
    irc = ttcit_mm_FIFO_create(&L1_fifo, siz, nrObj);
    CANNA_ALLOC_FIFO(error_list,-7);

    /*
      RoI FIFO
     */
    siz = sizeof(struct ttc_it_seq_RoI);
    nrObj = ANALYZ_MAX_PEND_TRIG;
    irc = ttcit_mm_FIFO_create(&RoI_fifo, siz, nrObj);
    CANNA_ALLOC_FIFO(error_list,-8);

#ifdef _MONITOR_SCAN_BCID_DIFF__
    /*
      DirtyBCID FIFO
     */
    siz = sizeof(struct ttc_it_dirty_BCID);
    nrObj = TTCIT_MAX_DIRTY_BCID;
    irc = ttcit_mm_FIFO_create(&DirtyBCID_fifo, siz, nrObj);
    CANNA_ALLOC_FIFO(error_list,-9);
#endif

  }while(0);

  return irc;
}

/*
  Free all FIFO's
 */
void ttc_it_free_fifos(){
  ttcit_mm_FIFO_free(&AB_fifo);
  ttcit_mm_FIFO_free(&BC_fifo);
  ttcit_mm_FIFO_free(&L0L1_fifo);
  ttcit_mm_FIFO_free(&L0L1L1m_fifo);
  ttcit_mm_FIFO_free(&L0L1L1mL2_fifo);
  ttcit_mm_FIFO_free(&error_list);
  ttcit_mm_FIFO_free(&L1_fifo);
  ttcit_mm_FIFO_free(&RoI_fifo);
}

/*
  Empty all FIFO's
 */
void ttc_it_empty_fifos(){
  ttcit_mm_FIFO_empty(&AB_fifo);
  ttcit_mm_FIFO_empty(&BC_fifo);
  ttcit_mm_FIFO_empty(&L0L1_fifo);
  ttcit_mm_FIFO_empty(&L0L1L1m_fifo);
  ttcit_mm_FIFO_empty(&L0L1L1mL2_fifo);
  ttcit_mm_FIFO_empty(&error_list);
  ttcit_mm_FIFO_empty(&L1_fifo);
  ttcit_mm_FIFO_empty(&RoI_fifo);
}

void soft_monitor_empty_abbc_fifos(){
  ttcit_mm_FIFO_empty(&AB_fifo);
  ttcit_mm_FIFO_empty(&BC_fifo);
}

/*
  Set the SSM analyzer into initial state and make it ready to analyze 
  another Snap Shot
 */
void ttc_it_setstate_init(unsigned long SSmode){

  /*
    Clear error flags
   */
    ssm_analyz_errors.error = TTC_IT_ERR_NONE;
    ssm_analyz_errors.stop_on_error = FALSE;
    ssm_analyz_errors.err_case = 0;
    ssm_analyz_errors.err_info = 0;
    ssm_analyz_errors.err_tick = 0xbaddead;
    ssm_analyz_errors.stop_analysis = FALSE;

  /*
    Empty all FIFO's
   */
  ttc_it_empty_fifos();

  /*
    Set the TTCit state machine to initial state
   */
  maquina.virgin = TRUE;      /* We are starting the state machine */
  maquina.empty = FALSE;      /* AB FIFO has not yet been emptied */

#ifdef _TTCIT_L0_A_CHANNEL_ONLY__
  switch(options.l0){
  case A_CHANNEL:
  case A_WIRE:
    maquina.L0 = TRUE;
    break;
  case WIRE:             /* There is no L0 over WIRE, this is ignored */
  case ABSENT:
  case UNKNOWN:
  default:
    maquina.L0 = FALSE;
  }
#else
  switch(options.l0){
  case A_CHANNEL:
  case WIRE:
  case A_WIRE:
    maquina.L0 = TRUE;
    break;
  case ABSENT:
    maquina.L0 = FALSE;
    break;
  case UNKNOWN:
    maquina.L0 = FALSE;
    break;
  default:
    maquina.L0 = FALSE;
    break;
  }
#endif

  switch(SSmode){
  case TTCIT_IO_SSM_SCOPE:
    maquina.SSM_mode = TTCIT_IO_SSM_SCOPE;
    break;
  case TTCIT_IO_SSM_SEQ:
    maquina.SSM_mode = TTCIT_IO_SSM_SEQ;
    break;
  default:
    maquina.SSM_mode = TTCIT_IO_SSM_SEQ;  /* Better not to expect timing */
  }

  maquina.tick = -9999;
  maquina.l0_seen = 0;
  maquina.l1_seen = 0;

  /*
    Set the A channel state machine into starting position
   */
  maquina.A.state = EMPTY;   /* A channel machine is expecting L0 signal */

  maquina.A.virgin = TRUE;   /* A channel machine in state Virgin */
  maquina.A.empty = FALSE;   /* A channel FIFO not yet empty */

  maquina.A.L0_time = -999;
  maquina.A.L1_time = -999;
  maquina.A.BCID_L0 = -999;
  maquina.A.BCID_L1 = -999;

  maquina.A.L1_rising = FALSE;
  maquina.A.bcid_found = 0;

  maquina.A.tick_last_l0 = -999;
  maquina.A.tick_last_l1 = -999;
  maquina.A.bcid_last_l0 = -999;
  maquina.A.bcid_last_l1 = -999;

  /*
    Set the B channel state machine into starting position
   */

  maquina.B.strobe_state = INACTIVE;
  maquina.B.state = WAITING;
  maquina.B.group = G_NO;
  maquina.B.n_words = 0;
  maquina.B.n_words_received = 0;

  maquina.B.hdr = 0;

  maquina.B.l1f = FALSE;
  maquina.B.l2f = FALSE;
  maquina.B.roif = FALSE;

  maquina.B.tick_start = -999;
  maquina.B.tick_end = 999999;
}

/*
  This is out main alaysis tool that is supposed to work in on-line and 
  off-line

  Accumulate trigger statistics and errors

  All necessary data have already been stored in BC_fifo and AB_fifo
 */
int ttc_it_analyz_ssm(){
  int irc = 0;

  int ir;
  struct ttc_it_A_B_channel ab;

  int fifo_read;


  /*
    1-st read is made outside the loop
   */
  ir = ttcit_mm_FIFO_get(&AB_fifo, &ab, PICK);
  fifo_read = (ir > 0);                         /* If FALSE nothing done */

  guts.nBuffs++; /* Increment counter of inspected buffers */
  guts.l0_in_ssm = 0;
  guts.l1_in_ssm = 0;

  while(fifo_read){
    /*
      Feed A channel state machine 
     */
    ttc_it_A_machine(ab.ticks, ab.s);

    if((ssm_analyz_errors.error & TTC_IT_ERR_INTL) != 0){
      printf("ttc_it_analyz : INTERNAL ERROR IN A_MACHINE\n");
      break;
    }

    /*
      Feed B channel state machine 
     */
    ttc_it_B_machine(ab.ticks, ab.s);

    /*
      If we encountered an internal error, it we better stop
     */
    if((ssm_analyz_errors.error & TTC_IT_ERR_INTL) != 0){
      printf("ttc_it_analyz: INTERNAL ERROR IN A_B MACHINE\n");
      break;
    }
    /*
      If we reached some internal limitations - like insufficient
      memory for BC FIFO's, better stop processing this SSM, but not
      the MONITOR
     */
    if(ssm_analyz_errors.stop_analysis){
      break;
    }

    /*
      Read new ab from AB_fifo
     */
    ir = ttcit_mm_FIFO_get(&AB_fifo, &ab, PICK);
    fifo_read = (ir > 0);
  }

#ifdef TTCIT_EXTRA_DBG_PRINT_1__

  ttc_it_print_collections();

#endif

  /*
    Post processing 
   */
  ttc_it_AB_postprocess();

  /*
    Before exiting check the error flag
  */
  ssm_analyz_errors.stop_on_error = 
    ((ssm_analyz_errors.error & ssm_analyz_errors.err_mask) != 0);

  return irc;
}

void ttc_it_A_machine(int tick, unsigned long word){

  int irc;
  struct ttc_it_A_B_channel ab;
  int hit, diftick;
  enum ttc_it_A_channel_signal signal;
  int t;
  int once_more;
  unsigned long BCID;

  int tajmer;
  int valid;

  struct ttc_it_seq_L0_L1 item;
  struct error_collection ec;
  struct ttc_it_seq_L1 seq_l1;

  int st_time;
  int st_bcid;
  int st_dt, st_dbc;

  /*
    We can fully analyze the trigger sequences only as far as we have
    the BCID from BC FIFO available.

    If - especially in SEQ mode  - we exhaust all items from BC FIFO, we
    are no longer able to discriminate between L0 and L1 signals, and
    must terminate analysis loop, but NO ERROR is signalled, it is just a 
    limitation of the present HW + SW.

    Or maybe it is better not run SEQ mode when doing analysis.
  */

  do{
    /* 
       Signal recognition: L0 - 1 tick
       L1 - 2 ticks
			   
       At this momet we do not know which signal has arrived, but we may
       have a preview look at the next Object stred in AB_fifo and decide
       which signal has just arrive.

       If it is L1 we must instruct the A machine to ignore the next
       signal.
    */

    /* Is it a hit in A channel */
    hit = word & MASK_L1ACCEPT;
    if(hit == 0){                /* Not a hit in A channel, not our biznis */
      break;
    }

    maquina.tick = tick;

    /* If hit is just continuation of identified L1 signal, ignore it,
       since this L1 has already been processed */
    if(maquina.A.L1_rising){
      maquina.A.L1_rising = FALSE;
      break;
    }

    /* Is is L0 or L1? L1 must have a hit in the next word in AB_fifo */
    irc = ttcit_mm_FIFO_get(&AB_fifo, &ab, COPY); /* copy, don't remove */
    if(irc <= 0){  
      /*
	AB_fifo exhausted, cannot discriminate between L0 and L1
      */
      break;
    }
    hit = ab.s & MASK_L1ACCEPT;
    /*
      If the SSM is in the SEQ mode we cannot get the timing and we must
      have a look at the BC_fifo
    */
    switch(maquina.SSM_mode){
    case TTCIT_IO_SSM_SCOPE:      /* We have direct timing */
      diftick = ab.ticks - tick;
      break;
    case TTCIT_IO_SSM_SEQ:        /* SEQ shall not be used */
      printf("ttc_it_A_machine INTERNAL ERROR\n");
      ssm_analyz_errors.error |= TTC_IT_ERR_INTL;
      MARK_INTERNAL_ERROR(1);
      ssm_analyz_errors.err_case |= TTCIT_ERR_INTL_BUG;
      ssm_analyz_errors.err_info = -1;
      ssm_analyz_errors.err_tick = tick;
      ec.error = TTC_IT_ERR_INTL;
      ec.tick = tick;
      irc = ttcit_mm_FIFO_put(&error_list, &ec);
      diftick = 9999;
      ssm_analyz_errors.stop_analysis = TRUE;
      break;
    default:
      printf("ttc_it_A_machine INTERNAL ERROR\n");
      ssm_analyz_errors.error |= TTC_IT_ERR_INTL;
      MARK_INTERNAL_ERROR(2);
      ssm_analyz_errors.err_case |= TTCIT_ERR_INTL_BUG;
      ssm_analyz_errors.err_info = -1;
      ssm_analyz_errors.err_tick = tick;
      ec.error = TTC_IT_ERR_INTL;
      ec.tick = tick;
      irc = ttcit_mm_FIFO_put(&error_list, &ec);
      diftick = 9999;
      ssm_analyz_errors.stop_analysis = TRUE;
      break;
    }
    /*
      stop_analysis == TRUE : We are no longer able to discriminate 
      between L0 and L1, better stop it 
    */
    if(ssm_analyz_errors.stop_analysis){
      break;
    }

    /* L1 == 2 consecutive hits */
    maquina.A.L1_rising = ((hit != 0) && (diftick == 1));
    if(maquina.A.L1_rising){
      signal = SIGNAL_L1;
    }else{
      signal = SIGNAL_L0;
    }
    /*
      If the L0 is NOT sent over A channel, then the L1 signal is just 
      1 bit (not 2) and the L1 has just been identified as L0m while 
      L1 shall never be indetified

      Only L1 signal can be found in the A channel
     */
    if((options.l0 != A_CHANNEL) && (options.l0 != A_WIRE)){
      signal = SIGNAL_L1;
    }

    /*
      Update raw signal count, do not bother with correctness of L0-L1 seq.
    */
    switch(signal){
    case SIGNAL_L0:
      counters.l0++;
      guts.l0_seen = TRUE;
      guts.l0_in_ssm++;
      ttcit_mm_FIFO_empty(&L1_fifo); /* A fix to stupid thinko */
      maquina.l0_seen++;
      break;
    case SIGNAL_L1:
      counters.l1++;
      guts.l1_in_ssm++;
      /* Store it into L1_fifo to be used with RoI */
      seq_l1.l1_tick = tick;
      irc = ttcit_mm_FIFO_get(&BC_fifo, &BCID, COPY);
      seq_l1.l1_bcid = (irc > 0) ? BCID : 99999;
      seq_l1.used_l1m = FALSE;
      seq_l1.used_roi = FALSE;
      /*
	Not very clever way to store 2 integers, but some code may
	depend on this stupid choice
       */
      ttcit_mm_FIFO_empty(&L1_fifo);
      irc = ttcit_mm_FIFO_put(&L1_fifo, &seq_l1); /* Only 1 L1 */
      maquina.l1_seen++;
      break;
    }

    /* 
       We know what signal has just arrived we can process it
     

       Here start the A channel state machine 

    */
    t = tick;     /* we want to change them if needed */

    /*
      We want to accumulate the L0, L1 and BCID timing here
     */
    do{
      if(!maquina.L0){   /* No L0 signal, no timing */
	break;
      }
      if(signal == SIGNAL_L1){
	irc = ttcit_mm_FIFO_get(&BC_fifo, &BCID, COPY);
      }else{
	BCID = 0; /* Dummy stuff, no BC */
	irc = 1;
      }

      if(irc <= 0){   /* No BCID info - no need to bother */
	break;
      }
      st_bcid = BCID;
      switch(maquina.SSM_mode){
      case TTCIT_IO_SSM_SCOPE:
	st_time = tick;
	break;
      case TTCIT_IO_SSM_SEQ:   /* This is an error, should not be there */
	st_time = 0;
	break;
      }
      switch(signal){
      case SIGNAL_L0:
	if(maquina.A.tick_last_l0 == -999){
	  maquina.A.tick_last_l0 = st_time;
	  maquina.A.bcid_last_l0 = 0;
	}else{
	  st_dt = st_time - maquina.A.tick_last_l0;

	  timing.l0_l0_mean += (double)st_dt;
	  timing.l0_l0_n += 1.;
	  timing.l0_l0_min = (st_dt < timing.l0_l0_min) ? 
	    st_dt : timing.l0_l0_min;
	  timing.l0_l0_max = (st_dt > timing.l0_l0_max) ? 
	    st_dt : timing.l0_l0_max;

	  maquina.A.tick_last_l0 = st_time;
	  maquina.A.bcid_last_l0 = 0;
	}
	break;
      case SIGNAL_L1:
	if(maquina.A.tick_last_l1 == -999){
	  maquina.A.tick_last_l1 = st_time;
	  maquina.A.bcid_last_l1 = st_bcid;
	  if(maquina.A.tick_last_l0 != -999){
	    st_dt = st_time - maquina.A.tick_last_l0;
	    st_dbc = 0;                               /* BS */

	    timing.l0_l1_mean += (double)st_dt;
	    timing.l0_l1_n += 1.;
	    timing.l0_l1_min = ((unsigned long)st_dt < timing.l0_l1_min) ? 
	      st_dt : timing.l0_l1_min;
	    timing.l0_l1_max = ((unsigned long)st_dt > timing.l0_l1_max) ? 
	      st_dt : timing.l0_l1_max;
	  }
	}else{
	  if(maquina.A.tick_last_l0 != -999){

	    st_dt = st_time - maquina.A.tick_last_l0;
	    st_dbc = 0;

	    timing.l0_l1_mean += (double)st_dt;
	    timing.l0_l1_n += 1.;
	    timing.l0_l1_min = ((unsigned long)st_dt < timing.l0_l1_min) ? 
	      st_dt : timing.l0_l1_min;
	    timing.l0_l1_max = ((unsigned long)st_dt > timing.l0_l1_max) ? 
	      st_dt : timing.l0_l1_max;
	  }
	  maquina.A.tick_last_l1 = st_time;
	  maquina.A.bcid_last_l1 = st_bcid;
	}
	break;
      }
    }while(0);

    do{
      once_more = FALSE;
      switch(maquina.A.state){
      case EMPTY:
	switch(signal){
	case SIGNAL_L0:
	  maquina.A.bcid_found++;
	  maquina.A.BCID_L0 = 0;
	  maquina.A.L0_time = t;
	  maquina.A.state = L0;
	  break;
	case SIGNAL_L1:
	  /* 
	     If no L0 is transmitted we should generate it and 
	     rerun the machine with fake L0 signal
	  */
	  if(!maquina.L0){
	    /* As if L0 has been properly received in correct time */
	    maquina.A.L0_time = t - options.l0_l1_win;
	    maquina.A.BCID_L0 = 0;
	    /* And pretend we received normal L0 signal */
	    maquina.A.state = L0;
	    once_more = TRUE;
	    maquina.A.virgin = FALSE;  /* we do not use it in this case */
	    break;
	  }

	  /*
	    If this is the 1-st hit, the corresponding L0 signal may have 
	    been lost, so we just skip this and pretend that nothing has 
	    happened. 
	  */
	  if(maquina.A.virgin){
	    /*
	      Virgin state cannot last longer then upper limit for the
	      L0-L1 window.
	    */
	    if(tick > -1){  /* Virgin can lose only L0 before 1st hit */
	      maquina.A.virgin = FALSE;
	      once_more = TRUE;  /* Try it again, and trigger an error */
	      break;
	    }
	    /*
	      Looks like truly virgin state, ignore L1, drop its BCID
	      from BC_fifo and set state to EMPTY
	    */
	    irc = ttcit_mm_FIFO_get(&BC_fifo, NULL, DELETE);
	    if(irc <= 0){
	      if(maquina.A.bcid_found < TTCIT_MAX_ADDR_BCFIFO){
		printf("ttc_it_A_machine: INTERNAL ERROR: Not enough BCs\n");
		ssm_analyz_errors.error |= TTC_IT_ERR_INTL;
		MARK_INTERNAL_ERROR(3);
		ssm_analyz_errors.err_case |= TTCIT_ERR_INTL_NOTBC;
		ec.error = TTC_IT_ERR_INTL;
	      }else{
		ssm_analyz_errors.stop_analysis = TRUE;
		ec.error = TTC_IT_STOP_AN;
	      }
	      ssm_analyz_errors.err_info = maquina.A.bcid_found;
	      ssm_analyz_errors.err_tick = tick;
	      ec.tick = tick;
	      irc = ttcit_mm_FIFO_put(&error_list, &ec);
	      break;
	    }
	    maquina.A.bcid_found++;
	    break;   /* try next hit */
	  }else{                                       /* Not virgin, L1S */
	    errores.l1s++; /* Surplus L1 */
	    ssm_analyz_errors.error |= TTC_IT_ERR_L1S;
	    ssm_analyz_errors.err_tick = tick;
	    ec.error = TTC_IT_ERR_L1S;
	    ec.tick = tick;
	    irc = ttcit_mm_FIFO_put(&error_list, &ec);
	    /* Drop BCID from BC_fifo */
	    irc = ttcit_mm_FIFO_get(&BC_fifo, NULL, DELETE);
	    if(irc <= 0){
	      if(maquina.A.bcid_found < TTCIT_MAX_ADDR_BCFIFO){
		printf("ttc_it_A_machine: INTERNAL ERROR: Not enough BCs\n");
		ssm_analyz_errors.error |= TTC_IT_ERR_INTL;
		MARK_INTERNAL_ERROR(4);
		ssm_analyz_errors.err_case |= TTCIT_ERR_INTL_NOTBC;
		ec.error = TTC_IT_ERR_INTL;
	      }else{
		ssm_analyz_errors.stop_analysis = TRUE;
		ec.error = TTC_IT_STOP_AN;
	      }
	      ssm_analyz_errors.err_info = maquina.A.bcid_found;
	      ssm_analyz_errors.err_tick = tick;
	      ec.tick = tick;
	      irc = ttcit_mm_FIFO_put(&error_list, &ec);
	      break;
	    }
	    maquina.A.bcid_found++;
            /*
	      Internal error or requested stop: does not matter which state
	      the A machine is ON
	    */
	    maquina.A.state = EMPTY;
	  }
	  break;
	}  
	break;              /* ---> End CASE EMPTY : switch(signal) */
      case L0:
	switch(signal){
	case SIGNAL_L0:
	  /* 
	     If L0 arrived inside L0 - L1 decision window -> L0S error

	     If later then upper limit of the L0-L1 window, it is just 
	     L1 reject case
	  */
	  if((t - maquina.A.L0_time) > options.up_l0l1){  /* L1 reject */
	    counters.l1_reject++;
	    maquina.A.state = EMPTY;
	    once_more = TRUE;
	  }else{  /* L0S error */
	    errores.l0s++;
	    ssm_analyz_errors.error |= TTC_IT_ERR_L0S;
	    ssm_analyz_errors.err_info = tick;
	    ec.error = TTC_IT_ERR_L0S;
	    ec.tick = tick;
	    irc = ttcit_mm_FIFO_put(&error_list, &ec);
	    maquina.A.state = EMPTY;
	  }
	  break;
	case SIGNAL_L1:
	  irc = ttcit_mm_FIFO_get(&BC_fifo, &BCID, PICK); /* Use the 1-st */
	  if(irc <= 0){
#ifdef _IGNORE_INTERNAL_ERR_5
	    BCID = 0xfff;        /* I wnat to see it */
#else
	    BCID = 0;    /* This should not be !!! */
	    if(maquina.A.bcid_found < TTCIT_MAX_ADDR_BCFIFO){
	      ssm_analyz_errors.error |= TTC_IT_ERR_INTL;
	      MARK_INTERNAL_ERROR(5);
	      ssm_analyz_errors.err_case |= TTCIT_ERR_INTL_NOTBC;
	      ec.error = TTC_IT_ERR_INTL;
	    }else{
	      ssm_analyz_errors.stop_analysis = TRUE;
	      ec.error = TTC_IT_STOP_AN;
	    }
	    ssm_analyz_errors.err_info = maquina.A.bcid_found;
	    ssm_analyz_errors.err_tick = tick;
	    ec.tick = tick;
	    irc = ttcit_mm_FIFO_put(&error_list, &ec);
	    break;
#endif
	  }
	  maquina.A.bcid_found++;
	  maquina.A.BCID_L1 = (int)BCID;
	  maquina.A.L1_time = t;
	  /*
	    Is it a valid L0 - L1 sequence ?
	  */
	  tajmer = maquina.A.L1_time - maquina.A.L0_time;
	  valid = ((tajmer >= options.low_l0l1) && 
		   (tajmer <= options.up_l0l1));

	  if(valid){   /* Valid L0 - L1 accepted sequence */

	    /*
	      Update counters, add timer statistics 
	    */

	    /*
	      Store L0 - L1 pair inna pipeline
	    */
	    item.tick_l0 = maquina.A.L0_time;
	    item.tick_l1 = maquina.A.L1_time;
	    item.bcid_l0 = maquina.A.BCID_L0;
	    item.bcid_l1 = maquina.A.BCID_L1;
	    item.fake = FALSE;                  /* True l0-l1 sequence */
	    irc = ttcit_mm_FIFO_put(&L0L1_fifo, &item);
	    if(irc <= 0){
	      ssm_analyz_errors.error |= TTC_IT_ERR_INTL;
	      MARK_INTERNAL_ERROR(6);
	      ssm_analyz_errors.err_case |= TTCIT_ERR_INTL_L0L1;
	      ssm_analyz_errors.err_info = L0L1_fifo.infifo;
	      ssm_analyz_errors.err_tick = tick;
	      ec.error = TTC_IT_ERR_INTL;
	      ec.tick = tick;
	      irc = ttcit_mm_FIFO_put(&error_list, &ec);
	      break;
	    }

	    /*
	      Set A channel machine state to EMPTY
	    */
	    maquina.A.state = EMPTY;

	  }else{       /* L1T time violation error */
	    errores.l1t++;
	    ssm_analyz_errors.error |= TTC_IT_ERR_L1T;  /* set err flag */
	    ssm_analyz_errors.err_info = tick;
	    ec.error = TTC_IT_ERR_L1T;
	    ec.tick = tick;
	    irc = ttcit_mm_FIFO_put(&error_list, &ec);
	    /*
	      This L0-L1 sequence is not stored into pipeline, since it
	      should be interpreted as L1 rejected trigger 
	    */
	    counters.l1_reject++;

	    /*
	      Set the A channel machine to EMPTY, change if needed
	    */
	    maquina.A.state = EMPTY;
	  }
	  break;
	}            /* End of CASE L0 : */
      }                 /* --> end switch(mawuina.A.state} */
    }while(once_more);

  }while(0);
}

void ttc_it_B_machine(int tick, unsigned long word){

  struct error_collection ec;
  int skip_processing;
  int irc;

  enum data_strobe_signal read_control;
  unsigned long TTC_address;
  unsigned long masked;
  enum ttc_it_B_groups new_group;
  enum ttc_it_B_subtypes subtype;
  enum ttc_it_B_ferrors format_err;

  int once_more;
  int i;
  int dum_dat;
  void *vptr;

  skip_processing = FALSE;

  do{

    /*
      We must react only on Data Strobe going from LOW -> HIGH transition
      (i.e. reading only during the 1-st tick with Data Strobe HIGH
    */

    read_control = ((word & MASK_DOUTSTR) != 0) ? HIGH : LOW;
    switch(maquina.B.strobe_state){
    case INACTIVE:
      switch(read_control){
      case LOW:                             /* Nothing in B channel */
	maquina.B.strobe_state = INACTIVE;
	skip_processing = TRUE;
	break;
      case HIGH:
	maquina.B.strobe_state = READING;   /* Valid data in B channel */
	skip_processing = FALSE;
	break;
      }
      break;
    case READING:
      switch(read_control){
      case LOW:
	maquina.B.strobe_state = INACTIVE;   /* B channel empty */
	skip_processing = TRUE;
	break;
      case HIGH:
	maquina.B.strobe_state = READING;   /* Already processed data */
	skip_processing = TRUE;
	break;
      }
      break;
    default:
      printf("ttc_it_B_machine: Internal STROBE BUG!!!\n");
      ssm_analyz_errors.error |= TTC_IT_ERR_INTL;
      MARK_INTERNAL_ERROR(7);
      ssm_analyz_errors.err_case |= TTCIT_ERR_INTL_DOUT;
      skip_processing = TRUE;
      ec.error = TTC_IT_ERR_INTL;
      ec.tick = tick;
      irc = ttcit_mm_FIFO_put(&error_list, &ec);
      break;
    }
    if(skip_processing){
      break;
    }

    maquina.tick = tick;

    /*
      Here we have new data in B channel, determine the data type

      TTC Address = { subtype, group }
    */
    TTC_address = word & TTC_ADDR_MASK;
    masked = word & TTC_DATA_MASK;
    switch(TTC_address){
    case TTC_ADDR_L1_H:
      subtype = T_HDR;
      new_group = G_L1M;
      counters.l1m++;
      break;
    case TTC_ADDR_L1_D:
      subtype = T_DAT;
      new_group = G_L1M;
      break;
    case TTC_ADDR_L2A_H:
      subtype = T_HDR;
      new_group = G_L2A;
      counters.l2a++;
      break;
    case TTC_ADDR_L2A_D:
      subtype = T_DAT;
      new_group = G_L2A;
      break;
    case TTC_ADDR_L2R:
      subtype = T_HDR;
      new_group = G_L2R;
      counters.l2r++;
      break;
      break;
    case TTC_ADDR_ROI_H:
      subtype = T_HDR;
      new_group = G_ROI;
      counters.roi++;
      break;
    case TTC_ADDR_ROI_D:
      subtype = T_DAT;
      new_group = G_ROI;
      break;
    default:
      subtype = T_RES;
      new_group = G_RES;
      break;
    }

    /*
      Evaluate the B channel state machine
    */
    once_more = FALSE;
    do{
      once_more = FALSE;             /* Avoid infinite loops */
      switch(maquina.B.state){
      case WAITING:
	switch(subtype){
	case T_HDR:              /* New header O.K. */
	  maquina.B.state = HEADER;
	  maquina.B.group = new_group;
	  maquina.B.n_words_received = 0;
	  maquina.B.tick_start = tick;
	  maquina.B.format_error = FALSE;
	  maquina.B.l1f = FALSE;
	  maquina.B.l2f = FALSE;
	  maquina.B.roif = FALSE;
	  switch(new_group){
	  case G_L1M:
	    maquina.B.n_words = TTC_WORDS_L1M;
	    break;
	  case G_L2A:
	    maquina.B.n_words = TTC_WORDS_L2A;
	    break;
	  case G_L2R:
	    maquina.B.n_words = TTC_WORDS_L2R;
	    break;
	  case G_ROI:
	    maquina.B.n_words = TTC_WORDS_ROI;
	    break;
	  case G_RES:
	  case G_NO:
	    maquina.B.n_words = TTC_WORDS_RES;  /* This should not be reached */
	    break;
	  }
	  maquina.B.hdr =(int)masked;
	  if(maquina.B.group == G_L2R){  /* L2R, just assemble and wait */
	    maquina.B.tick_end = tick;
	    irc = ttc_it_assemble_seq(); /* Error is handled by assembly */
	    maquina.B.state = WAITING;
	  }else{
	    maquina.B.state = HEADER;
	  }
	  break;
	case T_DAT:              /* Data? Where is Header? Format errro */
	  /*
	    To keep the data flow we must:

	    1) Instert fake header
	    2) Raise the format error
	    3) Set the B machine into state HEADER
	    3) reprocess the signal
	   */
	  switch(new_group){
	  case G_L1M:
	    format_err = FER_L1F;
	    maquina.B.l1f = TRUE;
	    maquina.B.n_words = TTC_WORDS_L1M;
	    maquina.B.hdr = TTC_ADDR_L1_H;
	    ttc_it_B_format_error(format_err);
	    break;
	  case G_L2A:
	    format_err = FER_L2F;
	    maquina.B.state = WAITING;
	    maquina.B.l2f = TRUE;
	    maquina.B.n_words = TTC_WORDS_L2A;
	    maquina.B.hdr = TTC_ADDR_L2A_H;
	    ttc_it_B_format_error(format_err);
	    break;
	  case G_ROI: 
	    format_err = FER_ROIF;
	    maquina.B.roif = TRUE;
	    maquina.B.n_words = TTC_WORDS_ROI;
	    maquina.B.hdr = TTC_ADDR_ROI_H;
	    ttc_it_B_format_error(format_err);
	    break;
	  default:
	    /*
	      Do not know what to do here, reset to WAITING, ignore
	    */
	    maquina.B.state = WAITING;
	    skip_processing = TRUE;
	    break;
	  }
	  maquina.B.format_error = TRUE;
	  maquina.B.state = HEADER;
	  maquina.B.group = new_group;
	  maquina.B.n_words_received = 0;
	  maquina.B.tick_start = tick;
	  once_more = TRUE;
	  break;
	case T_RES:              /* Reserved, just ignore it */
	  maquina.B.state = WAITING;
	  skip_processing = TRUE;
	  break;
	}
	break;
      case HEADER:
	switch(subtype){
	case T_HDR:             /* Header should not follow after header */
	  /*
	    We raise the errror for the already started sequence, but
	    start the new one. The 1-st one is surely wrong, but the new one
	    can be O.K.
	  */

	  /*
	    To keep the data flow going we must:

	    1) Finish existing sequence with dummy data
	    2) Flag the sequence as format error
	    3) Assemble the dummy sequence
	    4) Start a new sequence
	  */
	  maquina.B.swap = maquina.tick;
	  maquina.tick = maquina.B.tick_start;
	  switch(maquina.B.group){
	  case G_L1M:
	    format_err = FER_L1F;
	    ttc_it_B_format_error(format_err);
	    maquina.B.l1f = TRUE;
	    maquina.B.words[0] = TTC_ADDR_L1_D;
	    maquina.B.n_words = TTC_WORDS_L1M;
	    break;
	  case G_L2A:
	    format_err = FER_L2F;
	    ttc_it_B_format_error(format_err);
	    maquina.B.l2f = TRUE;
	    maquina.B.words[0] = TTC_ADDR_L2A_D;
	    maquina.B.n_words = TTC_WORDS_L2A;
	    break;
	  case G_L2R:    /* We should never happed to be there */
	    break;
	  case G_ROI: 
	    format_err = FER_ROIF;
	    ttc_it_B_format_error(format_err);
	    maquina.B.roif = TRUE;
	    maquina.B.words[0] = TTC_ADDR_ROI_D;
	    maquina.B.n_words = TTC_WORDS_ROI;
	    break;
	  default:
	    break;
	  }
	  maquina.tick = maquina.B.swap;
	  maquina.B.format_error = TRUE;
	  for(i=1; i<20; i++){
	    maquina.B.words[i] = maquina.B.words[0];
	  }
	  maquina.B.n_words_received = maquina.B.n_words;
	  irc = ttc_it_assemble_seq();

	  /*
	    Start new sequence
	  */
	  maquina.B.state = WAITING;
	  once_more = TRUE;

	  break;
	case T_DAT:             /* Data after header, O.K., check GROUP */
	  /*
	    If groups do not agree, we have a format error
	  */
	  if(maquina.B.group != new_group){
	    switch(maquina.B.group){
	    case G_L1M:
	      format_err = FER_L1F;
	      ttc_it_B_format_error(format_err);
	      maquina.B.l1f = TRUE;
	      dum_dat = TTC_ADDR_L1_D;
	      break;
	    case G_L2A:
	      format_err = FER_L2F;
	      ttc_it_B_format_error(format_err);
	      maquina.B.l2f = TRUE;
	      dum_dat = TTC_ADDR_L2A_D;
	      break;
	    case G_ROI:   
	      format_err = FER_ROIF;
	      ttc_it_B_format_error(format_err);
	      maquina.B.roif = TRUE;
	      dum_dat = TTC_ADDR_ROI_D;
	      break;
	    default:
	      break;
	    }
	    /*
	      1) Fill the rest of the sequence with dummy data
	      2) Assemble sequence

	     */
	    maquina.B.format_error = TRUE;
	    for(i = maquina.B.n_words_received;
		i < maquina.B.n_words; i++){
	      maquina.B.words[i] = dum_dat;
	    }
	    maquina.B.n_words_received = maquina.B.n_words;
	    irc = ttc_it_assemble_seq();

	    /*
	      Let us try to reset the group and process it as a new group
	     */
	    maquina.B.group = new_group;
	    once_more = TRUE;
	    maquina.B.state = WAITING;
	    break;
	  }

	  /*
	    Groups agree, start reading data
	  */
	  maquina.B.words[maquina.B.n_words_received++] = (int)masked;
	  maquina.B.tick_end = tick;
	  if(maquina.B.n_words_received == maquina.B.n_words){ /* Finish it */
	    irc = ttc_it_assemble_seq();
	    maquina.B.state = WAITING;
	  }else{
	    maquina.B.state = WORDS;
	  }
	  break;
	case T_RES:             /* Reserved, should not be here */
	  switch(maquina.B.group){
	  case G_L1M:
	    format_err = FER_L1F;
	    ttc_it_B_format_error(format_err);
	    maquina.B.state = WAITING;
	    maquina.B.l1f = TRUE;
	    dum_dat = TTC_ADDR_L1_D;
	    break;
	  case G_L2A:
	    format_err = FER_L2F;
	    ttc_it_B_format_error(format_err);
	    maquina.B.state = WAITING;
	    maquina.B.l2f = TRUE;
	    dum_dat = TTC_ADDR_L2A_D;
	    break;
	  case G_ROI:           
	    format_err = FER_ROIF;
	    ttc_it_B_format_error(format_err);
	    maquina.B.state = WAITING;
	    maquina.B.roif = TRUE;
	    dum_dat = TTC_ADDR_ROI_D;
	    break;
	  default: 
	    maquina.B.state = WAITING;
	    break;
	  }
	  /*
	    Fill the rest of the sequence with dummy data and assemble the 
	    sequence
	   */
	  maquina.B.format_error = TRUE;
	  for(i = maquina.B.n_words_received;
	      i < maquina.B.n_words; i++){
	    maquina.B.words[i] = dum_dat;
	  }
	  maquina.B.n_words_received = maquina.B.n_words;
	  irc = ttc_it_assemble_seq();
	  maquina.B.state = WAITING;

	  break;
	}
	break;
      case WORDS:
	switch(subtype){
	case T_HDR:            /* This is no place for hedear - ERROR */
	  /*
	    If the last message has collected all words we can appear 
	    to be here
	   */
	  if(maquina.B.n_words_received == maquina.B.n_words){
	    maquina.B.state = WAITING;
	    once_more = TRUE;
	    maquina.B.n_words_received = 0;
	    break;
	  }
	  maquina.B.swap = maquina.tick;
	  maquina.tick = maquina.B.tick_end;
	  switch(maquina.B.group){
	  case G_L1M:
	    format_err = FER_L1F;
	    ttc_it_B_format_error(format_err);
	    maquina.B.l1f = TRUE;
	    dum_dat = TTC_ADDR_L1_D;
	    break;
	  case G_L2A:
	    format_err = FER_L2F;
	    ttc_it_B_format_error(format_err);
	    maquina.B.l2f = TRUE;
	    dum_dat = TTC_ADDR_L2A_D;
	    break;
	  case G_ROI:    
	    format_err = FER_ROIF;
	    ttc_it_B_format_error(format_err);
	    maquina.B.roif = TRUE;
	    dum_dat = TTC_ADDR_ROI_D;
	    break;
	  default:      /* I should not be there */
	    break;
	  }
	  maquina.tick = maquina.B.swap;
	  maquina.B.format_error = TRUE;
	  for( i = maquina.B.n_words_received; 
	       i < maquina.B.n_words; i++){
	    maquina.B.words[i] = dum_dat;
	  }
	  maquina.B.n_words_received = maquina.B.n_words;
	  irc = ttc_it_assemble_seq();
	  /*
	    Stop processing the errorneous sequence, start new one
	    reprocess
	   */
	  once_more = TRUE;
	  maquina.B.state = WAITING;
	  break;
	case T_DAT:            /* Data can be here, but not too many */
	  /*
	    Groups must agree, if not raise an error

	    1) Flag format error
	    2) Fill the rest of the sequence with dummy data
	    3) Assemble the sequence
	  */
	  if(maquina.B.group != new_group){
	    if(maquina.B.n_words_received == maquina.B.n_words){
	      maquina.B.state = WAITING; /* All words read */
	      once_more = TRUE;
	      maquina.B.n_words_received = 0;
	      break;
	    }else{                      /* Not all words read - error */
	      switch(maquina.B.group){
	      case G_L1M:
		format_err = FER_L1F;
		ttc_it_B_format_error(format_err);
		maquina.B.l1f = TRUE;
		dum_dat = TTC_ADDR_L1_D;
		break;
	      case L2A:
		format_err = FER_L2F;
		ttc_it_B_format_error(format_err);
		maquina.B.l2f = TRUE;
		dum_dat = TTC_ADDR_L2A_D;
		break;
	      case G_ROI:     
		format_err = FER_ROIF;
		ttc_it_B_format_error(format_err);
		maquina.B.roif = TRUE;
		dum_dat = TTC_ADDR_ROI_D;
		break;
	      default:          /* Do not know what to do here */
		break;
	      }
	      maquina.B.format_error = TRUE;
	      for(i = maquina.B.n_words_received; 
		  i < maquina.B.n_words; i++){
		maquina.B.words[i] = dum_dat;
	      }
	      maquina.B.n_words_received = maquina.B.n_words;
	      irc = ttc_it_assemble_seq();
	      once_more = TRUE;
	      maquina.B.state = WAITING;
	      break;
	    }
	  }

	  /*
	    Accumulate words, if the whole sequence until all words are
	    collected
	  */
	  maquina.B.words[maquina.B.n_words_received++] = (int)masked;
	  maquina.B.tick_end = tick;
	  if(maquina.B.n_words_received == maquina.B.n_words){
	    irc = ttc_it_assemble_seq();
	    maquina.B.state = WORDS;
	  }else{
	    if(maquina.B.n_words_received > maquina.B.n_words){
	      /*
		Reading more words than expected
	       */
	      switch(maquina.B.group){
	      case G_L1M:
		format_err = FER_L1F;
		ttc_it_B_format_error(format_err);
		maquina.B.l1f = TRUE;
		vptr = ttcit_mm_FIFO_get_addr(&L0L1L1m_fifo);
		if(vptr != NULL){
		  ((struct ttc_it_seq_L0_L1_L1M *)vptr)->l1f = TRUE;
		}
		break;
	      case G_L2A:
		format_err = FER_L2F;
		ttc_it_B_format_error(format_err);
		maquina.B.l2f = TRUE;
		vptr = ttcit_mm_FIFO_get_addr(&L0L1L1mL2_fifo);
		if(vptr != NULL){
		  ((struct ttc_it_seq_L0_L1_L1M_L2 *)vptr)->l2f = TRUE;
		}
		break;
	      case G_ROI:
		format_err = FER_ROIF;
		ttc_it_B_format_error(format_err);
		maquina.B.roif = TRUE;
		break;
	      default:
		break;
	      }
	      maquina.B.format_error = TRUE;
	      /*
		Pretend the last word has never happened
	       */
	      maquina.B.n_words_received = maquina.B.n_words;
	    }
	    maquina.B.state = WORDS;
	  }
	  break;
	case T_RES:            /* This is an ERROR, Reserved  */
	  /*
	    Not an error if we have already read all our data words
	   */
	  if(maquina.B.n_words_received == maquina.B.n_words){
	    maquina.B.state = WAITING;
	    once_more = TRUE;
	    maquina.B.n_words_received = 0;
	    break;
	  }
	  /*
	    1) Flag the format error
	    2) Fill the rest of sequence with dummy data
	    3) assemble the sequence 
	    4) Try it once more time
	   */
	  switch(maquina.B.group){
	  case G_L1M:
	    format_err = FER_L1F;
	    ttc_it_B_format_error(format_err);
	    maquina.B.l1f = TRUE;
	    dum_dat = TTC_ADDR_L1_D;
	    break;
	  case G_L2A:
	    format_err = FER_L2F;
	    ttc_it_B_format_error(format_err);
	    maquina.B.l2f = TRUE;
	    dum_dat = TTC_ADDR_L2A_D;
	    break;
	  case G_ROI:  
	    format_err = FER_ROIF;
	    ttc_it_B_format_error(format_err);
	    maquina.B.roif = TRUE;
	    dum_dat = TTC_ADDR_ROI_D;
	    break;
	  default:              /* Do not know what to do here */
	    break;
	  }
	  maquina.B.format_error = TRUE;
	  for(i = maquina.B.n_words_received;
	      i < maquina.B.n_words; i++){
	    maquina.B.words[i] = dum_dat;
	  }
	  maquina.B.n_words_received = maquina.B.n_words;
	  irc = ttc_it_assemble_seq();

	  maquina.B.state = WAITING;
	  maquina.B.group = new_group;
	  once_more = TRUE;
	  break;
	}
	break;
      }
    }while(once_more);

    if(skip_processing){
      break;
    }

  }while(0);
}

int ttc_it_assemble_seq(){
  int irc = 0;

  struct ttcit_mm_FIFO *fifo = NULL;
  void *Obj = NULL;

  struct ttc_it_seq_L0_L1 L_01;
  struct ttc_it_seq_L0_L1_L1M L_01M;
  struct ttc_it_seq_L0_L1_L1M_L2 L_012;
  struct ttc_it_seq_L1 L_1;
  struct ttc_it_seq_L1 *L1_ptr;
  struct ttc_it_seq_RoI RoI;
  int fifo_irc, fifo_error;
  int j;
  int i;

  int timeout, bc1, bc2, dbc;
  int old_l2;

  size_t siz;

  struct error_collection ec;

  int do_nothing = FALSE;
  int once_more;
  int l2t_already_found;

#ifdef _MONITOR_SCAN_BCID_DIFF__
  struct ttc_it_dirty_BCID bcd_dirt;
  int irc_bcd_dirt;
#endif

  l2t_already_found = FALSE;

  do{

    do_nothing = FALSE;
    once_more = FALSE;

    switch(maquina.B.group){
    case G_L1M:                         /* Store L0-L1-L1m  item */
      /*
	Get the oldest L0-L1 pair, and add L1m to them
       */
      fifo_irc = ttcit_mm_FIFO_get(&L0L1_fifo, &L_01, PICK);
      fifo_error = (fifo_irc <= 0);
      /*
	If we had lost the 1-st L0 there is no L0-L1 in L0L1_fifo, but
	the L1 message follows L1 and L2a/L2r is to be expected.

	Create sequence, but flag it as fake
       */
      if(fifo_error){
	fifo_irc = ttcit_mm_FIFO_get(&L1_fifo, &L_1, COPY);
	if(fifo_irc > 0){
	  /*
	    Create fake L_01 structure, pretend that L0 is here
	  */
	  L1_ptr = (struct ttc_it_seq_L1 *) ttcit_mm_FIFO_get_addr(&L1_fifo);

	  fifo_error = (L1_ptr == NULL);
	  if(!fifo_error){
	    fifo_error = L1_ptr->used_l1m;
	  }

	  if(!fifo_error){
	    L1_ptr->used_l1m = TRUE; /* Flag it as used, don't use it again */

	    L_01.tick_l0 = L_1.l1_tick - options.l0_l1_win;
	    L_01.tick_l1 = L_1.l1_tick;
	    L_01.bcid_l0 = L_1.l1_bcid - options.l0_l1_win;
	    L_01.bcid_l1 = L_1.l1_bcid;
	    L_01.fake = TRUE;           /* Artificial sequence, flag it */
	    counters.l0l1_fakes++;
	    fifo_error = FALSE;
	  }
	}
	/*
	  1-st L0 cannot be lost, if no L0 is sent over A channel
	*/
	fifo_error = fifo_error || (!maquina.L0);
	/*
	  1-st L0 cannot be lost if the 1-st L0 and L1 have already been seen
	*/
	fifo_error = ( fifo_error || (maquina.l0_seen != 0) || 
		       (maquina.l1_seen != 1) );
      }


      if(fifo_error){  /* L1M error */
	errores.l1m++;
	ssm_analyz_errors.error |= TTC_IT_ERR_L1M;
	ssm_analyz_errors.err_tick = maquina.tick;
	ec.error = TTC_IT_ERR_L1M;
	ec.tick = maquina.tick;
	j = ttcit_mm_FIFO_put(&error_list, &ec);
	do_nothing = TRUE;
	break;
      }

      siz = sizeof(struct ttc_it_seq_L0_L1);
      memcpy(&L_01M.l0_l1, &L_01, siz);
      L_01M.tick_l1m = maquina.B.tick_start;
      L_01M.nw = maquina.B.n_words_received;
      L_01M.header = maquina.B.hdr;
      for(i=0; i<4; i++){
	L_01M.data[i] = maquina.B.words[i];
      }
      L_01M.l1f = (maquina.B.format_error && maquina.B.l1f);

      /* 
	 Check for L1Mo error: L1M trasmitted outside programmable 
	 window
       */
      timeout = L_01M.tick_l1m - L_01M.l0_l1.tick_l1;

      timing.l1_l1m_mean += (double)timeout;
      timing.l1_l1m_n += 1.;
      timing.l1_l1m_min = ((unsigned long)timeout < timing.l1_l1m_min) ? 
	timeout : timing.l1_l1m_min;
      timing.l1_l1m_max = ((unsigned long)timeout > timing.l1_l1m_max) ? 
	timeout : timing.l1_l1m_max;

      if(timeout > options.l1_l1m_timeout){
	errores.l1mo++;
	ssm_analyz_errors.error |= TTC_IT_ERR_L1MO;
	ssm_analyz_errors.err_tick = maquina.tick;
	ec.error = TTC_IT_ERR_L1MO;
	ec.tick = maquina.tick;
	j = ttcit_mm_FIFO_put(&error_list, &ec);
	do_nothing = TRUE;
	break;
      }

      fifo = &L0L1L1m_fifo;
      Obj = &L_01M;
      break;
    case G_L2A:                         /* Store L0-L1-L1m-L2a item */
    case G_L2R:                         /* Store L0-L1-L1m-L2r item */
      /*
	Extract the oldest L0-L1-L1m sequence and add L2 to them
       */
      fifo_irc = ttcit_mm_FIFO_get(&L0L1L1m_fifo, &L_01M, PICK);
    
      fifo_error = (fifo_irc <= 0);

      /*
	If only the 1-st L0 arrived and just received L2a/L2r may belong
	to the trigger that started before we started filling the SSM.

	This is not an error, just forget about the L2a/L2r

	This may happen only for the case: 1 L0, 0 L1.
       */
      if(fifo_error){
	if((guts.l0_in_ssm == 1) && (guts.l1_in_ssm == 0)){
	  fifo_error = FALSE;
	  do_nothing = TRUE;
	  break;
	}
      }

      if(fifo_error){  /* L2Ts error */
	errores.l2ts++;
	ssm_analyz_errors.error |= TTC_IT_ERR_L2TS;
	ssm_analyz_errors.err_tick = maquina.tick;
	ec.error = TTC_IT_ERR_L2TS;
	ec.tick = maquina.tick;
	j = ttcit_mm_FIFO_put(&error_list, &ec);
	do_nothing = TRUE;
	break;
      }

      siz = sizeof(struct ttc_it_seq_L0_L1_L1M);
      memcpy(&L_012.l0_l1, &L_01M, siz);
      L_012.tick_l2 = maquina.B.tick_start;
      L_012.type = (maquina.B.group == G_L2A) ? L2A : L2R;

      L_012.nw = maquina.B.n_words_received;
      L_012.header = maquina.B.hdr;
      for(i=0; i<maquina.B.n_words; i++){
	L_012.data[i] = maquina.B.words[i];
      }
      L_012.l2f = (maquina.B.format_error && maquina.B.l2f);

      /*
	At the beginning of the SSM we may have L2a/L2r messages belonging 
	to L0-L1's which happened long before we started to fill the SSM.

	Trying to filter these cases out:

	WE MUST WRITE SOME SENSIBLE CODE TO HANDLE THIS

	FOR TIME BEING NOTHING IS DONE

       */

      old_l2 = FALSE;
      if(old_l2){
	do_nothing = TRUE;
	break;
      }

      /* 
	 Store time differences
       */
      timeout = L_012.tick_l2 - L_012.l0_l1.l0_l1.tick_l1;

      timing.l1_l2_mean += (double)timeout;
      timing.l1_l2_n += 1.;
      timing.l1_l2_min = ((unsigned long)timeout < timing.l1_l2_min) ? 
	timeout :
	timing.l1_l2_min;
      timing.l1_l2_max = ((unsigned long)timeout > timing.l1_l2_max) ? 
	timeout :
	timing.l1_l2_max;

      bc1 = L_012.l0_l1.l0_l1.bcid_l1 & TTC_DATA_MASK;
      bc2 = L_012.header & TTC_DATA_MASK;
      dbc = bc1 - bc2 + TTCIT_MAX_BUNCH_XING + 1;
      dbc %= (TTCIT_MAX_BUNCH_XING + 1);

#ifdef _MONITOR_SCAN_BCID_DIFF__
      bcd_dirt.tick = maquina.tick;
      bcd_dirt.bc_l1 = bc1;
      bcd_dirt.bc_l2a = bc2;
      bcd_dirt.dbc = dbc;
      irc_bcd_dirt = ttcit_mm_FIFO_put(&DirtyBCID_fifo, &bcd_dirt);
      if(irc_bcd_dirt < 0){
	printf("Increase size of DirtyBCID_fifo, FIFO FULL with %d objects\n",
	       DirtyBCID_fifo.infifo);
      }
#endif

      timing.bc_l1_l2_mean += (double)dbc;
      timing.bc_l1_l2_n += 1.;
      timing.bc_l1_l2_min = ((unsigned long)dbc < timing.bc_l1_l2_min) ? dbc : 
	timing.bc_l1_l2_min;
      timing.bc_l1_l2_max = ((unsigned long)dbc > timing.bc_l1_l2_max) ? dbc :
	timing.bc_l1_l2_max;

      /*
	Check Ll - L2 timeout
       */

      if(timeout > options.l1_l2_timeout){
	errores.l2t++;
	if(!l2t_already_found){
	  ssm_analyz_errors.error |= TTC_IT_ERR_L2T;
	  ssm_analyz_errors.err_tick = maquina.tick;
	  ec.error = TTC_IT_ERR_L2T;
	  ec.tick = maquina.tick;
	  j = ttcit_mm_FIFO_put(&error_list, &ec);
	}
	do_nothing = TRUE;
	/*
	  Maybe we are just trying to merge L2a/L2r with the L0-L1-L1m
	  segment suffering from L2T error.

	  Let us try the next L0-L1-L1m segment (if exists and try to
	  compose the L0-L1-L1m-L2a/L2r sequence
	 */
	l2t_already_found = TRUE;
	once_more = TRUE;
	break;
      }

      /* 
	 Check BCID difference
       */

      if((dbc < options.low_bc) || (dbc > options.up_bc)){
	errores.bcid++;
	ssm_analyz_errors.error |= TTC_IT_ERR_BCID;
	ssm_analyz_errors.err_tick = maquina.tick;
	ec.error = TTC_IT_ERR_BCID;
	ec.tick = maquina.tick;
	j = ttcit_mm_FIFO_put(&error_list, &ec);
	do_nothing = TRUE;
	break;
      }

      fifo = &L0L1L1mL2_fifo;
      Obj = &L_012;
      if(maquina.B.group == G_L2A){
	counters.accepted++;
      }
      if(maquina.B.group == G_L2R){
	counters.l2_reject++;
      }
      break;
    case G_ROI:      

      fifo_irc = ttcit_mm_FIFO_get(&L1_fifo, &L_1, PICK);
      fifo_error = (fifo_irc <= 0);
      if(!fifo_error){
	fifo_error = L_1.used_roi;
      }
      if(!fifo_error){
	L1_ptr = (struct ttc_it_seq_L1 *) ttcit_mm_FIFO_get_addr(&L1_fifo);
	fifo_error = (L1_ptr == NULL);
      }

      if(fifo_error){   /* RoI-S error: surplus RoI */
	ssm_analyz_errors.error |= TTC_IT_ERR_ROIS;
	ssm_analyz_errors.err_tick = maquina.tick;
	ec.error = TTC_IT_ERR_ROIS;
	ec.tick = maquina.tick;
	j = ttcit_mm_FIFO_put(&error_list, &ec);
	errores.rois++;
	do_nothing = TRUE;
	break;
      }

      L1_ptr->used_roi = TRUE;  /* L1 can be used for RoI only once */

      RoI.l1_tick = L_1.l1_tick;
      RoI.l1_bcid = L_1.l1_bcid;
      RoI.RoI_tick = maquina.tick;
      RoI.hdr = maquina.B.hdr;
      for(i=0; i<maquina.B.n_words; i++){
	RoI.data[i] = maquina.B.words[i];
      }
      RoI.roif = (maquina.B.format_error && maquina.B.roif);

      /*
	Store time differences
       */

      timeout = RoI.RoI_tick - RoI.l1_tick;

      timing.l1_roi_mean += (double)timeout;
      timing.l1_roi_n += 1.;
      timing.l1_roi_min = ((unsigned long)timeout < timing.l1_roi_min) ? 
	timeout :
	timing.l1_roi_min;
      timing.l1_roi_max = ((unsigned long)timeout > timing.l1_roi_max) ? 
	timeout :
	timing.l1_roi_max;

      bc1 = RoI.l1_bcid;
      bc2 = ( RoI.hdr & TTC_DATA_MASK);
      dbc = bc2 - bc1;

      timing.bc_l1_roi_mean += (double)dbc;
      timing.bc_l1_roi_n += 1.;
      timing.bc_l1_roi_min = ((unsigned long)dbc < timing.bc_l1_roi_min) ? 
	dbc :
	timing.bc_l1_roi_min;
      timing.bc_l1_roi_max = ((unsigned long)dbc > timing.bc_l1_roi_max) ? 
	dbc :
	timing.bc_l1_roi_max;

      /*

      You must set the test for the RoI-T : you need: Error items
                                                      Timeout value
      */
      if(timeout > options.l1_roi_timeout){
	errores.roit++;
	ssm_analyz_errors.error |= TTC_IT_ERR_ROIT;
	ssm_analyz_errors.err_tick = maquina.tick;
	ec.error = TTC_IT_ERR_ROIT;
	ec.tick = maquina.tick;
	j = ttcit_mm_FIFO_put(&error_list, &ec);
	do_nothing = TRUE;
	break;
      }

      fifo = &RoI_fifo;
      Obj = &RoI;                   
      do_nothing = FALSE;
      break;
    case G_RES:                         /* This should not happen */
    case G_NO:
      do_nothing = TRUE;
      break;
    }

    if(once_more){
      continue;
    }

    if(do_nothing){
      break;
    }

    fifo_irc = ttcit_mm_FIFO_put(fifo, Obj);
    if(fifo_irc <= 0){
      ssm_analyz_errors.error |= TTC_IT_ERR_INTL;
      MARK_INTERNAL_ERROR(8);
      ssm_analyz_errors.err_info = 101;
      ssm_analyz_errors.err_case |= TTCIT_ERR_INTL_SMALF;
      ec.tick = maquina.tick;
      ec.error = TTC_IT_ERR_INTL;
      j = ttcit_mm_FIFO_put(&error_list, &ec);
      irc = 1;
      break;
    }

  }while(once_more);

  return irc;
}

void ttc_it_B_format_error(enum ttc_it_B_ferrors fer){
  int irc;

  struct error_collection ec;

  switch(fer){
  case FER_L1F:
    ec.error = TTC_IT_ERR_L1F;
    ssm_analyz_errors.error |= TTC_IT_ERR_L1F;
    ssm_analyz_errors.err_tick = maquina.tick;
    errores.l1f++;
    break;
  case FER_L2F:
    ec.error = TTC_IT_ERR_L2F;
    ssm_analyz_errors.error |= TTC_IT_ERR_L2F;
    ssm_analyz_errors.err_tick = maquina.tick;
    errores.l2f++;
    break;
  case FER_ROIF:
    ec.error  = TTC_IT_ERR_ROIF;
    ssm_analyz_errors.error |= TTC_IT_ERR_ROIF;
    ssm_analyz_errors.err_tick = maquina.tick;
    errores.roif++;
    break;
  }
  ec.tick = maquina.tick;
  irc = ttcit_mm_FIFO_put(&error_list, &ec);
}

void ttc_it_AB_postprocess(){
  /*
    1) Catching all missing L1M when only L0-L1 were transmitted
       These cannot be caught by A/B machines.
   */

  struct error_collection ec;
  struct ttc_it_seq_L0_L1 l01;
  int irc;
  int l1_tick;
  int tick_diff;

  int i;
  struct error_collection errors_l1mo[ANALYZ_MAX_ERR_LIST];
  int Nlist = -1;
  int Nswap;
  int t0, t1;

  do{
    /*
      If we had internal error, do not do anything
     */
    if((ssm_analyz_errors.error & TTC_IT_ERR_INTL) != 0){
      break;
    }


    /*
      All L1 without L1M must be in L0_L1 FIFO, which empties the oldest
      entries when L1M is encountered.

      Loop over L0_L1 fifo and mark all L1M that did not materialize as
      missing
     */
    irc = ttcit_mm_FIFO_get(&L0L1_fifo, &l01, PICK);
    while(irc > 0){
      l1_tick = l01.tick_l1;   /* Arrival time for L1 */
      tick_diff = TTCIT_MAX_ADDR_SSM - l1_tick;
      if(tick_diff > options.l1_l1m_timeout){  /* Missing L1m */
	errors_l1mo[++Nlist].error = TTC_IT_ERR_L1MO;
	errors_l1mo[Nlist].tick = l1_tick;
	errores.l1mo++;
	ssm_analyz_errors.error |= TTC_IT_ERR_L1MO;
	ssm_analyz_errors.err_tick = l1_tick + options.l1_l1m_timeout;
	if(Nlist >= (ANALYZ_MAX_ERR_LIST-1)){
	  printf("### ttc_it_AB_postprocess : ANALYZ_MAX_ERR_LIST small\n");
	  break; 
	}
      }

      irc = ttcit_mm_FIFO_get(&L0L1_fifo, &l01, PICK);
    }

    /*
      If something added, merge old errors with the new ones and sort
     */
    do{
      if(Nlist < 0){ /* Nothing added, nothing done */
	break;
      }
      if(Nlist >= (ANALYZ_MAX_ERR_LIST-1)){
	printf("### ttc_it_AB_postprocess : ELARGE ANALYZ_MAX_ERR_LIST\n");
	break;
      }
      irc = ttcit_mm_FIFO_get(&error_list, &ec, PICK);
      while(irc > 0){
	errors_l1mo[++Nlist].error = ec.error;
	errors_l1mo[Nlist].tick = ec.tick;
	if(Nlist >= (ANALYZ_MAX_ERR_LIST-1)){
	  printf("### ttc_it_AB_postprocess: Insufficient  ANALYZ_MAX_ERR_LIST\n");
	  break;
	}
	irc = ttcit_mm_FIFO_get(&error_list, &ec, PICK);
      }

      /* Safety clean */
      ttcit_mm_FIFO_empty(&error_list);

      /* Sort array of errors with increasing ticks */
      do{
	Nswap = 0;
	for(i = 0; i < Nlist; i++){
	  t0 = errors_l1mo[i].tick;
	  t1 = errors_l1mo[i+1].tick;
	  if(t0 > t1){                 /* Swap if next is earlier */
	    ec.error = errors_l1mo[i].error;
	    ec.tick = errors_l1mo[i].tick;
	    errors_l1mo[i].error = errors_l1mo[i+1].error;
	    errors_l1mo[i].tick = errors_l1mo[i+1].tick;
	    errors_l1mo[i+1].error = ec.error;
	    errors_l1mo[i+1].tick = ec.tick;
	    Nswap++;
	  }
	}
      }while(Nswap > 0);

      /* Refill error_list FIFO */
      for(i = 0; i <= Nlist; i++){
	ec.error = errors_l1mo[i].error;
	ec.tick = errors_l1mo[i].tick;
	irc = ttcit_mm_FIFO_put(&error_list,&ec);
      }

    }while(0);


  }while(0);
}

void ttc_it_ab_copy(struct ttc_it_A_B_channel *src,
		    struct ttc_it_A_B_channel *dest){
  dest->ticks = src->ticks;
  dest->s = src->s;
}

int soft_monitor_put_ab(struct ttc_it_A_B_channel *s){
  int irc = 0;
  irc = (ttcit_mm_FIFO_put(&AB_fifo, s) > 0) ? 0 : 1;
  return irc;
}

int soft_monitor_put_bc(unsigned long *bc){
  int irc = 0;
  irc = (ttcit_mm_FIFO_put(&BC_fifo, bc) > 0) ? 0 : 1;
  return irc;
}

/*
  This should Print commented contents of AB_fifo and BC_fifo with
  cross references
 */
void ttc_it_show_ABBC(){

  char line[80];
  char toLine[50];

  char *sAddr = "  Addr  ";            /* 8 */
  char *sTicks = " Ticks ";            /* 7 */
  char *sA_Channel = " A Channel  ";   /* 12 */
  char *sB_Channel = " B Channel  ";   /* 15 */
  char *sBCID = "TTC Rx - BCID - wrd";
  char *sL0 = "L0 ";
  char *sL1a = "L1-";
  char *sL1b = "-L1";
  char *sL01 = "L01";

  char *sMissing = "MISSING ";

  int iAdded = 0xabcadded;

  char *s;

  char *fmtTxt = "%8s  %7s  %12s  %12s  %19s\n";

  unsigned long bc;
  struct ttc_it_A_B_channel ab;
  struct ttc_it_A_B_channel xx;
  int t;
  int irc;
  int ticks, address;
  unsigned long data;
  unsigned long ttc_address, ttc_data;
  int i, j;
  int n = 0;

  int signal_in_A;
  int data_in_B;
  unsigned long bcid_rx, bcid_word;
  int bc_in_ttcrx, bc_in_word;
  unsigned long empty = 0x80000000;

  enum { L0_A, L1_A, UNKNOWN_A } signal_type;
  enum { L1_START, L1_END, L1_EMPTY } signal_state;

  char *e = NULL;
  int et;
  int leave_it;

  int nr_L0 = 0;
  int nr_L1 = 0;

  int diff_tick;

  struct error_collection dummy_ec;

  /*
    Print Error flags array for the last event (if used)
   */
  ttc_it_show_errmask();

  /*
    Quick analys and side-by-side print of AB_fifo and BC_fifo
   */

  leave_it = FALSE;

  printf(" \n");
  j = printf(fmtTxt,sAddr,sTicks,sA_Channel,sB_Channel,sBCID);
  for(i=0; i<j; i++){
    line[i] = '-';
  }
  line[j] = '\0';
  printf("%s\n",line);
  
  ttc_it_get_next_error(&e, &et, &dummy_ec);

  signal_state = L1_EMPTY;
  for(irc = ttcit_mm_FIFO_get(&AB_fifo, &ab, PICK); irc > 0; 
      irc = ttcit_mm_FIFO_get(&AB_fifo, &ab, PICK)){
    /*
      Skip everything, that is neither A channel nor valid data in 
      B channel
     */
    bc_in_ttcrx = FALSE;
    bc_in_word = FALSE;

    data = ab.s;
    signal_in_A = ((data & MASK_L1ACCEPT) != 0);
    data_in_B = ((data & MASK_DOUTSTR) != 0);
    if((!signal_in_A) && (!data_in_B)){          /* Skip if not valid info */
      continue;
    }
    n++;
    /*
      Clear the line[]
    */
    for(i=0; i<80; i++){
      line[i] = '\0';
    }
    /*
      Get the address and ticks, put them into line[]
    */
    ticks = ab.ticks;
    address = (ticks >= 0) ? ticks : iAdded;
    sprintf(&toLine[0], "%08X  %+07d  ",address,ticks);
    s = strcat(&line[0], &toLine[0]);

    /*
      Signal in A channel - this can be either L0 or L1
     */
    if(signal_in_A){
      /*
	Try to determine the type of signal in A channel, 
	use the next word in the AB fifo
       */
      do{
	if(signal_state == L1_START){ /* This is the 2-nd part of L1 */
	  signal_type = L1_A;
	  signal_state = L1_END;
	  break;
	}

	j = ttcit_mm_FIFO_get(&AB_fifo, &xx, COPY);
	if(j < 0){  /* No data in FIFO, signal unknown */
	  signal_type = UNKNOWN_A;
	  signal_state = L1_EMPTY;
	  printf("AB FIFO exhausted, L0/L1 discrimination impossible %s\n",
	       " STOPING data dump");
	  goto safety_exit;
	  break;
	}
	diff_tick = xx.ticks - ab.ticks; /* L1 must be not too far */
	t = (((xx.s & MASK_L1ACCEPT) == 0) || (diff_tick != 1));
	if(t){  /* No signal in A channel follows, it is L0 */
	  signal_type = L0_A;
	  signal_state = L1_EMPTY;
	  break;
	}else{   /* This must be a start of the L1 signal */
	  signal_type = L1_A;
	  signal_state = L1_START;
	  break;
	}
	break;
      }while(0);

      switch(signal_type){
      case L0_A:
	sprintf(&toLine[0], "  %3s%3s    ", sL0, " ");
	nr_L0++;
	break;
      case L1_A:
	switch(signal_state){
	case L1_START:
	  sprintf(&toLine[0], "  %3s%3s    ", sL1a, " ");
	  nr_L1++;
	  break;
	case L1_END:
	  sprintf(&toLine[0], "  %3s%3s    ", " ", sL1b);
	  /*
	    At the end of the L1 signal we should pick the BC value
	    from the BC FIFO
	   */
	  j = ttcit_mm_FIFO_get(&BC_fifo, &bc, PICK);
	  bcid_rx = (j > 0) ? bc : empty;
	  bc_in_ttcrx = (j > 0);
	  break;
	default:
	  sprintf(&toLine[0], "%10s    ", "L1BUG L1BUG");
	  break;
	}
	break;
      case UNKNOWN_A:
	sprintf(&toLine[0], " %3s%3s    ", sL01, "???");
	break;
      default:
	sprintf(&toLine[0], "%10s    ", "BUG BUG BUG ");
	break;
      }
    }else{
      sprintf(&toLine[0], "%12s  "," ");

    }
    s = strcat(&line[0], &toLine[0]);

    /*
      Signal in B channel - this can be several things
     */
    if(data_in_B){
      ttc_address = ((data & TTC_ADDR_MASK) >> TTC_ADDR_RSH);
      ttc_data = (data & TTC_DATA_MASK);
      switch(ttc_address){
      case 0:
	sprintf(&toLine[0], "%7s  %03lX","Reservd",ttc_data);
	break;
      case 1:
	sprintf(&toLine[0], "%7s  %03lX","L1M Hdr",ttc_data);
	break;
      case 2:
	sprintf(&toLine[0], "%7s  %03lX","L1M Dat",ttc_data);
	break;
      case 3:
	sprintf(&toLine[0], "%7s  %03lX","L2a Hdr",ttc_data);
	bc_in_word = TRUE;
	bcid_word = ttc_data;
	break;
      case 4:
	sprintf(&toLine[0], "%7s  %03lX","L2a Dat",ttc_data);
	break;
      case 5:
	sprintf(&toLine[0], "%7s  %03lX","L2r Adr",ttc_data);
	bc_in_word = TRUE;
	bcid_word = ttc_data;
	break;
      case 6:
	sprintf(&toLine[0], "%7s  %03lX","RoI Hdr",ttc_data);
	break;
      case 7:
	sprintf(&toLine[0], "%7s  %03lX","RoI Dat",ttc_data);
	break;
      default:
	break;
      }
    }else{
      sprintf(&toLine[0], "%15s  "," ");
    }
    s = strcat(&line[0], &toLine[0]);

    /*
      BCID from TTC Rx
     */
    if(bc_in_ttcrx){
      if((bcid_rx & empty) == 0){
	sprintf(&toLine[0], "     %03lX  ",bcid_rx);
      }else{
	sprintf(&toLine[0], "%8s  ", sMissing);
	leave_it = TRUE;
      }
    }else{
      sprintf(&toLine[0], "%8s  "," ");
    }
    s = strcat(&line[0], &toLine[0]);

    /*
      BCID from L2a/L2r
     */
    if(bc_in_word){
      if((bcid_word & empty ) == 0){
	sprintf(&toLine[0], "    %03lX", bcid_word);
      }else{
	sprintf(&toLine[0], "    %8s  ", sMissing);
      }
    }else{
      sprintf(&toLine[0], "    %3s", " ");
    }
    s = strcat(&line[0], &toLine[0]);

    /*
      And print next line
    */
    printf("%s\n",&line[0]);
    if(leave_it){
      goto safety_exit;
    }

    /*
      Look in the error list, maybe some error happened here
     */
    if(e != NULL){
      if(ticks == et){
	printf(">>---------->> E R R O R >>--> %s\n",e);
	ttc_it_get_next_error(&e, &et, &dummy_ec);
      }else if(ticks > et){
	printf(">>>---->>> UNMATCHED ERROR: %s at Tick : %d\n",s,et);
	ttc_it_get_next_error(&e, &et, &dummy_ec);
      }
    }
  }

 safety_exit:

  printf(" --- \n");
  printf("Read %d events from the FIFO\n",n);
  printf("Seen  L0 signals %d\n",nr_L0);
  printf("Seen  L1 signals %d\n",nr_L1);
  printf(" \n");

}

int ttc_it_was_error(){
  int retval;
  int irc;
  struct error_collection ec;

  irc = ttcit_mm_FIFO_get(&error_list, &ec, COPY);
  retval = (irc > 0);

  return retval;
}

void ttc_it_get_next_error(char **s, int *tk, struct error_collection *ec){
  int irc;

  irc = ttcit_mm_FIFO_get(&error_list, ec, PICK);
  if(irc > 0){
    switch(ec->error){
    case TTC_IT_ERR_L0S:
      *s = "L0S";
      break;
    case TTC_IT_ERR_L1S:
      *s = "L1S";
      break;
    case TTC_IT_ERR_L1T:
      *s = "L1T";
      break;
    case TTC_IT_ERR_L1M:
      *s = "L1M";
      break;
    case TTC_IT_ERR_L1MO:
      *s = "L1MO";
      break;
    case TTC_IT_ERR_L1F:
      *s = "L1F";
      break;
    case TTC_IT_ERR_L2T:
      *s = "L2T";
      break;
    case TTC_IT_ERR_L2TS:
      *s = "L2Ts";
      break;
    case TTC_IT_ERR_L2F:
      *s = "L2F";
      break;
    case TTC_IT_ERR_BCID:
      *s = "BC ID dont match";
      break;
    case TTC_IT_ERR_INTL:
      *s = "INERNAL ERROR";
      break;
    case TTC_IT_STOP_AN:
      *s = "Stop analysis";
      break;
    case TTC_IT_ERR_ROIF:
      *s = "RoI-F";
      break;
    case TTC_IT_ERR_ROIS:
      *s = "RoI-S";
      break;
    case TTC_IT_ERR_ROIT:
      *s = "RoI-T";
      break;
    default:
      *s = "SOME ERROR";
      break;
    }
    *tk = ec->tick;
  }else{
    *s = NULL;
    *tk = 0xdeaddead;
  }
}

void ttc_it_show_errmask(){
  int i;
  char e[60];
  char *s;
  unsigned long r;
  int intl;

  int tk;

  struct error_collection err_backup[ANALYZ_MAX_ERR_LIST];
  int n_err;
  int i_err;

  e[0] = '\0';
  r = ssm_analyz_errors.error;
  intl = FALSE;

  for(i = 1; i <= 12; i++){
    switch(i){
    case 1:
      if((r & TTC_IT_ERR_L0S) != 0){
	s = strcat(&e[0], "L0S ");
      }
      break;
    case 2:
      if((r & TTC_IT_ERR_L1S) != 0){
	s = strcat(&e[0], "L1S ");
      }
      break;
    case 3:
      if((r & TTC_IT_ERR_L1T) != 0){
	s = strcat(&e[0], "L1T ");
      }
      break;
    case 4:
      if((r & TTC_IT_ERR_L1M) != 0){
	s = strcat(&e[0], "L1M ");
      }
      break;
    case 5:
      if((r & TTC_IT_ERR_L1MO) != 0){
	s = strcat(&e[0], "L1Mo ");
      }
      break;
    case 6:
      if((r & TTC_IT_ERR_L1F) != 0){
	s = strcat(&e[0], "L1F ");
      }
      break;
    case 7:
      if((r & TTC_IT_ERR_L2T) != 0){
	s = strcat(&e[0], "L2T ");
      }
      break;
    case 8:
      if((r & TTC_IT_ERR_L2TS) != 0){
	s = strcat(&e[0], "L2Ts ");
      }
      break;
    case 9:
      if((r & TTC_IT_ERR_L2F) != 0){
	s = strcat(&e[0], "L2F ");
      }
      break;
    case 10:
      if((r & TTC_IT_ERR_BCID) != 0){
	s = strcat(&e[0], "BCID ");
      }
      break;
    case 11:
      if((r & TTC_IT_ERR_BCID) != 0){
	s = strcat(&e[0], "INTERNAL");
	intl = TRUE;
      }
      break;
    case 12:
      if((r & TTC_IT_ERR_ROIF) != 0){
	s = strcat(&e[0],"RoI-F");
      }
    default:
      break;
    }
  }
  printf("ERROR MASK\n");
  printf("----------\n");
  printf(" \n");
  printf("Error list      %s\n",e);
  printf("Error flags     %lX\n",r);
  printf("Last error tick %d     address: %X\n",ssm_analyz_errors.err_tick,
	 ssm_analyz_errors.err_tick);
  if(intl){
    printf("Internal error: CASE = %d\n",ssm_analyz_errors.err_case);
    printf("Internal error: INFO = %d\n",ssm_analyz_errors.err_info);
  }
  if(ssm_analyz_errors.stop_analysis){
    printf("SSM analysis STOPPED with INFO %d\n",ssm_analyz_errors.err_info);
  }
  printf("Nr. of errors stored : %u\n",error_list.infifo);
  printf("List of error occurences:\n");
  n_err = -1;
  for(ttc_it_get_next_error(&s, &tk, &err_backup[++n_err]); s != NULL; 
      ttc_it_get_next_error(&s, &tk, &err_backup[++n_err])){
    printf("Tick %8d    : ERROR = %s\n",tk,s);
  }
  printf(" \n");
  printf(" -- \n");
  
  /*
    Regerate the Error list. It is a bit stupid, but it is done in such a 
    way and I do not want to change the code on many places
   */
  ttcit_mm_FIFO_empty(&error_list);
  for(i_err = 0; i_err < n_err; i_err++){
    if((i = ttcit_mm_FIFO_put(&error_list, &err_backup[i_err])) < 0){
      printf("ttc_it_show_backup: Cannot restore error list!!!\n");
    }  
  }
}

int get_SSM_ANALYZ_options(struct ttc_it_options *s){
  int irc = 0;
  size_t siz;
  void *dummy = NULL;
  do{
    if(s == NULL){
      irc = 1;
      break;
    }
    siz = sizeof(struct ttc_it_options);
    dummy = memcpy(s,&options,siz);
    if(dummy == NULL){
      irc = 2;
    }
  }while(0);
  return irc;
}

#ifdef TTCIT_EXTRA_DBG_PRINT_1__
void ttc_it_print_collections(){

  struct ttc_it_seq_L0_L1 l01;
  struct ttc_it_seq_L0_L1_L1M l011m;
  struct ttc_it_seq_L0_L1_L1M_L2 l01m2;
  struct ttc_it_seq_RoI l1roi;
  int dbg_irc;
  int i;

  /*
    During soft MONITOR run all extra print is suppressed 
    (when looping over many events, this may lead to HUGE output
  */

  if(ttc_it_extra_print_2_want__){
    return;
  }

  /*
    For time being let us print the FIFO's at the occurence of any error
  */

  if((ttc_it_extra_print_1_want__) && (error_list.infifo == 0)){
    return;
  }

  printf("DEBUG PRINT: L0L1_fifo \n");
  while((dbg_irc = ttcit_mm_FIFO_get(&L0L1_fifo, &l01, PICK)) > 0){
    printf("L0L1_fifo: item %5d | L0 time = %6d / L0_BCID = %3X\n",
	   dbg_irc,l01.tick_l0, l01.bcid_l0);
    printf("                       | L1 time = %6d / L1_BCID = %3X\n",
	   l01.tick_l1, l01.bcid_l1);
  }
  printf(" -- \n");

  printf("DEBUG PRINT: L0L1L1m_fifo \n");
  while((dbg_irc = ttcit_mm_FIFO_get(&L0L1L1m_fifo, &l011m, PICK)) > 0){
    printf("L0L1m_fifo: item %5d | L0 time = %6d / L0_BCID = %3X\n",
	   dbg_irc, l011m.l0_l1.tick_l0, l011m.l0_l1.bcid_l0 );
    printf("                      | L1 time = %6d / L1_BCID = %3X\n",
	   l011m.l0_l1.tick_l1, l011m.l0_l1.bcid_l1);
    printf("                        L1m HDR = %3X | L1m = %3X %3X %3X %3X \n",
	   l011m.header, l011m.data[0], l011m.data[1], l011m.data[2],
	   l011m.data[3]);
    printf(" >>\n");
  }
  printf(" -- \n");

  printf("DEBUG PRINT: L0L1L1mL2_fifo \n");
  while((dbg_irc = ttcit_mm_FIFO_get(&L0L1L1mL2_fifo, &l01m2, PICK)) > 0){
    printf("L0L1_fifo: item %5d | L0 time = %6d / L0_BCID = %3X\n",
	   dbg_irc, l01m2.l0_l1.l0_l1.tick_l0, l01m2.l0_l1.l0_l1.bcid_l0);
    printf("                      | L1 time = %6d / L1_BCID = %3X\n",
	   l01m2.l0_l1.l0_l1.tick_l1, l01m2.l0_l1.l0_l1.bcid_l1);
    printf("                        L1m HDR = %3X | L1m = %3X %3X %3X %3X \n",
	   l01m2.l0_l1.header, l01m2.l0_l1.data[0], 
	   l01m2.l0_l1.data[1], l01m2.l0_l1.data[2],
	   l01m2.l0_l1.data[3]);
    printf("                        L2a/L2r HDR = %3X | msg = ",l01m2.header);
    for(i=0; i<l01m2.nw; i++){
      printf("%3X ",l01m2.data[i]);
    }
    printf("\n");
    if(l01m2.l0_l1.l0_l1.fake){
      printf("FAKE-FAKE-FAKE-FAKE-FAKE-FAKE-FAKE-FAKE-FAKE-FAKE\n");
    }
    if(l01m2.l0_l1.l1f){
      printf("L1F FORMAT ERROR L1F FORMAT ERROR L1F FORMAT ERROR L1F\n");
    }
    if(l01m2.l2f){
      printf("L2F FORMAT ERRPR L2F FORMAT ERRPR L2F FORMAT ERROR L2F\n");
    }
    printf(" *** \n");
  }
  printf(" -- \n");

  printf("DEBUG PRINT: RoI_fifo \n");
  while((dbg_irc = ttcit_mm_FIFO_get(&RoI_fifo, &l1roi, PICK)) > 0){

  }
  printf(" -- \n");


}
#endif

#ifdef _MONITOR_SCAN_BCID_DIFF__

int ttc_it_dirty_bcid_add(int clear){
  int irc = 0;
  int j;
  struct ttc_it_dirty_BCID bc;
  int hmin, hmax;
  int first_error;
  int isError, iret;
  struct error_collection ec;

  switch(clear){
  case 0:
    iret = ttcit_mm_FIFO_get(&error_list, &ec, COPY);
    isError = (iret > 0);

    /*
      If there was no error encountered, take all sampled BCID differences,
      if some error happend, samples only thsose that happened BEFORE the
      error
     */
    if(isError){
      first_error = ec.tick;
    }
    while((j = ttcit_mm_FIFO_get(&DirtyBCID_fifo, &bc, PICK)) > 0){
      if(isError){
	if(bc.tick >= first_error){   /* Do not sample behind error */
	  break;
	}
      }
      ttcit_mm_I1Di1_fill(&hist_dirty_dbc, bc.dbc);
    }
    break;
  case 1:
    hmin = options.bc_l1_l2_win - options.bc_l1_l2_wid - 5;
    if(hmin < 0){ hmin = 0;};
    hmax = options.bc_l1_l2_win + options.bc_l1_l2_wid + 5;
    j = ttcit_mm_I1Di1_create(&hist_dirty_dbc, hmin, hmax);
    if(j < 0){
      irc = j;
    }
    break;
  case 2:
    ttcit_mm_I1Di1_delete(&hist_dirty_dbc);
    break;
  default:
    break;
  }
  return irc;
}

void ttc_it_dirty_bcid_print(){
  ttcit_mm_I1Di1_print(&hist_dirty_dbc, "BC ID L1-L2a differences\n");
}

#endif
