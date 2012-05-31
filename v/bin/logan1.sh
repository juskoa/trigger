#!/bin/bash
# find time between .pcfg/get+got and .rcfg/got
`cdvme ctp`
logs=ctp_proxy1105010616.log
#grep -e 'timestamp.pcfg1' -e': L012' $logs
grep -e 'timestamp.pcfg1:rc' -e': L012' $logs
