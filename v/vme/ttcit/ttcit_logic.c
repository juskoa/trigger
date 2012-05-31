#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "vmewrap.h"
#include "ttcit.h"

#include "ttcit_logic.h"
#include "ttcit_io.h"
#include "ssm_analyz.h"
#include "ttcit_om.h"
#include "ttcit_n.h"

/*
  Event errors set by ssm_analyz
 */

extern struct ttc_it_event_errors ssm_analyz_errors;

extern struct ttc_it_counters counters;
extern struct ttc_it_errors errores;
extern struct ttc_it_times timing;

extern struct ttc_it_counters sum_count;
extern struct ttc_it_errors sum_err;
extern struct ttc_it_times sum_time;

/*
  Snap Shot Memory buffer
 */

struct SSMbuffer SSM;
struct BCfifo bcnt;

struct logic_StatusInternal StatusInternal;

struct ttcit_io_event EVENT;

/*
  Some debug data
 */

struct TestFtStr {

  int after_last_read;
  int before_read_ssm;
  int after_read_ssm;
  int i_th_read;

};

static struct {

  int n_addr_read;
  int n_addr_funnies;
  int n_retries;
  int n_failed_fetches;
  int n_ssm_fetches;

#ifdef  _LOGIC_SSM_CANNOT_BE_FFFFFF__
  int n_ssm_ff;
  int n_ssm_retries;
  int n_data_failed_ssm;
#endif

  int n_set_retries;

  int min_dumloop;
  int max_dumloop;

} TFtStat;

#ifdef _MONITOR_SCAN_BCID_DIFF__
static struct {
  int doit;                  /* True if Dirty BCID handling requested */
  int samples;
  int maxloops;

  int error;
  int collected;
  int loops;
  int nErrors;
} DirtyBCID_control;
#endif

void reset_TTCit_logic(){

  /*
    Reset on board
   */

  VMEW32(RESET,0x0);

  /*
    Reset some buffers
   */
  SSM.top = 0;
}

void print_thread_error_message(const char *s, int irc){
  if(s == NULL){
    printf("Empty string, IRC = %d   ( %X   hex )\n",irc,irc);
  }else{
    printf("%s returned IRC = %d    ( %X    hex)\n",s,irc,irc);
  }
}

int init_TTCIT_logic(){
  int irc = 0;
  pthread_attr_t *p_att;

  do{

    /*
      Reset the internal state to default starting values
     */

    StatusInternal.mode.ul = TTCIT_IO_SSM_SCOPE;
    StatusInternal.nseq = 0;
    StatusInternal.nSCOPE = 1;
    StatusInternal.nSEQ = 0;
    StatusInternal.nLoops = 0;
    StatusInternal.nAnalyzd = 0;

    /*
      Set SSM status to SCOPE
     */
    set_SS_mode(0);

    /*
      Disable justDump flag by default
     */
    StatusInternal.justDump = FALSE;

    /*
      Soft monitor must be stopped when everything has started
     */
    StatusInternal.monitor.run = STOPPED;
    StatusInternal.monitor.th = NULL;
    StatusInternal.monitor.fd = -1;
    p_att = &StatusInternal.monitor.threadAttr;

    StatusInternal.monitor.create_file = FALSE;
    StatusInternal.monitor.online_diag = TRUE;

    StatusInternal.monitor.nSnapShots = 0;

    irc = pthread_attr_init(p_att);
    if(irc != 0){
      print_thread_error_message("pthread_attr_init\0",irc);
      break;
    }

    irc = pthread_attr_setdetachstate(p_att,PTHREAD_CREATE_DETACHED);
    if(irc != 0){
      print_thread_error_message("pthread_attr_init\0",irc);
      break;
    }
    printf("Thread attributes for the software MONITOR set successfully\n");

    /*
      Reset options for Software MONITOR
     */
    set_TTCIT_default_stops();

#ifdef _MONITOR_SCAN_BCID_DIFF__
    /*
      Initialize BCID Mismatch Analyzator
     */
    DirtyBCID_control.doit = FALSE;
    DirtyBCID_control.samples = 0;
    DirtyBCID_control.maxloops = 0;
    DirtyBCID_control.error = FALSE;
    DirtyBCID_control.collected = 0;
    DirtyBCID_control.nErrors = 0;
#endif
  }while(0);

  return irc;
}

void print_TTCIT_logic_state(){
  char *s = NULL;
  char Sseq[] = "SEQ\0";
  char Sscope[] = "SCOPE\0";
  char Sunknown[] = "UNKNOWN\0";
  char sRunning[] = "RUNNING\0";
  char sStopping[] = "STOPPING\0";
  char sStopped[] = "STOPPED\0";
  char sBug[] = "BUG impossible value\0";

  printf("TTCIT logic internal state\n");
  printf("\n");
  switch(StatusInternal.mode.ul){
  case TTCIT_IO_SSM_SCOPE:
    s = &Sscope[0];
    break;
  case TTCIT_IO_SSM_SEQ:
    s = &Sseq[0];
    break;
  default:
    s = &Sunknown[0];
    break;
  }
  printf("SSM mode = %8lX         %s\n",StatusInternal.mode.ul, s);
  printf("Nr. of sequences inspected = %lu\n",StatusInternal.nseq);
  printf("Count fractions of SSM sequences in SCOPE mode = %d\n",
	 StatusInternal.nSCOPE);
  printf("Count fractions of SSM sequences in SEQ mode = %d\n",
	 StatusInternal.nSEQ);
  printf("Number of loops remaining = %d\n",StatusInternal.nLoops);

  switch(StatusInternal.monitor.run){
  case RUNNING:
    s = &sRunning[0];
    break;
  case STOPPED:
    s = &sStopped[0];
    break;
  case STOPPING:
    s = &sStopping[0];
  default:
    s = &sBug[0];
    break;
  }
  printf("Stoft MONITOR status : %s\n",s);

}

void set_TTCIT_stop_conds(struct MONITOR_stops *s){
  size_t siz;
  void *dummy = NULL;

  do{
    if(s == NULL){
      printf("set_TTCIT_stop_conds: Canna set from NULL pointer\n");
      break;
    }
    siz = sizeof(struct MONITOR_stops);
    dummy = memcpy(&StatusInternal.stop, s, siz);
    if(dummy == NULL){
      printf("set_TTCIT_stop_conds: Canna copy stop conditions\n");
      print_TTCIT_stop_conds();
      break;
    }
    soft_monitor_set_errmask();
  }while(0);
}

void get_TTCIT_stop_conds(struct MONITOR_stops *s){
  size_t siz;
  void *dummy = NULL;

  do{
    if(s == NULL){
      printf("get_TTCIT_stop_conds: Canna return to NULL pointer\n");
      break;
    }
    siz = sizeof(struct MONITOR_stops);
    dummy = memcpy(s, &StatusInternal.stop, siz);
    if(dummy == NULL){
      printf("get_TTCIT_stop_conds: memcpy return NULL pointer \n");
      break;
    }
  }while(0);
}

void set_TTCIT_default_stops(){

  StatusInternal.stop.NSEQ = 0;    /* This disables condition */

  StatusInternal.stop.ANY_ERROR = FALSE;

  StatusInternal.stop.Stop_at_L0S = FALSE;
  StatusInternal.stop.Stop_at_L1S = FALSE;
  StatusInternal.stop.Stop_at_L1T = FALSE;
  StatusInternal.stop.Stop_at_L1M = FALSE;
  StatusInternal.stop.Stop_at_L1Mo = FALSE;
  StatusInternal.stop.Stop_at_L1F = FALSE;
  StatusInternal.stop.Stop_at_L2T = FALSE;
  StatusInternal.stop.Stop_at_L2Ts = FALSE;
  StatusInternal.stop.Stop_at_L2F = FALSE;
  StatusInternal.stop.Stop_at_BCID_diff = FALSE;

  soft_monitor_set_errmask();
}

