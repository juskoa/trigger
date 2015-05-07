#!/bin/bash
function savelog() {
ds=`date +%y%m%d%H%M`   # %S -seconds
cd logs
if [ -e "$1.log" ] ;then
  #echo "exists: $1"
  mv "$1.log" "$1$ds.log"
else
  echo "$1.log does not exist"
fi
cd ..
}
function status_rrd() {
# ps --columns 120 -C ltu_proxy o user,pid,args |colrm 16 50
echo '----- rrd status (2 processes):'
ps -C readctpc -C readltuc -o"%p %a"
}
function status_pydim() {
echo '----- pydim status (2 processes), HAS TO BE STARTED BEFORE ctpproxy!:'
ps -C pydimserver.py -C server -o"%p %a"
pids=`ps -C pydimserver.py -C server -o"%p %a" --no-headers |colrm 6`
#if [ "$pids" != "" ] ;then
#  echo "pid: $pids"
#fi
}
function status_html() {
echo '----- html status (1 process):'
#ps -C htmlCtpBusys.py -o"%p %a"
line=`ps --no-headers -C htmlCtpBusys.py -o"%p %a"`
if [ "$line" != "" ] ;then
  declare -a dl=($line) 
  spid=${dl[0]}
  echo $line
  echo pid: $spid
fi
}
function st3() {
#echo "st3:$1 script:$2 user@machine:$3 action:$4"
#echo hanme:$hname
#echo "PATH:$PATH"
task=$1
script=$2
remlogin=$3   #NONE -available only in the pit, 
#              LOCAL: start on server (no ssh i.e. trigger@alitri)
action=$4
#which $script
#if [ $? -eq 0 ] ;then
#  echo "ok"
#else
#  echo "unknown script"
#  return 8
#fi
#if [ `expr index $remlogin NONE` -ne 0 ] ;then  matches begining of the string
if [ "$remlogin" = "trigger@NONE" ] ;then
  echo "$task available only in the pit"
  return 8
fi
if [ "$action" = "stop" ] ;then
  if [ "$remlogin" == 'LOCAL' -o "$remlogin" == "trigger@$hname"  ] ;then
    $script stop
  else
    ssh -2 -q  $remlogin "$script stop"
  fi
  return $?
elif [ "$action" = "start" -o "$action" = "startnd" -o  "$action" = "startnr" ] ;then
  #echo "ssh -2 -q -f $remlogin $script start"
  if [ "$remlogin" == 'LOCAL' -o "$remlogin" == "trigger@$hname"  ] ;then
    $script start
  else
    nohup ssh -2 -q -f $remlogin "$script start"
  fi
  return $?
else
  if [ "$action" != "status" ] ;then
    echo "------- bad action:$action.   ----- $task status:"
  #else
  #  echo "----- $task status:"
  fi
  echo "----- $task status:"
  if [ "$remlogin" == 'LOCAL' -o "$remlogin" == "trigger@$hname"  ] ;then
    $script status 
  else
    ssh -2 -q $remlogin "$script status" 
  fi
  return $?
fi
}
function showpids() {
s_dnames="pydimserver.py htmlCtpBusys.py readctpc readltuc udpmon irdim xcounters gmonscal masksServer ctpwsgi"
c_dnames="ctpproxy dims gcalib.exe gdb test"
mi_dnames="ttcmidims"
#echo "dnames:$dnames"
#echo "1,2:$1,$2"
for dn in $s_dnames ;do
  pgrep -l $dn
  [ $? -eq 1 ] && echo ----	$dn
done
echo ------------ $ctpvme: $c_dnames
sshcmd=""
for dn in $c_dnames ;do
  sshcmd="pgrep -x -l $dn; $sshcmd"
done
#echo $sshcmd
ssh -2 trigger@$ctpvme "$sshcmd"
echo ------------ $ttcmivme: $mi_dnames
sshcmd=""
for dn in $mi_dnames ;do
  sshcmd="pgrep -x -l $dn; $sshcmd"
done
#echo $sshcmd
ssh -2 trigger@$ttcmivme "$sshcmd"
#for dn in $c_dnames ;do
#  ssh -2 trigger@$ctpvme pgrep -x -l $dn
#  [ $? -eq 1 ] && echo ----	$dn
#done
}
#--------------------------------------------------
hname=`hostname -s`
if [ "$hname" != 'zenaj' -a "$hname" != 'avmes' -a "$hname" != 'pcalicebhm10' \
     -a "$hname" != 'alidcscom188' -a "$hname" != 'alidcscom835' ] ;then
