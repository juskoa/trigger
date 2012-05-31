#las modification 30/11/2006
#/usr/local/bin/wish

wm title . "Read RF_RX_D Console     Angel Monera"

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
	if {[string match *1382* $idstatus] > 0 } {
	set idstatus "RX_D FOUND!!"
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
label .wrtittle -text " Set up (V.Ref) for unbalancing the Truelight Receivers"
pack .wrtittle

#blank line
label .sp2 -text ""
pack .sp2


#vref frame   - Write section-----------------------------------------------------------------------------------------------
frame .refs
pack .refs



#names
set refaddr(reg1) 000A
set refaddr(reg2) 000C
set refaddr(reg3) 000E
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
	READFREQ
	READREF
	}
button .refs.r1.refbttzero -text "Set to 0" -command {
	set refdata(reg1) 0x0000
	set addrpeek $brdheader
	append addrpeek $refaddr(reg1)
	exec vme_poke $addrpeek $refdata(reg1) 2 1
	READFREQ
	READREF
	}

grid .refs.r1 -row 1
grid .refs.r1.lbl -colum 1 -row 1
#grid .refs.r1.refaddr(reg1) -colum 2 -row 1
grid .refs.r1.refdata(reg1) -colum 3 -row 1
grid .refs.r1.refbtt -colum 4 -row 1
grid .refs.r1.refbttzero -colum 5 -row 1

frame .refs.r2
label .refs.r2.lbl -textvariable refname(reg2) -padx 0 -width 10
label .refs.r2.refaddr -textvariable refaddr(reg2) -width 10
entry .refs.r2.refdata -relief sunken -textvariable refdata(reg2) -width 10
button .refs.r2.refbtt -text "WRITE" -command {
	set addrpeek $brdheader
	append addrpeek $refaddr(reg2)
	exec vme_poke $addrpeek $refdata(reg2) 2 1
	READFREQ
	READREF
	}
button .refs.r2.refbttzero -text "Set to 0" -command {
	set refdata(reg2) 0x0000
	set addrpeek $brdheader
	append addrpeek $refaddr(reg2)
	exec vme_poke $addrpeek $refdata(reg2) 2 1
	READFREQ
	READREF
		}
grid .refs.r2 -row 2
grid .refs.r2.lbl -colum 1 -row 2
#grid .refs.r2.refaddr -colum 2 -row 2
grid .refs.r2.refdata -colum 3 -row 2
grid .refs.r2.refbtt -colum 4 -row 2
grid .refs.r2.refbttzero -colum 5 -row 2

frame .refs.r3
label .refs.r3.lbl -textvariable refname(reg3) -padx 0 -width 10
label .refs.r3.refaddr -textvariable refaddr(reg3) -width 10
entry .refs.r3.refdata -relief sunken -textvariable refdata(reg3) -width 10
button .refs.r3.refbtt -text "WRITE" -command {
	set addrpeek $brdheader
	append addrpeek $refaddr(reg3)
	exec vme_poke $addrpeek $refdata(reg3) 2 1
	READFREQ
	READREF
	}
button .refs.r3.refbttzero -text "Set to 0" -command {
	set refdata(reg3) 0x0000
	set addrpeek $brdheader
	append addrpeek $refaddr(reg3)
	exec vme_poke $addrpeek $refdata(reg3) 2 1
	READFREQ
	READREF
	}
grid .refs.r3 -row 3
grid .refs.r3.lbl -colum 1 -row 3
grid .refs.r3.refdata -colum 3 -row 3
grid .refs.r3.refbtt -colum 4 -row 3
grid .refs.r3.refbttzero -colum 5 -row 3

label .refs.infovrefswr -text "Each unit has an equivalent 2.8mV (aprox)"
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


#Blank line
label .sp3 -text ""
pack .sp3

#Frequency Detector
set Freq(ch1)  "Press Read!"
set Freq(ch2)  "Press Read!"
set Freq(ch3)  "Press Read!"
set freqaddr(ch1a)	0010
set freqaddr(ch1b)	0012
set freqaddr(ch2a)	0014
set freqaddr(ch2b)	0016
set freqaddr(ch3a)	0018
set freqaddr(ch3b)	001A

