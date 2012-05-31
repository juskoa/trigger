# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/cosmicfo
VMECC=gcc
COMMONCFLAGS=-Wall -DCERNLAB_SETUP -D$(VMEDRIVER) -I$(VMEBDIR)/vmeblib
#libobjs=$(VMECFDIR)/CTPcommon/snapshot.o $(VMECFDIR)/CTPcommon/vmefpga.o

ifeq (cosmicfo,VME2FPGA)
  libobjs +=
endif
ifeq (cosmicfo,ltu)
  LDFLAGS +=-Lltulib -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
ifeq (cosmicfo,cosmicfo)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
ifeq (cosmicfo,lvdst)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
ifeq (cosmicfo,ctp)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/libctp.a
  LDFLAGS +=-Lctplib -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (cosmicfo,ADCI)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (cosmicfo,ctpt)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/libctp.a
  LDFLAGS +=-Lctplib -lctp
  CFLAGS +=-DSSMCONNECTIONS
endif
LDFLAGS +=-L$(VMEBDIR)/vmeblib -lvmeb -lpthread
include $(VMEBDIR)/vmeai.make.$(VMEDRIVER)
#OBJFILES=$(VMEBDIR)/cmdbase.o $(libobjs) cosmicfo_cf.o cosmicfo.o 
OBJFILES=$(VMEBDIR)/cmdbase.o cosmicfo_cf.o cosmicfo.o 
cosmicfo.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o cosmicfo.exe
cosmicfo_cf.o: cosmicfo_cf.c $(VMEBDIR)/vmeaistd.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) cosmicfo_cf.c
#cosmicfo_sim.o: cosmicfo_sim.c $(VMEBDIR)/vmesim.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) cosmicfo_sim.c
$(VMEBDIR)/cmdbase.o: $(VMEBDIR)/cmdbase.c $(VMEBDIR)/vmeaistd.h
	cd $(VMEBDIR); $(VMECC) -g -c $(CFLAGS) cmdbase.c -o $(VMEBDIR)/cmdbase.o
clean:
	rm cosmicfo_cf; rm cosmicfo_cf.c; rm cosmicfo_cf.py; rm cosmicfo.make; rm *.o *.pyc *.exe
cosmicfo.o: cosmicfo.c ../../vmeb/vmeblib/lexan.h $(VMEBDIR)/vmeblib/vmewrap.h cosmicfo.h
	 $(VMECC) -g -c $(CFLAGS) cosmicfo.c
