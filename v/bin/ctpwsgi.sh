#!/bin/bash
# $1: start stop or status
onlyhost='alidcscom835'
hname=`hostname -s`
if [ "$hname" != "$onlyhost" -a "$hname" != "pcalicebhm10" -a "$hname" != "adls" ] ;then
  echo "This script can be started only on $onlyhost or pcalicebhm10 adls"
  exit
fi
sss=$1
#pgrep -l ctpwsgi bavi
# must be last (to pass retcode)
daemoncom.sh ctpwsgi $VMECFDIR/wsgi/ctpwsgi.py log $sss
