# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/ttcvi
VMECC=gcc
COMMONCFLAGS=-Wall -DCERNLAB_SETUP -D$(VMEDRIVER) -I$(VMEBDIR)/vmeblib
#libobjs=$(VMECFDIR)/CTPcommon/snapshot.o $(VMECFDIR)/CTPcommon/vmefpga.o

ifeq (ttcvi,VME2FPGA)
  libobjs +=
endif
ifeq (ttcvi,ltu)
  LDFLAGS +=-Lltulib -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
ifeq (ttcvi,cosmicfo)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
endif
ifeq (ttcvi,lvdst)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
ifeq (ttcvi,ctp)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/libctp.a
  LDFLAGS +=-Lctplib -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ttcvi,ADCI)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ttcvi,ctpt)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/libctp.a
  LDFLAGS +=-Lctplib -lctp
  CFLAGS +=-DSSMCONNECTIONS
endif
LDFLAGS +=-L$(VMEBDIR)/vmeblib -lvmeb -lpthread
include $(VMEBDIR)/vmeai.make.$(VMEDRIVER)
#OBJFILES=$(VMEBDIR)/cmdbase.o $(libobjs) ttcvi_cf.o ttcvi.o 
OBJFILES=$(VMEBDIR)/cmdbase.o ttcvi_cf.o ttcvi.o 
ttcvi.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o ttcvi.exe
ttcvi_cf.o: ttcvi_cf.c $(VMEBDIR)/vmeaistd.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ttcvi_cf.c
#ttcvi_sim.o: ttcvi_sim.c $(VMEBDIR)/vmesim.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ttcvi_sim.c
$(VMEBDIR)/cmdbase.o: $(VMEBDIR)/cmdbase.c $(VMEBDIR)/vmeaistd.h
	cd $(VMEBDIR); $(VMECC) -g -c $(CFLAGS) cmdbase.c -o $(VMEBDIR)/cmdbase.o
clean:
	rm ttcvi_cf; rm ttcvi_cf.c; rm ttcvi_cf.py; rm ttcvi.make; rm *.o *.pyc *.exe
ttcvi.o: ttcvi.c $(VMEBDIR)/vmeblib/vmewrap.h ttcvi.h
	 $(VMECC) -g -c $(CFLAGS) ttcvi.c
