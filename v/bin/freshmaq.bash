#!/bin/sh
# staretd on 707:
#nfiles=1
#cd ~/SMAQProject/gui/archive
#ff=`ls -t *.pdf| head -$nfiles`
#cat $ff
# staretd on aldaqacr07 (is in trg@aldaqacr07:bin/getsmaq):
sm=trigger@alidcscom707
fn=`ssh $sm 'cd ~/SMAQProject/gui/archive;ls -t *.pdf| head -1'`
#echo $fn
cd ~/pdf
echo 707:$fn ---\> pdf/$fn
scp -p $sm:SMAQProject/gui/archive/$fn .
