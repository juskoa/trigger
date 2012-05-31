#ifndef _TTCIT_N_H__
#define _TTCIT_N_H__

/*
  Stuff for firmware version 24 onward
 */

#include "ttcit_conf.h"
#include "ttcit_logic.h"

/*
  Definitions, macros
*/

/* 
             Event signature
 */
#define SSM_SIG_L0LVDS  0x0   /* L0 over LVDS */
#define SSM_SIG_L1ACC   0x2   /* L1accept */
#define SSM_SIG_DOUTST  0x1   /* Dout Strobe */
#define SSM_SIG_RXMES   0x3   /* TTCrx command/data/whatever */

/*
             Event Type masks
*/


/*
  Data types
 */

struct ttcit1_ssm_a {
  unsigned int empty :16;    /* Bits not needed */
  unsigned int sig   : 2;    /* Event signature */
  unsigned int dummy :14;    /* Bits not available in SSM */ 
};

struct ttcit1_ssm_b {
  unsigned int data: 12;     /* TTC data */
  unsigned int add :  4;     /* TTC address */
  unsigned int sig :  2;     /* Event signature */
  unsigned int dummy:14;     /* Bits not available in SSM */
};

struct ttcit1_ssm_rx {
  unsigned int data  :  4;    /* TTCrx data */
  unsigned int empty : 12;    /* Bits not used */
  unsigned int sig   :  2;    /* Event signature */
  unsigned int dummy : 14;    /* Bits not available in SSM */
};

union ttcit1_ssm_word {
  struct ttcit1_ssm_a a;     /* Handle A channel and Event selection */
  struct ttcit1_ssm_b b;    /* Handle B channel */
  struct ttcit1_ssm_rx rx; /* Handle TTCrx data */
};

enum ttcit1_lastDumper { ld_HEX_, ld_NEW_, ld_NONE_ };

struct ttcit1_opt {
  unsigned int orbit;     /* Current relatine Orbit nr. */
  unsigned int tick;      /* Current tick number */
  unsigned int base_orb;  /* Base orbit number */
  int ev_pos;             /* Event nr. of the last printed */
  int i;                  /* Index in the SSM buffer of the last printed */
  int frst_time;          /* Helper logical variable */
  int pos_1st;            /* Position number of the 1-st event */

  int N;            /* How many events to print in one go */

  enum ttcit1_lastDumper lastDumper; /* Who dumped data as the last */

  int pTick;     /* Print Tick if 1 # */
  int pOrbit;    /* Prit Orbit nr if 1 */
  int pBC;       /* Print BC if 1 */
  int pTTC_A;    /* Print traffic in Channel A if 1 */
  int pTTC_B;    /* Print traffic in Channel B if 1 */
  int pL0_LVDS;  /* Print L0 over cable */
  int pPP;       /* Print Pre Pulse */
  int pTTCrx;    /* Print TTCrx data/command/whatever */
  int pSig;      /* Print event signature */

  unsigned int sel1st;   /* Mask for the 1-st selected event */
  unsigned int curev;    /* Current event mask */

  int L0_over_fibre;    /* 1 if L0 is sent over fibre, 0 if not */

  int t_out_s;          /* Max timeout in sec for combined fetch and dumps */

  /*
    Counters
   */
  int l0f;             /* Nr. of L0's over fibre */
  int l0w;             /* Nr. of L0's over wire */
  int l1;              /* Nr. of L1 in A */
  int l1m;             /* Nr. of L1m headers */
  int l1mw;            /* Nr. of L1m words */
  int l2a;             /* Nr. of L2a headers */
  int l2aw;            /* Nr. of L2a words */
  int l2r;             /* Nr. of L2r messages */
  int pp;              /* Nr. of PP signals */
  int RoIh;            /* Nr. of RoI header */
  int RoIw;            /* Nr. of RoI words */
  int rx;              /* Nr. of TTCrx commands/data/whatever */
  int orb;             /* Nr. of orbit signals */
  int unknown;         /* Nr. of unknown cases */
  int B_undef;         /* Nr. of undefied items in B channel */
};

enum ttcit1_eventType { L0f_, L0w_, L1_, L1Mh_, L1Mw_, L2ah_, L2aw_,
			L2r_, RoIh_, RoIw_, PP_,
			ORBIT_, BCreset_, RxCmd_, UNKNOWN_ };

