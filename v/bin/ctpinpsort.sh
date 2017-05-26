#!/bin/bash
# see also vme/inputs/inputs.cfg
# sort l0 inputs by ctp-l0 (1..24)
grep -v -e'^#' -e'^1' -e'^l0' $dbctp/ctpinputs.cfg |sort -n -k6,6 >/tmp/l0s.cfg
