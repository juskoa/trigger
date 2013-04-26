#!/bin/sh
dnsdimnode='pcald30'
rdir=/opt/ltusw
if [ -n "$1" ] ;then
  dnsdimnode=$1
fi
export DIM_DNS_NODE=$dnsdimnode
export VMEBDIR=$rdir/vmeb
export VMECFDIR=$rdir/vme
export VMEWORKDIR=~/v/vme
export dbctp=$VMECFDIR/CFG/ctp/DB
[ -z "$VMELIBS" ] && export VMELIBS=/lib/modules/daq
[ -z "$DIMDIR" ] && export DIMDIR=/opt/dim
[ -z "$SMIDIR" ] && export SMIDIR=/opt/smi
[ -z "$DATE_DAQLOGBOOK_DIR" -a -d /opt/libDAQlogbook ] && export DATE_DAQLOGBOOK_DIR=/opt/libDAQlogbook
[ -z "$DATE_INFOLOGGER_DIR" -a -d /opt/infoLogger ] && export DATE_INFOLOGGER_DIR=/opt/infoLogger
if [ -e /opt/infoLogger/infoLoggerStandalone.sh ] ;then
. /opt/infoLogger/infoLoggerStandalone.sh
export DATE_INFOLOGGER_SYSTEM=TRG
fi
if [[ $LD_LIBRARY_PATH != *:$VMELIBS:* ]] ;then
  #export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ATLAS/lib:$DIMDIR/linux:$SMIDIR/linux:/opt/dip/lib
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$VMELIBS:$DIMDIR/linux:$SMIDIR/linux
fi
if [[ $PATH != *$rdir/scripts* ]] ;then
  export PATH="$PATH:$rdir/scripts"
fi
alias vmecrate=$VMEBDIR/crate.py
export PYTHONPATH=$VMEBDIR
alias ltuproxy=$VMECFDIR/../scripts/ltuproxy.sh
alias vmedirs='echo VMEDRIVER:$VMEDRIVER   VMESITE:$VMESITE   VMEGCC:$VMEGCC; echo VMEBDIR:$VMEBDIR;echo VMECFDIR:$VMECFDIR; echo VMEWORKDIR:$VMEWORKDIR; echo VMELIBS:$VMELIBS ; echo DIMDIR:$DIMDIR   SMIDIR:$SMIDIR; echo DATE_INFOLOGGER_DIR:$DATE_INFOLOGGER_DIR   DATE_DAQLOGBOOK_DIR:$DATE_DAQLOGBOOK_DIR ; echo ACT_DB:$ACT_DB'
shopt -s expand_aliases
