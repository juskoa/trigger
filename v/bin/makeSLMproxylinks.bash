#!/bin/bash
function makedirs() {
det=$1
cd $VMEWORKDIR/..
mkdir -p $det/CFG/ltu/SLMproxy $det/CFG/ltu/SLM $det/WORK
}
function makelinks() {
det=$1
cd $VMEWORKDIR/../$det/CFG/ltu/SLMproxy
echo "making links: $VMEWORKDIR/../$det/CFG/ltu/SLMproxy/*.seq"
echo "          --> $ABSEFI/"
ln -sf "$ABSEFI/sod.seq" sod.seq
ln -sf "$ABSEFI/sod_run1.seq" sod_run1.seq
ln -sf "$ABSEFI/eod.seq" eod.seq
ln -sf "$ABSEFI/eod_run1.seq" eod_run1.seq
ln -sf "$ABSEFI/L2a.seq" L2a.seq
ln -sf "$ABSEFI/L2a_run1.seq" L2a_run1.seq
ln -sf "$ABSEFI/sync.seq" sync.seq
ln -sf "$ABSEFI/sync_run1.seq" sync_run1.seq
echo ; pwd ; ls -go --time-style='+%d.%m.%Y'
}
SEQFILES=noclasses
ABSEFI=$VMECFDIR/ltu_proxy/$SEQFILES
if [ -n "$1" ] ;then
  makedirs $1
  makelinks $1
else
  cat - <<-EOF

makeSLMproxylinks.bash detname
Prepares sod,eod,L2a,sync links: 
  $VMEWORKDIR/../detname/CFG/ltu/SLMproxy/*.seq"
  --> $ABSEFI/"
EOF
fi
