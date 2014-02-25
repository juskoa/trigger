
ObjSuf        = o
SrcSuf        = cc
CXX           = g++
CXXFLAGS      = -g -Wall -fPIC -I$(VMEBDIR)/vmeblib

libctp++.a: time.o convert.o CTPRDIRD.o
	ar -q libctp++.a *.o
	rm *.o

time.o: time.cc
convert.o: convert.cc
CTPRDIRD.o: CTPRDIRD.cc

.$(SrcSuf).$(ObjSuf):
	$(CXX) $(CXXFLAGS) -c $<

