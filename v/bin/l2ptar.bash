#!/bin/bash
vd='.'
cd $VMECFDIR/..
tar -czvf ~/l2ptar.tgz -T - <<-EOF
$vd/bin/l2ptar.bash
$vd/vmeb/vmeblib/vmeblib.h
$vd/vmeb/vmeblib/makefile
$vd/vme/ltu/ltu_u.py
$vd/vme/tinproxy/readme
$vd/vme/ctp/ctp.h
$vd/vme/ctp/ctp.c
$vd/vme/ctp/ctplib/Partition.c
$vd/vme/ctp/ctplib/shared.c
$vd/vme/ctp/ctplib/Tpartition.c
$vd/vme/ctp/ctplib/makefile
$vd/vme/ctp_proxy/test.c
$vd/vme/ctp_proxy/fixedcnts.c
$vd/vme/ctp_proxy/dimservices.c
$vd/vme/ctp_proxy/ctp_proxy.c
$vd/vme/ctp_proxy/Tpartition.h
$vd/vme/ctp_proxy/clgroups.c
EOF
