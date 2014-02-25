#!/bin/bash
#[alidcscom188] /data/dl/root/usr/local/trigger/v/vme/fanio/dim > ./make707.bash 
# link vmeblib:
#cd $VMEBDIR/vmeblib
#
cd $VMECFDIR/fanio/dim
g++ -Wall -g -DVMERCC -DCPLUSPLUS -I/usr/local/include -I/opt/dim/dim -I$VMEBDIR/vmeblib/linux_s/.. -lpthread client.c -L/opt/dim/linux -ldim -L$VMEBDIR/vmeblib/linux_s -lvmeb -o linux_c/client
#g++ -Wall -g -DVMERCC -DCPLUSPLUS -I/opt/dim/dim -I$VMEBDIR/vmeblib client.c -lpthread -L/opt/dim/linux -ldim -L ~/v/vmeb/vmeblib/linux_s -lvmeb -o ~/v/linux64/client
