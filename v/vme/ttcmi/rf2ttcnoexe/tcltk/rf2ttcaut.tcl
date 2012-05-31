#last modification 03/04/2007 - 
#/usr/local/bin/wish

wm title . "RF_2_TTC Automatic Mode Control     Angel Monera - Sophie Baron"

#variables and constants

set idstatus "Click in identify!!"
set brdheader 0x00

#-----___-----___-----___-----___-----___-----___


set bc1mxbeam	X
set bc2mxbeam	X
set bcrmxbeam	X
set bcmmxbeam	X
set orb1mxbeam	x
set orb2mxbeam	x
set orbmmxbeam	X

set bc1mxnobm	X
set bc2mxnobm	X
set bcrmxnobm	X
set bcmmxnobm	X
set orb1mxnobm	x
set orb2mxnobm	x
set orbmmxnobm	X

set bc1mxst		"Read!"
set bc2mxst	"Read!"
set bcrmxst		"Read!"
set bcmmxst	"Read!"
set orb1mxst	"Read!"
set orb2mxst	"Read!"
set orbmmxst	"Read!"

set bst			"Read!!"
set offsetbst 	7FA9C

set bstst  "BST status: Press Read!!"
set offsetbstst	7FAA0

set beam 		"Press Read for Beam Status!!"

set mode 		"Read!!"
set offsetmode 7FA7C

set offsetworkmode   7FA78

set offsetbc1mxbeam		7FBF8
set offsetbc2mxbeam		7FBC8
set offsetbcrmxbeam		7FBA8
set offsetbcmmxbeam	7FB88
set offsetorb1mxbeam	7FB68
set offsetorb2mxbeam	7FB28
set offsetorbmmxbeam	7FAE8

set offsetbc1mxnobm		7FBF4
set offsetbc2mxnobm		7FBC4
set offsetbcrmxnobm		7FBA4
set offsetbcmmxnobm	7FB84
set offsetorb1mxnobm	7FB64
set offsetorb2mxnobm	7FB24
set offsetorbmmxnobm	7FAE4

set offsetbc1mxst		0
set offsetbc2mxst		0
set offsetbcrmxst		0
set offsetbcmmxst		0
set offsetorb1mxst	0
set offsetorb2mxst	0
set offsetorbmmxst	0


set offsetbc1delay 	7D000
set offsetbc2delay		7D100
set offsetbcrefdelay	7D080
set offsetorb1sdelay	7D020
set offsetorb1fdelay	7D030
set offsetorb2sdelay	7D120
set offsetorb2fdelay	7D130
set offsetorbmfdelay	7D0B0
set offsetreset  		7FA38
set offsetqpllreset 	7FA40
set offsetdelayreset 	7FA3C

set offsetttcrxreg	7E000
set offsetttcrxdata	7E004

set offsetBset  00010
set offsetBclear 00014

#________--------________________________________

set peeklocation "vme_peek"
set pokelocation "vme_poke"
set var "debug window"
set var2 "debug window"
set var3 "debug window"

#END VARIABLES

#Panel Header and board identificication

frame .brdheader
label .brdheader.labell -text "Board Address"
entry .brdheader.entryy -textvariable brdheader -width 5
label .brdheader.identify -relief sunken -textvariable idstatus -width 30
button .brdheader.buttonn -text "Identify" -command {
	set addrpeek $brdheader; append addrpeek "00000"
	set pipe [open "|$peeklocation $addrpeek 4 2"];	gets $pipe idstatus;
	if {[string match *080030* $idstatus] == 0 } {	set idstatus "Not Found!! :(";
	} else {set idstatus "FOUND RF2TTC :D"};	catch {close $pipe};
}
pack .brdheader -fill x
grid .brdheader.labell -colum 1 -row 1
grid .brdheader.entryy -colum 2 -row 1
grid .brdheader.buttonn -colum 3 -row 1
grid .brdheader.identify -colum 4 -row 1

#blank line
label .sp0 -text " "
pack .sp0


#Main buttons to reset and initialize the board, qpll and delays
frame .btts
button .btts.reset -text "Reset Board" -width 15 -command {
						set addrpoke $brdheader;  
						append addrpoke $offsetBset;  
						exec $pokelocation $addrpoke 0x0008 4 2 ; 
						set addrpoke $brdheader;  
						append addrpoke $offsetBclear;  
						exec $pokelocation $addrpoke 0x0008 4 2 
						}

button .btts.dlyrst -text "Reset Delays" -width 15 -command {
						set addrpoke $brdheader;  
						append addrpoke $offsetBset;  
						exec $pokelocation $addrpoke 0x0001 4 2 ; 
						set addrpoke $brdheader;  
						append addrpoke $offsetBclear;  
						exec $pokelocation $addrpoke 0x0001 4 2 
						}
						
button .btts.ttcrxrst -text "Reset TTCrx" -width 15 -command {
						set addrpoke $brdheader;  
						append addrpoke $offsetBset;  
						exec $pokelocation $addrpoke 0x0004 4 2 ; 
						set addrpoke $brdheader;  
						append addrpoke $offsetBclear;  
						exec $pokelocation $addrpoke 0x0004 4 2 
						}
						
