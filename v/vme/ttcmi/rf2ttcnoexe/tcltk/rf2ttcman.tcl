#las modification  01/02/2007 - 19
#/usr/local/bin/wish


#Comments in the END


wm title . "Read RF_2_TTC Manual Mode Control       Angel Monera - Sophie Baron		03/04/2007"

#variables and constants

set idstatus "press identify!!"
set brdheader 0x00

#-----___-----___-----___-----___-----___-----___
set bc1vref 		"click Read"
set bc1mx 		x
set bc1delay 		"click Read"
set bc1qpllmode 	x
set bc1qpllst 		"click Read"

set offsetbc1vref 		7FBEC
set offsetbc1mx 		7FBFC
set offsetbc1delay 		7D000
set offsetbc1qpllmode 	7FBF0
set offsetbc1qpllst 		7FBE8


set bc2vref 		"click Read"
set bc2mx 		x
set bc2delay 		"click Read"
set bc2qpllmode 	x
set bc2qpllst 		"click Read"

set offsetbc2vref		7FBBC
set offsetbc2mx		7FBCC
set offsetbc2delay		7D004
set offsetbc2qpllmode	7FBC0
set offsetbc2qpllst		7FBB8

set bcrefvref 		"click Read"
set bcrefmx 		x
set bcrefdelay 	"click Read"
set bcrefqpllmode 	x
set bcrefqpllst 	"click Read"

set offsetbcrefvref		7FB9C
set offsetbcrefmx		7FBAC
set offsetbcrefdelay		7D008
set offsetbcrefqpllmode	7FBA0
set offsetbcrefqpllst		7FB98

set offsetfifo25 7D200

set mainselector x
set mqpll x
set stqpll "Press Read"

set offsetmainselector "7FB8C"
set offsetmqpll "7FB80"
set offsetstqpll "7FB7C"
set offsetbcmaindelay		7D00c
#-----___-----___-----___-----___-----___-----___

set offsetworkmode 7FA78
#________--------________________________________
set orb1vref "Press Read"
set orb1sdelay "Press Read"
set orb1select x
set orb1cdelay "Press Read"
set orb1length "Press Read"
set orb1pol x
set orb1fdelay "Press Read"
set orb1intperiode "Press Read"
set orb1percounter "Press Read"
set orb1orbcounter "Press Read"


set offsetorb1vref		7FB3C
set offsetorb1select	7FB6C
set offsetorb1sdelay	7D020
set offsetorb1cdelay	7FB5C
set offsetorb1fdelay	7D040
set offsetorb1lenght	7FB58
set offsetorb1pol		7FB60
set offsetorb1periode			7FB54
set offsetorb1intcnt			7FB50
set offsetorb1percntrdlast	7FB48
set offsetorb1percntrdfifo	7FB40
set offsetorb1orbcntrd		7FB4C



set orb2vref "Press Read"
set orb2sdelay "Press Read"
set orb2select x
set orb2cdelay "Press Read"
set orb2length "Press Read"
set orb2pol x
set orb2fdelay "Press Read"
set orb2intperiode "Press Read"
set orb2percounter "Press Read"
set orb2orbcounter "Press Read"

set offsetorb2vref		7FAFC
set offsetorb2select	7FB2C
set offsetorb2sdelay	7D024
set offsetorb2cdelay	7FB1C
set offsetorb2fdelay	7D044
set offsetorb2lenght	7FB18
set offsetorb2pol		7FB20
set offsetorb2periode			7FB14
set offsetorb2intcnt			7FB10
set offsetorb2percntrdlast	7FB08
set offsetorb2percntrdfifo	7FB00
set offsetorb2orbcntrd		7FB0C


set orbmvref "Press Read"
set orbmsdelay "Press Read"
set orbmselect x
set orbmcdelay "Press Read"
set orbmlength "Press Read"
set orbmpol x
set orbmfdelay "Press Read"
set orbmintperiode "Press Read"
set orbmpercounter "Press Read"
set orbmorbcounter "Press Read"


#NO INT DAC
set offsetorbmselect		7FAEC
#NO INT DELAY
set offsetorbmcdelay		7FADC
set offsetorbmfdelay		7D048
set offsetorbmlenght		7FAD8
set offsetorbmpol			7FAE0
set offsetorbmperiode		7FAD4
set offsetorbmintcnt			7FAD0
set offsetorbmpercntrdlast	7FAC8
set offsetorbmpercntrdfifo	7FAC0
set offsetorbmorbcntrd		7FACC

set offsetintcnten	7FA6C
	set int1en 0
	set int2en 0
	set intmen 0
set offsetorbcnten	7FA68
	set orb1en 0 
	#sophie 18.01.07
	set orb2en 0 
	#sophie 18.01.07
	set orbmen 0 
	#sophie 18.01.07
set offsetpercnten	7FA64
	set per1en 0
	set per2en 0
	set permen 0

set offsetintcntrst	7FA4C
	set int1rst 0
	set int2rst 0
	set intmrst 0
set offsetorbcntrst	7FA44
	set orb1rst 0
	set orb2rst 0
	set orbmrst 0
set offsetpercntrst	7FA48
	set per1rst 0
	set per2rst 0
	set permrst 0

set offsetBset  00010
set offsetBclear 00014

#-----TTCrx----sophie 03.04.07
set ttcrxstatus "Press Read"
set machinemode "Press Read"
set ttcrxinitreg B3

set offsetttcrxreg	7E000
set offsetttcrxdata	7E004
set offsetttcrxrd	7E200
set offsetbstmachinemode 7FA9C

set offsetttcrxinternalstatus 16
set offsetttcrxinternalcontrol 3

set bst			"Read!!"
set offsetbst 	7FA9C

set bstst  "BST status: Press Read!!"
set offsetbstst	7FAA0

set beam 		"Press Read for Beam Status!!"

set mode 		"Read!!"
set offsetmode 7FA7C

set offsetworkmode   7FA78


#________--------________________________________

set peeklocation "vme_peek"
set pokelocation "vme_poke"
set var "debug window"
set var2 "debug window"
set var3 "debug window"

#END VARIABLES

# Panel header and Board identification----------------------------------------------------------------------------------------------------

frame .brdheader
label .brdheader.label -text "Board Address"
entry .brdheader.entry -textvariable brdheader -width 5
label .brdheader.identify -relief sunken -textvariable idstatus -width 30
button .brdheader.button -text "Identify" -command {
	set addrpeek $brdheader; append addrpeek "00000"
	set pipe [open "|$peeklocation $addrpeek 4 2"];	gets $pipe idstatus;
	if {[string match *080030* $idstatus] == 0 } {	set idstatus "Not Found!!";
	} else {set idstatus "FOUND RF2TTC"};	catch {close $pipe};
}

pack .brdheader -fill x
grid .brdheader.label -column 1 -row 1
grid .brdheader.entry -column 2 -row 1
grid .brdheader.button -column 3 -row 1
grid .brdheader.identify -column 4 -row 1

#blank line
label .sp0 -text " "
pack .sp0



#Main buttons-------------******************************************************************************

frame .btts
button .btts.reset -text "Reset Board" -width 15 -command {
						set addrpoke $brdheader;  
						append addrpoke $offsetBset;  
						exec $pokelocation $addrpoke 0x0008 4 2 ; 
						set addrpoke $brdheader;  
						append addrpoke $offsetBclear;  
						exec $pokelocation $addrpoke 0x0008 4 2 
						}
button .btts.qpllrst -text "Reset QPLL" -width 15 -command {
						set addrpoke $brdheader;  
						append addrpoke $offsetBset;  
						exec $pokelocation $addrpoke 0x0002 4 2 ; 
						set addrpoke $brdheader;  
						append addrpoke $offsetBclear;  
						exec $pokelocation $addrpoke 0x0002 4 2 
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
				set addrpoke $brdheader;	append addrpoke $offsetbcmaindelay;	exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;

				set addrpoke $brdheader;	append addrpoke $offsetorb1sdelay;		exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;
				set addrpoke $brdheader;	append addrpoke $offsetorb1fdelay;		exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;
				set addrpoke $brdheader;	append addrpoke $offsetorb2sdelay;		exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;
				set addrpoke $brdheader;	append addrpoke $offsetorb2fdelay;		exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;
				set addrpoke $brdheader;	append addrpoke $offsetorbmfdelay;		exec $pokelocation $addrpoke $data 4 2; set var $data; 	set var2 $addrpoke;
			}

label .btts.blank1 -text "" -width 5
button .btts.manual -text "Change to Manual" -width 15 -background  yellow -command {
				set addrpoke $brdheader;  append addrpoke $offsetworkmode;  exec $pokelocation $addrpoke 0x0000 4 2;
				.btts.manual configure -background green }

pack .btts -fill x
grid .btts.reset -column 1 -row 1
grid .btts.qpllrst -column 2 -row 1
grid .btts.dlyrst -column 3 -row 1
grid .btts.ttcrxrst -column 4 -row 1
grid .btts.dlyini -column 5 -row 1
grid .btts.blank1 -column 6 -row 1
grid .btts.manual -column 7 -row 1



#blank line
label .sp01 -text ""
pack .sp01






#-------------------------------------------****************************************************************
#               BCx SECTION 4 BLOCKS BC.BCX..
#-------------------------------------------****************************************************************
frame .bc
	pack .bc -fill x

	frame .bc.bc1  -borderwidth 1 -relief raised
		grid .bc.bc1 -column 1 -row 1

		frame .bc.bc1.head
			label .bc.bc1.head.lb -text "BC1     ";
			button .bc.bc1.head.rd -text "READ ALL" -command {
				.bc.bc1.vref.rd invoke
				.bc.bc1.mx.rd invoke
				.bc.bc1.delay.rd invoke
				.bc.bc1.qpll.rd invoke
				.bc.bc1.qpllst.rd invoke
			}
			pack .bc.bc1.head -fill x
			grid .bc.bc1.head.lb -column 1 -row 1
			grid .bc.bc1.head.rd -column 2 -row 1

	#blank line
		frame .bc.bc1.vref
		pack .bc.bc1.vref -fill x
			label .bc.bc1.vref.sp -text " "
			pack .bc.bc1.vref.sp -fill x
		
		#frame .bc.bc1.vref
