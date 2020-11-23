#!/bin/bash
# the copy of this (ln -s does not work -selinux...) is in /usr/local/bin
echo "/usr/local/bin/r3afterboot.bash on" `hostname` pwd: `pwd` `whoami`
echo "VME CFDIR:$VMECFDIR BDIR:$VMEBDIR SITE:$VMESITE"
stat -c "%u %g" /proc/$BASHPID/
echo "no init yet..."
exit
# LTU load, init -see configFPGA.bash 
cd $VMECFDIR
VME2FPGA/VME2FPGA.exe $base -noinitmac <<-EOF
LoadFPGA()
q
EOF
# init RF2TTC+CORDE   (LOCAL, all CORDE regs to 512)
#ttcmidims.sh startinit
# see also startdaemons
