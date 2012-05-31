#!/bin/bash
# starting IR server processing and publishing IRs through DIM
#usage: irdims.sh start/stop/status
# this script is in trigger/bin and its copy is in trigger@27:bin/
function getpid() {
pidcmd=`ps ax -o'%p %u %a' |grep 'dim_IR_serv.exe'`
#13643 trigger  ./linux/ttcmidims
declare -a dl=($pidcmd)
#echo -e "pid:${dl[0]} cmd:${dl[1]} "
if [ "${dl[2]}" == 'dim_IR_serv.exe' ] ;then
  spid=${dl[0]}
  user=${dl[1]}
else
  spid=''
fi
#echo getpid: $spid $dl $pidcmd
}
function savelog() {
ds=`date +%y%m%d%H%M`   # %S -seconds
if [ -e "$1.log" ] ;then
  #echo "exists: $1"
  mv "$1.log" "$1$ds.log"
else
  echo "$1.log does not exist"
fi
}
#-------------------------------------
  echo 'This script is obsolete. Use irdims.py @alidcscom521'
exit
hname=`hostname`
if [ "$hname" != 'xxxxxxxxxxxx' -a "$hname" != 'alidcscom027' ] ;then
  echo 'This script can be started only from trigger@alidcscom027'
  exit
fi
getpid
if [ "$1" == 'stop' ] ;then   #-------------------------- stop
  if [ -n "$spid" ] ;then
    kill -s QUIT $spid
    echo "killing: $spid/$user"
    ps
  else
    echo "IR dimserver is not running"
  fi
elif [ "$1" == 'start' ] ;then    #----------------------- start
 if [ -n "$spid" ] ;then
  echo "IR dim server is running already! pid: $spid user:$user"
 else
  #no1min=''
  #if [ "$2" == 'no1min' ] ;then
  #  no1min='no1min'
  #fi
  logdir=~/IRS/LOG
  cd $logdir
  savelog ttcmidims
  cd ~/IRS
  nohup dim_IR_serv.exe >$logdir/dim_ir.log &
  #nohup linux/ttcmidims $no1min >$logdir/ttcmidims.log &
  cat - <<-EOF 
  IR dim server (trigger@27:IRS/dim_IR_serv.exe) started. 
  log: $logdir/dim_ir.log
EOF
fi
elif [ "$1" == 'status' ] ;then     #----------------------- status
  if [ -z "$spid" ] ;then
    echo "IR server is not running"
  else
    echo "IR server is running, pid: $spid user:$user"
  fi
  exit
else     #-------------------------------------------------- bad param
  echo
  if [ -z "$spid" ] ;then
    echo "IR server is not running"
  else
    echo "IR server is running, pid: $spid user: $user"
  fi
  cat - <<-EOF
Usage:
irdims.sh start
irdims.sh stop
irdims.sh status
EOF
fi

