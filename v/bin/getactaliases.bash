#!/bin/bash
# this script is called form tri@alitri to download aliases.txt file from ACT
export dbctp='.'
if [ ! -d CFG/ctp/DB ] ;then
mkdir -p CFG/ctp/DB
fi
actexe=$VMECFDIR/ctp_proxy/linux/act.exe
VMECFDIR='.'
$actexe aliases.txt
