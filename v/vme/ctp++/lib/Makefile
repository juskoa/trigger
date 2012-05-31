
ObjSuf        = o
SrcSuf        = cc
CXX           = g++
CXXFLAGS      = -g -Wall -fPIC

libctp++.a: time.o convert.o
	rm *.a
	ar -q libctp++.a *.o
	rm *.o

time.o: time.cc
convert.o: convert.cc

.$(SrcSuf).$(ObjSuf):
	$(CXX) $(CXXFLAGS) -c $<