button .btts.dlyini -text "Initialize Delays" -width 15 -command {
				set delay 0
				set dataex [expr $delay + 64]
				set dataex [format %x $dataex]
				set data 0x00

				append data $dataex
				set addrpoke $brdheader;	append addrpoke $offsetbc1delay;	exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;
				set addrpoke $brdheader;	append addrpoke $offsetbc2delay;	exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;
				set addrpoke $brdheader;	append addrpoke $offsetbcrefdelay;	exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;

				set addrpoke $brdheader;	append addrpoke $offsetorb1sdelay;		exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;
				set addrpoke $brdheader;	append addrpoke $offsetorb1fdelay;		exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;
				set addrpoke $brdheader;	append addrpoke $offsetorb2sdelay;		exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;
				set addrpoke $brdheader;	append addrpoke $offsetorb2fdelay;		exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;
				set addrpoke $brdheader;	append addrpoke $offsetorbmfdelay;		exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;
			}
button .btts.ttcrxini -text "Initialize TTCrx" -width 15 -command {
				set addrpoke $brdheader
				append addrpoke $offsetttcrxreg
				exec $pokelocation $addrpoke 0x0003 4 2
				set addrpoke $brdheader
				append addrpoke $offsetttcrxdata
				exec $pokelocation $addrpoke 0x00B3 4 2
				}
			
						
label .btts.blank1 -text "" -width 5
button .btts.manual -text "Change to Automatic" -width 15 -background  yellow -command {
				set addrpoke $brdheader;  append addrpoke $offsetworkmode;  exec $pokelocation $addrpoke 0xFFFFFF 4 2;
				.btts.manual configure -background green }

pack .btts -fill x
grid .btts.reset -colum 1 -row 1
grid .btts.dlyrst -colum 2 -row 1
grid .btts.ttcrxrst -colum 3 -row 1
grid .btts.dlyini -colum 1 -row 2
grid .btts.ttcrxini -colum 2 -row 2
grid .btts.blank1 -colum 4 -row 1
grid .btts.manual -colum 5 -row 1




#blank line
label .sp01 -text ""
pack .sp01


#-----------------------------------------------------------------------------------------------------------------------------
#Multiplexor beem mode
#-----------------------------------------------------------------------------------------------------------------------------


