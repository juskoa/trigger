ODIR= .
SDIR= .
CFLAGS =-DCPLUSPLUS
CC=g++
ALL: $(ODIR)/analyse
$(ODIR)/analyse: $(ODIR)/analyse.o $(ODIR)/common.o
	$(CC) $(LDFLAGS) $< $(ODIR)/common.o -o $@
$(ODIR)/common.o:$(SDIR)/common.c $(SDIR)/common.h
	$(CC) $(CFLAGS) -c $(INCS) $< -o $@
$(ODIR)/analyse.o:$(SDIR)/analyse.c $(SDIR)/common.h
	$(CC) $(CFLAGS) -c -o $@ $<

