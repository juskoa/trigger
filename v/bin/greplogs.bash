#!/bin/bash
exit
cd /data/ClientLocalRootFs
ltus="SPD V0"
allvmes='alidcsvme002 alidcsvme003 alidcsvme004 alidcsvme005 alidcsvme006 alidcsvme007'
for hn in $allvmes ;do
  fn=`ls -t1 $hn/home/alice/trigger/v/*/WORK/LTU-*.log |head -2`
  echo $fn
done
