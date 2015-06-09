#!/bin/tcsh
#
# Usage:     
#
#   ttcit_upgrade.csh [TTCit_code_version] [dir]
#
# [TTCit_code_version]   = version of the TTCit FPGA code
#                          The file is to be named "TTCit_v${version}.ttf"
#
#                       (default directories are : .
#                                                  $VMECFDIR/ttcit
#
# [dir]               = alternative directory, where the FPGA code is stored
#                       (optional)
#
set ttcit = $VMECFDIR/ttcit/ttcit.exe
#
if ( $#argv < 1 ) then
    echo "Missing argument: You must specify the file with the FPGA code"
    echo " "
    echo "ttcit_upgrage.csh [code_version] [dir]"
    echo "[code_version] = versio of the TTCit code (i.e. 15)"
    echo "[dir]          = alternative directory where to look for the code"
    exit(0)
endif
#
set fcv = $1
set fc = "TTCit_v${fcv}.ttf"
#if ( -e ttcit_fpga_code ) then
#    /bin/rm -f ttcit_fpga_code
#endif
#
if ( $#argv >= 2 ) then
    set fd = $2
else
    set fd = "."
endif
#
if ( -e ./${fc} ) then
    #ln -s ${fc} ttcit_fpga_code
    echo "Using ${fc} from ."
else if ( -e $VMECFDIR/CFG/ttcit/${fc} ) then
    ln -s $VMECFDIR/CFG/ttcit/${fc} ttcit_fpga_code
    echo "Using ${fc} from $VMECFDIR/CFG/ttcit"
else if ( -e ${fd}/${fc} ) then
    ln -s ${fd}/${fc} ttcit_fpga_code    
    echo "Using ${fc} from ${fd}"
else
    echo "Cannot find ${fc}   --> EXITING"
    exit
endif
#
#  Create a short macro for the ttcit code
#
${ttcit} <<EOF
WriteCodeFM()
q
EOF
#
if ( -e ttcit_fpga_code ) then
    /bin/rm -f ttcit_fpga_code
endif
exit(0)
#
