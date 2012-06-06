include $(VMEBDIR)/com_defs.make

ODIR=linux
CC=g++
CFLAGS=-I/opt/dip/include
LDFLAGS =-lpthread
$(ODIR)/fill2file: fill2file.cpp $(odl64)/libdip.so
ifeq (,$(findstring $(ODIR),$(wildcard $(ODIR) )))
	-mkdir -p $(ODIR)
endif
	$(CC) $(CFLAGS) $< $(LDFLAGS) -L$(odl64) -ldip -ldl -o $@
#
$(ODIR)/dump_bc: dump_bc.cpp $(odl64)/libdip.so
	$(CC) $(CFLAGS) $< $(LDFLAGS) -L$(odl64) -ldip -ldl -o $@
$(ODIR)/fill2file_dbg: fill2file_dbg.cpp $(odl64)/libdip.so
	$(CC) $(CFLAGS) $< $(LDFLAGS) -L$(odl64) -ldip -ldl -o $@
clean:
	-rm -f $(ODIR)/*.o
