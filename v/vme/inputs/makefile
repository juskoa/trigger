# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/inputs
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

ifeq (inputs,VME2FPGA)
  libobjs +=
endif
ifeq ($(findstring inputs,ltu cosmicfo lvdst),ltu)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib/linux_c -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
#ifeq (inputs,cosmicfo)
#endif
#ifeq (inputs,lvdst)
#  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
#  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
#endif
ifeq (inputs,inputs)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp -L$(DIMDIR)/linux -ldim
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (inputs,ttcmi)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (inputs,ctpt)
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
#OBJFILES=$(VMEBDIR)/cmdbase.o $(libobjs) inputs_cf.o signature.o ssmbrowser.o inputs.o ssm.o alignment.o sync.o 
OBJFILES=$(VMEBDIR)/cmdbase.o inputs_cf.o signature.o ssmbrowser.o inputs.o ssm.o alignment.o sync.o 
inputs.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o inputs.exe
inputs_cf.o: inputs_cf.c $(VMEBDIR)/vmeaistd.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) inputs_cf.c
#inputs_sim.o: inputs_sim.c $(VMEBDIR)/vmesim.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) inputs_sim.c
$(VMEBDIR)/cmdbase.o: $(VMEBDIR)/cmdbase.c $(VMEBDIR)/vmeaistd.h
	cd $(VMEBDIR); $(VMECC) -g -c $(CFLAGS) cmdbase.c -o $(VMEBDIR)/cmdbase.o
clean:
	rm inputs_cf; rm inputs_cf.c; rm inputs_cf.py; rm inputs.make; rm *.o *.pyc *.exe
ssm.o: ssm.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ssm.c
ssmbrowser.o: ssmbrowser.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ssmbrowser.c
alignment.o: alignment.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) alignment.c
signature.o: signature.c
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) signature.c
inputs.o: inputs.c $(VMEBDIR)/vmeblib/vmewrap.h ../ctp/ctplib/ctplib.h ../ctp/ctp.h shmaccess.h ../ctp_proxy/Tpartition.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) inputs.c
sync.o: sync.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) sync.c
