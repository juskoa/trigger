#!/usr/bin/python
import os, sys, os.path, random, string, time

ctpnames="cnames.sorted2"    # in $VMECFDIR/dimcdistrib/
ltunames="ltunames.sorted2"
monnames="monnames.txt"
#RRDDB= "rrd/ctpcounters.rrd"
RRDDB= "rrd/mon.rrd"
time0= 1243119600   #1163718000
LTUS=('SPD', 'SDD', 'SSD', 'TPC', 'TRD', 'TOF', 'PHOS',
  'CPV', 'HMPID', 'MUON_TRK', 'MUON_TRG', 'PMD',
  'FMD', 'T0', 'V0', 'ZDC', 'ACORDE', 'EMCAL', 'DAQ')
#LTUS=('HMPID','MUON_TRK')

def readcnames(ri, ctpltu):
  #name="temp"; gaucou="COUNTER"; ri.write("DS:%s:%s:120:U:U "%(name,gaucou))
  #name="cnt"; gaucou="COUNTER"; ri.write("DS:%s:%s:120:U:U "%(name,gaucou))
  cnames= os.path.join(os.environ['VMECFDIR'], 'dimcdistrib', ltunames)
  if ctpltu=="ctp":
    cnames= os.path.join(os.environ['VMECFDIR'], 'dimcdistrib', ctpnames)
  if ctpltu=="mon":
    cnames= os.path.join(os.environ['VMECFDIR'], 'dimcdistrib', monnames)
  cnfi= open(cnames,"r")
  if not cnfi:
    print "Where is %s ?"%cnames
    return
  nofds=0
  while (1):
    linestr= cnfi.readline()
    if not linestr: break
    line= string.split(linestr)
    if len(line) >3:
      gaucou=line[3]
    else:
      gaucou='C'
    if gaucou=='C': gaucou= "COUNTER"
    elif gaucou=='G': gaucou= "GAUGE"
    elif gaucou=='T': gaucou= "COUNTER"
    elif gaucou=='S': gaucou="COUNTER"    #spare
    else:
      print " C, G, S(taken as counter) or T expected as 4th par. in ",cnames
      #print "line:",linestr
      break
    if len(line)<1:
      print 'Empty line, eof'
      break
    name= line[0]
    if string.find(name, "volts")!= -1:   #4 voltages packed in 1 word (only CTP)
      #print "* ",name,gaucou
      for ixstr in ("_1","_2","_3","_4"):
        #print "  ",name+ixstr,gaucou
        ri.write("DS:%s:%s:120:U:U "%(name+ixstr,gaucou))
        nofds= nofds+1
    else:
      #print name,gaucou
      ri.write("DS:%s:%s:120:U:U "%(name,gaucou))
      nofds= nofds+1
  cnfi.close()
  return nofds
def rrd_create(strtime0, ctpltu):
  if strtime0=='c': strtime0= str(time0)
  if strtime0=='cnow': strtime0= "now"
  if ctpltu=='ctp' or ctpltu=='mon':
    rrd_create1(strtime0, ctpltu)
  if ctpltu=='ltu':
    for ixltun in range(len(LTUS)):
      db= LTUS[ixltun]
      rrd_create1(strtime0, db)
def rrd_create1(strtime0, rrdb):
  if rrdb=='ctp':
    ctpltu='ctp'
    rrddb_text="rrd/ctpcounters.text"
    rrddb="rrd/ctpcounters.rrd"
  elif rrdb=='mon':
    ctpltu='mon'
    rrddb_text="rrd/mon.text"
    rrddb="rrd/mon.rrd"
  else:
    ctpltu='ltu'
    rrddb_text="rrd/"+rrdb+"create.text"
    rrddb="rrd/"+rrdb+"counters.rrd"
  if os.path.exists(rrddb):
    os.remove(rrddb)
  #time0=`date --date=11/17/2006 +%s`
  ri= open(rrddb_text,"w")
  ri.write("create %s --start %s --step 60 "% (rrddb,strtime0))
  nofds= readcnames(ri, ctpltu)
  if ctpltu=='mon':
    ri.write("RRA:LAST:0.5:1:10080 ")  # last 4 weeks (24*7*60)/1min
    ri.write("RRA:AVERAGE:0.5:60:8760 ")   # last year (365*60)/1hour
  else:
    #ri.write("RRA:AVERAGE:0.5:1:600 ")   # last 10 hours/1min
    #ri.write("RRA:AVERAGE:0.5:1:1440 ")   # last 24 hours/1min
    ri.write("RRA:AVERAGE:0.5:1:10080 ")   # last week/1min
    ri.write("RRA:AVERAGE:0.5:60:720 ")   # last month/1hour 24*30=720
    ri.write("RRA:AVERAGE:0.5:1440:365 ")   # last year/1day
  ri.close()
  print "RRD:",rrdb, "# of datasets:", nofds
  os.system("rrdtool - <"+rrddb_text)