frame .mx
pack .mx -fill x

	frame .mx.beam -borderwidth 1 -relief raised
	grid .mx.beam -colum 1 -row 1

		label .mx.beam.lb -text "Setting for BEAM Mode"
		pack .mx.beam.lb -fill x

		frame 				.mx.beam.bc1 -borderwidth 1 -relief raised
			label 				.mx.beam.bc1.lb -text "BC1 input select:       "
			#Sophie 24.01.07 - invert 0 and 1
			radiobutton 	.mx.beam.bc1.int -text "Int  " -variable bc1mxbeam -value 0x00000000
			radiobutton 	.mx.beam.bc1.ext -text "Ext  " -variable bc1mxbeam -value 0x00000001
			button 			.mx.beam.bc1.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetbc1mxbeam
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set bc1mxbeam "error"
				} else {set bc1mxbeam $var}
			}

			button .mx.beam.bc1.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetbc1mxbeam;  exec $pokelocation $addrpoke $bc1mxbeam 4 2 }
			pack	.mx.beam.bc1 -fill x
			grid	.mx.beam.bc1.lb		-colum 1 -row 1
			grid	.mx.beam.bc1.int	-colum 2 -row 1
			grid	.mx.beam.bc1.ext	-colum 2 -row 2
			grid	.mx.beam.bc1.rd	-colum 3 -row 1
			grid	.mx.beam.bc1.wt	-colum 3 -row 2

		frame 				.mx.beam.bc2 -borderwidth 1 -relief raised
			label 				.mx.beam.bc2.lb -text "BC2 input select:       "
			radiobutton 	.mx.beam.bc2.int -text "Int  " -variable bc2mxbeam -value 0x00000000
			radiobutton 	.mx.beam.bc2.ext -text "Ext  " -variable bc2mxbeam -value 0x00000001
			button 			.mx.beam.bc2.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetbc2mxbeam
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set bc2mxbeam "error"
				} else {set bc2mxbeam $var}
			}

			button .mx.beam.bc2.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetbc2mxbeam;  exec $pokelocation $addrpoke $bc2mxbeam 4 2 }
			pack	.mx.beam.bc2 -fill x
			grid	.mx.beam.bc2.lb		-colum 1 -row 1
			grid	.mx.beam.bc2.int	-colum 2 -row 1
			grid	.mx.beam.bc2.ext	-colum 2 -row 2
			grid	.mx.beam.bc2.rd	-colum 3 -row 1
			grid	.mx.beam.bc2.wt	-colum 3 -row 2

		frame 				.mx.beam.bcr -borderwidth 1 -relief raised
			label 				.mx.beam.bcr.lb -text "BCref input select:    "
			radiobutton 	.mx.beam.bcr.int -text "Int  " -variable bcrmxbeam -value 0x00000000
			radiobutton 	.mx.beam.bcr.ext -text "Ext  " -variable bcrmxbeam -value 0x00000001
			button 			.mx.beam.bcr.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetbcrmxbeam
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set bcrmxbeam "error"
				} else {set bcrmxbeam $var}
			}

			button .mx.beam.bcr.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetbcrmxbeam;  exec $pokelocation $addrpoke $bcrmxbeam 4 2 }
			pack	.mx.beam.bcr -fill x
			grid	.mx.beam.bcr.lb		-colum 1 -row 1
			grid	.mx.beam.bcr.int	-colum 2 -row 1
			grid	.mx.beam.bcr.ext	-colum 2 -row 2
			grid	.mx.beam.bcr.rd	-colum 3 -row 1
			grid	.mx.beam.bcr.wt	-colum 3 -row 2

		frame 				.mx.beam.bcm -borderwidth 1 -relief raised
			label 				.mx.beam.bcm.lb -text "BCmain in sel:"
			#sophie 24.01.07 - change the mode encoding
			radiobutton 	.mx.beam.bcm.int -text "Int" -variable bcmmxbeam -value 	0x00000000
			radiobutton 	.mx.beam.bcm.b1 -text "BC1" -variable bcmmxbeam -value 	0x00000003
			radiobutton 	.mx.beam.bcm.b2 -text "BC2" -variable bcmmxbeam -value 	0x00000002
			radiobutton 	.mx.beam.bcm.br -text "BCref" -variable bcmmxbeam -value 0x00000001
			button 			.mx.beam.bcm.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetbcmmxbeam
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set bcmmxbeam "error"
				} else {set bcmmxbeam $var}
			}

			button .mx.beam.bcm.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetbcmmxbeam;  exec $pokelocation $addrpoke $bcmmxbeam 4 2 }
			pack	.mx.beam.bcm -fill x
			grid	.mx.beam.bcm.lb		-colum 1 -row 1
			grid	.mx.beam.bcm.int	-colum 2 -row 1
			grid	.mx.beam.bcm.b1	-colum 2 -row 2
			grid	.mx.beam.bcm.b2	-colum 3 -row 1
			grid	.mx.beam.bcm.br	-colum 3 -row 2
			grid	.mx.beam.bcm.rd	-colum 4 -row 1
			grid	.mx.beam.bcm.wt	-colum 4 -row 2

		frame 				.mx.beam.orb1 -borderwidth 1 -relief raised
			label 				.mx.beam.orb1.lb -text "Orb1 input select:       "
			radiobutton 	.mx.beam.orb1.int -text "Int  " -variable orb1mxbeam -value 0x00000000
			radiobutton 	.mx.beam.orb1.ext -text "Ext  " -variable orb1mxbeam -value 0x00000001
			button 			.mx.beam.orb1.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb1mxbeam
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb1mxbeam "error"
				} else {set orb1mxbeam $var}
			}

			button .mx.beam.orb1.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetorb1mxbeam;  exec $pokelocation $addrpoke $orb1mxbeam 4 2 }
			pack	.mx.beam.orb1 -fill x
			grid	.mx.beam.orb1.lb		-colum 1 -row 1
			grid	.mx.beam.orb1.int	-colum 2 -row 1
			grid	.mx.beam.orb1.ext	-colum 2 -row 2
			grid	.mx.beam.orb1.rd	-colum 3 -row 1
			grid	.mx.beam.orb1.wt	-colum 3 -row 2

		frame 				.mx.beam.orb2 -borderwidth 1 -relief raised
			label 				.mx.beam.orb2.lb -text "Orb2 input select:       "
			radiobutton 	.mx.beam.orb2.int -text "Int  " -variable orb2mxbeam -value 0x00000000
			radiobutton 	.mx.beam.orb2.ext -text "Ext  " -variable orb2mxbeam -value 0x00000001
			button 			.mx.beam.orb2.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb2mxbeam
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb2mxbeam "error"
				} else {set orb2mxbeam $var}
			}

			button .mx.beam.orb2.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetorb2mxbeam;  exec $pokelocation $addrpoke $orb2mxbeam 4 2 }
			pack	.mx.beam.orb2 -fill x
			grid	.mx.beam.orb2.lb		-colum 1 -row 1
			grid	.mx.beam.orb2.int	-colum 2 -row 1
			grid	.mx.beam.orb2.ext	-colum 2 -row 2
			grid	.mx.beam.orb2.rd	-colum 3 -row 1
			grid	.mx.beam.orb2.wt	-colum 3 -row 2



		frame 				.mx.beam.orbm -borderwidth 1 -relief raised
			label 				.mx.beam.orbm.lb -text "Orb Main in select:   "
			radiobutton 	.mx.beam.orbm.int -text "Int  " -variable orbmmxbeam -value 		0x00000002
			radiobutton 	.mx.beam.orbm.o1 -text "Orb1 " -variable orbmmxbeam -value 	0x00000000
			radiobutton 	.mx.beam.orbm.o2 -text "Orb2 " -variable orbmmxbeam -value 	0x00000001
			button 			.mx.beam.orbm.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorbmmxbeam
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orbmmxbeam "error"
				} else {set orbmmxbeam $var}
			}

			button .mx.beam.orbm.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetorbmmxbeam;  exec $pokelocation $addrpoke $orbmmxbeam 4 2 }
			pack	.mx.beam.orbm -fill x
			grid	.mx.beam.orbm.lb		-colum 1 -row 1
			grid	.mx.beam.orbm.int	-colum 2 -row 1
			grid	.mx.beam.orbm.o1	-colum 2 -row 2
			grid	.mx.beam.orbm.o2	-colum 2 -row 3
			grid	.mx.beam.orbm.rd	-colum 3 -row 1
			grid	.mx.beam.orbm.wt	-colum 3 -row 2

