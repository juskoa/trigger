
ObjSuf        = o
SrcSuf        = cc
CXX           = g++
CXXFLAGS      = -g -Wall -fPIC -I$(VMEBDIR)/vmeblib

libctp++.a: time.o convert.o CTPRDIRD.o parse.o
	rm -f libctp++.a
	ar -q libctp++.a *.o
	rm *.o

time.o: time.cc
convert.o: convert.cc
CTPRDIRD.o: CTPRDIRD.cc
parse.o: parse.cc

.$(SrcSuf).$(ObjSuf):
	$(CXX) $(CXXFLAGS) -c $<

