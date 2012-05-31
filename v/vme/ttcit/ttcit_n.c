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

struct ttcit1_opt t1o;
extern struct SSMbuffer SSM;

#define _T_DEBUG_

#ifdef _T_DEBUG_

struct {

  int state;
  int l0_c;
  int l1_c;
  int i_l0;
  int i_l1;

} d1;

void ttcit1_d1(struct ttcit1_event *e, int event);
#endif

void ttcit1_init(){

  t1o.orbit = 0;
  t1o.tick = 0;

  t1o.N = 100;
  t1o.ev_pos = 0;
  t1o.i = 0;
  t1o.lastDumper = ld_NONE_;
  t1o.frst_time = 0;

  t1o.pTick = 1;
  t1o.pOrbit = 1;
  t1o.pBC = 1;
  t1o.pTTC_A = 1;
  t1o.pTTC_B = 1;
  t1o.pL0_LVDS = 1;
  t1o.pPP = 1;
  t1o.pTTCrx = 1;
  t1o.pSig = 1;

  t1o.sel1st = 0xffff;
  t1o.curev = 0x0;

  t1o.L0_over_fibre = 1;

  t1o.t_out_s = 300;
}

int ttcit1_event_decoder(int i, struct SSMbuffer *buf,
			 struct ttcit1_event *evt){
  int irc = 0;
  union ttcit1_ssm_word *s;
  union ttcit1_ssm_word *ns;
  unsigned int w;
  unsigned int nw;
  int j;
  int last_dato;
  unsigned int sig;
  unsigned int ttc_addr;
  unsigned int ttc_data;
  unsigned int BC;
  unsigned int maxBCmask = 0xfff;
  unsigned int nonBC;

  do{
    /*
      When evt == NULL reset counters, otherwise you may end up with
      bull shit
     */
    if(evt == NULL){
      ttcit1_evdec_reset();
      break;
    }

    evt->tick = 0;
    *((unsigned int *)(&evt->EventType)) = 0x0;
    evt->eventData = 0x0;
    evt->BC = 0;
    evt->extra = 0;
    evt->signature = 4;

    last_dato = (buf->top > (MEGA-1))? MEGA - 1 : buf->top;
    j = i;
    if(j >= (last_dato - 2)){   /* I need at least 2 words in the buffer */
#ifdef _T_DEBUG_
      printf("SSM exhausted j = %d, LAST_DATO = %d\n",j,last_dato);
#endif
      irc = 0;
      break;
    }

    w = buf->buf[j];
    s = (union ttcit1_ssm_word *)(&w);

    sig = s->a.sig;
    evt->signature = s->a.sig;
    switch(sig){
    case 0x0:                             /* L0 over wire */
      evt->EventType.L0_wire = 1;
      evt->eventData = 0x0;
      break;
    case 0x2:                             /* TTC A channel L0/L1 */
      nw = buf->buf[j+1];
      ns = (union ttcit1_ssm_word *)(&nw);
      if(ns->a.sig == 0x2){                /* L1 */
	evt->EventType.L1 = 1;
	j++;
      }else{                               /* L0 */
	evt->EventType.L0 = 1;
      }
      evt->eventData = 0x0;
      break;
    case 0x1:                             /* TTC B channel */
      ttc_addr = s->b.add;
      ttc_data = s->b.data;
      switch(ttc_addr){
      case 1:
	evt->EventType.L1Mh = 1;
	evt->EventType.L1m = 1;
	break;
      case 2:
	evt->EventType.L1Mw = 1;
	break;
      case 3:
	evt->EventType.L2ah = 1;
	evt->EventType.L2a = 1;
	break;
      case 4:
	evt->EventType.L2aw = 1;
	break;
      case 5:
	evt->EventType.L2r = 1;
	break;
      case 6:
	evt->EventType.RoIh = 1;
	break;
      case 7:
	evt->EventType.RoIw = 1;
	break;
      case 8:
	evt->EventType.FEE = 1;
      default:
	evt->EventType.B_undef = 1;
	break;
      }
      evt->eventData = ttc_data;
      evt->extra = ttc_addr;
      break;
    case 0x3:                             /* TTC Rx data/command/whatever */
      evt->EventType.RXcmd = 1;
      switch(s->rx.data){
      case 0x0:
	evt->EventType.Orbit = 1;
	evt->eventData = s->a.empty;
	break;
      case 0xf:
	evt->EventType.PP = 1;
	evt->eventData = s->a.empty;
	break;
      default:
	evt->eventData = s->a.empty;
      }
      break;
    }
    /*
      If L0 is sent only over LVDS cable, then L0 should not be seen
      int the A signal and L1 signature is just one pulse in A channel
     */
    if((t1o.L0_over_fibre == 0) & (evt->EventType.L0 == 1)){
      evt->EventType.L1 = 1;
      evt->EventType.L0 = 0;
    }

    j++;   /* Now this should be an index to BC count */
    BC = buf->buf[j];
    nonBC = ~maxBCmask & BC;   /* i.e. what is outside BC range */
    if(nonBC != 0){
      printf("SSM format error at i = %d, BC = %X\n",j,BC);
      irc = -1;
      break;
    }else{
      evt->BC = BC;
    }
    /*
      Now we have to make some Orbit + BC arithmetics
     */
    if(evt->EventType.Orbit == 1){
      if(t1o.frst_time == 1){
	t1o.orbit++;
	t1o.base_orb += TTCIT_MAX_BUNCH_XING + 1;   
      }        
    }
    t1o.tick = t1o.base_orb + BC;   /* BC starts from 0 */
    evt->tick = t1o.tick;
    if(t1o.frst_time == 0){
      t1o.frst_time = 1;
    }

    /*
      All done successfully, set pointers to next event/SSM address 
    */
    irc = j + 1;   /* New index in SSM */
    t1o.ev_pos++;  /* Update event count */
  }while(0);

  return irc;
}

