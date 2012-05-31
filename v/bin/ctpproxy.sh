#!/bin/bash
#export DIM_DNS_NODE=pcald30
function mvfile() {
# $1: relative path of the log file NO SUFFIX, i.e.: WORK/ctpproxy
# operation: WORK/ctpproxy.log -> WORK/ctpproxyYYMMDDhhmm.log
ds=`date +%y%m%d%H%M`   # %S -seconds
if [ -e "$1.log" ] ;then
  #echo "exists: $1"
  mv "$1.log" "$1$ds.log"
else
  echo "$1.log does not exist"
fi
}
function startproxy() {
    echo "Starting ctp_proxy -> see $VMEWORKDIR/WORK/ctp_proxy.log"
    cd $VMEWORKDIR
    mvfile WORK/ctp_proxy
    #nohup $VMECFDIR/ctp_proxy/linux/ctp_proxy TRIGGER::CTP >WORK/ctp_proxy.log &
    #</dev/null >foglight.out 2>&1
    # if following not working, try /etc/security/limits.conf
    #*  hard  core unlimited
    ulimit -c unlimited
    $VMECFDIR/ctp_proxy/linux/ctp_proxy TRIGGER::CTP DAQRO </dev/null >WORK/ctp_proxy.log 2>&1 &
    #echo "ctp_proxy rc:$? is 0 always (in case ctp_proxy does not exist)"
    pid=`ps -C ctp_proxy o user,pid,args | awk '{if($4==detname) {print $2}}' detname=$proxyname`
    echo "pid:$pid"
}
proxyname="TRIGGER::CTP"
pid=`ps -C ctp_proxy o user,pid,args | awk '{if($4==detname) {print $2}}' detname=$proxyname`
if [ -z $pid ] ;then
  echo "TRIGGER::CTP not running"
  if [ -z $1 ] ;then
    echo "start, status or starttest expected"
  elif [ "$1" = "status" ] ;then
    exit 8
  elif [ "$1" = "start" ] ;then
    startproxy
  elif [ "$1" = "starttest" ] ;then
    cd $VMEWORKDIR
#set args NODAQLOGBOOK NODAQRO
    cat - <<-EOF >.gdbinit
set args NODAQLOGBOOK NODAQRO
EOF
    gdb $VMECFDIR/ctp_proxy/linux/test
  elif [ $# -gt 0 ] ;then
    echo "Bad parameter(s) (start or starttest expected)"
  fi
else
  echo "TRIGGER::CTP running. pid:$pid"
  if [ "$1" = "stop" ] ;then
    echo "stopping $pid... ctpproxy will wait for all active partitions to stop before exiting)" ; kill -s SIGQUIT $pid
  elif [ "$1" = "status" ] ;then
    exit 0
  elif [ "$1" = "kill" ] ;then
    echo "killing $pid" ; kill -s SIGKILL $pid
  elif [ "$1" = "restart" ] ;then
    echo "restarting $pid" ; kill -s SIGKILL $pid
    startproxy 
  elif [ $# -gt 0 -a "$1" != "status" ] ;then
    cat - <<-EOF
Bad parameter:$1. One of the following expected:
status  -check if ctpproxy is started
stop    -ask ctpproxy to stop. ctpproxy will wait for the stop of all 
         active partitions (check with ECS operator). 
kill    -stop ctpproxy immediately. This option should not be used!
restart = kill + start (i.e. preferred way is: 'ctpproxy stop ; ctpproxy start')
EOF
  fi
fi
