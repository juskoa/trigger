#!/bin/bash
if [ $# = 1 ] ;then
 echo "config file" $1 "is being processed"
 if [ -d  pdf ] ; then
    rm -r pdf > /dev/null
 fi
 mkdir pdf
 cp $1.txt pdf/.
 /home/alice/trigger/rl/source/extract $1 > pdf/out.log
 if [ -d pdf$1 ] ;then 
  echo "Results are in directory pdf"
  exit 1
 fi
 mv pdf pdf$1
 echo "results are in directory pdf$1"
else
 echo "One parameter (config file name) expected."
fi