#			label .bc.bc1.vref.lb  -text "     DAC Vref    "  -width 14
#			entry .bc.bc1.vref.ent -textvariable bc1vref -width 14
#			button .bc.bc1.vref.rd -text "Read" -command {
#				set addrpeek $brdheader
#				append addrpeek $offsetbc1vref
#				set pipe [open "|$peeklocation $addrpeek 4 2"]
#				gets $pipe var
#				catch {close $pipe}
#				if {[string match *error* $var] > 0 } {
#					set bc1vref "error"
#				} else {set bc1vref $var}
#			}
#			button .bc.bc1.vref.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetbc1vref;  exec $pokelocation $addrpoke $bc1vref 4 2 }
#			pack .bc.bc1.vref
#			grid .bc.bc1.vref.lb -column 1 -row 1
#			grid .bc.bc1.vref.ent -column 2 -row 1
#			grid .bc.bc1.vref.rd -column 3 -row 1
#			grid .bc.bc1.vref.wt -column 4 -row 1

		frame .bc.bc1.mx
			label .bc.bc1.mx.lb -text "Mx Select"  -width 14
			radiobutton .bc.bc1.mx.int -variable bc1mx -text "int " -value 0x00000000  -width 4
			radiobutton .bc.bc1.mx.ext -variable bc1mx -text "ext " -value 0x00000001  -width 3
			button .bc.bc1.mx.rd 	-text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetbc1mx
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set bc1mx "error"
				} else {set bc1mx $var}
			}
			button .bc.bc1.mx.wt 	-text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetbc1mx;  exec $pokelocation $addrpoke $bc1mx 4 2 }
			pack .bc.bc1.mx  -fill x
			grid .bc.bc1.mx.lb  -column 1 -row 1
			grid .bc.bc1.mx.int  -column 2 -row 1
			grid .bc.bc1.mx.ext  -column 3 -row 1
			grid .bc.bc1.mx.rd -column 4 -row 1
			grid .bc.bc1.mx.wt -column 5 -row 1

		frame .bc.bc1.delay
			label .bc.bc1.delay.lb -text "     Delay 25     "   -width 14
			entry .bc.bc1.delay.ent -textvariable bc1delay -width 14
			button .bc.bc1.delay.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetbc1delay
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				exec sleep 0.02
				set addrpeek $brdheader
				append addrpeek $offsetfifo25
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var2
				catch {close $pipe}
				if {[string match *error* $var2] > 0 } {
					set bc1delay "error"
				} else {
					set var [string range $var2 8 9]
					set var3 0x0;
					append var3 $var;
					set var3 [expr $var3 + 0]
					if {$var3 >  128} {
						set var3 [expr $var3 - 128];
					}
					if { $var3 >= 64 } { .bc.bc1.delay.lb configure -text "  Delay 25( E )  "; set var3 [expr $var3 - 64 ] ;
					} else { .bc.bc1.delay.lb configure -text "  Delay 25( D )  ";  }
					set bc1delay $var3
				}
			}

			button .bc.bc1.delay.wt -text "Write" -command  {
				if { $bc1delay > 63 } {	tk_messageBox -type ok -message "Waka!!! Tas pasao.. Max value 63";  set bc1delay 63}
				set dataex [expr $bc1delay + 64]
				set dataex [format %x $dataex]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetbc1delay;
				exec $pokelocation $addrpoke $data 4 2

				set var $data
				set var2 $addrpoke
				set var3 $data
			}
			pack .bc.bc1.delay -fill x
			grid .bc.bc1.delay.lb -column 1 -row 1
			grid .bc.bc1.delay.ent -column 2 -row 1
			grid .bc.bc1.delay.rd -column 3 -row 1
			grid .bc.bc1.delay.wt -column 4 -row 1

		frame .bc.bc1.qpll
			label .bc.bc1.qpll.lb -text "QPLL Mod"  -width 14
			radiobutton .bc.bc1.qpll.reset -text "Rst  " -variable bc1qpllmode -value 0x00000000  -width 4
			radiobutton .bc.bc1.qpll.lost -text "Lst  " -variable bc1qpllmode -value 0x00000001  -width 3
			button .bc.bc1.qpll.rd	-text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetbc1qpllmode
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set bc1qpllmode "error"
				} else {set bc1qpllmode $var}
			}
			button .bc.bc1.qpll.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetbc1qpllmode;  exec $pokelocation $addrpoke $bc1qpllmode 4 2 }
			pack .bc.bc1.qpll -fill x
			grid .bc.bc1.qpll.lb -column 1 -row 1
		grid .bc.bc1.qpll.reset -column 2 -row 1
		grid .bc.bc1.qpll.lost -column 3 -row 1
		grid .bc.bc1.qpll.rd -column 4 -row 1
		grid .bc.bc1.qpll.wt -column 5 -row 1


	frame .bc.bc1.qpllst
		label .bc.bc1.qpllst.lb -text "QPLL status"  -width 14
		label .bc.bc1.qpllst.st -textvariable bc1qpllst -relief sunken  -width 14
		button .bc.bc1.qpllst.rd -text "Read" -command {
			set addrpeek $brdheader
			append addrpeek $offsetbc1qpllst
			set pipe [open "|$peeklocation $addrpeek 4 2"]
			gets $pipe var
			catch { close $pipe }
			if { [ string match *error* $var ] > 0 } {
				set bc1qpllst "error"
			} elseif { [ string match *00000000* $var ] > 0} {
				set bc1qpllst "PLL ERR"
			} elseif { [ string match *00000001* $var ] > 0} {
				set bc1qpllst "LOCKED"
			} elseif { [ string match *00000002* $var ] > 0} {
				set bc1qpllst "UnLOKED/ERROR"
			} else {
				set bc1qpllst "NPI"
			}
		}
		label .bc.bc1.qpllst.lb2 -text "           "

		pack .bc.bc1.qpllst
		grid .bc.bc1.qpllst.lb -column 1 -row 1
		grid .bc.bc1.qpllst.st -column 2 -row 1
		grid .bc.bc1.qpllst.rd -column 3 -row 1
		grid .bc.bc1.qpllst.lb2 -column 4 -row 1

#******************************************************************************
#******************************************************************************
frame .bc.bc2  -borderwidth 1 -relief raised
	grid .bc.bc2 -column 2 -row 1

	frame .bc.bc2.head
		label .bc.bc2.head.lb -text "BC2     ";
		button .bc.bc2.head.rd -text "READ ALL" -command {
			.bc.bc2.vref.rd invoke
			.bc.bc2.mx.rd invoke
			.bc.bc2.delay.rd invoke
			.bc.bc2.qpll.rd invoke
			.bc.bc2.qpllst.rd invoke
		}
		pack .bc.bc2.head -fill x
		grid .bc.bc2.head.lb -column 1 -row 1
		grid .bc.bc2.head.rd -column 2 -row 1

	#blank line
	frame .bc.bc2.vref
	pack .bc.bc2.vref -fill x
		label .bc.bc2.vref.sp -text ""
		pack .bc.bc2.vref.sp -fill x
	
	#frame .bc.bc2.vref
#			label .bc.bc2.vref.lb  -text "     DAC Vref    "  -width 14
#			entry .bc.bc2.vref.ent -textvariable bc2vref -width 14
#			button .bc.bc2.vref.rd -text "Read" -command {
#				set addrpeek $brdheader
#				append addrpeek $offsetbc2vref
#				set pipe [open "|$peeklocation $addrpeek 4 2"]
#				gets $pipe var
#				catch {close $pipe}
#				if {[string match *error* $var] > 0 } {
#					set bc2vref "error"
#				} else {set bc2vref $var}
#			}
#			button .bc.bc2.vref.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetbc2vref;  exec $pokelocation $addrpoke $bc2vref 4 2 }
#			pack .bc.bc2.vref
#			grid .bc.bc2.vref.lb -column 1 -row 1
#			grid .bc.bc2.vref.ent -column 2 -row 1
#			grid .bc.bc2.vref.rd -column 3 -row 1
#			grid .bc.bc2.vref.wt -column 4 -row 1

	frame .bc.bc2.mx
		label .bc.bc2.mx.lb -text "Mx Select"  -width 14
		radiobutton .bc.bc2.mx.int -variable bc2mx -text "int " -value 0x00000000  -width 4
		radiobutton .bc.bc2.mx.ext -variable bc2mx -text "ext " -value 0x00000001  -width 3
		button .bc.bc2.mx.rd 	-text "Read" -command {
			set addrpeek $brdheader
			append addrpeek $offsetbc2mx
			set pipe [open "|$peeklocation $addrpeek 4 2"]
			gets $pipe var
			catch {close $pipe}
			if {[string match *error* $var] > 0 } {
				set bc2mx "error"
			} else {set bc2mx $var}
		}
		button .bc.bc2.mx.wt 	-text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetbc2mx;  exec $pokelocation $addrpoke $bc2mx 4 2 }
		pack .bc.bc2.mx  -fill x
		grid .bc.bc2.mx.lb  -column 1 -row 1
		grid .bc.bc2.mx.int  -column 2 -row 1
		grid .bc.bc2.mx.ext  -column 3 -row 1
		grid .bc.bc2.mx.rd -column 4 -row 1
		grid .bc.bc2.mx.wt -column 5 -row 1

	frame .bc.bc2.delay
		label .bc.bc2.delay.lb -text "     Delay 25     "   -width 14
		entry .bc.bc2.delay.ent -textvariable bc2delay -width 14
		button .bc.bc2.delay.rd -text "Read" -command {
			set addrpeek $brdheader
			append addrpeek $offsetbc2delay
			set pipe [open "|$peeklocation $addrpeek 4 2"]
			gets $pipe var
			catch {close $pipe}
			exec sleep 0.02
			set addrpeek $brdheader
			append addrpeek $offsetfifo25
			set pipe [open "|$peeklocation $addrpeek 4 2"]
			gets $pipe var2
			catch {close $pipe}
			if {[string match *error* $var2] > 0 } {
				set bc2delay "error"
			} else {
				set var [string range $var2 8 9]
				set var3 0x0;
				append var3 $var;
				set var3 [expr $var3 + 0]
				if {$var3 >  128} {
					set var3 [expr $var3 - 128];
				}
				if { $var3 >= 64 } { .bc.bc2.delay.lb configure -text "  Delay 25( E )  "; set var3 [expr $var3 - 64 ] ;
				} else { .bc.bc2.delay.lb configure -text "  Delay 25( D )  ";  }
				set bc2delay $var3
			}
		}

		button .bc.bc2.delay.wt -text "Write" -command  {
			if { $bc2delay > 63 } {	tk_messageBox -type ok -message "Waka!!! Tas pasao.. Max value 63";  set bc2delay 63}
			set dataex [expr $bc2delay + 64]
			set dataex [format %x $dataex]
			set data 0x00
			append data $dataex
			set addrpoke $brdheader
			append addrpoke $offsetbc2delay;
			exec $pokelocation $addrpoke $data 4 2

			set var $data
			set var2 $addrpoke
			set var3 $data
		}
		pack .bc.bc2.delay -fill x
		grid .bc.bc2.delay.lb -column 1 -row 1
		grid .bc.bc2.delay.ent -column 2 -row 1
		grid .bc.bc2.delay.rd -column 3 -row 1
		grid .bc.bc2.delay.wt -column 4 -row 1

	frame .bc.bc2.qpll
		label .bc.bc2.qpll.lb -text "QPLL Mod"  -width 14
		radiobutton .bc.bc2.qpll.reset -text "Rst  " -variable bc2qpllmode -value 0x00000000  -width 4
		radiobutton .bc.bc2.qpll.lost -text "Lst  " -variable bc2qpllmode -value 0x00000001  -width 3
		button .bc.bc2.qpll.rd	-text "Read" -command {
			set addrpeek $brdheader
			append addrpeek $offsetbc2qpllmode
			set pipe [open "|$peeklocation $addrpeek 4 2"]
			gets $pipe var
			catch {close $pipe}
			if {[string match *error* $var] > 0 } {
				set bc2qpllmode "error"
			} else {set bc2qpllmode $var}
		}
		button .bc.bc2.qpll.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetbc2qpllmode;  exec $pokelocation $addrpoke $bc2qpllmode 4 2 }
		pack .bc.bc2.qpll -fill x
		grid .bc.bc2.qpll.lb -column 1 -row 1
		grid .bc.bc2.qpll.reset -column 2 -row 1
		grid .bc.bc2.qpll.lost -column 3 -row 1
		grid .bc.bc2.qpll.rd -column 4 -row 1
		grid .bc.bc2.qpll.wt -column 5 -row 1


	frame .bc.bc2.qpllst
		label .bc.bc2.qpllst.lb -text "QPLL status"  -width 14
		label .bc.bc2.qpllst.st -textvariable bc2qpllst -relief sunken  -width 14
		button .bc.bc2.qpllst.rd -text "Read" -command {
			set addrpeek $brdheader
			append addrpeek $offsetbc2qpllst
			set pipe [open "|$peeklocation $addrpeek 4 2"]
			gets $pipe var
			catch { close $pipe }
			if { [ string match *error* $var ] > 0 } {
				set bc2qpllst "error"
			} elseif { [ string match *00000000* $var ] > 0} {
				set bc2qpllst "PLL ERR"
			} elseif { [ string match *00000001* $var ] > 0} {
				set bc2qpllst "LOCKED"
			} elseif { [ string match *00000002* $var ] > 0} {
				set bc2qpllst "UnLOKED/ERROR"
			} else {
				set bc2qpllst "NPI"
			}
		}
		label .bc.bc2.qpllst.lb2 -text "           "

		pack .bc.bc2.qpllst
		grid .bc.bc2.qpllst.lb -column 1 -row 1
		grid .bc.bc2.qpllst.st -column 2 -row 1
		grid .bc.bc2.qpllst.rd -column 3 -row 1
		grid .bc.bc2.qpllst.lb2 -column 4 -row 1

