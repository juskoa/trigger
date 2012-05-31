# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/ctp
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

ifeq (ctp,VME2FPGA)
  libobjs +=
endif
ifeq ($(findstring ctp,ltu cosmicfo lvdst),ltu)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib/linux_c -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
#ifeq (ctp,cosmicfo)
#endif
#ifeq (ctp,lvdst)
#  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
#  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
#endif
ifeq (ctp,ctp)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-Lctplib/linux_c -lctp  -L$(DIMDIR)/linux -ldim
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ctp,ADCI)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ctp,ttcmi)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ctp,ctpt)
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
#OBJFILES=$(VMEBDIR)/cmdbase.o $(libobjs) ctp_cf.o ssmbrowser.o intint.o ctp.o ssm.o toobusy.o 
OBJFILES=$(VMEBDIR)/cmdbase.o ctp_cf.o ssmbrowser.o intint.o ctp.o ssm.o toobusy.o 
ctp.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o ctp.exe
ctp_cf.o: ctp_cf.c $(VMEBDIR)/vmeaistd.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ctp_cf.c
#ctp_sim.o: ctp_sim.c $(VMEBDIR)/vmesim.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ctp_sim.c
$(VMEBDIR)/cmdbase.o: $(VMEBDIR)/cmdbase.c $(VMEBDIR)/vmeaistd.h
	cd $(VMEBDIR); $(VMECC) -g -c $(CFLAGS) cmdbase.c -o $(VMEBDIR)/cmdbase.o
clean:
	rm ctp_cf; rm ctp_cf.c; rm ctp_cf.py; rm ctp.make; rm *.o *.pyc *.exe
ssmbrowser.o: ssmbrowser.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ssmbrowser.c
ssm.o: ssm.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ctplib.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ssm.c
ctp.o: ctp.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ctplib.h $(VMEBDIR)/vmeblib/vmeblib.h shmaccess.h ssmctp.h ../ctp_proxy/Tpartition.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ctp.c
toobusy.o: toobusy.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ctplib.h $(VMEBDIR)/vmeblib/vmeblib.h ssmctp.h ../ctp_proxy/Tpartition.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) toobusy.c
intint.o: intint.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ctplib.h intint.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) intint.c
