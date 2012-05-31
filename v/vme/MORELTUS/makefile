# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/MORELTUS
PGM=irrmon
VMECC=$(VMEGCC)
COMMONCFLAGS=-D$(VMEDRIVER) -I$(VMEBDIR)/vmeblib
CTPCOMMON=../CTPcommon
include $(VMEBDIR)/vmeai.make.$(VMEDRIVER)
ifeq ($(VMEGCC),g++)
CCDEFS +=-DCPLUSPLUS
endif

LIBDEP=$(VMEBDIR)/vmeblib/linux_c/libvmeb.a
LDFLAGS+=-L $(VMEBDIR)/vmeblib/linux_c -lvmeb

#OBJFILES= findltus.o $(VMECFDIR)/ltu/ltu.o
OBJFILES= findltus.o

all: findboards.exe readtemp.exe

findboards.exe: findboards.o $(LIBDEP)
	$(VMECC) findboards.o $(LDFLAGS) -lm -o findboards.exe
$(PGM).exe: $(PGM).o $(VMEBDIR)/vmeblib/linux_c/libvmeb.a
	$(VMECC) $(PGM).o $(LDFLAGS) -lm -o $(PGM).exe
readtemp.exe: readtemp.o $(LIBDEP) 
	$(VMECC) readtemp.o $(LDFLAGS) -lm -o readtemp.exe
findxltus.exe: findxltus.o $(LIBDEP)
	$(VMECC) findxltus.o $(LDFLAGS) -lm -o findxltus.exe
findltus.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o findltus.exe
findboards.o: findboards.c
	$(VMECC) -c $(CFLAGS) -o findboards.o findboards.c

$(PGM).o: $(PGM).c
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) \
	-o $(PGM).o $(PGM).c
readtemp.o: readtemp.c
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) -I$(VMECFDIR)/ltu \
        -o readtemp.o readtemp.c
findxltus.o: findxltus.c $(VMECFDIR)/ltu/ltu.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) \
        -o findxltus.o findxltus.c
findltus.o: findltus.c $(VMECFDIR)/ltu/ltu.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) \
        -o findltus.o findltus.c
clean:
	-rm *.o
#$(VMECFDIR)/ltu/ltu.o: $(VMECFDIR)/ltu/ltu.c $(VMEBDIR)/vmewrap.h $(VMECFDIR)/ltu/ltu.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) $(VMECFDIR)/ltu/ltu.c
#ADCI.o: ADCI.c $(VMECFDIR)/ltu/ltu.h $(VMEBDIR)/vmewrap.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ADCI.c