#############################################

frame .bc.bcref  -borderwidth 1 -relief raised
	grid .bc.bcref -column 3 -row 1

	frame .bc.bcref.head
		label .bc.bcref.head.lb -text "BCref";
		button .bc.bcref.head.rd -text "READ ALL" -command {
			.bc.bcref.vref.rd invoke
			.bc.bcref.mx.rd invoke
			.bc.bcref.delay.rd invoke
			.bc.bcref.qpll.rd invoke
			.bc.bcref.qpllst.rd invoke
		}
		pack .bc.bcref.head -fill x
		grid .bc.bcref.head.lb -column 1 -row 1
		grid .bc.bcref.head.rd -column 2 -row 1

	#blank line
	frame .bc.bcref.vref
	pack .bc.bcref.vref -fill x
		label .bc.bcref.vref.sp -text ""
		pack .bc.bcref.vref.sp -fill x
	

	#frame .bc.bcref.vref
#			label .bc.bcref.vref.lb  -text "     DAC Vref    "  -width 14
#			entry .bc.bcref.vref.ent -textvariable bcrefvref -width 14
#			button .bc.bcref.vref.rd -text "Read" -command {
#				set addrpeek $brdheader
#				append addrpeek $offsetbcrefvref
#				set pipe [open "|$peeklocation $addrpeek 4 2"]
#				gets $pipe var
#				catch {close $pipe}
#				if {[string match *error* $var] > 0 } {
#					set bcrefvref "error"
#				} else {set bcrefvref $var}
#			}
#			button .bc.bcref.vref.wt -text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetbcrefvref;  exec $pokelocation $addrpoke $bcrefvref 4 2 }
#			pack .bc.bcref.vref
#			grid .bc.bcref.vref.lb -column 1 -row 1
#			grid .bc.bcref.vref.ent -column 2 -row 1
#			grid .bc.bcref.vref.rd -column 3 -row 1
#			grid .bc.bcref.vref.wt -column 4 -row 1

	frame .bc.bcref.mx
		label .bc.bcref.mx.lb -text "Mx Select"  -width 14
		radiobutton .bc.bcref.mx.int -variable bcrefmx -text "int " -value 0x00000000  -width 4
		radiobutton .bc.bcref.mx.ext -variable bcrefmx -text "ext " -value 0x00000001  -width 3
		button .bc.bcref.mx.rd 	-text "Read" -command {
			set addrpeek $brdheader
			append addrpeek $offsetbcrefmx
			set pipe [open "|$peeklocation $addrpeek 4 2"]
			gets $pipe var
			catch {close $pipe}
			if {[string match *error* $var] > 0 } {
				set bcrefmx "error"
			} else {set bcrefmx $var}
		}
		button .bc.bcref.mx.wt 	-text "Write" -command { set addrpoke $brdheader;  append addrpoke $offsetbcrefmx;  exec $pokelocation $addrpoke $bcrefmx 4 2 }
		pack .bc.bcref.mx  -fill x
		grid .bc.bcref.mx.lb  -column 1 -row 1
		grid .bc.bcref.mx.int  -column 2 -row 1
		grid .bc.bcref.mx.ext  -column 3 -row 1
		grid .bc.bcref.mx.rd -column 4 -row 1
		grid .bc.bcref.mx.wt -column 5 -row 1

	frame .bc.bcref.delay
		label .bc.bcref.delay.lb -text "     Delay 25     "   -width 14
		entry .bc.bcref.delay.ent -textvariable bcrefdelay -width 14
		button .bc.bcref.delay.rd -text "Read" -command {
			set addrpeek $brdheader
			append addrpeek $offsetbcrefdelay
			set pipe [open "|$peeklocation $addrpeek 4 2"]
			gets $pipe var
			catch {close $pipe}
			exec sleep 0.02
			set addrpeek $brdheader
			append addrpeek $offsetfifo25
			set pipe [open "|$peeklocation $addrpeek 4 2"]
			gets $pipe var2
			catch {close $pipe}
			if {[string match *error* $var2] > 0 } {
				set bcrefdelay "error"
			} else {
				set var [string range $var2 8 9]
				set var3 0x0;
				append var3 $var;
				set var3 [expr $var3 + 0]
				if {$var3 >  128} {
					set var3 [expr $var3 - 128];
				}
				if { $var3 >= 64 } { .bc.bcref.delay.lb configure -text "  Delay 25( E )  "; set var3 [expr $var3 - 64 ] ;
				} else { .bc.bcref.delay.lb configure -text "  Delay 25( D )  ";  }
				set bcrefdelay $var3
			}
		}

		button .bc.bcref.delay.wt -text "Write" -command  {
			if { $bcrefdelay > 63 } {	tk_messageBox -type ok -message "Waka!!! Tas pasao.. Max value 63";  set bcrefdelay 63}
			set dataex [expr $bcrefdelay + 64]
			set dataex [format %x $dataex]
			set data 0x00
			append data $dataex
			set addrpoke $brdheader
			append addrpoke $offsetbcrefdelay;
			exec $pokelocation $addrpoke $data 4 2

			set var $data
			set var2 $addrpoke
			set var3 $data
		}
		pack .bc.bcref.delay -fill x
		grid .bc.bcref.delay.lb -column 1 -row 1
		grid .bc.bcref.delay.ent -column 2 -row 1
		grid .bc.bcref.delay.rd -column 3 -row 1
		grid .bc.bcref.delay.wt -column 4 -row 1

	frame .bc.bcref.qpll
		label .bc.bcref.qpll.lb -text "QPLL Mod"  -width 14
		radiobutton .bc.bcref.qpll.reset -text "Rst  " -variable bcrefqpllmode -value 0x00000000  -width 4
		radiobutton .bc.bcref.qpll.lost -text "Lst  " -variable bcrefqpllmode -value 0x00000001  -width 3
		button .bc.bcref.qpll.rd	-text "Read" -command {
			set addrpeek $brdheader
			append addrpeek $offsetbcrefqpllmode
			set pipe [open "|$peeklocation $addrpeek 4 2"]
			gets $pipe var
			catch {close $pipe}
			if {[string match *error* $var] > 0 } {
				set bcrefqpllmode "error"
			} else {set bcrefqpllmode $var}
		}
		button .bc.bcref.qpll.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetbcrefqpllmode;  exec $pokelocation $addrpoke $bcrefqpllmode 4 2 }
		pack .bc.bcref.qpll -fill x
		grid .bc.bcref.qpll.lb -column 1 -row 1
		grid .bc.bcref.qpll.reset -column 2 -row 1
		grid .bc.bcref.qpll.lost -column 3 -row 1
		grid .bc.bcref.qpll.rd -column 4 -row 1
		grid .bc.bcref.qpll.wt -column 5 -row 1


	frame .bc.bcref.qpllst
		label .bc.bcref.qpllst.lb -text "QPLL status"  -width 14
		label .bc.bcref.qpllst.st -textvariable bcrefqpllst -relief sunken  -width 14
		button .bc.bcref.qpllst.rd -text "Read" -command {
			set addrpeek $brdheader
			append addrpeek $offsetbcrefqpllst
			set pipe [open "|$peeklocation $addrpeek 4 2"]
			gets $pipe var
			catch { close $pipe }
			if { [ string match *error* $var ] > 0 } {
				set bcrefqpllst "error"
			} elseif { [ string match *00000000* $var ] > 0} {
				set bcrefqpllst "PLL ERR"
			} elseif { [ string match *00000001* $var ] > 0} {
				set bcrefqpllst "LOCKED"
			} elseif { [ string match *00000002* $var ] > 0} {
				set bcrefqpllst "UnLOKED/ERROR"
			} else {
				set bcrefqpllst "NPI"
			}
		}
		label .bc.bcref.qpllst.lb2 -text "           "

		pack .bc.bcref.qpllst
		grid .bc.bcref.qpllst.lb -column 1 -row 1
		grid .bc.bcref.qpllst.st -column 2 -row 1
		grid .bc.bcref.qpllst.rd -column 3 -row 1
		grid .bc.bcref.qpllst.lb2 -column 4 -row 1
#******************************************************************************
#******************************************************************************



frame .bc.bcmain -borderwidth 1 -relief raised
grid .bc.bcmain -column 4 -row 1

frame .bc.bcmain.head
	label .bc.bcmain.head.lb -text "BCmain";
	button .bc.bcmain.head.rd -text "READ ALL" -command {
		.bc.bcmain.mx.rd invoke
		.bc.bcmain.delay.rd invoke
		.bc.bcmain.mqpll.rd invoke
		.bc.bcmain.stqpll.rd invoke
	}
	pack .bc.bcmain.head -fill x
	grid .bc.bcmain.head.lb -column 1 -row 1
	grid .bc.bcmain.head.rd -column 2 -row 1


frame .bc.bcmain.mx
pack .bc.bcmain.mx -fill x
	label .bc.bcmain.mx.lb -text "Mx select"
	#radiobutton .bc.bcmain.mx.bc1 -text "BC1 " -variable mainselector -value 0x0e000000
	radiobutton .bc.bcmain.mx.bc1 -text "BC1 " -variable mainselector -value 0x00000003
	#radiobutton .bc.bcmain.mx.bc2 -text "BC2 " -variable mainselector -value 0x00000001
	radiobutton .bc.bcmain.mx.bc2 -text "BC2 " -variable mainselector -value 0x00000002
	#radiobutton .bc.bcmain.mx.bcref -text "BCref " -variable mainselector -value 0x0e000002
	radiobutton .bc.bcmain.mx.bcref -text "BCref " -variable mainselector -value 0x00000001
	#radiobutton .bc.bcmain.mx.bcint -text "BCint " -variable mainselector -value 0x0e000003
	radiobutton .bc.bcmain.mx.bcint -text "BCint " -variable mainselector -value 0x00000000
	button .bc.bcmain.mx.rd -text "Read" -command {
		set addrpeek $brdheader
		append addrpeek $offsetmainselector
		set pipe [open "|$peeklocation $addrpeek 4 2"]
		gets $pipe var
		catch {close $pipe}
		if {[string match *error* $var] > 0 } {
			set mainselector "error"
		} else {set mainselector $var}
	}
	button .bc.bcmain.mx.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetmainselector;  exec $pokelocation $addrpoke $mainselector 4 2 }
	grid .bc.bcmain.mx.lb -column 1 -row 1
	grid .bc.bcmain.mx.bc1 -column 2 -row 1
	grid .bc.bcmain.mx.bc2 -column 3 -row 1
	grid .bc.bcmain.mx.bcref -column 2 -row 2
	grid .bc.bcmain.mx.bcint -column 3 -row 2
	grid .bc.bcmain.mx.rd -column 4 -row 1
	grid .bc.bcmain.mx.wt -column 4 -row 2

#************************************************
#NEW_____SOPHIE_____18.01.07		
frame .bc.bcmain.delay
		label .bc.bcmain.delay.lb -text "  Delay 25  "   -width 14
			entry .bc.bcmain.delay.ent -textvariable bcmaindelay -width 14
			button .bc.bcmain.delay.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetbcmaindelay
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				exec sleep 0.02
				set addrpeek $brdheader
				append addrpeek $offsetfifo25
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var2
				catch {close $pipe}
				if {[string match *error* $var2] > 0 } {
					set bcmaindelay "error"
				} else {
					set var [string range $var2 8 9]
					set var3 0x0;
					append var3 $var;
					set var3 [expr $var3 + 0]
					if {$var3 >  128} {
						set var3 [expr $var3 - 128];
					}
					if { $var3 >= 64 } { .bc.bcmain.delay.lb configure -text "  Delay 25( E )  "; set var3 [expr $var3 - 64 ] ;
					} else { .bc.bcmain.delay.lb configure -text "  Delay 25( D )  ";  }
					set bcmaindelay $var3
				}
			}

			button .bc.bcmain.delay.wt -text "Write" -command  {
				if { $bcmaindelay > 63 } {	tk_messageBox -type ok -message "Waka!!! Tas pasao.. Max value 63";  set bcmaindelay 63}
				set dataex [expr $bcmaindelay + 64]
				set dataex [format %x $dataex]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetbcmaindelay;
				exec $pokelocation $addrpoke $data 4 2

				set var $data
				set var2 $addrpoke
				set var3 $data
			}
			pack .bc.bcmain.delay -fill x
			grid .bc.bcmain.delay.lb -column 1 -row 1
			grid .bc.bcmain.delay.ent -column 2 -row 1
			grid .bc.bcmain.delay.rd -column 3 -row 1
			grid .bc.bcmain.delay.wt -column 4 -row 1
			
