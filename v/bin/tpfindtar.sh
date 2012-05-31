#!/bin/bash
cd $VMEBDIR/..
find .  -mtime -5 \( -name '*.[ch]' -o -name '*akefi*' -o -name '*eadm*' -o -name '*.py' -o -name '*.sh' \) >/tmp/tf
tar -zcf ~/tocern.tgz -T/tmp/tf

