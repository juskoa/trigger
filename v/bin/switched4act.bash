#!/bin/bash
tdb=/tmp/switched
test ! -d /tmp/switched  && mkdir $tdb
cd $dbctp
cp -a L0.INPUTS CTP.SWITCH VALID.CTPINPUTS $tdb/
echo
echo "Now using $tdb directory..."
ls -l $tdb
dbctp=$tdb
cd /data/dl/root/usr/local/trigger/v/vme/switchgui; ./switched.py
echo
echo "see $dbctp"

