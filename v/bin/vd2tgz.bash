#!/bin/bash
#
hname=`hostname`
if [ "$hname" = 'alidcscom026' ] ;then
  BCKP=archive/vbin.tgz
  vd='v' ; rf2ttc='rf2ttc'
  echo "copying $VMECFDIR/../../v,rf2ttc to $BCKP..."
elif [ "$hname" = 'pcalicebhm10' ] ;then
  BCKP=~/vdbin.tgz
  vd='vd' ; rf2ttc=''
  echo "copying $VMECFDIR/../../vd to $BCKP..."
else
  echo "$vd $rf2ttc -> $BCKP"
  echo "can be started only form alidcscom026 or pcalicebhm05"
  exit
fi
# tar -ztvf ~/vdbin.tgz |sort -n -k 3 -r |head
cd $VMECFDIR/../..
tar '--exclude=*.exe' '--exclude=*.o' '--exclude=*.pyc' '--exclude=*~' \
 '--exclude=*.a' '--exclude=*.dump' '--exclude=*.rbf' '--exclude=*.sof' \
 '--exclude=*.tgz' '--exclude=*.tar' '--exclude=*.bckp' '--exclude=*ARXIV*' \
 '--exclude=*.swp' '--exclude=*.swo' '--exclude=*.cnt' '--exclude=*linux/*' \
 '--exclude=vme/backup/*' '--exclude=vme/WORK/*' \
 "--exclude=$vd/vme/monscal++/MONSCAL/*" \
 "--exclude=$vd/vme/monalisa/*" \
 '--exclude=vme/monscal++/logs/*' '--exclude=*.svn*' '--exclude=*.ttf' \
 '--exclude=*.nfs*' '--exclude=vme/ctp++/*' \
 -zcf $BCKP $vd $rf2ttc
ls -l $BCKP
if [ "$hname" = 'pcalicebhm05' ] ;then
  scp $BCKP jusko1@lxplus:
  #scp $BCKP trigger@altri1:
fi

