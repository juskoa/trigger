#!/bin/bash
# the copy of this (ln -s does not work -selinux...) is in /usr/local/bin
echo "/usr/local/bin/r3afterboot.bash on" `hostname` pwd: `pwd` `whoami`
stat -c "%u %g" /proc/$BASHPID/
export CCTFS=/home/alice/trigger/git
cd $CCTFS/trigger ; . v/bin/vmebse.bash
echo "VME CFDIR:$VMECFDIR BDIR:$VMEBDIR SITE:$VMESITE"
echo "ttcmidims.sh startinit..."
# init RF2TTC+CORDE   (LOCAL, all CORDE regs to 512)
ttcmidims.sh startinit
exit
# LTU load, init -see configFPGA.bash 
cd $VMECFDIR
VME2FPGA/VME2FPGA.exe $base -noinitmac <<-EOF
LoadFPGA()
q
EOF
# see also startdaemons