void print_TTCIT_stop_conds(){
  const char *t;
  char *txt;
  int op = 0;
  int i;
  int num;

  printf("STOP conditions for Software MONITOR\n");
  printf("------------------------------------\n");
  printf(" \n");

  for(i = 1; i <= 12; i++){
    switch(i){
    case 1:
      num = StatusInternal.stop.NSEQ > 0;
      if(num){
	txt = "Sttop after processing %d sequences\n";
	op = StatusInternal.stop.NSEQ;
      }else{
	txt = "Stop after processing N sequences %s\n";
	t = "FALSE";
      }
      break;
    case 2:
      txt = "Stop at ANY error %s\n";
      t = StatusInternal.stop.ANY_ERROR ? "TRUE" : "FALSE";
      num = FALSE;
      break;
    case 3:
      txt = "Stop at error L0  %s\n";
      t = StatusInternal.stop.Stop_at_L0S ? "TRUE" : "FALSE";
      num = FALSE;
      break;
    case 4:
      txt = "Stop at error L1S %s\n";
      t = StatusInternal.stop.Stop_at_L1S ? "TRUE" : "FALSE";
      num = FALSE;
      break;
    case 5:
      txt = "Stop at errpr L1T %s\n";
      t = StatusInternal.stop.Stop_at_L1T ? "TRUE" : "FALSE";
      num = FALSE;
      break;
    case 6:
      txt = "Stop at error L1M %s\n";
      t = StatusInternal.stop.Stop_at_L1M ? "TRUE" : "FALSE";
      num = FALSE;
      break;
    case 7:
      txt = "Stop at error L1Mo %s\n";
      t = StatusInternal.stop.Stop_at_L1Mo ? "TRUE" : "FALSE";
      num = FALSE;
      break;
    case 8:
      txt = "Stop at error L1F %s\n";
      t = StatusInternal.stop.Stop_at_L1F ? "TRUE" : "FALSE";
      num = FALSE;
      break;
    case 9:
      txt = "Stop at error L2T %s\n";
      t = StatusInternal.stop.Stop_at_L2T ? "TRUE" : "FALSE";
      num = FALSE;
      break;
    case 10:
      txt = "Stop at error L2Ts %s\n";
      t = StatusInternal.stop.Stop_at_L2Ts ? "TRUE" : "FALSE";
      num = FALSE;
      break;
    case 11:
      txt = "Stop at error L2F %s\n";
      t = StatusInternal.stop.Stop_at_L2F ? "TRUE" : "FALSE";
      num = FALSE;
      break;
    case 12:
      txt = "Stop at error BC ID difference %s\n";
      t = StatusInternal.stop.Stop_at_BCID_diff ? "TRUE" : "FALSE";
      num = FALSE;
      break;
    default:
      break;
    }
    if(num){
      printf(txt,op);
    }else{
      printf(txt,t);
    }
  }
  printf(" \n");
  printf("Error mask = %lX\n",ssm_analyz_errors.err_mask);
  printf(" ---- \n");
}

void soft_monitor_set_errmask(){
  unsigned long e;

  e = TTC_IT_ERR_NONE;

  e |= StatusInternal.stop.Stop_at_L0S ? TTC_IT_ERR_L0S : 0;
  e |= StatusInternal.stop.Stop_at_L1S ? TTC_IT_ERR_L1S : 0;
  e |= StatusInternal.stop.Stop_at_L1T ? TTC_IT_ERR_L1T : 0;
  e |= StatusInternal.stop.Stop_at_L1M ? TTC_IT_ERR_L1M : 0;
  e |= StatusInternal.stop.Stop_at_L1Mo ? TTC_IT_ERR_L1MO : 0;
  e |= StatusInternal.stop.Stop_at_L1F ? TTC_IT_ERR_L1F : 0;
  e |= StatusInternal.stop.Stop_at_L2T ? TTC_IT_ERR_L2T : 0;
  e |= StatusInternal.stop.Stop_at_L2Ts ? TTC_IT_ERR_L2TS : 0;
  e |= StatusInternal.stop.Stop_at_L2F ? TTC_IT_ERR_L2F : 0;
  e |= StatusInternal.stop.Stop_at_BCID_diff ? TTC_IT_ERR_BCID : 0;

  e |= StatusInternal.stop.ANY_ERROR ? TTC_IT_ERR_ANY : 0;

  /*
    Always stop at internal error: This is a BUG and this can help
    to detect it
   */

  e |= TTC_IT_ERR_INTL;

  ssm_analyz_errors.err_mask = e;
}

int read_SSM_address(){
  int retval;
  retval = VMER32(READ_ADDR_COUNT);
  return retval;
}

int read_last_SSM_word(){
  int retval;
  retval = VMER32(READ_SNAPSHOT);
  return retval;
}

int fetch_SSM(){
  int retval = 0;

  int nw = 0;
  int i;

  int j;
  int expected;
  int ssm_val;
#ifdef  _LOGIC_SSM_CANNOT_BE_FFFFFF__
  int ssm_ff = 0xffffffff;
#endif
  int nloops = 100;
  int retry = 1000;


  do{

    SSM.top = 0;

    /*
      Read nr. of words stored in SSM
     */
    nw = read_SSM_address();
    if(nw < 0){
      TFtStat.n_addr_funnies++;
      for(j = 0; j < retry; j++){
	my_dummy_loop(nloops);
	nw = read_SSM_address();
	TFtStat.n_retries++;
	if(nw >= 0){
	  break;
	}
      }
      if(j >= retry){
	TFtStat.n_failed_fetches++;
	nw = -1;
	SSM.top = -1;
	goto last_one;                /* Unrecoverable, SSM Fetch FAILED */
      }
    }
    if(nw == 0){
      retval = nw;
      printf("fetch_SSM: Empty SSM\n");
      SSM.top = 0;
      break;
    }

    SSM.top = 0;
    expected = nw;
    for(i = nw-1; i >= 0; i-- ){
      ssm_val = read_last_SSM_word();

#ifdef  _LOGIC_SSM_CANNOT_BE_FFFFFF__

      if(ssm_val == ssm_ff){
	TFtStat.n_ssm_ff++;
	for(j = 0; j < retry; j++){
	  my_dummy_loop(nloops);
	  ssm_val = read_last_SSM_word();
	  TFtStat.n_ssm_retries++;
	  if(ssm_val != ssm_ff){
	    break;
	  }
	}
	if(j >= retry){
	  TFtStat.n_data_failed_ssm++;
	  SSM.top = -1;
	  nw = -1;
	  goto last_one;         /* UNRECOVERABLE, SSM Fetch FAILED */
	}
      }

#endif

      nw = read_SSM_address();
      if(nw != (--expected)){   /* NW changed after read in a funny way */
	TFtStat.n_addr_funnies++;
	for(j = 0; j < retry; j++){
	  my_dummy_loop(nloops);
	  nw = read_SSM_address();
	  TFtStat.n_retries++;
	  if(nw == expected){
	    break;
	  }
	}
	if(i >= retry){
	  TFtStat.n_failed_fetches++;
	  SSM.top = -1;
	  nw = -1;
	  goto last_one;    /* Cannot recover, SSM Fetch FAILED */
	}
      }

      SSM.buf[i] = ssm_val;
      SSM.top++;
    }

    /* NW after the last read must show 0 */
    if(nw != 0){
      printf("ERROR::: SSM not fully cleared, SSM dump incomplete\n");
    }

  last_one:
    retval = SSM.top;
  }while(0);

  return retval;
}

void dump_SSM(int nw){
  int n = 0;

  do{

    n = SSM.top;
    if(n <= 0){
      printf("dump_SSM: No data in SSM\n");
      break;
    }
    if(nw == 0){
      nw = n + 1;
    }
    if(n > nw){
      n = nw;
    }

    dump_SS_buffer(SSM, n);

  }while(0);
}

