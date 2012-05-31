#!/bin/bash
if [ $# == 0 ] ;then
cat - <<-EOF
Start: from directory where seq file is:
 seq2slm.bash file (without .seq sufix)
EOF
exit
fi
abspath=`pwd`/$1.seq
$VMECFDIR/ltu/slmcomp.py $abspath
ls -l $1.seq
#ls -l $1.slm

