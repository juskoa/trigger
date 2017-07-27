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
  resp=`ssh  trigger@alidcscom521 ls -ld /mnt/alidcsfs001/Scratch_data`
  #echo $resp
  declare -a ar=($resp)
  #echo ${ar[0]}
  if [ "${ar[0]}" = "drwxrwxrwx" -a "${ar[2]}" = "root" ] ;then
    echo $resp
    exit 0
  else
    exit 8
  fi
else
  exit 4
fi