void dump_SS_buffer(struct SSMbuffer S, int nw){
  int i;
  unsigned long word;
  unsigned long word_old = 0xdeadabcd;
  int ndump;

#ifdef TTCIT_OLD_SEQ_FORMAT__

  unsigned char l1accept;
  unsigned char doutstr;
  unsigned char subadr;
  unsigned char dout;


  char Sicount[] = "Icount\0";
  char Saddrs[] = "(Addr)\0";
  char Sl1acc[] = "L1\0";
  char Sdoutstr[] = "DoStr\0";
  char Ssubadr[] = "SUBADR\0";
  char Sdata[] = "DATA\0";
  char Sword[] = "WORD\0";

#endif /* TTCIT_OLD_SEQ_FORMAT__ */

#ifdef TTCIT_TRIGGER_SEQ_FORMAT__

  int tick, A, Strobe, addr, data;

#endif /* TTCIT_TRIGGER_SEQ_FORMAT__ */

  do{
    ndump = nw;
    if(ndump <= 0){
      ndump = S.top;
    }
    if(S.top < ndump){
      ndump = S.top;
    }

    printf("Dumping SSM data - Snap Shot memory NDATA = %d\n",ndump);

#ifdef TTCIT_OLD_SEQ_FORMAT__
    
    printf("%7s  %6s  %2s  %5s  %6s  %4s     %s\n",
	   Sicount, Saddrs, Sl1acc, Sdoutstr, Ssubadr, Sdata, Sword);

#endif /* TTCIT_OLD_SEQ_FORMAT__ */

    if(ndump <= 0){
      printf("SSM buffer empty - nothing to print\n");
      break;
    }

#ifdef TTCIT_TRIGGER_SEQ_FORMAT__

    printf("SNAP SHOT MEMORY DUMP\n");
    printf("-------------------------------\n");
    printf(" \n");
    printf("Tick         A       Data      TTC       TTC\n");
    printf("          Channel   Strobe    Address   data\n");

    word_old = ~S.buf[0];
    for(i=0; i<ndump; i++){
      word = S.buf[i];
      A = ((word & MASK_L1ACCEPT) >> RSH_L1ACCEPT);
      /*
	Print all words that are not just repetition of the previous ones,
	but print also ALL WORDS WITH A BIT 1 IN CHANNEL A
       */
      if((word != word_old) || (A != 0)){
	tick = i;
	Strobe = ((word & MASK_DOUTSTR) >> RSH_DOUTSTR);
	addr = ((word & TTC_ADDR_MASK) >> TTC_ADDR_RSH);
	data = (word & TTC_DATA_MASK);
	printf("%07d       %1X         %1X        %1X     %3X\n",
	       tick,A,Strobe,addr,data);
	word_old = word;
      }
    }

#endif /* TTCIT_TRIGGER_SEQ_FORMAT__ */

#ifdef TTCIT_OLD_SEQ_FORMAT__

    word_old = ~S.buf[0];          /* safely different from 1-st word */
    for(i = 0; i < ndump; i++){
      word = S.buf[i];
      l1accept = (word & MASK_L1ACCEPT) >> RSH_L1ACCEPT;
      doutstr = (word & MASK_DOUTSTR) >> RSH_DOUTSTR;
      subadr = (word & MASK_SUBADR) >> RSH_SUBADR;
      dout = (word & MASK_DOUT);
      if((word != word_old)||(l1accept != 0)){
	printf("%07d  %6X   %1X    %1X      %2X     %2X       %8X\n",
	       i,i,l1accept,doutstr,subadr,dout,word);
      }
      word_old = word;
    }

#endif /* TTCIT_OLD_SEQ_FORMAT__ */

  }while(0);
}

void dump_raw_SSM_buffer(int begin, int end){
  int i;
  int start,finish;
  int n_dumped = 0;
  int word;
  int tick, A, strobe, addr, data;

  start = begin;
  finish = end;

  printf("RAW dump SSM\n\n");
  if(begin < 0){
    printf("Starting address error, %d replaced with 0\n",begin);
    start = 0;
  }
  if(end < 0){
    printf("Wrong last address %d : NO ACTION\n",end);
    goto last_one;
  }
  if(end < begin){
    printf("Upper limit %d less than the lower one %d - NO ACTION\n",
	   end, begin);
    goto last_one;
  }
  if(end > SSM.top){
    printf("Requested upper limit %d larger than nr. of data stored %d\n",
	   end, SSM.top);
    finish = SSM.top;
    printf(" ............ resetting to %d\n", finish);
  }

  printf("SNAP SHOT MEMORY DUMP\n");
  printf("-------------------------------\n");
  printf(" \n");
  printf("Tick         A       Data      TTC       TTC\n");
  printf("          Channel   Strobe    Address   data\n");

  for(i = start; i <= finish; i++){
    word = SSM.buf[i];
    if(word == 0x0){
      continue;
    }
    tick = i;
    A = ((word & MASK_L1ACCEPT) >> RSH_L1ACCEPT);
    strobe = ((word & MASK_DOUTSTR) >> RSH_DOUTSTR);
    addr = ((word & TTC_ADDR_MASK) >> TTC_ADDR_RSH);
    data = (word & TTC_DATA_MASK);
	printf("%07d       %1X         %1X        %1X     %3X\n",
	       tick,A,strobe,addr,data);
	n_dumped++;
  }
  printf("-- \n");

 last_one:
  printf("RAW dump SSM: %d nonzero words dumped \n",n_dumped);

}

void fetch_print_SS_bc(){
#define TTCIT_CLONE_FETCH_PRINT_SS__
#define TTCIT_CLONE_FETCH_PRINT_SS_1__
#include "ttcit_replicas.h"
#undef TTCIT_CLONE_FETCH_PRINT_SS_1__
}

void clear_SS_memory_on_board(){
  int i;

  /*
    1) Reset the SSM address counter to 0
   */
  VMEW32(RESET_SNAPSHOT_N,0x1);

  /*
    2) Fill all the SSM with 0's
   */
  for(i = 0; i < MEGA; i++){
    VMEW32(READ_SNAPSHOT,0x0);
  }
  VMEW32(RESET_SNAPSHOT_N,0x1);
}

void test_clear_SSM_on_board(){
  int i;
  int n_nonzero_init = 0;
  int n_nonzero_clrd = 0;
  int seen = 0;
  int dato;

  /*
    Check that the contents of the memory is Not ZERO
   */
  seen = 0;
  for(i = 0; i < MEGA; i++){
    dato = VMER32(READ_SNAPSHOT);
    seen++;
    if(dato != 0x0){
      n_nonzero_init++;
    }
  }
  printf("TestClearSSM: Nonzero count BEFORE = %d   seen = %d\n",
	 n_nonzero_init, seen);

  /*
    Clear the SSM
   */
  ClearSSM();

  /*
    Count nonzero's after clear operation
   */
  seen = 0;
  for(i = 0; i < MEGA; i++){
    dato = VMER32(READ_SNAPSHOT);
    seen++;
    if(dato != 0x0){
      n_nonzero_clrd++;
    }
  }
  printf("TestClearSSM: Nonzero count AFTER  = %d   seen = %d\n",
	 n_nonzero_clrd, seen);
  if(n_nonzero_clrd == 0){
    printf("TestClearSSM: SUCCESS!\n");
  }else{
    printf("TestClearSSM: FAILURE!\n");
  }

}

void clear_SS_memory_buffer(){
  int i;
  SSM.top = 0;
  for(i = 0; i < MEGA; i++){
    SSM.buf[i] = 0;
  }
}

int get_SS_top_mem_buffer(){
  return SSM.top;
}

int write_SS_buffer(){
  int retval = -1;
  char fname[] = TTCIT_SSDUMP_FILE;
  int fd = 0;
  int buf[BFSIZ];
  int datasiz;
  int bytes = 0;
  int nbuf = 0;
  int i,j;
  int pntr, pnss;
  int ncopy, rest, written_bytes, toWrite, wb;
  int irc;
  int words = 0;

  bytes = sizeof(int);

  do{
    /*
      If nothing is stored in SSM buffer, nothing can be written
     */
    if(SSM.top <= 0){
      printf(
	     "write_SS_buffer(): Nothing stored in the buffer, nothing written\n");
      break;
    }

    /*
      Open the file: User can READ, WRITE and EXECUTE the file
                     S_IRXWU tam byt musi, ak 
     */
    fd = open(fname,O_WRONLY | O_CREAT, S_IRWXU );

    /*
      Split data sample into buffers and write each buffer.
      Do not forget to put the data size first
     */
    datasiz = SSM.top;
    rest = datasiz;
    nbuf = 1 + (datasiz + 1) / BFSIZ;
    pnss = 0;
    for(i = 0; i < nbuf; i++){
      pntr = 0;
      if(i == 0){
	buf[pntr++] = datasiz * bytes;
	ncopy = BFSIZ - 1;
      }else{
	ncopy = BFSIZ;
      }
      if(ncopy > rest){
	ncopy = rest;
      }
      for(j = 0; j < ncopy; j++){
	buf[pntr++] = SSM.buf[pnss++];
      }
      toWrite = pntr * bytes;
      wb = write(fd, &buf[0], toWrite);
      switch(wb){
      case 0:
	printf("No more data to write????\n");
	break;
      case -1:
	printf("WRITE error - SSM data on disk may be corrupt\n");
	break;
      default:
	written_bytes += wb;
	rest = datasiz - written_bytes;
	break;
      }
    }
    words = written_bytes / bytes;
    printf("%d bytes written to ssm_dump.dat, INT WORDS %d\n",
	   written_bytes, words);

    /*
      Close the file
     */
    irc = close(fd);
    switch(irc){
    case -1:
      printf("Error closing ssm_dump.dat file - SSM on disk may be corrupt\n");
      retval = -1;
      break;
    case 0:
      printf("File ssm_dump.dat closed\n");
      retval = written_bytes;
      break;
    default:
      printf("I should not be there, IRC from close = %d\n",irc);
      retval = -999;
      break;
    }
    
  }while(0);

  return retval;
}

