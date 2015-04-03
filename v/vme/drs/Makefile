FILE          = l1inmon

CXX           = g++
CXXFLAGS      = -g -Wall -fPIC -Isrc
LDFLAGS       = -g
LIBS          = -lg++ -lm -ldl -rdynamic
GLIBS         = -L/usr/X11R6/lib -lX11 -lm -ldl -rdynamic

all: $(FILE).e

%.e: drs_exam_file.o
	g++ drs_exam_file.o $(LDFLAGS) $(GLIBS) -o drs.e

obj/%.o: drs_exam_file.cpp tools.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm *.o
