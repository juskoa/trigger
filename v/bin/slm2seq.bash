#!/bin/bash
if [ $# == 0 ] ;then
cat - <<-EOF
Start: from directory where slm file is:
 slm2seq.bash file [-run1] (file: without .slm sufix)
EOF
exit
fi
if [ $# -eq 2 ] ;then
  if [ "$2" = "-run1" ] ;then
    r12="-run1"
  else
    r12=""
    echo "Bad parameter: $2 ignored (i.e. run2 format)"
  fi
fi
cp $1.slm $VMEWORKDIR/WORK/
cd $VMEWORKDIR
$VMECFDIR/ltu/slmcmp.py $r12 WORK/$1.slm
cd -
cp $VMEWORKDIR/WORK/slmseq.seq $1.seq
ls -l $1.slm
ls -l $1.seq