void throw_SS_data(int addr){

  int inSSM = 0;
  int toSkip = 0;
  int i;
  int word;

  do{

    inSSM = read_SSM_address();  /* Current address in the SSM */
    if(inSSM < addr){
      printf("Not enough words in SSM\n");
      break;
    }
    toSkip = inSSM - addr -1;
    for(i = 0; i < toSkip; i++){
      word = read_last_SSM_word();
    }

  }while(0);
}

void set_SS_mode(int mode){

  switch(mode){
  case 0:
#ifdef SIMVME
    VMEW32(TTCIT_CONTROL, SSM_SET_MODE_SCOPE);
#else
    /*
    printf("The code for setting the SSM mode has not yet been written\n");
    */
#endif
    StatusInternal.mode.ul = TTCIT_IO_SSM_SCOPE;
    break;
  case 1:
#ifdef SIMVME
    VMEW32(TTCIT_CONTROL, SSM_SET_MODE_SEQ);
#else
    /*
    printf("The code for setting the SSM mode has not yet been written\n");
    */
#endif
    StatusInternal.mode.ul = TTCIT_IO_SSM_SEQ;
    break;
  default:
    printf("Unknown SSM mode - keeping the last set\n");
    break;
  }
}

int get_SS_mode(){
  int retval = -1;

  switch(StatusInternal.mode.ul){
  case TTCIT_IO_SSM_SCOPE:
    retval = SSM_SCOPE;
    break;
  case TTCIT_IO_SSM_SEQ:
    retval = SSM_SEQ;
    break;
  default:
    printf("Unknown code, BUG IN TTCIT CODE, Contact experts\n");
    retval = 9999;
    break;
  }

  return retval;
}

int get_nr_words_bcnt_fifo(){
  int retval = 0;
  retval = VMER32(N_BCNT_TTCRX);
  return retval;
}

int fetch_bcnt_fifo(){
  int retval = -1;

  int i;
  int nword;

  do{
    /*
      Clear the FIFO buffer
     */
    for(i = 0; i < BCNT_FIFO_SIZ; i++){
      bcnt.buf[i] = 0;
    }
    bcnt.top = 0;

    /*
      Get the number of words stored in BCNT fifo
     */
    nword = get_nr_words_bcnt_fifo();
    if(nword <= 0){
      printf("No data in BCNT TTCRx FIFO - nothing fetched\n");
      retval = 0;
      break;
    }

    /*
      Move the data from FIFO to memory
     */
    for(i = 0; i < nword; i++){
      bcnt.buf[i] = VMER32(BCNT_TTCRX);
    }
    bcnt.top = nword;
    retval = bcnt.top;

  }while(0);

  return retval;
}

void print_bcnt_fifo_buf(int nw){

  int word, old, nprinted;
  int i;
  int nPrint = 0;

  char Sn[] = " N  \0";
  char Saddr[] = "(Addr)\0";
  char SBChex[] = "BC (hex)\0";
  char SBCdec[] = "BC (dec)\0";

  do{
    /*
      If no words has been stored inna fifo buffer, do not print anything
     */
    if(bcnt.top <= 0){
      printf("BC fifo buffer empty\n");
      break;
    }
    switch(nw){
    case 0:
      nPrint = bcnt.top;
      break;
    default:
      nPrint = nw;
      if(nPrint > bcnt.top){
	nPrint = bcnt.top;
      }
    }

    /*
      Print only differing words, save paper
     */
    printf("Dumping %d words from BC FIFO buffer\n",nPrint);
    printf("%4s  %6s  %8s  %8s\n",Sn,Saddr,SBChex,SBCdec);
    old = ~bcnt.buf[0];              /* Make sure it is different */
    nprinted = 0;
    for(i = 0; i < nPrint; i++){
      word = bcnt.buf[i];
      nprinted++;
      if((word != old)||(nprinted == nPrint)){
	printf("%04d    %3X   %8X  %d\n",i,i,word,word);
      }
      old = word;
    }

  }while(0);
}

int soft_monitor_start(int nScope, int nSeq, int nLoops){
  int irc = 0;

  char SfileName[] = TTCIT_MONITOR_FILE;
  struct ttcit_io_header IOheader;
  int i;

  do{
    /*
      If the soft monitor is already running do not start it again.
      If not running, initialize it for running
    */
    switch(StatusInternal.monitor.run){
    case RUNNING:
      printf("Soft MONITOR already running, stop it first\n");
      irc = 1;
      break;
    case STOPPING:
      printf("Soft MONITOR terminating, wait for full STOP\n");
      irc = 2;
      break;
    case STOPPED:
      StatusInternal.nseq = 0;
      StatusInternal.nSCOPE = nScope;
      StatusInternal.nSEQ = nSeq;
      if(nLoops > 0){
	StatusInternal.nLoops = nLoops;
      }else{
	StatusInternal.nLoops = StatusInternal.stop.NSEQ;
      }
      StatusInternal.nAnalyzd = 0;
      StatusInternal.monitor.nSnapShots = 0;
      StatusInternal.monitor.fd = -1;
      break;
    default:
      printf("soft_monitor_status is wrong state - BUG, call expert\n");
      irc = -9999;
      break;
    }
    if(irc != 0){
      break;
    }

    /*
      Opening the I/O file, and write header - if requested
     */
    if(StatusInternal.monitor.create_file){
      if(StatusInternal.monitor.fd != -1){
	printf("soft_monitor_start: I/O file in use, closing it\n");
	i = ttcit_io_close(StatusInternal.monitor.fd);
	if(i == -1){
	  printf("soft_monitor_start: Cannot close I/O file\n");
	  irc = -111;
	  break;
	}
      }
      StatusInternal.monitor.fd = ttcit_io_open(&SfileName[0], TTCIT_IO_WRITE);
      if(StatusInternal.monitor.fd == -1){
	printf("soft_monitor_start : Cannot open %s for WRITE, cannot start\n",
	       &SfileName[0]);
	irc = 11;
	break;
      }

      i = ttcit_io_write_header(StatusInternal.monitor.fd, &IOheader);
      if(i <= 0){
	printf("soft_monitor_start: Cannot write header IRC = %d\n",i);
	irc = 12;
	break;
      }
    }

    /*
      If you want to use on-line diagnostics, 

      1) Initialize the SSM_ANALYZ package (create FIFO's)

      2) clear all trigger counts and error counts

      3) Make sure all trigger masks are set properly
     */

    if(StatusInternal.monitor.online_diag){

#ifdef TTCIT_EXTRA_DBG_PRINT_1__
      ttc_it_set_prt_suppress();
#endif
      i = ttc_it_start_analyz();  

      /* Clear counters */
      ttc_it_clear_counters(&sum_count);
      ttc_it_clear_errors(&sum_err);
      ttc_it_clear_timing(&sum_time);

      i = ttc_it_alloc_fifos();
      if(i != 0){
	printf("soft_monitor_start: Cannot allocate FIFO's IRC = %d\n",i);
	irc = 51;
	break;
      }

    }

    /*
      Start looping thread and let it loop
     */
    i = pthread_create(&StatusInternal.monitor.monitorThread,
		       &StatusInternal.monitor.threadAttr,
		       soft_monitor_loop,NULL);
    if(i != 0){
      printf("Cannot start MONITOR thread, IRC = %d\n",i);
      printf("Soft MONITOR is NOT running \n");
      irc = 21;
      break;
    }

  }while(0);

  return irc;
}

