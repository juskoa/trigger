# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/rf2ttc
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

ifeq (rf2ttc,VME2FPGA)
  libobjs +=
endif
ifeq ($(findstring rf2ttc,ltu cosmicfo lvdst),rf2ttc)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib/linux_c -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
#ifeq (rf2ttc,cosmicfo)
#endif
#ifeq (rf2ttc,lvdst)
#  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
#  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
#endif
ifeq (rf2ttc,ctp)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-Lctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (rf2ttc,inputs)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
  COMMONCFLAGS +=-I$(DIMDIR)/dim
endif
ifeq (rf2ttc,ADCI)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (rf2ttc,ttcmi)
#  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp -L$(DIMDIR)/linux -ldim
  LDFLAGS +=-L$(DIMDIR)/linux -ldim
  INCDIRS=-I$(DIMDIR)/dim
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (rf2ttc,ctpt)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-Lctplib/linux_c -lctp
  CFLAGS +=-DSSMCONNECTIONS
endif
# ctplib has to be before vmeblib
LDFLAGS +=-L$(VMEBDIR)/vmeblib/linux_c -lvmeb
# dimlib has to be after vmeblib (at least for SIMVME)
ifdef DIMDIR
ifeq (rf2ttc,ctp)
  LDFLAGS +=-L$(DIMDIR)/linux -ldim
endif
ifeq (rf2ttc,inputs)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-L$(DIMDIR)/linux -ldim
endif
endif
ifdef DATE_INFOLOGGER_DIR
INCDIRS +=-I$(DATE_INFOLOGGER_DIR)
LDFLAGS +=-L$(DATE_INFOLOGGER_DIR) -lInfo
endif
ifdef DATE_DAQLOGBOOK_DIR
MYSQLLIBS=`/usr/bin/mysql_config --libs`
INCDIRS +=-I$(DATE_DAQLOGBOOK_DIR)
LDFLAGS +=-L$(DATE_DAQLOGBOOK_DIR) -lDAQlogbook $(MYSQLLIBS)
endif
LDFLAGS +=-lpthread
include $(VMEBDIR)/vmeai.make.$(VMEDRIVER)
#OBJFILES=$(VMEBDIR)/cmdbase.o $(libobjs) rf2ttc_cf.o cordel.o rf2ttc.o 
OBJFILES=$(VMEBDIR)/cmdbase.o rf2ttc_cf.o cordel.o rf2ttc.o 
rf2ttc.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o rf2ttc.exe
rf2ttc_cf.o: rf2ttc_cf.c $(VMEBDIR)/vmeaistd.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) rf2ttc_cf.c
#rf2ttc_sim.o: rf2ttc_sim.c $(VMEBDIR)/vmesim.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) rf2ttc_sim.c
$(VMEBDIR)/cmdbase.o: $(VMEBDIR)/cmdbase.c $(VMEBDIR)/vmeaistd.h
	cd $(VMEBDIR); $(VMECC) -g -c $(CFLAGS) cmdbase.c -o $(VMEBDIR)/cmdbase.o
clean:
	rm -f rf2ttc_cf rf2ttc_cf.c rf2ttc_cf.py rf2ttc.make *.o *.pyc *.exe
cordel.o: cordel.c $(VMEBDIR)/vmeblib/vmewrap.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) cordel.c
rf2ttc.o: rf2ttc.c $(VMEBDIR)/vmeblib/vmewrap.h ttcmi.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) rf2ttc.c
