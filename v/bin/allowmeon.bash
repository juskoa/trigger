#!/bin/bash
if [ $# -lt 2 ] ;then
  echo allowmeon logname host [port]
  exit
fi
ln=$1
hn=$2
port=""
if [ $# -eq 3 ] ;then
  port="-p $3"
fi
if [ -f ~/.ssh/id_rsa.pub ] ;then
cat ~/.ssh/id_rsa.pub |ssh $port $ln@$hn 'cat - >>~/.ssh/authorized_keys'
elif [ -f ~/.ssh/id_dsa.pub ] ;then
cat ~/.ssh/id_dsa.pub |ssh $port $ln@$hn 'cat - >>~/.ssh/authorized_keys'
else
echo
echo ".ssh/id_?sa.pub ?"
fi