#END_NEW_____SOPHIE____18.01.07
#****************************************************

	frame .bc.bcmain.mqpll
		label .bc.bcmain.mqpll.lb -text "QPLL mode "
		radiobutton .bc.bcmain.mqpll.inrst -text "Rst " -variable mqpll -value 0x00000000
		radiobutton .bc.bcmain.mqpll.inlst -text "lst " -variable mqpll -value 0x00000001
		button .bc.bcmain.mqpll.rd -text "Read" -command {
			set addrpeek $brdheader
			append addrpeek $offsetmqpll
			set pipe [open "|$peeklocation $addrpeek 4 2"]
			gets $pipe var
			catch {close $pipe}
			if {[string match *error* $var] > 0 } {
				set mqpll "error"
			} else {set mqpll $var}
		}
		button .bc.bcmain.mqpll.wt -text "Write" -command  {set addrpoke $brdheader;  append addrpoke $offsetmqpll;  exec $pokelocation $addrpoke $mqpll 4 2 }
		pack .bc.bcmain.mqpll -fill x
		grid .bc.bcmain.mqpll.lb -column 1 -row 1
		grid .bc.bcmain.mqpll.inrst -column 2 -row 1
		grid .bc.bcmain.mqpll.inlst -column 3 -row 1
		grid .bc.bcmain.mqpll.rd -column 4 -row 1
		grid .bc.bcmain.mqpll.wt -column 5 -row 1


	frame .bc.bcmain.stqpll
		label .bc.bcmain.stqpll.lb -text "QPLL Status"
		label .bc.bcmain.stqpll.st -textvariable stqpll -relief sunken -width 10
		button .bc.bcmain.stqpll.rd -text "Read" -command {
			set addrpeek $brdheader
			append addrpeek $offsetstqpll
			set pipe [open "|$peeklocation $addrpeek 4 2"]
			gets $pipe var
			catch { close $pipe }
			if { [ string match *error* $var ] > 0 } {
				set stqpll "error"
			} elseif { [ string match *00000000* $var ] > 0} {
				set stqpll "PLL ERR"
			} elseif { [ string match *00000001* $var ] > 0} {
				set stqpll "LOCKED"
			} elseif { [ string match *00000002* $var ] > 0} {
				set stqpll "UnLOKED/ERROR"
			} else {
				set stqpll "NPI"
			}
		}
		pack .bc.bcmain.stqpll -fill x
		grid .bc.bcmain.stqpll.lb -column 1 -row 1
		grid .bc.bcmain.stqpll.st -column 2 -row 1
		grid .bc.bcmain.stqpll.rd -column 3 -row 1


#blank line
label .sp2 -text ""
pack .sp2


##_________________________________*************__________________________________________________________________
##_________________________________*************__________________________________________________________________
##_________________________________*************__________________________________________________________________


frame .orbit
pack .orbit -fill x

	frame .orbit.orb1 -borderwidth 1 -relief raised
	grid .orbit.orb1  -column 1 -row 1

		frame .orbit.orb1.head
		label .orbit.orb1.head.lb -text "Orb1"
		button .orbit.orb1.head.bt -text "READ ALL" -command {
			.orbit.orb1.vref.rd invoke
			.orbit.orb1.sdelay.rd invoke
			.orbit.orb1.select.rd invoke
			.orbit.orb1.cdelay.rd invoke
			.orbit.orb1.length.rd invoke
			.orbit.orb1.pol.rd invoke
			.orbit.orb1.fdelay.rd invoke
			.orbit.orb1.intcounter.rd invoke
			.orbit.orb1.percounter.rdl invoke
			.orbit.orb1.orbcounter.rdl invoke
		}
		pack .orbit.orb1.head -fill x
		grid .orbit.orb1.head.lb -column 1 -row 1
		grid .orbit.orb1.head.bt -column 2 -row 1

		frame .orbit.orb1.vref
			label .orbit.orb1.vref.lb -text "DAC Vref: " -width 14
			entry .orbit.orb1.vref.in -textvariable orb1vref -width 14
			button .orbit.orb1.vref.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb1vref
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb1vref "error"
				} else {set orb1vref $var}
			}
			button .orbit.orb1.vref.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetorb1vref;  exec $pokelocation $addrpoke $orb1vref 4 2 }
			pack .orbit.orb1.vref -fill x
			grid .orbit.orb1.vref.lb -column 1 -row 1
			grid .orbit.orb1.vref.in -column 2 -row 1
			grid .orbit.orb1.vref.rd -column 3 -row 1
			grid .orbit.orb1.vref.wt -column 4 -row 1

		frame .orbit.orb1.sdelay
			label .orbit.orb1.sdelay.lb -text "Shift Delay" -width 14
			entry .orbit.orb1.sdelay.in -textvariable orb1sdelay -width 14
			button .orbit.orb1.sdelay.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb1sdelay
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				exec sleep 0.02
				set addrpeek $brdheader
				append addrpeek $offsetfifo25
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var2
				catch {close $pipe}
				if {[string match *error* $var2] > 0 } {
					set orb1sdelay "error"
				} else {
					set var [string range $var2 8 9]
					set var3 0x0;
					append var3 $var;
					set var3 [expr $var3 + 0]
					if {$var3 >  128} {
						set var3 [expr $var3 - 128];
					}
					if { $var3 >= 64 } { .orbit.orb1.sdelay.lb configure -text "  Shift Delay( E )  "; set var3 [expr $var3 - 64 ] ;
					} else { .orbit.orb1.sdelay.lb configure -text "  Shift Delay( D )  ";  }
					set orb1sdelay $var3
				}
			}
			button .orbit.orb1.sdelay.wt -text "Write" -command {
				if { $orb1sdelay > 63 } {	tk_messageBox -type ok -message "Waka!!! Tas pasao.. Max value 63";  set orb1sdelay 63}
				set dataex [expr $orb1sdelay + 64]
				set dataex [format %x $dataex]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetorb1sdelay;
				exec $pokelocation $addrpoke $data 4 2
				set var $data
				set var2 $addrpoke
				set var3 $data
			 }
			pack .orbit.orb1.sdelay -fill x
			grid .orbit.orb1.sdelay.lb -column 1 -row 1
			grid .orbit.orb1.sdelay.in -column 2 -row 1
			grid .orbit.orb1.sdelay.rd -column 3 -row 1
			grid .orbit.orb1.sdelay.wt -column 4 -row 1

		frame .orbit.orb1.select
			label .orbit.orb1.select.lb -text "Source Selection" -width 14
			radiobutton .orbit.orb1.select.int -text "Int  " -variable orb1select -value 0x00000001
			radiobutton .orbit.orb1.select.ext -text "Ext " -variable orb1select -value 0x00000000
			button .orbit.orb1.select.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb1select
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb1select "error"
				} else {set orb1select $var}
			}
			button .orbit.orb1.select.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetorb1select;  exec $pokelocation $addrpoke $orb1select 4 2 }
			pack .orbit.orb1.select -fill x
			grid .orbit.orb1.select.lb -column 1 -row 1
			grid .orbit.orb1.select.int -column 2 -row 1
			grid .orbit.orb1.select.ext -column 3 -row 1
			grid .orbit.orb1.select.rd -column 4 -row 1
			grid .orbit.orb1.select.wt -column 5 -row 1

		frame .orbit.orb1.cdelay
			label .orbit.orb1.cdelay.lb -text "Coarse Delay" -width 14
			entry .orbit.orb1.cdelay.in -textvariable orb1cdelay -width 14
			button .orbit.orb1.cdelay.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb1cdelay
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb1cdelay "error"
				} else {set orb1cdelay $var}
			}
			button .orbit.orb1.cdelay.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetorb1cdelay;  exec $pokelocation $addrpoke $orb1cdelay 4 2 }
			pack .orbit.orb1.cdelay -fill x
			grid .orbit.orb1.cdelay.lb -column 1 -row 1
			grid .orbit.orb1.cdelay.in -column 2 -row 1
			grid .orbit.orb1.cdelay.rd -column 3 -row 1
			grid .orbit.orb1.cdelay.wt -column 4 -row 1

		frame .orbit.orb1.length
			label .orbit.orb1.length.lb -text "Pulse Length" -width 14
			entry .orbit.orb1.length.in -textvariable orb1length -width 14
			button .orbit.orb1.length.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb1lenght
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb1length "error"
				} else {set orb1length $var}
			}
			button .orbit.orb1.length.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetorb1lenght;  exec $pokelocation $addrpoke $orb1length 4 2 }
			pack .orbit.orb1.length -fill x
			grid .orbit.orb1.length.lb -column 1 -row 1
			grid .orbit.orb1.length.in -column 2 -row 1
			grid .orbit.orb1.length.rd -column 3 -row 1
			grid .orbit.orb1.length.wt -column 4 -row 1

		frame .orbit.orb1.pol
			label .orbit.orb1.pol.lb -text "Pulse Polarity" -width 14
			radiobutton .orbit.orb1.pol.int -text "Pos " -variable orb1pol -value 0x00000000
			radiobutton .orbit.orb1.pol.ext -text "Neg" -variable orb1pol -value 0x00000001
			button .orbit.orb1.pol.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb1pol
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb1pol "error"
				} else {set orb1pol $var}
			}
			button .orbit.orb1.pol.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetorb1pol;  exec $pokelocation $addrpoke $orb1pol 4 2 }
			pack .orbit.orb1.pol -fill x
			grid .orbit.orb1.pol.lb -column 1 -row 1
			grid .orbit.orb1.pol.int -column 2 -row 1
			grid .orbit.orb1.pol.ext -column 3 -row 1
			grid .orbit.orb1.pol.rd -column 4 -row 1
			grid .orbit.orb1.pol.wt -column 5 -row 1

		frame .orbit.orb1.fdelay
			label .orbit.orb1.fdelay.lb -text "Fine Delay" -width 14
			entry .orbit.orb1.fdelay.in -textvariable orb1fdelay -width 14
			button .orbit.orb1.fdelay.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb1fdelay
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				exec sleep 0.02
				set addrpeek $brdheader
				append addrpeek $offsetfifo25
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var2
				catch {close $pipe}
				if {[string match *error* $var2] > 0 } {
					set orb1fdelay "error"
				} else {
					set var [string range $var2 8 9]
					set var3 0x0;
					append var3 $var;
					set var3 [expr $var3 + 0]
					if {$var3 >  128} {
						set var3 [expr $var3 - 128];
					}
					if { $var3 >= 64 } { .orbit.orb1.fdelay.lb configure -text "  Fine Delay( E )  "; set var3 [expr $var3 - 64 ] ;
					} else { .orbit.orb1.fdelay.lb configure -text "  Fine Delay( D )  ";  }
					set orb1fdelay $var3
				}
			}
			button .orbit.orb1.fdelay.wt -text "Write" -command {
				if { $orb1fdelay > 63 } {	tk_messageBox -type ok -message "Waka!!! Tas pasao.. Max value 63";  set orb1fdelay 63}
				set dataex [expr $orb1fdelay + 64]
				set dataex [format %x $dataex]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetorb1fdelay;
				exec $pokelocation $addrpoke $data 4 2
				set var $data
				set var2 $addrpoke
				set var3 $data
			 }
			pack .orbit.orb1.fdelay -fill x
			grid .orbit.orb1.fdelay.lb -column 1 -row 1
			grid .orbit.orb1.fdelay.in -column 2 -row 1
			grid .orbit.orb1.fdelay.rd -column 3 -row 1
			grid .orbit.orb1.fdelay.wt -column 4 -row 1

		frame .orbit.orb1.blanck
		pack .orbit.orb1.blanck -fill x
			label .orbit.orb1.blanck.lb -text " ... "
			pack .orbit.orb1.blanck.lb -fill x

