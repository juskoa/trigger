SDIR= .
ODIR= linux
EXTRALIBS = -L$(DIMDIR)/linux -ldim
VMEBLIB = -L$(VMEBDIR)/vmeblib/linux_s -lvmeb
ifdef REDIS
REDISLIB= -L$(REDIS)/deps/hiredis -l hiredis
REDISINC= -I$(REDIS)/deps/hiredis
endif
CC=$(VMEGCC)
ifeq ($(CC),g++)
CFLAGS =-DCPLUSPLUS
else
CFLAGS =
endif

CFLAGS += -Wall -g  -c
INCS = -I$(DIMDIR)/dim -I$(VMEBDIR)/vmeblib
LDFLAGS = -pthread 
LIBS = -L$(VMEBDIR)/vmeblib/linux_s -lvmeb

ALL: $(ODIR)/readctpc $(ODIR)/readltuc $(ODIR)/analyse $(ODIR)/udpmonitor \
 $(ODIR)/udpclient

$(ODIR)/readctpc: $(ODIR)/readctpc.o $(ODIR)/common.o
	$(CC) $(LDFLAGS) -lm $< $(LIBS) $(REDISLIB) $(ODIR)/common.o \
 -o $@ $(EXTRALIBS)
$(ODIR)/readltuc: $(ODIR)/readltuc.o $(ODIR)/common.o
	$(CC) $(LDFLAGS) $< $(LIBS) $(ODIR)/common.o \
 -o $@ $(EXTRALIBS)
$(ODIR)/analyse: $(ODIR)/analyse.o $(ODIR)/common.o
	$(CC) $(LDFLAGS) $< $(ODIR)/common.o \
 -o $@
$(ODIR)/udpmonitor: $(ODIR)/udpmonitor.o 
	$(CC) $(LDFLAGS) $(LIBS) -o $@ $< \
 -L$(VMEBDIR)/vmeblib/linux_s -lvmeb
$(ODIR)/udpclient: $(ODIR)/udpclient.o
	$(CC) $(LDFLAGS) -lm $(LIBS) -o $@ $< \
 -L$(VMEBDIR)/vmeblib/linux_s -lvmeb

$(ODIR)/readctpc.o:$(SDIR)/readctpc.c $(SDIR)/common.h
	-mkdir -p $(ODIR)
	$(CC) $(CFLAGS) -o $@ $(INCS) $(REDISINC) $<
$(ODIR)/readltuc.o:$(SDIR)/readltuc.c $(SDIR)/common.h
	$(CC) $(CFLAGS) -o $@ $(INCS) $<
$(ODIR)/common.o:$(SDIR)/common.c $(SDIR)/common.h
	$(CC) $(CFLAGS) -o $@ $(INCS) $<
$(ODIR)/analyse.o:$(SDIR)/analyse.c $(SDIR)/common.h
	$(CC) $(CFLAGS) -o $@ $<
$(ODIR)/udpmonitor.o:$(SDIR)/udpmonitor.c
	$(CC) $(CFLAGS) -o $@ $(INCS) $<
$(ODIR)/udpclient.o:$(SDIR)/udpclient.c
	$(CC) $(CFLAGS) -o $@ $(INCS) $<
clean:
	rm -f linux/*
