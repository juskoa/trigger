#!/bin/tcsh
#
if ( $#argv >= 1 ) then
    set ans = $1
    if ( $ans == 'help' ) then
	echo "Usage:   tarsend.csh [help|noweb|web] [nosend]"
	echo " "
	echo "help     = to see this help message"
	echo "noweb    = do not update sources on the web page"
	echo "web      = store the sources on the web TTCit web page"
	echo " "
	echo "nosend   = anything given as 2-nd arg: tar but dont send"
	exit(0)
    else
       set noke = 'n'
    endif
else
    set ans = 'none'
    set noke = 'y'
endif
#
if ( $#argv >= 2 ) then
    set nosend = 'n'
else
    set nosend = 'y'
endif
#
set curdir = `pwd`
set wkdir = "${curdir}/.."
cd ${wkdir}
pwd
#
if ( -e ttcit_wk.tar ) then
    echo "ttcit_wk.tar exists - removing"
    /bin/rm -f ttcit_wk.tar
endif
#
set dt = `date +%d%b%H%M%S`
#
tar -cvf ttcit_${dt}.tar ttcit/*.c ttcit/*.h ttcit/ttcit_cf ttcit/ttcit_u.py \
ttcit/*.csh
# echo "PASSWORD for trigger@altrip2.cern.ch"
if ( $nosend == 'y' ) then
    echo "Sending file to kralik@lxplus.cern.ch:~/alice/trigger/."
    scp ttcit_${dt}.tar kralik@lxplus.cern.ch:~/alice/trigger/.
else
    echo "Tar file created, sending not requested"
    set noke = 'y'
endif
#
# Updating the manual/installation web page
#
if ( $noke == 'y' ) then
   exit(0)
endif
#
if ( $ans != 'noweb' ) then
    echo "Sending file to kralik@home.saske.sk:~/WWW/TTCit/code/."
    scp ttcit_${dt}.tar kralik@home.saske.sk:~/WWW/TTCit/code/.
endif
#
cd ${curdir}
#
exit(0)
#
