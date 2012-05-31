#!/bin/bash
# to be started from triad@altri1
anv='-av'       # -anv = dry
rem=trigger@pcalicebhm05:/data/ClientCommonRootFs/usr/local/trigger/v/vme
cd $VMECFDIR
rsync $anv ctp ctp_proxy ctpcnts ltu ltu_proxy $rem
rsync $anv VME2FPGA fanio SSMANA dimcoff MORELTUS ltudim $rem
rsync $anv CFG/ctp/DB/VALID.CTPINPUTS  $rem
rem=trigger@pcalicebhm05:/data/ClientCommonRootFs/usr/local/trigger
cd ../..
rsync $anv v/vmeb  $rem