struct ttcit1_1st_mask {
  unsigned int L0               : 1;
  unsigned int L0_wire          : 1;
  unsigned int L1               : 1;
  unsigned int Orbit            : 1;
  unsigned int L1m              : 1;
  unsigned int L2a              : 1;
  unsigned int L2r              : 1;
  unsigned int PP               : 1;
  unsigned int L1Mh             : 1;
  unsigned int L1Mw             : 1;
  unsigned int L2ah             : 1;
  unsigned int L2aw             : 1;
  unsigned int RoIh             : 1;
  unsigned int RoIw             : 1;
  unsigned int BCreset          : 1;
  unsigned int RXcmd            : 1;
  unsigned int FEE              : 1;
  unsigned int rest             :13;
  unsigned int B_undef          : 1;
  unsigned int Unknown          : 1;
};

struct ttcit1_event {
  int tick;                           /* Tick number of this event */
  struct ttcit1_1st_mask EventType;    /* Event type */
  unsigned int eventData;             /* Event data */
  unsigned int BC;                    /* BC associated with this event */
  unsigned int extra;
  unsigned int signature;
};

/* 
   Functions 
 */

/*
  Initialize, reset counters, set defaults
 */
void ttcit1_init();

/*
  Decode next event:

  i     = index in buf, at which the next event to be decoded is placed
  buf   = SSMbuffer filled with data from SSM
  evt   = information about next decoded event

  returns = position of the following event in the buf, or <0 if error
 */
int ttcit1_event_decoder(int i, struct SSMbuffer *buf, 
			 struct ttcit1_event *evt);

/*
  Reset counters needed for evet decoding
 */
void ttcit1_evdec_reset();

/*
  Very primitive Hexadecimal and Binary dumper

  buf     = SSM buffer
  begin   = Index of the 1-st word to be dumped, if -1 take the word next
            after the last one printed
  howmany = How many words to dump, 0 means take default

  returns = Nr. of words dumped, or <0 if end-of-buffer
 */
int ttcit1_hexbin_prt(struct SSMbuffer *buf, int begin, int howmany);

/*
  Human readable dump of the SSM contents

  The printing always starts from the 1-st word after begin that satisfies
  conditions defined by SelDump1stEvent(Sel) i.e. matching the Sel mask.

  buf     = SSM buffer
  begin   = Index of the 1-st word to be dumped, if -1 take the word next
            after the last printed
  howmany = How many words to dump, 0 means take default

  returns = Nr. of words dumped, or <0 if end-of-buffer
 */
int ttcit1_ssm_prt(struct SSMbuffer *buf, int begin, int howmany);

/*
  Conversion from unsigned int to binary

  a     = unsigned int we want to convert
  c32   = pointer to a character array : char bin[33] - the c32[32] = '\0'
 */
void ttcit1_int2bin(unsigned int a, char *c32);

/*
  Copy characters from srting src to dest. DO NOT COPY '\0'
 */
void ttcit1_ccp(char *dest, char *src);

/*
  Reset TTCit and wait until the SSM is full

  timeouts    = max tolerable timeout in sec
  min_needed  = min nr. of words that must be stored in the SSM
                (with v 24 it is impossible to have a fixed FULL value,
		because of inconsistent treatment of L1 : 2 words for
		every other event, but 3 words for L1 when L0 over fibre)
  verbal      = 0 do not print messages (when called from a long loop)
                1 print messages (when is to be called just once)

 returns:    0 on success, IRC < 0 when some problems were detected
 */
int start_wait_1(int timeouts, int min_needed, int verbal);

/*
  Resets, starts and dumps 
 */
int ttcit1_fetch_dump(int what);
#define TTCIT1_WHAT_BINHEX_  0x1
#define TTCIT1_WHAT_HUMAN_   0x2

/*
  Checks the 1st evet selection mask in t1o.curev, 
  
  evt     = event whose mask is to be checked

  returns:  0  = if match not found
            1  = if match found, t1o.curev <= 0xffffff ans all the rest
	         is selected
 */
int ttcit1_check_1stmask(struct ttcit1_event *evt);

/*
  Scan SSM for certain type of messages, fill corresponding counters and 
  print them

  This should be called after a function that does FetchSSM, i.e.
  SelFetchDump(), SelFetchHexBinDump(0) or something similar.

  It is meant as a simple tool for quick scan of the SSM contents 
 */
int ttcit1_counters();

/*
  Reposition the event position counter according to selected settings

  returns:   N  - index in the SSM buffer if found
            <0  - if desired event does not exist or some error occured    
 */
int ttcit1_reposit();

#endif /* _TTCIT_N_H__ */
