CC = $(VMEGCC)
#CFLAGS=-Wall -g
CFLAGS=-Wall -g -Wno-write-strings
HW := $(shell uname -i)
ifeq ($(HW), x86_64)
EXEDIR = lin64
else
EXEDIR = linux
endif
#VPATH := ../
allEXES := $(EXEDIR)/ $(EXEDIR)/ssmanv.exe $(EXEDIR)/ssmanls1.exe
all: $(allEXES)
$(EXEDIR)/:
	mkdir -p $(EXEDIR)
$(EXEDIR)/ssmanv.exe:ssmanv.c list.c
	$(CC) $(CFLAGS) ssmanv.c -o $@
$(EXEDIR)/ssmanls1.exe:ssmanls1.c
	$(CC) $(CFLAGS) ssmanls1.c -o $@
clean:
	-rm -f *.o *.exe

