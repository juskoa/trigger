#!/bin/bash
# $1: start stop or status
onlyhost='alidcscom835'
hname=`hostname -s`
if [ "$hname" != "$onlyhost" -a "$hname" != "pcalicebhm10" -a "$hname" != "adls" ] ;then
  echo "This script can be started only on $onlyhost or pcalicebhm10 adls"
  exit 4
fi
sss=$1
#daemoncom.sh ctpwsgi $VMECFDIR/wsgi/ctpwsgi.py log $sss
if [ "$sss" = "status" -o "$sss" = "" ] ;then
  #ps h -C dns
  #30366 pts/0    Sl     0:04 /opt/dim/linux/dns
  #ps h -o pid -C dns
  #30366
  # 
  resp=`ps h -C dns`
  #echo $resp
  declare -a ar=($resp)
  #echo ${ar[4]}
  if [ "${ar[4]}" = "/opt/dim/linux/dns" ] ;then
    echo $resp
    exit 0
  else
    exit 8
  fi
else
  exit 4
fi
