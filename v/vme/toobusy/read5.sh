#!/bin/bash
cat - <<-EOF
give 5 numbers with 1 space between each (in the following form):
a b c d e
where a is the end of your busysweep range (in microseconds)
b is the start of your busysweep range (in microseconds), or 0
c is the size of each step in the sweep (in microseconds)
d is the time spent measuring the busy PER STEP (in seconds)
and e is the detector number:
To find e, refer to below:
0 = CTP_BUSY
1-24: Detector BUSY (see table)
25-30 = Cluster 1 to 6 BUSY
31 = Test cluster BUSY
DETECTOR TABLE (BASED ON VALID.LTUs 03/10)
1=SDD    2=MUONTRK
3=MUONTRG    4=DAQ
5=SPD    6=TOF
7=V0    9=TRD
10=ZDC   11=EMCAL
13=TPC   14=PMD
15=ACORDE   17=SSD
18=FMD   19=T0
21=HMPID    22=PHOS
Please consider how long a full run will take with the options you provide
A sweep predicted to take longer than half an hour will abort.
EXAMPLE: to investigate busy distribution of the TPC when
its average busy is 4000 microseconds:
6000 2000 200 5 13
This run would last for approx. 100 seconds and observe 
the range surrounding the TPCs average busy-length.
EOF
read a b c d e
echo "numbers: $a $b $c $d $e"
#measure $a $b $c $d $e

ssh -2 trigger@alidcsvme001 "cd ~/v/vme/WORK ; \$VMECFDIR/toobusy/linux_c/toobusy.exe $a $b $c $d $e"

echo sshrc:$?
# now copy file:
scp -2 trigger@alidcsvme001:\~/v/vme/WORK/busysweep ~/v/vme/WORK/
echo scp rc:$?
