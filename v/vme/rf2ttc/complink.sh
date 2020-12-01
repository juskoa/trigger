#!/bin/bash
g++ -g -c -Wall -I$VMEBDIR/vmeblib -I/ATLAS/tdaq_drivers -I$VMECFDIR/ttcmi rf2ttc.c
g++ rf2ttc.o -L$VMEBDIR/vmeblib/linux_c -lvmeb -lpthread -L/ATLAS/lib -lvme_rcc -lrcc_error -lio_rcc -lcmem_rcc -lDFDebug -lm -o rf2ttc
