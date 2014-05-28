#!/bin/bash
#This has to be started on alidcscom026
#
#echo "nothing done (actions commented out)"
allvmes="alidcsvme002 alidcsvme003 alidcsvme004 alidcsvme005 alidcsvme006 alidcsvme007"
sdir=/data/ClientCommonRootFs/usr/local/trigger/v/vme/ltu_proxy/noclasses
seqfiles="sod eod L2a"
for hn in $allvmes ;do
  [ "$hn" == "alidcsvme002" ] && dets="ssd fmd t0"
  [ "$hn" == "alidcsvme003" ] && dets="hmpid phos cpv"
  [ "$hn" == "alidcsvme004" ] && dets="trd zdc emcal"
  [ "$hn" == "alidcsvme005" ] && dets="tpc pmd acorde"
  [ "$hn" == "alidcsvme006" ] && dets="sdd muon_trk muon_trg daq"
  [ "$hn" == "alidcsvme007" ] && dets="spd tof v0"
for detname in $dets ;do
  # copy ltuttc.cfg 188->835
  if   [ $detname == "muon_trk" ] ;then
    echo "skipping muon_trk"
    continue
  fi
  from188=/data/dl/snapshot/$hn/home/alice/trigger/v/$detname/CFG/ltu/ltuttc.cfg
  to835=/home/dl6/snapshot/$hn/home/alice/trigger/v/$detname/CFG/ltu/ltuttc.cfg
  echo $hn:$detname
  mkdir -p /home/dl6/snapshot/$hn/home/alice/trigger/v/$detname/CFG/ltu
  scp -p trigger@alidcscom188:$from188 $to835
  # add l0only.slm to everybody:
  #ddir=/data/dl/snapshot/$hn/home/alice/trigger/v/$detname/CFG/ltu/SLMproxy
  #echo $ddir  
  #cd $ddir
  #cp ~/aj/l0only.slm ../SLM/
  #ln -s /usr/local/trigger/v/vme/ltu_proxy/noclasses/l0only.seq l0only.seq
#
#  cd $ddir
#  echo "---------------------- $detname"
#for fn in $seqfiles ; do
#  #[ -e $fn.seq ] && mv $fn.seq $fn_old.seq
#  if [ -e "$fn.seq" ]; then
#    #mv $fn.seq "$fn"_old.seq
#    echo $fn exists, copied to "$fn" _old
#  fi
#  echo cp $sdir/$fn.seq .
#  #cp $sdir/$fn.seq .
#done
#  ls -l sod.seq eod.seq L2a.seq
done
done

