#!/bin/bash
cd $dbctp
#find .  -mtime -5 \( -name '*.[ch]' -o -name '*akefi*' -o -name '*eadm*' -o -name '*.py' -o -name '*.sh' \) >/tmp/tf
tar -zcf ~/db2act.tgz ctp.cfg L0.INPUTS CTP.SWITCH VALID.CTPINPUTS VALID.DESCRIPTORS VALID.BCMASKS VALID.LTUS ttcparts.cfg TRIGGER.PFS filter
echo "~/db2act.tgz created"
#scp ~/db2act.tgz aj@pcalicebhm11:
scp ~/db2act.tgz $dagw:

