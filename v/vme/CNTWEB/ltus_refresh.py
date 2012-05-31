#!/usr/bin/python
# note: http://pcalicebhm05.cern.ch/CNTWEB/ltus/     to initialise module
import cgi,string, os, sys, cntcom

sys.path.insert(0,cntcom.VMEBDIR)
import trigdb

CNTfile="/var/www/html/CNTWEB/cfgdir/ltunames.sorted2"
RRDDB= os.path.join(trigdb.VMEWORKDIR, "../../CNTRRD/rrd/")
LTUS=('SPD', 'SDD', 'SSD', 'TPC', 'TRD', 'TOF', 'PHOS',
  'CPV', 'HMPID', 'MUON_TRK', 'MUON_TRG', 'PMD',
  'FMD', 'T0', 'V0', 'ZDC', 'ACORDE', 'EMCAL', 'DAQ')

cfg=None

class Counter:
  def __init__(self, name, cgt='C', displayname=None):
    self.coname=name
    self.displayname=name
    self.selected=""   # 'y': selected
  def makeImage(self, ltuname):
    """period: month week day '10 hours' hour
    ltuname: one of names in LTUS
    """
    color="660000"
    RRDDBname= RRDDB+ltuname+"counters.rrd"
    finame= cntcom.BASEDIR+"imgs/"+ltuname+self.displayname+'.png'
    finame2= cntcom.IMAGES+ltuname+self.displayname+'.png'
    rrdinput=cntcom.BASEDIR+"imgs/graph.txt"
    #ri.write("graph graf.png --start %d -e %d --step 60 -w 600 "%
    #  (time0, time0+60*60))
    #ri.write("graph graf.png -s teatime --step 60 -w 600 ")
    #ri.write("graph graf.png -s 17:55 --step 60 -w 600 ")  # -10 hours max.
    # time: -s now-10h   -s 1:0 -e 4:0
    #ri.write("graph graf.png -s now-10h --step 60 -w 600 ")
    fromto,pixwidth= cntcom.getStartEnd(cfg)
    pixwidth="1175"
    ri= open(rrdinput,"w")
    ri.write("graph "+finame+" "+fromto+" --step 60 -w "+pixwidth+" ")
    cn= self.displayname
    if cn=="dead-busy_ts":
      ri.write("DEF:%s=%s:%s:AVERAGE "% ("busy", RRDDBname, "busy"))
      ri.write("DEF:%s=%s:%s:AVERAGE "% ("busy_ts", RRDDBname, "busy_ts"))
      # simplified version:
      #dead-busy_ts= busy*0.4/(busy_ts+1)
      #ri.write("CDEF:%s=busy,0.4,*,busy_ts,1,+,/ "%(cn))
      # right way:
      #dead-busy_ts= busy*0.4/1           if busy_ts==0
      #dead-busy_ts= busy*0.4/busy_ts     if busy_ts!=0
      ri.write("CDEF:%s=busy,0.4,*,busy_ts,0,EQ,1,busy_ts,IF,/ "%(cn))
    elif cn=="dead-l2a":
      ri.write("DEF:%s=%s:%s:AVERAGE "% ("busy", RRDDBname, "busy"))
      ri.write("DEF:%s=%s:%s:AVERAGE "% ("l2a_strobe", RRDDBname, "l2a_strobe"))
      #dead-busy_l2a= busy*0.4/1           if l2a==0
      #dead-busy_l2a= busy*0.4/l2a         if l2a!=0
      ri.write("CDEF:%s=busy,0.4,*,l2a_strobe,0,EQ,1,l2a_strobe,IF,/ "%(cn))
    elif cn=="busyOverTime":
      ri.write("DEF:%s=%s:%s:AVERAGE "% ("busy", RRDDBname, "busy"))
      ri.write("DEF:%s=%s:%s:AVERAGE "% ("time", RRDDBname, "time"))
      ri.write("CDEF:%s=busy,time,/ "%(cn))
    else:
      ri.write("DEF:%s=%s:%s:AVERAGE "% (cn, RRDDBname, cn))
    ri.write("LINE2:%s#%s:%s "%(cn,color,ltuname+'_'+cn))
    ri.close()
    # following allowed only without SElinux:
    #os.system("rrdtool - <"+rrdinput)
    pf= os.popen("rrdtool - <"+rrdinput); cmdout=pf.read(); pf.close()
    coa=string.split(cmdout)
    if (len(coa) >2) and (coa[1]=='OK'):
      return "<BR><IMG SRC=%s>\n"%(finame2)
    else:
      return "%s<BR><BR>\n"%(cmdout)
