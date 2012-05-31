#!/usr/bin/wish -f
#
# 1.7.2006
# always when file written, its name printed to stdout in format:
#written:name.seq
proc openfile {file array} {
	global num_seq
	global PP
	global L0
	global L1
	global L1M
	global L1L1M
	global L2M
	global L2R
	upvar $array arr
	exec touch $file
	set f [open $file.seq r]
#	while {![eof $f]} {
	set a 1
	set b 0
	gets $f name
	gets $f num_seq
	gets $f errcode
#
	set PP [lindex [split $errcode ""] 0]
	set L0 [lindex [split $errcode ""] 1]
	set L1 [lindex [split $errcode ""] 2]
	set L1M [lindex [split $errcode ""] 3]
	set L1L1M [lindex [split $errcode ""] 4]
	set L2M [lindex [split $errcode ""] 5]
	set L2R [lindex [split $errcode ""] 6]
#
	while {$a <= $num_seq} {
		while {$b <= 7} {
			gets $f arr($a,$b)
			incr b
		}
		incr a
		set b 0
	}	
#	}
	close $f
}
proc writefile {file array} {
	global num_seq                   
	global PP
	global L0
	global L1
	global L1M
	global L1L1M
	global L2M
	global L2R
	upvar $array arr
	set f [open $file.seq w+]
	set errcode 00000		  ;# just for now
	puts $f $file
	puts $f $num_seq
	set errcode [concat $PP$L0$L1$L1M$L1L1M$L2M$L2R]
	puts $f $errcode
	set a 1
	set b 0
	while {$a <= 32} {
		while {$b <= 7} {
			puts $f $arr($a,$b)
			set b [expr $b+1]
		}
		set a [expr $a+1]
		set b 0
	}	
	close $f
        puts "written:$file.seq"
}
proc format_file {L1cl L2cl er code wd last res rc clus} {
	global num_seq
	upvar $L1cl L1file
	upvar $L2cl L2file
	upvar $er error
	upvar $code scode
	upvar $wd word
	upvar $last la 
	upvar $res rs
	upvar $rc roc
	upvar $clus dc
	
	set sn 1
	set la($num_seq) 1
	while {$sn <= $num_seq} {
	set cit(1) [lindex [split $scode($sn) ""] 0]
	set bits(1) [concat $roc($sn,3)$roc($sn,2)$roc($sn,1)$roc($sn,0)]
	set word($sn,0) \
	[concat 0$cit(1)$bits(1)0$cit(1)$L1file($sn,50)$L1file($sn,50)]
	set word($sn,0) \
	[concat $word($sn,0)$error($sn)$la($sn)$rs($sn)$scode($sn)]
#
	set id 48
	set word($sn,1) ""
	while {$id >= 33} {
		set word($sn,1) [concat $word($sn,1)$L1file($sn,$id)]
		incr id -1
	}	
	set id 32
	set word($sn,2) ""
	while {$id >= 17} {
		set word($sn,2) [concat $word($sn,2)$L1file($sn,$id)]
		incr id -1
	}	
	set id 16
	set word($sn,3) ""
	while {$id >= 1} {
		set word($sn,3) [concat $word($sn,3)$L1file($sn,$id)]
		incr id -1
	}	
	set L2Cl \
	[concat $dc($sn,5)$dc($sn,4)$dc($sn,3)$dc($sn,2)$dc($sn,1)$dc($sn,0)]
	set word($sn,4) \
	[concat 000$cit(1)$cit(1)$L2Cl$L2file($sn,50)$L2file($sn,49)]
	set word($sn,4) [concat $word($sn,4)$L2file($sn,48)$L2file($sn,47)]
	set word($sn,4) [concat $word($sn,4)$L2file($sn,46)]
#	
	set id2 45
	set word($sn,5) ""
	while {$id2 >= 30} {
		set word($sn,5) [concat $word($sn,5)$L2file($sn,$id2)]
		incr id2 -1
	}
	set id2 29
	set word($sn,6) ""
	while {$id2 >= 14} {
		set word($sn,6) [concat $word($sn,6)$L2file($sn,$id2)]
		incr id2 -1
	}
	set id2 13
	set word($sn,7) ""
	while {$id2 >= 1} {
		set word($sn,7) [concat $word($sn,7)$L2file($sn,$id2)]
		incr id2 -1
	}
		set word($sn,7) [concat $word($sn,7)000]
		set sn [expr $sn+1]
	}
}	
proc decode_file {L1cl L2cl er ern code wd res rc clus} {
	global num_seq
	upvar $L1cl L1file
	upvar $L2cl L2file
	upvar $ern error
	upvar $er err
	upvar $code scode
	upvar $wd word
	upvar $res rs
	upvar $rc roc
	upvar $clus dc
	set sn 1 
	while {$sn <= $num_seq} {
		set roc($sn,3) [lindex [split $word($sn,0) ""] 2]
		set roc($sn,2) [lindex [split $word($sn,0) ""] 3]
		set roc($sn,1) [lindex [split $word($sn,0) ""] 4]
		set roc($sn,0) [lindex [split $word($sn,0) ""] 5]
		set L1file($sn,50) [lindex [split $word($sn,0) ""] 8]
		set L1file($sn,49) [lindex [split $word($sn,0) ""] 9]
		set error($sn) [lindex [split $word($sn,0) ""] 10] 
		if {$error($sn) == 1} {set err($sn) Yes}
		set rs($sn) [lindex [split $word($sn,0) ""] 12]
		set a [lindex [split $word($sn,0) ""] 13] 
		set b [lindex [split $word($sn,0) ""] 14] 
		set c [lindex [split $word($sn,0) ""] 15] 
		set scode($sn) [concat $a$b$c]
		set i 0
		set j 48
		while {$i <= 15} {
			set L1file($sn,$j) [lindex [split $word($sn,1) ""] $i] 
			incr i
			incr j -1
		}	
		set i 0
		set j 32
		while {$i <= 15} {
			set L1file($sn,$j) [lindex [split $word($sn,2) ""] $i] 
			incr i
			incr j -1
		}	
		set i 0
		set j 16
		while {$i <= 15} {
			set L1file($sn,$j) [lindex [split $word($sn,3) ""] $i] 
			incr i
			incr j -1
		}	
		set dc($sn,5) [lindex [split $word($sn,4) ""] 5]
		set dc($sn,4) [lindex [split $word($sn,4) ""] 6]
		set dc($sn,3) [lindex [split $word($sn,4) ""] 7]
		set dc($sn,2) [lindex [split $word($sn,4) ""] 8]
		set dc($sn,1) [lindex [split $word($sn,4) ""] 9]
		set dc($sn,0) [lindex [split $word($sn,4) ""] 10]
		set L2file($sn,50) [lindex [split $word($sn,4) ""] 11]
		set L2file($sn,49) [lindex [split $word($sn,4) ""] 12]
		set L2file($sn,48) [lindex [split $word($sn,4) ""] 13]
		set L2file($sn,47) [lindex [split $word($sn,4) ""] 14]
		set L2file($sn,46) [lindex [split $word($sn,4) ""] 15]
		set i 0
		set j 45
		while {$i <= 15} {
			set L2file($sn,$j) [lindex [split $word($sn,5) ""] $i]
			incr i
			incr j -1
		}
		set i 0
		set j 29
		while {$i <= 15} {
			set L2file($sn,$j) [lindex [split $word($sn,6) ""] $i]
			incr i
			incr j -1
		}
		set i 0
		set j 13
		while {$i <= 12} {
			set L2file($sn,$j) [lindex [split $word($sn,7) ""] $i]
			incr i
			incr j -1
		}
		incr sn
	}
}	
#
proc listseq {} {
#	.ltext2 delete 1.0 end
#	.seqlist insert end [eval exec ls [glob -nocomplain *.seq]] 
#	exec [ls *.seq | sed 's/.seq$//' > seq.list]
##	exec /home/alice/trigger/v/vme/ltu/seq_list
##	set h [open seq.list r]
##	.seqlist insert end [read $h]
##	close $h
if [catch {set seqnames [glob *.seq]} errmsg] {
  #puts $errmsg
} else {
  foreach seqname $seqnames {
    .seqlist insert end [file rootname $seqname]
    .seqlist insert end \n
  }
}

}
# 
set mtype 0
set stype 1
global seq_num
global num_seq
set seqname "one"
set seqname2 "one"
frame .tot -bg #0af
frame .allr -bg #fa0
frame .all -bg #0af
frame .one -bg #0af
frame .two -bg #0af
frame .three -bg #0af
pack .tot
pack .all .allr -side left -pady 1m -padx 2m -fill x -fill y -in .tot
#pack .all .allr -side left -fill x -fill y -in .tot
#pack .one .two .three -pady 2m -padx 2m -fill x -in .all
pack .one .two .three -padx 2m -fill x -in .all
frame .topleft -bg #ff8
frame .topright -bg #0f0
frame .topright2 -bg #0f0
frame .topright3 -bg #0f0 
frame .middleleft -bg #0f0
frame .middleleft2 -bg #0f0
frame .middleright -bg #0f0
frame .middleright2 -bg #0f0
frame .three-left -bg #0f0
frame .three-right -bg #0f0
frame .bottoma -bg #0f0
frame .bottomb -bg #0f0
frame .bottomc -bg #0f0
frame .bottomd -bg #0f0
frame .bottom -bg #0af
frame .bottom2 -bg #0f0
frame .bottom3 -bg #fa0
frame .bottom-left -bg #0f0
frame .bottom-med -bg #0f0
frame .bottom-right -bg #0f0
frame .bottom2-left -bg #0f0
frame .bottom2-med -bg #0f0
frame .bottom2-right -bg #0f0
frame .bottom3t -bg #fa0
frame .bottom3m -bg #0f0
frame .bottom3b -bg #fa0
frame .bottom3tl -bg #0f0
frame .bottom3tr -bg #0f0
pack .topleft .topright .topright2 .topright3 -side left -padx 6m \
-pady 1m -in .one
#pack .topright2 .topright -side right -padx 3m -pady 3m -in .one
pack .middleleft .middleleft2 .middleright .middleright2 -side left -fill y\
-padx 4m -pady 1m -in .two
frame .midrightup -bg #0f0
frame .midrightmid -bg #0f0
frame .midrightdown -bg #0f0
#pack .midrightup .midrightmid .midrightdown -side top -padx 3m -pady 6m \
#-in .middleright2
#pack .middleright2 .middleright -side right -padx 3m -pady 3m -in .two
pack .bottom .bottom3 -side left -padx 3m -pady 1m -in .three
pack .bottoma .bottomb -side top -padx 2m -in .bottom
pack .bottomc .bottomd -side left -padx 2m -pady 1m -in .bottomb
pack .bottom-left .bottom-med .bottom-right -side left -in .bottomc
pack .bottom2-left .bottom2-med .bottom2-right -side left -in .bottomd
pack .bottom3t .bottom3m .bottom3b -side top -in .bottom3
pack .bottom3tl .bottom3tr -side left -padx 2m -in .bottom3t
#
frame .rightone -bg #fa0
frame .righttwo -bg #fa0
frame .rightthree -bg #fa0
frame .rightfour -bg #fa0
pack .rightone .righttwo .rightthree .rightfour -side top -pady 1m -in .allr
frame .toplr
frame .topmr
frame .toprr -bg #fa0
pack .toplr .topmr .toprr -side left -padx 3m -pady 1m -in .righttwo -fill y
frame .toplr2
frame .topmr2
frame .toprr2 -bg #fa0
pack .toplr2 .topmr2 .toprr2 -side left -padx 3m -pady 1m -in .rightfour \
-fill y
#
##############################################################
#
proc sequence {number name name2 name3} {
		global seq_num
        	label .l$number -textvariable $name3 -bg green -anchor w
        	radiobutton .btest$number -textvariable $name -variable \
        	seq_num -value $number -anchor w -bg green -command \
		{remake L1class; remake2 L2class; remake3 dc; remake4 roc}
        	label .lt$number -textvariable $name2 -anchor w -bg #ff8
		if {$number < 17} {
        		pack .l$number -fill x -pady 0.2m -in .bottom-left
        		pack .btest$number -fill x -in .bottom-med
        		pack .lt$number -fill x -pady 0.2m -in .bottom-right
		} else {
        		pack .l$number -fill x -pady 0.2m -in .bottom2-left
			pack .btest$number -fill x -in .bottom2-med
        		pack .lt$number -fill x -pady 0.2m -in .bottom2-right
		}
	} 
