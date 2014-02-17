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
sigcolors=["660000", "66ff00", "6600ff", "66ffff", "ff0000","ff00ff","ffff00", "333333"]

cfg=None

class RRDgraph:
  def __init__(self):
    self.rrdinput=cntcom.BASEDIR+"imgs/graph.txt"
    self.nimg= 0
    self.ri= None
  def newgraph(self):   # open, assign name, delete old files
    self.ri= open(self.rrdinput,"w")
    fromto,pixwidth= cntcom.getStartEnd(cfg)
    namesname= "img%d"%self.nimg ; self.nimg= self.nimg+1
    finame= cntcom.BASEDIR+"imgs/"+namesname+'.png'
    finame2= cntcom.IMAGES+namesname+'.png'
    #ri.write("graph graf.png --start %d -e %d --step 60 -w 600 "%
    #  (time0, time0+60*60))
    #ri.write("graph graf.png -s teatime --step 60 -w 600 ")
    #ri.write("graph graf.png -s 17:55 --step 60 -w 600 ")  # -10 hours max.
    # time: -s now-10h   -s 1:0 -e 4:0
    #ri.write("graph graf.png -s now-10h --step 60 -w 600 ")
    self.ri.write("graph "+finame+" "+fromto+" --step 60 -w "+pixwidth+" ")
    # delete old files:
    return finame2
  def rrdtool(self):
    # following allowed only without SElinux?:
    pf= os.popen("rrdtool - <"+self.rrdinput); cmdout=pf.read(); pf.close()
    return cmdout
  def doDEF(self, RRDDBname, sign):
    """ purpose: avoid double definition of busy in case
    of simultaneous graph for calculated trends, i.e.
    dead-busy_ts/dead-l2a/busyOverTime
    """
    if not self.sigsdef.has_key(sign):
      self.ri.write("DEF:%s=%s:%s:AVERAGE "% (sign, RRDDBname, sign))
      self.sigsdef[sign]= 1
  def DefLine(self, ix_color, ltuname, cn):
    """ ri: text file, opened, where rrdtool cmd is prepared
    ix_color: 0,1,2,...
    ltuname: ltu name
    cn: signal name
    operation: add the DEF,LINE2 chunks of the command in self.ri file
    """
    RRDDBname= RRDDB+ltuname+"counters.rrd"
    #cn_ix= self.displayname+"%2.2d"%ix_color
    cn_ix= "ds%2.2d"%ix_color
    if cn=="dead-busy_ts":
      self.doDEF(RRDDBname, "busy")
      self.doDEF(RRDDBname, "busy_ts")
      # simplified version:
      #dead-busy_ts= busy*0.4/(busy_ts+1)
      #ri.write("CDEF:%s=busy,0.4,*,busy_ts,1,+,/ "%(cn))
      # right way:
      #dead-busy_ts= busy*0.4/1           if busy_ts==0
      #dead-busy_ts= busy*0.4/busy_ts     if busy_ts!=0
      self.ri.write("CDEF:%s=busy,0.4,*,busy_ts,0,EQ,1,busy_ts,IF,/ "%(cn_ix))
    elif cn=="dead-l2a":
      self.doDEF(RRDDBname, "busy")
      self.doDEF(RRDDBname, "l2a_strobe")
      #dead-busy_l2a= busy*0.4/1           if l2a==0
      #dead-busy_l2a= busy*0.4/l2a         if l2a!=0
      self.ri.write("CDEF:%s=busy,0.4,*,l2a_strobe,0,EQ,1,l2a_strobe,IF,/ "%(cn_ix))
    elif cn=="busyOverTime":
      self.doDEF(RRDDBname, "busy")
      self.doDEF(RRDDBname, "time")
      #self.ri.write("DEF:%s=%s:%s:AVERAGE "% ("time", RRDDBname, "time"))
      self.ri.write("CDEF:%s=busy,time,/ "%(cn_ix))
    else:
      self.ri.write("DEF:%s=%s:%s:AVERAGE "% (cn_ix, RRDDBname, cn))
    self.ri.write("LINE2:%s#%s:%s "%(cn_ix,sigcolors[ix_color],ltuname+'_'+cn))
    return
  def MakeImage(self, ltunames, signames):
    """ One of ltunames/signames contains only 1 item
    Operation: create one image 
    rc: html code
    """
    self.sigsdef= {}
    finame2= RRDimg.newgraph()   # open, assign name, delete old files
    ix_color= 0; cmdout= ""
    if len(ltunames)>1:
        for ltuname in ltunames:
          for signame in signames:
            self.DefLine(ix_color, ltuname, signame)
            ix_color= ix_color+1
    elif len(ltunames)==1:
      ltuname= ltunames[0]
      for signame in signames:
        self.DefLine(ix_color, ltuname, signame)
        ix_color= ix_color+1
    else:
      cmdout="<BR><BR>Choose at least 1 LTU...\n"
    self.ri.close() ; self.ri= None
    if cmdout !="": return cmdout
    cmdout= RRDimg.rrdtool()
    coa=string.split(cmdout)
    if (len(coa) >2) and (coa[1]=='OK'):
      return "<BR><IMG SRC=%s>\n"%(finame2)
    else:
      return "%s<BR><BR>\n"%(cmdout)

RRDimg= RRDgraph()

