#las modification 30/11/2006
#/usr/local/bin/wish

wm title . "Read RF_TX_D Console     Angel Monera"

#blank line

label .sp05 -text ""
pack .sp05

set idstatus "Click in identify!!"
set brdheader 0x00
frame .brdheader
label .brdheader.labell -text "Board Address"
entry .brdheader.entryy -textvariable brdheader -width 5
label .brdheader.identify -relief sunken -textvariable idstatus -width 30
button .brdheader.buttonn -text "Identify" -command {
	set addrpeek $brdheader
	append addrpeek "0000"
	set pipe [open "|$peeklocation $addrpeek 2 1"]
	gets $pipe idstatus
	if {[string match *1380* $idstatus] > 0 } {
	set idstatus "TX_D FOUND!!"
	} else {set idstatus "NOT FOUND!! :("}
	catch {close $pipe}
}
pack .brdheader -fill x
grid .brdheader.labell -colum 1 -row 1
grid .brdheader.entryy -colum 2 -row 1
grid .brdheader.buttonn -colum 3 -row 1
grid .brdheader.identify -colum 4 -row 1

#blank line
label .sp0 -text ""
pack .sp0


#set rmodedelay 1.0
#frame .rmode
#label .rmode.emptylb -text "  "
#label .rmode.rdelaylb -text "Break Time(s)"
#entry .rmode.rdelayentry -relief sunken -textvariable rmodedelay -width 5
#pack .rmode -fill x
#grid .rmode.emptylb -colum 5 -row 1
#grid .rmode.rdelaylb -colum 6 -row 1
#grid .rmode.rdelayentry -colum 7 -row 1

#blank line
label .sp1 -text ""
pack .sp1

#blank line
label .wrtittle1 -text " Set up (V.Ref) for change the Comparator Reference"
label .wrtittle2 -text "(0x0000 if sinusoidal or square, and > 0x0050 if pulse)"
pack .wrtittle1
pack .wrtittle2

#blank line
label .sp2 -text ""
pack .sp2


#vref frame   - Write section-----------------------------------------------------------------------------------------------
frame .refs
pack .refs



#names
set refaddr(reg1) 0010
set refaddr(reg2) 0012
set refaddr(reg3) 0014
#address
set refname(reg1) "Vref CH1"
set refname(reg2) "Vref CH2"
set refname(reg3) "Vref CH3"
#data for write
set refdata(reg1) "0x0000"
set refdata(reg2) "0x0000"
set refdata(reg3) "0x0000"
#data for read
set rdrefdata(reg1) "Press Read!"
set rdrefdata(reg2) "Press Read!"
set rdrefdata(reg3) "Press Read!"



frame .refs.r1
label .refs.r1.lbl -textvariable refname(reg1) -padx 0 -width 10
label .refs.r1.refaddr(reg1) -textvariable refaddr(reg1) -width 10
entry .refs.r1.refdata(reg1) -relief sunken -textvariable refdata(reg1) -width 10
button .refs.r1.refbtt -text "WRITE" -command {
	set addrpeek $brdheader
	append addrpeek $refaddr(reg1)
	exec vme_poke $addrpeek $refdata(reg1) 2 1
	READPWR
	READREF
	}

	grid .refs.r1 -row 1
grid .refs.r1.lbl -colum 1 -row 1
#grid .refs.r1.refaddr(reg1) -colum 2 -row 1
grid .refs.r1.refdata(reg1) -colum 3 -row 1
grid .refs.r1.refbtt -colum 4 -row 1

frame .refs.r2
label .refs.r2.lbl -textvariable refname(reg2) -padx 0 -width 10
label .refs.r2.refaddr -textvariable refaddr(reg2) -width 10
entry .refs.r2.refdata -relief sunken -textvariable refdata(reg2) -width 10
button .refs.r2.refbtt -text "WRITE" -command {
	set addrpeek $brdheader
	append addrpeek $refaddr(reg2)
	exec vme_poke $addrpeek $refdata(reg2) 2 1
	READPWR
	READREF
	}
grid .refs.r2 -row 2
grid .refs.r2.lbl -colum 1 -row 2
#grid .refs.r2.refaddr -colum 2 -row 2
grid .refs.r2.refdata -colum 3 -row 2
grid .refs.r2.refbtt -colum 4 -row 2

frame .refs.r3
label .refs.r3.lbl -textvariable refname(reg3) -padx 0 -width 10
label .refs.r3.refaddr -textvariable refaddr(reg3) -width 10
entry .refs.r3.refdata -relief sunken -textvariable refdata(reg3) -width 10
button .refs.r3.refbtt -text "WRITE" -command {
	set addrpeek $brdheader
	append addrpeek $refaddr(reg3)
	exec vme_poke $addrpeek $refdata(reg3) 2 1
	READPWR
	READREF
	}
