#las modification 30/11/200610/01/2007  15h
#/usr/local/bin/wish

wm title . "RF SCAN console     Angel Monera"

label .sp05 -text ""
pack .sp05
				frame .btts
				button .btts.read 	-text "Generic Reader" -command { exec wish read.tcl & }
				button .btts.write 	-text "Generic Writer" -command { exec wish write.tcl & }
				button .btts.txd 		-text "RF_TX_D Reader" -command { exec wish readTXD.tcl & }
				button .btts.rxd 		-text "RF_RX_D Reader" -command { exec wish readRXD.tcl & }
				button .btts.rxdmod -text "RX_MODD Reader" -command { exec wish readRXDMOD.tcl & }
				button .btts.rfa	 	-text "TX_RX_A Reader" -command { exec wish readRFA.tcl & }
				button .btts.sp		-text "RF2TTC Reader" -command { exec wish rf2ttc.tcl &}
				pack .btts -fill x
				grid .btts.read -colum 1 -row 1
				grid .btts.write -colum 2 -row 1
				grid .btts.txd -colum 3 -row 1
				grid .btts.rxd -colum 4 -row 1
				grid .btts.rxdmod -colum 5 -row 1
				grid .btts.rfa -colum 6 -row 1
				grid .btts.sp -colum 7 -row 1

label .sp06 -text ""
pack .sp06



set datamode 2
set accmode 1
label .scanposition -textvariable address -width 5
button .scanbtt -text "SCAN" -command {

 #Destroing all the data generated before generate again
	for {set j 0} {$j < 256 } {incr j} {
		set i [expr $j * 1]
		destroy .board($i)
	}

# Redimensionate window
	destroy .blanck
	label .blanck -text " "
	pack .blanck

#for loop to generate the board addres (actually using 8 bits 256 address). in the next lines including counter to know the actual read and know when the scan is done and
#format translator from decimal to hex
	for {set j 0} {$j < 256 } {incr j} {
		set i [expr $j * 1]
		set debug3 $i
		set address "Scaning Address: "
		append address $j
		set addr($i) "0x"
		append addr($i) [format %x $i] "0000"

		set debug1  "|$peeklocation $addr($i) 2 1"
		set pipe [open "|$peeklocation $addr($i) 2 1"]
		gets $pipe idname
		catch {close $pipe}
		set debug2 $idname
		update

#Identifiin the response in the offset  0
		if {[string match *1331* $idname] > 0 } {
			set id($i) "TX_A FOUND!!"
				frame .board($i)
				label .board($i).name -textvariable "id($i)" -width 20
				label .board($i).addr -textvariable "addr($i)" -width 20
				button .board($i).btt -text "Launch RF_A Reader" -command { exec wish readRFA.tcl & }
				pack .board($i) -fill x
				grid .board($i).name -colum 1 -row 1
				grid .board($i).addr -colum 2 -row 1
				grid .board($i).btt -colum 3 -row 1
		} elseif {[string match *1332* $idname] > 0 } {
			set id($i) "RX_A FOUND!!"
				frame .board($i)
				label .board($i).name -textvariable "id($i)" -width 20
				label .board($i).addr -textvariable "addr($i)" -width 20
				button .board($i).btt -text "Launch RF_A Reader" -command { exec wish readRFA.tcl & }
				pack .board($i) -fill x
				grid .board($i).name -colum 1 -row 1
				grid .board($i).addr -colum 2 -row 1
				grid .board($i).btt -colum 3 -row 1
		} elseif {[string match *1380* $idname] > 0 } {
			set id($i) "TX_D FOUND!!"
				frame .board($i)
				label .board($i).name -textvariable "id($i)" -width 20
				label .board($i).addr -textvariable "addr($i)" -width 20
				button .board($i).btt -text "Launch Tx_D Reader" -command { exec wish readTXD.tcl & }
				pack .board($i) -fill x
				grid .board($i).name -colum 1 -row 1
				grid .board($i).addr -colum 2 -row 1
				grid .board($i).btt -colum 3 -row 1
		} elseif {[string match *1382* $idname] > 0 } {
			set id($i) "RX_D FOUND!!"
				frame .board($i)
				label .board($i).name -textvariable "id($i)" -width 20
				label .board($i).addr -textvariable "addr($i)" -width 20
				button .board($i).btt -text "Launch Rx_D Reader" -command { exec wish readRXD.tcl & }
				pack .board($i) -fill x
				grid .board($i).name -colum 1 -row 1
				grid .board($i).addr -colum 2 -row 1
				grid .board($i).btt -colum 3 -row 1
		} elseif {[string match *2831* $idname] > 0 } {
			set id($i) "RX_D_MOD FOUND!!"
				frame .board($i)
				label .board($i).name -textvariable "id($i)" -width 20
				label .board($i).addr -textvariable "addr($i)" -width 20
				button .board($i).btt -text "Launch MODD Reader" -command { exec wish readRXDMOD.tcl & }
				pack .board($i) -fill x
				grid .board($i).name -colum 1 -row 1
				grid .board($i).addr -colum 2 -row 1
				grid .board($i).btt -colum 3 -row 1
#		} elseif {[string match *0030* $idname] > 0 } {
#			set id($i) "RF2TTC_MOD FOUND!!"
#				frame .board($i)
#				label .board($i).name -textvariable "id($i)" -width 20
#				label .board($i).addr -textvariable "addr($i)" -width 20
#				button .board($i).btt -text "Launch Rf2Tcc Reader" -command { exec wish rf2ttc.tcl & }
#				pack .board($i) -fill x
#				grid .board($i).name -colum 1 -row 1
#				grid .board($i).addr -colum 2 -row 1
#				grid .board($i).btt -colum 3 -row 1		
		} elseif {[string match *error* $idname] == 0 } {
				set id($i) "Unknown Board"
				frame .board($i)
				label .board($i).name -textvariable "id($i)" -width 20
				label .board($i).addr -textvariable "addr($i)" -width 20
				button .board($i).btt -text "Launch GEN Reader" -command { exec wish read.tcl & }
				pack .board($i) -fill x
				grid .board($i).name -colum 1 -row 1
				grid .board($i).addr -colum 2 -row 1
				grid .board($i).btt -colum 3 -row 1
		} else {
			set addr($i) "0x"
			append addr($i) [format %x $i] "00000"
			set debug1  "|$peeklocation $addr($i) 4 2"
			set pipe [open "|$peeklocation $addr($i) 4 2"]
			gets $pipe idname
			catch {close $pipe}
			set debug2 $idname
			update
# if no response in a24/d16 we change to a32/d32 to identify the RF 2 TTC board (reading in a DAC registers
			if {[string match *080030* $idname] > 0 } {
				set id($i) "RF 2 TTC BOARD"
				frame .board($i)
				label .board($i).name -textvariable "id($i)" -width 20
				label .board($i).addr -textvariable "addr($i)" -width 20
				button .board($i).btt -text "Rf2Tcc Reader" -command { exec wish rf2ttc.tcl & }
				pack .board($i) -fill x
				grid .board($i).name -colum 1 -row 1
				grid .board($i).addr -colum 2 -row 1
				grid .board($i).btt -colum 3 -row 1
			} else {
				set idname "NOT FOUND!!"
			}
#
		}
	update
	}
}
pack .scanbtt -fill x
pack .scanposition -fill x
label .blankline -text "     "
pack .blankline
frame .buttons
button .buttons.backbutton -text "EXIT" -width 20 -command "exit"
pack .buttons
pack .buttons.backbutton -side right



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


proc PEEKDEFAULT {} {

	global peeklocation
	set peeklocation {/home/vme_peek}

}

