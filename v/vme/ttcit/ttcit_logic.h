#ifndef _TTCIT_LOGIC_H__
#define _TTCIT_LOGIC_H__

/*
  Definitions 
 */

#include "ttcit_conf.h"

/*
  Variable declarations
*/

struct SSMbuffer {
  unsigned long buf[MEGA];  /* 1M of 18 bit words   */
  int top;                  /* Nr. of elements filled */
};

struct BCfifo {
  unsigned long buf[BCNT_FIFO_SIZ]; /* Buffer, appropriately dimensioned */
  int top;                          /* Nr. of data */
};

/*
  Stop conditions
 */
struct MONITOR_stops {

  int NSEQ;  /* Number of SSM sequences to be read before STOP */

  int ANY_ERROR;  /* Stop at any error, TRUE/FALSE */

  /*  TRUE/FALSE if you want to stop at some particular error */

  int Stop_at_L0S;
  int Stop_at_L1S;
  int Stop_at_L1T;
  int Stop_at_L1M;
  int Stop_at_L1Mo;
  int Stop_at_L1F;
  int Stop_at_L2T;
  int Stop_at_L2Ts;
  int Stop_at_L2F;
  int Stop_at_BCID_diff;
};

enum runStatus { RUNNING, STOPPING, STOPPED };

struct MonitorStatus {
  pthread_t monitorThread;    /* Soft montitor thread */
  pthread_t *th;              /* This is set to NULL when no active thread */
  pthread_attr_t threadAttr; /* Soft monitor thread attributes */
  enum runStatus run;        /* Soft monitor running status RUNNING, STOPPED */
  int fd;                    /* File descriptor */

  /*
    Monitor options
   */

  int create_file;      /* TRUE/FALSE  create or do not create file */
  int online_diag;      /* TRUE/FALSE  perform or do not online diagnostics */

  /*
    Monitor counters
   */
  unsigned long nSnapShots; /* Number of SnapShots taken */

};

struct my_funky_data {
  unsigned int loops_per_minute;
};

union sign_indef {
  int i;
  unsigned long ul;
};

struct logic_StatusInternal {

  union sign_indef mode;     /* SSM mode: TTCIT_IO_SSM_SCOPE : timing
                                          TTCIT_IO_SSM_SEQ   : no timing
			      */
  unsigned long nseq;        /* Number of sequences encountered */
  int nSCOPE;                /* Number of sequences with SCOPE reads of SSM */
  int nSEQ;                  /* Number of sequences with SEQ reads of SSM */
  int nLoops;                /* Number of loops to be performed */
  int nAnalyzd;               /* Number of sequences analyzed */

  struct MonitorStatus monitor; /* Soft monitor status record */
  struct MONITOR_stops stop;    /* Stopping options for MONITOR */

  int justDump;              /* Do not analyze when rescan, just dump data */

};

/*
  TTCit logic in software - functions
 */

void print_thread_error_message(const char *s, int irc);

void reset_TTCit_logic();
int init_TTCIT_logic();
void print_TTCIT_logic_state();
void set_TTCIT_stop_conds(struct MONITOR_stops *s);
void get_TTCIT_stop_conds(struct MONITOR_stops *s);
void set_TTCIT_default_stops();
void print_TTCIT_stop_conds();

int read_SSM_address();
int read_last_SSM_word();
int fetch_SSM();
void dump_SS_buffer(struct SSMbuffer S, int nw);
void dump_SSM(int nw);
void clear_SS_memory_buffer();
void clear_SS_memory_on_board();
void test_clear_SSM_on_board();
int get_SS_top_mem_buffer();
int write_SS_buffer();
void throw_SS_data(int addr);
void set_SS_mode(int mode);
int get_SS_mode();

int get_nr_words_bcnt_fifo();
int fetch_bcnt_fifo();
void print_bcnt_fifo_buf(int nw);

void fetch_print_SS_bc();

int wait_till_SSM_full(int verbal);

int soft_monitor_start(int nScope, int nSeq, int nLoops);
void soft_monitor_stop();
int soft_monitor_status();
unsigned long soft_monitor_snapshots();
void *soft_monitor_loop(void *dummy);
void soft_monitor_file(int what);
void soft_monitor_online_diag(int what);
int soft_monitor_diagnoze(struct SSMbuffer *S, struct BCfifo *BC);
int soft_monitor_fill_ABC(struct SSMbuffer *S, struct BCfifo *BC);
void soft_monitor_set_errmask();
void soft_monitor_single_ssm(int what);
void soft_monitor_N_ssm(int n);
void soft_monitor_rescan(int action);
void soft_monitor_dumpfile();
void test_fetch_SSM(int noprint);
void loop_test_fetch_SSM(int n, int nretries);
void dump_raw_SSM_buffer(int begin, int end);
int BCID_mismatch_finder(int nSamples, int nMaxrep);

void my_dummy_loop(int nrLoops);
unsigned int my_funky_sleep(unsigned int seconds);
unsigned int my_funky_calib();

#endif /* !_TTCIT_LOGIC_H__ */
