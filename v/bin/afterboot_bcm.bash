#!/bin/bash
# configure LTU board (alidcsvme003) and TTCit board (alidcsvme056)
export HOME=/home/alice/trigger
. /usr/local/trigger/bin/vmebse.bash
hn=`hostname`
if [ "$hn" = "alidcsvme056" ] ;then
$VMECFDIR/ttcit/ttcit.exe <<-EOF
ScopeSelect_AB(1,20)
ScopeSelect_AB(1,3)
q
EOF
$VMECFDIR/rfrx/rfrx.exe <<-EOF2
q
EOF2
elif [ "$hn" = "alidcsvme003" ] ;then
  $VMECFDIR/ltu/ltu.exe 0x813000 <<-EOF
vmeopw32(STDALONE_MODE, 0x3)
q
EOF
else
  echo "Not allowed on machine $hn"
fi

