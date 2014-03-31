#FILE          = ttcit
FILE          = ctpro
#FILE          = ltu
#FILE          = l0l1l2
#FILE          = generL1

CXX           = g++
CXXFLAGS      = -g -Wall -fPIC -Ilib/ -I$(VMEBDIR)/vmeblib -Isrc
LDFLAGS       = -g -L/lib/modules/daq -lvme_rcc -lrcc_error -lio_rcc -lcmem_rcc
LIBS          = -lg++ -lm -ldl -rdynamic
GLIBS         = -L/usr/X11R6/lib -lX11 -lm -ldl -rdynamic
VMERCCCXX    =  -g -DVMERCC -I$(VMEBDIR)/vmeblib -I/usr/local/include

FILES = x/BOARDBASIC.cc x/BOARD.cc x/SSMTOOLs.cc x/LTUBOARD.cc x/CTP.cc x/CONNECT.cc \
x/L0BOARD.cc x/L1BOARD.cc x/L2BOARD.cc x/INTBOARD.cc x/DETECTOR.cc \
x/TTCITBOARD.cc 

SRCS=$(subst x/,src/,$(FILES))
HFILES = $(SRCS:.cc:=.h)
OBJ = $(subst x/,obj/,$(FILES))
OBJFILES = $(OBJ:.cc=.o) obj/vmewrap.o

$(FILE).e: obj/$(FILE).o $(OBJFILES)
	g++ obj/$(FILE).o $(OBJFILES) $(LDFLAGS) $(GLIBS) lib/libctp++.a -o $(FILE).e

obj/$(FILE).o:  macros/$(FILE).cc 
	$(CXX) $(CXXFLAGS) -c macros/$(FILE).cc -o obj/$(FILE).o
obj/vmewrap.o: $(VMEBDIR)/vmeblib/vmewrap.c
	g++ -c $(VMERCCCXX) $(VMEBDIR)/vmeblib/vmewrap.c -o obj/vmewrap.o

obj/%.o: src/%.cc src/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm obj/*.o
