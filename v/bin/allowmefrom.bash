#!/bin/bash
if [ $# -ne 2 ] ;then
  echo allowmefrom logname host
  exit
fi
ln=$1
hn=$2
ssh $ln@$hn 'cat ~/.ssh/id_rsa.pub' >>~/.ssh/authorized_keys
echo "rc:$?"

