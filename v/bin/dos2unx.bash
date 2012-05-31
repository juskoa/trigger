#!/bin/bash
if [ $# -gt 0 ] ;then
  cd $dbctp/../pardefs
  # -q because we need INFO at the start of the line in pydim server
  dos2unix -q $1
else
cd $dbctp
dos2unix ctp.cfg
dos2unix L0.INPUTS
dos2unix CTP.SWITCH
dos2unix VALID.CTPINPUTS
dos2unix VALID.DESCRIPTORS
dos2unix VALID.BCMASKS
fi

