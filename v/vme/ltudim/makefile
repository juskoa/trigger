# test makefile
include makefile_common
SDIR= .
LIBS = $(EXTRALIBS)

#ALL: $(ODIR)/ltuclient $(ODIR)/ltucmd $(ODIR)/cmd1
ALL: $(ODIR)/ltuclient $(ODIR)/ltucmd $(ODIR)/cmd1

#$(ODIR)/ltuserver: $(SDIR)/ltuserver.c $(ODIR)/popen2.o \
#	$(SDIR)/ltudimservices.o $(DIMDIR)/$(ODIR)/libdim.a
#	$(CC) $(CFLAGS) $(LDFLAGS) $(SDIR)/ltuserver.c $(ODIR)/popen2.o \
#$(SDIR)/ltudimservices.o -L$(DIMDIR)/linux -ldim -o $(ODIR)/ltuserver $(LIBS)
	
#$(ODIR)/popen2.o:$(SDIR)/popen2.c
#	$(CC) $(CFLAGS) -c -o $(ODIR)/popen2.o popen2.c
#$(ODIR)/ltudimservices.o:$(SDIR)/ltudimservices.c
#	$(CC) $(CFLAGS) -c -o $(ODIR)/ltudimservices.o ltudimservices.c
$(ODIR)/ltuclient:      $(SDIR)/ltuclient.c $(DIMDIR)/$(ODIR)/libdim.a
	mkdir -p linux
	$(CC) $(CFLAGS) $(LDFLAGS) $(SDIR)/ltuclient.c -L$(DIMDIR)/linux -ldim -o $(ODIR)/ltuclient $(LIBS)

$(ODIR)/ltucmd: $(SDIR)/ltucmd.c $(DIMDIR)/$(ODIR)/libdim.a
	$(CC) $(CFLAGS) $(LDFLAGS) $(SDIR)/ltucmd.c -L$(DIMDIR)/linux -ldim -o $(ODIR)/ltucmd $(LIBS)
$(ODIR)/cmd1: $(SDIR)/cmd1.c $(DIMDIR)/$(ODIR)/libdim.a
	$(CC) $(CFLAGS) $(LDFLAGS) $(SDIR)/cmd1.c -L$(DIMDIR)/linux -ldim -o $(ODIR)/cmd1 $(LIBS)
clean:
	-rm $(ODIR)/ltuclient $(ODIR)/ltucmd $(ODIR)/cmd1

