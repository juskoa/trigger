#!/bin/bash
# configure LTU boards and start ltu_proxy daemons on each alidcsvme00[2-7]
export VMECFDIR=/usr/local/trigger/v/vme
cfgfile=$VMECFDIR/CFG/ctp/DB/ttcparts.cfg
hn=`hostname`
export VMEWORKDIR=$VMECFDIR
awk '{if(!/^#/ && ($2==host)) {print "/usr/local/trigger/bin/configFPGA.bash " $1 " " $3 >"/tmp/configALL"}}' host=$hn $cfgfile
#exit
#echo '/usr/local/trigger/bin/start_daemons >>/tmp/configALL.log' >>/tmp/configALL
#echo 'sleep 2' >>/tmp/configALL
echo '/usr/local/trigger/bin/start_daemons' >>/tmp/configALL
chmod o+x /tmp/configALL
sleep 9 
/bin/bash /tmp/configALL >/tmp/configALL.log
