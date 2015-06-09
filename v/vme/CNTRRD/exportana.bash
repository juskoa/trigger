#!/bin/bash
dedi=$HOME/analyse ; mkdir -p $dedi
echo copying files...
cp -a common.[ch] analyse.c analyse.make $VMECFDIR/ctp/ctpcounters.h $VMECFDIR/ctp/ctpcounters_run1.h $dedi/
cd $dedi
make -f analyse.make
