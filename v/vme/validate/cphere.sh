#!/bin/bash
echo $0
if [ $0 != "./cphere.sh" ] ;then
  echo "Start: ./cphere"
  exit
fi
mkdir ~/validate ; cp MANIFEST setup.py ~/validate/ ; cd ~/validate
p1="$VMEBDIR/myw.py $VMEBDIR/trigdb.py $VMEBDIR/txtproc.py $VMEBDIR/downscaling.py"
p2="$VMECFDIR/TRG_DBED/parted.py $VMECFDIR/TRG_DBED/validate.py"
pyfiles="myw.py trigdb.py txtproc.py parted.py validate.py downscaling.py"
#echo $pyfiles
cp $p1 ./
cp $p2 ./
#echo 'Copy of these files made to local directory:'
#ls -l *.py
#echo 'python setup.py sdist   (bdist_rpm creates .rpm distribution)'
#see readme for explanation of next line:
#echo "%_unpackaged_files_terminate_build 0" >> ~/.rpmmacros
echo "%_unpackaged_files_terminate_build 0" > ~/.rpmmacros
read -p "pwd:`pwd` continue? (y/n):" yn
python setup.py bdist_rpm 
echo "pwd:`pwd` Do:"
#echo 'scp dist/validate-1.0.tar.gz $aj11:'
echo 'scp ~/validate/dist/validate-2.1-1.noarch.rpm $lxp:t/'
read -p "remove $pyfiles in pwd:`pwd` ? (y/n):" yn
if [ $yn = 'y' ] ;then
  rm $pyfiles
fi

