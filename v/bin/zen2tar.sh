#!/bin/sh
#[alitoftm00] /home/tof > ls -l SOFT/ltusw6* SOFT/ltu/ltusw6*
#-r--r--r-- 1 tof tof 0 Mar 18 13:44 SOFT/ltu/ltusw64bit
#-r--r--r-- 1 tof tof 0 Feb 25 14:58 SOFT/ltusw64bit
#
cd ~/SOFT/ltu
find . -newer ../ltusw64bit \( -name '*.[ch]' -o -name '*[Mm]ake*' -o -name '*eadm*' -o -name '*.py' -o -name '*.sh' -o -name '*.bash' \) >/tmp/tar.inp
#find . -newer ../ltusw64bit >/tmp/tarall.inp
tar -zcf ~/ltuv.tgz -T/tmp/tar.inp