class Board:
  def __init__(self,name, color="#ffffff"):
    self.name=name
    self.color=color
    self.counters=[]   #in the ascending order of rel. address of the counter
  def addCounter(self, lines):
    """lines: cntname rel._position boardname cgt
    """
    cname= lines[0]
    self.counters.append(Counter(cname, lines[3]))
  def select(self, lcnts):
    """ lcnts: list of counters (displayed names) to be selected
    return: None (OK) or
            err message
    """
    #return str(lcnts) + "<BR>"
    for counter in self.counters:
      counter.selected=''
      lenlcnts= len(lcnts)
      if lenlcnts==0: continue
      for ix in range(lenlcnts):
        if counter.displayname==lcnts[ix]:
          counter.selected='y'
          del lcnts[ix]
          break
    if len(lcnts)>0:
      return "Error: not selected:"+str(lcnts)
    return None
  def makeTD(self):
    helpname=self.name
    #helpname="../../../htmls/busyL0.html"
    helpname=helpname+"_counters.htm"
    #<TD align=center CLASS=%s>
    #%(self.name,cntcom.BASEHELPS,helpname, self.name.upper(), self.name)
    tdelem="""
<TD align=center>
<a href="%s%s">%s</a> <BR>
<SELECT NAME="%s" SIZE="10" MULTIPLE=True>
"""%(cntcom.BASEHELPS,helpname, self.name.upper(), self.name)
    for ix in range(len(self.counters)):
      cnt= self.counters[ix]
      selected= cnt.selected
      if selected != "": selected=" SELECTED"
      tdelem= tdelem + "<OPTION %s>%s\n"%(selected,cnt.displayname)
    tdelem= tdelem + "</SELECT>\n"
    return tdelem
class Config:
  def __init__(self):
    self.period= "default"
    self.customperiod= ""; self.startgraph= ""
    self.errs=""
    self.cfginit="just initialised"
    self.allboards= {
      "ltu":Board("ltu","#cccccc"),
      "allltus":Board("allltus","#cccccc") }
     # read in counter definiions:
    f= open(CNTfile,"r")
    if not f:
      #return(os.getcwd()+ ' ' + string.join(os.listdir('.')))
      self.errs="cannot open file"
      return
    alllines= f.readlines(); f.close()
    for line in alllines:
      if line[0]=='#': continue
      lines= line.split()
      if len(lines)<4:   # name position board CGT 
        self.errs= "error in ltunames.sorted2 file, line:%s:",line
        return
      #if self.allboards.has_key(lines[2]):
      self.allboards["ltu"].addCounter(lines)
    self.allboards["ltu"].addCounter(["dead-busy_ts",'bla','bla','N'])
    self.allboards["ltu"].addCounter(["dead-l2a",'bla','bla','N'])
    self.allboards["ltu"].addCounter(["busyOverTime",'bla','bla','N'])
    for ltu in LTUS:
      self.allboards["allltus"].addCounter([ltu,'bla','bla', 'N'])
    #self.allboards['l0'].counters[0].selected='y'    #for testing
    # cookies

def deselectAll():
  global cfg
  for ixcnt in range(len(cfg.allboards["ltu"].counters)):
    cnt= cfg.allboards["ltu"].counters[ixcnt]
    if cnt.selected:
      cnt.selected=''

def index():
  global cfg
  if cfg==None:
    cfg= Config()
  else:
    deselectAll()
    cfg.cfginit="index, alredy initialised"
  s= cfg.errs
  if s=="": s= _makehtml()
  return s

# Receive the Request object
def show(req):
   global cfg
   if cfg==None:
     cfg= Config()
   else:
     cfg.cfginit="show, alredy initialised"
   errmsg= cfg.errs
   if errmsg!="": return errmsg
   # The getfirst() method returns the value of the first field with the
   # name passed as the method argument
   #word = req.form.getfirst('word', '')
   deselectAll()
   for bs in req.form.keys():
     if bs=='period':
       cfg.period= req.form['period']
       continue
     if bs=='customperiod':
       cfg.customperiod= req.form['customperiod']
       continue
     if bs=='startgraph':
       cfg.startgraph= req.form['startgraph']
       continue
     #if bs=='deselect':
     #  deselectAll()
     #  break
     if bs=='ltu':
       cnts = req.form.getlist(bs)   # selected counters ??? (deep copy???)
       errmsg=cfg.allboards["ltu"].select(cnts)
     if errmsg: return errmsg
     if bs=='allltus':
       cnts = req.form.getlist(bs)
       errmsg=cfg.allboards["allltus"].select(cnts)
     if errmsg: return errmsg
   # Escape the user input to avoid script injection attacks
   #word = cgi.escape(word)
   return _makehtml()
   s = """
<html><body>
<p>The submitted word was "%s"</p>
<p><a href="./">again!</a></p>
</body></html>
"""
   return s % word

def _makeGraphs(cnames=("cnt","temp"), finame="graf.png"):
  """not used now (should be if more counters in 1 graph...)
  cnames: names in RRDDB (not displayname)
  """
  colors=["660000","ff0000", "770000"]
  if len(cnames)==1: finame= cnames[0]+'.png'
  ri= open("graph.txt","w")
  #ri.write("graph graf.png --start %d -e %d --step 60 -w 600 "%
  #  (time0, time0+60*60))
  #ri.write("graph graf.png -s teatime --step 60 -w 600 ")
  #ri.write("graph graf.png -s 17:55 --step 60 -w 600 ")  # -10 hours max.
  # time: -s now-10h   -s 1:0 -e 4:0
  #ri.write("graph graf.png -s now-10h --step 60 -w 600 ")
  ri.write("graph "+finame+" -s now-2d --step 60 -w 600 ")
  ix=0
  while ix<len(cnames):
    cn=cnames[ix]
    while ixltu<len(LTUS):
      ri.write("DEF:%s=%s:%s:AVERAGE "% (cn, RRDDB_ctpORltu, cn))
    ix=ix+1
  ix=0
  while ix<len(cnames):
    cn=cnames[ix]
    ri.write("LINE1:%s#%s:%s "%(cn,colors[ix],cn))
    ix=ix+1
  ri.close()
  os.system("rrdtool - <graph.txt")

