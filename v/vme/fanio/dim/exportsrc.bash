#!/bin/bash
mkdir -p ~/faninsc
cp server.c client.c ../fanio.h $VMEBDIR/vmeblib/vmewrap.h \
 $VMEBDIR/vmeblib/linux_c/libvmeb.a exportsrc.bash ~/faninsc/
cat - <<-EOF
server.c
../fanio.h   ->   fanio.h

client.c
//#include "lexan.h"   ->
#include <ctype.h>
uncomment gethexdec+UPPER

Compile/link server+client:
g++ -Wall -Wno-write-strings -g -DCPLUSPLUS -I/usr/local/include -I/opt/dim/dim -lpthread server.c -L/opt/dim/linux -ldim -L. -lvmeb -L/lib/modules/daq -lvme_rcc -lrcc_error -lio_rcc -lcmem_rcc -lDFDebug -o server.exe
g++ -Wall -g -DCPLUSPLUS -I/opt/dim/dim -lpthread client.c -L/opt/dim/linux -ldim -L. -lvmeb -o client.exe


EOF
