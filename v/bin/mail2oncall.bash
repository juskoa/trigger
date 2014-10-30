#!/bin/bash
addr="Anton.Jusko@cern.ch"
# seems following not working at 27.10.2014:
#addr="41764872090@mail2sms.cern.ch"
#/bin/mail -s "From pit:" $addr <<-EOF
/bin/mail $addr <<-EOF
$*
EOF
