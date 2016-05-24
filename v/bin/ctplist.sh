#!/bin/bash
lastn=$1
if [ -z "$1" ] ;then
  lastn=10
fi
ls -lt ctp_proxy*.log |head -$lastn