class Counter:
  def __init__(self, name, cgt='C', displayname=None):
    self.coname=name
    self.displayname=name
    self.selected=""   # 'y': selected
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
    self.grouping= "none"   # trend grouping, used only for LTUs, can be: alls,sigs,ltus,none
    self.customperiod= ""; self.startgraph= ""
    self.errs=""
    self.cfginit="just initialised"
    self.allboards= {
      "ltu":Board("ltu","#cccccc"),    # better name woud be "signals" (not "ltu")
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
  cfg.grouping="none"

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
     if bs=='grouping':
       cfg.grouping= req.form['grouping']
       continue
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

def getddm(assmbling):
  grhtm= cntcom.MyTemplate(htmstr="""
&nbsp&nbsp&nbsp Grouping:
<select name="grouping">
<option value="alls" $alls >all -> 1 graph</option>
<option value="ltus" $ltus >LTUs -> 1 graph</option>
<option value="sigs" $sigs >Signals -> 1 graph</option>
<option value="none" $none >None</option>
</select> 
""")
  selection= {"alls":"", "ltus":"", "sigs":"", "none":""}
  selection[assmbling]= "selected"
  return(grhtm.substitute(**selection))

def _makehtml():
  global cfg
  html="""
<HTML>
<HEAD>
<TITLE>LTU counters</TITLE> 
<!-- <link type="text/css" rel="stylesheet" href="trigsim.css"> -->
<style type="text/css">
  body { color: black; background: white; }
  textarea { background: rgb(204,204,255); font-size:80%;} 
  input,select { background: rgb(204,204,255); } 
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
  #nebavi dajak:
  #<FORM NAME="testform" METHOD="GET" ACTION="ltus/show">
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
<li><b>dead-busy_ts: </b>the average deadtime in ms calculated as: busy*0.4/busy_ts  </li>
<li><b>dead-l2a: </b>the average deadtime calculated as: busy*0.4/l2a_strobe  </li>
<li><b>busyOverTime: </b>the fraction of the busy time caculated as: busy/time  </li>
</ul>
"""
  #html= html + "<TR>"
  assmbling= cfg.grouping
  html= html + cntcom.userrequest(cfg, getddm(assmbling))
  if assmbling == "none":
    for ixcnt in range(len(cfg.allboards["ltu"].counters)):
      cnt= cfg.allboards["ltu"].counters[ixcnt]
      if cnt.selected:
        #html=html+' '+cnt.coname
        for ixltu in range(len(cfg.allboards["allltus"].counters)):
          ltu= cfg.allboards["allltus"].counters[ixltu]
          if ltu.selected:
            #html=html+': '+ltu.coname
            html= html + RRDimg.MakeImage([ltu.coname],[cnt.coname])
  elif assmbling == "alls":
    cntnames= []
    for ixcnt in range(len(cfg.allboards["ltu"].counters)):   #over signals
      cnt= cfg.allboards["ltu"].counters[ixcnt]
      if cnt.selected:
        #html=html+' '+cnt.coname
        # assembling all ltus into 1 signal-graph
        cntnames.append(cnt.coname)
        ltunames= []   # list of LTU names
        for ixltu in range(len(cfg.allboards["allltus"].counters)):
          ltu= cfg.allboards["allltus"].counters[ixltu]
          if ltu.selected:
            ltunames.append(ltu.coname)
    #html= html + ' ' + str(ltunames) + str(cntnames)+ "<BR>\n"
    html= html + RRDimg.MakeImage(ltunames, cntnames)
  elif assmbling == "ltus":
    for ixcnt in range(len(cfg.allboards["ltu"].counters)):   #over signals
      cnt= cfg.allboards["ltu"].counters[ixcnt]
      if cnt.selected:
        #html=html+' '+cnt.coname
        # assembling all ltus into 1 signal-graph
        conames= []   # list of LTU names
        for ixltu in range(len(cfg.allboards["allltus"].counters)):
          ltu= cfg.allboards["allltus"].counters[ixltu]
          if ltu.selected:
            conames.append(ltu.coname)
        #html= html + cnt.makeImage(conames)
        #html= html + ' ' + str(conames) + cnt.coname + "<BR>\n"
        html= html + RRDimg.MakeImage(conames, [cnt.coname])
  elif assmbling == "sigs":
    for ixltu in range(len(cfg.allboards["allltus"].counters)):   # over ltus
      ltu= cfg.allboards["allltus"].counters[ixltu]
      if ltu.selected:
        #html=html+': '+ltu.coname
        conames= []   # list of CNT names
        for ixcnt in range(len(cfg.allboards["ltu"].counters)):
          ltusig= cfg.allboards["ltu"].counters[ixcnt]
          if ltusig.selected:
            conames.append(ltusig.coname)
        #html= html + cnt.makeImage([ltu.coname])
        #html= html + ltu.coname + ' ' + str(conames) + "<BR>\n"
        html= html + RRDimg.MakeImage([ltu.coname], conames)
  #else   # 1 signal = 1 png
  html= html +"<BR>\n"
  #pf= os.popen("printenv"); cmdout=pf.read(); pf.close()
  #html= html +cmdout +"<BR>\n"
  #html= html +"<img src=graf.png>\n"
  #pf= os.popen("ls"); cmdout=pf.read(); pf.close()
  #html= html +cmdout +"<BR>\n"
  #html= html +"<HR> dbg:" + cfg.cfginit + ' period:' + cfg.period + ':'
  html= html + """</BODY>
</HTML>
"""
  return html

if __name__ == "__main__":
  print index()
  #main()

