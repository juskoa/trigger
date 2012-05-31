PY=/usr/include/python2.3
VMEBLIB=$(VMEBDIR)/vmeblib
CTPLIB=$(VMECFDIR)/ctp/ctplib
CFLAGS= -D$(VMEDRIVER) -I$(VMEBLIB)

ifeq ($(VMEDRIVER),VMERCC)
LDFLAGS=-L/lib/modules/daq -lvme_rcc -lrcc_error -lio_rcc -lcmem_rcc \
 -L$(CTPLIB) -lctp -L$(VMEBLIB) -lvmeb
#OBJFILES=../ctp/vmewrap.o ../ctp/ctp.o ../ctp/ssm.o shmaccess.o
OBJFILES=../ctp/ctp.o ../ctp/ssm.o shmaccess.o
else
OBJFILES=vmewrap.o vmesim.o shmaccess.o
endif

all:shmaccess.o hwreader shmpyext.so
hwreader:hwreader.c $(OBJFILES) shmaccess.h \
 $(VMEBLIB)/libvmeb.a $(CTPLIB)/libctp.a 
	gcc $(CFLAGS) hwreader.c $(OBJFILES) $(LDFLAGS) -o hwreader
shmaccess.o:shmaccess.c shmaccess.h
	gcc -c $(CFLAGS) shmaccess.c
shmpyext.so:shmpyext.c shmaccess.o shmaccess.h
	gcc $(CFLAGS) shmpyext.c -g -I$(PY) -fpic -shared shmaccess.o \
	-o shmpyext.so
vmewrap.o:$(VMEBDIR)/vmewrap.c $(VMEBDIR)/vmewrap.h
	gcc -c $(CFLAGS) $(VMEBDIR)/vmewrap.c
vmesim.o:$(VMEBDIR)/vmesim.c $(VMEBDIR)/vmesim.h
	gcc -c $(CFLAGS) $(VMEBDIR)/vmesim.c
clean:
	rm -f shmpyext.so *.o hwreader core


