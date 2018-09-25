ApMon notes.
1. sources, compilation, start
2. config file
3. debugging, scripts

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
Note 20.9.2018: CPPFLAGS = -std=gnu++0x in examples/Makefile (was just CPPFLAGS =)

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

=== 3. debugging, scripts
apmon4.sh stop/start
tail -f ~/v/vme/WORK/apmon4.log

./grunsctrl.py   -> see sec1update.txt 'logging control':

dns: adls
a[dd] [runn] [t]   choose run number (or random), t: only 1 detector
d[elete]           random number
D[elete all] 
Log Nolog 
r[andom automat] p[rint]q[uit]

=== case: show all messages coming from HMP
./grunsctrl.py
L
>a
s 1537343195 14   9ed0     hmpid:0x40 i.e. is in run 14
SEND n:1 nminr:42 1537348201 14 DET(PHS):2  busyLimit:2:200 busyTime:2:0
SEND n:1 nminr:43 1537348322 14 DET(HMP):2  busyLimit:2:320 busyTime:2:0
SEND n:1 nminr:43 1537348323 14 DET(PHS):2  busyLimit:2:200 busyTime:2:0
SEND n:1 nminr:44 1537348444 14 DET(HMP):2  busyLimit:2:320 busyTime:2:0
cca 120s between why?  -> forced message after 2min sent from 

[trigger@altri1 WORK]$ tail -f ltudimserver.log 

[trigger@altri1 v]$ cd vme/ltu_proxy/
linux/ltu_shm 811000 L
linux/ltu_shm 811000 N


=== 4. history, bugs
16.3. MCH <-> MTR fixed, apmon.log shortened
13.6.2016 TPC 1000us -> 1300 us
 3.10.2017 HMP 280 -> 320 in example_3.cpp
26.3. 2018 ACO 120 -> 123
