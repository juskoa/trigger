# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/ctpt
VMECC=gcc
COMMONCFLAGS=-Wall -DCERNLAB_SETUP -D$(VMEDRIVER) -I$(VMEBDIR)/vmeblib
#libobjs=$(VMECFDIR)/CTPcommon/snapshot.o $(VMECFDIR)/CTPcommon/vmefpga.o

ifeq (ctpt,VME2FPGA)
  libobjs +=
endif
ifeq (ctpt,ltu)
  LDFLAGS +=-Lltulib -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
ifeq (ctpt,lvdst)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
ifeq (ctpt,ctp)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/libctp.a
  LDFLAGS +=-Lctplib -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ctpt,ADCI)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (ctpt,ctpt)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/libctp.a
  LDFLAGS +=-Lctplib -lctp
  CFLAGS +=-DSSMCONNECTIONS
endif
LDFLAGS +=-L$(VMEBDIR)/vmeblib -lvmeb -lpthread
include $(VMEBDIR)/vmeai.make.$(VMEDRIVER)
#OBJFILES=$(VMEBDIR)/cmdbase.o $(libobjs) ctpt_cf.o ssmconl1.o data.o ssm.o ssmconl0.o ssmbrowser.o ssmconbu.o pastfut.o ssmconui.o ssmconfo.o ssmconnection.o ssmconl2.o ctpt.o none.o ssmconint.o 
OBJFILES=$(VMEBDIR)/cmdbase.o ctpt_cf.o ssmconl1.o data.o ssm.o ssmconl0.o ssmbrowser.o ssmconbu.o pastfut.o ssmconui.o ssmconfo.o ssmconnection.o ssmconl2.o ctpt.o none.o ssmconint.o 
ctpt.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o ctpt.exe
ctpt_cf.o: ctpt_cf.c $(VMEBDIR)/vmeaistd.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ctpt_cf.c
#ctpt_sim.o: ctpt_sim.c $(VMEBDIR)/vmesim.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ctpt_sim.c
$(VMEBDIR)/cmdbase.o: $(VMEBDIR)/cmdbase.c $(VMEBDIR)/vmeaistd.h
	cd $(VMEBDIR); $(VMECC) -g -c $(CFLAGS) cmdbase.c -o $(VMEBDIR)/cmdbase.o
clean:
	rm ctpt_cf; rm ctpt_cf.c; rm ctpt_cf.py; rm ctpt.make; rm *.o *.pyc *.exe
pastfut.o: pastfut.c ssmconnection.h
	 $(VMECC) -g -c $(CFLAGS) pastfut.c
ssm.o: ssm.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) ssm.c
ssmbrowser.o: ssmbrowser.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) ssmbrowser.c
ssmconbu.o: ssmconbu.c ssmconnection.h
	 $(VMECC) -g -c $(CFLAGS) ssmconbu.c
ssmconfo.o: ssmconfo.c ssmconnection.h
	 $(VMECC) -g -c $(CFLAGS) ssmconfo.c
ssmconl2.o: ssmconl2.c ssmconnection.h
	 $(VMECC) -g -c $(CFLAGS) ssmconl2.c
ssmconnection.o: ssmconnection.c ssmconnection.h
	 $(VMECC) -g -c $(CFLAGS) ssmconnection.c
ssmconui.o: ssmconui.c ssmconnection.h
	 $(VMECC) -g -c $(CFLAGS) ssmconui.c
none.o: none.c ssmconnection.h
	 $(VMECC) -g -c $(CFLAGS) none.c
ssmconl1.o: ssmconl1.c ssmconnection.h
	 $(VMECC) -g -c $(CFLAGS) ssmconl1.c
data.o: data.c ssmconnection.h
	 $(VMECC) -g -c $(CFLAGS) data.c
ctpt.o: ctpt.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) ctpt.c
ssmconint.o: ssmconint.c ssmconnection.h
	 $(VMECC) -g -c $(CFLAGS) ssmconint.c
ssmconl0.o: ssmconl0.c ssmconnection.h
	 $(VMECC) -g -c $(CFLAGS) ssmconl0.c
