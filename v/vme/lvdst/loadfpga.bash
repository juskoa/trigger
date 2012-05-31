#!/bin/bash
if [ $# -eq 1 ] ;then
  LTUBASE=$1
else
  LTUBASE='0x810000'
fi
if [ "$VMECFDIR" == "" ] ;then
  echo "VMECFDIR not defined"
  exit
fi
cd $VMECFDIR
if [ $# -eq 0 ] ;then
  cat - <<-EOF
Usage: loadfpga.bash BASE          (default: 0x810000)
EOF
exit
fi
cd $VMECFDIR
ls -l FlashMem.cfg
ln -sf CFG/lvdst/lvds_tester.rbf FlashMem.cfg
VME2FPGA/VME2FPGA.exe $LTUBASE -noinitmac <<-EOF
EraseFM()
LoadFM()
LoadFPGA()
q
EOF
#ln -sf CFG/ltu/ltu_f3.rbf FlashMem.cfg
echo "LTU base address: $LTUBASE"

