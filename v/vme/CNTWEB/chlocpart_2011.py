#!/usr/bin/python
#from cgi import escape
#from urllib import unquote
import popen2,shutil,cntcom
from mod_python import util

# The Publisher passes the Request object to the function
atable={'__getattribute__':1, 'status':1, 'args':1, 'bytes_sent':1, \
  'filename':1}

class iopipe:
  def __init__(self, cmd):
    #os.chdir(os.environ['VMEWORKDIR'])
    #clientcmd= os.path.join(os.getenv('VMECFDIR'),'ltudim/linux/ltuclient')+' '
    self.cmd= cmd
    #print "popen2("+cmd+")"
    self.iop= popen2.popen2(cmd, 1) #0- unbuffered, 1-line buffered
    #print "iopipe.iop:",self.iop
    self.getout()
  def getout(self):
    """
    """
    #self.iop[1].close()
    iline=0; self.outlines=[]
    while(1):
      line= self.iop[0].readline()
      #print "%s:"%self.cmd,line
      if line =='':
        break
      #print line[:-1]
      self.outlines.append(line[:-1])
      iline=iline+1
      if iline>100: 
        self.outlines.append("ERROR: too many lines")
        break
    #print self.outlines
  def cmd(self, cmd):
    out=None
    self.iop[1].write(cmd+'\n')
    #print self.nbcmd+':'+cmd
    return out
  def outtext(self):
    s=''
    for i in range(len(self.outlines)):
      s= s+self.outlines[i]+'\n'
    return s
  def close(self):
    #print "closing"
    self.iop[0].close()
    self.iop[1].close()

def copypart(partname, fname):
  #sdir= "/data/ClientCommonRootFs/usr/local/trigger/v/vme/CFG/ctp/pardefs2010"
  #ddir= "/data/ClientCommonRootFs/usr/local/trigger/v/vme/CFG/ctp/pardefs"
  sdir="/data/dl/root/usr/local/trigger/v/vme/CFG/ctp/pardefs2011"
  ddir="/data/dl/root/usr/local/trigger/v/vme/CFG/ctp/pardefs"
  src= sdir+'/'+fname+".partition"
  dest= ddir+'/'+partname+".partition"
  ot="%s -> %s\n\n"%(fname, partname)
  #shutil.copyfile(src, dest)
  #sudoers file:
  iop= iopipe("sudo -u trigger cp %s %s ; echo rc:$?"%(src, dest))
  ot= ot + iop.outtext()
  iop.close()
  iop= iopipe("ls -l %s"%dest)
  ot= ot + iop.outtext()
  iop.close()
  iop= iopipe("echo ; cat %s"%dest)
  ot= ot + iop.outtext()
  iop.close()
  return ot
def getpar(req, par):
  inpdata= util.FieldStorage(req)
  return inpdata.get(par,None)
