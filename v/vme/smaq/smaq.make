# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/smaq
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

ifeq (smaq,VME2FPGA)
  libobjs +=
endif
ifeq ($(findstring smaq,ltu cosmicfo lvdst),smaq)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib/linux_c -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
#ifeq (smaq,cosmicfo)
#endif
#ifeq (smaq,lvdst)
#  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
#  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
#endif
ifeq (smaq,ctp)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-Lctplib/linux_c -lctp  -L$(DIMDIR)/linux -ldim
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (smaq,inputs)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp  -L$(DIMDIR)/linux -ldim
  COMMONCFLAGS +=-I$(DIMDIR)/dim
endif
ifeq (smaq,ADCI)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (smaq,ttcmi)
#  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp -L$(DIMDIR)/linux -ldim
  LDFLAGS +=-L$(DIMDIR)/linux -ldim
  INCDIRS=-I$(DIMDIR)/dim
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (smaq,ctpt)
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
#OBJFILES=$(VMEBDIR)/cmdbase.o $(libobjs) smaq_cf.o checkdmp.o smaq.o intint.o ssm.o smaq2.o 
OBJFILES=$(VMEBDIR)/cmdbase.o smaq_cf.o checkdmp.o smaq.o intint.o ssm.o smaq2.o 
smaq.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o smaq.exe
smaq_cf.o: smaq_cf.c $(VMEBDIR)/vmeaistd.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) smaq_cf.c
#smaq_sim.o: smaq_sim.c $(VMEBDIR)/vmesim.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) smaq_sim.c
$(VMEBDIR)/cmdbase.o: $(VMEBDIR)/cmdbase.c $(VMEBDIR)/vmeaistd.h
	cd $(VMEBDIR); $(VMECC) -g -c $(CFLAGS) cmdbase.c -o $(VMEBDIR)/cmdbase.o
clean:
	rm -f smaq_cf smaq_cf.c smaq_cf.py smaq.make *.o *.pyc *.exe
smaq.o: smaq.c $(VMEBDIR)/vmeblib/vmewrap.h ../ctp/ctplib/ctplib.h ../ctp/ctp.h ../ctp_proxy/Tpartition.h shmaccess.h ssmctp.h intint.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) smaq.c
ssm.o: ssm.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ctplib.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ssm.c
checkdmp.o: checkdmp.c $(VMEBDIR)/vmeblib/vmewrap.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) checkdmp.c
smaq2.o: smaq2.c $(VMEBDIR)/vmeblib/vmewrap.h ../ctp/ctplib/ctplib.h ../ctp/ctp.h ../ctp_proxy/Tpartition.h shmaccess.h ssmctp.h intint.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) smaq2.c
intint.o: intint.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ctplib.h intint.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) intint.c
