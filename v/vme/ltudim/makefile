# test makefile
include makefile_common
SDIR= .
LIBS = -L$(DIMDIR)/linux -ldim $(EXTRALIBS)

ALL: $(ODIR)/ $(ODIR)/ltuclient $(ODIR)/ltucmd $(ODIR)/cmd1

$(ODIR)/:
	mkdir -p $(ODIR)
$(ODIR)/ltuclient:      $(SDIR)/ltuclient.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(SDIR)/ltuclient.c $(LIBS) -o $@

$(ODIR)/ltucmd: $(SDIR)/ltucmd.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(SDIR)/ltucmd.c $(LIBS) -o $@
$(ODIR)/cmd1: $(SDIR)/cmd1.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(SDIR)/cmd1.c $(LIBS) -o $@
clean:
	-rm -f $(ODIR)/ltuclient $(ODIR)/ltucmd $(ODIR)/cmd1

