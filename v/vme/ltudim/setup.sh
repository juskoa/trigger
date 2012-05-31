#!/bin/sh
# this script is part of ltuclient distribution (bin/distrdimclient)
export DIMDIR=/opt/dim
export SMIDIR=/opt/smi
#export DIM_DNS_NODE=pcald30
# next line : VME driver shared libraries + DIM/SMI libraries
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DIMDIR/linux:$SMIDIR/linux
# next line only if needed (i.e. VMERCC)
if [ $# -eq 0 ] ;then
  rdir=/opt/ltuclient
else
  rdir=$1
fi
export VMEBDIR=$rdir/vmeb
#export VMEWORKDIR=~/ltuclient/vme       # see cratedim.sh
export VMECFDIR=$rdir/vme
export PYTHONPATH=$VMEBDIR
shopt -s expand_aliases
alias vmedirs='echo   VMEBDIR:$VMEBDIR;echo   VMECFDIR:$VMECFDIR; echo VMEWORKDIR:$VMEWORKDIR'
alias vmecomp=$VMEBDIR/comp.py
export LTU_CLIENT=$VMEBDIR/cratedim.sh
alias ltuclient=$LTU_CLIENT
alias slmshow=$VMECFDIR/ltu/slmcomp.py
