#!/bin/bash 
# to be started from root@pcalicebhm05
# TODO: check on alidcscom026:
# dhcpd.conf (servad ,last line)
# alctp directory (master copy):
#           fstab (servadr
# alctp/etc/sysconfig/network (if GATEWAY set correctly
# alctp/etc/sysconfig/network-scripts/ifcfg-eth1    (100, 00:
#       home/.fstab.local (servadr
#       home/alice/trigger/.xscreensaver   .exrc
#
IPAD=$1
if [ $# -ne 2 ] ;then
  echo "Usage CheckIPD.bash Name IP"
  exit
fi
NAME=$1
IPAD=$2
CCTDIR=/CCT
#CCTDIR=/tftpboot/CCTp2
cd /tftpboot/pxelinux.cfg
ipcode=`$CCTDIR/install/ip2hex $IPAD |tr a-z A-Z`
echo "IPCODE $ipcode"
ls -l "$ipcode"
echo "dhcpd.conf:"
grep -e "$NAME" -e "$IPAD" /etc/dhcpd.conf
cd /data/ClientLocalRootFs/
cd "$NAME/etc"
echo "ifcfg-eth1:"
grep -e "$NAME" -e "$IPAD" sysconfig/network-scripts/ifcfg-eth1
echo ".custom.rc:"
cat ../home/.custom.rc
echo ".fstab.local:"
cat ../home/.fstab.local
echo "/etc/exports:"
grep "$NAME" /etc/exports

