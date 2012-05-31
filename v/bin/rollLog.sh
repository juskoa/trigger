#!/bin/bash
#--------------------------------------------------
hname=`hostname`
if [ "$hname" != 'alidcscom026' ] ;then
echo 'This script can be started only on trigger@alidcscom026 '
exit
fi
#export PROMPT_COMMAND='echo -ne "\033]0;${USER}" CTPPROXY LOG"\007"'
#export ABC='aaa'
#echo $ABC

if [ $# -eq 0 ] ;then
  log='help'
elif [ $# -eq 1 ] ;then
  log=$1
fi

if [ $log = "ctpproxy" ] ;then
  logdir=/data/ClientLocalRootFs/alidcsvme001/home/alice/trigger/v/vme/WORK
  tail -n 20 -f  $logdir/ctp_proxy.log
fi

if [ $log = "dim"  ] ;then
  logdir=/data/ClientLocalRootFs/alidcsvme001/home/alice/trigger/v/vme/WORK
  tail -n 20 -f $logdir/dims.log
fi

if [ $log = "pydim"  ] ;then
  tail -n 20 -f ~trigger/CNTRRD/logs/pydimserver.log
fi

if [ $log = 'help' ] ;then
cat - <<-EOF
rollLog usage:
   rollLog.sh      - print help
   rollLog.sh help - print help
   rollLog.sh ctpproxy  -open ctpproxy log
   rollLog.sh dim       -open dim log
   rollLog.sh pydim     -open pydim log
   To stop - press CTRL-C
EOF
exit
fi

