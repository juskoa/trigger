FILE          = test8

CXX           = g++
CXXFLAGS      = -g -Wall -fPIC -Ilib/ -I$(VMEBDIR)/vmeblib -I$(VMEBDIR)/../vme/ctp++/src
#LDFLAGS       = -g -L/lib/modules/daq -lvme_rcc -lrcc_error -lio_rcc -lcmem_rcc
LDFLAGS       = -g -L/lib/modules/daq
LIBS          = -lg++ -lm -ldl -rdynamic
GLIBS         = -L/usr/X11R6/lib -lX11 -lm -ldl -rdynamic
VMERCCCXX    =  -g -DVMERCC -I$(VMEBDIR)/vmeblib -I/home/dl/root/usr/local/include

FILES =   
HFILES = $(SRCS:.cc:=.h)
#OBJ = $(subst x/,obj/,$(FILES))
#OBJFILES = $(OBJ:.cc=.o)

$(FILE).e: $(FILE).o $(OBJFILES)
	g++ $(FILE).o $(OBJFILES) $(LDFLAGS) $(GLIBS) ../lib/libctp++.a -o $(FILE).e

obj/%.o: %.cc %.h
	$(CXX) $(CXXFLAGS) -c $< -o $@
