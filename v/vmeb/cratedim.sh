#!/bin/sh
export VMEWORKDIR=~/ltuclient/$1
#cd $VMEWORKDIR -will be daone later (in cratedim.py)
$VMEBDIR/cratedim.py $1