void *soft_monitor_loop(void *dummy){

  int tooMany = INT_MAX;
  int looped = 0;
  int toLoop = 0;
  enum { SCOPE, SEQ, INIT } ssmMode;
  int i;
  int terminate;
  int nreads;
  unsigned long ss = 0;
  int irc;
  int jwait;

  struct ttcit_io_footer footer;

  do{
    if(StatusInternal.monitor.run == RUNNING){
      printf("soft_monitor_loop : Invalid state, LOOP TERMINATED\n");
      break;
    }

    StatusInternal.monitor.run = RUNNING;

    /*
      Loop logic - basic loop is to loop forever
     */
    if(StatusInternal.nLoops > 0){
      toLoop = StatusInternal.nLoops;
    }else{
      toLoop = tooMany;
    }
    StatusInternal.nLoops = toLoop;
    looped = 0;
    ssmMode = INIT;
    do{
      /*
	A loop is: nScope times in SSM mode SCOPE
                   nSeq times in SSM mode Seq
       */
      do{
	switch(ssmMode){
	case INIT:
	  ssmMode = SCOPE;  /* Start with SCOPE mode */
	  set_SS_mode(0);
	  ss = TTCIT_IO_SSM_SCOPE;
	  terminate = 0;
	  nreads = StatusInternal.nSCOPE;
	  break;
	case SCOPE:
	  ssmMode = SEQ;    /* After SCOPE set to SEQ mode */
	  set_SS_mode(1);
	  ss = TTCIT_IO_SSM_SEQ;
	  terminate = 0;
	  nreads = StatusInternal.nSEQ;
	  continue;                     /* SEQ mode excluded */
	  break;
	case SEQ:
	  ssmMode = INIT;   /* After SEQ mode, set to init terminate loop */
	  terminate = 1;
	  ss = 0;
	  nreads = 0;
	  break;
	default:
	  printf("soft_monitor_loop: If you see this BUG BUG BUG !!!\n");
	  break;
	}
	for(i = 0; i < nreads; i++){

	  jwait = wait_till_SSM_full(0);
	  if(jwait != 0){
	    printf("SSM cannot reach full state - exiting loop\n");
	    terminate = 1;
	    break;
	  }

	  irc = ttc_it_start_analyz();
	  irc = ttc_it_alloc_fifos();

	  irc = fetch_SSM();
	  if(irc < 0){
	    printf("SSM returned bogus data,  IRC = %d \n",irc);
	    terminate = 1;
	    break;
	  }

	  irc = fetch_bcnt_fifo();

	  if(StatusInternal.monitor.online_diag){
	    irc = soft_monitor_diagnoze(&SSM, &bcnt);
	    if(irc != 0){
	      printf("Cannot perform on-line diagnostics IRC = %d\n",irc);
	      terminate = 1;
	      break;
	    }
	    ttc_it_finish_analyz();
	    soft_monitor_empty_abbc_fifos();
	    irc = soft_monitor_fill_ABC(&SSM, &bcnt);
	  }

	  if(StatusInternal.monitor.create_file){
	    /*
	      Assemble and write one event : If the MONITOR is STOPPING
	      drop the last SSM (it can be corrupted and analysis may lead to
	      some false error reports)
	    */
	    if(StatusInternal.monitor.run == RUNNING){
	      irc = ttcit_io_make_event(StatusInternal.monitor.fd,
					&EVENT, &SSM, ss, &bcnt);
	      if(irc <= 0){
		printf("ERROR writing a new event, IRC = %d\n",irc);
		terminate = 1;
		break;
	      }
	    }
	  }

	  StatusInternal.monitor.nSnapShots++;

	  if(StatusInternal.monitor.run == STOPPING){ /* Requested STOP */
	    terminate = 1;
	  }

	}

      }while(terminate == 0);

      if(StatusInternal.stop.NSEQ > 0){
	looped++;  /* Another loop successfully finished */
	StatusInternal.nLoops--;
      }

    }while((looped < toLoop) && (StatusInternal.monitor.run != STOPPING));

    /*
      All loops were performed or STOPPING was requested

      Write footer, close the file, set stop and exit
     */
    if(StatusInternal.monitor.create_file){

      irc = ttcit_io_write_footer(StatusInternal.monitor.fd, &footer );
      if(irc < 0){
	printf("Cannot write FOOTER, IRC = %d\n",irc);
      }

      irc = ttcit_io_close( StatusInternal.monitor.fd );
      StatusInternal.monitor.fd = -1;
      if(irc < 0){
	printf("Error on closing the IO file, IRC = %d\n",irc);
      }
    }

    if(StatusInternal.monitor.online_diag){
      /*
	Print the final statistics and dump the last SSM 
       */
#ifdef TTCIT_EXTRA_DBG_PRINT_1__
      ttc_it_set_noprint(TRUE);
      ttc_it_print_collections();
#endif
      ttc_it_print_counters(&sum_count);
      ttc_it_print_errors(&sum_err);
      ttc_it_print_timing(&sum_time);
      ttc_it_show_ABBC();
 
    }

  }while(0);

  StatusInternal.monitor.run = STOPPED;
  pthread_exit(NULL);
  return NULL;
}

void soft_monitor_stop(){

  unsigned long msec26 = 26000;  /* 26 milisecond */
  unsigned long mySleep = 0;  
  unsigned long totSleep = 0;
  unsigned long maxSleep = 1000000; /* 1 sec */
  float fslep;

  switch(StatusInternal.monitor.run){
  case RUNNING:
    printf("Stopping the Soft MONITOR, please wait...\n");
    StatusInternal.monitor.run = STOPPING;
    while(StatusInternal.monitor.run != STOPPED){
      usleep(msec26);
      totSleep += msec26;
      mySleep += msec26;
      if(mySleep >= maxSleep){
	fslep = ((float)totSleep) / 1000000.;
	mySleep = 0;
	printf("Waiting for MINITOR to stop, already slept %f sec\n",fslep);
      }
    }
    printf("Soft MONITOR stopped successfully after %lu snapshots\n",
	   StatusInternal.monitor.nSnapShots);
    break;
  case STOPPED:
    printf("Soft MONITOR has already been stopped, no action taken\n");
    break;
  default:
    printf("soft_monitor_status BUG. I canna be in this state\n");
    break;
  }

}

int soft_monitor_status(){
  int irc = -1;

  switch(StatusInternal.monitor.run){
  case STOPPED:
    irc = MON_STOPPED;
    break;
  case RUNNING:
    irc = MON_RUNNING;
    break;
  case STOPPING:
    irc = MON_STOPPING;
    break;
  default:
    irc = 0xbad;
    break;
  }

  return irc;
}

unsigned long soft_monitor_snapshots(){
  return StatusInternal.monitor.nSnapShots;
}

void soft_monitor_file(int what){
  char *Sans = NULL;
  char Son[] = "ON\0";
  char Soff[] = "OFF\n";
  
  switch(what){
  case 0:
    StatusInternal.monitor.create_file = FALSE;
    break;
  case 1:
    StatusInternal.monitor.create_file = TRUE;
    break;
  case 2:
    Sans = StatusInternal.monitor.create_file ? &Son[0] : &Soff[0];
    printf("Soft MONITOR create file = %s\n",Sans);
    break;
  default:
    printf("If you see this, you are witnessing a BUG!\n");
    break;
  }
}

void soft_monitor_online_diag(int what){
  char *Sans = NULL;
  char Son[] = "ON\0";
  char Soff[] = "OFF\n";

  switch(what){
  case 0:
    StatusInternal.monitor.online_diag = FALSE;
    break;
  case 1:
    StatusInternal.monitor.online_diag = TRUE;
    break;
  case 2:
    Sans = StatusInternal.monitor.online_diag ? &Son[0] : &Soff[0];
    printf("Soft MONITOR on-line diagnostics is %s\n",Sans);
    break;
  default:
    printf("If you see this you are witnessing a BUG!\n");
    break;
  }
}

/*
  On-line diagnostics

  returns:
              0 = O.K.
             >0   some error
 */
int soft_monitor_diagnoze(struct SSMbuffer *S, struct BCfifo *BC){
  int irc = 0;

  int big_error = FALSE;
  int stopCond = FALSE;

  do{

    /*
      Set the SSM analyzer into starting position
    */
    ttc_it_setstate_init(StatusInternal.mode.ul);

    /*
      Put A and BC into fifos
    */
    big_error = soft_monitor_fill_ABC(S, BC);
    if(big_error){
      break;
    }

    /*
      Now we can start to analyze this snapshot
    */
    StatusInternal.nAnalyzd++;

    irc = ttc_it_analyz_ssm();
    if(irc != 0){
      printf("SSM analyzer finished with error IRC = %d\n",irc);
      irc = 4;
      break;
    }
    stopCond = ssm_analyz_errors.stop_on_error;
    if(StatusInternal.stop.NSEQ > 0){
      stopCond = (stopCond || 
		  (StatusInternal.nAnalyzd >= StatusInternal.stop.NSEQ));
    }

    if(stopCond){ /* Stop_at_ERROR condition reached */
      StatusInternal.monitor.run = STOPPING;
      /*
	Store the last SSM and BC contents for postmortem inspection

	AB_fifo and BC_fifo now can be dumped and inspected
       */
      /*
	This is now responsibility of caller

      soft_monitor_empty_abbc_fifos();
      big_error = soft_monitor_fill_ABC(S, BC);
      */
      break;
    }

  }while(0);
  return irc;
}

