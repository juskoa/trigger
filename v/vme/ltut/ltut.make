# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/ltut
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

ifeq (ltut,VME2FPGA)
  libobjs +=
endif
ifeq ($(findstring ltut,ltu cosmicfo lvdst),ltut)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib/linux_c -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
#ifeq (ltut,cosmicfo)
#endif
#ifeq (ltut,lvdst)
#  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
#  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
#endif
ifeq (ltut,ctp)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-Lctplib/linux_c -lctp  -L$(DIMDIR)/linux -ldim
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ltut,inputs)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp  -L$(DIMDIR)/linux -ldim
  COMMONCFLAGS +=-I$(DIMDIR)/dim
endif
ifeq (ltut,ADCI)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ltut,ttcmi)
#  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp -L$(DIMDIR)/linux -ldim
  LDFLAGS +=-L$(DIMDIR)/linux -ldim
  INCDIRS=-I$(DIMDIR)/dim
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ltut,ctpt)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-Lctplib/linux_c -lctp
  CFLAGS +=-DSSMCONNECTIONS
endif
# ctplib has to be before vmeblib
LDFLAGS +=-L$(VMEBDIR)/vmeblib/linux_c -lvmeb
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
#OBJFILES=$(VMEBDIR)/cmdbase.o $(libobjs) ltut_cf.o ltut.o 
OBJFILES=$(VMEBDIR)/cmdbase.o ltut_cf.o ltut.o 
ltut.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o ltut.exe
ltut_cf.o: ltut_cf.c $(VMEBDIR)/vmeaistd.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ltut_cf.c
#ltut_sim.o: ltut_sim.c $(VMEBDIR)/vmesim.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ltut_sim.c
$(VMEBDIR)/cmdbase.o: $(VMEBDIR)/cmdbase.c $(VMEBDIR)/vmeaistd.h
	cd $(VMEBDIR); $(VMECC) -g -c $(CFLAGS) cmdbase.c -o $(VMEBDIR)/cmdbase.o
clean:
	rm -f ltut_cf ltut_cf.c ltut_cf.py ltut.make *.o *.pyc *.exe
ltut.o: ltut.c ltut.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ltut.c
