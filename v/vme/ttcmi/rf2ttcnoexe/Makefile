INCLUDELIB=~/tdaq-01-09-00/include
LIBLIB=~/tdaq-01-09-00/lib
CFLAGS  = -c -g -DDEBUG_LEVEL=1 -Irf2ttc -I$(INCLUDELIB)
CFLAGS2 = -pedantic -funsigned-char -Wall -Wpointer-arith -Wcast-qual -Wcast-align

LFLAGS      = -Llib -L$(LIBLIB)
LIBS_ATLICE = -latlice_glue -lvme_rcc -lio_rcc -lcmem_rcc -lgetinput -lrcc_time_stamp -lrcc_error -lDFDebug
LIBS_LHCB   = -L/usr/lib -llhcb_glue -lCAENVME -lgetinput -lrcc_time_stamp -lrcc_error -lDFDebug
LIBS_CMS    = -L/usr/lib -lcms_glue -lCAENVME -lgetinput -lrcc_time_stamp -lrcc_error -lDFDebug


all: bin/rf2ttcscope_atlice bin/rfrxscope_atlice bin/rf2ttcscope_lhcb bin/rfrxscope_lhcb bin/rf2ttcscope_cms \
     bin/rfrxscope_cms lib/librfrx.a lib/librf2ttc.a

atlice: bin/rf2ttcscope_atlice bin/rfrxscope_atlice lib/librfrx.a lib/librf2ttc.a

lhcb: bin/rf2ttcscope_lhcb bin/rfrxscope_lhcb lib/librfrx.a lib/librf2ttc.a

cms: bin/rf2ttcscope_cms bin/rfrxscope_cms lib/librfrx.a lib/librf2ttc.a


bin/rf2ttcscope_lhcb: lib/rf2ttcscope.o lib/liblhcb_glue.a lib/librf2ttc.a 
	g++ lib/rf2ttcscope.o $(LFLAGS) $(LIBS_LHCB) -lrf2ttc -o bin/rf2ttcscope_lhcb

bin/rf2ttcscope_cms: lib/rf2ttcscope.o lib/libcms_glue.a lib/librf2ttc.a 
	g++ lib/rf2ttcscope.o $(LFLAGS) $(LIBS_CMS) -lrf2ttc -o bin/rf2ttcscope_cms

bin/rf2ttcscope_atlice: lib/rf2ttcscope.o lib/libatlice_glue.a lib/librf2ttc.a
	g++ lib/rf2ttcscope.o $(LFLAGS) $(LIBS_ATLICE) -lrf2ttc -o bin/rf2ttcscope_atlice




bin/rfrxscope_cms: lib/rfrxscope.o lib/libcms_glue.a lib/librfrx.a
	g++ lib/rfrxscope.o $(LFLAGS) $(LIBS_CMS) -lrfrx -o bin/rfrxscope_cms

bin/rfrxscope_lhcb: lib/rfrxscope.o lib/liblhcb_glue.a lib/librfrx.a
	g++ lib/rfrxscope.o $(LFLAGS) $(LIBS_LHCB) -lrfrx -o bin/rfrxscope_lhcb

bin/rfrxscope_atlice: lib/rfrxscope.o lib/libatlice_glue.a lib/librfrx.a
	g++ lib/rfrxscope.o $(LFLAGS) $(LIBS_ATLICE) -lrfrx -o bin/rfrxscope_atlice


	
	
lib/rf2ttcscope.o: src/test/rf2ttcscope.cpp
	g++ $(CFLAGS) $(CFLAGS2) src/test/rf2ttcscope.cpp -o lib/rf2ttcscope.o	
	
lib/rfrxscope.o: src/test/rfrxscope.cpp
	g++ $(CFLAGS) $(CFLAGS2) src/test/rfrxscope.cpp -o lib/rfrxscope.o
	
	
	

lib/libatlice_glue.a: lib/atlice_glue.o
	ar ru lib/libatlice_glue.a lib/atlice_glue.o
	ranlib lib/libatlice_glue.a

lib/atlice_glue.o: src/atlice_glue.cpp
	g++ $(CFLAGS) $(CFLAGS2) src/atlice_glue.cpp -o lib/atlice_glue.o
	


lib/liblhcb_glue.a: lib/lhcb_glue.o
	ar ru lib/liblhcb_glue.a lib/lhcb_glue.o
	ranlib lib/liblhcb_glue.a
	
lib/lhcb_glue.o: src/lhcbcms_glue.cpp
	g++ $(CFLAGS) $(CFLAGS2) -DLINUX -DLHCB src/lhcbcms_glue.cpp -o lib/lhcb_glue.o



lib/libcms_glue.a: lib/cms_glue.o
	ar ru lib/libcms_glue.a lib/cms_glue.o
	ranlib lib/libcms_glue.a
	
lib/cms_glue.o: src/lhcbcms_glue.cpp
	g++ $(CFLAGS) $(CFLAGS2) -DLINUX -DCMS src/lhcbcms_glue.cpp -o lib/cms_glue.o


	
lib/librfrx.a: lib/rfrx.o lib/common.o
	ar ru lib/librfrx.a lib/common.o lib/rfrx.o
	ranlib lib/librfrx.a
	
lib/rfrx.o: src/rfrxlib.cpp
	g++ $(CFLAGS) $(CFLAGS2) src/rfrxlib.cpp -o lib/rfrx.o	



lib/librf2ttc.a: lib/rf2ttc.o lib/common.o
	ar ru lib/librf2ttc.a lib/common.o lib/rf2ttc.o
	ranlib lib/librf2ttc.a
	
lib/rf2ttc.o: src/rf2ttclib.cpp
	g++ $(CFLAGS) $(CFLAGS2) src/rf2ttclib.cpp -o lib/rf2ttc.o	


	
lib/common.o: src/common_lib.cpp
	g++ $(CFLAGS) $(CFLAGS2) src/common_lib.cpp -o lib/common.o	

	
	
clean:
	rm -rf lib/*
	rm -rf bin/rf*	