echo 'This script can be started only on trigger@alidcscom835/188 or trigger@pcalicebhm10/avmes'
exit 8
fi
if [ "$VMESITE" = 'ALICE' ] ;then
  ctpvme=alidcsvme001
  ttcmivme=alidcsvme017
  server=alidcscom835   # alidcscom188  alitri
  server27=alidcscom707 # alitrir   see also alidcscom521
elif [ "$VMESITE" = 'SERVER' ] ;then
  ctpvme=altri1
  ttcmivme=altri1
  server=$hname
  server27=NONE
fi
if [ "$1" = 'help' ] ;then
cat - <<-EOF
Usage:                    
startClients              -get status of all daemons
startClients pids         -get pids of all daemons
startClients help         -this message

startClients pydim stop | start | status
             html 
             rrd 
             udpmon 
             ctpproxy 
             ctpdim 
             gcalib
             masksServer

Available only in P2:
             ttcmidim 
             irdim 
             xcounters
             diprfrx
             gmonscal
Purpose:
pydim:     ~/v/vme/WORK/RCFG/*.rcfg files creation/deleting
html:      global runs status big screen (http://$server ...)
rrd:       reading CTP/LTU counters and storing them into RRD database
udpmon:    gathering udp monitoring messages (ttcmi, bobr, daemons...)
           log: ~/CNTRRD/logs/udpmon.log
ctpproxy:  allows global partitions (has to be restarted with pydim)
              start: just start 
           actstart: load CTP config from ACT + load CTpswitch + start
ctpdim:    DIM server running on alidcsvme001, reading counters once per minute
ttcmidim:  DIM server monitoring/changing global clock (warning when 
           the clock change foreseen).
           miclock: the client controlling ttcmidim server
irdim:     running on alitrir. DIM server processing Interaction records
xcounters: creating xcounters files and posting them to DCS XFS
           updating counters in DAQlogbook.    RUNS in tri account
           see ~tri/readme for more info about compile/start/stop
diprfrx    DIP service publishing the LHC TTCmi RF -see http://cern.ch/ttcpage1
gcalib     sending cal. triggers to the detectores during global run
gmonscal   running on trigger@alitrir. Creating: 
           $server:v/vme/WORK/MONSCAL/inputs.png
           $server:v/vme/WORK/MONSCAL/RUNNUMBER_CLUSTER.png
masksServer running on alitri. Commands: start/stop/status/update
           See v/vme/WORK/masksServer.log,.pid 
           DIM publications of CTPBCM/A,C,S,SA,... -masks available 
           in VALID.CTPINPUTS).
ctpwsgi    running on alitri. start/stop/status
           wsgi server. needed for BUSY status screen when the way how
           busy is calculated needs to be changed. Log: v/vme/WORK
Problems: see corresponding files in:
   ~/CNTRRD/logs                     -pydim html rrd
   trigger@alidcsvme001:v/vme/WORK/  -ctpproxy ctpdim gcalib
   trigger@alidcsvme017:v/vme/WORK/  -ttcmidim diprfrx
   ~tri/logs                         -xcounters
   trigger@alitrir:rl/monscal_root        -gmonscal sources
   trigger@alitrir:rl/monscal_root/WORK   -gmonscal log
   trigger@alitrir:IRS/LUMI_FILES/MASSI  IRS/LOG/LOG   -irdim
   $VMEWORKDIR/vme/masksServer.log/pid    -masksServer
EOF
exit
fi
dnames="pydim html rrd udpmon ctpproxy ctpdim ttcmidim irdim xcounters diprfrx gcalib gmonscal masksServer ctpwsgi diprfrx"
cd ~/CNTRRD
if [ $# -eq 0 ] ;then
  echo "Current status:                 (type help to get help message)"
  dmn='all'
  sss='status'
elif [ $# -eq 1 -a "$1" = "pids" ] ;then
  showpids
  dmn=pids
elif [ $# -eq 1 ] ;then
  echo "Current sttus of $1            (type help to get help message)"
  dmn=$1
  sss='status'
else
  dmn=$1
  sss=$2
fi
echo $dnames | grep "$dmn " >/dev/null
rc=$?
echo "rc:$rc dmn:$dmn:"
if [ "$dmn" != "all" -a "$rc" = "1" ] ;then
  echo "unknown daemon: $dmn"
  exit 8
fi
#echo "$dmn $sss..."
if [ $dmn = "masksServer" -o "$dmn" = "all" ] ;then
  if [ "$sss" = "stop" ] ;then
    if [ -f $VMEWORKDIR/WORK/masksServer.pid ] ; then
      kill `cat $VMEWORKDIR/WORK/masksServer.pid`
    else
      echo " not started"
    fi
  elif [ "$sss" = "update" ] ;then
    if [ -f $VMEWORKDIR/WORK/masksServer.pid ] ; then
      kill -s SIGUSR1 `cat $VMEWORKDIR/WORK/masksServer.pid`
    else
      echo " not started"
    fi
  elif [ "$sss" = "start" ] ;then
    if [ -f $VMEWORKDIR/WORK/masksServer.pid ] ; then
      echo " already started"
    else
      nohup python $VMECFDIR/pydim/masksServer.py >/dev/null &
    fi
  elif [ "$sss" = "status" ] ;then
    echo '----- masksServer status:'
    if [ -f $VMEWORKDIR/WORK/masksServer.pid ] ; then
      echo '----- masksServer status (1 process):'
      echo "pid: `cat $VMEWORKDIR/WORK/masksServer.pid`"
    else
      echo " not started"
    fi
  fi
fi
if [ $dmn = "pydim" -o "$dmn" = "all" ] ;then
#--------------------------------------- pydimserver
  if [ "$sss" = "stop" ] ;then
    echo "stopping pydim..."
    cd $VMECFDIR/pydim 
    linux_s/client CTPRCFG/RCFG stop
  elif [ "$sss" = "start" ] ;then
    savelog pydimserver
    #is in vmebse.bash export ACT_DB=daq:daq@aldaqdb/ACT
    export PYTHONPATH=$PYTHONPATH:$VMECFDIR/TRG_DBED:$VMECFDIR/ttcmidaemons
    nohup $VMECFDIR/pydim/pydimserver.py >logs/pydimserver.log &
    sleep 1
    echo '2 processes should be running if started successfully:'
    echo
    echo 'Note: after pydim was restarted, RESTART ctp_proxy !'
    echo 'It seems, .rcfg file requests from ctp_proxy are not'
    echo 'processed in time without ctp_proxy restart!'
    echo
  elif [ "$sss" = "status" ] ;then
    status_pydim
  fi
fi
if [ $dmn = "html" -o "$dmn" = 'all' ] ;then
#------------------------------------------- htmlCtpBusy
  if [ "$sss" = "stop" ] ;then
    echo q >/tmp/htmlfifo   
  elif [ "$sss" = "start" ] ;then
    savelog htmlCtpBusys
    nohup $VMECFDIR/CNTRRD/htmlCtpBusys.py htmlfifo >logs/htmlCtpBusys.log &
    #nohup ~/CNTRRD/htmlCtpBusys.py htmlfifo >logs/htmlCtpBusys.log &
  elif [ "$sss" = "status" ] ;then
    status_html
  fi
fi
if [ $dmn = "rrd" -o "$dmn" = 'all' ] ;then
#------------------------------------------------ rrd
  if [ "$sss" = "stop" ] ;then
    #echo "not done (kill manually: kill readctpcPID readltucPID)"
    ps -C readctpc -C readltuc -o"%p %a"
    rpids=`ps --no-headers -C readctpc -C readltuc -o"%p %a" | \
awk '
BEGIN {pids=""}
{ 
# print $1
  pids=pids $1 " "
}
END {printf "%s", pids}
' -`
    echo "killing $rpids ... html WILL BE STOPPED ALSO"
    kill $rpids
  elif [ "$sss" = "start" ] ;then
    if [ ! -p /tmp/htmlfifo ] ;then
      echo "creating /tmphtmlfifo..."
      rm -f /tmp/htmlfifo
      mkfifo /tmp/htmlfifo
    fi
    cd ~/CNTRRD
    savelog readctpc
    nohup $VMECFDIR/CNTRRD/linux/readctpc >logs/readctpc.log &
    echo 'check if html is running.' 
    #nohup linux/readctpc >logs/readctpc.log &
# always after readctpc restart, /tmp/htmlfifo has to be read out.
# it seems, htmlCtpBusys.py is stopped automatically when readctpc killed.
# 1.
# htmlCtpBusys.py reads /tmp/htmlfifo to get current detectors' busys
# check if it is PROPERLY running (CNTRRD/logs/readctpc.log,htmlCtpBusys.log
# 2. 
# if htmlCtpBusys.py is supposed to be stopped, arrange the readings:
# nohup cat /tmp/htmlfifo >logs/htmlfifo.log &
#
    savelog readltuc
    nohup $VMECFDIR/CNTRRD/linux/readltuc >logs/readltuc.log &
  elif [ "$sss" = "status" ] ;then
    status_rrd
  fi
fi
if [ $dmn = "udpmon" -o "$dmn" = 'all' ] ;then #-------------------- udpmon
  #st3 udpmon udpmon.sh trigger@$server $sss  -we are already on server!
  echo '----- udpmon status (1 process):'
  udpmon.sh $sss
  retc=$?
  [ $dmn = "udpmon" ] && exit $retc
fi
if [ $dmn = "ctpproxy" -o "$dmn" = 'all' ] ;then #-------------------- ctpproxy
  if [ $sss = "actstart" ] ;then
    cd $VMECFDIR/switchgui
    ./switched.py act
    rc=$?
    echo "switched.py act rc: $rc"
    if [ $rc -ne 0 ] ;then
      rc=1 #return 1
    else
      st3 ctpproxy ctpproxy.sh trigger@$ctpvme start
    fi
  else
    st3 ctpproxy ctpproxy.sh trigger@$ctpvme $sss
  fi
  #echo "st3 rc:$?"
fi
if [ $dmn = "ctpdim" -o "$dmn" = 'all' ] ;then #----------------------- ctpdim
  st3 ctpdim ctpdims.sh trigger@$ctpvme $sss
fi
if [ $dmn = "ttcmidim" -o "$dmn" = 'all' ] ;then #-------------------- ttcmidim
  st3 ttcmidim ttcmidims.sh trigger@$ttcmivme $sss
fi
# group of daemons available only in the pit:
if [ $dmn = "irdim" -o "$dmn" = 'all' ] ;then #-------------------- irdim
  if [ "$VMESITE" != "ALICE" ] ;then
    echo "irdim available only in the pit"
  else
    st3 irdim irdims.py trigger@alidcscom521 $sss
  fi
fi
if [ $dmn = "xcounters" -o "$dmn" = 'all' ] ;then #------------------- xcounters
  if [ "$VMESITE" != "ALICE" ] ;then
    #echo "xcounters available only in the pit"
    xcounters.sh $sss
  else
    st3 xcounters xcounters.sh tri@$server $sss
  fi
fi
if [ $dmn = "diprfrx" -o "$dmn" = 'all' ] ;then #------------------- diprfrx
  if [ "$VMESITE" != "ALICE" ] ;then
    echo "diprfrx available only in the pit"
  else
    st3 diprfrx diprfrx.sh trigger@$ttcmivme $sss
  fi
fi
#if [ $dmn = "monscal" -o "$dmn" = 'all' ] ;then #-------------------- monscal
#  st3 monscal monscal.sh trigger@$server $sss
#fi
if [ $dmn = "gmonscal" -o "$dmn" = 'all' ] ;then #-------------------- gmonscal
  st3 gmonscal gmonscal.sh trigger@$server27 $sss
fi
if [ $dmn = "ctpwsgi" -o "$dmn" = 'all' ] ;then #-------------------- ctpwsgi
  st3 ctpwsgi ctpwsgi.sh LOCAL $sss
fi
#---------------
if [ $dmn = "gcalib" -o "$dmn" = 'all' ] ;then #------------------- gcalib
  st3 gcalib gcalib.sh trigger@$ctpvme $sss
  retc=$?
  [ $dmn = "gcalib" ] && exit $retc
fi