def rrd_update():
  time1= time0; nval=0; maxval=300; cntval1=cntval2=cntval3=3000
  #time1=`date --date=5/24/2006 +%s`
  time1=1243119600
  print "update:%s from time:%d, %d values"%(RRDDB, time1, maxval)
  rrdin= os.popen("rrdtool -", "w")
  #rrdin= open("update.txt", "w")
  rrdin.write("update %s "%(RRDDB))
  while nval < maxval:
    time1= time1 + 60
    cntval1= cntval1 + random.randint(0,40) + 0
    cntval2= cntval2 + random.randint(0,60) + 0
    cntval3= cntval3 + random.randint(0,80) + 0
    line="%d:%d:%d:%d"%(time1, cntval1, cntval2, cntval3)
    #for ix in range(50-2):
    #  #line=line+":%d "%(cntval3)
    #  line=line+":U"
    rrdin.write(line+' ')
    #rrdin.write("%d:%d "%(time1, cntval))
    nval= nval+1
  print "update:",rrdin.close()
def rrd_graphOrig(cnames=("l2orbit","l0inp6"), finame="", Lh="1h"):
  colors=["660000","ff0000", "770000"]
  ri= open("rrd/graph.txt","w")
  #ri.write("graph graf.png --start %d -e %d --step 60 -w 600 "%
  #  (time0, time0+60*60))
  #ri.write("graph graf.png -s teatime --step 60 -w 600 ")
  #ri.write("graph graf.png -s 17:55 --step 60 -w 600 ")  # -10 hours max.
  # time: -s now-10h   -s 1:0 -e 4:0
  #ri.write("graph graf.png -s now-2d --step 60 -w 600 ")
  if finame=="": finame='graf'
  gcmd="graph "+finame+".png -s now-"+Lh+" --step 60 -w 600 "
  print gcmd
  ri.write(gcmd)
  ix=0
  while ix<len(cnames):
    cn=cnames[ix]
    ri.write("DEF:%s=%s:%s:AVERAGE "% (cn, RRDDB, cn))
    ix=ix+1
  ix=0
  while ix<len(cnames):
    cn=cnames[ix]
    ri.write("LINE1:%s#%s:%s "%(cn,colors[ix],cn))
    ix=ix+1
  ri.close()
  os.system("rrdtool - <rrd/graph.txt")
def rrd_graph(rrd, cnames=("l2orbit","l0inp6"), finame="", Lh="1h"):
  """
  rrd: ctp, mon, SPD SSD ...
  cnames: tuple of counter names
  finame: name of the gaph: finame.png
  Lh: 1h 10h
  """
  if rrd=='mon': RRDDBname='rrd/mon.rrd'
  else: RRDDBname="rrd/"+rrd+"counters.rrd"
  colors=["660000","ff0000", "770000"]
  ri= open("rrd/graph.txt","w")
  #ri.write("graph graf.png --start %d -e %d --step 60 -w 600 "%
  #  (time0, time0+60*60))
  #ri.write("graph graf.png -s teatime --step 60 -w 600 ")
  #ri.write("graph graf.png -s 17:55 --step 60 -w 600 ")  # -10 hours max.
  # time: -s now-10h   -s 1:0 -e 4:0
  #ri.write("graph graf.png -s now-2d --step 60 -w 600 ")
  if finame=="": finame='graf'
  #gcmd="graph "+finame+".png -s now-"+Lh+" --step 60 -w 600 "
  gcmd="graph "+finame+".png -s %d --step 60 -w 600 "%(time0)
  print gcmd
  ri.write(gcmd)
  ix=0
  while ix<len(cnames):
    cn=cnames[ix]
    ri.write("DEF:%s=%s:%s:AVERAGE "% (cn, RRDDBname, cn))
    ix=ix+1
  ix=0
  #ri.write("CDEF:dt=%s,%s,/ "% (cnames[0], cnames[1]))
  while ix<len(cnames):
    cn=cnames[ix]
    ri.write("LINE1:%s#%s:%s "%(cn,colors[ix],cn))
    ix=ix+1
  #ri.write("LINE1:%s#%s:%s "%("dt",colors[0],"deadtime"))
  ri.close()
  os.system("rrdtool - <rrd/graph.txt")
