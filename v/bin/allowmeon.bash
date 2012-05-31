#!/bin/bash
if [ $# -ne 2 ] ;then
  echo allowmeon logname host
  exit
fi
ln=$1
hn=$2
if [ -f ~/.ssh/id_rsa.pub ] ;then
cat ~/.ssh/id_rsa.pub |ssh $ln@$hn 'cat - >>~/.ssh/authorized_keys'
elif [ -f ~/.ssh/id_dsa.pub ] ;then
cat ~/.ssh/id_dsa.pub |ssh $ln@$hn 'cat - >>~/.ssh/authorized_keys'
else
echo
echo ".ssh/id_?sa.pub ?"
fi
