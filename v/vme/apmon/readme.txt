ApMon notes.
1. sources, compilation, start
2. config file

=== 1. sources, compilation, start
Here ($VMECFDIR/apmon/ ) only alice specifics are kept:
- example_3.cpp 
- apmonConfig_lab.conf   (_alice at p2)
There is a link from whole ApMon package pointing to these files.

Whole ApMon package is in /home/dl6/local/sw/ApMon_cpp-2.2.8
The name of this directory is in $APMON, which is set in vmebse.bash:
export APMON=$TRG_ADDONS/ApMon_cpp-2.2.8

$APMON/examples/example_3.cpp is linked to $VMECFDIR/apmon/   i.e.:
[trigger@alidcscom835 examples]$ pwd
/home/dl6/local/sw/ApMon_cpp-2.2.8/examples
[trigger@alidcscom835 examples]$ ln -s /local/trigger/v/vme/apmon/example_3.cpp 
[trigger@alidcscom835 examples]$ ln -s /local/trigger/v/vme/apmon/apmonConfig_lab.conf
[trigger@alidcscom835 examples]$ ln -s /local/trigger/v/vme/apmon/apmonConfig_alice.conf

Compiled executable is in  $APMON/examples directory.
Installation:
cd $APMON ; ./configure ; make clean ; make
Compliation:
[trigger@alidcscom835 examples]$ make
...
startClients.bash rrd stop
startClients.bash rrd start
startClients.bash html start   (not needed! -done by monitor automatically )

Start explained:
Normally, example_3.exe is started using popen() from CNTRRD/readctpc.c,
from working directory ~/CNTRRD i.e.:
  apmonsw= getenv("APMON");
  sprintf(cmd, "%s/examples/example_3 >logs/apmon.log 2>&1", apmonsw);
  apmonpipe= openpipew(cmd);

=== 2. config file
is placed in $APMON/example (there is a link to $VMECFDIR/apmon).
2 files: 
apmonConfig_alice.conf   -used in p2 (used when VMESITE: ALICE)
apmonConfig_lab.conf     -used elsewhere

=== 3. history, bugs
16.3. MCH <-> MTR fixed, apmon.log shortened
13.6.2016 TPC 1000us -> 1300 us
 3.10.2017 HMP 280 -> 320 in example_3.cpp
26.3. 2018 ACO 120 -> 123