#*************************************************************************************************************
#***multiplexor in no beam mode***************************************************************************
#*************************************************************************************************************
	frame .mx.nobm -borderwidth 1 -relief raised
	grid .mx.nobm -colum 2 -row 1

		label .mx.nobm.lb -text "Setting for NO BEAM Mode"
		pack .mx.nobm.lb -fill x

		frame 				.mx.nobm.bc1 -borderwidth 1 -relief raised
			label 				.mx.nobm.bc1.lb -text "BC1 input select:       "
			radiobutton 	.mx.nobm.bc1.int -text "Int  " -variable bc1mxnobm -value 0x00000000
			radiobutton 	.mx.nobm.bc1.ext -text "Ext  " -variable bc1mxnobm -value 0x00000001
			button 			.mx.nobm.bc1.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetbc1mxnobm
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set bc1mxnobm "error"
				} else {set bc1mxnobm $var}
			}

			button .mx.nobm.bc1.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetbc1mxnobm;  exec $pokelocation $addrpoke $bc1mxnobm 4 2 }
			pack	.mx.nobm.bc1 -fill x
			grid	.mx.nobm.bc1.lb		-colum 1 -row 1
			grid	.mx.nobm.bc1.int	-colum 2 -row 1
			grid	.mx.nobm.bc1.ext	-colum 2 -row 2
			grid	.mx.nobm.bc1.rd	-colum 3 -row 1
			grid	.mx.nobm.bc1.wt	-colum 3 -row 2

		frame 				.mx.nobm.bc2 -borderwidth 1 -relief raised
			label 				.mx.nobm.bc2.lb -text "BC2 input select:       "
			radiobutton 	.mx.nobm.bc2.int -text "Int  " -variable bc2mxnobm -value 0x00000000
			radiobutton 	.mx.nobm.bc2.ext -text "Ext  " -variable bc2mxnobm -value 0x00000001
			button 			.mx.nobm.bc2.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetbc2mxnobm
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set bc2mxnobm "error"
				} else {set bc2mxnobm $var}
			}

			button .mx.nobm.bc2.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetbc2mxnobm;  exec $pokelocation $addrpoke $bc2mxnobm 4 2 }
			pack	.mx.nobm.bc2 -fill x
			grid	.mx.nobm.bc2.lb		-colum 1 -row 1
			grid	.mx.nobm.bc2.int	-colum 2 -row 1
			grid	.mx.nobm.bc2.ext	-colum 2 -row 2
			grid	.mx.nobm.bc2.rd	-colum 3 -row 1
			grid	.mx.nobm.bc2.wt	-colum 3 -row 2

		frame 				.mx.nobm.bcr -borderwidth 1 -relief raised
			label 				.mx.nobm.bcr.lb -text "BCref input select:    "
			radiobutton 	.mx.nobm.bcr.int -text "Int  " -variable bcrmxnobm -value 0x00000000
			radiobutton 	.mx.nobm.bcr.ext -text "Ext  " -variable bcrmxnobm -value 0x00000001
			button 			.mx.nobm.bcr.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetbcrmxnobm
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set bcrmxnobm "error"
				} else {set bcrmxnobm $var}
			}

			button .mx.nobm.bcr.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetbcrmxnobm;  exec $pokelocation $addrpoke $bcrmxnobm 4 2 }
			pack	.mx.nobm.bcr -fill x
			grid	.mx.nobm.bcr.lb		-colum 1 -row 1
			grid	.mx.nobm.bcr.int	-colum 2 -row 1
			grid	.mx.nobm.bcr.ext	-colum 2 -row 2
			grid	.mx.nobm.bcr.rd	-colum 3 -row 1
			grid	.mx.nobm.bcr.wt	-colum 3 -row 2

		frame 				.mx.nobm.bcm -borderwidth 1 -relief raised
			label 				.mx.nobm.bcm.lb -text "BCmain in sel:"
			radiobutton 	.mx.nobm.bcm.int -text "Int" -variable bcmmxnobm -value 	0x00000000
			radiobutton 	.mx.nobm.bcm.b1 -text "BC1" -variable bcmmxnobm -value 	0x00000003
			radiobutton 	.mx.nobm.bcm.b2 -text "BC2" -variable bcmmxnobm -value 	0x00000002
			radiobutton 	.mx.nobm.bcm.br -text "BCref" -variable bcmmxnobm -value 0x00000001
			button 			.mx.nobm.bcm.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetbcmmxnobm
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set bcmmxnobm "error"
				} else {set bcmmxnobm $var}
			}

			button .mx.nobm.bcm.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetbcmmxnobm;  exec $pokelocation $addrpoke $bcmmxnobm 4 2 }
			pack	.mx.nobm.bcm -fill x
			grid	.mx.nobm.bcm.lb		-colum 1 -row 1
			grid	.mx.nobm.bcm.int	-colum 2 -row 1
			grid	.mx.nobm.bcm.b1	-colum 2 -row 2
			grid	.mx.nobm.bcm.b2	-colum 3 -row 1
			grid	.mx.nobm.bcm.br	-colum 3 -row 2
			grid	.mx.nobm.bcm.rd	-colum 4 -row 1
			grid	.mx.nobm.bcm.wt	-colum 4 -row 2

		frame 				.mx.nobm.orb1 -borderwidth 1 -relief raised
			label 				.mx.nobm.orb1.lb -text "Orb1 input select:       "
			radiobutton 	.mx.nobm.orb1.int -text "Int  " -variable orb1mxnobm -value 0x00000000
			radiobutton 	.mx.nobm.orb1.ext -text "Ext  " -variable orb1mxnobm -value 0x00000001
			button 			.mx.nobm.orb1.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb1mxnobm
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb1mxnobm "error"
				} else {set orb1mxnobm $var}
			}

			button .mx.nobm.orb1.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetorb1mxnobm;  exec $pokelocation $addrpoke $orb1mxnobm 4 2 }
			pack	.mx.nobm.orb1 -fill x
			grid	.mx.nobm.orb1.lb		-colum 1 -row 1
			grid	.mx.nobm.orb1.int	-colum 2 -row 1
			grid	.mx.nobm.orb1.ext	-colum 2 -row 2
			grid	.mx.nobm.orb1.rd	-colum 3 -row 1
			grid	.mx.nobm.orb1.wt	-colum 3 -row 2

		frame 				.mx.nobm.orb2 -borderwidth 1 -relief raised
			label 				.mx.nobm.orb2.lb -text "Orb2 input select:       "
			radiobutton 	.mx.nobm.orb2.int -text "Int  " -variable orb2mxnobm -value 0x00000000
			radiobutton 	.mx.nobm.orb2.ext -text "Ext  " -variable orb2mxnobm -value 0x00000001
			button 			.mx.nobm.orb2.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb2mxnobm
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb2mxnobm "error"
				} else {set orb2mxnobm $var}
			}

			button .mx.nobm.orb2.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetorb2mxnobm;  exec $pokelocation $addrpoke $orb2mxnobm 4 2 }
			pack	.mx.nobm.orb2 -fill x
			grid	.mx.nobm.orb2.lb		-colum 1 -row 1
			grid	.mx.nobm.orb2.int	-colum 2 -row 1
			grid	.mx.nobm.orb2.ext	-colum 2 -row 2
			grid	.mx.nobm.orb2.rd	-colum 3 -row 1
			grid	.mx.nobm.orb2.wt	-colum 3 -row 2



		frame 				.mx.nobm.orbm -borderwidth 1 -relief raised
			label 				.mx.nobm.orbm.lb -text "Orb Main in select:   "
			radiobutton 	.mx.nobm.orbm.int -text "Int  " -variable orbmmxnobm -value 		0x00000002
			radiobutton 	.mx.nobm.orbm.o1 -text "Orb1 " -variable orbmmxnobm -value 	0x00000000
			radiobutton 	.mx.nobm.orbm.o2 -text "Orb2 " -variable orbmmxnobm -value 	0x00000001
			button 			.mx.nobm.orbm.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorbmmxnobm
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orbmmxnobm "error"
				} else {set orbmmxnobm $var}
			}

			button .mx.nobm.orbm.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetorbmmxnobm;  exec $pokelocation $addrpoke $orbmmxnobm 4 2 }
			pack	.mx.nobm.orbm -fill x
			grid	.mx.nobm.orbm.lb		-colum 1 -row 1
			grid	.mx.nobm.orbm.int	-colum 2 -row 1
			grid	.mx.nobm.orbm.o1	-colum 2 -row 2
			grid	.mx.nobm.orbm.o2	-colum 2 -row 3
			grid	.mx.nobm.orbm.rd	-colum 3 -row 1
			grid	.mx.nobm.orbm.wt	-colum 3 -row 2


