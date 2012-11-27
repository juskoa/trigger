#!/bin/bash
# common script for start/stop/status of a daemon running
# on one of trigger machines sharing $VMECFDIR.
# Currently used with diprfrx, gcalib.
# environment: VMECFDIR VMEWORKDIR
#usage: daemoncom.sh daemon_name daemon_path log sss
#  where:
#  daemon_name.sh is script calling this script
#  daemon_path is executable relative to $VMECFDIR
#  log is: 
#    nolog -send log to /dev/null
#    log   -send log to $VMEWORKDIR/$dname.log
#  sss is one of: start stop status
#
#. $CCRFS/usr/local/trigger/bin/auxfunctions
. $VMECFDIR/../bin/auxfunctions
#
dname=$1   # daemon name
dpath=$2   # executable
dlog=$3    # nolog: >/dev/null other: $VMEWORKDIR/WORK/$dname.log
sss=$4     # start/stop/status
onlyonhost=$5
onlyhost $onlyonhost
if [ $? -ne 0 ] ;then
  return
fi
getpid "$dpath"
#echo "daemoncom.sh: getpidspid $spid rc:$?"
if [ "$sss" = 'stop' -o "$sss" = 'kill' ] ;then   #-------------------------- stop
  if [ -n "$spid" ] ;then
    #kill -s QUIT $spid
    kill $spid
    echo "killing: $spid/$user"
    #ps
  else
    echo "$dname dimserver is not running"
  fi
elif [ "$sss" = 'start' ] ;then    #----------------------- start
 if [ -n "$spid" ] ;then
  echo "$dname dim server is running already! pid: $spid user:$user"
 else
  #no1min=''
  #if [ "$2" = 'no1min' ] ;then
  #  no1min='no1min'
  #fi
  logdir=$VMEWORKDIR/WORK
  cd $logdir
  savelog $dname
  cd $VMEWORKDIR
  if [ "$dlog" = 'nolog' ] ;then
    nohup $VMECFDIR/$dpath 2>&1>/dev/null &
    lg='/dev/null'
  else
    nohup $VMECFDIR/$dpath >$logdir/$dname.log &
    lg="$logdir/$dname.log"
  fi
  cat - <<-EOF 
  $dname server ($VMECFDIR/$dpath) started. 
  log: $lg
EOF
 fi
elif [ "$sss" = 'status' ] ;then     #----------------------- status
  if [ -z "$spid" ] ;then
    echo "$dname server is not running"
    retc=4
  else
    echo "$dname server is running, pid: $spid user:$user"
    retc=0
  fi
  exit $retc
else     #-------------------------------------------------- bad param
  echo "bad parameter:$sss (status, start or stop expected)"
  if [ -z "$spid" ] ;then
    echo "$dname server is not running"
  else
    echo "$dname server is running, pid: $spid user: $user"
  fi
  cat - <<-EOF
Usage:
$dname.sh [ start | stop | status ]
EOF
fi

