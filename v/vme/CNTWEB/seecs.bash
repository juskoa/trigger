#!/bin/bash
function gr() {
nn=`grep $1 $fs.alice |grep -v '*' |wc -l`
}
fs=$1 ; line=""
btyps='B A C E'
for btyp in B A C E ;do
  gr $btyp
  echo $btyp:$nn
  line=$line" $btyp:$nn"
done
echo $line
