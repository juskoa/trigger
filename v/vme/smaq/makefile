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

LDFLAGS +=-lpthread -L$(VMECFDIR)/ctp/ctplib/linux_c -lctp -L$(VMEBDIR)/vmeblib/linux_c -lvmeb

ifdef DATE_INFOLOGGER_DIR
INCDIRS=-I$(DIMDIR)/dim -I$(DATE_INFOLOGGER_DIR) \
 -I$(VMECFDIR)/ctp_proxy -I$(VMECFDIR)/ctp/ctplib
LDFLAGS +=-L$(DIMDIR)/linux -ldim -L$(DATE_INFOLOGGER_DIR) -lInfo
endif
include $(VMEBDIR)/vmeai.make.$(VMEDRIVER)

OBJFILES=smaq.o ssm.o intint.o
smaq.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o smaq.exe
ssm.o: ssm.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) ssm.c
intint.o: intint.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ctplib.h ssmctp.h intint.h
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) intint.c
smaq.o: smaq.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h 
	 $(VMECC) -g -c $(CFLAGS) $(INCDIRS) smaq.c
clean:
	rm *.o *.pyc *.exe
