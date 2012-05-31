#!/bin/bash
csn="$dbctp/fs/$1.alice"
CSLINK="COLLISIONS.SCHEDULE"
csn="$dbctp/COLLISIONS.SCHEDULE"
head -1 $csn
if [ "$1" != 'B' -a "$1" != 'A' -a "$1" != 'C' -a "$1" != 'E' ] ;then 
  echo "B, A, C or E expected with optional 2nd all parameter"
else
if [ "$2" == 'all' ] ;then
  grep $1 $csn
else
  grep $1 $csn | grep -v '*'
fi
fi
