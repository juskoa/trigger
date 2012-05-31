#!/bin/bash
#this script is called through rcmd N... from trigger@alidcscom026 (from general dim server)
echo 'going down...'
echo "down: " `date` >>/home/updown.log ; rm -f /home/alice/trigger/SystemIsUp
if [ "$1" = 'reboot' ] ;then
/sbin/shutdown -f -r now
else
sync
/sbin/shutdown -h now
fi

