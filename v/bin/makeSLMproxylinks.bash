#!/bin/bash
function makinglinks() {
det=$1
    SEQFILES=noclasses
    cd $VMEWORKDIR/../$det/CFG/ltu/SLMproxy
    ABSEFI=$VMECFDIR/ltu_proxy/$SEQFILES
    echo "making links: $VMEWORKDIR/CFG/ltu/SLMproxy/*.seq"
    echo "          --> $ABSEFI/"
    #ln -sf "$ABSEFI/sod.seq" sod.seq
    #ln -sf "$ABSEFI/eod.seq" eod.seq
    #ln -sf "$ABSEFI/L2a.seq" L2a.seq
    ln -sf "$ABSEFI/sync.seq" sync.seq
}
if [ -n "$1" ] ;then
  makinglinks $1
else
  echo "usage: makeSLMproxylinks.bash detname"
fi
