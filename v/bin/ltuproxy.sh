#!/bin/bash
# this script can be started after environment is set i.e.:
# cd /usr/local/trigger/stable ; . bin/vmebse.bash
function ipcremove() {
dtn=$1
ltubase=`awk '{if($1==detname) {print $3}}' detname=$dtn $cfgfile`
hn=`awk '{if($1==detname) {print $2}}' detname=$dtn $cfgfile`
if [ "$hn" = "$HOSTNAME"  -a -n "$ltubase" ] ;then
  key=${ltubase/0x/0x00}
  shmid=`ipcs -m |awk '{if($1==key) {print $2}}' key=$key`
  if [ -z "$shmid" ] ;then
    echo "Error: shmid not found for $dtn base:$ltubase key:$key"
  else
    ipcrm shm $shmid
    echo "ipcrm shm $shmid"
  fi
else
  echo "$dtn: not known or not on this machine (possible cpu: $hn)"
fi
}
#function makelinks() {
#    SEQFILES=noclasses
#    cd $VMEWORKDIR/CFG/ltu/SLMproxy
#    ABSEFI=$VMECFDIR/ltu_proxy/$SEQFILES
#    echo "making links: $VMEWORKDIR/CFG/ltu/SLMproxy/*.seq"
#    echo "          --> $ABSEFI/"
#    ln -sf "$ABSEFI/sod.seq" sod.seq
#    ln -sf "$ABSEFI/eod.seq" eod.seq
#    ln -sf "$ABSEFI/L2a.seq" L2a.seq
#    ln -sf "$ABSEFI/sync.seq" sync.seq
#}
function makedirs() {
echo "$VMEWORKDIR does not exist, creating, also SLMproxy links for $1..."
mkdir -p $VMEWORKDIR/CFG/ltu/SLMproxy
mkdir -p $VMEWORKDIR/CFG/ltu/SLM
if [ -e $VMEWORKDIR/CFG/ltu/ltuttc.cfg ]   ;then
  echo "ltuttc.cfg already exists, not overwritten"
else
  cp -a $VMECFDIR/CFG/ltu/ltuttc.cfg $VMEWORKDIR/CFG/ltu/
  echo "default ltuttc.cfg created"
fi
cp -a $VMECFDIR/CFG/ltu/SLM/*.slm $VMEWORKDIR/CFG/ltu/SLM/
mkdir -p $VMEWORKDIR/WORK
makeSLMproxylinks.bash $1
}
function mvfile() {
# $1: relative path of the log file NO SUFFIX, i.e.: WORK/ctpproxy
# operation: WORK/ctpproxy.log -> WORK/ctpproxyYYMMDDhhmm.log
ds=`date +%y%m%d%H%M`   # %S -seconds
if [ -e "$1.log" ] ;then
  #echo "exists: $1"
  mv "$1.log" "$1$ds.log"
else
  echo "$1.log does not exist (not renamed)"
fi
}

function ProxyOn() {
#declare -a doexist=(`$SMIBIN/proxyExists $proxyname`)
line=`$SMIBIN/proxyExists $1`
}
function StartProxy() {
# $1 -dtn (ssd, acorde, spd,...)
# $pid -should be empty
export VMEWORKDIR=~/v/$1      # started from trigger or triad account
if [ ! -d $VMEWORKDIR/WORK ] ; then
  echo "making working dirs for $1 ..."
  makedirs $1
fi
cd $VMEWORKDIR
curpwd=`pwd`; echo "StartProxy:pwd:$curpwd"
DTN=`echo $1 | awk '{ print toupper($0) }' -`
proxyname="TRIGGER::LTU-$DTN"
pid=`ps --columns 120 -C ltu_proxy o user,pid,args | awk '{if($4==detname) {print $2}}' detname=$proxyname`
if [ -z "$pid" ] ;then
  #export DIM_DNS_NODE=10.161.37.8
  #echo "starting $proxyname"
  #declare -a doexist=(`$SMIBIN/proxyExists $proxyname`)
  linemsg=`$SMIBIN/proxyExists $proxyname`
  declare -a doexist=($linemsg)
  proxyon=${doexist[2]}
  #proxyon=`ProxyOn $proxyname`
  echo "$proxyname proxyon:$proxyon doexist:$doexist"
  #exit
  #if [ "$proxyon" != "no" ] ;then
  #  echo "$proxyname is already running (pid:$pid). Forced start..."
  #  proxyon="no"
  #fi
  if [ "$proxyon" = "no" ] ;then
    #makelinks
    #echo
    echo "***** Starting LTU proxy $proxyname"
    cd $VMEWORKDIR
    # Following parameters can be specified:
    # -mode=$trigger 
    # -BCrate=$rate -L0=$l0 -busy=$busy -orbitbc=$orbitbc $nosodeod $nodim 
    ltubase=`awk '{if($1==detname) {print $3}}' detname=$1 $cfgfile`
    if [ -z "$ltubase" ] ;then
      echo "Bad name:$1" ; exit
    fi
    echo "DATE_INFOLOGGER_LOGHOST: $DATE_INFOLOGGER_LOGHOST proxy:$proxyname $ltubase"
    mvfile WORK/LTU-$DTN
    # gdb $VMECFDIR/ltu_proxy/linux/ltu_proxy
    # set args TRIGGER::LTU-DAQ -address=0x812000
    #nohup $VMECFDIR/ltu_proxy/linux/ltu_proxy  $proxyname \
    # -address=$ltubase </dev/null 2>&1 >WORK/LTU-$DTN.log & 
    nohup $VMECFDIR/ltu_proxy/linux/ltu_proxy  $proxyname \
     -address=$ltubase </dev/null 2>&1 >WORK/LTU-$DTN.log & 
    echo "nohup rc:$?"
  elif [ "$proxyon" = "-" ] ;then
    # seems the case:
    #[acorde@aco-vme WORK]$ /opt/smi/linux/proxyExists  TRIGGER::LTU-ACORDE
    # PID 11411 - Mon Apr 28 03:55:52 2014 - (ERROR) Client Connecting to DIM_DNS on acordetest: Connection refused
    echo "Error: $linemsg"
    echo "Server not started"
  else
    echo "$proxyname is already running (pid:$pid proxyon:$proxyon). Server not started"
    return
  fi
else
  echo "$1 server is already running (pid:$pid). Server not started"
  return
fi
}
function KillProxy() {
DTN=`echo $1 | awk '{ print toupper($0) }' -`
proxyname="TRIGGER::LTU-$DTN"
ltubase=`awk '{if($1==detname) {print $3}}' detname=$1 $cfgfile`
pid=`ps --columns 120 -C ltu_proxy o user,pid,args | awk '{if($4==detname) {print $2}}' detname=$proxyname`
pid2=`ps --columns 120 -C ltu.exe o user,pid,args | awk '{if($5==vmebase) {print $2}}' vmebase=$ltubase`
echo "KillProxy:$proxyname $ltubase going to kill:$pid $pid2"
kill $pid
pids=$pid
pid2=`ps --columns 120 -C ltu.exe o user,pid,args | awk '{if($5==vmebase) {print $2}}' vmebase=$ltubase`
if [ -n "$pid2" ] ;then
  kill $pid2
  pids="$pid $pid2"
fi
echo "$1 (pid:$pids) killed"
}
#------------------------------------------------ main
export OS=Linux
export ODIR=linux
export SMIBIN=$SMIDIR/linux
export LD_LIBRARY_PATH=/lib/modules/daq:/opt/dim/linux:/opt/smi/linux

#alldets="spd muon_trk ssd acorde tof hmpid muon_trg phos sdd v0 tpc"
cfgfile=$VMECFDIR/CFG/ctp/DB/ttcparts.cfg
# 
if [ $# -eq 2 ] ;then
  if [ $2 = "stop" -o $2 = "kill" -o $2 = "start" -o $2 = "restart" -o $2 = "status" ] ;then
    action=$2
    if [ $action = "stop" ] ;then
      action="kill"
    fi
  else
    action="error"
  fi
else
  if [ "$1" = "startall" ] ;then
    #echo "dbg: $HOSTNAME $cfgfile"
    for dtn1 in `awk '{if(($2==host) && ($3!="0")) {print $1}}' host=$HOSTNAME $cfgfile` ;do
      ix=`expr match "$dtn1" '#'`
      #echo "----------------------starting $dtn1 $ix"
      [ "$ix" = '1' ] && continue
      StartProxy $dtn1
    done
    exit
  elif [ "$1" = "killall" ] ;then
    for pid in `ps --columns 120 -C ltu_proxy o user,pid,args | awk '{print $2}'` ; do
      if [ "$pid" != 'PID' ] ;then 
        echo "killing $pid" ; kill $pid
      fi
    done
    for dtn1 in `awk '{if(($2==host) && ($3!="0")) {print $1}}' host=$HOSTNAME $cfgfile` ;do
      ipcremove $dtn1
    done
    exit
  elif [ "$1" = "all" -o "$1" = "active" ] ;then
    if [ "$1" = "all" ] ;then
    echo "Known detectors:"
    grep -v '^#' $cfgfile
    echo
  fi
  echo "On this machine, `hostname -s`, these servers are running:"
  #ps --columns 120 -C ltu_proxy o user,pid,args |colrm 16 72
  ps --columns 120 -C ltu_proxy o user,pid,args | awk '{print $1 "\t" $2 "\t" $4 " " $5}'
  exit
  else
  action="error"
  fi
fi
dtn=$1
export VMEWORKDIR=~/v/$dtn      # started from trigger account
# find base and right hostname for this detector:
ltubase=`awk '{if($1==detname) {print $3}}' detname=$dtn $cfgfile`
hn=`awk '{if($1==detname) {print $2}}' detname=$dtn $cfgfile`
if [ `hostname -s` != "$hn" ] ;then
  echo "On this host only:"
  grep `hostname -s` $cfgfile
  echo
elif [ ! -d $VMEWORKDIR/WORK ] ; then
  if [ "$2" = "start" ] ; then
     makedirs $dtn
     #makelinks
     #makeSLMproxylinks.bash $dtn
  elif [ "$2" = "status" ] ;then
    echo 2
    exit
  else
    echo "detector: $dtn par2: $2"
    echo "$VMEWORKDIR does not exist, exiting"
    exit
  fi
fi
cd $VMEWORKDIR
#
if [ -z "$ltubase" -o "$action" = "error" ] ;then
  cat - <<-EOF
Usage:
ltuproxy DETNAME [re]start -[re]start server for DETNAME on this computer
ltuproxy DETNAME kill      -kill server for DETNAME on this computer
ltuproxy DETNAME status    -return to stdout 0:ok  1:down  2:error(in DETNAME)
ltuproxy killall           -kill all active
ltuproxy startall          -start all (as in $cfgfile)
ltuproxy all               -show all possible and active
ltuproxy active            -show active ltuproxies on this machine
EOF
elif [ `hostname -s` = $hn ] ;then
  # we are on right server
  if [ "$action" = 'kill' -o "$action" = "restart" ] ;then
    KillProxy $dtn
    ipcremove $dtn
  fi
  if [ "$action" = 'start' -o "$action" = "restart" ] ;then
    StartProxy $dtn 
  elif [ "$action" = 'status' ] ;then
    DTN=`echo $dtn | awk '{ print toupper($0) }' -`
    ps --columns 120 -C ltu_proxy o user,pid,args | grep "LTU-$DTN" >/dev/null
    rcstatus=$?
    echo $rcstatus
    #return $rcstatus
  else
    if [ "$action" != 'kill' ] ;then
      echo "Unknown action:$action ???"
    fi
  fi
else
  echo "$dtn is on $hn, server not started"
fi