int soft_monitor_fill_ABC(struct SSMbuffer *S, struct BCfifo *BC){
  int big_error;
  int irc = 0;

  int i;
  unsigned long word, old;
  struct ttc_it_A_B_channel ab;

  big_error = FALSE;

  do{

    /*
      Put BC into BC fifo
    */
    for(i = 0; i < BC->top; i++){
      word = BC->buf[i];
      irc = soft_monitor_put_bc(&word);
      big_error = (irc != 0);
      if(big_error){
	irc = 1;
	break;
      }
    }
    if(big_error){
      break;
    }

    /*
      SSM should contain every word that has at least one bit set

      1-st L0 which is not stored into SSM is added with tick[0] = -1
    */

    ab.ticks = -1;
    ab.s = MASK_L1ACCEPT;
    irc =  soft_monitor_put_ab(&ab);
    big_error = (irc != 0);
    if(big_error){
      irc = 2;
      break;
    }

    old = (S->top > 0) ? ~S->buf[0] : 0xadadead;
    for(i = 0; i < S->top; i++){
      word = S->buf[i];
      if(word != 0x0){      /* If at least 1 bit set - store it */
	ab.ticks = i;
	ab.s = word;
	irc = soft_monitor_put_ab(&ab);
	big_error = (irc != 0);
	if(big_error){
	  irc = 3;
	  break;
	}
	old = word;
      }
    }

  }while(0);

  return big_error;
}

void soft_monitor_single_ssm(int what){
#define TTCIT_CLONE_SM_SINGLE_SSM__
#define TTCIT_CLONE_SM_SINGLE_SSM_1__
#include "ttcit_replicas.h"
#undef TTCIT_CLONE_SM_SINGLE_SSM_1__
}

void soft_monitor_N_ssm(int n){
  int i;
  int found_error;
  int decd = 1;
  int modull;

  ttc_it_clear_counters(&sum_count);
  ttc_it_clear_errors(&sum_err);
  ttc_it_clear_timing(&sum_time);

  found_error = FALSE;
  for(i=0; i<n; i++){
    /*
      Run one SSM analyz
     */
#ifdef _MONITOR_SCAN_BCID_DIFF__
    /*
      Do not forget to reset error flag before starting a new sample
     */
    DirtyBCID_control.error = FALSE;
#endif

    soft_monitor_single_ssm(FALSE);
#ifdef _MONITOR_SCAN_BCID_DIFF__
    DirtyBCID_control.loops++;
#endif
    modull = i % decd;
    if(modull == 0){
      printf("soft_monitor_N_ssm :    ....   %d\n",i);
    }
    modull = decd * 10;
    if(i >= modull){
      decd *= 10;
    }
    found_error = ttc_it_was_error();
    if(found_error){
#ifdef _MONITOR_SCAN_BCID_DIFF__
      if(DirtyBCID_control.doit){
	DirtyBCID_control.error = TRUE;
	DirtyBCID_control.nErrors++;
      }else{
	break;
      }
#else
      break;
#endif
    }
#ifdef _MONITOR_SCAN_BCID_DIFF__

    ttc_it_dirty_bcid_add(0);
    DirtyBCID_control.collected++;
    if(DirtyBCID_control.collected >= DirtyBCID_control.samples){
      printf("Requested number %d of SSM samples collected %d \n",
	     DirtyBCID_control.samples, DirtyBCID_control.collected);
      break;
    }

#endif
  }
#ifdef TTCIT_EXTRA_DBG_PRINT_1__
  ttc_it_set_noprint(TRUE);
  ttc_it_print_collections();
#endif
  ttc_it_print_counters(&sum_count);
  ttc_it_print_errors(&sum_err);
  ttc_it_print_timing(&sum_time);
  ttc_it_show_ABBC();
}

void soft_monitor_dumpfile(){

  StatusInternal.justDump = TRUE;

  soft_monitor_rescan(0);

  StatusInternal.justDump = FALSE;
}

int BCID_mismatch_finder(int nSamples, int nMaxrep){
  int irc = 0;

#ifdef _MONITOR_SCAN_BCID_DIFF__


  DirtyBCID_control.doit = TRUE;
  DirtyBCID_control.samples = nSamples;
  DirtyBCID_control.maxloops = nSamples * nMaxrep;
  DirtyBCID_control.collected = 0;
  DirtyBCID_control.error = FALSE;
  DirtyBCID_control.nErrors = 0;
  DirtyBCID_control.loops = 0;

  irc = ttc_it_dirty_bcid_add(1);  /* Create histograms */
  if(irc < 0){
    printf("Cannot allocate memory for histograms, IRC = %d \n",irc);
    irc = -2;
    goto last_order;
  }

  soft_monitor_N_ssm(DirtyBCID_control.maxloops);

  if(DirtyBCID_control.collected < DirtyBCID_control.samples){
    printf("Requested number %d of samples %d not accumltated, %d errors\n",
	   DirtyBCID_control.samples, DirtyBCID_control.collected,
	   DirtyBCID_control.nErrors);
  }
  if(DirtyBCID_control.collected <= 0){
    printf("RESULTS of BCID Micmatch Scan ARE USELESS\n");
    irc = -1;
    goto last_order;
  }

  printf("BCID Mismatch Analyzer Encountered %d Errorneous Snap Shots\n",
	 DirtyBCID_control.nErrors);
  printf("\n");
  ttc_it_dirty_bcid_print();
  ttc_it_dirty_bcid_add(2);  /* Delete histograms */

 last_order:
  DirtyBCID_control.doit = FALSE;
#else
    printf("BCID Mismatch analysis has not been compiled in\n");
#endif

  return irc;
}

void soft_monitor_rescan(int action){ /* action = 0 : Open/Close
					        = 1 : Open/Don't close
						= 2 : Don't open/Don't close
				      */
  int irc;

  char Sfilename[] = TTCIT_MONITOR_FILE;

  int save_monitor_diag;
  int save_monitor_create_file;

  unsigned long SSMmode;

  int wanna_open;
  int wanna_close;

  int n_events_seen = 0;

  wanna_open = (action == 0) || (action == 1);
  wanna_close = (action == 0);

  /*
    Save the current status of whatever that is to be saved

    Set some options specially for this case
  */

  save_monitor_diag = StatusInternal.monitor.online_diag;
  save_monitor_create_file = StatusInternal.monitor.create_file;

  WriteFile_OFF();   /* We do not want to write */
  OnlineDiag_ON();   /* We want to rescan the stored sample */

  do{

    /*
      Open file for reading
    */
    if(wanna_open){
      if(StatusInternal.monitor.fd != -1){
	printf("soft_monitor_rescan: Input file in use, closing it first\n");
	StatusInternal.monitor.fd = ttcit_io_close(StatusInternal.monitor.fd);
	StatusInternal.monitor.fd = -1;

      }

      StatusInternal.monitor.fd = ttcit_io_open(&Sfilename[0], TTCIT_IO_READ);
      if(StatusInternal.monitor.fd == -1){
	printf("soft_monitor_rescan: Canna open the file %s\n",Sfilename);
	break;
      }
    }

    /*
      Loop over all events that can be found on the file

      Stop on error when requested
    */

    StatusInternal.monitor.run = RUNNING;  /* Start the fake monitor */

    /* Initialize diagnostics */

#ifdef TTCIT_EXTRA_DBG_PRINT_1__
    ttc_it_set_prt_suppress();
#endif
    irc = ttc_it_start_analyz();

    if(action != 2){
      ttc_it_clear_counters(&sum_count);
      ttc_it_clear_errors(&sum_err);
      ttc_it_clear_timing(&sum_time);
    }

    irc = ttc_it_alloc_fifos();
    if(irc != 0){
      printf("soft_monitor_rescan: Canna allocate FIFOs\n");
      goto dont_do;
    }

    StatusInternal.monitor.nSnapShots = 0;

    /* Start looping */

    do{

      irc = ttcit_io_next_event(StatusInternal.monitor.fd, 
				&SSM, &bcnt, &SSMmode, &EVENT);
      if(irc <= 0){
	printf("soft_monitor_rescan: Canna fetch next event, IRC = %d\n",irc);
	StatusInternal.monitor.run = STOPPING;
	break;
      }
      if(SSM.top == -1){     /* No more data on input file */
	StatusInternal.monitor.run = STOPPING;
	StatusInternal.monitor.fd = -1;        /* File must be closed */
	break;
      }

      switch(SSMmode){
      case TTCIT_IO_SSM_SCOPE:
	set_SS_mode(0);
	break;
      case TTCIT_IO_SSM_SEQ:
	set_SS_mode(1);
	break;
      default:
	printf("soft_monitor_rescan: Unknown SSM mode: %lu \n",SSMmode);
	StatusInternal.monitor.run = STOPPING;
	break;
      }
      if(StatusInternal.monitor.run == STOPPING){
	break;
      }

      irc = ttc_it_start_analyz();
      irc = ttc_it_alloc_fifos();
      if(!StatusInternal.justDump){
	irc = soft_monitor_diagnoze(&SSM, &bcnt);
      }
      if(irc != 0){
	printf("soft_monitor_rescan: Canna perform on-line diagnostics: %d\n",
	       irc);
	StatusInternal.monitor.run = STOPPING;
	break;
      }
      ttc_it_finish_analyz();
      soft_monitor_empty_abbc_fifos();
      irc = soft_monitor_fill_ABC(&SSM, &bcnt);
      if(StatusInternal.justDump){
	n_events_seen++;
	printf("soft_monitor_rescan: Dumping %d -th event\n",n_events_seen);
	ttc_it_show_ABBC();
      }

      StatusInternal.monitor.nSnapShots++;

    }while(StatusInternal.monitor.run != STOPPING);

    /*
      Close the file
    */
    if(wanna_close){
      if(StatusInternal.monitor.fd != -1){
	irc = ttcit_io_close(StatusInternal.monitor.fd);
	if(irc != -1){
	  StatusInternal.monitor.fd = -1;
	}else{
	  printf("soft_monitor_rescan: Canna close I/O file\n");
	}
      }
    }

    /*
      Dump the last Snap Shot
     */
    if(!StatusInternal.justDump){
#ifdef TTCIT_EXTRA_DBG_PRINT_1__
      ttc_it_set_noprint(TRUE);
      ttc_it_print_collections();
#endif
      ttc_it_print_counters(&sum_count);
      ttc_it_print_errors(&sum_err);
      ttc_it_print_timing(&sum_time);
      ttc_it_show_ABBC();
    }

  }while(0);

 dont_do:

  /*
    Restore StatusInternal and set it to STOPPED
   */

  StatusInternal.monitor.online_diag = save_monitor_diag;
  StatusInternal.monitor.create_file = save_monitor_create_file;
  StatusInternal.monitor.run = STOPPED;
}

