#!/bin/bash
vd='vd'
cd $VMECFDIR/../..
tar -czvf ~/l2ptar.tgz -T - <<-EOF
$vd/vme/ltu/ltu.h
$vd/vme/ltu/ltulib/ttcsubs.c
$vd/vme/ltu/ltulib/ltuCounters.c
$vd/vme/ltu/ltulib/ltuinit.c
$vd/vme/ltu_proxy/ltu_utils.c
$vd/vme/ltu_proxy/ltu_proxy.c
$vd/vme/ltu_proxy/ltudimservices.c
$vd/vme/ltu_proxy/ltu_utils.h
$vd/vme/ctp/ctplib/bobr.c
$vd/vme/ctp_proxy/gcalib.c
$vd/vme/ctp_proxy/ctp_proxy.c
$vd/vme/dimcdistrib/monnames.txt
$vd/vme/ttcmidaemons/ttcmidims.c
$vd/bin/ltuproxy.sh
$vd/bin/ctpproxy.py
$vd/bin/l2ptar26apr.bash
$vd/vme/CNTRRD/readctpc.c
$vd/vme/CNTWEB/mons.py
EOF