void ttcit1_evdec_reset(){
  t1o.orbit = 0;
  t1o.tick = 0;
  t1o.base_orb = 0;
  t1o.ev_pos = 0;
  t1o.i = 0;
  t1o.frst_time = 0;

  /*
    Reset counters
   */
  t1o.l0f = 0;
  t1o.l0w = 0;
  t1o.l1 = 0;
  t1o.l1m = 0;
  t1o.l1mw = 0;
  t1o.l2a = 0;
  t1o.l2aw = 0;
  t1o.l2r = 0;
  t1o.pp = 0;
  t1o.RoIh = 0;
  t1o.RoIw = 0;
  t1o.rx = 0;
  t1o.orb = 0;
  t1o.unknown = 0;
  t1o.B_undef = 0;
}

int ttcit1_hexbin_prt(struct SSMbuffer *buf, int begin, int howmany){
  int irc = 0;
  int start = 0;
  int end = 0;
  int i;
  char line[101];
  char blanks[101];
  char item[80];
  unsigned int w;
  struct ttcit1_ssm_a *a;
  unsigned int u;
  char c32[33];
  int j, isf;
  int saflen;

  do{
    switch(t1o.lastDumper){
    case ld_HEX_:
      break;
    case ld_NONE_:
      t1o.i = 0;        /* Not yet read, default it to 0 */
      break;
    case ld_NEW_:
      t1o.i = 0;        /* in NEW human readble format it has other meaning */
      break;
    }
    t1o.lastDumper = ld_HEX_;

    if(begin >= 0){
      start = begin;
      t1o.i = start;
    }else{
      start = t1o.i;
    }
    end = (howmany > 0) ? start + howmany : start + t1o.N;

    /*
      Empty the line
     */
    for(i = 0; i < 100; i++){
      line[i] = ' ';
      blanks[i] = ' ';
    }
    line[100] = '\0';                  /* Safety measure */
    blanks[100] = '\0';
    /*
      First 2 lines
    */
    ttcit1_ccp(&line[1]," Index");
    ttcit1_ccp(&line[10]," HEX");
    ttcit1_ccp(&line[18],"ID");
    ttcit1_ccp(&line[22],"DATA");
    strcpy(&line[30],"DATA (binary)");    /* We need '\0' at the end */
    printf("%s\n",&line[0]);
    saflen = strlen(&line[0]) + 10;
    for(i = 0; i < saflen; i++){
      line[i] = '-';
    }
    line[saflen] = '\0';
    printf("%s\n",&line[0]);

    for(i = start; i < end; i++){
      ttcit1_ccp(&line[0],&blanks[0]);
      if(i >= buf->top){
	irc = -1;                   /* No mo' data inna buffa */
	break;
      }
      sprintf(&item[0],"%7d",i);           /* Index */
      ttcit1_ccp(&line[1],&item[0]);

      w = buf->buf[i];
      a = (struct ttcit1_ssm_a *)&w;

      sprintf(&item[0],"%5X",w);           /* SSM word HEX */
      ttcit1_ccp(&line[10],&item[0]);
      u = a->sig;
      ttcit1_int2bin(u, &c32[0]);          /* Event ID 2 bits binary */
      ttcit1_ccp(&line[18],&c32[30]);
      u = a->empty;
      sprintf(&item[0],"%4X",u);           /* Event Data HEX */
      ttcit1_ccp(&line[22],&item[0]);

      ttcit1_int2bin(u, &c32[0]);          /* Event data binary */
      isf = 0;
      for(j = 0; j < 16; j++){
	if((j % 4) == 0){
	  line[30+isf+j] = ' ';
	  isf++;
	}
	line[30+isf+j] = c32[16+j];
      }
      line[30+isf+17] = '\0';        /* Safe string ending */
      printf("%s\n",&line[0]);
      t1o.i++;
    }

  }while(0);
  return irc;
}

