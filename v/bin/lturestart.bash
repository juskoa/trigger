#!/bin/bash
# this script is invoked from: ltucheck.py
if [ $# -eq 0 ] ;then
  cat - <<-EOF
Usage:
lturestart DETECTOR               or lturestart all

where DETECTOR is detector name (e.g.: ssd, spd, sdd,...)
This script should be started from trigger account on alidcscom188
EOF
exit
fi
detname=$1
#readme $detname
if [ "$detname" == 'all' ] ;then
  allhns="2 3 4 5 6 7"
  echo 'killing all...'
  for hn in $allhns ;do
    rcmd $hn 'ltuproxy killall'
  for hn in $allhns ;do
    rcmd $hn 'ltuproxy active'
    echo 'starting all at $hn...'
    rcmd $hn 'ltuproxy startall' &
  echo 'status of all ltuproxies:'
  for hn in $allhns ;do
    rcmd $hn 'ltuproxy active'
else
declare -a arna=($(readme $detname))
hostn=${arna[1]}
#echo ssh -2 -f "trigger@$hostn" "ltuproxy $detname blabla" $USER >lp.out
#ssh -2 "trigger@$hostn" "ltuproxy $detname restart"
nohup ssh -2 "trigger@$hostn" "ltuproxy $detname restart" &
#echo ${arna[1]}
fi
