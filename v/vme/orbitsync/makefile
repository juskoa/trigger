CC = $(VMEGCC)
COMMONCFLAGS=-g -Wall -I$(VMEBDIR)/vmeblib
LDFLAGS=-L$(VMEBDIR)/vmeblib/linux_c -lvmeb
include $(VMEBDIR)/vmeai.make.$(VMEDRIVER)

ifeq ($(VMEGCC),g++)
CFLAGS +=-DCPLUSPLUS
endif

all: myudplib.exe orbitsyn.exe
myudplib.exe: myudplib.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@
orbitsyn.exe: orbitsyn.c
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@
clean:
	-rm -f myudplib.exe orbitsyn.exe
