#!/bin/sh
#invoked from:$CLRFS/alidcsvme*/home/.custom.rc
echo hello from `hostname`
#touch /var/log/wtmp /var/log/lastlog
#echo "Executing: cd / ; ln -s /usr/local/opt opt"
#cd / ; ln -s /usr/local/opt opt ; mkdir /root/NOTES ; chmod a+rx /root/NOTES
#ls -ld /opt /root
echo "up: " `date` >>/home/updown.log ; chown trigger.alice /home/updown.log
touch /home/alice/trigger/SystemIsUp; chown trigger.alice /home/alice/trigger/SystemIsUp

