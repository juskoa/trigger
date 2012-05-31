#!/bin/bash
function dorsync() {
# anv n:dry run
rsync -a --include-from='-' $1 ~/archive <<-EOF
+ */
+ *.c
+ *.cc
+ *.C
+ *.cxx
+ *.h
+ *akefile*
+ *eadme*
+ *.txt
+ *.pl
+ *.py
+ *.sh
+ *.pl
- *
EOF
}
cd
dorsync SMAQProject
dorsync IRS
dorsync busytool
dorsync bin
dorsync rl
