#!/bin/sh
cd ~/IRS/DATA ; fn=`ls -t1|head -1`
grep 'INT1, 346 :' $fn
tail -800 $fn | grep -e 'INT1, 340 :' -e 'INT2, 340 :' 

