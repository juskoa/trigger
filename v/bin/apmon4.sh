#!/bin/bash
sss=$1
. $VMECFDIR/../bin/auxfunctions
onlyserver
if [ "$?" != "0" ] ;then
  echo "not on sever"
  exit 8
fi
executable="$APMON/examples/apmon4"
execrun="$APMON/examples/.libs/lt-apmon4"
getpid $execrun
if [ "$sss" == 'stop' ] ;then   #-------------------------- stop
  if [ -n "$spid" ] ;then
    #kill -s QUIT $spid
    echo "killing: $spid/$user"
    kill $spid
    #ps
  else
    echo "$executable is not running"
  fi
elif [ "$sss" == 'start' ] ;then    #----------------------- start
  if [ -n "$spid" ] ;then
    echo "$executable already running: $spid, not started"
  else
    #mvfile WORK/gmonscal
    cd  $VMEWORKDIR/WORK
    savelog apmon4
    nohup $executable apm > apmon4.log &
    echo "$executable started"
  fi
else
  if [ -z "$spid" ] ;then
    echo "$dname is not running"
  else
    echo "$dname is running, pid: $spid user:$user"
  fi
fi

