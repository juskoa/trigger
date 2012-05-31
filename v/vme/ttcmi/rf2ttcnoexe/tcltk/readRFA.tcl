#las modification 30/11/2006
#/usr/local/bin/wish

wm title . "Read RF_RX_A & RF_TX_A Console     Angel Monera"

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
	if {[string match *1331* $idstatus] > 0 } {
	set idstatus "TX_A FOUND!!"
	} elseif {[string match *1332* $idstatus] > 0 } {
	set idstatus "RX_A FOUND!!"
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


#blank line
label .sp1 -text ""
pack .sp1



#Blank line
label .sp3 -text ""
pack .sp3

#Power Monitor
set pwr(ch1)  "Press Read!"
set pwr(ch2)  "Press Read!"
set pwraddr(ch1)	0004
set pwraddr(ch2)	0006


frame .pwr
frame .pwr.ch1
label .pwr.ch1.lbl -text "Ch1 Pwr" -width 10
label .pwr.ch1.pwr -relief sunken -textvariable pwr(ch1) -width 30
frame .pwr.ch2
label .pwr.ch2.lbl -text "Ch2 Pwr" -width 10
label .pwr.ch2.pwr -relief sunken -textvariable pwr(ch2) -width 30

button .pwr.btt -text "Read Pwr" -command READPWR

pack .pwr -fill x
grid .pwr.ch1 -row 1
grid .pwr.ch1.lbl -row 1 -colum 1
grid .pwr.ch1.pwr -row 1 -colum 2
grid .pwr.ch2 -row 2
grid .pwr.ch2.lbl -row 2 -colum 1
grid .pwr.ch2.pwr -row 2 -colum 2
grid .pwr.btt -row 4
#Blank line
label .sp4 -text ""
pack .sp4


label .blankline -text "     "
pack .blankline

label .linepwth -text "Power Threshold will only afect to the Front Pannel Leds "
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

		foreach item [ array names refaddr *] {
			# Pipe generated for avoid the erros that produce vme_peek when finish
			set addrpeek $brdheader
			append addrpeek $refaddr($item)
			set pipe [open "|$peeklocation $addrpeek 2 1"]
			gets $pipe rdrefdata($item)
			if {[string match *error* $rdrefdata($item)] > 0 } {
				set rdrefdata($item) "Error"
				} else {
				if { $item == "reg1" } 		{ set value1  [expr ( 6 + ($rdrefdata($item) *4.7))/1000];	append rdrefdata($item) "  (" $value1 " Volts)"
				} elseif { $item == "reg2" }	{ set value2  [expr ( 9 + ($rdrefdata($item)*4.7))/1000];	append rdrefdata($item) "  (" $value2 " Volts)"
				} elseif { $item == "reg3" }	{ set value3  [expr (15 + ($rdrefdata($item)*4.7))/1000];	append rdrefdata($item) "  (" $value3 " Volts)"
				}
			}
			catch {close $pipe}
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
