SDIR= .
ODIR= linux
MYSQLLIBS=`/usr/bin/mysql_config --libs`
INFOLOG=/opt/infoLogger
DAQLOGBOOK=/opt/libDAQlogbook
EXTRALIBS = -L$(DIMDIR)/linux -ldim -L$(VMEBDIR)/vmeblib/linux_s -lvmeb \
 -L$(DAQLOGBOOK) -lDAQlogbook -L$(INFOLOG) -lInfo $(MYSQLLIBS) 
CC=$(VMEGCC)
SFX=c
#CC=g++
#SFX=cxx
CFLAGS = -Wall -g -I$(DIMDIR)/dim -I$(VMEBDIR)/vmeblib -I$(INFOLOG) \
 -I$(VMECFDIR)/ctp
ifeq ($(VMEGCC),g++)
CFLAGS +=-DCPLUSPLUS
endif

LDFLAGS = -lpthread

all: $(ODIR) $(ODIR)/dimccounters $(ODIR)/sendcomm
$(ODIR):
	mkdir -p $(ODIR)
$(ODIR)/dimccounters: $(ODIR)/dimccounters.o
	$(CC) $(LDFLAGS) $(ODIR)/dimccounters.o \
 -o $(ODIR)/dimccounters $(EXTRALIBS)
$(ODIR)/dimccounters2: $(ODIR)/dimccounters2.o
	$(CC) $(LDFLAGS) $(ODIR)/dimccounters2.o \
 -o $(ODIR)/dimccounters2 $(EXTRALIBS)
$(ODIR)/dimcswtrg: $(ODIR)/dimcswtrg.o
	$(CC) $(LDFLAGS) $(ODIR)/dimcswtrg.o \
 -o $(ODIR)/dimcswtrg $(EXTRALIBS)
$(ODIR)/server: $(ODIR)/server.o
	$(CC) $(LDFLAGS) $(ODIR)/server.o \
 -o $(ODIR)/server $(EXTRALIBS)
$(ODIR)/sendcomm: $(ODIR)/sendcomm.o
	$(CC) $(LDFLAGS) $(ODIR)/sendcomm.o \
 -o $(ODIR)/sendcomm $(EXTRALIBS)

$(ODIR)/dimccounters.o:$(SDIR)/dimccounters.$(SFX) ../ctp/ctpcounters.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/dimccounters.o dimccounters.$(SFX)
$(ODIR)/dimccounters2.o:$(SDIR)/dimccounters2.$(SFX) ../ctp/ctpcounters.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/dimccounters2.o dimccounters2.$(SFX)
$(ODIR)/dimcswtrg.o:$(SDIR)/dimcswtrg.$(SFX) $(SDIR)/dimctypes.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/dimcswtrg.o dimcswtrg.$(SFX)
$(ODIR)/server.o:$(SDIR)/server.c ../ctp/ctpcounters.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/server.o server.c
	$(CC) $(CFLAGS) -c -o $(ODIR)/dimccounters.o dimccounters.$(SFX)
$(ODIR)/sendcomm.o:$(SDIR)/sendcomm.c ../ctp/ctpcounters.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/sendcomm.o sendcomm.c
clean:
	-rm -f linux/*.o

