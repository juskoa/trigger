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
  sdir="/home/dl6/local/trigger/v/vme/CFG/ctp/pardefs2012"
  ddir="/home/dl6/local/trigger/v/vme/CFG/ctp/pardefs"
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
  tech_l1r_l0f3_BCmasks= ["tech_l1r_l0f3_BCmasks","technical run + L1R events + complex l0f3 used, BCmask>4 used"]
  lowrate10hz= ["10hz","just rnd1=10hz, not using any CTP input. Foreseen to run in parallel with cosmics"]
  trd_l0f34= ["trd_l0f34","test complex l0f with 0RG* inputs. WU-classes only"]
  TestFast= ["TestFast","""test the influence of FAST cluster on 
ALL/ALLNOTRD rates with different rates of rnd1. 
Should be started also without FAST cluster.
All triggers generated according to bcmB mask."""]
  TestFastNomask= ["TestFastNomask","test the influence of FAST cluster on ALL/ALLNOTRD rates with different rates of rnd1. Should be started also without FAST cluster. BCmask is not used"]
  hlttest= ["hlttest","ALLNOTRD cluster triggered by rnd1=1khz"]
  options={"PHYSICS_1":[
  ["TPCKr","TPC krypton run"],
  ["techn_12bc_4l0f","technical run, max bc , max l0f"],
  ["PFtest","test past future protection -180 +100us for events sent to tpc"],
  ["technical","technical run, minimal configuration, random trigger"],
  ["techn_BCmasks","technical run, minimal configuration, random trigger with BCmasks"],
  ["tech_l1r","technical run + L1R events triggered by random"],
  ["tech_l1r_BCmask","technical run + L1R events, BCmasks used"],
  tech_l1r_l0f3_BCmasks,
  ["cosmic0OB3","cosmic triggered by 0OB3"],
  ["cosmic0OB3_1HCO","2 clusters: NOTRD triggered by DTRDCO2 and ALL by 0OB3"],
  ["vasco_test","testting the DAQ counting for detector included in 2 clusters"],
  lowrate10hz
  ],
  "PHYSICS_2":[
  hlttest,
  ["techbig","default technical close to real"],
  ["techbigphos","default technical close to real with special phos cluster for taking calibration data"],
  tech_l1r_l0f3_BCmasks,
  trd_l0f34, TestFast, TestFastNomask,
  lowrate10hz
  ],
  "PHYSICS_3":[["cosmicMuon","cosmic run with muon only"]],
  "TEST_1":[]
  }
  selected="" ; title="" ; ll=""
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
  helptxt=" Choose 1 option and click Submit button"
  lastargs= req.__getattribute__("args")
  #kwa= act(**lastargs)
  partname= getpar(req,"partname")
  fname= getpar(req,"fname")
  if partname==None:
    # chp?partname=PHYSICS_1
    return "","partname not given (PHYSICS_1 or TEST_1 expected)"
  if partname!="PHYSICS_1" and partname!="PHYSICS_2" and \
    partname!="PHYSICS_3" and partname!="TEST_1":
    return "","Bad partname:%s (PHYSICS_1/2 or TEST_1 expected)"%partname
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
  return partname,"""
<form action="chlocpart" method="GET">
<!---
  comment
---!>
  <input type="text" name="partname" size="10" readonly value="%s"/>
  local configurations (place mouse cursor for short explanation, then click and submit):<br>
  <SELECT NAME="fname" SIZE="10" TITLE="%s">
  %s
  </SELECT>
  <input value="Submit" type="submit" TITLE="click to set choosen config as PHYSICS_1"/>
  <br><pre>%s</pre><br><br>
%s
</form>"""%(partname, helptxt, get_options(partname,fname), kwa, 
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
  pname,form= form1(req)
  htm= cntcom.HtmlEditPrint("/var/www/html/CNTWEB/chlocpart.html",
    FormChoosePart=form, pname=pname)
  return htm

if __name__ == "__main__":
  print "main:",index()
  #main()

