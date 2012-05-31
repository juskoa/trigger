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
if [ $# -lt 3 ] ;then
  echo "Usage: NewDiskless.bash Name IPaddress hw:ad:dr:es:s_:af [5]"
  exit
fi
hname=`hostname`
pxecfg=default_diskless
cctnbi="CCT.nbi"
clrfs="/data/ClientLocalRootFs/"
if [ "$hname" = 'alidcscom026' ] ;then
  CCTDIR=/tftpboot/CCTp2
elif [ "$hname" = 'pcalicebhm05' ] ;then
  CCTDIR=/CCT
  if [ "$4" = "5" ] ;then
    pxecfg=default_diskless5
    cctnbi="CCT5.nbi"
    clrfs="/b/CLRFS/"
  fi
else
  echo "can e started only form alidcscom026 or pcalicebhm05"
  exit
fi
NAME=$1
IPAD=$2
HWAD=$3
###if [ $# -eq 100 ] ;then
echo "modifying pxeboot, dhcpd.conf..."
cd /tftpboot/pxelinux.cfg
ipcode=`$CCTDIR/install/ip2hex $IPAD |tr a-z A-Z`
echo "$ipcode"
cp $pxecfg $ipcode
if [ $? -ne 0 ] ;then
  echo "are you root?"
  exit
fi
sed '$d' /etc/dhcpd.conf >/tmp/dhcpd.conf
cat - <<-EOF >>/tmp/dhcpd.conf
  host $NAME
  {
    hardware ethernet     $HWAD;
    fixed-address         $IPAD;
    server-name           "$hname.cern.ch";
    option host-name      "$NAME";
    if substring (option vendor-class-identifier, 0, 9) = "PXEClient"
    {
      filename "pxelinux.0";
    }
    else
    {
      filename "$cctnbi";
    }
  }
}
EOF
mv /tmp/dhcpd.conf /etc/dhcpd.conf
echo "making local filesystem for client..."
cd $clrfs
cp -a alctp "$NAME" ; cd "$NAME/etc"
###fi
###cd $clrfs ; cd $NAME/etc
sed "s/alctp/$NAME/g" sysconfig/network >/tmp/sn && mv /tmp/sn sysconfig/network
sed "s/100.100.100.100/$IPAD/g
s/00:00:00:00:00:00/$HWAD/g" sysconfig/network-scripts/ifcfg-eth1 >/tmp/sn && mv /tmp/sn sysconfig/network-scripts/ifcfg-eth1
sed "s/alctp/$NAME/g" ../home/.custom.rc >/tmp/sn && mv /tmp/sn ../home/.custom.rc 
chmod a+x ../home/.custom.rc
sed "s/alctp/$NAME/g" ../home/.fstab.local >/tmp/sn && mv /tmp/sn ../home/.fstab.local
echo "user specific (nothing done)"
echo "running /sbin/service nfs restart..."
/sbin/service nfs restart
cat - <<-EOF
VP315 bios:
Main-> Boot Options:                                                            
    Set the parameter Option ROM Loading to Load All                            
    Set the parameter PXE Boot Firmware to Enabled                              
    Auto retry on boot fail: Enabled                                            
Advanced:                                                                       
Ethernet 1 Connector to Front panel (MAC address is on right chip)              
Universe:                                                                       
VME System reset enabled


do not forget to modify /etc/exports and run exportfs -ar
AND Finally: service dhcpd restart...
EOF
#/sbin/service dhcpd restart

