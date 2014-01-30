#!/bin/bash
#This has to be started on alitri from ltu_proxy/noclasses
# and followed by makeSLMproxylinks.bash DETECTOR for each
# detector on all trigger@diskless machines before 1st ltu_proxy satrt.
function cmp() {
if [ "$2" = "run1" ] ;then
  r="run1"
else:
  r=""
fi
echo "cd $VMECFDIR/ltu_proxy/noclasses ..."
cd $VMECFDIR/ltu_proxy/noclasses
cp $1.slm $VMEWORKDIR/WORK/
cd $VMEWORKDIR
$VMECFDIR/ltu/slmcmp.py WORK/$1.slm
cp WORK/slmseq.seq WORK/$1.seq
$VMECFDIR/ltu/slmcmp.py -run1 WORK/$1.slm
cp WORK/slmseq.seq WORK/$1_run1.seq
cd -
cp $VMEWORKDIR/WORK/$1.seq .
cp $VMEWORKDIR/WORK/$1_run1.seq .
ls -l $1.seq $1_run1.seq
}

cmp sod
cmp eod
cmp sync
cmp L2a
cmp L2async
