CC = $(VMEGCC)
CFLAGS=-Wall -g
EXEDIR = linux
#VPATH := ../

$(EXEDIR)/ssmanv.exe:ssmanv.c list.c
	$(CC) $(CFLAGS) ssmanv.c -o ssmanv.exe
clean:
	-rm -f *.o *.exe

