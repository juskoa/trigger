#!/bin/bash
# starting udpmon server reading CTPcounters stream and sending
# the subset of these counters (in accordance with active partitions)
# to FXS
#usage: xcounters.sh
function getpid() {
pidcmd=`ps ax --columns 120 -o'%p %u %a' |grep "$1"|grep -v grep`
#13643 trigger  ./linux/ttcmidims
#echo "pidcmd:$1:$pidcmd"
declare -a dl=($pidcmd)
#echo -e "pid:${dl[0]} cmd:${dl[1]} "
#echo -e "$1 ? ${dl[2]}"
if [ "${dl[2]}" = "$1" ] ;then
  spid=${dl[0]}
  user=${dl[1]}
else
  spid=''
fi
#echo getpid: $spid $dl $pidcmd
}
function savelog() {
ds=`date +%y%m%d%H%M`   # %S -seconds (not used)
if [ -e "$1.log" ] ;then
  #echo "exists: $1"
  mv "$1.log" "$1$ds.log"
else
  echo "$1.log does not exist"
fi
}
#-------------------------------------
DNAME="udpmon"
export EXENAME="$VMECFDIR/CNTRRD/linux/udpmonitor"
logdir=~/CNTRRD/logs
hname=`hostname`
if [ "$1" = 'test' ] ;then   #-------------------------- stop
  exit 7
fi
if [ "$hname" != 'pcalicebhm05' -a "$hname" != 'alidcscom188' \
  -a "$hname" != 'pcalicebhm10' ] ;then
  echo 'can be started only from trigger@alidcscom188 or pcalicebhm05/10'
  exit 8
fi
getpid $EXENAME
#echo "spid:$spid"
if [ "$1" = 'stop' -o "$1" = 'kill' ] ;then   #-------------------------- stop
  if [ -n "$spid" ] ;then
    kill -s 9 $spid
    echo "killing: $spid/$user"
    ps
  else
    echo "$DNAME is not running"
  fi
elif [ "$1" = 'start' ] ;then    #----------------------- start
 if [ -n "$spid" ] ;then
  echo "$DNAME is running already! pid: $spid user:$user"
 else
  #no1min=''
  #if [ "$2" = 'no1min' ] ;then
  #  no1min='no1min'
  #fi
  cd $logdir
  savelog $DNAME
  cd $logdir ; cd ../
  nohup $EXENAME >$logdir/$DNAME.log &
  cat - <<-EOF 
  $DNAME daemon ($EXENAME) started. 
  log: $logdir/$DNAME.log
EOF
 fi
elif [ "$1" = 'status' ] ;then     #----------------------- status
  if [ -z "$spid" ] ;then
    echo "$DNAME server is not running"
    retc=4
  else
    echo "$DNAME server is running, pid: $spid user:$user"
    retc=0
  fi
  exit $retc
else     #-------------------------------------------------- bad param
  echo
  if [ -z "$spid" ] ;then
    echo "$DNAME server is not running"
  else
    echo "$DNAME server is running, pid: $spid user: $user"
  fi
  cat - <<-EOF
Usage:
$DNAME.sh start
$DNAME.sh stop
$DNAME.sh status
EOF
fi

