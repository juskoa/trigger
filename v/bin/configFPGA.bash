#!/bin/bash
#this script is used by afterboot.bash, invoked from home/.custom.rc
# and/or directly from .custom.rc on alidcsvme004 (cosmic fanout)
if [ $# -ne 2 ] ;then
  echo "Usage:   configFPGA.bash detector_name 0xBASE"
  exit
fi
detname=$1
base=$2
cd /usr/local/trigger/v/vme
VME2FPGA/VME2FPGA.exe $base -noinitmac <<-EOF
LoadFPGA()
q
EOF
# allocate shared memory + fill it from ltutttc.cfg file:
export VMECFDIR=/usr/local/trigger/v/vme
export VMEBDIR=/usr/local/trigger/v/vmeb
#export VMEWORKDIR=/home/alice/trigger/v/$detname
export VMEWORKDIR=/tmp/$detname
#run1: sudo -u trigger mkdir -p $VMEWORKDIR
mkdir -p $VMEWORKDIR
cd $VMEWORKDIR
#run1: sudo -u trigger $VMECFDIR/ltu/ltu.exe $base <<-EOF2
$VMECFDIR/ltu/ltu.exe $base <<-EOF2
q
EOF2