void test_fetch_SSM(int noprint){  /* noprint = 1, no print, only stats */
  int nw;
  int retry = 1000;
  int i;
  int last_nw;
  int dummy_ssm;
  int expected;
  int n_retries = 0;
  int n_funnies = 0;
#ifdef  _LOGIC_SSM_CANNOT_BE_FFFFFF__
  int n_funny_ssm_data = 0;
  int n_ssm_data_retries = 0;
  int ssm_ff = 0xffffffff;
#endif
  int nloops = 100;

  struct TestFtStr log = { 0, 0, 0, 0 };

#define LOGIC_TEST_PRINT(A) printf( \
	   "%s after %d-th SSM READ\n%s%d\n%s%d\n%s%d\n", \
                                   A, \
				   log.i_th_read, \
				   " . ADDR from previous READ: ", \
				   log.after_last_read,\
				   ". ADDR before READ        : ", \
				   log.before_read_ssm, \
				   ". ADDR after READ:          ", \
				   log.after_read_ssm)

#define LOGIC_TEST_ERROR(A) printf("\n ERROR ERROR ERROR %s\n",A)

#define LOGIC_TEST_UNEXP(A) printf("... NW = %d unecpected SSM Address: ",A)

#define LOGIC_TEST_CORR(A) printf("CORRECTED after %d retries\n",A)

#define LOGIC_TEST_UNCORR(A) printf("REMAINS after %d retries\n",A)

#define LOGIC_TEST_SOFAR(A) printf("ADDR READ RETRIES SO FAR = %d\n",A)

#define LOGIC_TEST_SSM_SOFAR(A) printf("SSM READ RETRIES SO FAR = %d\n",A)

  if(noprint == 0){
    printf("TestFetchSSM: Searching for problems with reading SSM\n\n");
  }else{
    nloops = TFtStat.n_set_retries;
  }

  /*
    Read Nr. of words stored in SSM
   */
  nw = read_SSM_address();
  if(noprint != 0){ TFtStat.n_addr_read++; };
  if(nw <= 0){
    /*
      Retry the reading, if the nonpositive value remains, call it an
      error
     */
    if(noprint == 0){
      LOGIC_TEST_ERROR("First ReadAddressSSM");
      LOGIC_TEST_UNEXP(nw);
    }
    n_funnies++;
    if(noprint != 0){ TFtStat.n_addr_funnies++;};
    for(i = 0; i < retry; i++){
      my_dummy_loop(nloops);
      nw = read_SSM_address();
      n_retries++;
      if(noprint != 0){ TFtStat.n_retries++;};
      if(nw > 0){ 
	break;
      }
    }
    if(i < retry){
      if(noprint == 0){
	LOGIC_TEST_CORR(i);
	LOGIC_TEST_SOFAR(n_retries);
      }
    }else{
      if(noprint == 0){
	LOGIC_TEST_UNCORR(i);
      }else{
	TFtStat.n_failed_fetches++;
      }
      goto last_one;
    }
  }
  /*
    First ReadAddressSSM seem to be sucessfull, now try to read the
    SSM
   */
  last_nw = nw;
  expected = nw;

  while(nw > 0){       /* NW = 0 is the last read */
    nw = read_SSM_address();
    log.before_read_ssm = nw;
    if(noprint != 0){ TFtStat.n_addr_read++;};
    if(nw != expected){       /* NW changed before read */
      if(noprint == 0){
	LOGIC_TEST_ERROR("Unexpected ADDR before ReadLastSSM");
	LOGIC_TEST_PRINT("ADDR BEFORE");
	LOGIC_TEST_UNEXP(nw);
      }
      n_funnies++;
      if(noprint != 0){ TFtStat.n_addr_funnies++;};
      for(i = 0; i < retry; i++){
	my_dummy_loop(nloops);
	nw = read_SSM_address();
	n_retries++;
	if(noprint != 0){ TFtStat.n_retries++;};
	if(nw == expected){
	  break;
	}
      }
      if(i < retry){
	if(noprint == 0){
	  LOGIC_TEST_CORR(i);
	  LOGIC_TEST_SOFAR(n_retries);
	}
      }else{
	if(noprint == 0){
	  LOGIC_TEST_UNCORR(i);
	}else{
	  TFtStat.n_failed_fetches++;
	}
	goto last_one;
      }
    }
    /*
      Now we know that the AddressSSM is what we expect it to be
     */
    dummy_ssm = read_last_SSM_word();

#ifdef  _LOGIC_SSM_CANNOT_BE_FFFFFF__
    /*
      An attempt to identify wrong SSM as as 0xFFFFFF's.
      Use this feature only when you are 100% sure that 0xFFFFFF cannot be a 
      valid word.
     */
    if(dummy_ssm == ssm_ff){    /* Expected sign of problems */
      if(noprint == 0){
	LOGIC_TEST_ERROR("SSM Data Read as FF FF FF FF");
      }
      n_funny_ssm_data++;
      if(noprint != 0){ TFtStat.n_ssm_ff++;};
      for(i = 0; i < retry; i++){
	my_dummy_loop(nloops);
	dummy_ssm = read_last_SSM_word();
	n_ssm_data_retries++;
	if(noprint != 0){ TFtStat.n_ssm_retries++;};
	if(dummy_ssm != ssm_ff){
	  break;
	}
      }
      if(i < retry){
	if(noprint == 0){
	  LOGIC_TEST_CORR(i);
	  LOGIC_TEST_SSM_SOFAR(n_ssm_data_retries);
	}
      }else{
	if(noprint == 0){
	  LOGIC_TEST_UNCORR(i);
	}else{
	  TFtStat.n_data_failed_ssm++;
	}
	goto last_one;
      }
    }

#endif

    log.i_th_read++;

    nw = read_SSM_address();
    log.after_read_ssm = nw;
    if(noprint != 0){ TFtStat.n_addr_read++;};
    if(nw != (--expected)){    /* NW chaned after read in funny way */
      if(noprint  == 0){
	LOGIC_TEST_ERROR("Unexpected ADDR after ReadLastSSM");
	LOGIC_TEST_PRINT("ADDR AFTER");
	LOGIC_TEST_UNEXP(nw);
      }
      n_funnies++;
      if(noprint != 0){ TFtStat.n_addr_funnies++;};
      for(i = 0; i < retry; i++){
	my_dummy_loop(nloops);
	nw = read_SSM_address();
	n_retries++;
	if(noprint != 0){ TFtStat.n_retries++;};
	if(nw == expected){
	  break;
	}
      }
      if(i < retry){
	if(noprint == 0){
	  LOGIC_TEST_CORR(i);
	  LOGIC_TEST_SOFAR(n_retries);
	}
      }else{
	if(noprint == 0){
	  LOGIC_TEST_UNCORR(i);
	}else{
	  TFtStat.n_failed_fetches++;
	}
	goto last_one;
      }
    }
    last_nw = nw;
    log.after_last_read = last_nw;
  }

 last_one:
  if(noprint == 0){
    printf("FetchTestSSM final summary:\n");
    printf("                           ReadLastSSM operations  : %d\n",
	   log.i_th_read);
    printf("                           Unexpecteds encountered : %d\n",
	   n_funnies);
    printf("                           Nr. of retries needed   : %d\n",
	   n_retries);
    printf("        \n");
#ifdef  _LOGIC_SSM_CANNOT_BE_FFFFFF__
    printf("                           Error READ SSM DATA     : %d\n",
	   n_funny_ssm_data);
    printf("                           Nr. of SSM data retries : %d\n",
	   n_ssm_data_retries);
#endif
    printf("Last Address in SSM\n");
    RA();
  }else{
    TFtStat.n_ssm_fetches++;
  }

}