proc warning {} {
	bell 
	.comments insert 1.0 \
	"max number\nof trigger\nsequences\nis 32 \n \n"
#	toplevel .warning
#        button .warning.b -bg red -text \
#        "max number of trigger \n sequences is 32 \n OK ?" -command \
#	{destroy .warning}
#        pack .warning.b
}	
proc emseq {name flag mess messb messc err res} {
	global seq_num num_seq 
	upvar $flag fl
	upvar $mess me
	upvar $messb meb
	upvar $messc mec
	upvar $err er
	upvar $res rs
	#
	if {$seq_num > 32} {
		warning
		return
	} else {
		if {$fl($seq_num) == 0} {
			sequence $seq_num mess($seq_num) messb($seq_num) \
			messc($seq_num)
			set fl($seq_num) 1
			incr num_seq
		}
		set mec($seq_num) $seq_num
		if {$rs($seq_num) == 1} {set mec($seq_num) "$seq_num R"}
		set me($seq_num)  $name 
		set meb($seq_num) [concat " err allowed = " $er($seq_num)] 
		set seq_num [expr $seq_num+1]
		set rc 0
	}
}
proc load_seq {scode flag mess messb messc err res} {
	global seq_num 
	global num_seq
	upvar $scode sc
	upvar $flag fl
	upvar $mess me
	upvar $messb meb
	upvar $messc mec
	upvar $err er
	upvar $res rs
	#
	set seq_num 1
	while {$seq_num <= $num_seq} {
		sequence $seq_num mess($seq_num) messb($seq_num) \
		messc($seq_num)
		set fl($seq_num) 1
		set mec($seq_num) $seq_num
		if {$sc($seq_num) == 001} {set me($seq_num) L0 
		} elseif {$sc($seq_num) == 010} {set me($seq_num) L0-L1-L2a 
		} elseif {$sc($seq_num) == 011} {set me($seq_num) L0-L1-L2r 
	     } elseif {$sc($seq_num) == 100} {set me($seq_num) "Pre-Pulse (PP)" 
		} elseif {$sc($seq_num) == 101} {set me($seq_num) PP-L0 
		} elseif {$sc($seq_num) == 110} {set me($seq_num) PP-L0-L1-L2a 
		} elseif {$sc($seq_num) == 111} {set me($seq_num) PP-L0-L1_l2r 
		} else {set me($seq_num) ERROR}
		set meb($seq_num) [concat " err allowed = " $er($seq_num)] 
		if {$rs($seq_num) == 1} {set mec($seq_num) "$seq_num R"}
		incr seq_num
	}
}
proc remake L1c  {
	global seq_num
	upvar $L1c L1class
	set id 1
	while {$id <= 50} {
		destroy .1c$id
		set id [expr $id+1]
	}	
	set cl 1
	while {$cl<51} {
		checkbutton .1c$cl -text "class $cl" -variable \
		L1class($seq_num,$cl) -anchor w -bg #ff8
		set cl [expr $cl+1]
	}
pack .l1class -in .rightone -fill x
pack .1c1 .1c2 .1c3 .1c4 .1c5 .1c6 .1c7 .1c8 .1c9 .1c10 .1c11 \
.1c12 .1c13 .1c14 .1c15 .1c16 .1c17 -in .toplr -side top -fill x 
pack .1c18 .1c19 .1c20 .1c21 .1c22 .1c23 .1c24 .1c25 .1c26 .1c27 \
.1c28 .1c29 .1c30 .1c31 .1c32 .1c33 .1c34 -in .topmr -side top -fill x 
pack .1c35 .1c36 .1c37 .1c38 .1c39 .1c40 .1c41 .1c42 .1c43 .1c44 \
.1c45 .1c46 .1c47 .1c48 .1c49 .1c50  -in .toprr -side top -fill x 
}
#
proc remake2 L2c {
	global seq_num
	upvar $L2c L2class
	set id 1
	while {$id <= 50} {
		destroy .2c$id
		set id [expr $id+1]
	}	
	set cl 1
	while {$cl<51} {
		checkbutton .2c$cl -text "class $cl" -variable \
		L2class($seq_num,$cl) -anchor w -bg #ff8
		set cl [expr $cl+1]
	}
pack .l2class -in .rightthree -fill x
pack .2c1 .2c2 .2c3 .2c4 .2c5 .2c6 .2c7 .2c8 .2c9 .2c10 .2c11 \
.2c12 .2c13 .2c14 .2c15 .2c16 .2c17 -in .toplr2 -side top -fill x 
pack .2c18 .2c19 .2c20 .2c21 .2c22 .2c23 .2c24 .2c25 .2c26 .2c27 \
.2c28 .2c29 .2c30 .2c31 .2c32 .2c33 .2c34 -in .topmr2 -side top -fill x 
pack .2c35 .2c36 .2c37 .2c38 .2c39 .2c40 .2c41 .2c42 .2c43 .2c44 \
.2c45 .2c46 .2c47 .2c48 .2c49 .2c50  -in .toprr2 -side top -fill x 
}
#
proc remake3 clus {
	global seq_num
	upvar $clus dc
	set id 1
	while {$id <= 6} {
		destroy .dc$id 
		incr id
		}
	set id 1
	set el 0
	while {$id <= 6} {
		checkbutton .dc$id -text "L2Cl\[$el\]" -variable \
		dc($seq_num,$el) -anchor w -bg #ff8
		incr id
		incr el
		}
	pack .cluster .dc1 .dc2 .dc3 .dc4 .dc5 .dc6 -in .bottom3tl -side top \
	-fill x
}	
#
proc remake4 rc {
	global seq_num
	upvar $rc roc
	set id 1
	while {$id <= 4} {
		destroy .r$id 
		incr id
		}
	set id 1
	set el 0
	while {$id <= 4} {
		checkbutton .r$id -text "RoC\[$el\]" -variable \
		roc($seq_num,$el) -anchor w -bg #ff8
		incr id
		incr el
		}
	pack .roc .r1 .r2 .r3 .r4 -in .bottom3tr -side top -fill x
}	
#
proc delete {sn L1cl L2cl fl er ern word res} {
	global num_seq
	global seq_num
	upvar $fl flag
	upvar $er err
	upvar $L1cl L1class
	upvar $L2cl L2class
	upvar $ern errn
	upvar $word wd
	upvar $res rs
#
	if {$sn == 0} {return}
	set id $sn
	while {$id <= $num_seq} {
		destroy .l$id .btest$id .lt$id
		set flag($id) 0
		set err($id) No
		set errn($id) 0
		set el 1
		while {$el <= 50} {
			set L1class($id,$el) 0
			set L2class($id,$el) 0
			incr el
		}
		set el 0
		while {$el <= 7} {
			set wd($id,$el) 0000000000000000
			incr el
		}
		incr id
	}
	if {$seq_num == [expr $num_seq+1]} {
		incr seq_num -1
	}
	incr num_seq -1
	set rs($sn) 0
	if {$sn == 1} {
		set seq_num 1
		set num_seq 0
	}
}
proc delete2 {sn L1cl L2cl fl er ern word res ms msb msc sc rc clus} {
	global num_seq
	global seq_num
	upvar $fl flag
	upvar $er err
	upvar $L1cl L1class
	upvar $L2cl L2class
	upvar $ern errn
	upvar $word wd
	upvar $res rs
	upvar $ms mess
	upvar $msb messb
	upvar $msc messc
	upvar $sc scode
	upvar $rc roc
	upvar $clus dc
#
	if {$sn == 0} {return}
	set id [expr $sn+1]
	set idt 1
	while {$id <= $num_seq} {
#		set flagt($idt) flag($id)
		set errt($idt) $err($id)
		set errnt($idt) $errn($id)
		set rst($idt) $rs($id)
		set messt($idt) $mess($id)
		set messbt($idt) $messb($id)
		set messct($idt) $messc($id)
		set scodet($idt) $scode($id)
		set el 1
		while {$el <= 50} {
			set L1classt($idt,$el) $L1class($id,$el)
			set L2classt($idt,$el) $L2class($id,$el)
			incr el
			}
		set el 0
		while {$el <= 7} {
			set wdt($idt,$el) $wd($id,$el)
			incr el
			}
		set el 0
		while {$el <= 3} {
			set roct($idt,$el) $roc($id,$el)
			incr el
			}
		set el 0
		while {$el <= 5} {
			set dct($idt,$el) $dc($id,$el)
			incr el
			}
		incr id
		incr idt
		}
	set num_seqt [expr $num_seq - 1]
	delete $sn L1class L2class flag err errn wd rs
	set num_seq [expr $sn - 1]
#	
	set id $sn
	set idt 1
	while {$id <= $num_seqt} {
		set flag($id) 0
		set err($id) $errt($idt)
		set errn($id) $errnt($idt)
		set rs($id) $rst($idt)
		set mess($id) $messt($idt)
		set messb($id) $messbt($idt)
		set messc($id) $messct($idt)
		set scode($id) $scodet($idt)
		set el 1
		while {$el <= 50} {
			set L1class($id,$el) $L1classt($idt,$el)
			set L2class($id,$el) $L2classt($idt,$el)
			incr el
			}
		set el 0
		while {$el <= 7} {
			set wd($id,$el) $wdt($idt,$el)
			incr el
			}
		set el 0
		while {$el <= 3} {
			set roc($id,$el) $roct($idt,$el)
			incr el
			}
		set el 0
		while {$el <= 5} {
			set dc($id,$el) $dct($idt,$el)
			incr el
			}
		incr id
		incr idt
		}
	set seq_num $sn
	set id $sn
	set idt 1
	while {$id <= $num_seqt} {
		set name $messt($idt)
		emseq $name flag mess messb messc err rs
		incr idt
		incr id
		}

}	
#
##########################################################################
set seq_num 1
set num_seq 0
set a 1
while {$a<33} {
	set b 0
	while {$b<51} {
		set L1class($a,$b) 0
		set L2class($a,$b) 0
		set L2class($a,$b) 0
		set b [expr $b+1]
	}
	set c 0
	while {$c<8} {
		set wd($a,$c) 0000000000000000
		incr c
	}	
#	
	set flag($a) 0
	set err($a) No
	set errn($a) 0
	set last($a) 0
	set res($a) 0
        set a [expr $a+1]
	set mess($a) Blank
	set messb($a) Blank
	set messc($a) Blank
	set scode($a) 000
	}