def rrd_graphnew(rrd, interval, fromt, minhours, cnames):
  """
  rrd: ctp, mon, SPD SSD ...
  interval: m
  fromt: time0
  minhours: minutes or hours   (mins now)
  cnames: tuple of counter names
  
  """
  if rrd=='mon': RRDDBname='rrd/mon.rrd'
  else: RRDDBname="rrd/"+rrd+"counters.rrd"
  colors=["003300","0000cc","660000","9900cc", "ff0000", "ff00ff","000000"]
  ri= open("rrd/graph.txt","w")
  #ri.write("graph graf.png --start %d -e %d --step 60 -w 600 "%
  #  (time0, time0+60*60))
  #ri.write("graph graf.png -s teatime --step 60 -w 600 ")
  #ri.write("graph graf.png -s 17:55 --step 60 -w 600 ")  # -10 hours max.
  # time: -s now-10h   -s 1:0 -e 4:0
  #ri.write("graph graf.png -s now-2d --step 60 -w 600 ")
  finame='graf'
  #gcmd="graph "+finame+".png -s now-"+Lh+" --step 60 -w 600 "
  tot= fromt+ minhours*60
  # -r --rigid (yaxis)
  gcmd="graph "+finame+".png -s %d -e %d --step 60 -w 600 -u 10000 -l 8000 "%(fromt, tot)
  print gcmd
  ri.write(gcmd)
  ix=0
  while ix<len(cnames):
    cn=cnames[ix]
    ri.write("DEF:%s=%s:%s:AVERAGE "% (cn, RRDDBname, cn))
    ix=ix+1
  ix=0
  #ri.write("CDEF:dt=%s,%s,/ "% (cnames[0], cnames[1]))
  while ix<len(cnames):
    cn=cnames[ix]
    ri.write("LINE1:%s#%s:%s "%(cn,colors[ix],cn))
    ix=ix+1
  #ri.write("LINE1:%s#%s:%s "%("dt",colors[0],"deadtime"))
  ri.close()
  os.system("rrdtool - <rrd/graph.txt")
  
def main():
  if len(sys.argv)>1: p1=sys.argv[1];
  else: p1=''
  if p1=="c" or p1=="cnow":
    rrd_create(p1, ctpltu=sys.argv[2])     # ctp or mon
  elif p1=="u":
    rrd_update()
  elif p1=="g":     # g mon h/m fromt minhours n1 n2...
    rrddb='mon'
    interval='m'
    fromt=time0
    minhours=600
    sigs=['ds001', 'ds002', 'ds003']
    if len(sys.argv)>2: 
      rrddb= sys.argv[2]
    if len(sys.argv)>3: 
      interval= sys.argv[3]
    if len(sys.argv)>4: 
      fromt= int(sys.argv[4])
    if len(sys.argv)>5: 
      minhours= int(sys.argv[5])
    rrd_graphnew(rrddb, interval, fromt, minhours, sigs)
  elif p1=="g1h" or p1=="g10h":
    interval=p1[1:]
    if len(sys.argv)>2: 
      sigs=[]
      for ix in range(2, len(sys.argv)):
        print ix,sys.argv[ix]
        sigs.append(sys.argv[ix])
      rrd_graph("mon", sigs,Lh=interval)
    else:
      rrd_graph("mon", sigs,Lh=interval)
  elif p1=="gtime":
    im=0
    while 1:
      tim= time.localtime()
      tstr="%2.2d_%2.2d"%(tim[3], tim[4])
      rrd_graph("ctp", ["l0inp5"], "m"+tstr)
      rrd_graph("ctp", ["l0inp6"], "s"+tstr)
      im=im+1
      if (im%10)==0:
        rrd_graph("ctp", ["l0inp5"], "m10h_"+tstr, Lh='10h')
        rrd_graph("ctp", ["l0inp6"], "s10h_"+tstr, Lh='10h')
      time.sleep(3600)
  else:
    print """rrd.py c | u | g        -for testing 
cnow XXX   or     (c:...2006    cnow:now)
c    XXX -create new RRD databases in rrd subdirectory
     XXX is: ctp, mon or ltu
     Files rrd/XXXcreate.txt -what was used as input to rrdtool...

or (for using with linux/readctpc):
rrd.py g1h l0byclst1 l0inp6         ->graf.png (last 1h)
rrd.py g10h l0byclst1 l0inp6        ->graf.png (last 10h)
rrd.py gtime                -special for l0inp5/6
"""   

if __name__ == "__main__":
    main()