# " INT Orbit Counter/GEN"

		label .orbit.orb1.lbintcounter -text " INT Orbit Counter/GEN"
			pack .orbit.orb1.lbintcounter -fill x

		frame .orbit.orb1.intcounter
			label .orbit.orb1.intcounter.lb -text "Int Periode" -width 14
			button .orbit.orb1.intcounter.en -text "En/Dis-able" -command	{
				# reading actual state of counters
				READINTSTATE
				# activating or deactiving depending of the actual state
				if { $int1en == 1 } {
					set int1en 0;
					.orbit.orb1.intcounter.en configure -text "Enable";
					.orbit.orb1.lbintcounter configure -text "INT Orbit Counter/GEN (DIS)";
				} elseif { $int1en == 0 } {
					set int1en 1;
					.orbit.orb1.intcounter.en configure -text "Disable";
					.orbit.orb1.lbintcounter configure -text "INT Orbit Counter/GEN (EN)";
				}
#------              																		Conflictive line when copy and paste.. restore order.. m*4 or2*2 or1*1
				set dataex [expr $intmen * 4 + $int2en * 2 + $int1en * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetintcnten
				exec $pokelocation $addrpoke $data 4 2

			}
			button .orbit.orb1.intcounter.rst -text "Reset" -command {
				# RESETING orbit int bc1
				set int1rst 1
#------              																		Conflictive line when copy and paste.. restore order.. m*4 or2*2 or1*1
				set dataex [expr $intmrst * 4 + $int2rst * 2 + $int1rst * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetintcntrst
				exec $pokelocation $addrpoke $data 4 2
				set var $data
				set int1rst 0

			}
			entry .orbit.orb1.intcounter.in -textvariable orb1intperiode -width 14
			button .orbit.orb1.intcounter.rd -text "Rd Periode" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb1periode
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb1intperiode "error"
				} else {set orb1intperiode $var}
			}
			button .orbit.orb1.intcounter.wt -text "Wr Periode" -command {set addrpoke $brdheader;  append addrpoke $offsetorb1periode;  exec $pokelocation $addrpoke $orb1intperiode 4 2 }
			pack .orbit.orb1.intcounter
			grid .orbit.orb1.intcounter.lb 	-column 1 -row 1
			grid .orbit.orb1.intcounter.en 	-column 2 -row 1
			grid .orbit.orb1.intcounter.rst 	-column 3 -row 1
			grid .orbit.orb1.intcounter.in 	-column 1 -row 2
			grid .orbit.orb1.intcounter.rd 	-column 2 -row 2
			grid .orbit.orb1.intcounter.wt 	-column 3 -row 2

		frame .orbit.orb1.blanck2
		pack .orbit.orb1.blanck2 -fill x
			label .orbit.orb1.blanck2.lb -text " ... "
			pack .orbit.orb1.blanck2.lb -fill x

		label .orbit.orb1.lbpercounter -text " PERIODE Orbit Counter"
			pack .orbit.orb1.lbpercounter -fill x

#" PERIODE Orbit Counter"

		frame .orbit.orb1.percounter
			label .orbit.orb1.percounter.lb -text "Periode counted" -width 14
			button .orbit.orb1.percounter.en -text "En/Dis-able" -command {
				# reading actual state of counters
				READPERSTATE
				# activating or deactiving depending of the actual state
				set var2 $per1en
				set var3 $per2en
				if { $per1en == 1 } {
					set per1en 0;
					.orbit.orb1.percounter.en configure -text "Enable";
					.orbit.orb1.lbpercounter configure -text "PERIODE Orbit Counter (DIS)";
				} elseif { $per1en == 0 } {
					set per1en 1;
					.orbit.orb1.percounter.en configure -text "Disable";
					.orbit.orb1.lbpercounter configure -text "PERIODE Orbit Counter (EN)";
				}
#------              																		Conflictive line when copy and paste.. restore order.. m*4 or2*2 or1*1

				set dataex [expr $permen * 4 + $per2en * 2 + $per1en * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetpercnten
				exec $pokelocation $addrpoke $data 4 2

			}
			button .orbit.orb1.percounter.rst -text "Reset" -command {
				# RESETING orbit int bc1 ***
				set per1rst 1
#------              																		Conflictive line when copy and paste.. restore order.. m*4 or2*2 or1*1
				set dataex [expr $permrst * 4 + $per2rst * 2 + $per1rst * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetpercntrst
				exec $pokelocation $addrpoke $data 4 2
				set per1rst 0
				set var $data
			}
			entry .orbit.orb1.percounter.in -textvariable orb1percounter -width 14
			button .orbit.orb1.percounter.rdl -text "RD last" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb1percntrdlast
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb1percounter "error"
				} else {set orb1percounter $var}
			}
			button .orbit.orb1.percounter.rdf -text "RD fifo" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb1percntrdfifo
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb1percounter "error"
				} else {set orb1percounter $var}
			}
			pack .orbit.orb1.percounter
			grid .orbit.orb1.percounter.lb 	-column 1 -row 1
			grid .orbit.orb1.percounter.en 	-column 2 -row 1
			grid .orbit.orb1.percounter.rst 	-column 3 -row 1
			grid .orbit.orb1.percounter.in 	-column 1 -row 2
			grid .orbit.orb1.percounter.rdl 	-column 2 -row 2
			grid .orbit.orb1.percounter.rdf 	-column 3 -row 2

		frame .orbit.orb1.blanck3
		pack .orbit.orb1.blanck3 -fill x
			label .orbit.orb1.blanck3.lb -text " ... "
			pack .orbit.orb1.blanck3.lb -fill x

# ORbit Counter"

		label .orbit.orb1.lborbcounter -text " Orbit Counter"
			pack .orbit.orb1.lborbcounter -fill x

		frame .orbit.orb1.orbcounter
			button .orbit.orb1.orbcounter.en -text "En/Dis-able" -command {
				# reading actual state of counters
				READORBSTATE
				# activating or deactiving depending of the actual state
				if { $orb1en == 1 } {
					set orb1en 0;
					.orbit.orb1.orbcounter.en configure -text "Enable";
					.orbit.orb1.lborbcounter configure -text "Orbit Counter (DIS)";
				} elseif { $orb1en == 0 } {
					set orb1en 1;
					.orbit.orb1.orbcounter.en configure -text "Disable";
					.orbit.orb1.lborbcounter configure -text "ORbit Counter (EN)";
				}
#------              																		Conflictive line when copy and paste.. restore order.. m*4 or2*2 or1*1
				set dataex [expr $orbmen * 4 + $orb2en * 2 + $orb1en * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetorbcnten
				exec $pokelocation $addrpoke $data 4 2

			}
			button .orbit.orb1.orbcounter.rst -text "Reset" -command {
			# RESETING orbit int bc1 ***
				set orb1rst 1
#------              																		Conflictive line when copy and paste.. restore order.. m*4 or2*2 or1*1
				set dataex [expr $orbmrst * 4 + $orb2rst * 2 + $orb1rst * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetorbcntrst
				exec $pokelocation $addrpoke $data 4 2
				set orb1rst 0
				set var $addrpoke
				set var2 $data
			}
			entry .orbit.orb1.orbcounter.in -textvariable orb1orbcounter -width 14
			button .orbit.orb1.orbcounter.rdl -text "RD" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb1orbcntrd
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb1orbcounter "error"
				} else {set orb1orbcounter $var}
			}
			pack .orbit.orb1.orbcounter
			grid .orbit.orb1.orbcounter.en 	-column 1 -row 1
			grid .orbit.orb1.orbcounter.rst 	-column 2 -row 1
			grid .orbit.orb1.orbcounter.in 	-column 3 -row 1
			grid .orbit.orb1.orbcounter.rdl 	-column 4 -row 1