#
if {$argc == 1} {
	#puts "argc==1: $argv"
	if {[file exists $argv.seq] == 1} {
		openfile $argv wd 
		decode_file L1class L2class err errn scode wd res roc dc
		load_seq scode flag mess messb messc err res
		set seqname2 $argv
		set seqname $argv
                #puts $argv
		}
	}	
#
#
#label .type -text "Mode" -bg #ff8
#radiobutton .norm -text "Start Emulation" -variable mtype -value 0 -anchor w \
#-relief groove -bg #ff8
#radiobutton .emul -text "Stop Emulation" -variable mtype -value 1 -anchor w \
#-relief groove -bg #ff8
button .exit -text "Stop & Exit" -bg red -command {exit}
#pack .type .norm .emul .exit -in .topleft -side top -fill x 
pack .exit -in .topleft -side top -fill x 
#
#label .sequ -text "Sequence Pattern" -bg #ff8
#radiobutton .single -text "Single pass" -variable stype -value 1 -anchor w \
#-command {set messc($restart) $restart; set restart 0} -bg #ff8
#radiobutton .contin -text "Continuous loop" -variable stype -value 2 \
#-anchor w -command {set messc($restart) $restart; set restart 0} -bg #ff8
#radiobutton .xcontin -text "Extended continuous loop" -variable stype \
#-value 3 -anchor w -bg #ff8
#radiobutton .xvar -text "Extended loop variation" -variable stype \
#-value 4 -anchor w -bg #ff8
#pack .sequ .single .contin .xcontin .xvar -in .topright -side top -fill x
#
label .ltext -text "Comments" -bg #0f0
text .comments -relief sunken -bd 2 -width 14 -height 6 -bg #f66\
-yscrollcommand ".scroll set"
scrollbar .scroll -command ".comments yview"
pack .ltext -side top -in .topright2
pack .comments -side left -in .topright2
pack .scroll -side right -fill y -in .topright2
#
# text to list sequence files
#
label .ltext2 -text "List of\nseq. files" -bg #0f0
text .seqlist -relief sunken -bd 2 -width 14 -height 6 -bg #6ff\
-yscrollcommand ".scroll2 set"
scrollbar .scroll2 -command ".seqlist yview"
pack .ltext2 -side top -in .topright3
pack .seqlist -side left -in .topright3
pack .scroll2 -side right -fill y -in .topright3
#
listseq
#
label .empat -text "Emulation Sequence" -bg #0f0
button .s1  -text "L0" -command {set scode($seq_num) 001; \
emseq L0 flag mess messb messc err res} -bg #0af
button .s2  -text "L0-L1-L2a" -command {set scode($seq_num) 010; \
emseq L0-L1-L2a flag mess messb messc err res} -bg #0af
button .s3  -text "L0-L1-L2r" -command {set scode($seq_num) 011; \
emseq L0-L1-L2r flag mess messb messc err res} -bg #0af
button .s4 -text "Pre-Pulse (PP)" -command {set scode($seq_num) 100; \
emseq PP flag mess messb messc err res} -bg #0af
button .s5 -text "PP-L0" -command {set scode($seq_num) 101; \
emseq PP-L0 flag mess messb messc err res} -bg #0af 
button .s6 -text "PP-L0-L1-L2a" -command {set scode($seq_num) 110; \
emseq PP-L0-L1-L2a flag mess messb messc err res} -bg #0af
button .s7 -text "PP-L0-L1-L2r" -command {set scode($seq_num) 111; \
emseq PP-L0-L1-L2r flag mess messb messc err res} -bg #0af
pack .empat .s1 .s2 .s3 .s4 .s5 .s6 .s7 -pady 2m -padx 2m -in .middleleft \
-side top -fill x
#
label .emcon -text "Sequence options" -bg #0f0 
button .sc1 -text "Insert\nRestart\nflag" -command {
set res($seq_num) 1; set messc($seq_num) "$seq_num R"} -bg #0af
button .sc2 -text "Remove\n Restart\nflag" \
-command {set messc($seq_num) $seq_num; set res($seq_num) 0} -bg #0af
button .sc3 -text "Delete current\nsequence" -command \
{delete2 $seq_num L1class L2class flag err errn wd res mess messb messc \
scode roc dc} -bg #0af
button .sc4 -text "Delete last\nsequence" -command \
{delete $num_seq L1class L2class flag err errn wd res} -bg #0af
button .sc5 -text "Delete all\nsequences" -command \
{delete 1 L1class L2class flag err errn wd res} -bg #f00
#{set id 1; while {$id <= $num_seq} {\
#destroy .l$id .btest$id .lt$id; set flag($id) 0; set err($id) No; incr id}; \
#set num_seq 0; set restart 0; set seq_num 1} 
# others sets needed maybe use proc to reset everything
pack .emcon .sc1 .sc2 .sc3 .sc4 .sc5 -pady 2m -padx 2m -in .middleleft2 -side \
top -fill x
#
label .cluster -text "Detector\nClusters" -bg #fa0
checkbutton .dc1 -text "L2Cl\[5\]" -variable dc($seq_num,5) -anchor w -bg #ff8
checkbutton .dc2 -text "L2Cl\[4\]" -variable dc($seq_num,4) -anchor w -bg #ff8
checkbutton .dc3 -text "L2Cl\[3\]" -variable dc($seq_num,3) -anchor w -bg #ff8
checkbutton .dc4 -text "L2Cl\[2\]" -variable dc($seq_num,2) -anchor w -bg #ff8
checkbutton .dc5 -text "L2Cl\[1\]" -variable dc($seq_num,1) -anchor w -bg #ff8
checkbutton .dc6 -text "L2Cl\[0\]" -variable dc($seq_num,0) -anchor w -bg #ff8
pack .cluster .dc1 .dc2 .dc3 .dc4 .dc5 .dc6 -in .bottom3tl -side top -fill x
#
label .roc -text "User bits\nRoC \[3..0\]" -bg #fa0
checkbutton .r1 -text "RoC\[3\]" -variable roc($seq_num,3) -anchor w -bg #ff8
checkbutton .r2 -text "RoC\[2\]" -variable roc($seq_num,2) -anchor w -bg #ff8
checkbutton .r3 -text "RoC\[1\]" -variable roc($seq_num,1) -anchor w -bg #ff8
checkbutton .r4 -text "RoC\[0\]" -variable roc($seq_num,0) -anchor w -bg #ff8
pack .roc .r1 .r2 .r3 .r4 -in .bottom3tr -side top -fill x
#
label .aerr -text "Allow error with \n current sequence ?" -bg #fa0
button .aerr1 -text "Yes" -command {set errn($seq_num) 1; \
set messb($seq_num) "err allowed = Yes"; set err($seq_num) Yes} -bg #0af
button .aerr2 -text "No" -command {set errn($seq_num) 0; \
set messb($seq_num) "err allowed = No"; set err($seq_num) No} -bg #0af
pack .aerr .aerr1 .aerr2 -pady 2m -padx 2m -in .bottom3b -side top -fill x
#
label .readseq -text "Read/Write sequence file" -bg #0f0
#button .prog1 -text "Seq file 1" -command \
#{openfile seq1.file wd}
#button .prog2 -text "Seq file 2" -command \
#{openfile seq2.file wd}
#button .prog3 -text "Seq file 3" -command \
#{openfile seq3.file wd}
#button .prog4 -text "Seq file 4" -command \
#{openfile seq4.file wd}
label .save -text "Output sequence\nfile name:" -bg #0f0
entry .seqname -width 10 -relief sunken -bd 2 -textvariable seqname
label .get -text "Input sequence\nfile name:" -bg #0f0
entry .seqname2 -width 10 -relief sunken -bd 2 -textvariable seqname2
button .prog -text "Read Sequence" -command \
{delete 1 L1class L2class flag err errn wd res; openfile $seqname2 wd; \
decode_file L1class L2class err errn scode wd res roc dc; \
load_seq scode flag mess messb messc err res} -bg #f00
button .acc -text "Write Sequence" -command {format_file L1class L2class \
errn scode wd last res roc dc; writefile $seqname wd} -bg #f00
pack .readseq .save .seqname .get .seqname2 .prog .acc -pady 2m -padx 2m \
-in .middleright \
-side top -fill x
#
label .errors -text "Allow errors with" -bg #ff8
#checkbutton .e5 -text "Pre-pulse" -anchor w
checkbutton .e1 -text "Pre-pulse" -variable PP -anchor w -bg #ff8 
checkbutton .e2 -text "L0" -variable L0 -anchor w -bg #ff8
checkbutton .e3 -text "L1" -variable L1 -anchor w -bg #ff8 
checkbutton .e4 -text "L1 Message" -variable L1M -anchor w -bg #ff8 
checkbutton .e5 -text "L1 & L1 Message" -variable L1L1M -anchor w -bg #ff8 
checkbutton .e6 -text "L2a Message" -variable L2M -anchor w -bg #ff8 
checkbutton .e7 -text "L2r" -variable L2R -anchor w -bg #ff8 
pack .errors .e1 .e2 .e3 .e4 .e5 .e6 .e7 -in .topright -side top -fill x
#
#label .save -text "Save current\n sequence as:" -bg #0f0
#entry .seqname -width 10 -relief sunken -bd 2 -textvariable seqname
#pack .save .seqname -side top -in .midrightup
#
#label .get -text "Input sequence\n name:" -bg #0f0
#entry .seqname2 -width 10 -relief sunken -bd 2 -textvariable seqname2
#pack .get .seqname2 -side top -in .midrightmid
#
label .lclass -text "Class options" -bg #0f0
#
button .l1opt1 -text "set all L1class to 1" -command { \
set id 1; while {$id <= $num_seq} {set cl 1; while {$cl <= 50} \
{set L1class($id,$cl) 1; incr cl}; incr id}} -bg #0af
button .l1opt2 -text "set all L1class to 0" -command { \
set id 1; while {$id <= $num_seq} {set cl 1; while {$cl <= 50} \
{set L1class($seq_num,$cl) 0; incr cl}; incr id}} -bg #0af
button .l1opt3 -text "set all L1class to\n alternating 0 1" -command { \
set id 1; while {$id <= $num_seq} {set cl 1; while {$cl <= 50} \
{set L1class($id,$cl) 1; incr cl}; incr id}; \
set id 1; while {$id <= $num_seq} {set cl 1; while {$cl <= 50} \
{set L1class($id,$cl) 0; incr cl +2}; incr id}} -bg #0af
#
button .l2set -text "Set L2class = L1class\nfor current seq" \
-command {set cl 1; while {$cl <= 50} {set L2class($seq_num,$cl) \
$L1class($seq_num,$cl); incr cl}} -bg #0af
#pack .l2set -side top -in .midrightup
button .l2seta -text "Set L2class = L1class\nfor all sequences" \
-command {set id 1; while {$id <= $num_seq} {set cl 1; while {$cl <= 50} \
{set L2class($id,$cl) $L1class($id,$cl); incr cl}; incr id}} -bg #0af
#pack .l2seta -side top -in .midrightmid
#pack .acc -side top -in .midrightdown
pack .lclass .l1opt1 .l1opt2 .l1opt3 .l2set .l2seta -fill x -side top \
-pady 2m -padx 2m -in .middleright2
#
label .l1class -text "Active Level 1 Classes" -bg #ff8
set cl 1
while {$cl<51} {
	checkbutton .1c$cl -text "class $cl" -variable L1class($seq_num,$cl) \
	-anchor w -bg #ff8
	set cl [expr $cl+1]
	}
