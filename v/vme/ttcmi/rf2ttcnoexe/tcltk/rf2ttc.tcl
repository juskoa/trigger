#las modification 11/01/2007 - 19
#/usr/local/bin/wish


wm title . "RF_2_TTC Automatic Mode Control     Angel Monera"


button .manual -text "Manual Control Console" -command { exec wish rf2ttcman.tcl &}
button .auto -text "Automatic Control Console" -command {exec wish rf2ttcaut.tcl &}
button .beam -text "Beam/No Beam Panel"  -command {exec wish rf2ttcbm.tcl &}
label .blk1 -text ""
button .ex -text "Exit" -command {exit}
pack .manual -fill x
pack .auto -fill x
pack .beam -fill x
pack .blk1 -fil x
pack .ex -fill x
