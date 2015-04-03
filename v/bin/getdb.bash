#!/bin/bash
#cd ~/act/db do not change wd (this script invoked from validate4act.bash,...)
if [ $# -eq 0 ] ;then
  cp $dbctp/ttcparts.cfg .
  cp $dbctp/ctpinputs.cfg .
  cp $dbctp/VALID.DESCRIPTORS .
  cp $dbctp/ctp.cfg .
  cp $dbctp/TRIGGER.PFS .
  cp $dbctp/VALID.BCMASKS .
  cp $dbctp/VALID.LTUS .
  cp $dbctp/ttcparts.cfg .
  cp $dbctp/gcalib.cfg .
  cp $dbctp/filter .
  cp $dbctp/clockshift .
else
  cp $dbctp/$1 .
fi
