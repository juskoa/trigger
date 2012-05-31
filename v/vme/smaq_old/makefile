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

EXTRALIBS = -L$(DIMDIR)/linux -ldim 

LDFLAGS +=-L$(VMECFDIR)/ctp/ctplib -lctp

ifdef DATE_INFOLOGGER_DIR
INCDIRS=-I$(DIMDIR)/dim -I$(DATE_INFOLOGGER_DIR) \
 -I$(VMECFDIR)/ctp_proxy -I$(VMECFDIR)/ctp/ctplib
LDFLAGS +=-L$(VMEBDIR)/vmeblib/linux_c -lvmeb -L$(VMECFDIR)/ctp/ctplib/linux_c -lctp -L$(DIMDIR)/linux -ldim -L$(DATE_INFOLOGGER_DIR) -lInfo -lpthread
else
LDFLAGS +=-L$(VMEBDIR)/vmeblib/linux_c -lvmeb -lpthread
endif
include $(VMEBDIR)/vmeai.make.$(VMEDRIVER)

OBJFILES=ssm.o smaq.o  
smaq.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o smaq.exe
ssm.o: ssm.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ssm.c
smaq.o: smaq.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h 
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) smaq.c
clean:
	rm *.o *.pyc *.exe
