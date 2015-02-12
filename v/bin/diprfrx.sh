#!/bin/bash
# $1: start stop or status
hname=`hostname -s`
if [ "$hname" != 'alidcsvme017' ] ;then
  echo 'This script can be started only on alidcsvme017'
  exit
fi
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/dip/lib
sss=$1
#[alidcscom026] /data/ClientCommonRootFs/usr/local/trigger/rf2ttc/page1_server> 
#make dipTest/dipPubTest
#
cd ~/v/vme/WORK
# out to nohup.out:
#nohup $VMECFDIR/../../rf2ttc/page1_server/dipAlice/dipPubTest & 2>&1>/dev/null
# out to /dev/null
#nohup $VMECFDIR/../../rf2ttc/page1_server/dipAlice/dipPubTest 2>&1>/dev/null &
#pgrep -l dipPubTest
daemoncom.sh diprfrx ../../../rf2ttc/page1_server/dipAlice/dipPubTest log $sss
#
#cd $VMECFDIR/../../rf2ttc/page1_server
#dipAlice/dipPubTest
# logs are in ~/diplogs
# dipBrowser on 26:
#export CLASSPATH=/opt/dip/lib/dip.jar
#[alidcscom026] /data/ClientCommonRootFs/usr/local/trigger/rf2ttc/page1_server > java -jar /opt/dip/tools/dipBrowser.jar
