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
if [ $# -ne 3 ] ;then
  echo "Usage NewIPDiskless.bash Name OldIP NewIP"
  exit
fi
hname=`hostname -s`
if [ "$hname" = 'alidcscom026' ] ;then
  CCTDIR=/tftpboot/CCTp2
elif [ "$hname" = 'pcalicebhm05' ] ;then
  CCTDIR=/CCT
else
  echo "can e started only form alidcscom026 or pcalicebhm05"
  exit
fi
NAME=$1
OLDIPAD=$2
IPAD=$3
###if [ $# -eq 100 ] ;then
echo "modifying pxeboot, dhcpd.conf..."
cd /tftpboot/pxelinux.cfg
oldipcode=`$CCTDIR/install/ip2hex $OLDIPAD |tr a-z A-Z`
ipcode=`$CCTDIR/install/ip2hex $IPAD |tr a-z A-Z`
echo "$oldipcode ---> $ipcode"
##cp default_diskless $ipcode
mv "$oldipcode" "$ipcode"
if [ $? -ne 0 ] ;then
  echo "are you root?"
  exit
fi
##sed '$d' /etc/dhcpd.conf >/tmp/dhcpd.conf
sed "s/$OLDIPAD/$IPAD/g" /etc/dhcpd.conf >/tmp/dc && mv /tmp/dc /etc/dhcpd.conf
echo "making local filesystem for client..."
cd /data/ClientLocalRootFs/
##cp -a alctp $NAME ; cd $NAME/etc
cd "$NAME/etc"
###fi
##sed "s/alctp/$NAME/g" sysconfig/network >/tmp/sn && mv /tmp/sn sysconfig/network
snie0="sysconfig/network-scripts/ifcfg-eth0"
snie1="sysconfig/network-scripts/ifcfg-eth1"
if [ -w "$snie0" ] ;then
  snie=$snie0
elif [ -w "$snie1" ] ;then
  snie=$snie1
else
  echo "$snie0 (neither ...1) not found"
  exit
fi
echo "modifying $snie ..."
sed "s/$OLDIPAD/$IPAD/g" $snie >/tmp/sn && mv /tmp/sn $snie
##sed "s/alctp/$NAME/g" ../home/.custom.rc >/tmp/sn && mv /tmp/sn ../home/.custom.rc 
##sed "s/alctp/$NAME/g" ../home/.fstab.local >/tmp/sn && mv /tmp/sn ../home/.fstab.local
echo "user specific (nothing done)"
echo "running /sbin/service nfs restart..."
/sbin/service nfs restart
echo "running exportfs -ar"
/usr/sbin/exportfs -ar
echo "At last: dhcpd restart..."
/sbin/service dhcpd restart

