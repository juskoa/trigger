#!/bin/bash
# starting xcounters server reading CTPcounters stream and sending
# the subset of these counters (in accordance with active partitions)
# to FXS
#usage: xcounters.sh
function getpid() {
pidcmd=`ps ax -o'%p %u %a' |grep './xcountersdaq' |grep -v grep`
#13643 trigger  ./linux/ttcmidims
declare -a dl=($pidcmd)
#echo -e "pid:${dl[0]} cmd:${dl[1]} "
if [ "${dl[2]}" == './xcountersdaq' ] ;then
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
hname=`hostname`
if [ "$hname" != 'xxxxxxxxxxxx' -a "$hname" != 'alidcscom188' ] ;then
  echo 'This script can be started only from tri@alidcscom188'
  exit
fi
getpid
if [ "$1" == 'stop' ] ;then   #-------------------------- stop
  if [ -n "$spid" ] ;then
    kill -s SIGKILL $spid
    echo "killing: $spid/$user"
    ps
  else
    echo "xcounters is not running"
  fi
elif [ "$1" == 'start' ] ;then    #----------------------- start
 if [ -n "$spid" ] ;then
  echo "xcounters is running already! pid: $spid user:$user"
 else
  #no1min=''
  #if [ "$2" == 'no1min' ] ;then
  #  no1min='no1min'
  #fi
  logdir=logs
  cd ~/$logdir
  savelog xcountersdaq
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/dim/linux
  cd 
  nohup ./xcountersdaq 6 1 >$logdir/xcountersdaq.log &
  cat - <<-EOF 
  xcounters daemon ($VMECFDIR/monscal++/linux/MonScal = /home/tri/xcountersdaq) started. 
  log: ~tri/$logdir/xcountersdaq.log"
EOF
fi
elif [ "$1" == 'status' ] ;then     #----------------------- status
  if [ -z "$spid" ] ;then
    echo "xcounters server is not running"
  else
    echo "xcounters server is running, pid: $spid user:$user"
  fi
  exit
else     #-------------------------------------------------- bad param
  echo
  if [ -z "$spid" ] ;then
    echo "xcounters server is not running"
  else
    echo "xcounters server is running, pid: $spid user: $user"
  fi
  cat - <<-EOF
Usage:
xcounters.sh start
xcounters.sh stop
xcounters.sh status
EOF
fi

