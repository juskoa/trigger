# VMERCC command line interface compile&link
# current dir: $(VMECFDIR)/MORELTUS
VMECC=$(VMEGCC)
COMMONCFLAGS=-D$(VMEDRIVER) -I$(VMEBDIR)/vmeblib
include $(VMEBDIR)/vmeai.make.$(VMEDRIVER)

#OBJFILES= findltus.o vmewrap.o $(VMECFDIR)/ltu/ltu.o
#.PHONY: $(VMEBDIR)/vmewrap.h
OBJFILES= findltus.o vmewrap.o
findltus.exe: $(OBJFILES)
	$(VMECC) $(OBJFILES) $(LDFLAGS) -lm -o findltus.exe
vmewrap.o: $(VMEBDIR)/vmeblib/vmewrap.c $(VMEBDIR)/vmeblib/vmewrap.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) $(VMEBDIR)/vmeblib/vmewrap.c -o vmewrap.o

findltus.o: findltus.c $(VMEBDIR)/vmeblib/vmewrap.h $(VMECFDIR)/ltu/ltu.h
	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) \
        -o findltus.o findltus.c
#$(VMECFDIR)/ltu/ltu.o: $(VMECFDIR)/ltu/ltu.c $(VMEBDIR)/vmewrap.h $(VMECFDIR)/ltu/ltu.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) $(VMECFDIR)/ltu/ltu.c
#ADCI.o: ADCI.c $(VMECFDIR)/ltu/ltu.h $(VMEBDIR)/vmewrap.h
#	$(VMECC) -g -c $(CFLAGS) -I$(VMEBDIR) ADCI.c

