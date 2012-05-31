#!/usr/bin/python
# note: http://pcalicebhm05.cern.ch/CNTWEB/cnt/     to initialise module
# 14.2. i2c counters added (23 more)
import cgi,string, os,sys, cntcom

#os.environ['VMECFDIR']="/data/ClientCommonRootFs/usr/local/trigger/vd/vme/"
sys.path.insert(0,cntcom.VMEBDIR)
import trigdb

CNTfile="/var/www/html/CNTWEB/cfgdir/monnames.txt"
RRDDB= os.path.join(trigdb.VMEWORKDIR, "../../CNTRRD/rrd/mon.rrd")
groups=("ttcmi",) #,"daemons","others")

cfg=None

def findInputs(boardname):
  title="%s not implemented yet"%boardname
  return title

class Counter:
  def __init__(self, dsname, cgt='C', displayname='N'):
    self.coname=dsname
    if displayname=='N':
      self.displayname= dsname
    else:
      self.displayname=displayname
    self.selected=""   # 'y': selected
  def makeImage(self):
    color="660000"
    finame= cntcom.BASEDIR+"imgs/"+self.displayname+'.png'
    finame2= cntcom.IMAGES+self.displayname+'.png'
    rrdinput=cntcom.BASEDIR+"imgs/graph.txt"
    fromto,pixwidth= cntcom.getStartEnd(cfg)
    ri= open(rrdinput,"w")
    #ri.write("graph graf.png --start %d -e %d --step 60 -w 600 "%
    #  (time0, time0+60*60))
    #ri.write("graph graf.png -s teatime --step 60 -w 600 ")
    #ri.write("graph graf.png -s 17:55 --step 60 -w 600 ")  # -10 hours max.
    # time: -s now-10h   -s 1:0 -e 4:0
    #ri.write("graph graf.png -s now-10h --step 60 -w 600 ")
    ri.write("graph "+finame+" "+fromto+" --step 60 -w "+pixwidth+" ")
    cn= self.displayname
    ri.write("DEF:%s=%s:%s:AVERAGE "% (cn, RRDDB, self.coname))
    ri.write("LINE2:%s#%s:%s "%(cn,color,cn))
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
    """lines: name_from_clients not_used name_on_graph cgt

    """
    dsname= lines[0]
    disname= lines[2]
    if len(lines)>4: helptxt= lines[4]
    if disname!='any':
      self.counters.append(Counter(dsname, lines[3], disname))
  def select(self, lcnts):
    """ lcnts: list of counters to be selected
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
    helpsuf='_help.htm'
    currentInputs= findInputs(self.name)
    tdelem="""
<TD align=center CLASS=%s>
<a href="%s%s%s">%s</a> <BR>
<SELECT NAME="%s" SIZE="10" MULTIPLE=True TITLE="%s">
"""%(self.name,cntcom.BASEHELPS,helpname, helpsuf, self.name.upper(), 
    self.name, currentInputs)
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
      "ttcmi":Board("ttcmi","#ccff00"),
      "daemons":Board("daemons","#cc9933"),
      "others":Board("others","#cc33cc")}
     # read in counter definiions:
    f= open(CNTfile,"r")
    if not f:
      #return(os.getcwd()+ ' ' + string.join(os.listdir('.')))
      self.errs="cannot open file"
      return
    alllines= f.readlines(); f.close()
    boardg='ttcmi'
    for line in alllines:
      if line[0]=='#': continue
      if len(line)==0: continue
      lines= line.split(' ',4)
      # for monnames.txt:ds001- position name CGT ANY_NOTE
      if len(lines)<4:   # name notused  dispn CGT
        self.errs= "error in %s file, line:%s:",CNTfile,line
        return
      if self.allboards.has_key(boardg):
        self.allboards[boardg].addCounter(lines)
    #self.allboards['l0'].counters[0].selected='y'    #for testing
    # cookies

def deselectAll():
  global cfg
  for brd in groups:
    for ixcnt in range(len(cfg.allboards[brd].counters)):
      cnt= cfg.allboards[brd].counters[ixcnt]
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
   s= cfg.errs
   if s!="": return s
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
     cnts = req.form.getlist(bs)   # selected counters ??? (deep copy???)
     s=cfg.allboards[bs].select(cnts)
     if s: return s
   # Escape the user input to avoid script injection attacks
   #word = cgi.escape(word)
   return _makehtml()
   s = """\
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
    ri.write("DEF:%s=%s:%s:AVERAGE "% (cn, RRDDB, cn))
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
<TITLE>Trigger monitoring</TITLE> 
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
<center><H2> Trigger monitoring </H2></center>
<FORM NAME="testform" METHOD="GET" ACTION="show">
<TABLE>
<TR>
"""
  for brd in groups:
  #for brd in ("l0",):
    print "board:",brd
    html= html + cfg.allboards[brd].makeTD()
  #<INPUT TYPE="submit" NAME="deselect" VALUE="deselect all">
  html= html + cntcom.userrequest(cfg)
  for brd in groups:
  #for brd in ("l0",):
    for ixcnt in range(len(cfg.allboards[brd].counters)):
      cnt= cfg.allboards[brd].counters[ixcnt]
      if cnt.selected:
        html= html + cnt.makeImage()
  html= html +"<BR>\n"
  #pf= os.popen("printenv"); cmdout=pf.read(); pf.close()
  #html= html +cmdout +"<BR>\n"
  #html= html +"<img src=graf.png>\n"
  #pf= os.popen("ls"); cmdout=pf.read(); pf.close()
  #html= html +cmdout +"<BR>\n"
  html= html +"<HR> dbg:" + cfg.cfginit
  #html= html + str(os.environ)
  html= html + """</BODY>
</HTML>
"""
  return html

if __name__ == "__main__":
  print index()
  #main()

