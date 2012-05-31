#!/bin/bash
# starting TTCMI server controlling the ALICE clock
#usage: ttcmidims.sh
. $CCRFS/usr/local/trigger/bin/auxfunctions
#
hname=`hostname`
if [ "$hname" != 'alidcsvme017' -a "$hname" != 'pcalicebhm05' ] ;then
  echo 'This script can be started only on alidcsvme017 (or pcalicebhm05 for test)'
  exit
fi
getpid 'linux/ttcmidims'
if [ "$1" == 'stop' ] ;then   #-------------------------- stop
  if [ -n "$spid" ] ;then
    kill -s QUIT $spid
    echo "killing: $spid/$user"
    ps
  else
    echo "TTCMI dimserver is not running"
  fi
elif [ "$1" == 'start' ] ;then    #----------------------- start
 if [ -n "$spid" ] ;then
  echo "TTCMI dim server is running already! pid: $spid user:$user"
 else
  #no1min=''
  #if [ "$2" == 'no1min' ] ;then
  #  no1min='no1min'
  #fi
  logdir=$VMEWORKDIR/WORK
  cd $logdir
  savelog ttcmidims
  cd $VMECFDIR/ttcmidaemons
  nohup linux/ttcmidims >$logdir/ttcmidims.log &
  #nohup linux/ttcmidims $no1min >$logdir/ttcmidims.log &
  cat - <<-EOF 
  TTCMI dim server ($VMECFDIR/ttcmidaemons/linux/ttcmidims) started. 
  log: $VMEWORKDIR/WORK/ttcmidims.log"
EOF
fi
elif [ "$1" == 'status' ] ;then     #----------------------- status
  if [ -z "$spid" ] ;then
    echo "TTCMI server is not running"
  else
    echo "TTCMI server is running, pid: $spid user:$user"
  fi
  exit
else     #-------------------------------------------------- bad param
  echo
  if [ -z "$spid" ] ;then
    echo "TTCMI server is not running"
  else
    echo "TTCMI server is running, pid: $spid user: $user"
  fi
  cat - <<-EOF
Usage:
ttcmidims.sh start
ttcmidims.sh stop
ttcmidims.sh status
EOF
fi

