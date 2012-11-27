#!/bin/bash
# $1: start stop or status
onlyhost='alidcscom188'
hname=`hostname`
if [ "$hname" != "$onlyhost" -a "$hname" != "pcalicebhm10" ] ;then
  echo "This script can be started only on $onlyhost or pcalicebhm10"
  exit
fi
sss=$1
#pgrep -l ctpwsgi bavi
# must be last (to pass retcode)
daemoncom.sh ctpwsgi wsgi/ctpwsgi.py log $sss