#*************************************************************************************************************
#---- Read ALL section
#---- Read ALL section

		button .rdall -text "Read all" -command {

		.mx.beam.bc1.rd invoke
		.mx.beam.bc2.rd invoke
		.mx.beam.bcr.rd invoke
		.mx.beam.bcm.rd invoke
		.mx.beam.orb1.rd invoke
		.mx.beam.orb2.rd invoke
		.mx.beam.orbm.rd invoke

		.mx.nobm.bc1.rd invoke
		.mx.nobm.bc2.rd invoke
		.mx.nobm.bcr.rd invoke
		.mx.nobm.bcm.rd invoke
		.mx.nobm.orb1.rd invoke
		.mx.nobm.orb2.rd invoke
		.mx.nobm.orbm.rd invoke

		}
		pack .rdall -fill x


#blank line
label .sp3 -text ""
pack .sp3

#*************************************************************************************************************
#-----MONITOR SECTION************************************************************************
#*************************************************************************************************************

set bucle 0
set bstop 0
set monitor.mode "Read"
frame .monitor
	pack .monitor
	label .monitor.lb -text "STATUS"

	label .monitor.stlb -text "Bst Mode:" -width 10
	label .monitor.st	-textvariable bst  -relief sunken -width 10

	label .monitor.bc1 -text "Bc1St:" -width 10
	label .monitor.bc1st -textvariable bc1mxst -relief sunken -width 10

	label .monitor.bc2 -text "Bc2St:" -width 10
	label .monitor.bc2st -textvariable bc2mxst -relief sunken -width 10

	label .monitor.bcref -text "BcRefSt:" -width 10
	label .monitor.bcrefst -textvariable bcrmxst -relief sunken -width 10

	label .monitor.bcmain -text "BCMainSt:" -width 10
	label .monitor.bcmainst -textvariable bcmmxst -relief sunken -width 10

	label .monitor.blk3 -text "   "

	label .monitor.orb1 -text "Orb1St:" -width 10
	label .monitor.orb1st -textvariable orb1mxst -relief sunken -width 10

	label .monitor.orb2 -text "Orb2st:" -width 10
	label .monitor.orb2st -textvariable orb2mxst -relief sunken -width 10

	label .monitor.orbmain -text "OrbMainSt:" -width 10
	label .monitor.orbmainst -textvariable orbmmxst -relief sunken -width 10

	label .monitor.blk6 -text "   "
	
	button .monitor.rd -text "Read" -width 10 -command 		{ set bucle 0; set bstop 0; READSTATE;}
	button .monitor.mon -text "Monitor" -width 10 -command 	{ set bucle 1; set bstop 0; READSTATE;}
	button .monitor.stop -text "Stop" -width 10 -command 	{ set bstop 1; 	set bucle 0}

	grid .monitor.stlb -colum 3 -row 1
	grid .monitor.st -colum 4 -row 1

	grid .monitor.lb -colum 1 -row 1
	grid .monitor.bc1 -colum 1  -row 3
	grid .monitor.bc1st -colum 2 -row 3
	grid .monitor.bc2 -colum 1 -row 4
	grid .monitor.bc2st -colum 2 -row 4
	grid .monitor.bcref -colum 1 -row 5
	grid .monitor.bcrefst -colum 2 -row 5
	grid .monitor.bcmain -colum 1 -row 6
	grid .monitor.bcmainst -colum 2 -row 6

	grid .monitor.blk3 -colum 3 -row 6

	grid .monitor.orb1 -colum 4 -row 3
	grid .monitor.orb1st -colum 5 -row 3
	grid .monitor.orb2 -colum 4 -row 4
	grid .monitor.orb2st -colum 5 -row 4
	grid .monitor.orbmain -colum 4 -row 5
	grid .monitor.orbmainst -colum 5 -row 5

	grid .monitor.blk6 -colum 6 -row 7

	grid .monitor.rd -colum 7 -row 3
	grid .monitor.mon -colum 7 -row 4
	grid .monitor.stop -colum 7 -row 5


