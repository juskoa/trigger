#!/bin/bash
if [ $# -eq 0 ] ;then
  #find . -name '*.py' |xargs head -1
  find . -name '*.py' |xargs -l1 head -1
  cat - <<-EOF

l          -recursive list of all .py files
file.py    -futurize file.py
EOF
  exit
fi
if [ $1 = 'l' ] ;then
  find . -name '*.py' |xargs head -1
  exit
fi
fn=$1
futurize -w $fn
#sed "s/#!\/usr\/bin\/python/#!\/usr\/bin\/env python/" $fn >temp ; mv temp $fn
h_b=`head -1 $fn`
sed -i "s/#!\/usr\/bin\/python/#!\/usr\/bin\/env python/" $fn
h_a=`head -1 $fn`
echo
echo "$h_b  ->  $h_a"
grep string $fn
