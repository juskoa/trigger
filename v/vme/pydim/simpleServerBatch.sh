#!/bin/bash
# hmpid phos excluded (provided by real LTUs)
s_dnames="trd zdc emcal tpc pmd acorde sdd muon_trk muon_trg daq ssd fmd t0 cpv ad spd tof v0"
#s_dnames="trd zdc emcal pmd sdd muon_trk muon_trg daq ssd fmd t0 hmpid phos cpv as spd tof v0"
#s_dnames="t0 v0 cpv"
# killall python
for dn in $s_dnames ;do
  #sleep $((RANDOM % MAXWAIT))   MAXWAIT: in seconds
  fr=$((RANDOM%100))
  secsf=$(echo "scale=2; 5/$fr" | bc)
  sleep $secsf
  nohup ./simpleServer.py $dn >/dev/null &
  echo $dn
done
