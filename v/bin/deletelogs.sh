#!/bin/bash
cd /data/ClientLocalRootFs
#find . -mtime +30 -type f |xargs -l40 echo
# following done 9.6.2009:  (1400 files)
# find alidcsvme00[2-7]/home/alice/trigger/v/*/WORK -mtime +120 -type f |xargs -l10 rm
# following done 17.12.2009:  (381 files)
#find alidcsvme00[2-7]/home/alice/trigger/v/*/WORK -mtime +30 -type f |xargs -l10 rm
#
# following done 18.5.2010:  (417files)
#find alidcsvme00[2-7]/home/alice/trigger/v/*/WORK -mtime +30 -type f |wc
#find alidcsvme00[2-7]/home/alice/trigger/v/*/WORK -mtime +30 -type f |xargs -l10 rm
# following done 18.11.2010:  ( 225 files)
#find alidcsvme00[2-7]/home/alice/trigger/v/*/WORK -mtime +30 -type f |wc
#find alidcsvme00[2-7]/home/alice/trigger/v/*/WORK -mtime +30 -type f |xargs -l10 rm
# su - ; cd /home/alice
#find */v/vme/WORK -mtime +30 -type f |xargs -l10 rm

