#rrdtool graph graf.png -s 14:00 31.01.2013 -e start+1h --step 60 -w 600 -u 10000 -l 8000 DEF:ds001=rrd/mon.rrd:ds001:AVERAGE DEF:ds002=rrd/mon.rrd:ds002:AVERAGE DEF:ds003=rrd/mon.rrd:ds003:AVERAGE LINE1:ds001#003300:ds001 LINE1:ds002#0000cc:ds002 LINE1:ds003#660000:ds003 
#rrdtool - <<-EOF
# forcing maximum Y axis > max Y value:
#rrdtool graph graf.png -s '14:00 31.01.2013' -e 'start+1h' --step 60 -w 600 -u 50000000 DEF:ds001=rrd/ctpcounters.rrd:l2orbit:AVERAGE LINE1:ds001#003300:xbcs
# rigid (cutting off too high values)
#rrdtool graph graf.png -s '14:00 31.01.2013' -e 'start+1h' --step 60 -w 600 -r -u 5000000 DEF:ds001=rrd/ctpcounters.rrd:l2orbit:AVERAGE LINE1:ds001#003300:xbcs
#
rrdtool - <<-EOF
graph graf.png -s '14:00 31.01.2013' -e 'start+1h' --step 60 \
-w 600 -r -u 8000000 \
DEF:ds001=rrd/ctpcounters.rrd:l2orbit:AVERAGE \
LINE1:ds001#003300:l2orbit \
COMMENT:maximum\: \
VDEF:maxhod=ds001,MAXIMUM \
VDEF:avrhod=ds001,AVERAGE \
GPRINT:maxhod:%.le \
COMMENT:average\: \
GPRINT:avrhod:%.le
EOF
