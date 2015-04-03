#!/bin/bash
#cd ~/act/db do not change wd (this script invoked from validate4act.bash,...)
if [ $# -eq 0 ] ;then
cat <<-EOF
give destination, e.g.: user@host:t/
EOF
  exit
fi
cd $dbctp
files="ttcparts.cfg ctpinputs.cfg VALID.DESCRIPTORS ctp.cfg TRIGGER.PFS VALID.BCMASKS VALID.LTUS ttcparts.cfg gcalib.cfg filter clockshift"
scp $files $1
