#!/bin/bash
#--------------------------------------------------
hname=`hostname -s`
if [ "$hname" != 'alidcscom026' ] ;then
echo 'This script can be started only on trigger@alidcscom026 '
exit
fi
tail -n 20 -f /data/ClientLocalRootFs/alidcsvme001/home/alice/trigger/v/vme/WORK/dims.log

