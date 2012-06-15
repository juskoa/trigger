OPT2 = -g
ROOTCONFIG   := root-config

ROOTCFLAGS   := $(shell $(ROOTCONFIG) --cflags)
ROOTLDFLAGS  := $(shell $(ROOTCONFIG) --ldflags)
ROOTLIBS     := $(shell $(ROOTCONFIG) --libs)
ROOTGLIBS    := $(shell $(ROOTCONFIG) --glibs)

# Linux
CXX           = g++
CXXFLAGS      = $(OPT2) -Wall -fPIC
LD            = g++
LDFLAGS       = $(OPT2)

CXXFLAGS     += $(ROOTCFLAGS)
LDFLAGS      += $(ROOTLDFLAGS)
LIBS          = $(ROOTLIBS) $(SYSLIBS)
GLIBS         = $(ROOTGLIBS) $(SYSLIBS)

extract: main.o extract.o
	g++ $(LDFLAGS) main.o extract.o $(LIBS) -o extract

extract.o: extract.cc
	$(CXX) $(CXXFLAGS) -c extract.cc
main.o: main.cc
	$(CXX) $(CXXFLAGS) -c main.cc