##_________________________________*************__________________________________________________________________

	frame .orbit.orb2 -borderwidth 1 -relief raised
	grid .orbit.orb2  -column 2 -row 1

	frame .orbit.orb2.head
		label .orbit.orb2.head.lb -text "Orb2"
		button .orbit.orb2.head.bt -text "READ ALL" -command {
			.orbit.orb2.vref.rd invoke
			.orbit.orb2.sdelay.rd invoke
			.orbit.orb2.select.rd invoke
			.orbit.orb2.cdelay.rd invoke
			.orbit.orb2.length.rd invoke
			.orbit.orb2.pol.rd invoke
			.orbit.orb2.fdelay.rd invoke
			.orbit.orb2.intcounter.rd invoke
			.orbit.orb2.percounter.rdl invoke
			.orbit.orb2.orbcounter.rdl invoke
		}
		pack .orbit.orb2.head -fill x
		grid .orbit.orb2.head.lb -column 1 -row 1
		grid .orbit.orb2.head.bt -column 2 -row 1


		frame .orbit.orb2.vref
			label .orbit.orb2.vref.lb -text "DAC Vref: " -width 14
			entry .orbit.orb2.vref.in -textvariable orb2vref -width 14
			button .orbit.orb2.vref.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb2vref
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb2vref "error"
				} else {set orb2vref $var}
			}
			button .orbit.orb2.vref.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetorb2vref;  exec $pokelocation $addrpoke $orb2vref 4 2 }
			pack .orbit.orb2.vref -fill x
			grid .orbit.orb2.vref.lb -column 1 -row 1
			grid .orbit.orb2.vref.in -column 2 -row 1
			grid .orbit.orb2.vref.rd -column 3 -row 1
			grid .orbit.orb2.vref.wt -column 4 -row 1

		frame .orbit.orb2.sdelay
			label .orbit.orb2.sdelay.lb -text "Shift Delay" -width 14
			entry .orbit.orb2.sdelay.in -textvariable orb2sdelay -width 14
			button .orbit.orb2.sdelay.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb2sdelay
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				exec sleep 0.02
				set addrpeek $brdheader
				append addrpeek $offsetfifo25
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var2
				catch {close $pipe}
				if {[string match *error* $var2] > 0 } {
					set orb2sdelay "error"
				} else {
					set var [string range $var2 8 9]
					set var3 0x0;
					append var3 $var;
					set var3 [expr $var3 + 0]
					if {$var3 >  128} {
						set var3 [expr $var3 - 128];
					}
					if { $var3 >= 64 } { .orbit.orb2.sdelay.lb configure -text "  Shift Delay( E )  "; set var3 [expr $var3 - 64 ] ;
					} else { .orbit.orb2.sdelay.lb configure -text "  Shift Delay( D )  ";  }
					set orb2sdelay $var3
				}
			}
			button .orbit.orb2.sdelay.wt -text "Write" -command {
				if { $orb2sdelay > 63 } {	tk_messageBox -type ok -message "Waka!!! Tas pasao.. Max value 63";  set orb2sdelay 63}
				set dataex [expr $orb2sdelay + 64]
				set dataex [format %x $dataex]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetorb2sdelay;
				exec $pokelocation $addrpoke $data 4 2
				set var $data
				set var2 $addrpoke
				set var3 $data
			 }
			pack .orbit.orb2.sdelay -fill x
			grid .orbit.orb2.sdelay.lb -column 1 -row 1
			grid .orbit.orb2.sdelay.in -column 2 -row 1
			grid .orbit.orb2.sdelay.rd -column 3 -row 1
			grid .orbit.orb2.sdelay.wt -column 4 -row 1

		frame .orbit.orb2.select
			label .orbit.orb2.select.lb -text "Source Selection" -width 14
			radiobutton .orbit.orb2.select.int -text "Int  " -variable orb2select -value 0x00000001
			radiobutton .orbit.orb2.select.ext -text "Ext " -variable orb2select -value 0x00000000
			button .orbit.orb2.select.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb2select
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb2select "error"
				} else {set orb2select $var}
			}
			button .orbit.orb2.select.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetorb2select;  exec $pokelocation $addrpoke $orb2select 4 2 }
			pack .orbit.orb2.select -fill x
			grid .orbit.orb2.select.lb -column 1 -row 1
			grid .orbit.orb2.select.int -column 2 -row 1
			grid .orbit.orb2.select.ext -column 3 -row 1
			grid .orbit.orb2.select.rd -column 4 -row 1
			grid .orbit.orb2.select.wt -column 5 -row 1

		frame .orbit.orb2.cdelay
			label .orbit.orb2.cdelay.lb -text "Coarse Delay" -width 14
			entry .orbit.orb2.cdelay.in -textvariable orb2cdelay -width 14
			button .orbit.orb2.cdelay.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb2cdelay
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb2cdelay "error"
				} else {set orb2cdelay $var}
			}
			button .orbit.orb2.cdelay.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetorb2cdelay;  exec $pokelocation $addrpoke $orb2cdelay 4 2 }
			pack .orbit.orb2.cdelay -fill x
			grid .orbit.orb2.cdelay.lb -column 1 -row 1
			grid .orbit.orb2.cdelay.in -column 2 -row 1
			grid .orbit.orb2.cdelay.rd -column 3 -row 1
			grid .orbit.orb2.cdelay.wt -column 4 -row 1

		frame .orbit.orb2.length
			label .orbit.orb2.length.lb -text "Pulse Length" -width 14
			entry .orbit.orb2.length.in -textvariable orb2length -width 14
			button .orbit.orb2.length.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb2lenght
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb2length "error"
				} else {set orb2length $var}
			}
			button .orbit.orb2.length.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetorb2lenght;  exec $pokelocation $addrpoke $orb2length 4 2 }
			pack .orbit.orb2.length -fill x
			grid .orbit.orb2.length.lb -column 1 -row 1
			grid .orbit.orb2.length.in -column 2 -row 1
			grid .orbit.orb2.length.rd -column 3 -row 1
			grid .orbit.orb2.length.wt -column 4 -row 1

		frame .orbit.orb2.pol
			label .orbit.orb2.pol.lb -text "Pulse Polarity" -width 14
			radiobutton .orbit.orb2.pol.int -text "Pos " -variable orb2pol -value 0x00000000
			radiobutton .orbit.orb2.pol.ext -text "Neg" -variable orb2pol -value 0x00000001
			button .orbit.orb2.pol.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb2pol
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb2pol "error"
				} else {set orb2pol $var}
			}
			button .orbit.orb2.pol.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetorb2pol;  exec $pokelocation $addrpoke $orb2pol 4 2 }
			pack .orbit.orb2.pol -fill x
			grid .orbit.orb2.pol.lb -column 1 -row 1
			grid .orbit.orb2.pol.int -column 2 -row 1
			grid .orbit.orb2.pol.ext -column 3 -row 1
			grid .orbit.orb2.pol.rd -column 4 -row 1
			grid .orbit.orb2.pol.wt -column 5 -row 1

		frame .orbit.orb2.fdelay
			label .orbit.orb2.fdelay.lb -text "Fine Delay" -width 14
			entry .orbit.orb2.fdelay.in -textvariable orb2fdelay -width 14
			button .orbit.orb2.fdelay.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb2fdelay
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				exec sleep 0.02
				set addrpeek $brdheader
				append addrpeek $offsetfifo25
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var2
				catch {close $pipe}
				if {[string match *error* $var2] > 0 } {
					set orb2fdelay "error"
				} else {
					set var [string range $var2 8 9]
					set var3 0x0;
					append var3 $var;
					set var3 [expr $var3 + 0]
					if {$var3 >  128} {
						set var3 [expr $var3 - 128];
					}
					if { $var3 >= 64 } { .orbit.orb2.fdelay.lb configure -text "  Fine Delay( E )  "; set var3 [expr $var3 - 64 ] ;
					} else { .orbit.orb2.fdelay.lb configure -text "  Fine Delay( D )  ";  }
					set orb2fdelay $var3
				}
			}
			button .orbit.orb2.fdelay.wt -text "Write" -command {
				if { $orb2fdelay > 63 } {	tk_messageBox -type ok -message "Waka!!! Tas pasao.. Max value 63";  set orb2fdelay 63}
				set dataex [expr $orb2fdelay + 64]
				set dataex [format %x $dataex]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetorb2fdelay;
				exec $pokelocation $addrpoke $data 4 2
				set var $data
				set var2 $addrpoke
				set var3 $data
			 }
			pack .orbit.orb2.fdelay -fill x
			grid .orbit.orb2.fdelay.lb -column 1 -row 1
			grid .orbit.orb2.fdelay.in -column 2 -row 1
			grid .orbit.orb2.fdelay.rd -column 3 -row 1
			grid .orbit.orb2.fdelay.wt -column 4 -row 1

		frame .orbit.orb2.blanck
		pack .orbit.orb2.blanck -fill x
			label .orbit.orb2.blanck.lb -text " ... "
			pack .orbit.orb2.blanck.lb -fill x

# " INT Orbit Counter/GEN"

		label .orbit.orb2.lbintcounter -text " INT Orbit Counter/GEN"
			pack .orbit.orb2.lbintcounter -fill x

		frame .orbit.orb2.intcounter
			label .orbit.orb2.intcounter.lb -text "Int Periode" -width 14
			button .orbit.orb2.intcounter.en -text "En/Dis-able" -command	{
				# reading actual state of counters
				READINTSTATE
				# activating or deactiving depending of the actual state
				if { $int2en == 1 } {
					set int2en 0;
					.orbit.orb2.intcounter.en configure -text "Enable";
					.orbit.orb2.lbintcounter configure -text "INT Orbit Counter/GEN (DIS)";
				} elseif { $int2en == 0 } {
					set int2en 1;
					.orbit.orb2.intcounter.en configure -text "Disable";
					.orbit.orb2.lbintcounter configure -text "INT Orbit Counter/GEN (EN)";
				}
				set dataex [expr $intmen * 4 + $int2en * 2 + $int1en * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetintcnten
				exec $pokelocation $addrpoke $data 4 2

			}
			button .orbit.orb2.intcounter.rst -text "Reset" -command {
				# RESETING orbit int bc1 ***
				set int2rst 1
				set dataex [expr $intmrst * 4 + $int2rst * 2 + $int1rst * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetintcntrst
				exec $pokelocation $addrpoke $data 4 2
				set int2rst 0
				set var $data
			}
			entry .orbit.orb2.intcounter.in -textvariable orb2intperiode -width 14
			button .orbit.orb2.intcounter.rd -text "Rd Periode" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb2periode
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb2intperiode "error"
				} else {set orb2intperiode $var}
			}
			button .orbit.orb2.intcounter.wt -text "Wr Periode" -command {set addrpoke $brdheader;  append addrpoke $offsetorb2periode;  exec $pokelocation $addrpoke $orb2intperiode 4 2 }
			pack .orbit.orb2.intcounter
			grid .orbit.orb2.intcounter.lb 	-column 1 -row 1
			grid .orbit.orb2.intcounter.en 	-column 2 -row 1
			grid .orbit.orb2.intcounter.rst 	-column 3 -row 1
			grid .orbit.orb2.intcounter.in 	-column 1 -row 2
			grid .orbit.orb2.intcounter.rd 	-column 2 -row 2
			grid .orbit.orb2.intcounter.wt 	-column 3 -row 2

		frame .orbit.orb2.blanck2
		pack .orbit.orb2.blanck2 -fill x
			label .orbit.orb2.blanck2.lb -text " ... "
			pack .orbit.orb2.blanck2.lb -fill x

		label .orbit.orb2.lbpercounter -text " PERIODE Orbit Counter"
			pack .orbit.orb2.lbpercounter -fill x

#" PERIODE Orbit Counter"

		frame .orbit.orb2.percounter
			label .orbit.orb2.percounter.lb -text "Periode counted" -width 14
			button .orbit.orb2.percounter.en -text "En/Dis-able" -command {
				# reading actual state of counters
				READPERSTATE
				# activating or deactiving depending of the actual state
				set var2 $per1en
				set var3 $per2en
				if { $per2en == 1 } {#sophie 16.01.07
					set per2en 0;#sophie 16.01.07
					.orbit.orb2.percounter.en configure -text "Enable";
					.orbit.orb2.lbpercounter configure -text "PERIODE Orbit Counter (DIS)";
				} elseif { $per2en == 0 } {#sophie 16.01.07
					set per2en 1;#sophie 16.01.07
					.orbit.orb2.percounter.en configure -text "Disable";
					.orbit.orb2.lbpercounter configure -text "PERIODE Orbit Counter (EN)";
				}

				set dataex [expr $permen * 4 + $per2en * 2 + $per1en * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetpercnten
				exec $pokelocation $addrpoke $data 4 2

			}
			button .orbit.orb2.percounter.rst -text "Reset" -command {
				# RESETING orbit int bc1 ***
				set per2rst 1
				set dataex [expr $permrst * 4 + $per2rst * 2 + $per1rst * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetpercntrst
				exec $pokelocation $addrpoke $data 4 2
				set var $data
				set per2rst 0
			}
			entry .orbit.orb2.percounter.in -textvariable orb2percounter -width 14
			button .orbit.orb2.percounter.rdl -text "RD last" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb2percntrdlast
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb2percounter "error"
				} else {set orb2percounter $var}
			}
			button .orbit.orb2.percounter.rdf -text "RD fifo" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb2percntrdfifo
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb2percounter "error"
				} else {set orb2percounter $var}
			}
			pack .orbit.orb2.percounter
			grid .orbit.orb2.percounter.lb 	-column 1 -row 1
			grid .orbit.orb2.percounter.en 	-column 2 -row 1
			grid .orbit.orb2.percounter.rst 	-column 3 -row 1
			grid .orbit.orb2.percounter.in 	-column 1 -row 2
			grid .orbit.orb2.percounter.rdl 	-column 2 -row 2
			grid .orbit.orb2.percounter.rdf 	-column 3 -row 2

		frame .orbit.orb2.blanck3
		pack .orbit.orb2.blanck3 -fill x
			label .orbit.orb2.blanck3.lb -text " ... "
			pack .orbit.orb2.blanck3.lb -fill x

# ORbit Counter"

		label .orbit.orb2.lborbcounter -text " Orbit Counter"
			pack .orbit.orb2.lborbcounter -fill x

		frame .orbit.orb2.orbcounter
			button .orbit.orb2.orbcounter.en -text "En/Dis-able" -command {
				# reading actual state of counters
				READORBSTATE
				# activating or deactiving depending of the actual state
				if { $orb2en == 1 } {
					set orb2en 0;
					.orbit.orb2.orbcounter.en configure -text "Enable";
					.orbit.orb2.lborbcounter configure -text "Orbit Counter (DIS)";
				} elseif { $orb2en == 0 } {
					set orb2en 1;
					.orbit.orb2.orbcounter.en configure -text "Disable";
					.orbit.orb2.lborbcounter configure -text "ORbit Counter (EN)";
				}

				set dataex [expr $orbmen * 4 + $orb2en * 2 + $orb1en * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetorbcnten
				exec $pokelocation $addrpoke $data 4 2

			}
			button .orbit.orb2.orbcounter.rst -text "Reset" -command {
				# RESETING orbit int bc1 ***
				set orb2rst 1
				set dataex [expr $orbmrst * 4 + $orb2rst * 2 + $orb1rst * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetorbcntrst
				exec $pokelocation $addrpoke $data 4 2
				set orb2rst 0
				set var $addrpoke
				set var2 $data
			}
			entry .orbit.orb2.orbcounter.in -textvariable orb2orbcounter -width 14
			button .orbit.orb2.orbcounter.rdl -text "RD" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorb2orbcntrd
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orb2orbcounter "error"
				} else {set orb2orbcounter $var}
			}
			pack .orbit.orb2.orbcounter
			grid .orbit.orb2.orbcounter.en 	-column 1 -row 1
			grid .orbit.orb2.orbcounter.rst 	-column 2 -row 1
			grid .orbit.orb2.orbcounter.in 	-column 3 -row 1
			grid .orbit.orb2.orbcounter.rdl 	-column 4 -row 1

