#!/bin/bash
function dodir() {
mkdir $tdb
mkdir $tdb/CTP.SWITCH.ARXIV 
mkdir $tdb/L0.INPUTS.ARXIV 
mkdir $tdb/VALID.CTPINPUTS.ARXIV
}
function cpfiles() {
cd $dbctp
echo ; echo "Copying:"
echo "$dbctp -> $tdb"
cp -a L0.INPUTS CTP.SWITCH VALID.CTPINPUTS $tdb/
}
tdb=/tmp/switched
test ! -d /tmp/switched  && (dodir ; cpfiles)
if test "$1" = "fresh"  ;then
  cpfiles
else
  echo "Using old files (use fresh to get fresh copy from dbctp)"
fi
echo
echo "Now using $tdb directory..."
ls -l $tdb
dbctp=$tdb
cd $VMECFDIR/switchgui; ./switched.py
echo
echo "see $dbctp"

