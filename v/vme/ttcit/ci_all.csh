#!/bin/tcsh
#
set files = `cat ttcit_cf`
#
set branch = 'none'
if ( $#argv >= 1 ) then
    set branch = $1
    if( $branch == 'help' ) then
	echo "Usage:      ci_all.csh [rev] [nolock]"
	echo " "
	echo "[rev] is optional argument and may be the following"
	echo " "
	echo "           help        - to see this help"
	echo "           an integer  - to set a new branch. Before setting a "
	echo "                         new branch check the highest branch by"
	echo "                         rlog -b ttcit.c"
	echo "[nolock]   anything here means the no co -l afteward is done" 
	echo " "
	set branch = 'none'
	exit(0)
    else
	echo "Requested branch change to ${branch}"
    endif
else
    set branch = 'none'
endif
if ( $#argv >= 2 ) then
    set nolock = 1
else
    set nolock = 0
endif
#
foreach f ( ${files} )
#
    echo "Checking in : ${f}"
#
    if ( $branch == 'none') then
	if ( $nolock == '0' ) then
	    ci -l ${f}
	else
	    ci ${f}
	endif
    else 
	echo "Setting a new branch: ${branch}"
	ci -f${branch} ${f}
	if ( $nolock == '0' ) then
	    co -l ${f}
	endif
    endif
end
#
ci -l ttcit_cf
#
