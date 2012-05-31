#!/bin/bash
if [ $# -ne 1 ] ;then
  echo go to v/XXX/CFG/ltu/SLMproxy direrectory
  echo before starting this script with any parameter
  echo It creates links:
  echo 'eod.seq -> /usr/local/trigger/v/vme/ltu_proxy/noclasses/eod.seq'
  echo 'sod.seq ->...'
  echo 'L2a.seq ->...'
  exit
fi
ln -sf /usr/local/trigger/v/vme/ltu_proxy/noclasses/eod.seq eod.seq
ln -sf /usr/local/trigger/v/vme/ltu_proxy/noclasses/sod.seq sod.seq
ln -sf /usr/local/trigger/v/vme/ltu_proxy/noclasses/L2a.seq L2a.seq