#	label lbmode -text "Mode" -width 10
	label .mode -textvariable beam -relief sunken -width 15 -background yellow
	pack .mode -fill x

	label .bst -textvariable bstst -relief sunken -width 15 -background yellow
	pack .bst -fill x

#blank line
label .sp4 -text ""
pack .sp4

#*************************************************************************************************************
#*************************************************************************************************************
#*************************************************************************************************************

frame .peeklocation
label .peeklocation.labelpeek -text "Vme_Peek Location: " -width 20
entry .peeklocation.entrypeek -textvariable peeklocation -width 30
button .peeklocation.peekdefaultbtt -text "Default Loc" -command PEEKDEFAULT -width 15
pack .peeklocation -fill x
grid .peeklocation.labelpeek -colum 1 -row 1
grid .peeklocation.entrypeek -colum 2 -row 1
grid .peeklocation.peekdefaultbtt -colum 3 -row 1


frame .pokelocation
label .pokelocation.labelpoke -text "Vme_Poke Location: " -width 20
entry .pokelocation.entrypoke -textvariable pokelocation -width 30
button .pokelocation.pokedefaultbtt -text "Default Loc" -command POKEDEFAULT -width 15
pack .pokelocation -fill x
grid .pokelocation.labelpoke -colum 1 -row 1
grid .pokelocation.entrypoke -colum 2 -row 1
grid .pokelocation.pokedefaultbtt -colum 3 -row 1

