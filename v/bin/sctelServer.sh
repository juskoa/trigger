#!/bin/bash
cd $VMECFDIR/ttcmidaemons
if [ "$1" = "start" ] ;then
echo "starting..."
nohup ./sctelServer.py >$VMEWORKDIR/WORK/sctelServer.log &
fi
echo "ps --columns 130 aux|grep sctelServer.py..."
ps --columns 130 aux |grep sctelServer.py
exit
# $1: start stop or status
onlyhost='alidcscom188'
hname=`hostname`
if [ "$hname" != "$onlyhost" ] ;then
  echo "This script can be started only on $onlyhost "
  exit
fi
sss=$1
#pgrep -l gcalib
# must be last (to pass retcode)
export PYTHONPATH="$PYTHONPATH:$VEMCFDIR/ttcmidaemons"
daemoncom.sh sctelServer ttcmidaemons/sctelServer.py log $sss
