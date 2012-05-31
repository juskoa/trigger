SDIR= .
ODIR= linux
EXTRALIBS = -L$(DIMDIR)/linux -ldim
CC=$(VMEGCC)
SFX=c
CFLAGS = -Wall -g -I$(DIMDIR)/dim
LDFLAGS = -lpthread
ifeq ($(VMEGCC),g++)
CFLAGS +=-DCPLUSPLUS
endif

ALL: $(ODIR)/dimcswtrg $(ODIR)/dimccounters $(ODIR)/allrare_client

$(ODIR)/ttcrxreset: $(ODIR)/ttcrxreset.o
	$(CC) $(LDFLAGS) $(ODIR)/ttcrxreset.o \
 -o $(ODIR)/ttcrxreset $(EXTRALIBS)
$(ODIR)/dimccounters: $(ODIR)/dimccounters.o
	$(CC) $(LDFLAGS) $(ODIR)/dimccounters.o \
 -o $(ODIR)/dimccounters $(EXTRALIBS)
$(ODIR)/dimconce_only: $(ODIR)/dimconce_only.o
	$(CC) $(LDFLAGS) $(ODIR)/dimconce_only.o \
 -o $(ODIR)/dimconce_only $(EXTRALIBS)
$(ODIR)/dimccounters2: $(ODIR)/dimccounters2.o
	$(CC) $(LDFLAGS) $(ODIR)/dimccounters2.o \
 -o $(ODIR)/dimccounters2 $(EXTRALIBS)
$(ODIR)/allrare_client: $(ODIR)/allrare_client.o
	$(CC) $(LDFLAGS) $(ODIR)/allrare_client.o \
 -o $(ODIR)/allrare_client $(EXTRALIBS)
$(ODIR)/dimcswtrg: $(ODIR)/dimcswtrg.o
	$(CC) $(LDFLAGS) $(ODIR)/dimcswtrg.o \
 -o $@ $(EXTRALIBS)
$(ODIR)/dimcltus: $(ODIR)/dimcltus.o
	$(CC) $(LDFLAGS) $(ODIR)/dimcltus.o \
 -o $(ODIR)/dimcltus $(EXTRALIBS)
$(ODIR)/server: $(ODIR)/server.o
	$(CC) $(LDFLAGS) $(ODIR)/server.o \
 -o $(ODIR)/server $(EXTRALIBS)

$(ODIR)/ttcrxreset.o:$(SDIR)/ttcrxreset.$(SFX) $(SDIR)/ctpcounters.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/ttcrxreset.o ttcrxreset.$(SFX)
$(ODIR)/dimccounters.o:$(SDIR)/dimccounters.$(SFX) $(SDIR)/ctpcounters.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/dimccounters.o dimccounters.$(SFX)
$(ODIR)/dimconce_only.o:$(SDIR)/dimconce_only.$(SFX) $(SDIR)/ctpcounters.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/dimconce_only.o dimconce_only.$(SFX)
$(ODIR)/dimccounters2.o:$(SDIR)/dimccounters2.$(SFX) $(SDIR)/ctpcounters.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/dimccounters2.o dimccounters2.$(SFX)
$(ODIR)/allrare_client.o:$(SDIR)/allrare_client.$(SFX) $(SDIR)/dimctypes.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/allrare_client.o allrare_client.$(SFX)
$(ODIR)/dimcswtrg.o:$(SDIR)/dimcswtrg.$(SFX) $(SDIR)/dimctypes.h
	$(CC) $(CFLAGS) -c -o $@ dimcswtrg.$(SFX)
$(ODIR)/dimcltus.o:$(SDIR)/dimcltus.$(SFX) $(SDIR)/dimctypes.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/dimcltus.o dimcltus.$(SFX)
$(ODIR)/server.o:$(SDIR)/server.c $(SDIR)/ctpcounters.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/server.o server.c
	$(CC) $(CFLAGS) -c -o $(ODIR)/dimccounters.o dimccounters.$(SFX)
clean:
	-rm $(ODIR)/*.o

