#!/bin/bash
if [[ $# -eq 0 ]] ;then
  echo see also $VMECFDIR/inputs/inputs.cfg
  ctpins=$dbctp/ctpinputs.cfg
  echo "sorting l0 inputs by ctp-l0 (1..24) to /tmp/l0s.cfg ..."
else
  echo "sorting l0 inputs in $ctpins by ctp-l0 (1..24) to /tmp/l0s.cfg ..."
  ctpins=$1
fi
grep -v -e'^#' -e'^1' -e'^l0' $ctpins |sort -n -k6,6 >/tmp/l0s.cfg
