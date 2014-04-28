#ifndef SSM_ANALYZ_H__
#define SSM_ANALYZ_H__

#include "ttcit_conf.h"

/*
  Some defines
 */

#define TTC_IT_SMALL_U_NR  0
#define TTC_IT_BIG_U_NR    9999999

/*
  New L2a and L1M messages are longer, we must keep dimensions large 
  enough
 */
#define TTC_IT_L1M_MAX 10
#define TTC_IT_L2A_MAX 20

/*
  Define signals and internal states of the trigger logic simulator
 */

#define LOGIC_A_L0         1      /* L0 signal, 1 tick in A channel */
#define LOGIC_A_L1         2      /* L1 signal, 2 ticks in A channel */
#define LOGIC_B_L1M_H     11      /* L1M header in B channel */
#define LOGIC_B_L1M_W     12      /* L1M word in B channel */
#define LOGIC_B_L2A_H     21      /* L2a Header in B channel */
#define LOGIC_B_L2A_W     22      /* L2a Word in B channel */
#define LOGIC_B_L2R       32      /* L2r message in B channel */
#define LOGIC_B_ELSE      99      /* Anythin else in B channel */ 

/*
  Bits representing errors and error masks
 */

#define TTC_IT_ERR_NONE   0x0

#define TTC_IT_ERR_L0S    0x1
#define TTC_IT_ERR_L1S    0x2
#define TTC_IT_ERR_L1T    0x4
#define TTC_IT_ERR_L1M    0x8
#define TTC_IT_ERR_L1MO   0x10
#define TTC_IT_ERR_L1F    0x20
#define TTC_IT_ERR_L2T    0x40
#define TTC_IT_ERR_L2TS   0x80
#define TTC_IT_ERR_L2F    0x100
#define TTC_IT_ERR_BCID   0x200
#define TTC_IT_ERR_ROIF   0x400
#define TTC_IT_ERR_INTL   0x800
#define TTC_IT_ERR_ROIS   0x2000
#define TTC_IT_ERR_ROIT   0x4000

#define TTC_IT_STOP_AN    0x1000

#define TTC_IT_ERR_ANY    0x3ff

/*
  Default constants and limits
 */
#define TTC_IT_L0_L1_WIN          223
#define TTC_IT_L0_L1_WID            3
#define TTC_IT_L1_L1M_TIMEOUT     500
#define TTC_IT_L1_L2_TIMEOUT     4098
#define TTC_IT_BC_L1_L2_WIN         1   /* DEBUG value */
#define TTC_IT_BC_L1_L2_WID      5000   /* DEBUG value */
#define TTC_IT_L1_ROI_TIMEOUT     500


#define TTCIT_ADDTO(A) dest->A += src->A

