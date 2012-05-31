#!/bin/bash
function ipcremove() {
dtn=$1
ltubase=`awk '{if($1==detname) {print $3}}' detname=$dtn $cfgfile`
hn=`awk '{if($1==detname) {print $2}}' detname=$dtn $cfgfile`
if [ "$hn" = "$HOSTNAME"  -a -n "$ltubase" ] ;then
  key=${ltubase/0x/0x00}
  shmid=`ipcs -m |awk '{if($1==key) {print $2}}' key=$key`
  if [ -z "$shmid" ] ;then
    echo "Error: shmid not found for $dtn base:$ltubase key:$key"
  else
    ipcrm shm $shmid
    echo "ipcrm shm $shmid"
  fi
else
  echo "$dtn: not known or not on this machine (possible cpu: $hn)"
fi
}
if [ $# -eq 0 ] ;then
cat - <<-EOF
ltushmrm DETNAME
ltushmrm all
ipcs -m        ipcrm shm shmid
EOF
else
dtn=$1
cfgfile=$VMECFDIR/CFG/ctp/DB/ttcparts.cfg
ipcs -m
if [ $dtn = 'all' ] ;then
  echo "removing all shms..."
  for dtn1 in `awk '{if($2==host) {print $1}}' host=$HOSTNAME $cfgfile` ;do
    ipcremove $dtn1
  done
else
  echo "removing $dtn shm..."
  ipcremove $dtn
fi
fi