void ttcit1_ccp(char *dest, char *src){
  int i = 0;
  char c;
  while((c = *(src + i)) != '\0'){
    *(dest + i++ ) = c;
  }
}

void ttcit1_int2bin(unsigned int a, char *c32){
  char *str;
  int cnt = 31;
  str = c32;
  while ( cnt > -1 ){
    str[cnt]= '0';
    cnt --;
  }
  str[32] = '\0';
  cnt = 31;
  while (a > 0){
    if (a%2==1){
      str[cnt] = '1';
    }
    cnt--;
    a = a/2 ;
  }
}

int start_wait_1(int timeouts, int min_needed, int verbal){
  int irc = 0;
  int v = 0;
  int to;
  int minwait = 15; /* Wait minimum 30 seconds */
  int timleft;
  int filled;

  do{
    /*
      Reset the TTCit, but save all OM constants (set in the course
      of TTCit activity)
     */
    OM_noprint();
    OM_reset_board();
    OM_yes_print();

    v = (verbal != 0) ? (1==1) : (1==0);

    to = (timeouts > 0)? timeouts : t1o.t_out_s;

    if(to < minwait){
      minwait = to;
    }
    timleft = to;

    while((filled = read_SSM_address()) < min_needed){
      sleep(minwait);
      timleft -= minwait;
      if(timleft < minwait){
	minwait = timleft;
      }
      if(timleft <= 0){
	if(v){
	  printf("SSM not filled after %d seconds\n",timeouts);
	  printf("Needed %d     Filled   %d    SSM words\n",min_needed,
		 filled);
	}
	irc = -1;
	break;
      }
    }
  }while(0);
  return irc;
}

