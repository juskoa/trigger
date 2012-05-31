# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/ttcmi
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

ifeq (ttcmi,VME2FPGA)
  libobjs +=
endif
ifeq ($(findstring ttcmi,ltu cosmicfo lvdst),ltu)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib/linux_c -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
#ifeq (ttcmi,cosmicfo)
#endif
#ifeq (ttcmi,lvdst)
#  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
#  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
#endif
ifeq (ttcmi,ctp)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-Lctplib/linux_c -lctp  -L$(DIMDIR)/linux -ldim
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ttcmi,ADCI)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ttcmi,ttcmi)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ttcmi,ctpt)
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
#OBJFILES=$(VMEBDIR)/cmdbase.o $(libobjs) ttcmi_cf.o ttcmi.o 
OBJFILES=$(VMEBDIR)/cmdbase.o ttcmi_cf.o ttcmi.o 
ttcmi.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o ttcmi.exe
ttcmi_cf.o: ttcmi_cf.c $(VMEBDIR)/vmeaistd.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ttcmi_cf.c
#ttcmi_sim.o: ttcmi_sim.c $(VMEBDIR)/vmesim.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ttcmi_sim.c
$(VMEBDIR)/cmdbase.o: $(VMEBDIR)/cmdbase.c $(VMEBDIR)/vmeaistd.h
	cd $(VMEBDIR); $(VMECC) -g -c $(CFLAGS) cmdbase.c -o $(VMEBDIR)/cmdbase.o
clean:
	rm ttcmi_cf; rm ttcmi_cf.c; rm ttcmi_cf.py; rm ttcmi.make; rm *.o *.pyc *.exe
ttcmi.o: ttcmi.c $(VMEBDIR)/vmeblib/vmewrap.h infolog.h $(VMEBDIR)/vmeblib/vmeblib.h ctplib.h ttcmi.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ttcmi.c
