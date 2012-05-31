/*
  Replicated code - better to to clone it from single source
 */
// #include "ttcit_n.h"
/*
  this is clone for fetch_print_SS_bc         (1)
                    OM_fetch_print_SS_bc      (2)
 */
#ifdef TTCIT_CLONE_FETCH_PRINT_SS__

  int irc = 0;
  int big_error;
  int maxrep = 3;
  int n_rep;


  /*  unsigned int usec_26ms = 26000; */

  do{
    /*
      Allocate BC FIFO's, if they have not yet been allocated
    */
    irc = ttc_it_alloc_fifos();
    if(irc != 0){
      printf("Memory allocation problem, nothing is done\n");
      break;
    }else{
      printf("Dynamic memory allocation ........ SUCCESS\n");
    }

    /*
      Reset TTCit and wait till the SSM is full
    */
    n_rep = 0;
#ifndef TTCIT_USE_SLEEPS_
  try_again:
#endif

#ifdef _LOGIC_COMBINED_1_RADDR__
    RA();
#endif

#ifdef TTCIT_CLONE_FETCH_PRINT_SS_1__
    /*
    irc = wait_till_SSM_full(1);
    */
    irc = start_wait_1(-1,TTCIT_MAX_ADDR_SSM,1);
#endif
#ifdef TTCIT_CLONE_FETCH_PRINT_SS_2__
    irc = OM_wait_till_SSM_full(1,minutes);
#endif
#ifdef TTCIT_CLONE_FETCH_PRINT_SS_3__
    irc = 0;
#endif 

    if(irc != 0){
      if((++n_rep) < maxrep){
#ifndef TTCIT_USE_SLEEPS_
	printf("Filling SSM failed, Trying again\n");
	goto try_again;
#endif
      }else{
	printf("Cannot fill SSM: CHECK THE OPTICAL CABLE\n");
	goto last_one;
      }
    }

#ifdef _LOGIC_COMBINED_1_RADDR__
    RA();
#endif

    /*
      Fetch the contents of the Snap Shot memory
    */
#ifdef TTCIT_CLONE_FETCH_PRINT_SS_1__
    irc = fetch_SSM();   /* Check IRC, ignore if failed */
#endif
#ifdef TTCIT_CLONE_FETCH_PRINT_SS_2__
    irc = OM_fetch_SSM();
#endif
#ifdef TTCIT_CLONE_FETCH_PRINT_SS_3__
    irc = OM_fetch_SSM();
#endif
    if(irc > 0){
      printf("SnapShot memory read : %d  words\n",irc);
    }else{
      printf("SnapShot memory seem to be empty: NW = %d\n",irc);
      break;
    }
#ifdef _LOGIC_COMBINED_1_RADDR__
    RA();
#endif
    /*
      Fetch the contents of the BC FIFO
    */
#ifdef TTCIT_CLONE_FETCH_PRINT_SS_1__
    irc = fetch_bcnt_fifo();  /* No need to check IRC */
#endif
#ifdef TTCIT_CLONE_FETCH_PRINT_SS_2__
    irc = OM_fetch_bcnt_fifo();
#endif
#ifdef TTCIT_CLONE_FETCH_PRINT_SS_3__
    irc = OM_fetch_bcnt_fifo();
#endif
    if(irc > 0){
      printf("BC FIFO read :  %d  words extracted\n",irc);
    }else{
      printf("BC FIFO seems to be empty : NW = %d\n",irc);
    }
#ifdef _LOGIC_COMBINED_1_RADDR__
    RA();
#endif
    /*
      Store Snap Shot memory and BC FIFO into AB_fifo, BC_fifo
    */
    big_error = soft_monitor_fill_ABC(&SSM, &bcnt);
    if(big_error){
      printf(" Internal error or BUG, soft_monitor_fill_ABC\n");
      printf(" Nothing is going to be printed\n");
      break;
    }else{
      printf("FIFOs filled ........... SUCCESS\n");
      printf("\n\n");
    }
#ifdef _LOGIC_COMBINED_1_RADDR__
    RA();
#endif

    /*
      Print AB_fifo and BC_fifo side-by-side
    */
    ttc_it_show_ABBC();
    break;
  last_one:
    printf("All attempts to read SSM failed!!!\n");
    printf("CHECK THE OPTICAL CABLE CONNECTION - TTCit OPERATION FAILURE\n");
    printf("----------------------------------\n");

  }while(0);

