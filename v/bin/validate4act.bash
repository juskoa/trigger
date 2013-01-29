#!/bin/bash
function cpfiles() {
  cd /tmp/validate/DB
  cp $dbctp/ttcparts.cfg .
  cp $dbctp/VALID.CTPINPUTS .
  cp $dbctp/CTP.SWITCH .
  cp $dbctp/L0.INPUTS .
  cp $dbctp/VALID.DESCRIPTORS .
  cp $dbctp/ctp.cfg .
  cp $dbctp/TRIGGER.PFS .
  cp $dbctp/VALID.BCMASKS .
  cp $dbctp/LTU.SWITCH .
  cp $dbctp/VALID.LTUS .
  cp $dbctp/ttcparts.cfg .
  cp $dbctp/gcalib.cfg .
  cp $dbctp/filter .
  cp $dbctp/clockshift .
}
if test "$#" -eq 0  ;then
cat - <<-EOF

validate4act.bash pname [fresh]
pname: pname.partition in /tmp/validate/pardefs directory
fresh: copy current dbctp files into /tmp/validate/DB/
operation:
1. cp /tmp/validate/pardefs/pname.partition $dbctp/../pardefs/val4act.partition
2. validate val4act partition using dbctp files in /tmp/validate/DB

EOF
exit
fi
if test "$2" = "fresh"  ;then
  cpfiles
else
  echo "Using old files (use fresh to get fresh copy from $dbctp)"
fi
cp /tmp/validate/pardefs/$1.partition $dbctp/../pardefs/val4act.partition
dbctp=/tmp/validate/DB
validate val4act
