#!/bin/bash
clog=$1
extpat1=""
mine=""
if [ -z "$1" ] ;then
  cat - <<-EOF
Usage:
ctplog.sh ctp_proxyYYMMDDhhmm.log patt

Looks for ^SMIRTL and 'patt'

EOF
  clog="ctp_proxy.log"
elif [ $# -eq 2 ] ;then
  extpat1="$2"
  mine="-e"
  echo $extpat1
fi
#ls -lt ctp_proxy*.log |head -$lastn
echo "pars: $# examining file: $clog"
echo
grep -e '^SMIRTL' $mine $extpat1 $clog |grep -v 'state EXECUTING' | grep -v 'in state RUNNING'
echo grep -e '^SMIRTL' $mine \'$extpat1\' $clog \| grep -v 'state EXECUTING' -v 'in state RUNNING'