#undef TTCIT_CLONE_FETCH_PRINT_SS__
#endif

/*
  This is clone for soft_monitor_single_ssm(int)         (1)
                    OM_soft_monitor_single_ssm(int)      (2)
 */
#ifdef TTCIT_CLONE_SM_SINGLE_SSM__

  int irc = 0;
  int j;

  /*
    Initialize this stuff
   */
  set_TTCIT_default_stops();   /* Keep all errors, show when printing */

#ifdef TTCIT_EXTRA_DBG_PRINT_1__
  ttc_it_set_noprint(what);
#endif
  /*
    Start Snap Shot memory. wait till full
   */

#ifdef TTCIT_CLONE_SM_SINGLE_SSM_1__
  irc = wait_till_SSM_full(0);
#endif
#ifdef TTCIT_CLONE_SM_SINGLE_SSM_2__
  irc = OM_wait_till_SSM_full(1,minutes);
#endif
#ifdef TTCIT_CLONE_SM_SINGLE_SSM_3__
  irc = 0;
#endif
  if(irc != 0){                  /* After this there is no need to go on */
    printf("SSM cannot reach FULL status, avoiding infinite loop - exiting\n");
    return;
  }

  /*
    Fetch and analyze its contents
   */

  j = ttc_it_start_analyz();
  j = ttc_it_alloc_fifos();
  if(j != 0){
    printf("Cannot allocate memory for FIFOs\n");
    return;
  }
#ifdef TTCIT_CLONE_SM_SINGLE_SSM_1__
  j = fetch_SSM();
#endif
#ifdef TTCIT_CLONE_SM_SINGLE_SSM_2__
  j = OM_fetch_SSM();
#endif
#ifdef TTCIT_CLONE_SM_SINGLE_SSM_3__
  j = OM_fetch_SSM();
#endif
  if(j < 0){
    printf("SSM returned wrong data\n");
    return;
  }
#ifdef TTCIT_CLONE_SM_SINGLE_SSM_1__
  j = fetch_bcnt_fifo();
#endif
#ifdef TTCIT_CLONE_SM_SINGLE_SSM_2__
  j = OM_fetch_bcnt_fifo();
#endif
#ifdef TTCIT_CLONE_SM_SINGLE_SSM_3__
  j = OM_fetch_bcnt_fifo(); 
#endif
  /*
    Refill the AB_fifo, BC_fifo, prepare for print
   */

  j = 0;
  j = soft_monitor_diagnoze(&SSM, &bcnt); 
  if(j != 0){
    printf("SSM analyzer returned IRC = %d\n",j);
    printf("This is error or BUG\n");
    return;
  }
  ttc_it_finish_analyz();
  soft_monitor_empty_abbc_fifos();
  j = soft_monitor_fill_ABC(&SSM, &bcnt);
  /*
    Print counters, errors and timers
   */
  if(what){
    ttc_it_print_counters(&counters);
    ttc_it_print_errors(&errores);
    ttc_it_print_timing(&timing);

    /*
      Print Error mask
    */

    ttc_it_show_ABBC();
  }
  /*
    Protection against internal inconsistencies

    After this the Soft Monitor must be stopped
   */
  StatusInternal.monitor.run  = STOPPED;

#undef TTCIT_CLONE_SM_SINGLE_SSM__
#endif

#ifdef TTCIT_OM_SYNC_DECL__
#ifdef TTCIT_OM_SYNC_DECL_S__
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
#endif

#ifdef TTCIT_OM_SYNC_DECL_H__
  int L1MM;
  int L1MS;
  int L1MI;
  int L1MD;
  int L2MM; 
  int L2MS;
  int L2MI;
  int L2MD;
  int PP;
  int L0S;
  int L1S;
  int CAL;
  int BCNT;
#endif

#undef TTCIT_OM_SYNC_DECL__
#endif
