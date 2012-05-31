#!/bin/bash
# ltu client not copied
# ltudim ltu/*.py ../vmeb/*.py
cd $VMECFDIR
#tar -zcf ~/vserver.tgz dimcdistrib CNTRRD TRG_DBED \
tar -zcf ~/vserver.tgz CNTRRD TRG_DBED \
 pydim/linux pydim/*.c pydim/*.py pydim/makefile \
 pydim/linux pydim/*.c pydim/*.py pydim/makefile \
 CFG/ctp/DB/VALID.LTUS CFG/ctp/DB/VALID.CTPINPUTS \
 CFG/ctp/DB/VALID.BCMASKS CFG/ctp/DB/VALID.DESCRIPTORS \
 CFG/ctp/DB/TRIGGER.PFS CFG/ctp/DB/ttcparts.cfg \
 CFG/ctp/DB/L0.INPUTS \
 CFG/ctp/DB/CTP.SWITCH CFG/ctp/DB/LTU.SWITCH \
 CFG/ctp/pardefs/ALICE.partition
cd ..
tar -zcf ~/vmeb.tgz vmeb/vmeblib/*.[ch] vmeb/vmeblib/makefile \
 vmeb/*.py vmeb/vmeai.make.SIMVME
cd ..
tar -zcf ~/vbin.tgz bin/startClients.bash bin/setdsenv bin/cp2server.bash
exit
#
# on server:
mkdir -p ~/v/vme/WORK/RCFG ; mkdir -p ~/v/vme/WORK/PCFG
. setdsenv
cd $VMEBDIR ; cd vmeblib ; make
cd $VMECFDIR/pydim ; make
cd ; mkdir -p CNTRRD/logs CNTRRD/rawcnts CNTRRD/rrd
cp -a $VMECFDIR/CNTRRD/htmls CNTRRD/
