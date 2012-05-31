#!/bin/bash
function sbun() {
Xall=`grep $1 $csn |wc -l`
X=`grep -v '*' $csn | grep " $1" |wc -l`
result="$1: $Xall/$X"
}
#csn="$dbctp/COLLISIONS.SCHEDULE"
csn=$1.alice
head -1 $csn
if [ "$2" == "brief" -o $# -eq 1 ] ;then
  #Aall=`grep 'A' $csn |wc -l`
  #A=`grep -v '*' $csn | grep 'A' |wc -l`
  #echo "A: $Aall/$A"
  sbun 'B' ; B=$result
  sbun 'A' ; A=$result
  sbun 'C' ; C=$result
  sbun 'E' ; E=$result
  echo "$B     $A     $C     $E"
exit
fi
if [ "$2" != 'B' -a "$2" != 'A' -a "$2" != 'C' -a "$2" != 'E' ] ;then
  cat - <<-EOF
Filling scheme name and
brief         or
B, A, C or E expected with optional 3rd all parameter
EOF
else
if [ "$3" == 'all' ] ;then
  grep " $2" $csn
else
  #grep "\" $2\"" $csn | grep -v '*'
  grep " $2" $csn | grep -v '*'
fi
fi
