#!/bin/bash
#This has to be started on alidcscom026
#
echo "nothing done (actions commented out)"
#allvmes="alidcsvme001 alidcsvme002 alidcsvme003 alidcsvme004 alidcsvme005 alidcsvme006 alidcsvme007 alidcsvme008 alidcsvme017 alidcsvme052"
allvmes="alidcsvme003 alidcsvme004 alidcsvme005 alidcsvme006 alidcsvme007 alidcsvme008"
#CLRFS=/data/dl/snapshot  or /home/dl6/snapshot (see setenv)
cd
for hn in $allvmes ;do
 cd $CLRFS/$hn/home/alice/trigger/v
 pwd
 #cat .ssh/id_rsa.pub >>$CLRFS/$hn/home/alice/trigger/.ssh/authorized_keys
  #mv vme vmeDELME
done