#define TTCIT_ADDT(A) dest->A##_min = (src->A##_min < dest->A##_min) ?	\
    src->A##_min : dest->A##_min; \
  dest->A##_max = (src->A##_max > dest->A##_max) ? \
    src->A##_max : dest->A##_max;			   \
  dest->A##_mean = (src->A##_n != 0.) ? \
    ((dest->A##_n * dest->A##_mean) + src->A##_mean) / \
    (dest->A##_n + src->A##_n) : dest->A##_n; \
    dest->A##_n += src->A##_n;	   

/*
  Data structures
 */

enum ttc_it_L0 { A_CHANNEL, WIRE, A_WIRE, UNKNOWN, ABSENT };

enum ttc_it_stop { NEVER, ANY_ERROR };

struct ttc_it_options {

  enum ttc_it_L0 l0;     /* Presence and the mean of transmission of L0 */

  int l0_l1_win;   /* L0 - L1 time window in 26ns tick */
  int l0_l1_wid;   /* Width of L0 - L1 time window: WIN-WID < T < WIN+WID */

  int l1_l1m_timeout; /* L1m timeout, this time L1m is considered missing */

  int l1_l2_timeout; /* L1 - L2 timeout after which L2 is considered missing */

  int bc_l1_l2_win; /* Acceptable BC difference between L1 a L2a/L2r word */
  int bc_l1_l2_wid; /* Width of the BC difference window */

  /*
    Calculated values (these must be calculated during initialization)
   */

  int low_l0l1;   /* Lower edge of the L0 - L1 decision window */
  int up_l0l1;     /* Upper edge of the L0 - L1 decision window */

  int low_bc;      /* Lower edge of the BCID L1 - L2 acceptable difference */
  int up_bc;       /* Upper egdge of this window */

  int l1_roi_timeout; /* Max allowed time difference between L1 and RoI */
  int bc_l1_roi_win;  /* Mean allowed BCID difference between L1 and RoI */
  int bc_l1_roi_wid;  /* Half width of the allowed BCID L1 and RoI diff. */
  int low_bc_roi;     /* Lower edge BCID difference */
  int up_bc_roi;      /* Upper edge BCID difference */

  unsigned long bc_min;   /* Min BCID value */
  unsigned long bc_max;   /* Max BCID value */
  unsigned long bc_fiz_min;  /* Min BCID for physics */
  unsigned long bc_fiz_max;  /* Max BCID for physics */
  unsigned long bc_pp;       /* BCID where PrePulse is sent */

  unsigned long bc_l2_filter; /* BCID marking the "end of orbit" */
  unsigned long bc_l2_fil2;   /* BCID marking the "beginning of orbit" */

};

struct ttc_it_internal {

  int l0_seen;   /* TRUE/FALSE according to availability of L0 signal */

  unsigned long nBuffs;  /* Number of buffers inspected */

  time_t uptime; /* Running time */
  time_t start_time;

  int l0_in_ssm;  /* Number of L0's seen in the current SSM */
  int l1_in_ssm;  /* Number of L1's seen in the current SSM */

};

struct ttc_it_counters {

  unsigned long l0;                   /* Number of L0 triggers */
  unsigned long l1;                   /*           L1          */
  unsigned long l1m;                  /*           L1M messages correct */
  unsigned long l2a;                  /*           L2a messages correct */
  unsigned long l2r;                  /*           L2r messages correct */

  unsigned long roi;                  /* Number of RoI messages correct */

  unsigned long lxx;                  /*           Unknown messages */

  unsigned long accepted;             /* Nr. of L0-L1-L2a accepted triggers */
  unsigned long l1_reject;            /* Nr. of L1 rejected triggers */
  unsigned long l2_reject;            /* Nr. of L0-L1-L2r sequences */

  unsigned long l0l1_fakes;
};

struct ttc_it_errors {

  unsigned long l0s;    /* surplus L0 inside L0 - L1 decision time */
  unsigned long l1s;    /* surplus L1 without preceding L0 */
  unsigned long l1t;    /* L0 - L1 time violation, L1 outside window */
  unsigned long l1m;    /* L1M sent without preceding L1 signal */
  unsigned long l1mo;   /* No L1M sent within programmable window */
  unsigned long l1f;    /* L1M format error */
  unsigned long l2t;    /* No L2a/L2r sent within programmable window */
  unsigned long l2ts;   /* L2a/L2r sent but no L1 had been received */
  unsigned long l2f;    /* L2a/L2r format error */
  unsigned long bcid;   /* BCID in L1 and L2a/L2r no in programmable window */
  unsigned long roif;   /* RoI message format error */
  unsigned long rois;   /* Surplus RoI message without L1 signal */
  unsigned long roit;   /* RoI timeout */
};

struct ttc_it_times {

  double l0_l0_mean;              /* Statistics for the L0 - L0 timing */
  double l0_l0_n;
  double l0_l0_min;
  double l0_l0_max;

  double l0_l1_mean;              /* Mean L0 - L1 time in 26ns ticks */
  double l0_l1_n;                 /* Nr. of measured L0 - L1 pairs */
  unsigned long l0_l1_min;        /* Mininum L0 - L1 time seen */
  unsigned long l0_l1_max;        /* Maximum L0 - L1 time seen */

  double l1_l2_mean;              /* Mean L1 - L2a/r time in 26ns ticks */
  double l1_l2_n;                 /* Nr. of measured L1 - L2a/L2r pairs */
  unsigned long l1_l2_min;        /* Minimum L1 - L2a/L2r time seen */
  unsigned long l1_l2_max;        /* Maximum L1 - L2a/L2r time seen */

  double l1_l1m_mean;             /* Mean L1 - L1M time in 26ns ticks */
  double l1_l1m_n;                /* Nr. of measured L1 - L1M pairs */
  unsigned long l1_l1m_min;       /* Minimum L1 - L1M time seen */
  unsigned long l1_l1m_max;       /* Maximum L1 - L1M time seen */

  double bc_l1_l2_mean;        /* Mean difference in BC count L1 - L2a/L2r */
  double bc_l1_l2_n;           /* Nr. of L1 , L2a/L2r pairs with BC */
  unsigned long bc_l1_l2_min;  /* Minimum BC distance L1, L2a/L2r */
  unsigned long  bc_l1_l2_max; /* Maximum BC distance L1, L2a/L2r */

  double l1_roi_mean;          /* Time distance between L1 and RoI mean */
  double l1_roi_n;             /* Nr. of samples */
  unsigned long l1_roi_min;    /* Min value of time distance in 26ns ticks */
  unsigned long l1_roi_max;    /* Max value of time distance in 26ns ticks */

  double bc_l1_roi_mean;       /* BC difference between L1 BCID and RoI BCID */
  double bc_l1_roi_n;            /* Number of samples */
  unsigned long bc_l1_roi_min;   /* Min difference between L1 BCID and RoI */
  unsigned long bc_l1_roi_max;   /* Max differennce */
};

struct ttc_it_ssm_compressed {
  int nticks;                        /* Nr. of ticks stored */
  int ticks[MEGA_PLUS_ONE];          /* Ticks from SSM, incl. 1-st L0 */
  unsigned long s[MEGA_PLUS_ONE];    /* SSM for each tick */

  int bccount;                       /* Number of BC's in FIFO */
  unsigned long b[TTCIT_MAX_ADDR_BCFIFO]; /* BF fifo contents */
};

struct ttc_it_A_B_channel {
  int ticks;                 /* Ticks in SSM */
  unsigned long s;           /* SSM word for each tick */
};

struct ttc_it_L1m {

  int n;                 /* Number of words in L1M, must be 5 */

  int l1h_cit;           /* L1M Header:  CIT              */
  int l1h_roc;           /*              RoC[4..1]        */
  int l1h_esr;           /*              ESR              */
  int l1h_l1swc;         /*              L1swC            */
  int l1h_l1class;       /*              L1Class[50,49]   */

  int l1w_l1class[TTC_IT_L1M_MAX];    /* L1M : L1Class[]               */
};

struct ttc_it_L2a {

  int n;                 /* Number of words in L2a message, must be 8 */

  int l2h_bcid;          /* L2a Header:   BCID[12...1] */

  int l2w1_orbitid;      /* L2a Word 1:   OrbitID[24...13] */
  int l2w2_orbitid;      /*          2:   OrbitID[12....1] */

  int l2a_orbitId;       /*               L2a OrbitID full */

  int l2w3_cit;          /* L2a Word 3:   CIT              */ 
  int l2w3_l2sw;         /*               L2sw             */
  int l2w3_l2clust;      /*               L2Cluster[6..1]  */
  int l2w3_l2class;      /*               L2Class[50,49]   */

  int l2w_l2class[TTC_IT_L2A_MAX];    /* L2a Words :   L2Class[...]     */
};

struct ttc_it_L2r {

  int l2r_bcid;          /* L2r :        BCID */
};

struct ttc_it_RoI {

  int n;                 /* Number of words in RoI message, must be 4 */

  int roih_bcid;        /* RoI Header:     BCID */

  int roiw_data[3];    /*  RoI words:      RoI data */
};

struct ttc_it_xxx_message {

  int n;                                 /* Nr. of words in a message */

  int xxx_data[TTC_MAX_MESSAGE_LEN];     /* Message data */
};

/*
  Structures needed for storing the data in FIFO's
 */

/* L0 - L1 accepted pair */

struct ttc_it_seq_L0_L1 {
  int tick_l0;               /* Time of arrival of L0 in 26ns ticks */
  int tick_l1;               /* Time of arrival if L0 in 26ns ticks */

  int bcid_l0;               /* BCID from TTC Rx at the L0 arrival */
  int bcid_l1;               /* BCID from TTC Rx at the L1 arrival */

  int fake;                  /* if TRUE the sequence if FAKE */
};

/* After receiving L1 message */

struct ttc_it_seq_L0_L1_L1M {

  struct ttc_it_seq_L0_L1 l0_l1;  /* L0 - L1 preceding L1 message */

  int tick_l1m;          /* Time of arrival of L1M header in 26ns ticks */

  int nw;          /* Number of words of L1 message seen (Header + Data) */
  int header;      /* L1 message header, -1 if not seen */
  int data[4];     /* L1 data, only 4 words after header are written */

  int l1f;         /* TRUE if L1F errror has been detected */
};

/* After receiving L2a/L2r message, common to L2a and L2r */

enum ttc_it_l2_type { L2A, L2R };
struct ttc_it_seq_L0_L1_L1M_L2 {

  struct ttc_it_seq_L0_L1_L1M l0_l1;  /* L0-L1-L1m preceding this L2a/L2r */

  int tick_l2;

  enum ttc_it_l2_type type;  /* Was received L2a or L2r message? */

  int nw;               /* Number of words of L2a/L2r message received */
  int header;           /* L2a header or L2r word */
  int data[7];          /* L2a data, only 7 written, if L2r empty */

  int bcid_l2;          /* BCID from L2a/L2r message extracted from header */

  int l2f;              /* TRUE if L2F error has been detected */
};

struct ttc_it_seq_L1 {

  int l1_tick;          /* Tick time of arrival of L1 signal */
  int l1_bcid;          /* BCID from TTC Rx at the L1 signal arrival */

  int used_l1m;         /* TRUE, if this L1 has already been used in L1M */
  int used_roi;         /* TRUE, if this L1 has already been used in RoI */
};

struct ttc_it_seq_RoI {

  int l1_tick;            /* L1 tick info (if available) */
  int l1_bcid;            /* L1 BCID from TTC Rx */

  int RoI_tick;           /* RoI tick info (if available) */
  int hdr;                /* RoI header */
  int data[3];            /* RoI message */

  int roif;               /* TRUE if ROIF error took place */
};

/*
  Error treatment data structure and Internal errors detected
 */

#define TTCIT_ERR_INTL_NOTBC  0x1
#define TTCIT_ERR_INTL_BUG    0x2
#define TTCIT_ERR_INTL_L0L1   0x4
#define TTCIT_ERR_INTL_DOUT   0x8
#define TTCIT_ERR_INTL_SMALF  0x10

struct ttc_it_event_errors {

  unsigned long error;     /* Current errors (individual errors as bits) */

  unsigned long err_mask;  /* Error mas, stop errors have bit set */

  int err_case;       /* Error case, may help in locating internal errors */
  int err_info;       /* Some additional info about error, depends on error */
  int err_tick;       /* Tick position where and error has been detected 
			 May be usefull for postmortem analysis */

  int stop_on_error;       /* TRUE if stop on error was requested */

  int stop_analysis;      /* Stop further processing of SSM befause of
			     limited memory in BC FIFO, this should not lead
			     to stopping of the MONITOR, since BC FIFO and 
			     SSM does not match each other in SEQ mode */
};

struct error_collection {

  unsigned long error;     /* ERROR id */
  int tick;                /* Tick value at the moment of error detection */
};

/*
  A channel state
 */

enum ttc_it_A_channel_signal { SIGNAL_L0, SIGNAL_L1 };

/*
  B channels data structures
 */

enum data_strobe_machine_states { INACTIVE, READING };
enum data_strobe_signal { LOW, HIGH };
enum ttc_it_B_states { WAITING, HEADER, WORDS };
enum ttc_it_B_groups { G_NO, G_L1M, G_L2A, G_L2R, G_ROI, G_RES };
enum ttc_it_B_subtypes { T_HDR, T_DAT, T_RES };
enum ttc_it_B_ferrors { FER_L2F, FER_L1F, FER_ROIF };

/*
  Functions
 */

void ttc_it_clear_counters(struct ttc_it_counters *c);
void ttc_it_clear_errors(struct ttc_it_errors *e);
void ttc_it_clear_timing(struct ttc_it_times *t);
void ttc_it_clear_oldmes();
void ttc_it_clear_newmes();
void ttc_it_clear_printmes();
void ttc_it_clear_zeroos();

void ttc_it_add_counters(struct ttc_it_counters *src, 
			 struct ttc_it_counters *dest);
void ttc_it_add_errors(struct ttc_it_errors *src,
		       struct ttc_it_errors *dest);
void ttc_it_add_timing(struct ttc_it_times *src,
		       struct ttc_it_times *dest);

void ttc_it_print_counters(struct ttc_it_counters *c);
void ttc_it_print_errors(struct ttc_it_errors *e);
void ttc_it_print_timing(struct ttc_it_times *t);

void ttc_it_print_last_counters();
void ttc_it_print_last_errors();
void ttc_it_print_last_timing();
void ttc_it_collect_last_counterr(struct ttc_it_counters *cn,
				  struct ttc_it_errors *er);

void ttc_it_reset_options();

void ttc_it_set_opt_wins(int l0l1wind, int l0l1width,
			 int l1l1mtimeout, int l1l2timeout,
			 int bcwin, int bcwidth,
			 int l1roitimeout);
void ttc_it_print_opt_windows();
void ttc_it_set_l0_signal(int type);
void ttc_it_set_opt_const(int bcpp, int l2filter, int l2fil2);

int ttc_it_start_analyz();
void ttc_it_finish_analyz();
double ttc_it_mean_val(double val, double n);
int ttc_it_analyz_ssm();
int ttc_it_alloc_fifos();
void ttc_it_free_fifos();
void ttc_it_empty_fifos();

void ttc_it_ab_copy(struct ttc_it_A_B_channel *src, 
		   struct ttc_it_A_B_channel *dest);

void ttc_it_setstate_init(unsigned long SSmode);

void ttc_it_show_ABBC();
void ttc_it_show_errmask();
void ttc_it_get_next_error(char **s, int *tk, struct error_collection *ec);
int ttc_it_was_error();

/*
  State machines
 */
void ttc_it_A_machine(int tick, unsigned long word);
void ttc_it_B_machine(int tick, unsigned long word);
void ttc_it_AB_postprocess();
int ttc_it_assemble_seq();
void ttc_it_B_format_error(enum ttc_it_B_ferrors fer);

/*
  Helpers for ttcit_logic
 */
int soft_monitor_put_ab(struct ttc_it_A_B_channel *s);
int soft_monitor_put_bc(unsigned long *bc);
void soft_monitor_empty_abbc_fifos();
#ifdef TTCIT_EXTRA_DBG_PRINT_1__
void ttc_it_set_noprint(int what);
void ttc_it_set_prt_suppress();
void ttc_it_print_collections();
#endif

#ifdef _MONITOR_SCAN_BCID_DIFF__
int ttc_it_dirty_bcid_add(int clear);
void ttc_it_dirty_bcid_print();
#endif

/*
  Helper for communication with OnBoard monitor
 */
int get_SSM_ANALYZ_options(struct ttc_it_options *s);

/*
  Repeating error messages
 */

#define CANNA_ALLOC_FIFO(A,B) if(irc != 0){			    \
    printf("ttc_it_alloc_fifos: cannot allocate memory for %s\n",#A); \
    irc = B; \
    break; \
  }

#define _FIND_INTERNAL_ERROR

#ifdef _FIND_INTERNAL_ERROR
#define MARK_INTERNAL_ERROR(A) printf("ssm_analyz: INTERNAL ERROR NR %d\n",A)
#else
#define MARK_INTERNAL_ERROR(A) 
#endif

#define _IGNORE_INTERNAL_ERR_5 /* Ignore empty BC FIFO sooner than expected */

#endif /* SSM_ANALYZ_H__ */
