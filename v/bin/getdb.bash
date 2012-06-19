#!/bin/bash
cd ~/act/db
if [ $# -eq 0 ] ;then
  cp $dbctp/ttcparts.cfg .
  cp $dbctp/VALID.CTPINPUTS .
  cp $dbctp/CTP.SWITCH .
  cp $dbctp/L0.INPUTS .
  cp $dbctp/VALID.DESCRIPTORS .
  cp $dbctp/ctp.cfg .
  cp $dbctp/TRIGGER.PFS .
  cp $dbctp/VALID.BCMASKS .
  cp $dbctp/LTU.SWITCH .
  cp $dbctp/VALID.LTUS .
  cp $dbctp/ttcparts.cfg .
  cp $dbctp/gcalib.cfg .
  cp $dbctp/filter .
  cp $dbctp/clockshift .
else
  cp $dbctp/$1 .
fi
