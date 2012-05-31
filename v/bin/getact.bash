#!/bin/bash
if [ $# -gt 0 ] ;then
  # PHYSICS_
  fname="$1"
  ph=${fname:0:8}
  tst=${fname:0:5}
  if [ "$ph" = "PHYSICS_" -o "$tst" = "TEST_" ] ;then
    fpat="$dbctp/../pardefs/$1.partition"
  else
    fpat="$dbctp/$1"
  fi
  cd $VMECFDIR/ctp_proxy
  #ls -l $dbctp/../pardefs/$1.partition
  ls -l $fpat
  linux/act.exe $1 $2
  #ls -l $fpat    let's leave rc from linux/act.exe returned (-2->254)
else
cat - <<-EOF
part. name PHYSICS_. expected or config file name
Operation: 
download partition definition from ACT into $dbctp/../pardefs/  or
download config file into $dbctp/
EOF
fi

