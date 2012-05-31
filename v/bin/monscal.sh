#!/bin/bash
sss=$1
. $VMECFDIR/../bin/auxfunctions
executable='monscalnow'
getpid $executable

dname="monscal"
#if [ "$sss" == 'status' ] 
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
    #mvfile WORK/gmonscal
    cd  $VMEWORKDIR/WORK
    savelog MONSCAL/monscal
    savelog MONSCAL/display
    nohup $VMECFDIR/monscal++/linux/$executable 1 > MONSCAL/monscal.log </dev/null &
    echo "$dname started"
  fi
else
  if [ -z "$spid" ] ;then
    echo "$dname is not running"
  else
    echo "$dname is running, pid: $spid user:$user"
  fi
fi



