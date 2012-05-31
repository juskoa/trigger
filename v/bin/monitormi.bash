#!/bin/bash
echo "kill -s SIGUSR1 pid     to stop monitoring"
cd ~/v/vme
echo 'nothing started...'
exit
$VMECFDIR/ttcmi/ttcmi.exe -noboardInit <<-EOF
monitorstatus()
q
EOF

