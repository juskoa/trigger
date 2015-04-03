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

all: client clientchar server
clientchar: clientchar.o
	$(CC) $(LDFLAGS) clientchar.o $(EXTRALIBS) -o clientchar
client: client.o
	$(CC) $(LDFLAGS) client.o $(EXTRALIBS) -o client
server: server.o
	$(CC) $(LDFLAGS) server.o $(EXTRALIBS) -o server


clientchar.o:clientchar.cxx  
	$(CC) $(CFLAGS) -c -o clientchar.o clientchar.cxx

client.o:client.cxx  
	$(CC) $(CFLAGS) -c -o client.o client.cxx

server.o:server.cxx  
	$(CC) $(CFLAGS) -c -o server.o server.cxx

clean:
	-rm -f *.o

