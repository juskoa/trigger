#las modification  11/01/2007 - 19
#/usr/local/bin/wish

wm title . "RF_2_TTC Beam/No Beam Set Up Panel     Angel Monera - Sophie Baron"

#variables and constants
#variables and constants
#variables and constants
#variables and constants

set idstatus "Click in identify!!"
set brdheader 	0x00
set offsetmode 7FA7C

set peeklocation "vme_peek"
set pokelocation "vme_poke"
set var 		"debug 1"
set var2 		"debug 2"
set var3 		"debug 3"

#END VARIABLES
#END VARIABLES
#END VARIABLES
#END VARIABLES


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


#------------------------

set bit0 3
set bit1 3
set bit2 3
set bit3 3
set bit4 3
set bit5 3
set bit6 3
set bit7 3


frame .beamsel
pack .beamsel -fill x
	label .beamsel.lb -text "Beam and No Beam selector"
	pack .beamsel.lb -fill x


	frame .beamsel.bit0
	pack .beamsel.bit0 -fill x
		label .beamsel.bit0.lb -text "No Beam Mode" -width 15
		radiobutton .beamsel.bit0.bm -variable bit0 -text "Beam" -width 10 -value 1
		radiobutton .beamsel.bit0.nb -variable bit0 -text "No Beam"  -width 10 -value 0
		grid .beamsel.bit0.lb -colum 1 -row 1
		grid .beamsel.bit0.bm -colum 2 -row 1
		grid .beamsel.bit0.nb -colum 3 -row 1

	frame .beamsel.bit1
	pack .beamsel.bit1 -fill x
		label .beamsel.bit1.lb -text "Filling" -width 15
		radiobutton .beamsel.bit1.bm -variable bit1 -text "Beam" -width 10 -value 1
		radiobutton .beamsel.bit1.nb -variable bit1 -text "No Beam"  -width 10 -value 0
		grid .beamsel.bit1.lb -colum 1 -row 1
		grid .beamsel.bit1.bm -colum 2 -row 1
		grid .beamsel.bit1.nb -colum 3 -row 1

	frame .beamsel.bit2
	pack .beamsel.bit2 -fill x
		label .beamsel.bit2.lb -text "Ramping" -width 15
		radiobutton .beamsel.bit2.bm -variable bit2 -text "Beam" -width 10 -value 1
		radiobutton .beamsel.bit2.nb -variable bit2 -text "No Beam"  -width 10 -value 0
		grid .beamsel.bit2.lb -colum 1 -row 1
		grid .beamsel.bit2.bm -colum 2 -row 1
		grid .beamsel.bit2.nb -colum 3 -row 1

	frame .beamsel.bit3
	pack .beamsel.bit3 -fill x
		label .beamsel.bit3.lb -text "Physics" -width 15
		radiobutton .beamsel.bit3.bm -variable bit3 -text "Beam" -width 10 -value 1
		radiobutton .beamsel.bit3.nb -variable bit3 -text "No Beam"  -width 10 -value 0
		grid .beamsel.bit3.lb -colum 1 -row 1
		grid .beamsel.bit3.bm -colum 2 -row 1
		grid .beamsel.bit3.nb -colum 3 -row 1

	frame .beamsel.bit4
	pack .beamsel.bit4 -fill x
		label .beamsel.bit4.lb -text "Bit4" -width 15
		radiobutton .beamsel.bit4.bm -variable bit4 -text "Beam" -width 10 -value 1
		radiobutton .beamsel.bit4.nb -variable bit4 -text "No Beam"  -width 10 -value 0
		grid .beamsel.bit4.lb -colum 1 -row 1
		grid .beamsel.bit4.bm -colum 2 -row 1
		grid .beamsel.bit4.nb -colum 3 -row 1

	frame .beamsel.bit5
	pack .beamsel.bit5 -fill x
		label .beamsel.bit5.lb -text "Bit5" -width 15
		radiobutton .beamsel.bit5.bm -variable bit5 -text "Beam" -width 10 -value 1
		radiobutton .beamsel.bit5.nb -variable bit5 -text "No Beam"  -width 10 -value 0
		grid .beamsel.bit5.lb -colum 1 -row 1
		grid .beamsel.bit5.bm -colum 2 -row 1
		grid .beamsel.bit5.nb -colum 3 -row 1

	frame .beamsel.bit6
	pack .beamsel.bit6 -fill x
		label .beamsel.bit6.lb -text "Bit6" -width 15
		radiobutton .beamsel.bit6.bm -variable bit6 -text "Beam" -width 10 -value 1
		radiobutton .beamsel.bit6.nb -variable bit6 -text "No Beam"  -width 10 -value 0
		grid .beamsel.bit6.lb -colum 1 -row 1
		grid .beamsel.bit6.bm -colum 2 -row 1
		grid .beamsel.bit6.nb -colum 3 -row 1

	frame .beamsel.bit7
	pack .beamsel.bit7 -fill x
		label .beamsel.bit7.lb -text "Bit7" -width 15
		radiobutton .beamsel.bit7.bm -variable bit7 -text "Beam" -width 10 -value 1
		radiobutton .beamsel.bit7.nb -variable bit7 -text "No Beam"  -width 10 -value 0
		grid .beamsel.bit7.lb -colum 1 -row 1
		grid .beamsel.bit7.bm -colum 2 -row 1
		grid .beamsel.bit7.nb -colum 3 -row 1




