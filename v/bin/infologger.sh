#!/bin/bash
. /opt/infoLogger/infoLoggerStandalone.sh
echo /opt/infoLogger/log $1 $2
cat /dev/null | /opt/infoLogger/log $1 $2
