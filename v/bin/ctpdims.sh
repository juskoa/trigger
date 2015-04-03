#!/bin/bash
# starting CTPDIM server publishing CTP counters
#usage: ctpdims.sh
function getpid() {
pidcmd=`ps ax -o'%p %u %a' |grep 'linux/dims' |grep -v grep`
declare -a dl=($pidcmd)
#echo -e "pid:${dl[0]} cmd:${dl[1]} "
if [[ ${dl[2]} == *linux/dims ]] ;then   # linux/dims at the end of the command
  spid=${dl[0]}
  user=${dl[1]}
else
  spid=''
fi
#echo getpid: $spid
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
hname=`hostname -s`
if [ "$hname" != 'alidcsvme001' -a "$hname" != 'altri1' ] ;then
  echo 'This script can be started only on alidcsvme001 or altri1'
  exit
fi
getpid
if [ "$1" == 'stop' ] ;then   #-------------------------- stop
  if [ -n "$spid" ] ;then
    kill -s QUIT $spid
    echo "killing: $spid/$user"
  else
    echo "CTP dimserver is not running"
  fi
elif [ "$1" == 'start' ] ;then    #----------------------- start
  if [ -n "$spid" ] ;then
   echo "CTPDIM server is running already! pid: $spid user:$user"
  else
    no1min=''
    if [ "$2" == 'no1min' ] ;then
      no1min='no1min'
    fi
    logdir=$VMEWORKDIR/WORK ; cd $logdir ; savelog dims
    cd $VMEWORKDIR
    #pwd
    echo $LD_LIBRARY_PATH
    #echo 'no start, exiting...'
    #exit
    nohup $VMECFDIR/ctp_proxy/linux/dims $no1min >$logdir/dims.log &
    #nohup ~aj/v/vme/ctp_proxy/linux/dims >../WORK/dims.log &
    cat - <<-EOF 
    CTP dim server ($VMECFDIR/ctp_proxy/linux/dims) started. 
    log: $VMEWORKDIR/WORK/dims.log"
EOF
  fi
elif [ "$1" == 'status' ] ;then     #----------------------- status
  if [ -z "$spid" ] ;then
    echo "CTPDIM server is not running"
  else
    echo "CTPDIM server is running, pid: $spid user:$user"
  fi
  exit
else     #-------------------------------------------------- bad param
  echo
  if [ -z "$spid" ] ;then
    echo "CTPDIM server is not running"
  else
    echo "CTPDIM server is running, pid: $spid user: $user"
  fi
  cat - <<-EOF
Usage:
ctpdims start [no1min]
        no1min -> CTP COUNTERS READING THREAD WILL NOT BE STARTED
ctpdims stop
ctpdims status
EOF
fi

