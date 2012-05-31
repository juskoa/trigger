# this should be started (from bash): . ./dssetenv
# to be able to:
# - start smiTrans, dns, did, smiSM, smiGUI,...
# - compile user proxies (e.g. ltu directory)
export DIM_DNS_NODE=altri1
if [ "$DIM_DNS_NODE" = `hostname` ] ;then
  dimd=/opt/dim
else
  dimd=/opt/dim
fi
smid=/opt/smi
#
#for ltu makefile
export OS=Linux
export ODIR=linux
export DIMDIR=$dimd
export SMIDIR=$smid
#export PATH=$dimd/linux:$smid/linux:$PATH
# for dns, smiGUI:
export LD_LIBRARY_PATH=$dimd/linux:$smid/linux
#
# for LTUxx (has to be started without ./ from ltu directory)
export PATH=$PATH:.
alias TestServer=$DIMDIR/$ODIR/testServer
alias TestClient=$DIMDIR/$ODIR/testClient
alias Test_server=$DIMDIR/$ODIR/test_server
alias Test_client=$DIMDIR/$ODIR/test_client
alias Dns=$DIMDIR/$ODIR/dns
alias Dim_get_service=$DIMDIR/$ODIR/dim_get_service
alias Dim_send_command=$DIMDIR/$ODIR/dim_send_command
alias DimBridge=$DIMDIR/$ODIR/DimBridge
alias Did=$DIMDIR/$ODIR/did

alias smiTrans=$SMIDIR/$ODIR/smiTrans
alias smiGen=$SMIDIR/$ODIR/smiGen
alias smiSM=$SMIDIR/$ODIR/smiSM
alias smiSendCommand=$SMIDIR/$ODIR/smiSendCommand
alias smiPreproc=$SMIDIR/$ODIR/smiPreproc
alias smiKill=$SMIDIR/$ODIR/smiKill
alias smiGUI=$SMIDIR/$ODIR/smiGUI

