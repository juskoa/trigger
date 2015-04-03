#!/bin/bash
# $1: start stop or status
onlyhost='alidcsvme001'
hname=`hostname -s`
if [ "$hname" != "$onlyhost" -a "$hname" != "altri1" ] ;then
  echo "This script can be started only on $onlyhost or altri1"
  exit
fi
sss=$1
#pgrep -l gcalib
# must be last (to pass retcode)
daemoncom.sh gcalib ctp_proxy/linux/gcalib.exe log $sss
