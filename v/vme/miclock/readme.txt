************************
*** About	     ***
************************
A application to replace the existing ttmidaemons/miclock.py implementation. Same 
functionality but with a user interface. Written in Java with some of the helper
scripts from the old implementation kept. 

Version: 1.0
Author: Ørjan Breimo Helstrøm


************************
*** Installation     ***
************************

1) Make sure you have dim installed on the computer
2) Check that install folder looks like this:
		- miclockGui 	-> folder with the java source code. (.java files)
		- dim.jar 	-> the dim java implementation. can be compiled from dim
				   distribution.
		- libjdim.so	-> the dim.jar native library.
		- make.py	-> the python install script.
		- readme.txt 	-> this file.
3) Run the make.py script. (python make.py)

************************
*** Run		     ***
************************

1) Run the compiled jar file. (java -jar MiClockGui.jar)

************************
*** Environment	     ***
************************

Environment variables that need to be set before running the program.
	1) DIM_DNS_NODE
	2) VMECFDIR
	3) VMEWORKDIR
	4) VMESITE			-> Only in control room
	5) USER
	6) HOME
	
Log file: $VMEWORKDIR/WORK/miclock.log
	
************************
*** Changelog	     ***
************************

==Version 1.0==
	- Initial release

19.7.2012: installed in ~/jmiclock at trigger@alidcscom188
and also in trigger@pcalicebhm10:
mkdir -p ~/jmiclock/miclockGui
cd ~/jmiclock
ln -s /opt/dip/lib64/libjdim.so libjdim.so
  ln -s /opt/dip/lib/libjdim.so libjdim.so
ln -s /opt/dim/linux/dim.jar dim.jar

cd $VMECFDIR/miclock/workspace/miclockgui/src/miclockGui
cp *.java ../make.py ~/jmiclock/miclockGui/
cd ~/jmiclock ; python miclockGui/make.py

Start: an alis is defined in bin/setenv:
alias miclock="java -jar ~/jmiclock/MiClockGui.jar"

23.7.2012
- master copy in $VMECFDIR/miclok, .../workspace/miclockgui/src/ removed, i.e.
  miclock/readme.txt,...
-tooltips added (ButtonPanel.java modified)
28.8.
HTMLWriter.java modified. Now ~/CNTRRD/htmls/clockinfo changed
correctly according to operational mode.
28.8.
alias miclock (in bin/setenv) now it changes working 
directory to ~/miclock directory before starting miclock
31.8.2012
Installation:
mkdir -p ~/jmiclock/miclockGui
cd ~/jmiclock
ln -s /opt/dip/lib64/libjdim.so libjdim.so
  ln -s /opt/dip/lib/libjdim.so libjdim.so
ln -s /opt/dim/linux/dim.jar dim.jar

cd $VMECFDIR/miclock/miclockGui
cp *.java ~/jmiclock/miclockGui/
cp ../make.bash ~/jmiclock/
cd ~/jmiclock ; ./make.bash

Start: an alias is defined in bin/setenv:
alias miclock='echo -e '\''miclockold  -old, command line version\n'\''; cd ~/jmiclock; java -jar MiClockGui.jar'

14.11.2012
InfoPanel.java -comment extended " allow at least 4 min..."
