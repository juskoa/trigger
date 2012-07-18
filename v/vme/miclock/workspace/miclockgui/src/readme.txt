************************
*** About	     ***
************************
A application to replace the existing ttmidaemons implementation. Same 
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
	
	
************************
*** Changelog	     ***
************************

==Version 1.0==
	- Initial release
