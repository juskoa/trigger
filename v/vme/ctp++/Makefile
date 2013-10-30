FILE          = generL1
ObjSuf        = o
SrcSuf        = cc
HdrSuf        = h
ExeSuf        =
DllSuf        = so
EVENTLIB      = $(EVENTO)
OutPutOpt     = -o

ROOTLIBS      = -L$(ROOTSYS)/lib -lNew -lCint \
                -lGraf -lGraf3d -lHist -lHtml -lMatrix -lMinuit \
                -lPostscript -lProof -lTree 
ROOTGLIBS     = -lGpad -lGui -lGX11 -lX3d

# Linux
CXX           = g++
CXXFLAGS      = -g -Wall -fPIC -Ilib/ -I$(VMEBDIR)/vmeblib
LD            = g++
LDFLAGS       = -g -L/lib/modules/daq -lvme_rcc -lrcc_error -lio_rcc -lcmem_rcc
SOFLAGS       = -Wl,-soname,libEvent.so -shared
LIBS          = $(ROOTLIBS) -lg++ -lm -ldl -rdynamic
# GLIBS with root
#GLIBS         = $(ROOTLIBS) $(ROOTGLIBS) -L/usr/X11R6/lib \
                -lXpm -lX11 -lm -ldl -rdynamic
GLIBS         = -L/usr/X11R6/lib -lX11 -lm -ldl -rdynamic
VMERCCCXX    =  -g -DVMERCC -I$(VMEBDIR)/vmeblib -I/home/dl/root/usr/local/include

.SUFFIXES: .$(cc)

##$(LD) $(LDFLAGS) $(HWORLDO) $(GLIBS) $(OutPutOpt) $(HWORLD)
OBJFILES = vmewrap.o BOARDBASIC.o BOARD.o SSMTOOLs.o DETECTOR.o CTP.o CONNECT.o 

$(FILE).e: $(FILE).o $(OBJFILES)
	g++ $(FILE).o $(OBJFILES) $(LDFLAGS) $(GLIBS) lib/libctp++.a -o $(FILE).e
checkbckplane: checkbckplane.o CONNECT.o $(OBJFILES)
	g++ checkbckplane.o CONNECT.o $(OBJFILES) $(LDFLAGS) $(GLIBS) lib/libctp++.a -o checkbckplane
test: test.o
	g++ test.o $(LDFLAGS) $(GLIBS) lib/libctp++.a -o test


$(FILE).o:  $(FILE).cc 
test.o: test.cc
checkbckplane.o: checkbckplane.cc
BOARDBASIC.o: BOARDBASIC.cc BOARDBASIC.h
DETECTOR.o: DETECTOR.cc DETECTOR.h
BOARD.o:    BOARD.cc BOARD.h
DETECTOR.o:	DETECTOR.cc DETECTOR.h
SSMTOOLs.o:      SSMTOOLs.cc SSMTOOLs.h
CTP.o:	    CTP.cc CTP.h	
CONNECT.o:  CONNECT.cc CONNECT.h
vmewrap.o: $(VMEBDIR)/vmeblib/vmewrap.c
	g++ -c $(VMERCCCXX) $(VMEBDIR)/vmeblib/vmewrap.c

.$(SrcSuf).$(ObjSuf):
	$(CXX) $(CXXFLAGS) -c $<
