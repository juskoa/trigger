#!/bin/bash
# start as root:
allvmes='alidcsvme001 alidcsvme002 alidcsvme003 alidcsvme004 alidcsvme005 alidcsvme006 alidcsvme007 alidcsvme008 alidcsvme017'
cd /data/dl/snapshot
for hn in $allvmes ;do
 tar -zcf - $hn | ssh trigger@alidcscom027 "dd of=/b3/2011/snapshot/$hn.tgz"
done
# start as trigger:
cd ~trigger/CNTRRD
hn=norawcnts
tar -zcf - logs htmls rrd | ssh trigger@alidcscom027 "dd of=/b3/2011/CNTRRD/$hn.tgz"
#nebavi:
#tar '--exclude rawcnts/*' -zcf - logs htmls rrd | ssh trigger@alidcscom027 "dd of=/b3/2011/CNTRRD/$hn.tgz"
hn=rawcnts
tar -zcf - $hn | ssh trigger@alidcscom027 "dd of=/b3/2011/CNTRRD/$hn.tgz"
cd ~trigger/v/vme
hn=WORK
tar -zcf - $hn | ssh trigger@alidcscom027 "dd of=/b3/2011/$hn.tgz"
cd $VMECFDIR
hn=CFG
tar -zcf - $hn | ssh trigger@alidcscom027 "dd of=/b3/2011/$hn.tgz"


