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

all: client clientchar server serverchar clientdouble serverdouble
clientchar: clientchar.o
	$(CC) $(LDFLAGS) clientchar.o $(EXTRALIBS) -o clientchar
serverchar: serverchar.o
	$(CC) $(LDFLAGS) serverchar.o $(EXTRALIBS) -o serverchar
client: client.o
	$(CC) $(LDFLAGS) client.o $(EXTRALIBS) -o client
server: server.o
	$(CC) $(LDFLAGS) server.o $(EXTRALIBS) -o server
clientdouble: clientdouble.o
	$(CC) $(LDFLAGS) clientdouble.o $(EXTRALIBS) -o clientdouble
serverdouble: serverdouble.o
	$(CC) $(LDFLAGS) serverdouble.o $(EXTRALIBS) -o serverdouble


clientchar.o:clientchar.cxx  
	$(CC) $(CFLAGS) -c -o clientchar.o clientchar.cxx

serverchar.o:serverchar.cxx  
	$(CC) $(CFLAGS) -c -o serverchar.o serverchar.cxx

client.o:client.cxx  
	$(CC) $(CFLAGS) -c -o client.o client.cxx

server.o:server.cxx  
	$(CC) $(CFLAGS) -c -o server.o server.cxx

clientdouble.o:clientdouble.cxx  
	$(CC) $(CFLAGS) -c -o clientdouble.o clientdouble.cxx

serverdouble.o:serverdouble.cxx  
	$(CC) $(CFLAGS) -c -o serverdouble.o serverdouble.cxx

clean:
	-rm -f *.o

