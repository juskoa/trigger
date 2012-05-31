#!/bin/bash
function pfile() {
#echo "                               --------------------- $1"
echo "                                                    <---- $1"
cat $1
}
pfile VALID.LTUS
pfile VALID.CTPINPUTS
pfile TRIGGER.DESCRIPTORS
pfile TRIGGER.PFS
pfile p33.partition