pack .l1class -in .rightone -fill x
pack .1c1 .1c2 .1c3 .1c4 .1c5 .1c6 .1c7 .1c8 .1c9 .1c10 .1c11 \
.1c12 .1c13 .1c14 .1c15 .1c16 .1c17 -in .toplr -side top -fill x 
pack .1c18 .1c19 .1c20 .1c21 .1c22 .1c23 .1c24 .1c25 .1c26 .1c27 \
.1c28 .1c29 .1c30 .1c31 .1c32 .1c33 .1c34 -in .topmr -side top -fill x 
pack .1c35 .1c36 .1c37 .1c38 .1c39 .1c40 .1c41 .1c42 .1c43 .1c44 \
.1c45 .1c46 .1c47 .1c48 .1c49 .1c50  -in .toprr -side top -fill x 
label .l2class -text "Active Level 2 Classes" -bg #ff8
set cl 1
while {$cl<51} {
	checkbutton .2c$cl -text "class $cl" -variable L2class($seq_num,$cl) \
	-anchor w -bg #ff8
	set cl [expr $cl+1]
	}
pack .l2class -in .rightthree -fill x
pack .2c1 .2c2 .2c3 .2c4 .2c5 .2c6 .2c7 .2c8 .2c9 .2c10 .2c11 \
.2c12 .2c13 .2c14 .2c15 .2c16 .2c17 -in .toplr2 -side top -fill x 
pack .2c18 .2c19 .2c20 .2c21 .2c22 .2c23 .2c24 .2c25 .2c26 .2c27 \
.2c28 .2c29 .2c30 .2c31 .2c32 .2c33 .2c34 -in .topmr2 -side top -fill x 
pack .2c35 .2c36 .2c37 .2c38 .2c39 .2c40 .2c41 .2c42 .2c43 .2c44 \
.2c45 .2c46 .2c47 .2c48 .2c49 .2c50  -in .toprr2 -side top -fill x 
#
label .currlist -text "Current list of Sequences" -bg #0f0
pack .currlist -side top -in .bottoma
