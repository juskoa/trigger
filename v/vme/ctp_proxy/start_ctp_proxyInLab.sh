#export DIM_DNS_NODE=pcald30
cd $VMEWORKDIR
if [ $1 = "test" ] ;then
$VMECFDIR/ctp_proxy/linux/test
else
nohup $VMECFDIR/ctp_proxy/linux/ctp_proxy TRIGGER::CTP >WORK/ctp_proxy.log &
fi
