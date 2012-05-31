# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/ttcit
VMECC=$(VMEGCC)
ifeq ($(VMESITE), ALICE)
WHICHSETUP = CAVERN_SETUP
else
WHICHSETUP = CERNLAB_SETUP
endif
ifeq ($(VMECC),g++)
COMDEFS=-D$(WHICHSETUP) -D$(VMEDRIVER) -DCPLUSPLUS
else
COMDEFS=-D$(WHICHSETUP) -D$(VMEDRIVER)
endif
COMMONCFLAGS=-Wall $(COMDEFS) -I$(VMEBDIR)/vmeblib
#libobjs=$(VMECFDIR)/CTPcommon/snapshot.o $(VMECFDIR)/CTPcommon/vmefpga.o

ifeq (ttcit,VME2FPGA)
  libobjs +=
endif
ifeq ($(findstring ttcit,ltu cosmicfo lvdst),ltu)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib/linux_c -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
#ifeq (ttcit,cosmicfo)
#endif
#ifeq (ttcit,lvdst)
#  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
#  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
#endif
ifeq (ttcit,ctp)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-Lctplib/linux_c -lctp  -L$(DIMDIR)/linux -ldim
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ttcit,ADCI)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ttcit,ttcmi)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ttcit,ctpt)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-Lctplib/linux_c -lctp
  CFLAGS +=-DSSMCONNECTIONS
endif
ifdef DATE_INFOLOGGER_DIR
INCDIRS=-I$(DATE_INFOLOGGER_DIR)
LDFLAGS +=-L$(VMEBDIR)/vmeblib/linux_c -lvmeb -L$(DATE_INFOLOGGER_DIR) -lInfo
else
LDFLAGS +=-L$(VMEBDIR)/vmeblib/linux_c -lvmeb
endif
ifdef DATE_DAQLOGBOOK_DIR
MYSQLLIBS=`/usr/bin/mysql_config --libs`
INCDIRS=-I$(DATE_DAQLOGBOOK_DIR)
LDFLAGS +=-L$(DATE_DAQLOGBOOK_DIR) -lDAQlogbook $(MYSQLLIBS)
endif
LDFLAGS +=-lpthread
include $(VMEBDIR)/vmeai.make.$(VMEDRIVER)
#OBJFILES=$(VMEBDIR)/cmdbase.o $(libobjs) ttcit_cf.o ttcit_io.o ttcit_mm.o ttcit_n.o ttcit.o fm_fpga.o ssm_analyz.o ttcit_om.o ttcit_logic.o auxttcit.o 
OBJFILES=$(VMEBDIR)/cmdbase.o ttcit_cf.o ttcit_io.o ttcit_mm.o ttcit_n.o ttcit.o fm_fpga.o ssm_analyz.o ttcit_om.o ttcit_logic.o auxttcit.o 
ttcit.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o ttcit.exe
ttcit_cf.o: ttcit_cf.c $(VMEBDIR)/vmeaistd.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ttcit_cf.c
#ttcit_sim.o: ttcit_sim.c $(VMEBDIR)/vmesim.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ttcit_sim.c
$(VMEBDIR)/cmdbase.o: $(VMEBDIR)/cmdbase.c $(VMEBDIR)/vmeaistd.h
	cd $(VMEBDIR); $(VMECC) -g -c $(CFLAGS) cmdbase.c -o $(VMEBDIR)/cmdbase.o
clean:
	rm ttcit_cf; rm ttcit_cf.c; rm ttcit_cf.py; rm ttcit.make; rm *.o *.pyc *.exe
ttcit_n.o: ttcit_n.c $(VMEBDIR)/vmeblib/vmewrap.h ttcit.h ttcit_logic.h ttcit_io.h ssm_analyz.h ttcit_om.h ttcit_n.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ttcit_n.c
ttcit_io.o: ttcit_io.c ttcit_io.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ttcit_io.c
ttcit.o: ttcit.c $(VMEBDIR)/vmeblib/vmewrap.h ttcit.h ttcit_logic.h ssm_analyz.h ttcit_mm.h ttcit_io.h ttcit_om.h ttcit_n.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ttcit.c
fm_fpga.o: fm_fpga.c $(VMEBDIR)/vmeblib/vmewrap.h ttcit.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) fm_fpga.c
ttcit_om.o: ttcit_om.c $(VMEBDIR)/vmeblib/vmewrap.h ttcit_conf.h ttcit.h ttcit_om.h ttcit_logic.h ttcit_io.h ssm_analyz.h ttcit_replicas.h ttcit_replicas.h ttcit_replicas.h ttcit_replicas.h ttcit_replicas.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ttcit_om.c
auxttcit.o: auxttcit.c auxttcit.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) auxttcit.c
ttcit_mm.o: ttcit_mm.c ttcit_mm.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ttcit_mm.c
ttcit_logic.o: ttcit_logic.c $(VMEBDIR)/vmeblib/vmewrap.h ttcit.h ttcit_logic.h ttcit_io.h ssm_analyz.h ttcit_om.h ttcit_n.h ttcit_replicas.h ttcit_replicas.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ttcit_logic.c
ssm_analyz.o: ssm_analyz.c ttcit_conf.h ssm_analyz.h ttcit_mm.h ttcit_io.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ssm_analyz.c