int ttcit1_fetch_dump(int what){
  int irc = 0;

  int mineed = TTCIT_MAX_ADDR_SSM;
  int safety = 3;

  do{
    mineed -= safety;
    irc = start_wait_1(t1o.t_out_s,mineed,1);
    if(irc == 0){
      irc = start_wait_1(t1o.t_out_s,mineed,1);     /* Wait another 1sec */
    }else{
      irc = -1;
      break;
    }
    irc = fetch_SSM();
    if(irc > 0){
      printf("SnapShot memory read : %d words\n",irc);
    }else{
      printf("SnapShot memory looks empty NW = %d\n",irc);
      irc = -2;
      break;
    }
    switch(what){
    case TTCIT1_WHAT_BINHEX_:
      ttcit1_hexbin_prt(&SSM, 0, 0);
      break;
    case TTCIT1_WHAT_HUMAN_:
      ttcit1_ssm_prt(&SSM, 0, 0);
      break;
    default:
      printf("INTERNAL ERROR!!! BUG NOT A FEATURE!!!\n");
      irc = -3;
      break;
    }
  }while(0);
  return irc;
}

int ttcit1_ssm_prt(struct SSMbuffer *buf, int begin, int howmany){
  int irc = 0;

  int start;
  int evt_prted;
  struct ttcit1_event e;
  struct ttcit1_1st_mask em;

  char line[101];
  char blanks[101];
  char item[80];
  char c32[33];
  int i;
  int saflen;
  int sth2prt;
  char *it;

  do{
    /*
      When do we need to reset all counters
     */
    switch(t1o.lastDumper){
    case ld_HEX_:
      ttcit1_evdec_reset();
      break;
    case ld_NONE_:
      ttcit1_evdec_reset();
      break;
    case ld_NEW_:
      if((begin == 0) || ((begin < t1o.ev_pos) && (begin > 0))){
	ttcit1_evdec_reset();
      }
      break;
    }
    t1o.lastDumper = ld_NEW_;

    if(begin >= 0){
      start = begin + 1;
    }else{
      start = t1o.ev_pos;
    }

    /*
      Prepare some strings
     */
    for(i = 0; i < 100; i++){
      line[i] = ' ';
      blanks[i] = ' ';
    }
    line[100] = '\0';
    blanks[100] = '\0';
    
    /*
      Print the header lines
     */
    printf("\n");

    ttcit1_ccp(&line[22],"     BC ");
    ttcit1_ccp(&line[44],"TTC");
    strcpy(&line[60]," ");
    printf("%s\n",&line[0]);

    ttcit1_ccp(&line[0],&blanks[0]);
    ttcit1_ccp(&line[1]," Tick");
    ttcit1_ccp(&line[13]," Orbit");
    ttcit1_ccp(&line[21],"  hex   dec ");
    ttcit1_ccp(&line[33],"ID");
    ttcit1_ccp(&line[37],"Wire");
    ttcit1_ccp(&line[43],"A");
    ttcit1_ccp(&line[44],"  B   adr  data");
    strcpy(&line[63]," TTC Rx");        /* We need End of String */
    printf("%s\n",&line[0]);
    saflen = strlen(&line[0]) + 10;
    for(i = 0; i < saflen; i++){
      line[i] = '-';
    }
    line[saflen] = '\0';
    printf("%s\n",&line[0]);

    /*
      Main event loop
     */
    evt_prted = 0;
    t1o.curev = t1o.sel1st;
    do{          
      ttcit1_ccp(&line[0],&blanks[0]);       
      /*
	Decode the next event, and check whether it is the one you wanted
	to print
       */
      t1o.i = ttcit1_event_decoder(t1o.i, &SSM, &e);
      if(t1o.i <= 0){
	break;
      }
      if(t1o.ev_pos < start){  /* Not yet at the correct event */
	continue;
      }
      if(ttcit1_check_1stmask(&e) != 1){
	continue;
      }
      /*
	Start of the printer
       */
      em = e.EventType;
      sth2prt = (0 == 1);
      /*
	Tick, orbit and BC are always shown
       */
      sprintf(&item[0],"%10d",e.tick);
      ttcit1_ccp(&line[1],&item[0]);
      sprintf(&item[0],"%6u",t1o.orbit);
      ttcit1_ccp(&line[13],&item[0]);
      sprintf(&item[0],"%3X  %4u",e.BC,e.BC);
      ttcit1_ccp(&line[21],&item[0]);
      /*
	If yor have something to print set sth2prt = 1
       */
      if(t1o.pTTC_A == 1){
	it = NULL;
        if(em.L0 == 1){
	  it = "L0";
	}else if(em.L1 == 1){
	  it = "L1";
	}
	if(it != NULL){
	  sth2prt = 1;
	  sprintf(&item[0],"%2s",it);
	  ttcit1_ccp(&line[42],&item[0]);
	}
      }
      if(t1o.pTTC_B == 1){
	it = NULL;
	if(em.L1Mh == 1){
	  it = "L1M Hdr";
	}else if(em.L1Mw == 1){
	  it = "L1M Dat";
	}else if(em.L2ah == 1){
	  it = "L2a Hdr";
	}else if(em.L2aw == 1){
	  it = "L2a Dat";
	}else if(em.L2r == 1){
	  it = "L1r    ";
	}else if(em.RoIh == 1){
	  it = "RoI Hdr";
	}else if(em.RoIw == 1){
	  it = "RoI Dat";
	}else if(em.FEE == 1){
	  it = "FEE";
	}else if(em.B_undef == 1){
	  it = "B UNDEF";
	}
	if(it != NULL){
	  sth2prt = 1;
	  sprintf(&item[0],"%6s  %1X   %3X",it,e.extra,e.eventData);
	  ttcit1_ccp(&line[44],&item[0]);
	}
      }
      if(t1o.pL0_LVDS == 1){
	it = NULL;
	if(em.L0_wire == 1){
	  it = "L0";
	}
	if(it != NULL){
	  sth2prt = 1;
	  sprintf(&item[0],"%2s",it);
	  ttcit1_ccp(&line[37],&item[0]);
	}
      }
      if(t1o.pTTCrx == 1){
	it = NULL;
	if(em.RXcmd == 1){
	  it = "RX cmd";
	}
	if((em.Orbit == 1) || (em.PP == 1)){  /* Handle these two extra */
	  it = NULL;
	}
	if(it != NULL){
	  sth2prt = 1;
	  sprintf(&item[0],"%6s %4X",it,e.eventData);
	  ttcit1_ccp(&line[0],&item[0]);
	}
      }
      if(t1o.pOrbit == 1){
	it = NULL;
	if(em.Orbit == 1){
	  it = "ORBIT";
	}
	if(it != NULL){
	  sth2prt = 1;
	  sprintf(&item[0],"%6s %4X",it,e.eventData);
	  ttcit1_ccp(&line[63],&item[0]);
	}
      }
      if(t1o.pPP == 1){
	it = NULL;
	if(em.PP == 1){
	  it = "PP";
	}
	if(it != NULL){
	  sth2prt = 1;
	  sprintf(&item[0],"%6s %4X",it,e.eventData);
	  ttcit1_ccp(&line[63],&item[0]);
	}
      }
      if(t1o.pSig == 1){
	ttcit1_int2bin(e.signature,&c32[0]);
	sprintf(&item[0],"%2s",&c32[30]);
	ttcit1_ccp(&line[33],&item[0]);
      }
      line[saflen] = '\0';
      if(sth2prt == 0){         /* Nothing to print, try the next one */
	continue;
      }
      printf("%s\n",&line[0]);

      /*
	End of the printer
       */
      if((++evt_prted) > t1o.N){  /* All requested printed */
	break;
      }
    }while(1);


  }while(0);
  return irc;
}

