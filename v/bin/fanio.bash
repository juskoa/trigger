#!/bin/bash
#if [ -z "$VMECFDIR" ] ;then
#  . /usr/local/trigger/bin/vmebse.bash
#fi
cd $VMEWORKDIR
#vmedirs
nohup $VMECFDIR/fanio/dim/linux/server >WORK/server.log &