label .blankline2 -text "     "
pack .blankline2

frame .buttons
button .buttons.backbutton -text "EXIT" -command "exit" -width 20
pack .buttons
pack .buttons.backbutton -side right
label .blankline3 -text "     "
pack .blankline3



frame .debug
	label .debug.lb1 -textvariable var
	label .debug.lb2 -textvariable var2
	label .debug.lb3 -textvariable var3
	pack .debug -fill x
	grid .debug.lb1 -colum 1 -row 1
	grid .debug.lb2 -colum 2 -row 1
	grid .debug.lb3 -colum 3 -row 1



##############################---##########################################
#*************************************************************************************************************
#**********read the bst state, register of bst for know in which mode is**beam and no beam detection*****
#*************************************************************************************************************


proc READSTATE {} {

	global offsetbst
	global bst
	global beam
	global peeklocation
	global brdheader
	global var
	global var2
	global var3
	global mode
	global offsetmode
	global monitor.stop

	global bc1mxst
	global bc2mxst
	global bcrmxst
	global bcmmxst
	global orb1mxst
	global orb2mxst
	global orbmmxst

	global offsetbc1mxst
	global offsetbc2mxst
	global offsetbcrmxst
	global offsetbcmmxst
	global offsetorb1mxst
	global offsetorb2mxst
	global offsetorbmmxst

	global offsetbc1mxnobm
	global offsetbc2mxnobm
	global offsetbcrmxnobm
	global offsetbcmmxnobm
	global offsetorb1mxnobm
	global offsetorb2mxnobm
	global offsetorbmmxnobm

	global offsetbc1mxbeam
	global offsetbc2mxbeam
	global offsetbcrmxbeam
	global offsetbcmmxbeam
	global offsetorb1mxbeam
	global offsetorb2mxbeam
	global offsetorbmmxbeam

	global bstst
	global offsetbstst

	global bucle
	global bstop

	set nob "No Beam"
	set fill "No Beam"
	set ram "No Beam"
	#sophie 24.01.07 -> change Phy to phy
	set phy "No Beam"

	set bstop 0
	set salida 0
	set counter 0
	while { $salida == 0} {

		update
		if { $bstop == 1 } { set salida 1}
		if { $bucle == 0 } { set salida 1}

#debug
		set counter [expr $counter + 1]
		set var3 $counter
		update

#BST state Check
		set addrpeek $brdheader
		append addrpeek $offsetbstst
		set pipe [open "|$peeklocation $addrpeek 4 2"]
		gets $pipe var
		catch {close $pipe}
		if {[string match *error* $var] > 0 } { set bstst "error"; .bst configure -background blue -foreground red
		} elseif { $var  == "0x00000000" } { set bstst "TTCrx & BST not Ready"; .bst  configure -background red -foreground black
		} elseif { $var  == "0x00000001" } { set bstst "TTCrx & BST Ready"; .bst configure -background green -foreground black
		}

#Beam No beam selection on BST states
		set addrpeek $brdheader
		#append addrpeek $offsetbst changed by sophie 29.01.07
		append addrpeek $offsetmode
		set pipe [open "|$peeklocation $addrpeek 4 2"]
		gets $pipe var
		catch {close $pipe}
		if {[string match *error* $var] > 0 } {
			set mode "error"
		} else {set mode $var}
		set mode [expr $mode + 0]

		if { $mode >  15 } {
			set nob 	"error"
			set fill 	"error"
			set ram 	"error"
			set phy 	"error"
		} else {
			if { $mode >=  8 } {
				set phy 	"Beam"
				set mode [expr $mode - 8]
			} else { 		set phy 	"No Beam" }

			if { $mode >=  4 } {
				set ram 	"Beam"
				set mode [expr $mode - 4]
			} else { 		set ram 	"No Beam" }

			if { $mode >=  2 } {
				set fil 	"Beam"
				set mode [expr $mode - 2]
			} else { 		set fil 	"No Beam" }

			if { $mode >=  1 } {
				set nob 	"Beam"
				set mode [expr $mode - 1]
			} else { 		set nob 	"No Beam" }
		}

#BST state for automatical mode and Beam/No beam Detection
		set addrpeek $brdheader
		append addrpeek $offsetbst
		set pipe [open "|$peeklocation $addrpeek 4 2"]
		gets $pipe var
		catch {close $pipe}


		if {[string match *error* $var] > 0 } {
			set bst "error"
			set beam "Error"
		} elseif { $var  == "0x00000000" } {
			set bst "No Beam"
			set beam $nob
		} elseif {  $var == "0x00000001" } {
			set bst "Filling"
			set beam $fil
		} elseif { $var  == "0x00000002" } {
			set bst "Ramping"
			set beam $ram
		} elseif { $var  == "0x00000003" } {
			set bst "Physics"
			set beam $phy
		} else { set bst "Others"; set beam "Unknown"}
		set var3 $beam

# Beam No Beam Label Color
		if {$beam =="Error"} {
			.mode configure -background blue -foreground red
		} elseif { $beam  == "Beam" } {
			.mode configure -background green -foreground black
		} elseif { $beam  == "No Beam" } {
			.mode configure -background red -foreground black
		}

#Reading state of the multiplexor in current state (beam or no beam)
		if { $beam == "No Beam" } {
			set offsetbc1mxst		$offsetbc1mxnobm
			set offsetbc2mxst		$offsetbc2mxnobm
			set offsetbcrmxst		$offsetbcrmxnobm
			set offsetbcmmxst		$offsetbcmmxnobm
			set offsetorb1mxst	$offsetorb1mxnobm
			set offsetorb2mxst	$offsetorb2mxnobm
			set offsetorbmmxst	$offsetorbmmxnobm
			set var2 "no beam in"
		} elseif { $beam == "Beam" } {
			set offsetbc1mxst		$offsetbc1mxbeam
			set offsetbc2mxst		$offsetbc2mxbeam
			set offsetbcrmxst		$offsetbcrmxbeam
			set offsetbcmmxst		$offsetbcmmxbeam
			set offsetorb1mxst	$offsetorb1mxbeam
			set offsetorb2mxst	$offsetorb2mxbeam
			set offsetorbmmxst	$offsetorbmmxbeam
			set var2 "beam in"
		}
#Reading state of  the multiplexors in current state (beam or no beam)
		set addrpeek $brdheader; 	append addrpeek $offsetbc1mxst;
		set pipe [open "|$peeklocation $addrpeek 4 2"]; 	gets $pipe var;	 catch {close $pipe}
		if {[string match *error* $var] > 0 } { set bc1mxst "error"
		} elseif { $var == 0x00000001} 		{ set bc1mxst "ext"
		} elseif { $var == 0x00000000} 		{ set bc1mxst "int"}
			set addrpeek $brdheader; 	append addrpeek $offsetbc2mxst;
		set pipe [open "|$peeklocation $addrpeek 4 2"]; 	gets $pipe var;	 catch {close $pipe}
		if {[string match *error* $var] > 0 } { set bc2mxst "error"
		} elseif { $var == 0x00000001} 		{ set bc2mxst "ext"
		} elseif { $var == 0x00000000} 		{ set bc2mxst "int"}
			set addrpeek $brdheader; 	append addrpeek $offsetbcrmxst;
		set pipe [open "|$peeklocation $addrpeek 4 2"]; 	gets $pipe var;	 catch {close $pipe}
		if {[string match *error* $var] > 0 } { set bcrmxst "error"
		} elseif { $var == 0x00000001} 		{ set bcrmxst "ext"
		} elseif { $var == 0x00000000} 		{ set bcrmxst "int"}
			set addrpeek $brdheader; 	append addrpeek $offsetbcmmxst;
		set pipe [open "|$peeklocation $addrpeek 4 2"]; 	gets $pipe var;	 catch {close $pipe}
		if {[string match *error* $var] > 0 } { set bcmmxst "error"
		} elseif { $var == 0x00000003} 		{ set bcmmxst "Bc1"
		} elseif { $var == 0x00000002} 		{ set bcmmxst "Bc2"
		} elseif { $var == 0x00000001} 		{ set bcmmxst "BcRef"
		} elseif { $var == 0x00000000} 		{ set bcmmxst "int"}
			set addrpeek $brdheader; 	append addrpeek $offsetorb1mxst;
		set pipe [open "|$peeklocation $addrpeek 4 2"]; 	gets $pipe var;	 catch {close $pipe}
		if {[string match *error* $var] > 0 } { set orb1mxst "error"
		} elseif { $var == 0x00000001} 		{ set orb1mxst "ext"
		} elseif { $var == 0x00000000} 		{ set orb1mxst "int"}
			set addrpeek $brdheader; 	append addrpeek $offsetorb2mxst;
		set pipe [open "|$peeklocation $addrpeek 4 2"]; 	gets $pipe var;	 catch {close $pipe}
		if {[string match *error* $var] > 0 } { set orb2mxst "error"
		} elseif { $var == 0x00000001} 		{ set orb2mxst "ext"
		} elseif { $var == 0x00000000} 		{ set orb2mxst "int"}
			set addrpeek $brdheader; 	append addrpeek $offsetorbmmxst;
		set pipe [open "|$peeklocation $addrpeek 4 2"]; 	gets $pipe var;	 catch {close $pipe}
		if {[string match *error* $var] > 0 } { set orbmmxst "error"
		} elseif { $var == 0x00000000} 		{ set orbmmxst "Orb1"
		} elseif { $var == 0x00000001} 		{ set orbmmxst "Orb2"
		} elseif { $var == 0x00000002} 		{ set orbmmxst "int"}

		update
	}
}

proc PEEKDEFAULT {} {

	global peeklocation
	set peeklocation {/home/vme_peek}

}

proc POKEDEFAULT {} {

	global pokelocation
	set pokelocation {/home/vme_poke}

}
