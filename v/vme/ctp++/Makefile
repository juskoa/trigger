FILE          = ltustart

CXX           = g++
CXXFLAGS      = -g -Wall -fPIC -Ilib/ -I$(VMEBDIR)/vmeblib -Isrc
LDFLAGS       = -g -L/lib/modules/daq -lvme_rcc -lrcc_error -lio_rcc -lcmem_rcc
LIBS          = -lg++ -lm -ldl -rdynamic
GLIBS         = -L/usr/X11R6/lib -lX11 -lm -ldl -rdynamic
VMERCCCXX    =  -g -DVMERCC -I$(VMEBDIR)/vmeblib -I/usr/local/include

FILES = x/BOARDBASIC.cc x/BOARD.cc x/SSMTOOLs.cc x/LTUBOARD.cc x/CTP.cc x/CONNECT.cc \
x/L0BOARD.cc x/L0BOARD1.cc x/L0BOARD2.cc \
x/L1BOARD.cc x/L2BOARD.cc x/INTBOARD.cc x/DETECTOR.cc \
x/FOBOARD.cc x/TTCITBOARD.cc x/BUSYBOARD.cc x/ssmrecord.cc

SRCS=$(subst x/,src/,$(FILES))
OBJ = $(subst x/,obj/,$(FILES))
OBJFILES = $(OBJ:.cc=.o) obj/vmewrap.o
HFILES =   $(SRCS:.cc=.h)
.SECONDARY: $(OBJFILES)

all: readCTPcounters.e readBCstatus.e ttcit.e ctpttcit.e ctpstart.e startttc.e readttc.e ltu.e fo.e ttcittest.e l1inmon.e ltuttcit.e ttcit1ssm.e
# debugging
#all: $(FILE).e
#	@echo SRCS    : $(SRCS)
#	@echo HFILES  : $(HFILES)
#	@echo OBJFILES: $(OBJFILES)

%.e: obj/%.o $(OBJFILES)
	g++ $< $(OBJFILES) $(LDFLAGS) $(GLIBS) lib/libctp++.a -o $@

obj/%.o:  macros/%.C
	$(CXX) $(CXXFLAGS) -c $< -o $@
obj/vmewrap.o: $(VMEBDIR)/vmeblib/vmewrap.c
	g++ -c $(VMERCCCXX) $(VMEBDIR)/vmeblib/vmewrap.c -o obj/vmewrap.o

obj/%.o: src/%.cc src/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm obj/*.o
