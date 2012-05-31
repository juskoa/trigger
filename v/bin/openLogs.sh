#!/bin/bash
#--------------------------------------------------
hname=`hostname`
if [ "$hname" != 'alidcscom026' ] ;then
echo 'This script can be started only on trigger@alidcscom026 '
exit
fi

if [ $# -eq 0 ] ;then
  log='help'
elif [ $# -eq 1 ] ;then
  log=$1
fi

if [ $log = "ctpproxy" -o "$log" = "all" ] ;then
  logdir=/data/ClientLocalRootFs/alidcsvme001/home/alice/trigger/v/vme/WORK
  gview $logdir/ctp_proxy.log
fi

if [ $log = "dim" -o "$log" = "all" ] ;then
  logdir=/data/ClientLocalRootFs/alidcsvme001/home/alice/trigger/v/vme/WORK
  gview $logdir/dims.log
fi

if [ $log = "pydim" -o "$log" = "all" ] ;then
  gview  ~trigger/CNTRRD/logs/pydimserver.log
fi

if [ $log = 'help' ] ;then
cat - <<-EOF
openLogs usage:
   openLogs.sh      - print help
   openLogs.sh help - print help
   openLogs.sh all  - open ctpproxy, dim and pydim logs
   openLogs.sh ctpproxy  -open ctpproxy log
   openLogs.sh dim       -open dim log
   openLogs.sh pydim     -open pydim log
EOF
exit
fi