#*********************************************************



	frame .orbit.orbm -borderwidth 1 -relief raised
	grid .orbit.orbm  -column 3 -row 1

	frame .orbit.orbm.head
		label .orbit.orbm.head.lb -text "Orb Main"
		button .orbit.orbm.head.bt -text "READ ALL" -command {
			.orbit.orbm.select.rd invoke
			.orbit.orbm.cdelay.rd invoke
			.orbit.orbm.length.rd invoke
			.orbit.orbm.pol.rd invoke
			.orbit.orbm.fdelay.rd invoke
			.orbit.orbm.intcounter.rd invoke
			.orbit.orbm.percounter.rdl invoke
			.orbit.orbm.orbcounter.rdl invoke
		}
		pack .orbit.orbm.head -fill x
		grid .orbit.orbm.head.lb -column 1 -row 1
		grid .orbit.orbm.head.bt -column 2 -row 1


		frame .orbit.orbm.select
			label .orbit.orbm.select.lb -text "Source Selection" -width 14
			radiobutton .orbit.orbm.select.int -text "Int  " -variable orbmselect -value 0x00000002
			radiobutton .orbit.orbm.select.o1 -text "Orb1 " -variable orbmselect -value 0x00000000
			radiobutton .orbit.orbm.select.o2 -text "Orb2 " -variable orbmselect -value 0x00000001
			button .orbit.orbm.select.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorbmselect
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orbmselect "error"
				} else {set orbmselect $var}
			}
			button .orbit.orbm.select.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetorbmselect;  exec $pokelocation $addrpoke $orbmselect 4 2 }
			pack .orbit.orbm.select -fill x
			grid .orbit.orbm.select.lb -column 1 -row 1
			grid .orbit.orbm.select.int -column 2 -row 1
			grid .orbit.orbm.select.o1 -column 3 -row 1
			grid .orbit.orbm.select.o2 -column 4 -row 1
			grid .orbit.orbm.select.rd -column 5 -row 1
			grid .orbit.orbm.select.wt -column 6 -row 1

		frame .orbit.orbm.cdelay
			label .orbit.orbm.cdelay.lb -text "Coarse Delay" -width 14
			entry .orbit.orbm.cdelay.in -textvariable orbmcdelay -width 14
			button .orbit.orbm.cdelay.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorbmcdelay
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orbmcdelay "error"
				} else {set orbmcdelay $var}
			}
			button .orbit.orbm.cdelay.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetorbmcdelay;  exec $pokelocation $addrpoke $orbmcdelay 4 2 }
			pack .orbit.orbm.cdelay -fill x
			grid .orbit.orbm.cdelay.lb -column 1 -row 1
			grid .orbit.orbm.cdelay.in -column 2 -row 1
			grid .orbit.orbm.cdelay.rd -column 3 -row 1
			grid .orbit.orbm.cdelay.wt -column 4 -row 1

		frame .orbit.orbm.length
			label .orbit.orbm.length.lb -text "Pulse Length" -width 14
			entry .orbit.orbm.length.in -textvariable orbmlength -width 14
			button .orbit.orbm.length.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorbmlenght
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orbmlength "error"
				} else {set orbmlength $var}
			}
			button .orbit.orbm.length.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetorbmlenght;  exec $pokelocation $addrpoke $orbmlength 4 2 }
			pack .orbit.orbm.length -fill x
			grid .orbit.orbm.length.lb -column 1 -row 1
			grid .orbit.orbm.length.in -column 2 -row 1
			grid .orbit.orbm.length.rd -column 3 -row 1
			grid .orbit.orbm.length.wt -column 4 -row 1

		frame .orbit.orbm.pol
			label .orbit.orbm.pol.lb -text "Pulse Polarity" -width 14
			radiobutton .orbit.orbm.pol.int -text "Pos " -variable orbmpol -value 0x00000000
			radiobutton .orbit.orbm.pol.ext -text "Neg" -variable orbmpol -value 0x00000001
			button .orbit.orbm.pol.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorbmpol
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orbmpol "error"
				} else {set orbmpol $var}
			}
			button .orbit.orbm.pol.wt -text "Write" -command {set addrpoke $brdheader;  append addrpoke $offsetorbmpol;  exec $pokelocation $addrpoke $orbmpol 4 2 }
			pack .orbit.orbm.pol -fill x
			grid .orbit.orbm.pol.lb -column 1 -row 1
			grid .orbit.orbm.pol.int -column 2 -row 1
			grid .orbit.orbm.pol.ext -column 3 -row 1
			grid .orbit.orbm.pol.rd -column 4 -row 1
			grid .orbit.orbm.pol.wt -column 5 -row 1

		frame .orbit.orbm.fdelay
			label .orbit.orbm.fdelay.lb -text "Fine Delay" -width 14
			entry .orbit.orbm.fdelay.in -textvariable orbmfdelay -width 14
			button .orbit.orbm.fdelay.rd -text "Read" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorbmfdelay
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				exec sleep 0.02
				set addrpeek $brdheader
				append addrpeek $offsetfifo25
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var2
				catch {close $pipe}
				if {[string match *error* $var2] > 0 } {
					set orbmfdelay "error"
				} else {
					set var [string range $var2 8 9]
					set var3 0x0;
					append var3 $var;
					set var3 [expr $var3 + 0]
					if {$var3 >  128} {
						set var3 [expr $var3 - 128];
					}
					if { $var3 >= 64 } { .orbit.orbm.fdelay.lb configure -text "  Fine Delay( E )  "; set var3 [expr $var3 - 64 ] ;
					} else { .orbit.orbm.fdelay.lb configure -text "  Fine Delay( D )  ";  }
					set orbmfdelay $var3
				}
			}
			button .orbit.orbm.fdelay.wt -text "Write" -command {
				if { $orbmfdelay > 63 } {	tk_messageBox -type ok -message "Waka!!! Tas pasao.. Max value 63";  set orbmfdelay 63}
				set dataex [expr $orbmfdelay + 64]
				set dataex [format %x $dataex]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetorbmfdelay;
				exec $pokelocation $addrpoke $data 4 2
				set var $data
				set var2 $addrpoke
				set var3 $data
			 }
			pack .orbit.orbm.fdelay -fill x
			grid .orbit.orbm.fdelay.lb -column 1 -row 1
			grid .orbit.orbm.fdelay.in -column 2 -row 1
			grid .orbit.orbm.fdelay.rd -column 3 -row 1
			grid .orbit.orbm.fdelay.wt -column 4 -row 1

		frame .orbit.orbm.blanck0
		pack .orbit.orbm.blanck0 -fill x
			label .orbit.orbm.blanck0.lb -text "  "
			pack .orbit.orbm.blanck0.lb -fill x

		frame .orbit.orbm.blanck1
		pack .orbit.orbm.blanck1 -fill x
			label .orbit.orbm.blanck1.lb -text "  "
			pack .orbit.orbm.blanck1.lb -fill x

		frame .orbit.orbm.blanck2
		pack .orbit.orbm.blanck2 -fill x
			label .orbit.orbm.blanck2.lb -text "  "
			pack .orbit.orbm.blanck2.lb -fill x


		frame .orbit.orbm.blanck3
		pack .orbit.orbm.blanck3 -fill x
			label .orbit.orbm.blanck3.lb -text " ... "
			pack .orbit.orbm.blanck3.lb -fill x

# " INT Orbit Counter/GEN"

		label .orbit.orbm.lbintcounter -text " INT Orbit Counter/GEN"
			pack .orbit.orbm.lbintcounter -fill x

		frame .orbit.orbm.intcounter
			label .orbit.orbm.intcounter.lb -text "Int Periode" -width 14
			button .orbit.orbm.intcounter.en -text "En/Dis-able" -command	{
				# reading actual state of counters
				READINTSTATE
				# activating or deactiving depending of the actual state
				if { $intmen == 1 } {
					set intmen 0;
					.orbit.orbm.intcounter.en configure -text "Enable";
					.orbit.orbm.lbintcounter configure -text "INT Orbit Counter/GEN (DIS)";
				} elseif { $intmen == 0 } {
					set intmen 1;
					.orbit.orbm.intcounter.en configure -text "Disable";
					.orbit.orbm.lbintcounter configure -text "INT Orbit Counter/GEN (EN)";
				}
				set dataex [expr $intmen * 4 + $int2en * 2 + $int1en * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetintcnten
				exec $pokelocation $addrpoke $data 4 2

			}
			button .orbit.orbm.intcounter.rst -text "Reset" -command {
				# RESETING orbit int bc1 ***
				set intmrst 1
				set dataex [expr $intmrst * 4 + $int2rst * 2 + $int1rst * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetintcntrst
				exec $pokelocation $addrpoke $data 4 2
				set var $data
				set intmrst 0
			}
			entry .orbit.orbm.intcounter.in -textvariable orbmintperiode -width 14
			button .orbit.orbm.intcounter.rd -text "Rd Periode" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorbmperiode
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orbmintperiode "error"
				} else {set orbmintperiode $var}
			}
			button .orbit.orbm.intcounter.wt -text "Wr Periode" -command {set addrpoke $brdheader;  append addrpoke $offsetorbmperiode;  exec $pokelocation $addrpoke $orbmintperiode 4 2 }
			pack .orbit.orbm.intcounter
			grid .orbit.orbm.intcounter.lb 	-column 1 -row 1
			grid .orbit.orbm.intcounter.en 	-column 2 -row 1
			grid .orbit.orbm.intcounter.rst 	-column 3 -row 1
			grid .orbit.orbm.intcounter.in 	-column 1 -row 2
			grid .orbit.orbm.intcounter.rd 	-column 2 -row 2
			grid .orbit.orbm.intcounter.wt 	-column 3 -row 2

		frame .orbit.orbm.blanck4
		pack .orbit.orbm.blanck4 -fill x
			label .orbit.orbm.blanck4.lb -text " ... "
			pack .orbit.orbm.blanck4.lb -fill x

		label .orbit.orbm.lbpercounter -text " PERIODE Orbit Counter"
			pack .orbit.orbm.lbpercounter -fill x

#" PERIODE Orbit Counter"

		frame .orbit.orbm.percounter
			label .orbit.orbm.percounter.lb -text "Periode counted" -width 14
			button .orbit.orbm.percounter.en -text "En/Dis-able" -command {
				# reading actual state of counters
				READPERSTATE
				# activating or deactiving depending of the actual state
				if { $permen == 1 } {
					set permen 0;
					.orbit.orbm.percounter.en configure -text "Enable";
					.orbit.orbm.lbpercounter configure -text "PERIODE Orbit Counter (DIS)";
				} elseif { $permen == 0 } {
					set permen 1;
					.orbit.orbm.percounter.en configure -text "Disable";
					.orbit.orbm.lbpercounter configure -text "PERIODE Orbit Counter (EN)";
				}

				set dataex [expr $permen * 4 + $per2en * 2 + $per1en * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetpercnten
				exec $pokelocation $addrpoke $data 4 2

			}
			button .orbit.orbm.percounter.rst -text "Reset" -command {
				# RESETING orbit int bc1 ***
				set permrst 1
				set dataex [expr $permrst * 4 + $per2rst * 2 + $per1rst * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetpercntrst
				exec $pokelocation $addrpoke $data 4 2
				set permrst 0
				set var $data
			}
			entry .orbit.orbm.percounter.in -textvariable orbmpercounter -width 14
			button .orbit.orbm.percounter.rdl -text "RD last" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorbmpercntrdlast
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orbmpercounter "error"
				} else {set orbmpercounter $var}
			}
			button .orbit.orbm.percounter.rdf -text "RD fifo" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorbmpercntrdfifo
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orbmpercounter "error"
				} else {set orbmpercounter $var}
			}
			pack .orbit.orbm.percounter
			grid .orbit.orbm.percounter.lb 	-column 1 -row 1
			grid .orbit.orbm.percounter.en 	-column 2 -row 1
			grid .orbit.orbm.percounter.rst 	-column 3 -row 1
			grid .orbit.orbm.percounter.in 	-column 1 -row 2
			grid .orbit.orbm.percounter.rdl 	-column 2 -row 2
			grid .orbit.orbm.percounter.rdf 	-column 3 -row 2

		frame .orbit.orbm.blanck5
		pack .orbit.orbm.blanck5 -fill x
			label .orbit.orbm.blanck5.lb -text " ... "
			pack .orbit.orbm.blanck5.lb -fill x

