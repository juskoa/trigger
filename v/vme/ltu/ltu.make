# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/ltu
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

ifeq (ltu,VME2FPGA)
  libobjs +=
endif
ifeq ($(findstring ltu,ltu cosmicfo lvdst),ltu)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib/linux_c -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
#ifeq (ltu,cosmicfo)
#endif
#ifeq (ltu,lvdst)
#  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
#  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
#endif
ifeq (ltu,ctp)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-Lctplib/linux_c -lctp  -L$(DIMDIR)/linux -ldim
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ltu,ADCI)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ltu,ttcmi)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ltu,ctpt)
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
#OBJFILES=$(VMEBDIR)/cmdbase.o $(libobjs) ltu_cf.o ltu.o 
OBJFILES=$(VMEBDIR)/cmdbase.o ltu_cf.o ltu.o 
ltu.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o ltu.exe
ltu_cf.o: ltu_cf.c $(VMEBDIR)/vmeaistd.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ltu_cf.c
#ltu_sim.o: ltu_sim.c $(VMEBDIR)/vmesim.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ltu_sim.c
$(VMEBDIR)/cmdbase.o: $(VMEBDIR)/cmdbase.c $(VMEBDIR)/vmeaistd.h
	cd $(VMEBDIR); $(VMECC) -g -c $(CFLAGS) cmdbase.c -o $(VMEBDIR)/cmdbase.o
clean:
	rm ltu_cf; rm ltu_cf.c; rm ltu_cf.py; rm ltu.make; rm *.o *.pyc *.exe
ltu.o: ltu.c ../../vmeb/vmeblib/lexan.h $(VMEBDIR)/vmeblib/vmewrap.h ltu.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ltu.c