grid .refs.r3 -row 3
grid .refs.r3.lbl -colum 1 -row 3
#grid .refs.r3.refaddr -colum 2 -row 3
grid .refs.r3.refdata -colum 3 -row 3
grid .refs.r3.refbtt -colum 4 -row 3

label .refs.infovrefswr -text "Each unit has an equivalent 4.7mV (aprox)"
grid .refs.infovrefswr -row 4

#blank line
label .refs.sp1 -text ""
grid .refs.sp1 -row 5


#rdvref frame   - Read section-----------------------------------------------------------------------------------------------

frame .rdrefs
pack .rdrefs


frame .rdrefs.r1
label .rdrefs.r1.rdrefr1lbl -textvariable refname(reg1) -padx 0 -width 10
label .rdrefs.r1.rdrefaddr -textvariable refaddr(reg1) -width 10
label .rdrefs.r1.rdrefdata -relief sunken -textvariable rdrefdata(reg1) -width 30
grid .rdrefs.r1 -row 6
grid .rdrefs.r1.rdrefr1lbl -colum 1 -row 1
#grid .rdrefs.r1.rdrefaddr -colum 3 -row 1
grid .rdrefs.r1.rdrefdata -colum 4 -row 1

frame .rdrefs.r2
label .rdrefs.r2.rdrefr2lbl -textvariable refname(reg2) -padx 0 -width 10
label .rdrefs.r2.rdrefaddr -textvariable refaddr(reg2) -width 10
label .rdrefs.r2.rdrefdata -relief sunken -textvariable rdrefdata(reg2) -width 30
grid .rdrefs.r2 -row 7
grid .rdrefs.r2.rdrefr2lbl -colum 1 -row 1
#grid .rdrefs.r2.rdrefaddr -colum 3 -row 1
grid .rdrefs.r2.rdrefdata -colum 4 -row 1

frame .rdrefs.r3
label .rdrefs.r3.rdrefr3lbl -textvariable refname(reg3) -padx 0 -width 10
label .rdrefs.r3.rdrefaddr -textvariable refaddr(reg3) -width 10
label .rdrefs.r3.rdrefdata -relief sunken -textvariable rdrefdata(reg3) -width 30
#label .rdrefs.r3.rdrefdatamv -textvariable rdrefdatamv(reg3) -width 10
grid .rdrefs.r3 -row 8
grid .rdrefs.r3.rdrefr3lbl -colum 1 -row 1
#grid .rdrefs.r3.rdrefaddr -colum 3 -row 1
grid .rdrefs.r3.rdrefdata -colum 4 -row 1

button .rdrefs.readbtt -text "READ Refs" -command READREF
grid .rdrefs.readbtt -row 10

#label .rdrefs.infovrefsrd -text "Each unit has an equivalent 5mV (aprox)"
#grid .rdrefs.infovrefsrd -row 9

#Blank line
label .sp3 -text ""
pack .sp3

#Power Monitor
set pwr(ch1)  "Press Read!"
set pwr(ch2)  "Press Read!"
set pwr(ch3)  "Press Read!"
set pwraddr(ch1)	000A
set pwraddr(ch2)	000C
set pwraddr(ch3)	000E


frame .pwr
frame .pwr.ch1
label .pwr.ch1.lbl -text "Ch1 Pwr" -width 10
label .pwr.ch1.pwr -relief sunken -textvariable pwr(ch1) -width 30
frame .pwr.ch2
label .pwr.ch2.lbl -text "Ch2 Pwr" -width 10
label .pwr.ch2.pwr -relief sunken -textvariable pwr(ch2) -width 30
frame .pwr.ch3
label .pwr.ch3.lbl -text "Ch3 Pwr" -width 10
label .pwr.ch3.pwr -relief sunken -textvariable pwr(ch3) -width 30

button .pwr.btt -text "Read Pwr" -command READPWR

pack .pwr -fill x
grid .pwr.ch1 -row 1
grid .pwr.ch1.lbl -row 1 -colum 1
grid .pwr.ch1.pwr -row 1 -colum 2
grid .pwr.ch2 -row 2
grid .pwr.ch2.lbl -row 2 -colum 1
grid .pwr.ch2.pwr -row 2 -colum 2
grid .pwr.ch3 -row 3
grid .pwr.ch3.lbl -row 3 -colum 1
grid .pwr.ch3.pwr -row 3 -colum 2
grid .pwr.btt -row 4
#Blank line
label .sp4 -text ""
pack .sp4


label .blankline -text "     "
pack .blankline

label .linepwth -text "Power Threshold will afect only to the Front Pannel Leds "
pack .linepwth

