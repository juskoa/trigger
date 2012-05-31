#export VMEDRIVER=VMERCC     # VMERCC, VMECCT, AIX, MSVISA, SIMVME
rdir=~/v
export VMEBDIR=$rdir/vmeb
export VMECFDIR=$rdir/vme
#export PYTHONPATH=~/v/vmeb
shopt -s expand_aliases
alias vmedirs='echo   VMEDRIVER:$VMEDRIVER; echo   VMEBDIR:$VMEBDIR;echo   VMECFDIR:$VMECFDIR'
alias vmecomp=$VMEBDIR/comp.py
alias vmecrate=$VMEBDIR/crate.py
alias "ctp=$VMEBDIR/crate.py ctp"
alias slmshow=$VMECFDIR/ltu/slmcomp.py
alias slmedit='cd $VMECFDIR/CFG/ltu/SLM;$VMECFDIR/ltu/ltu6'
alias initttc='$VMEBDIR/../scripts/ttcvi'
alias vmeshow='/local/trigger/ECS/vmeshow'
. setdsenv
#echo "LD_LIBRARY_PATH:$LD_LIBRARY_PATH"
#echo 'useful aliases: vmedirs, vmecomp, vmecrate, slmshow, slmedit'
#vmedirs