int ttcit1_check_1stmask(struct ttcit1_event *evt){
  int irc = 0;
  unsigned int allyes = 0xffffffff;
  unsigned int sel1able = 0xff;      /* Only subset is selectable */
  unsigned int *type;
  unsigned int nt;
  unsigned int cm;
  unsigned int ans;
  int match;
  do{
    if(t1o.curev == allyes){   /* All selected, nothing to check */
      irc = 1;
      break;
    }
    cm = (t1o.curev & sel1able);
    type = (unsigned int *)(&evt->EventType);
    nt = (*type & sel1able);
    ans  = nt & cm;     /* At least one bit must be the same in both */
    match = (ans != 0x0);
  }while(0);
  if(match){  
    irc = 1;
    t1o.curev = allyes;
  }
  return irc;
}

#define _T1_CNT_(A,B) if(e.EventType.B == 1){t1o.A++;}
int ttcit1_counters(){
  int irc = 0;
  struct ttcit1_event e;
  int n = 0;
  do{
    ttcit1_evdec_reset();
    /*
      Main loop
     */
#ifdef _T_DEBUG_
    ttcit1_d1(NULL,0);
#endif
    while((t1o.i = ttcit1_event_decoder(t1o.i, &SSM, &e)) > 0){
      _T1_CNT_(l0f,L0);
      _T1_CNT_(l0w,L0_wire);
      _T1_CNT_(l1,L1);
      _T1_CNT_(l1m,L1Mh);
      _T1_CNT_(l1mw,L1Mw);
      _T1_CNT_(l2a,L2ah);
      _T1_CNT_(l2aw,L2aw);
      _T1_CNT_(l2r,L2r);
      _T1_CNT_(pp,PP);
      _T1_CNT_(RoIh,RoIh);
      _T1_CNT_(RoIw,RoIw);
      _T1_CNT_(rx,RXcmd);
      _T1_CNT_(orb,Orbit);
      _T1_CNT_(unknown,Unknown);
      _T1_CNT_(B_undef,B_undef);
      n++;
#ifdef _T_DEBUG_
      ttcit1_d1(&e,n);
#endif
    }
    printf("Scanned %d events in SSM\n",n);
  }while(0);
  return irc;
}
#undef _T1_CNT_