def get_options(parname, fname):
  #options={"PHYSICS_1":["PFtest","_noV0","_noV0_no0LSR","_0HWU","BEAM_7_tripwatch",
  #  "slow_rnd_trigger","hidaqtest","CTP_LUMITEST","testMUON_nomask",
  #  "CTP_HI_MB1v4","phos_led"],
  options={"PHYSICS_1":[["PFtest","test past future protection -180 +100us for events sent to tpc"],
  ["technical","technical run, minimal configuration"],
  ["tech_l1r","technical run + L1R events"],
  ["tech_l1r_l0f3","technical run + L1R events + complex l0f3 used"],
  ["cosmic0OB3","cosmic triggered by 0OB3"],
  ["vasco_test","testting the DAQ countig for detector in 2 clusters"],
  ],
  "PHYSICS_2":[
  ["MUONS_hvScan", "MUON cluster, triggered by 0MSL in colliding bunches"]
  ["MUONS_hvScan_NOmask", ""],
  ["MUONS_hvScan_Technical", ""],
  ["SPD_efficiency_inside_4BCs", "study of PIT efficiency w.r.t. the SPD/ALICE clock shift SPD 4BCs"],
  ["notrigger", "no trigger sent (0AMU & BCMempty)"],
  ["test0PH0_B", "single class: 0PH0 masked with bcmB"],
  ["testMUON_nomask",""]
  ["TOF_OM23UPtest",""],
  ["phos_led"""],
  ],
  ["PHYSICS_3":["SPD_efficiency_inside_4BCs", "Triggered by DINT1 in B-bunches"],
  ["emcalphos",,"",
  ["emcalphos_no0SMB",,"",
  ["emcal_no0SMB",,"",
  ["phos_no0SMB","",
  ],
  "TEST_1":["_V0only","phos_led"]}
  ll="" ;
  for ix in range(len(options[parname])):
    opt= options[parname][ix]
    selected=""
    if opt[0]==fname:
      selected="selected"
    title=""
    if len(opt[1])>0:
      title='TITLE="%s" '%opt[1]
    ll= ll+"<option "+title+selected+">"+opt[0]+"\n"
  if selected!="":
    o1= "<option >not selected\n"
  else:
    o1= "<option selected>not selected\n"
  return o1+ll
def form1(req):
  helptxt={"PHYSICS_1":" Choose 1 option and click Submit button",
  "PHYSICS_2":" Choose 1 option and click Submit button",
  "PHYSICS_3":"""
""",
  "TEST_1":" Choose 1 option and click Submit button"}
  lastargs= req.__getattribute__("args")
  #kwa= act(**lastargs)
  partname= getpar(req,"partname")
  fname= getpar(req,"fname")
  if partname==None:
    # chp?partname=PHYSICS_1
    return "partname not given (PHYSICS_1 or TEST_1 expected)"
  if partname!="PHYSICS_1" and partname!="PHYSICS_2" and \
    partname!="PHYSICS_3" and partname!="TEST_1":
    return "Bad partname:%s (PHYSICS_1/2 or TEST_1 expected)"%partname
  if fname!=None:
    kwa= copypart(partname, fname)
    prepared="""
<hr>
%s configuration above is prepared for run (if <b>not active</b> in ACT!). <br>
It can be modified before the start
by starting <b>parted %s</b> from aldaqacr07
<hr>
"""%(partname, partname)
  else:
    kwa=""
    prepared=""
  return """
<form action="chlocpart" method="GET">
  <input type="text" name="partname" size="10" readonly value="%s"/>
  <SELECT NAME="fname" SIZE="10" TITLE="%s">
  %s
  </SELECT>
  <input value="Submit" type="submit" />
  <br><pre>%s</pre><br><br>
%s
</form>"""%(partname, helptxt[partname], get_options(partname,fname), kwa, 
prepared)

def showreq(req):
   s = """
<h1> req object attribs: </h1>
<table cellspacing="0" cellpadding="0">%s</table>
"""
   attribs = ''
   # Loop over the Request object attributes
   for attrib in dir(req):
     #attribs += '<tr><td>%s</td><td>%s</td></tr>'
     #attribs %= (attrib, escape(unquote(str(req.__getattribute__(attrib)))))
     #attribs %= (attrib, str(req.__getattribute__(attrib)))
     try:
       if atable.has_key(attrib):
         value= str(req.__getattribute__(attrib))
       else:
         value='-'
     except:
       #value= sys.exc_info()[0]
       value='caught'
     attribs += '<tr><td>%s</td><td>%s</td></tr>'%(attrib, value)
   #return "showreq"
   return s % (attribs)
def index2(req):
  htm= """\
<html><head>
<style type="text/css">
td {padding:0.2em 0.5em;border:1px solid black;}
table {border-collapse:collapse;}
</style>
</head><body>
"""
  htm=htm+form1(req)
  #htm=htm+ showreq(req)
  htm=htm+"""
</body></html>
"""
  return htm
  #return showreq(req)
def index(req):
  form= form1(req)
  htm= cntcom.HtmlEditPrint("/var/www/html/CNTWEB/chlocpart.html",
    FormChoosePart=form)
  return htm

if __name__ == "__main__":
  print "main:",index()
  #main()

