# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/corde
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

ifeq (corde,VME2FPGA)
  libobjs +=
endif
ifeq (corde,ltu)
  LDFLAGS +=-Lltulib -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
ifeq (corde,cosmicfo)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
endif
ifeq (corde,lvdst)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
ifeq (corde,ctp)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/libctp.a
  LDFLAGS +=-Lctplib -lctp  -L$(DIMDIR)/linux -ldim
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (corde,ADCI)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (corde,ttcmi)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (corde,ctpt)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/libctp.a
  LDFLAGS +=-Lctplib -lctp
  CFLAGS +=-DSSMCONNECTIONS
endif
ifdef DATE_INFOLOGGER_DIR
INCDIRS=-I$(DATE_INFOLOGGER_DIR)
LDFLAGS +=-L$(VMEBDIR)/vmeblib -lvmeb -L$(DATE_INFOLOGGER_DIR) -lInfo
else
LDFLAGS +=-L$(VMEBDIR)/vmeblib -lvmeb
endif
ifdef DATE_DAQLOGBOOK_DIR
MYSQLLIBS=`/usr/bin/mysql_config --libs`
INCDIRS=-I$(DATE_DAQLOGBOOK_DIR)
LDFLAGS +=-L$(DATE_DAQLOGBOOK_DIR) -lDAQlogbook $(MYSQLLIBS)
endif
LDFLAGS +=-lpthread
include $(VMEBDIR)/vmeai.make.$(VMEDRIVER)
#OBJFILES=$(VMEBDIR)/cmdbase.o $(libobjs) corde_cf.o corde.o 
OBJFILES=$(VMEBDIR)/cmdbase.o corde_cf.o corde.o 
corde.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o corde.exe
corde_cf.o: corde_cf.c $(VMEBDIR)/vmeaistd.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) corde_cf.c
#corde_sim.o: corde_sim.c $(VMEBDIR)/vmesim.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) corde_sim.c
$(VMEBDIR)/cmdbase.o: $(VMEBDIR)/cmdbase.c $(VMEBDIR)/vmeaistd.h
	cd $(VMEBDIR); $(VMECC) -g -c $(CFLAGS) cmdbase.c -o $(VMEBDIR)/cmdbase.o
clean:
	rm corde_cf; rm corde_cf.c; rm corde_cf.py; rm corde.make; rm *.o *.pyc *.exe
corde.o: corde.c $(VMEBDIR)/vmeblib/vmewrap.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) corde.c