def _makehtml():
  global cfg
  html="""
<HTML>
<HEAD>
<meta HTTP-EQUIV="Refresh" CONTENT=60>
<TITLE>LTU counters</TITLE> 
<!-- <link type="text/css" rel="stylesheet" href="trigsim.css"> -->
<style type="text/css">
  body { color: black; background: white; }
  textarea { background: rgb(204,204,255); font-size:80%;} 
  input { background: rgb(204,204,255); } 
  TD.busy { background: #ccff00; } 
  TD.l0   { background: #cc9933; } 
  TD.l1   { background: #cc33cc; } 
  TD.l2   { background: #ccff99; } 
  TD.int  { background: #ccffff; } 
  TD.ltu  { background: #ccff00; } 
  TD.fo1,TD.fo2,TD.fo3,TD.fo4,TD.fo5,TD.fo6 { background: #cccccc; } 
  div.color {
    background: rgb(204,204,255);
    padding: 0.5em;
    border: none;
  }
</style>

<SCRIPT LANGUAGE="JavaScript">
var Result=""
function testSelect(select) {
  //Item = select.selectedIndex;
  for(Item=0; Item<select.length; Item++) {
    if(select.options[Item].selected) {
      Result = Result + select.options[Item].text + " ";
    }
  };
  //alert ("Index:"+select.selectedIndex+" Value:"+Result);
  //document.writeln("<HTML><HEAD></HEAD><BODY><h1>docwrit</h1></BODY></HTML>")
  alert (" Selected:"+Result);
}
function testButton (what){
    alert ("what:"+what+" Result:"+Result);
}
</SCRIPT>
</HEAD>
<BODY>
<center><H2> LTU counters </H2></center>
<FORM NAME="testform" METHOD="GET" ACTION="show">
<TABLE>
<TR>
"""
  #nebavi:
  #<FORM NAME="testform" METHOD="GET" ACTION="cnt/show">
  #<FORM NAME="testform" METHOD="GET" ACTION="http://pcalicebhm05.cern.ch/CNTWEB/show">
  for brd in ["allltus", "ltu"]:
  #for brd in ("l0",):
    print "board:",brd
    html= html + cfg.allboards[brd].makeTD()
  # brief help:
  html= html + """
<TD>
<ul>
<li> <b>time: </b> 1 second interval in 0.4 micsecs units (2.5M) </li>
<li> <b>busy: </b> the average time (in 0.4micsecs units), when LTU was busy during 1 second</li>
<li><b>busy_ts: </b> the average number of busy transitions per second</li>
<li><b>l2a_strobe: </b> the average number of accepted triggers per second</li>
<li><b>temp: </b> the board temperature</li>
<li><b>dead-busy_ts: </b>the average deadtime calculated as: busy*0.4/busy_ts  </li>
<li><b>dead-l2a: </b>the average deadtime calculated as: busy*0.4/l2a_strobe  </li>
<li><b>busyOverTime: </b>the fraction of the busy time caculated as: busy/time  </li>
</ul>
"""
  #html= html + "<TR>"
  html= html + cntcom.userrequest(cfg)
  for ixcnt in range(len(cfg.allboards["ltu"].counters)):
    cnt= cfg.allboards["ltu"].counters[ixcnt]
    if cnt.selected:
      #html=html+' '+cnt.coname
      for ixltu in range(len(cfg.allboards["allltus"].counters)):
        ltu= cfg.allboards["allltus"].counters[ixltu]
        if ltu.selected:
          #html=html+': '+ltu.coname
          html= html + cnt.makeImage(ltu.coname)
  html= html +"<BR>\n"
  #pf= os.popen("printenv"); cmdout=pf.read(); pf.close()
  #html= html +cmdout +"<BR>\n"
  #html= html +"<img src=graf.png>\n"
  #pf= os.popen("ls"); cmdout=pf.read(); pf.close()
  #html= html +cmdout +"<BR>\n"
  html= html +"<HR> dbg:" + cfg.cfginit + ':' + cfg.period + ':'
  html= html + """<BR>Warning: I (Renato Borges) have changed this page in 9/5/2009 to self-refresh every minute. If this causes problems, it can be reverted by running the following commands on a terminal:<BR>
  sshs<BR>
  mv /data/ClientCommonRootFs/usr/local/trigger/v/vme/CNTWEB/ltus.py.old /data/ClientCommonRootFs/usr/local/trigger/v/vme/CNTWEB/ltus.py<BR>
  </BODY>
</HTML>
"""
  return html

if __name__ == "__main__":
  print index()
  #main()

