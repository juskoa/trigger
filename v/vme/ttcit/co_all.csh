#!/bin/tcsh
#
set files = `cat ttcit_cf`
#
if ( $#argv >= 1 ) then
    set branch = $1
    if ( $branch == 'help' ) then
	echo "Usage:    co_all.csh [rev]"
	echo " "
	echo "[rev]  is an optional argument and may be:"
	echo "       {a number}   - set get that branch"
	echo "                      (to see a list of branches use "
	echo "                       rlob -b ttcit.c  )"
	echo " "
	set branch = 'none'
	exit(0)
    else
	echo "Requested branch to check out is ${branch}"
    endif
else
    set branch = ''
endif
#
foreach f ( ${files} )
    echo "Checking out : ${f}"
    co -l${branch} $f
end
#