void loop_test_fetch_SSM(int n, int nretries){
  int i, j;
  int nmax = 1000;
  int nLoop;
  int nretry = 100;
  int idummy, ipri;
  int maxdum = 200;
  int maxrep = 3;
  int n_rep;


  if(n <= 0){ 
    nLoop = nmax;
  }else{
    nLoop = n;
  }

  if(nretries > 0){
    nretry = nretries;
  }

  /*
    Clear all counters
   */
  TFtStat.n_addr_read = 0;
  TFtStat.n_addr_funnies = 0;
  TFtStat.n_retries = 0;
  TFtStat.n_failed_fetches = 0;
  TFtStat.n_ssm_fetches = 0;
  TFtStat.n_set_retries = nretry;
  TFtStat.min_dumloop = 999999;
  TFtStat.max_dumloop = 0;
#ifdef  _LOGIC_SSM_CANNOT_BE_FFFFFF__
  TFtStat.n_ssm_ff = 0;
  TFtStat.n_ssm_retries = 0;
  TFtStat.n_data_failed_ssm = 0;
#endif

  /*
    Main loop: ResetTTCit, ..wait.., TestFetchSMM , collect stats
   */
  for(i = 0; i < nLoop; i++){

    if(i < 10){
      ipri = (1==1);
    }else{
      idummy = i % 10;
      ipri = (idummy == 0);
    }
    if(ipri){
      printf("LoopFetchSSM: starting %d attempt to FetchSSM\n",i);
    }

    n_rep = 0;

  try_again:
    /*
    ResetTTCit();    
    */
    OM_noprint();
    OM_reset_board();
    OM_yes_print();
                                      /* Reset */

    idummy = 0;
    while((j = read_SSM_address()) < TTCIT_MAX_ADDR_SSM){  /* Wait */
      /* usleep(usec_26ms); */
      my_dummy_loop(1000);
      idummy++;
      ipri = (idummy < 10) ? (1==0) : ((idummy % 10) == 0);
      if(ipri){
	printf("SSM not filled after %d waiting loops, j = %d\n",idummy,j);
      }
      if(idummy > maxdum){
	printf("SSM not filled adter %d waiting loops, breaking\n",idummy);
	if((++n_rep) > maxrep){
	  printf("%d attempt to try again failed, exiting\n",n_rep);
	  goto last_one;
	}else{
	  printf("Trying again...\n");
	  goto try_again;
	}
      }
    }
    TFtStat.min_dumloop = (idummy < TFtStat.min_dumloop) ? idummy : 
      TFtStat.min_dumloop;
    TFtStat.max_dumloop = (idummy > TFtStat.max_dumloop) ? idummy :
      TFtStat.max_dumloop;

    test_fetch_SSM(1);                                    /* Test-Fetch */
 last_one:
    for(j=0;j<1;j++){ continue; };
  }

  /*
    Colect statistics and print results
   */
  printf("\n");
  printf("LoopTestFetchSSM: Summary:\n");
  printf("                  SSM Fetches attempted      : %d\n",
	 TFtStat.n_ssm_fetches);
#ifdef  _LOGIC_SSM_CANNOT_BE_FFFFFF__
  printf(" \n");
  printf("                  SSM Read ADDRESS\n");
#endif
  printf("                  SSM Address Reads total    : %d\n",
	 TFtStat.n_addr_read);
  printf("                  Number of Errorneous reads : %d\n",
	 TFtStat.n_addr_funnies);
  printf("                  Number of Retries          : %d\n",
	 TFtStat.n_retries);
  printf("                  Failed SSM fetches         : %d\n",
	 TFtStat.n_failed_fetches);
  printf("\n");
#ifdef  _LOGIC_SSM_CANNOT_BE_FFFFFF__
  printf("                  --- \n");
  printf("                  SSM READ DATA \n");
  printf("                  Number of Err. Data reads  : %d\n",
	 TFtStat.n_ssm_ff);
  printf("                  Nr. of Data Read Retries   : %d\n",
	 TFtStat.n_ssm_retries);
  printf("                  Failed SSM fetches - DATA  : %d\n",
	 TFtStat.n_data_failed_ssm);
#endif
  printf(" \n");
  printf("                  Min nr. of dummy loops     : %d\n",
	 TFtStat.min_dumloop);
  printf("                  Max nr. of dummy loops     : %d\n",
	 TFtStat.max_dumloop);

}

int wait_till_SSM_full(int verbal){
  int irc = 0;

#ifdef TTCIT_USE_SLEEPS_
  int maxdum = 3;
  int maxrep = 1;
#else
  int maxdum = 200;
  int maxrep = 3;
#endif
  int n_rep;
  int j;
  int idumloop = 0;
  int v;

  v = (verbal == 1);

  n_rep = 0;
 try_again:

  /*
  ResetTTCit();
  */
  OM_noprint();
  OM_reset_board();
  OM_yes_print();

  while((j = read_SSM_address()) < TTCIT_MAX_ADDR_SSM){
    my_dummy_loop(1000);
    idumloop++;
    if(idumloop > maxdum){
      if(v){
	printf("SSM not filled after %d waiting loops - try again\n",idumloop);
      }
      if((++n_rep) > maxrep){
	if(v){
	  printf("Reading SSM repeatedly failed after %d attempts\n",n_rep);
	}
	irc = 1;
	goto last_one;
      }else{
	goto try_again;
      }
    }
  }
  if(v){
    printf("SSM tried %d times, in %d attempts\n",idumloop, n_rep);
    printf("SSM full: SSM address = %d\n",j);
  }

 last_one:

  return irc;
}

void my_dummy_loop(int nrLoops){
#ifdef TTCIT_USE_SLEEPS_
  int secs;
  secs = (nrLoops / 1000) + 1;  
  secs = 30 * secs;
  sleep(secs);
#else
  unsigned int il;
  unsigned int jl;
  unsigned int nl;
  double a = 0.1;
  double dx;
  double c, d;

  nl = (unsigned int)nrLoops;
  dx = 3.14  / (double)nl;
  for(il = 0; il < nl; il++){
    a += dx * (double)il;
    c = cos(a);
    for(jl = 0; jl < nl; jl++){
      a -= dx * (double)jl / 2.;
	d = cos(a); 
    }
  }
#endif
}

unsigned int my_funky_sleep(unsigned int seconds){
  unsigned int retval = 0;
#ifndef TTCIT_USE_SLEEPS_
  time_t tosleep = 0;
  time_t slept = 0;
  time_t sleep_start;
  time_t sleep_end;
  int nloops = 1000;

  tosleep = (time_t)seconds;
  sleep_start = time(NULL);
  do{
    my_dummy_loop(nloops);
    sleep_end = time(NULL);
    slept = sleep_end - sleep_start;
  }while(slept < tosleep);
  retval = slept;
#else
  sleep(seconds);
  retval = seconds;
#endif
  return retval;
}

unsigned int my_funky_calib(){
  unsigned int min_loops = 0;

  return min_loops;
}