#*************************************************************************************************************
#---- Read ALL section
#---- Read ALL section

		button .rdall -text "Read" -command {
								set addrpeek $brdheader
								append addrpeek $offsetmode
								set pipe [open "|$peeklocation $addrpeek 4 2"]
								gets $pipe var
								catch {close $pipe}
								if {[string match *error* $var] > 0 } {
									set mode "error"

								} else {
									set mode $var
									set mode [expr $mode + 0]
									if { $mode >  256 } {
										set bit0 	"error"
										set bit1 	"error"
										set bit2 	"error"
										set bit3 	"error"
										set bit4 	"error"
										set bit5 	"error"
										set bit6 	"error"
										set bit7 	"error"

									} else {
										if { $mode >=  128 } {
											set bit7 	1
											set mode [expr $mode - 128]
										} else { 		set bit7 	0 }

										if { $mode >=  64 } {
											set bit6 	1
											set mode [expr $mode - 64]
										} else { 		set bit6 	0 }

										if { $mode >=  32 } {
											set bit5 	1
											set mode [expr $mode - 32]
										} else { 		set bit5 	0 }

										if { $mode >= 16 } {
											set bit4 	1
											set mode [expr $mode - 16]
										} else { 		set bit4 	0 }

										if { $mode >=  8 } {
											set bit3 	1
											set mode [expr $mode - 8]
										} else { 		set bit3 	0 }


										if { $mode >=  4 } {
											set bit2 	1
											set mode [expr $mode - 4]
										} else { 		set bit2 	0 }

										if { $mode >=  2 } {
											set bit1 	1
											set mode [expr $mode - 2]
										} else { 		set bit1 	0 }

										if { $mode >=  1 } {
											set bit0 	1
											set mode [expr $mode - 1]
										} else { 		set bit0 	0 }
									}
								}
		}
		pack .rdall -fill x

		button .wtall -text "Write" -command {
			set total [expr $bit0 * 1 + $bit1 * 2 + $bit2 *  4 + $bit3 * 8+ $bit4 *16 + $bit5 * 32 + $bit6 *64 + $bit7 *128 ]
			set dataex [format %x $total]
			set data 0x00
			append data $dataex
			set addrpoke $brdheader;	append addrpoke $offsetmode;	exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;
		}
		pack .wtall -fill x






#blank line
label .sp4 -text ""
pack .sp4


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




proc PEEKDEFAULT {} {

	global peeklocation
	set peeklocation {/home/vme_peek}

}

proc POKEDEFAULT {} {

	global pokelocation
	set pokelocation {/home/vme_poke}

}
