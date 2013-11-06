#!/bin/bash
# rsync -e ssh -aL trigger@altri1:v/vme/CFG/l0/l0.rbf
function rsyncmd() {
return
if [ `hostname` = 'altri1' -o `hostname` = 'altri1.cern.ch' ] ;then
  RSCMD=""
  echo "altri1 is master, rsync not necessary"
else
  RSCMD="rsync -e ssh -avL trigger@altri1:v/vme/CFG/$1/$1.rbf ."
  CFGDIR="v/vme/CFG/$1"
fi
#RSCMD="ssh altri1 ls -l v/vme/CFG/$1"
}

cd ~trigger/v/vme
RSCMD=""
if [ $# -eq 2 ] ;then
  CTPBASE=$2
else
  cat - <<-EOF

Usage:
loadCTPfpga configure BASE  -Flash memory -> CTP FPGA i.e.
                             (Flash mem. not modified)
loadCTPfpga load BASE       -load v/vme/CFG/board/board.rbf and configure 

   where BASE is:
     the name of CTP board or
     its base address (e.g. 0x829000 for L0)
   busy 0x828000
   l0   0x829000
   l1   0x82a000
   l2   0x82b000
   int  0x82c000
   fo1-6 0x821000-0x826000

Use name to force update of .rbf file from master computer (trigger@altri1)

Now, these links are active:
EOF
cd $VMECFDIR/CFG
ls -l busy/busy.rbf l0/l0.rbf l1/l1.rbf l2/l2.rbf fo/fo.rbf int/int.rbf
exit
fi
if [ "$CTPBASE" = 'busy' ] ;then
  CTPBASE="0x828000"
  rsyncmd busy
fi
if [ "$CTPBASE" = 'l0' ] ;then
  CTPBASE="0x829000"
  rsyncmd l0
fi
if [ "$CTPBASE" = 'l1' ] ;then
  CTPBASE="0x82a000"
  rsyncmd l1
fi
if [ "$CTPBASE" = 'l2' ] ;then
  CTPBASE="0x82b000"
  rsyncmd l2
fi
if [ "$CTPBASE" = 'int' ] ;then
  CTPBASE="0x82c000"
  rsyncmd int
fi
if [ "$CTPBASE" = 'fo0' ] ;then
  echo "fo0 DISCOURAGED!. Only fo1-fo6 should be used (i.e. 0x821...)"
  echo
  exit
  CTPBASE="0x820000"
  rsyncmd fo
fi
if [ "$CTPBASE" = 'fo1' ] ;then
  CTPBASE="0x821000"
  rsyncmd fo
fi
if [ "$CTPBASE" = 'fo2' ] ;then
  CTPBASE="0x822000"
  rsyncmd fo
fi
if [ "$CTPBASE" = 'fo3' ] ;then
  CTPBASE="0x823000"
  rsyncmd fo
fi
if [ "$CTPBASE" = 'fo4' ] ;then
  CTPBASE="0x824000"
  rsyncmd fo
fi
if [ "$CTPBASE" = 'fo5' ] ;then
  CTPBASE="0x825000"
  rsyncmd fo
fi
if [ "$CTPBASE" = 'fo6' ] ;then
  CTPBASE="0x826000"
  rsyncmd fo
fi
echo "CTP board base address: $CTPBASE"
#exit
if [ "$1" = 'configure' ] ;then
$VMECFDIR/VME2FPGA/VME2FPGA.exe $CTPBASE -noinitmac <<-EOF
LoadFPGA()
q
EOF
exit
fi
if [ "$1" = 'load' ] ;then
#if [ "$RSCMD" ] ;then
#  echo "   $RSCMD ---> $CFGDIR"
#  cd ~/$CFGDIR
#  $RSCMD
#fi
cd ~/v/vme
$VMECFDIR/VME2FPGA/VME2FPGA.exe $CTPBASE -noinitmac <<-EOF
EraseFM()
LoadFM()
LoadFPGA()
q
EOF
else
  echo "only load or configure supported"
  exit
fi

