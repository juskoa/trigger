#include <stdio.h>
#include "vmewrap.h"
#include "ltu.h"

void  printltuDefaultsMem(Tltucfg *ltc) {
//Tltucfg *ltc= &ltushm->ltucfg;
printf("LTU defaults:\n");
printf("l1format:%d     ltu_LHCGAPVETO:%d    ltu_event_rate:%f\n",
  ltc->l1format,   ltc->ltu_LHCGAPVETO,   ltc->ltu_event_rate);
printf("sodeod:%d   autostart_signal:%d   busy:%d\n",
  ltc->ltu_sodeod_present,   ltc->ltu_autostart_signal,   ltc->busy);
printf("L0/TTC:%d   orbitbc:%d   calibbc:%d   dim:%d\n",
  ltc->L0,   ltc->orbitbc,   ltc->calibbc, ltc->dim);
printf("Bgo0stdalone:%d   Bgo0global:%d\n",
  ltc->Sbgo0delay,   ltc->Gbgo0delay);
printf("ppdelay:%d   Gppdelay:%d   BC_DELAY_ADD:%d\n",
  ltc->ppdelay,   ltc->Gppdelay, ltc->bc_delay_add);
printf("mainEmulationSequence:%s\n",   ltc->mainEmulationSequence);
printf("ttcrx_reset:%d\n",   ltc->ttcrx_reset);
printf("flags:0x%x\n",ltc->flags);
printf("global_calibration rate:%f roc:0x%x\n", 
  ltc->global_calibration_rate, ltc->plist[IXG_calibration_roc]);
printf("PP_TIMEg/s:%d/%d ORBIT_TIMEg/s:%d/%d\n", 
  ltc->plist[IXGpp_time], ltc->plist[IXSpp_time],
  ltc->plist[IXGorbit_time], ltc->plist[IXSorbit_time]);
printf("RATE_LIMIT:%x \n", ltc->plist[IXrate_limit]);
printf("ecsnum::0x%x \n", ltc->plist[IXecsnum]);
printf("l1rtime::0x%x \n", ltc->plist[IXl1rtime]);
}