frame .freq
frame .freq.ch1
label .freq.ch1.lbl -text "Ch1 Freq" -width 10
label .freq.ch1.freq -relief sunken -textvariable Freq(ch1) -width 30
frame .freq.ch2
label .freq.ch2.lbl -text "Ch2 Freq" -width 10
label .freq.ch2.freq -relief sunken -textvariable Freq(ch2) -width 30
frame .freq.ch3
label .freq.ch3.lbl -text "Ch3 Freq" -width 10
label .freq.ch3.freq -relief sunken -textvariable Freq(ch3) -width 30

button .freq.btt -text "Read Freq" -command READFREQ


pack .freq -fill x
grid .freq.ch1 -row 1
grid .freq.ch1.lbl -row 1 -colum 1
grid .freq.ch1.freq -row 1 -colum 2
grid .freq.ch2 -row 2
grid .freq.ch2.lbl -row 2 -colum 1
grid .freq.ch2.freq -row 2 -colum 2
grid .freq.ch3 -row 3
grid .freq.ch3.lbl -row 3 -colum 1
grid .freq.ch3.freq -row 3 -colum 2
grid .freq.btt -row 4
#Blank line
label .sp4 -text ""
pack .sp4

label .blankline -text "     "
pack .blankline

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

label .blankline2 -text "     "
pack .blankline2

frame .buttons
button .buttons.backbutton -text "EXIT" -command "exit" -width 20
pack .buttons
pack .buttons.backbutton -side right
label .blankline3 -text "     "
pack .blankline3


#label .run -textvariable run -width 15
#pack .run

#set pepe 0
#label .pim -textvariable pepe -width 30
#pack .pim

#set pepeto ada
#label .pepeto -textvariable pepeto
#pack .pepeto

#set pepeto2 dada
#label .pepeto2 -textvariable pepeto2
#pack .pepeto2


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
			if {[string match *error* $rdrefdata($item)] > 0 } {
				set rdrefdata($item) "Error"
				} else {
				if { $item == "reg1" } 		{ set refdata($item) $rdrefdata($item); set value1  [expr 0.006 + 0.002792*($rdrefdata($item))];	append rdrefdata($item) "  (" $value1 " Volts)"
				} elseif { $item == "reg2" }	{ set refdata($item) $rdrefdata($item); set value2  [expr 0.006 + 0.002792*($rdrefdata($item))];	append rdrefdata($item) "  (" $value2 " Volts)"
				} elseif { $item == "reg3" }	{ set refdata($item) $rdrefdata($item); set value3  [expr 0.006 + 0.002792*($rdrefdata($item))];	append rdrefdata($item) "  (" $value3 " Volts)"
				}
			}
			catch {close $pipe}
		}
}


proc READFREQ {} {
	global peeklocation
	global Freq
	global freqaddr
	global brdheader

	foreach item [ array names freqaddr *] {
		# Pipe generated for avoid the erros that produce vme_peek when finish
		set addrpeek $brdheader
		append addrpeek $freqaddr($item)
		set pipe [open "|$peeklocation $addrpeek 2 1"]
		gets $pipe data
		if {[string match *error* $data] > 0 } {
			set data "Error"
		} else {
				if { $item == "ch1a" } 		{ set value1a [expr $data + 0.0];
				} elseif { $item == "ch1b" } 	{ set value1b [expr $data * 65536.0];
				} elseif { $item == "ch2a" }	{ set value2a [expr $data + 0.0];
				} elseif { $item == "ch2b" }	{ set value2b [expr $data * 65536.0];
				} elseif { $item == "ch3a" }	{ set value3a [expr $data + 0.0];
				} elseif { $item == "ch3b" }	{ set value3b [expr $data * 65536.0];
				}
			}
		catch {close $pipe}
   		}


	set Freq(ch1) [expr 32*11 *80.0 / ( $value1a + $value1b)];
	append Freq(ch1) "  MHz"
	set Freq(ch2) [expr 32*11 *80.0 / ($value2a + $value2b)];
	append Freq(ch2) "  MHz"
	set Freq(ch3) [expr 32*11 *80.0 / ($value3a + $value3b)];
	append Freq(ch3) "  MHz"

}



proc PEEKDEFAULT {} {

	global peeklocation
	set peeklocation {/home/vme_peek}

}

proc POKEDEFAULT {} {

	global pokelocation
	set pokelocation {/home/vme_poke}

}
