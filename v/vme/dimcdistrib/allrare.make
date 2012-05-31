SDIR= .
ODIR=linux
LDFLAGS= -lpthread
CC= gcc
CFLAGS= -Wall -g -I$(DIMDIR)/dim
EXTRALIBS = -L$(DIMDIR)/linux -ldim

$(ODIR)/allrare_client: $(ODIR)/allrare_client.o
	$(CC) $(LDFLAGS) $(ODIR)/allrare_client.o \
 -o $(ODIR)/allrare_client $(EXTRALIBS)
$(ODIR)/allrare_client.o:$(SDIR)/allrare_client.c $(SDIR)/dimctypes.h
	$(CC) $(CFLAGS) -c -o $(ODIR)/allrare_client.o allrare_client.c

