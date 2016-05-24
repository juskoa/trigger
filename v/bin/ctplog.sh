#!/bin/bash
clog=$1
if [ -z "$1" ] ;then
  cat - <<-EOF
Usage:
ctplog.sh ctp_proxyYYMMDDhhmm.log

EOF
  clog="ctp_proxy.log"
elif [ $# -eq 2 ] ;then
  extpat1="$2"
  echo $extpat1
fi
#ls -lt ctp_proxy*.log |head -$lastn
echo "pars: $# examining file: $clog"
echo
grep -e '^SMIRTL' -e $extpat1 $clog |grep -v 'state EXECUTING'
echo grep -e '^SMIRTL' -e \'$extpat1\' $clog \| grep -v 'state EXECUTING'
