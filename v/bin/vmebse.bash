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
vdir=`pwd`/v     # before git: vdir='v'
hname=`hostname -s`
if [ -n "$1" ] ;then
  if [ "$1" = 'swonly' ] ;then
    if [ -n "$2" ] ;then
      vdir=$2
    fi
  else
    vdir=$1
  fi
fi
if [ "$hname" = 'alidcscom026' -o "$hname" = 'pcalicebhm05' ] ;then
  ult=/data/ClientCommonRootFs/usr/local/trigger
elif [ "$hname" = 'pcalicebhm10' ] ;then
  ult=/home/dl/root/usr/local/trigger
elif [ "$hname" = 'alidcscom188' ] ;then
  ult=/data/dl/root/usr/local/trigger
#elif [ "$hname" = 'altri1' ] ;then
#  ult=/mnt/trigger
else
  ult=/usr/local/trigger
fi
first=`echo $vdir |cut -b 1`
#cannot be here if .bashrc invokes this! echo "hname:$hname vdir:$vdir"
if [ "$first" = '/' ] ;then
  rdir=$vdir
  ult=$vdir
else 
  rdir=$ult/$vdir
fi
# pit or lab:
export VMEDRIVER=VMERCC     # VMERCC, SIMVME
ix=`expr match "$hname" 'alidcs'`
if [ "$ix" = '6' ] ;then   #pit
  export VMESITE=ALICE
  export VMEGCC=g++ #export VMEGCC=gcc
  export DIM_DNS_NODE=aldaqecs
  #export ACT_DB=daq:daq@aldaqdb/ACT
  export ACT_DB=acttrg:CBNRR2be@aldaqdb/ACT
else
  export VMESITE=SERVER
  if [[ $hname != "altri1" ]] ;then
    export VMEDRIVER=SIMVME
  fi
  export VMEGCC=g++
  export DIM_DNS_NODE=pcald30
  export ACT_DB=daq:daq@pcald30/ACT
fi
export VMEBDIR=$rdir/vmeb
export VMECFDIR=$rdir/vme
export VMEWORKDIR=~/v/vme
export dbctp=$VMECFDIR/CFG/ctp/DB
export DIMDIR=/opt/dim
export SMIDIR=/opt/smi
export DATE_DAQLOGBOOK_DIR=/opt/libDAQlogbook
if [[ $PATH != *$ult/bin* ]] ;then
  export PATH="$PATH:$ult/bin"
fi
export PYTHONPATH=$VMEBDIR
if [[ $LD_LIBRARY_PATH != *:/lib/modules/daq:* ]] ;then
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lib/modules/daq:$DIMDIR/linux:$SMIDIR/linux:/opt/dip/lib
fi
if [ -e /opt/infoLogger/infoLoggerStandalone.sh ] ;then
. /opt/infoLogger/infoLoggerStandalone.sh
export DATE_INFOLOGGER_DIR=/opt/infoLogger
export DATE_INFOLOGGER_SYSTEM=TRG
fi
#aliases:
alias ssh="ssh -2"
alias vmecomp=$VMEBDIR/comp.py
alias vmecrate=$VMEBDIR/crate.py
alias vmedirs='echo   VMEDRIVER:$VMEDRIVER   VMESITE:$VMESITE   VMEGCC:$VMEGCC; echo   VMEBDIR:$VMEBDIR;echo   VMECFDIR:$VMECFDIR; echo VMEWORKDIR:$VMEWORKDIR; echo DATE_INFOLOGGER_DIR:$DATE_INFOLOGGER_DIR   DATE_DAQLOGBOOK_DIR:$DATE_DAQLOGBOOK_DIR
echo ACT_DB:$ACT_DB'
shopt -s expand_aliases
if [ "$hname" = 'alidcscom026' -o "$hname" = 'pcalicebhm05' ] ;then
  echo "Server cpu: $hname, alias defs in bin/setenv"
elif [ "$hname" = 'altri1' -o "$hname" = 'alidcsvme001' ] ;then
  #ctp vme cpu:
  alias "ctp=vmecrate nbi ctp"
  alias ctpdims=$ult/bin/ctpdims.sh
else
  # ltu vme cpu
  #alias ltuserver=/usr/local/trigger/bin/ltuserver.bash
  alias ltuproxy=/usr/local/trigger/bin/ltuproxy.sh
 fi
#
#alias slmshow=$VMECFDIR/ltu/slmcomp.py
#alias slmedit='cd $VMECFDIR/CFG/ltu/SLM;$VMECFDIR/ltu/ltu6'
#alias initttc='$VMEBDIR/../scripts/ttcvi'
#alias vmeshow='/local/trigger/ECS/vmeshow'
#echo "LD_LIBRARY_PATH:$LD_LIBRARY_PATH"
#echo 'useful aliases: vmedirs, vmecomp, vmecrate, slmshow, slmedit'
#vmedirs
