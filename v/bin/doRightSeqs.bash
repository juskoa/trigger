#!/bin/bash
#This has to be started on alitri
#
echo "nothing done (actions commented out)"
allvmes="alidcsvme002 alidcsvme003 alidcsvme004 alidcsvme005 alidcsvme006 alidcsvme007"
sdir=/data/dl/root/usr/local/trigger/v/vme/ltu_proxy/noclasses
#seqfiles="sod eod L2a"
seqfiles="sync"
for hn in $allvmes ;do
  [ "$hn" == "alidcsvme002" ] && dets="ssd fmd t0"
  [ "$hn" == "alidcsvme003" ] && dets="hmpid phos cpv"
  [ "$hn" == "alidcsvme004" ] && dets="trd zdc emc"
  [ "$hn" == "alidcsvme005" ] && dets="tpc pmd acorde"
  [ "$hn" == "alidcsvme006" ] && dets="sdd muon_trk muon_trg daq"
  [ "$hn" == "alidcsvme007" ] && dets="spd tof v0"
for detname in $dets ;do
  #ddir=/data/ClientLocalRootFs/$hn/home/alice/trigger/v/$detname/CFG/ltu/SLM
  ddir=/data/dl/snapshot/$hn/home/alice/trigger/v/$detname/CFG/ltu/SLM
  cd $ddir
  echo "---------------------- $detname"
for fn in $seqfiles ; do
  #[ -e $fn.seq ] && mv $fn.seq $fn_old.seq
  if [ -e "$fn.seq" ]; then
    #mv $fn.seq "$fn"_old.seq
    #echo $fn exists, copied to "$fn" _old
    echo $fn.seq exists, overwritten
  fi
  echo cp $sdir/$fn.seq/slm .
  cp $sdir/$fn.seq .
  cp $sdir/$fn.slm .
done
  #ls -l sod.seq eod.seq L2a.seq
  ls -l sync.s*
done
done

