#!/bin/bash
# starting TTCMI server controlling the ALICE clock
#usage: ttcmidims.sh
. $VMECFDIR/../bin/auxfunctions
#
hname=`hostname -s`
if [ ! -e /dev/vme_rcc ] ;then           #-------------------- not VME CPU
  echo 'Debug? VMERCC not available'
  exit
fi

#if [ "$VMESITE" != "PRIVATE" -a "$hname" != 'alidcsvme017' -a "$hname" != 'altri2' ] ;then
if [ "$hname" != 'altri23' -a "$hname" != 'altri22' ] ;then
  #echo 'Debug? ttcmi boards available only on alidcsvme017 (or altri2 for test)'
  echo 'Debug? ttcmi control available only from new C8 VP-E24 CPUs'
  exit
fi
getpid 'linux/ttcmidims'
if [ "$1" == 'stop' ] ;then   #-------------------------- stop
  if [ -n "$spid" ] ;then
    kill -s QUIT $spid
    echo "killing: $spid/$user"
    ps
  else
    echo "ttcmidim server is not running"
  fi
elif [ "$1" == 'start' -o "$1" == 'startinit' ] ;then    #----------------------- start
 if [ -n "$spid" ] ;then
  echo "ttcmidim server is running already! pid: $spid user:$user"
 else
  #no1min=''
  #if [ "$2" == 'no1min' ] ;then
  #  no1min='no1min'
  #fi
  writeall=''
  if [ "$1" == 'startinit' ] ;then
    writeall='-writeall'
  fi
  logdir=$VMEWORKDIR/WORK
  cd $logdir
  savelog ttcmidims
  cd $VMEWORKDIR
  nohup $VMECFDIR/ttcmidaemons/linux/ttcmidims $writeall >$logdir/ttcmidims.log &
  #nohup linux/ttcmidims $no1min >$logdir/ttcmidims.log &
  cat - <<-EOF 
  ttcmidim server ($VMECFDIR/ttcmidaemons/linux/ttcmidims) started. 
  log: $VMEWORKDIR/WORK/ttcmidims.log"
EOF
fi
elif [ "$1" == 'status' ] ;then     #----------------------- status
  if [ -z "$spid" ] ;then
    echo "ttcmidim server is not running"
  else
    echo "ttcmidim server is running, pid: $spid user:$user"
  fi
  exit
else     #-------------------------------------------------- bad param
  echo
  if [ -z "$spid" ] ;then
    echo "ttcmidim server is not running"
  else
    echo "ttcmidim server is running, pid: $spid user: $user"
  fi
  cat - <<-EOF
Usage:
ttcmidims.sh start         -do not write to boards
ttcmidims.sh startinit     -init RF2TTC+CORDE   (LOCAL, all CORDE regs to 512)
  Note:clock shift in CORDE will be set in time of the LOCAL->BEAM1 change
ttcmidims.sh stop
ttcmidims.sh status
EOF
fi

