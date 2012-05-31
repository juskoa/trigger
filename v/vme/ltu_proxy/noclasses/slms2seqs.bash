#!/bin/bash
#Start this script in ltu_proxy/XXX directory. 
#   XXX is directory, where sod/eod/L2a .seq files are kept
#   and used by ltu_proxy
cldir=`basename $PWD`
cd ../..
for name in sod eod L2a ;do
  ltu/slmcmp.py "ltu_proxy/$cldir/$name.slm"
  mv WORK/slmseq.seq "ltu_proxy/$cldir/$name.seq"
  echo "$name.seq is:"
  ltu/slmcomp.py "ltu_proxy/$cldir/$name.seq"
done
