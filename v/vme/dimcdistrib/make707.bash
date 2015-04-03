#!/bin/bash
# this script should be started from 707
cd $VMECFDIR/dimcdistrib
mkdir -p ~/v/vme/dimcdistrib/linux
g++ -Wall -g -I/opt/dim/dim -DCPLUSPLUS -c -o ~/v/vme/dimcdistrib/linux/dimccounters.o dimccounters.c
g++ -lpthread ~/v/vme/dimcdistrib/linux/dimccounters.o  -o ~/v/vme/dimcdistrib/linux/dimccounters -L/opt/dim/linux -ldim