# ORbit Counter"

		label .orbit.orbm.lborbcounter -text " Orbit Counter"
			pack .orbit.orbm.lborbcounter -fill x

		frame .orbit.orbm.orbcounter
			button .orbit.orbm.orbcounter.en -text "En/Dis-able" -command {
				# reading actual state of counters
				READORBSTATE
				# activating or deactiving depending of the actual state
				if { $orbmen == 1 } {
					set orbmen 0;
					.orbit.orbm.orbcounter.en configure -text "Enable";
					.orbit.orbm.lborbcounter configure -text "Orbit Counter (DIS)";
				} elseif { $orbmen == 0 } {
					set orbmen 1;
					.orbit.orbm.orbcounter.en configure -text "Disable";
					.orbit.orbm.lborbcounter configure -text "ORbit Counter (EN)";
				}
				set dataex [expr $orbmen * 4 + $orb2en * 2 + $orb1en * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetorbcnten
				exec $pokelocation $addrpoke $data 4 2

			}
			button .orbit.orbm.orbcounter.rst -text "Reset" -command {
				# RESETING orbit int bc1 ***
				set orbmrst 1
				set dataex [expr $orbmrst * 4 + $orb2rst * 2 + $orb1rst * 1]
				set data 0x00
				append data $dataex
				set addrpoke $brdheader
				append addrpoke $offsetorbcntrst
				exec $pokelocation $addrpoke $data 4 2
				set orbmrst 0
				set var $addrpoke
				set var2 $data
			}
			entry .orbit.orbm.orbcounter.in -textvariable orbmorbcounter -width 14
			button .orbit.orbm.orbcounter.rdl -text "RD" -command {
				set addrpeek $brdheader
				append addrpeek $offsetorbmorbcntrd
				set pipe [open "|$peeklocation $addrpeek 4 2"]
				gets $pipe var
				catch {close $pipe}
				if {[string match *error* $var] > 0 } {
					set orbmorbcounter "error"
				} else {set orbmorbcounter $var}
			}
			pack .orbit.orbm.orbcounter
			grid .orbit.orbm.orbcounter.en 	-column 1 -row 1
			grid .orbit.orbm.orbcounter.rst 	-column 2 -row 1
			grid .orbit.orbm.orbcounter.in 	-column 3 -row 1
			grid .orbit.orbm.orbcounter.rdl 	-column 4 -row 1
	

#*********************************************************
# TTCrx and BST control - sophie - new 03.04.07
#----------------------------------------------------------


	frame .orbit.bst -borderwidth 1 -relief raised
	grid .orbit.bst  -column 4 -row 1

		frame .orbit.bst.head
			label .orbit.bst.head.lb -text "BST"
			pack .orbit.bst.head -fill x
			grid .orbit.bst.head.lb 
			
		frame .orbit.bst.init
			label .orbit.bst.init.lb -text "TTCrx chip" -width 10
			button .orbit.bst.init.wt -text "INIT" -command {
				set addrpoke $brdheader
				append addrpoke $offsetttcrxreg
				exec $pokelocation $addrpoke 0x0003 4 2
				set addrpoke $brdheader
				append addrpoke $offsetttcrxdata
				exec $pokelocation $addrpoke 0x00B3 4 2
			}
			pack .orbit.bst.init -fill x
			grid .orbit.bst.init.lb -column 1 -row 1
			grid .orbit.bst.init.wt -column 2 -row 1
			
		set bucle 0
		set bstop 0
		set monitor.mode "Read"
		
		frame .orbit.bst.monitor
		
			
			label .orbit.bst.monitor.stlb -text "Bst Mode:" -width 10
			label .orbit.bst.monitor.st	-textvariable bst  -relief sunken -width 10
			pack .orbit.bst.monitor -fill x
			grid .orbit.bst.monitor.stlb -column 1 -row 1
			grid .orbit.bst.monitor.st -column 2 -row 1
			
			button .orbit.bst.monitor.rd -text "Read" -width 10 -command 	{ set bucle 0; set bstop 0; READBSTSTATE;}
			button .orbit.bst.monitor.mon -text "Monitor" -width 10 -command { set bucle 1; set bstop 0; READBSTSTATE;}
			button .orbit.bst.monitor.stop -text "Stop" -width 10 -command { set bstop 1; 	set bucle 0}
			
			grid .orbit.bst.monitor.rd -column 1 -row 2
			grid .orbit.bst.monitor.mon -column 1 -row 3
			grid .orbit.bst.monitor.stop -column 2 -row 3
			
			label .orbit.bst.monitor.mode -textvariable beam -relief sunken -width 30 -background yellow
			#pack .orbit.bst.monitor.mode
			grid .orbit.bst.monitor.mode -columnspan 3 -row 4
			
			label .orbit.bst.monitor.status -textvariable bstst -relief sunken -width 30 -background yellow		
			#pack .orbit.bst.monitor.status 
			grid .orbit.bst.monitor.status -columnspan 3 -row 5






##_________________________________*************__________________________________________________________________
##_________________________________*************__________________________________________________________________


#blank line
label .sp3 -text ""
pack .sp3


frame .peeklocation
label .peeklocation.labelpeek -text "Vme_Peek Location: " -width 20
entry .peeklocation.entrypeek -textvariable peeklocation -width 30
button .peeklocation.peekdefaultbtt -text "Default Loc" -command PEEKDEFAULT -width 15
pack .peeklocation -fill x
grid .peeklocation.labelpeek -column 1 -row 1
grid .peeklocation.entrypeek -column 2 -row 1
grid .peeklocation.peekdefaultbtt -column 3 -row 1


frame .pokelocation
label .pokelocation.labelpoke -text "Vme_Poke Location: " -width 20
entry .pokelocation.entrypoke -textvariable pokelocation -width 30
button .pokelocation.pokedefaultbtt -text "Default Loc" -command POKEDEFAULT -width 15
pack .pokelocation -fill x
grid .pokelocation.labelpoke -column 1 -row 1
grid .pokelocation.entrypoke -column 2 -row 1
grid .pokelocation.pokedefaultbtt -column 3 -row 1

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
	grid .debug.lb1 -column 1 -row 1
	grid .debug.lb2 -column 2 -row 1
	grid .debug.lb3 -column 3 -row 1



	##############################---##########################################

proc READINTSTATE {} {
	global offsetintcnten
	global intmen
	global int1en
	global int2en
	global var
	global var2
	global var3
	global peeklocation
	global pokelocation
	global brdheader

	set addrpeek $brdheader
	append addrpeek $offsetintcnten
	set pipe [open "|$peeklocation $addrpeek 4 2"]
	gets $pipe varx
	set var $varx
	catch {close $pipe}
# string to number converion
	set varx [expr $varx + 0]
# extracting information
	if { $varx >= 4 } { set intmen 1; set varx [expr $varx -4];	} else { set intmen 0 }
	if { $varx >= 2 } { set int2en 1; set varx [expr $varx -2];	 } else { set int2en 0 }
	if { $varx >= 1 } { set int1en 1; set varx [expr $varx -1];	} else { set int1en 0 }


}


proc READPERSTATE {} {
	global offsetpercnten
	global permen #sophie 18.01.07 was intmper
	global per1en #sophie 18.01.07 was int1per
	global per2en #sophie 18.01.07 was int2per
	global var
	global var2
	global var3
	global peeklocation
	global pokelocation
	global brdheader

	set addrpeek $brdheader
	append addrpeek $offsetpercnten
	set pipe [open "|$peeklocation $addrpeek 4 2"]
	gets $pipe varx
	set var $varx
	catch {close $pipe}
# string to number converion
	set varx [expr $varx + 0]
# extracting information
	if { $varx >= 4 } { set permen 1; set varx [expr $varx -4];	} else { set permen 0 }
	#sophie 18.01.07 was intmper
	if { $varx >= 2 } { set per2en 1; set varx [expr $varx -2];	 } else { set per2en 0 }
	#sophie 18.01.07 was int1per
	if { $varx >= 1 } { set per1en 1; set varx [expr $varx -1];	} else { set per1en 0 }
	#sophie 18.01.07 was int2per


}


proc READORBSTATE {} {
	global offsetorbcnten
	global orbmen
	global orb1en
	global orb2en
	global var
	global var2
	global var3
	global peeklocation
	global pokelocation
	global brdheader

	set addrpeek $brdheader
	append addrpeek $offsetorbcnten
	set pipe [open "|$peeklocation $addrpeek 4 2"]
	gets $pipe varx
	set var $varx
	catch {close $pipe}
# string to number converion
	set varx [expr $varx + 0]
# extracting information
	if { $varx >= 4 } { set orbmen 1; set varx [expr $varx -4];	} else { set orbmen 0 }
	if { $varx >= 2 } { set orb2en 1; set varx [expr $varx -2];	 } else { set orb2en 0 }
	if { $varx >= 1 } { set orb1en 1; set varx [expr $varx -1];	} else { set orb1en 0 }


}


#*************************************************************************************************************
#**********read the bst state*****
#*************************************************************************************************************


proc READBSTSTATE {} {

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
		if {[string match *error* $var] > 0 } { set bstst "error"; .orbit.bst.monitor.status configure -background blue -foreground red
		} elseif { $var  == "0x00000000" } { set bstst "TTCrx & BST not Ready"; .orbit.bst.monitor.status  configure -background red -foreground black
		} elseif { $var  == "0x00000001" } { set bstst "TTCrx & BST Ready"; .orbit.bst.monitor.status configure -background green -foreground black
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
			.orbit.bst.monitor.mode configure -background blue -foreground red
		} elseif { $beam  == "Beam" } {
			.orbit.bst.monitor.mode configure -background green -foreground black
		} elseif { $beam  == "No Beam" } {
			.orbit.bst.monitor.mode configure -background red -foreground black
		}


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




#Comments:
#The code structure is very symple:
#This panel is composed by 4 + 3 blocks, every group of bloks have the same structure and name.
#in case of addres modification, the address variables are in the beginning of the program and with the prename OFFSET. all the variables of the registers have thesame name as their offset.
#
#Master Buttons: (line 210): This section collect the buttons that control the initialization and reset of the board
#
#bc SECTION (260bc1 428bc2 591bcr 757bcmain): compose by 4 blocks which all of them are practically identical. the only thing that change is the BC name.. every row in the panel compose a different frame inside the frame bc.bcx / this make easy to expand coping and paste all the colum and remplacing the bc name with the function remplace in the text editor (be care that all the document are going to be remplaced)
#
#Orbit section: (860orb1 1300orb2 1732orbm) the same that the BC section but composed by 3 blocks).. easy to expand again...
#attention!!! take care with the enable or dissable counter section, this part must be done carefully, due to all of them are controled by bits and when you remplace can damage the bit structure, read coments on the first orbir section (arround line "set dataex" aprox 1110 line)
#
#
#read in state 2140 (read internal counter/generator state)
#read per state 2170 (read perdiode counter state)
#read orb state 2200 (read orbit counter state)
#
#
