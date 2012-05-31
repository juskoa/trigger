VMEB=$(VMEBDIR)/vmeblib
GCC=gcc 
#MYSQLCFLAGS=`mysql_config --cflags`
MYSQLCFLAGS=-I/usr/include/mysql
#CFLAGS= -Wall -g -DCERNLAB_SETUP     or CAVERN_SETUP
ifeq ($(VMESITE), ALICE)
WHICHSETUP = CAVERN_SETUP
else
WHICHSETUP = CERNLAB_SETUP
endif
CFLAGS= -Wall -g -D$(WHICHSETUP) -D$(VMEDRIVER) \
 -I $(VMEB) -I $(VMECFDIR)/ctp -I $(VMECFDIR)/ctp_proxy 

all: readBICfile.o checkCTP.o notInCrate.o readCounters.o \
 initCTP.o readTables.o swtrigger.o scope.o timingpars.o i2ctemp.o \
 inputsTools.o busyTools.o calcOverlap.o dumpCTP.o phase.o
# mysqlwrap.o
	-rm libctp.a
	ar rs libctp.a *.o

.o: .c 
readBICfile.o: readBICfile.c $(VMECFDIR)/ctp/ctp.h
swtrigger.o: swtrigger.c $(VMECFDIR)/ctp_proxy/Tpartition.h
readTables.o: readTables.c $(VMEB)/lexan.h $(VMECFDIR)/ctp_proxy/Tpartition.h
	$(GCC) $< -c -o $@ $(CFLAGS) $(MYSQLCFLAGS)
mysqlwrap.o: mysqlwrap.c
	$(GCC) $< -c -g -o $@ $(MYSQLCFLAGS)
