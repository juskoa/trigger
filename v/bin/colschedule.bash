#!/bin/bash
# 
if [ "$1" = "update" ] ;then
  csname=`head -1 $dbctp/VALID.BCMASKS |awk '{print $2}'`
  #csn="$dbctp/fs/$csname.alice"
  csn="fs/$csname.alice"
elif [ "$1" = "update_auto" ] ;then
  csname=`head -1 $dbctp/VALID.BCMASKS |awk '{print $2}'`
  #csn="$dbctp/fs_auto/$csname.alice"
  csn="fs_auto/$csname.alice"
else
  csname=$1
  #csn="$dbctp/fs/$csname.alice"
  csn="fs/$csname.alice"
fi
cd $dbctp
CSLINK="COLLISIONS.SCHEDULE"
if [ -e $csn ] ;then
  #echo "exists: $csname"
  #ln -sf fs/$csname.alice $CSLINK
  ln -sf $csn $CSLINK
  cd $VMECFDIR/pydim 
  linux/client CTPRCFG/RCFG csupdate
else                                              
  cat - <<-EOF
$dbctp/$csn 
does not exist, no action.
Usage:
colschedule.bash update             -called from ctpproxy.py
colschedule.bash update_auto        -called from getfsdip.py
Current schedule:
EOF
  ls -l $CSLINK
  echo "Latest schedules (man and auto):"
  ls -1t fs |head -4
  ls -1t fs_auto |head -4
  exit 9
fi