int ttcit1_reposit(){
  int irc = 0;
  struct ttcit1_event e;

  /*
    Back up values at the start of this function
   */
  unsigned int orbit;  
  unsigned int tick;
  unsigned int base_orb;
  int ev_pos;                
  int i;
  int frst_time;
  unsigned int curev;

  int new_pos_found;
  int ev_seen_count;

  /*
    Back up values of the last seen viable event
   */
  unsigned int orbit_last = 0;
  unsigned int tick_last = 0;
  unsigned int base_orb_last = 0;
  int ev_pos_last = 0;
  int i_last = 0;
  int frst_time_last = 0;
  int curev_last = 0;

  /*
    Back up values of the last viable event
   */
  unsigned int orbit_c = 0;
  unsigned int tick_c = 0;
  unsigned int base_orb_c = 0;
  int ev_pos_c = 0;
  int i_c = 0;
  int frst_time_c = 0;
  int curev_c = 0;

  int N;

  do{
    /*
      Set the universal value of N - nr of event to be selected

      if 0 set it to 1
      if -1 (i.e. the last one, set it to impossibly high value)
    */
    if(t1o.pos_1st == 0){
      N = 1;
    }else if(t1o.pos_1st < 0){
      N = INT_MAX;               /* Max integer value, impossibly high */
    }else{
      N = t1o.pos_1st;
    }

    /*
      Back up current variables, if the search fails they will be
      restored
    */
    orbit = t1o.orbit;
    tick = t1o.tick;
    base_orb = t1o.base_orb;
    ev_pos = t1o.ev_pos;
    i = t1o.i;
    frst_time = t1o.frst_time;
    curev = t1o.curev;
    /*
      Reset counters
    */
    ttcit1_evdec_reset();
    t1o.curev = t1o.sel1st;
    /*
      Loop over all events, try to find some match
    */
    ev_seen_count = 0;
    new_pos_found = 0;
    while(1){
      /*
	Store current event state, after decoding the state corresponds to
	the NEXT event
       */
      orbit_c = t1o.orbit;
      tick_c = t1o.tick;
      base_orb_c = t1o.base_orb;
      ev_pos_c = t1o.ev_pos;
      i_c = t1o.i;
      frst_time_c = t1o.frst_time;
      curev_c = t1o.curev;
      /*
	Decode event
       */
      t1o.i = ttcit1_event_decoder(t1o.i, &SSM, &e);
      if(t1o.i <= 0){
	break;           /* All SSM exhausted */
      }
      if(ttcit1_check_1stmask(&e) != 1){  /* No match with 1-st event mask */
	continue;                      
      }
      /*
	ttcit1_check_1stmask set t1o.curev to 0xffff, we must put it back
	to initial state
       */
      t1o.curev = t1o.sel1st;
      ev_seen_count++;
      /*
	Store position of the last seen viable event 
      */
      orbit_last = orbit_c;
      tick_last = tick_c;
      base_orb_last = base_orb_c;
      ev_pos_last = ev_pos_c;
      i_last = i_c;
      frst_time_last = frst_time_c;
      curev_last = curev_c;
      /*
	If i-th requested, stop and mark it
      */
      if(ev_seen_count == N){
	new_pos_found = 1;
	t1o.orbit = orbit_c;
	t1o.tick = tick_c;
	t1o.base_orb = base_orb_c;
	t1o.ev_pos = ev_pos_c;
	t1o.i = i_c;
	t1o.frst_time = frst_time_c;
	t1o.curev = curev_c;
	irc = t1o.i;
	break;
      }
    }
    if(new_pos_found == 0){  /* Event not found */
      if(ev_seen_count  <= 0){
	t1o.orbit = orbit;          /* Restore initial values */
	t1o.tick = tick;
	t1o.base_orb = base_orb;
	t1o.ev_pos = ev_pos;
	t1o.i = i;
	t1o.frst_time = frst_time;
	t1o.curev = curev;
      }else{                        /* Set the last viable event position */
	t1o.orbit = orbit_last;
	t1o.tick = tick_last;
	t1o.base_orb = base_orb_last;
	t1o.ev_pos = ev_pos_last;
	t1o.i = i_last;
	t1o.frst_time = frst_time_last;
	t1o.curev = curev_last;
      }
    }
  }while(0);
  printf("Scanned %d events in SSM\n",ev_seen_count);
  return irc;
}

