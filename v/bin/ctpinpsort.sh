#!/bin/bash
echo see also $VMECFDIR/inputs/inputs.cfg
echo "sorting l0 inputs by ctp-l0 (1..24) to /tmp/l0s.cfg ..."
grep -v -e'^#' -e'^1' -e'^l0' $dbctp/ctpinputs.cfg |sort -n -k6,6 >/tmp/l0s.cfg
