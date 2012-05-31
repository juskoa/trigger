#!/bin/bash
#This is to be started from trigger account on alidcsvme* machine.
# VMEWORKDIR (i.e. v/$1) for given detector has to exists
#ltu_proxy='ltu_proxy'   #DO NOT USE THIS -use start_ltuproxy.sh
ltu_proxy='ltudimserver' 
cfgfile=$VMECFDIR/CFG/ctp/DB/ttcparts.cfg
detname=$1
export VMEWORKDIR=~/v/$detname
if [ ! -d $VMEWORKDIR ] ; then
  echo "$VMEWORKDIR does not exist, exiting"
  exit
fi
cd $VMEWORKDIR
# 
if [ $# -eq 2 ] ;then
  if [ $2 = "kill" -o $2 = "start" ] ;then
    action=$2
  else
    action="error"
  fi
  else
    action="error"
fi
# find base and right hostname for this detector:
ltubase=`awk '{if($1==detname) {print $3}}' detname=$1 $cfgfile`
hn=`awk '{if($1==detname) {print $2}}' detname=$1 $cfgfile`
if [ -z "$ltubase" -o "$action" = "error" ] ;then
  cat - <<-EOF
Usage:
ltuserver.bash DETNAME start      -start server for DETNAME on this computer
ltuserver.bash DETNAME kill       -kill server for DETNAME on this computer

EOF
  echo "Known detectors:"
  cat $cfgfile
  echo ; echo "On this machine, `hostname`, these servers are running:"
  ps -C $ltu_proxy o user,pid,args
  #ps -C ltudimserver o user,pid,args
elif [ `hostname` = "$hn" ] ;then
  pid=`ps -C $ltu_proxy o user,pid,args | awk '{if($4==detname) {print $2}}' detname=$1`
  if [ "$action" = 'kill' ] ;then
    kill $pid
    echo "$1 (pid:$pid) killed"
  elif [ "$action" = 'start' ] ;then
    if [ -z $pid ] ;then
      if [ "$ltu_proxy" = 'ltudimserver' ] ;then
        #for those who are not using ltu_proxy yet
        nohup $VMECFDIR/ltu_proxy/Linux/$ltu_proxy $detname $ltubase >WORK/dimserver.log &
      else
        #not tested (should be used with -nodim first)
        nohup $VMECFDIR/ltu_proxy/Linux/$ltu_proxy $detname $ltubase >WORK/$detname.log &
      fi
      echo "$detname $ltubase server started($ltu_proxy)"
    else
      echo "$1 server is already running (pid:$pid). Server not started"
      exit
    fi
  else
    echo "action:$action ???"
  fi
else
  echo "$detname is on $hn, server not started"
fi

