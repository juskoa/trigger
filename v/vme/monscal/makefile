SDIR= .
ODIR= linux
MYSQLLIBS=`/usr/bin/mysql_config --libs`
INFOLOG=/opt/infoLogger
DAQLOGBOOK=/opt/libDAQlogbook
EXTRALIBS = -L$(DIMDIR)/linux -ldim -L$(VMEBDIR)/vmeblib -lvmeb \
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

all: $(ODIR)/monscal $(ODIR)/sendcomm
$(ODIR)/monscal: $(ODIR)/monscal.o
	$(CC) $(LDFLAGS) $(ODIR)/monscal.o \
 -o $(ODIR)/monscal $(EXTRALIBS)
$(ODIR)/monscal2: $(ODIR)/monscal2.o
	$(CC) $(LDFLAGS) $(ODIR)/monscal2.o \
 -o $(ODIR)/monscal2 $(EXTRALIBS)
$(ODIR)/dimcswtrg: $(ODIR)/dimcswtrg.o
	$(CC) $(LDFLAGS) $(ODIR)/dimcswtrg.o \
 -o $(ODIR)/dimcswtrg $(EXTRALIBS)
$(ODIR)/server: $(ODIR)/server.o
	$(CC) $(LDFLAGS) $(ODIR)/server.o \
 -o $(ODIR)/server $(EXTRALIBS)
$(ODIR)/sendcomm: $(ODIR)/sendcomm.o
	$(CC) $(LDFLAGS) $(ODIR)/sendcomm.o \
 -o $(ODIR)/sendcomm $(EXTRALIBS)

$(ODIR)/monscal.o:$(SDIR)/monscal.$(SFX) ../ctp/ctpcounters.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/monscal.o monscal.$(SFX)
$(ODIR)/monscal2.o:$(SDIR)/monscal2.$(SFX) ../ctp/ctpcounters.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/monscal2.o monscal2.$(SFX)
$(ODIR)/dimcswtrg.o:$(SDIR)/dimcswtrg.$(SFX) $(SDIR)/dimctypes.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/dimcswtrg.o dimcswtrg.$(SFX)
$(ODIR)/server.o:$(SDIR)/server.c ../ctp/ctpcounters.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/server.o server.c
	$(CC) $(CFLAGS) -c -o $(ODIR)/monscal.o monscal.$(SFX)
$(ODIR)/sendcomm.o:$(SDIR)/sendcomm.c ../ctp/ctpcounters.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/sendcomm.o sendcomm.c
clean:
	-rm linux/*.o