#ifdef _T_DEBUG_

void ttcit1_d1(struct ttcit1_event *e, int event){
  int what;
  int irc = 0;
  do{
    if(e == NULL){
      d1.state = -1;
      d1.l0_c = 0;
      d1.l1_c = 0;
      d1.i_l0 = 0;
      d1.i_l1 = 0;
      break;
    }
    what = -1;
    if(e->EventType.L0 == 1){
      what = 0;
    }
    if(e->EventType.L1 == 1){
      what = (what < 0) ? 1 : 2;   /* L0 and L1 both marked - BUG */
    }
    if(what < 0){
      break;
    }
    if(what == 0){
      d1.l0_c++;
    }
    if(what == 1){
      d1.l1_c++;
    }
    if(what == 2){
      d1.l1_c++;
      d1.l0_c++;
      printf("Misidentified L0/L1: Event/Tick/Orbit/BC = %d / %d  / %d / %d\n",
	     event, e->tick, t1o.orbit, e->BC);
      d1.state = -1;
      break;
    }
    switch(d1.state){
    case -1:
      switch(what){
      case 0:
	d1.state = 0;
	break;
      case 1:
	printf("Missing L0, L0 (-10): Event/Tick/Orbit/BC = %d / %d / %d / %d\n",
	       event, e->tick, t1o.orbit, e->BC); 
	d1.state = -1;
	irc = 1;
	break;
      }
      break;
    case 0:
      switch(what){
      case 0:
	d1.state = 0;
	break;
      case 1:
	d1.state = -1;
	break;
      }
      break;
    case 1:
      switch(what){
      case 0:
	d1.state = 0;
	break;
      case 1:
	printf("Missing L0, L0 (1/1):  Event/Tick/Orbit/BC = %d / %d / %d / %d\n",
	       event, e->tick, t1o.orbit, e->BC);
	d1.state = -1;
	break;
      }
      break;
    }

  }while(0);
}

#undef _T_DEBUG_
#endif

