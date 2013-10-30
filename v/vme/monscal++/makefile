SDIR= .
ODIR= linux
DAQDIR = /opt/libDAQlogbook
MYSQLLIBS=`/usr/bin/mysql_config --libs`
EXTRALIBS = -L$(VMEBDIR)/vmeblib/linux_s -lvmeb -L$(DIMDIR)/linux -ldim -L$(DAQDIR) -lDAQlogbook $(MYSQLLIBS) 
CC=g++
SFX=cxx
CFLAGS = -Wall -g3 -I$(DIMDIR)/dim -I$(VMEBDIR)/vmeblib -I$(VMECFDIR)/ctp -I$(DAQDIR)
#CFLAGS = -Wall -g3 -I$(DIMDIR)/dim -I$(VMEBDIR)/vmeblib  -I$(VMECFDIR)/ctp 
LDFLAGS = -lpthread
CFLAGS +=-DCPLUSPLUS

all: $(ODIR) $(ODIR)/MonScal
$(ODIR):
	mkdir -p $(ODIR)
$(ODIR)/MonScal: $(ODIR)/MonScal.o $(ODIR)/Dim.o $(ODIR)/TrigConf.o \
	$(ODIR)/ActiveRun.o $(ODIR)/Counter.o $(ODIR)/Output.o $(ODIR)/Log.o \
	$(ODIR)/main.o
	$(CC) $(LDFLAGS) $(ODIR)/MonScal.o $(ODIR)/Dim.o $(ODIR)/ActiveRun.o \
	$(ODIR)/TrigConf.o $(ODIR)/Counter.o $(ODIR)/Output.o $(ODIR)/Log.o \
	$(ODIR)/main.o \
	$(EXTRALIBS) -o $(ODIR)/MonScal
#	rm $(ODIR)/*.o

$(ODIR)/MonScal.o:$(SDIR)/MonScal.h $(SDIR)/MonScal.cxx
	$(CC) $(CFLAGS) -c -o $(ODIR)/MonScal.o MonScal.$(SFX)

$(ODIR)/Dim.o:$(SDIR)/Dim.h $(SDIR)/Dim.cxx  
	$(CC) $(CFLAGS) -c -o $(ODIR)/Dim.o Dim.cxx

$(ODIR)/TrigConf.o:$(SDIR)/TrigConf.h $(SDIR)/TrigConf.cxx  
	$(CC) $(CFLAGS) -c -o $(ODIR)/TrigConf.o TrigConf.cxx

$(ODIR)/ActiveRun.o:$(SDIR)/ActiveRun.h $(SDIR)/ActiveRun.cxx  
	$(CC) $(CFLAGS) -c -o $(ODIR)/ActiveRun.o ActiveRun.cxx

$(ODIR)/Counter.o:$(SDIR)/Counter.h $(SDIR)/Counter.cxx  
	$(CC) $(CFLAGS) -c -o $(ODIR)/Counter.o Counter.cxx

$(ODIR)/Output.o:$(SDIR)/Output.h $(SDIR)/Output.cxx  
	$(CC) $(CFLAGS) -c -o $(ODIR)/Output.o Output.cxx

$(ODIR)/Log.o:$(SDIR)/Log.h $(SDIR)/Log.cxx  
	$(CC) $(CFLAGS) -c -o $(ODIR)/Log.o Log.cxx

$(ODIR)/main.o:$(SDIR)/main.cxx  
	$(CC) $(CFLAGS) -c -o $(ODIR)/main.o main.cxx

clean:
	-rm $(ODIR)/*.o

