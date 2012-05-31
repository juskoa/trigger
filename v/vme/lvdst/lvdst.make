# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/lvdst
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

ifeq (lvdst,VME2FPGA)
  libobjs +=
endif
ifeq (lvdst,ltu)
  LDFLAGS +=-Lltulib -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
ifeq (lvdst,cosmicfo)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
endif
ifeq (lvdst,lvdst)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
ifeq (lvdst,ctp)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/libctp.a
  LDFLAGS +=-Lctplib -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (lvdst,ADCI)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (lvdst,ctpt)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/libctp.a
  LDFLAGS +=-Lctplib -lctp
  CFLAGS +=-DSSMCONNECTIONS
endif
ifdef DATE_INFOLOGGER_DIR
INCDIRS=-I$(DATE_INFOLOGGER_DIR)
LDFLAGS +=-L$(VMEBDIR)/vmeblib -lvmeb -L$(DATE_INFOLOGGER_DIR) -lInfo -lpthread
else
LDFLAGS +=-L$(VMEBDIR)/vmeblib -lvmeb -lpthread
endif
include $(VMEBDIR)/vmeai.make.$(VMEDRIVER)
#OBJFILES=$(VMEBDIR)/cmdbase.o $(libobjs) lvdst_cf.o lvdst.o 
OBJFILES=$(VMEBDIR)/cmdbase.o lvdst_cf.o lvdst.o 
lvdst.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o lvdst.exe
lvdst_cf.o: lvdst_cf.c $(VMEBDIR)/vmeaistd.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) lvdst_cf.c
#lvdst_sim.o: lvdst_sim.c $(VMEBDIR)/vmesim.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) lvdst_sim.c
$(VMEBDIR)/cmdbase.o: $(VMEBDIR)/cmdbase.c $(VMEBDIR)/vmeaistd.h
	cd $(VMEBDIR); $(VMECC) -g -c $(CFLAGS) cmdbase.c -o $(VMEBDIR)/cmdbase.o
clean:
	rm lvdst_cf; rm lvdst_cf.c; rm lvdst_cf.py; rm lvdst.make; rm *.o *.pyc *.exe
lvdst.o: lvdst.c $(VMEBDIR)/vmeblib/vmewrap.h lvdst.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) lvdst.c