set pwthreshold 0x00000050
set pwthresholdaddr 0002

frame .pwth
label .pwth.lbl -text "Power Threshold Value" -padx 0 -width 20
entry .pwth.ent -relief sunken -textvariable pwthreshold -width 10
button .pwth.rdbtt -text "READ" -command {
	set addrpeek $brdheader
	append addrpeek $pwthresholdaddr
	set pipe [open "|$peeklocation $addrpeek 2 1"]
	gets $pipe pwthreshold
	if {[string match *error* $idstatus] > 0 } {
				tk_messageBox -type ok -message "Error reading, Check BoardAddres with Identify Button"
	}
	catch {close $pipe}
}
button .pwth.wrbtt -text "WRITE" -command {
	set addrpeek $brdheader
	append addrpeek $pwthresholdaddr
	exec vme_poke $addrpeek $refdata(reg1) 2 1
}
pack .pwth -fill x
grid .pwth.lbl -row 1 -colum 1
grid .pwth.ent -row 1 -colum 2
grid .pwth.rdbtt -row 1 -colum 3
grid .pwth.wrbtt -row 1 -colum 4

label .blankline2 -text "     "
pack .blankline2



set peeklocation "vme_peek"
frame .peeklocation
label .peeklocation.labelpeek -text "Vme_Peek Location: " -width 20
entry .peeklocation.entrypeek -textvariable peeklocation -width 30
button .peeklocation.peekdefaultbtt -text "Default Loc" -command PEEKDEFAULT -width 15
pack .peeklocation -fill x
pack .peeklocation.labelpeek -side left
pack .peeklocation.peekdefaultbtt -side right
pack .peeklocation.entrypeek -side right

set pokelocation "vme_poke"
frame .pokelocation
label .pokelocation.labelpoke -text "Vme_Poke Location: " -width 20
entry .pokelocation.entrypoke -textvariable pokelocation -width 30
button .pokelocation.pokedefaultbtt -text "Default Loc" -command POKEDEFAULT -width 15
pack .pokelocation -fill x
pack .pokelocation.labelpoke -side left
pack .pokelocation.pokedefaultbtt -side right
pack .pokelocation.entrypoke -side right

label .blankline23 -text "     "
pack .blankline23

frame .buttons
button .buttons.backbutton -text "EXIT" -command "exit" -width 20
pack .buttons
pack .buttons.backbutton -side right
label .blankline3 -text "     "
pack .blankline3

proc READREF {} {

	global peeklocation
	global rdrefdata
	global refaddr
	global brdheader
	global refdata

		foreach item [ array names refaddr *] {
			# Pipe generated for avoid the erros that produce vme_peek when finish
			set addrpeek $brdheader
			append addrpeek $refaddr($item)
			set pipe [open "|$peeklocation $addrpeek 2 1"]
			gets $pipe rdrefdata($item)
			catch {close $pipe}
			if {[string match *error* $rdrefdata($item)] > 0 } {
				set rdrefdata($item) "Error"
				} else {
				if { $item == "reg1" } 		{ set refdata($item) $rdrefdata($item); set value1  [expr ( 6 + ($rdrefdata($item) *4.7))/1000];	append rdrefdata($item) "  (" $value1 " Volts)"
				} elseif { $item == "reg2" }	{ set refdata($item) $rdrefdata($item); set value2  [expr ( 9 + ($rdrefdata($item)*4.7))/1000];	append rdrefdata($item) "  (" $value2 " Volts)"
				} elseif { $item == "reg3" }	{ set refdata($item) $rdrefdata($item); set value3  [expr (15 + ($rdrefdata($item)*4.7))/1000];	append rdrefdata($item) "  (" $value3 " Volts)"
				}
			}
		}
}


proc READPWR {} {
	global peeklocation
	global pwr
	global pwraddr
	global brdheader

	foreach item [ array names pwraddr *] {
		# Pipe generated for avoid the erros that produce vme_peek when finish
		set addrpeek $brdheader
		append addrpeek $pwraddr($item)
		set pipe [open "|$peeklocation $addrpeek 2 1"]
		gets $pipe data
		if {[string match *error* $data] > 0 } {
			set data "Error"
		} else {
				if { $item == "ch1" } 		{set pwr(ch1) $data; # [expr $data + 0];
				} elseif { $item == "ch2" }	{ set pwr(ch2) $data; # [expr $data + 0];
				} elseif { $item == "ch3" }	{ set pwr(ch3) $data; # [expr $data + 0];
				}
			}
   		}
		catch {close $pipe}
}


proc PEEKDEFAULT {} {

	global peeklocation
	set peeklocation {/home/vme_peek}

}

proc POKEDEFAULT {} {

	global pokelocation
	set pokelocation {/home/vme_poke}

}
