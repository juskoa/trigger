#!/bin/bash
#Usage:
# . vmebse.bash [swonly] [vdir]
# swonly: change only VMEBDIR,VMECFDIR,PYTHONPATH + ALIASES!
# vdir:
# 1. v                       relative (rdir= ult/vdir )
# 2. vnew                    relative (rdir= ult/vdir )
# 3. /usr/local/trigger/v    absolute (rdir= vdir )
# Usefull for trigger@altri1:   production (/mnt) -> development (/usr):
# . vmebse.bash swonly /usr/local/trigger/v 
vdir='v'
hname=`hostname`
if [ -n "$1" ] ;then
  if [ "$1" = 'swonly' ] ;then
    if [ -n "$2" ] ;then
      vdir=$2
    fi
  else
    vdir=$1
  fi
fi
ult=~
first=`echo $vdir |cut -b 1`
#cannot be here if .bashrc invokes this! echo "hname:$hname vdir:$vdir"
if [ "$first" = '/' ] ;then
  rdir=$vdir
else 
  rdir=$ult/$vdir
fi
# pit or lab:
#ix=`expr match "$hname" 'alidcs'`
export VMESITE=SERVER
export VMEGCC=g++
export DIM_DNS_NODE=pcald30
#export ACT_DB=daq:daq@pcald30/ACT
export VMEBDIR=$rdir/vmeb
export VMECFDIR=$rdir/vme
export VMEWORKDIR=~/v/vme
if [ ! -d $VMEWORKDIR/WORK ] ;then
  mkdir -p $VMEWORKDIR/WORK
fi
export dbctp=$VMECFDIR/CFG/ctp/DB
# VMERCC, CAENVME, SIMVME, TSI148
[ -z "$VMEDRIVER" ] && export VMEDRIVER=VMERCC
[ -z "$VMEINCS" ] && export VMEINCS=/usr/local/inlude
[ -z "$VMELIBS" ] && export VMELIBS=/lib/modules/daq
[ -z "$DIMDIR" ] && export DIMDIR=/opt/dim
[ -z "$SMIDIR" ] && export SMIDIR=/opt/smi
[ -z "$DATE_DAQLOGBOOK_DIR" -a -d /opt/libDAQlogbook ] && export DATE_DAQLOGBOOK_DIR=/opt/libDAQlogbook
[ -z "$DATE_INFOLOGGER_DIR" -a -d /opt/infoLogger ] && export DATE_INFOLOGGER_DIR=/opt/infoLogger
if [ -e /opt/infoLogger/infoLoggerStandalone.sh ] ;then
. /opt/infoLogger/infoLoggerStandalone.sh
export DATE_INFOLOGGER_SYSTEM=TRG
fi
#
if [[ $PATH != *$rdir/scripts* ]] ;then
  export PATH="$PATH:$rdir/scripts"
fi
export PYTHONPATH=$VMEBDIR
if [[ $LD_LIBRARY_PATH != *:$VMELIBS:* ]] ;then
  #export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ATLAS/lib:$DIMDIR/linux:$SMIDIR/linux:/opt/dip/lib
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$VMELIBS:$DIMDIR/linux:$SMIDIR/linux
fi
#aliases:
alias ssh="ssh -2"
alias vmecomp=$VMEBDIR/comp.py
alias vmecrate=$VMEBDIR/crate.py
alias vmedirs='echo VMEDRIVER:$VMEDRIVER   VMESITE:$VMESITE   VMEGCC:$VMEGCC; echo VMEBDIR:$VMEBDIR;echo VMECFDIR:$VMECFDIR; echo VMEWORKDIR:$VMEWORKDIR; echo VMELIBS:$VMELIBS   VMEINCS:$VMEINCS; echo DIMDIR:$DIMDIR   SMIDIR:$SMIDIR; echo DATE_INFOLOGGER_DIR:$DATE_INFOLOGGER_DIR   DATE_DAQLOGBOOK_DIR:$DATE_DAQLOGBOOK_DIR
echo ACT_DB:$ACT_DB'
shopt -s expand_aliases
if [ "$hname" = 'altri1' -o "$hname" = 'alidcsvme001' ] ;then
  #ctp vme cpu:
  alias "ctp=vmecrate nbi ctp"
  alias ctpdims=$ult/bin/ctpdims.sh
else
  # ltu vme cpu
  #alias ltuserver=/usr/local/trigger/bin/ltuserver.bash
  alias ltuproxy=$VMECFDIR/../scripts/ltuproxy.sh
fi
#
#alias slmshow=$VMECFDIR/ltu/slmcomp.py
#alias slmedit='cd $VMECFDIR/CFG/ltu/SLM;$VMECFDIR/ltu/ltu6'
#alias initttc='$VMEBDIR/../scripts/ttcvi'
#alias vmeshow='/local/trigger/ECS/vmeshow'
#echo "LD_LIBRARY_PATH:$LD_LIBRARY_PATH"
#echo 'useful aliases: vmedirs, vmecomp, vmecrate, slmshow, slmedit'
#vmedirs
