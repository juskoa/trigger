#!/bin/bash
# starting TTCMI server controlling the ALICE clock
#usage: ttcmidims.sh
. $CCRFS/usr/local/trigger/bin/auxfunctions
#
hname=`hostname`
if [ "$hname" != 'alidcsvme017' -a "$hname" != 'altri1' ] ;then
  echo 'This script can be started only on alidcsvme017 (or altri1 for test)'
  exit
fi
getpid 'linux/ttcmidims'
if [ "$1" == 'stop' ] ;then   #-------------------------- stop
  if [ -n "$spid" ] ;then
    kill -s QUIT $spid
    echo "killing: $spid/$user"
    ps
  else
    echo "ttcmidim server is not running"
  fi
elif [ "$1" == 'start' ] ;then    #----------------------- start
 if [ -n "$spid" ] ;then
  echo "ttcmidim server is running already! pid: $spid user:$user"
 else
  #no1min=''
  #if [ "$2" == 'no1min' ] ;then
  #  no1min='no1min'
  #fi
  logdir=$VMEWORKDIR/WORK
  cd $logdir
  savelog ttcmidims
  cd $VMEWORKDIR
  nohup $VMECFDIR/ttcmidaemons/linux/ttcmidims >$logdir/ttcmidims.log &
  #nohup linux/ttcmidims $no1min >$logdir/ttcmidims.log &
  cat - <<-EOF 
  ttcmidim server ($VMECFDIR/ttcmidaemons/linux/ttcmidims) started. 
  log: $VMEWORKDIR/WORK/ttcmidims.log"
EOF
fi
elif [ "$1" == 'status' ] ;then     #----------------------- status
  if [ -z "$spid" ] ;then
    echo "ttcmidim server is not running"
  else
    echo "ttcmidim server is running, pid: $spid user:$user"
  fi
  exit
else     #-------------------------------------------------- bad param
  echo
  if [ -z "$spid" ] ;then
    echo "ttcmidim server is not running"
  else
    echo "ttcmidim server is running, pid: $spid user: $user"
  fi
  cat - <<-EOF
Usage:
ttcmidims.sh start
ttcmidims.sh stop
ttcmidims.sh status
EOF
fi

