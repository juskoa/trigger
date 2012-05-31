#!/usr/bin/python
#rrdtool fetch rrd/ltucounters.rrd AVERAGE -s-10m >fetchltu

import os, sys, os.path, random, string

cnames="../dimcdistrib/cnames.sorted2"
RRDDB= "rrd/ctpcounters.rrd"
time0= 1163718000

def fetchdata(l2orbit):
  finame= "fetch.data4"
  finame= "fetchltu"
  #l2orbit=16 #765 #839  #16
  fd= open(finame,"r")
  if not fd:
    print "Where is %s ?"%finame
    return
  else:
    print "File:",finame
  # names
  # time: values
  # time: values
  line1= fd.readline()
  names= string.split(line1)
  print "name items:", len(names), names[l2orbit]
  linen=0
  while (1):
    linestr= fd.readline()
    if not linestr: break
    if len(linestr)<2:
      print "linen, len:",linen, len(linestr)
      continue
    line= string.split(linestr)
    timestmp=line[0] ; del line[0]
    print timestmp, "items:", len(line), line[l2orbit]
    linen=linen+1
  fd.close()
  
def main():
  if len(sys.argv)>1: 
    p1=sys.argv[1];
    fetchdata(int(p1))
  else:
    print """fetch.py rel_position_from_cnames.sorted2
16   l0classB1
765  l2orbit copy as spare
839  l2orbit
"""   

if __name__ == "__main__":
    main()
