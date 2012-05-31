#!/bin/bash
if [ $# == 0 ] ;then
cat - <<-EOF
Start: from directory where slm file is:
 slm2seq.bash file (without .slm sufix)
EOF
exit
fi
cp $1.slm $VMEWORKDIR/WORK/
cd $VMEWORKDIR
$VMECFDIR/ltu/slmcmp.py WORK/$1.slm
cd -
cp $VMEWORKDIR/WORK/slmseq.seq $1.seq
ls -l $1.slm
ls -l $1.seq
