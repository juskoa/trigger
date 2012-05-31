#!/bin/bash
inp=$1
echo $inp
if [ inp == 'help' ] ;then
cat - <<-EOF
 link partition
EOF
exit
elif [ $1 ] ;then
echo linking : $inp.partition
ln -f $inp.partition ALICE.partition
ln -f $inp.pcfg ALICE.pcfg
else
echo NO parameter, try link help
fi
