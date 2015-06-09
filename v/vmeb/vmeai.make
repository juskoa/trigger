# VMEDRVAR command line interface compile&link
# current dir: $(VMECFDIR)/BNAME
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

ifeq (BNAME,VME2FPGA)
  libobjs +=
endif
ifeq ($(findstring BNAME,ltu cosmicfo lvdst),BNAME)
  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib/linux_c -lltu
  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
endif
#ifeq (BNAME,cosmicfo)
#endif
#ifeq (BNAME,lvdst)
#  LDFLAGS +=-L$(VMECFDIR)/ltu/ltulib -lltu
#  #CFLAGS +=-I$(VMECFDIR)/ltu/ltulib
#endif
ifeq (BNAME,ctp)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-Lctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (BNAME,inputs)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
  COMMONCFLAGS +=-I$(DIMDIR)/dim
endif
ifeq (BNAME,ADCI)
  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (BNAME,ttcmi)
#  LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib/linux_c -lctp -L$(DIMDIR)/linux -ldim
  LDFLAGS +=-L$(DIMDIR)/linux -ldim
  INCDIRS=-I$(DIMDIR)/dim
#  CFLAGS +=-I$(VMECFDIR)/ctp/ctplib
endif
ifeq (BNAME,ctpt)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-Lctplib/linux_c -lctp
  CFLAGS +=-DSSMCONNECTIONS
endif
# ctplib has to be before vmeblib
LDFLAGS +=-L$(VMEBDIR)/vmeblib/linux_c -lvmeb
# dimlib has to be after vmeblib (at least for SIMVME)
ifdef DIMDIR
ifeq (BNAME,ctp)
  LDFLAGS +=-L$(DIMDIR)/linux -ldim
endif
ifeq (BNAME,inputs)
  #libobjs +=$(VMECFDIR)/ctp/ctplib/linux_c/libctp.a
  LDFLAGS +=-L$(DIMDIR)/linux -ldim
endif
endif
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
#OBJFILES=$(VMEBDIR)/cmdbase.o $(libobjs) BNAME_cf.o NEXTOBJFILES
OBJFILES=$(VMEBDIR)/cmdbase.o BNAME_cf.o NEXTOBJFILES
BNAME.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o BNAME.exe
BNAME_cf.o: BNAME_cf.c $(VMEBDIR)/vmeaistd.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) BNAME_cf.c
#BNAME_sim.o: BNAME_sim.c $(VMEBDIR)/vmesim.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) BNAME_sim.c
$(VMEBDIR)/cmdbase.o: $(VMEBDIR)/cmdbase.c $(VMEBDIR)/vmeaistd.h
	cd $(VMEBDIR); $(VMECC) -g -c $(CFLAGS) cmdbase.c -o $(VMEBDIR)/cmdbase.o
clean:
	rm -f BNAME_cf BNAME_cf.c BNAME_cf.py BNAME.make *.o *.pyc *.exe
