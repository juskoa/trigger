# test makefile
include makefile_common
SDIR= .
LIBS = $(EXTRALIBS)
ifeq ($(CPP),yes)
#all:	$(ODIR)/server $(ODIR)/client $(ODIR)/testServer $(ODIR)/testClient
all:	$(ODIR)/server $(ODIR)/client
else
all:	$(ODIR)/server $(ODIR)/client
endif

$(ODIR)/server:	$(SDIR)/server.c $(DIMDIR)/$(ODIR)/libdim.a
	$(CC) $(CFLAGS) $(LDFLAGS) $(SDIR)/server.c -L$(DIMDIR)/linux -ldim -o $(ODIR)/server $(LIBS)
	
$(ODIR)/client:	$(SDIR)/client.c $(DIMDIR)/$(ODIR)/libdim.a
	$(CC) $(CFLAGS) $(LDFLAGS) $(SDIR)/client.c -L$(DIMDIR)/linux -ldim -o $(ODIR)/client $(LIBS)
clean:
	-rm $(ODIR)/*.o $(ODIR)/client $(ODIR)/server

