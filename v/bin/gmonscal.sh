#!/bin/bash
#
#sss=$1
#VMECFDIR=/home/alice/trigger/rl/monscal_root2
#VMEWORKDIR=/home/alice/trigger/rl/monscal_root2
#daemoncom.sh gmonscal gmonscal/linux/gmonscal log $sss alidcscom027
sss=$1
. $VMECFDIR/../bin/auxfunctions
onlyhost alidcscom707
executable='linux/gmonscal'
getpid $executable
#echo spid:$spid
dname="gmonscal"
if [ "$sss" == 'stop' ] ;then   #-------------------------- stop
  if [ -n "$spid" ] ;then
    #kill -s QUIT $spid
    echo "killing: $spid/$user"
    kill $spid
    #ps
  else
    echo "$dname is not running"
  fi
elif [ "$sss" == 'start' ] ;then    #----------------------- start
  if [ -n "$spid" ] ;then
    echo "$dname already running: $spid, not started"
  else
    cd /home/alice/trigger/monscal_root/
    #mvfile WORK/gmonscal
    savelog WORK/gmonscal
    nohup $executable > WORK/gmonscal.log </dev/null &
    echo "$dname started"
  fi
else
  if [ -z "$spid" ] ;then
    echo "$dname is not running"
  else
    echo "$dname is running, pid: $spid user:$user"
  fi
fi


