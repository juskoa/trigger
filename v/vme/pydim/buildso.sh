#!/bin/bash
nam=$1
if [ "$nam" = '' ] ;then
  cat - <<-EOF
  Usage: ./buildso.sh extension_name
  using: clientpy...
EOF
  nam='clientpy'
fi
# create $nam_wrap.c, $nam.py:
swig -python $nam.i
if [ $? -ne 0 ] ;then
  echo "swig rc:$?"
  exit
fi
# create $nam_wrap.o
gcc -fPIC -c $nam.c ${nam}_wrap.c -I/usr/include/python2.3 \
 -I/opt/dim/dim
# create _$nam.so
#ld -shared --export-dynamic -rpath /opt/dim/linux $nam.o ${nam}_wrap.o -o _$nam.so
#ld -shared $nam.o ${nam}_wrap.o -o _$nam.so
ld -shared --export-dynamic $nam.o ${nam}_wrap.o /opt/dim/linux/libdim.so -o _$nam.so

