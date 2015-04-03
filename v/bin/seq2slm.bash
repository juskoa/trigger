#!/bin/bash
if [ $# -eq 0 ] ;then
cat - <<-EOF
Start: from directory where seq file is:
 seq2slm.bash file [-run1] (file: without .seq sufix)
Operation: print .slm to stdout

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
abspath=`pwd`/$1.seq
$VMECFDIR/ltu/slmcomp.py $r12 $abspath
ls -l $1.seq
#ls -l $1.slm

