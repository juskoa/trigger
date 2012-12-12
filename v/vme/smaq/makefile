# cd $(VMECFDIR)/smaq ; make
include $(VMEBDIR)/com_defs.make
#with infoLogger:
#include $(VMEBDIR)/daq_defs.make
#CFLAGS=$(COMMONCFLAGS) -I$(VMEBinc) -I$(CTPinc) -I$(DIMinc) $(daqINCDIRS)
CFLAGS=$(COMMONCFLAGS) -I$(VMEBinc) -I$(CTPinc)
#$(info CFLAGS: $(CFLAGS))

#LIBS = $(CTPLD) $(VMEBLD) $(VMERCCLD) $(DIMLD)
LIBS = $(CTPLD) $(VMEBLD) $(VMERCCLD)
LDFLAGS += -lm

ifdef SERVER_LINK
  $(error Error: to be executed on $(CLIENT_HOST))
endif
OBJFILES=smaq.o ssm.o intint.o
OBJFILES2=smaq2.o ssm.o intint.o
all: smaqold.exe smaq2.exe
smaqold.exe: $(OBJFILES)
	$(CC) $(LDFLAGS) $(OBJFILES) $(LIBS) -o $@
smaq2.exe: $(OBJFILES2) $(CTPLIB)/libctp.a 
	$(CC) $(LDFLAGS) $(OBJFILES2) $(LIBS) -o $@
	ln -sf smaq2.exe smaq.exe
ssm.o: ssm.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h
	$(CC) -g -c $(CFLAGS) -o $@ $<
intint.o: intint.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ctplib.h ssmctp.h intint.h
	$(CC) -g -c $(CFLAGS) -o $@ $<
smaq.o: smaq.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h 
	$(CC) -g -c $(CFLAGS) -o $@ $<
smaq2.o: smaq2.c $(VMEBDIR)/vmeblib/vmewrap.h ctp.h ssmctp.h 
#	echo CFLAGS: $(CFLAGS)
	$(CC) -g -c $(CFLAGS) -o $@ $<
clean:
	rm *.o *.pyc *.exe
