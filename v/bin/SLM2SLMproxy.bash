#!/bin/bash
#Start this script in v/XXX/CFG/ltu/SLM directory on alidcsvme???. 
# SLM2SLMproxy.bash name
#detn=$1
#cfgfile=$VMECFDIR/CFG/ctp/DB/ttcparts.cfg
#hn=`awk '{if($1==detname) {print $2}}' detname=$detn $cfgfile`
if [ $# -ne 1 ] ;then
  echo Start this script in v/XXX/CFG/ltu/SLM directory on alidcsvme???. 
  echo SLM2SLMproxy.bash name
  echo where name is name of name.slm file
  exit
fi
d=`basename $PWD`
if [ "$d" != "SLM" ] ;then
  echo "cd to ...SLM directory first..."
  exit
fi
name=$1
slmname=$name.slm
cd ../../..
#export VMEWORKDIR=`pwd`
cp CFG/ltu/SLM/$slmname WORK/
$VMECFDIR/ltu/slmcmp.py WORK/$slmname
#
cp WORK/$name.slm CFG/ltu/SLMproxy/
cp WORK/slmseq.seq CFG/ltu/SLMproxy/$name.seq
echo "ls -lt CFG/ltu/SLMproxy |head -3"
ls -lt CFG/ltu/SLMproxy |head -3
echo "$name.slm:"
cat WORK/$name.slm 
echo "---------------------------- $name.seq is:"
$VMECFDIR/ltu/slmcomp.py WORK/slmseq.seq
