#!/usr/bin/python
# author: E.Kryshen
# mail: evgeny.kryshen@cern.ch
from Tkinter import *
import string,sys,os,time
from l0inputdb import SwitchInput,SwitchInputsDB,redline,PrintError,PrintFatal,PrintInfo

sys.path.append(os.environ.get('VMECFDIR')+'/TRG_DBED/')
import myw

TRGDBDIR = os.environ.get('dbctp')+'/'

fname_inputs = TRGDBDIR+'VALID.CTPINPUTS'
fname_switch = TRGDBDIR+'CTP.SWITCH'
fname_switch_inputs = TRGDBDIR+'L0.INPUTS'
fname_switch_arxiv = TRGDBDIR+'CTP.SWITCH.ARXIV/'+'CTP.SWITCH'
fname_inputs_arxiv = TRGDBDIR+'VALID.CTPINPUTS.ARXIV/'+'VALID.CTPINPUTS'

COLOR_WHITE="#ffffff"

def exssh(cmd):
  print cmd,":"
  if os.environ["VMESITE"]!="ALICE":
    print "not executed (not in the pit)"
    return 8 
  ou= os.popen(cmd)
  while 1:
    line= ou.readline()
    if line=='': break
    print line[:-1]
  rc= ou.close()
  if rc==None: rc=0
  return rc
#======================================================================
class setit:
  # class needed to update ctpnames in ctpname option menu
  def __init__(self, var, value):
    self.__value = value
    self.__var = var
  def __call__(self, *args):
    self.__var.set(self.__value)
#======================================================================


#======================================================================
class InputsDB:
  def __init__(self,fname):
    self.inputs = []
    self.comments = []
    # Open file with trigger inputs  
    try:
      self.infile = open(fname,'r')
      if self.infile: 
        print 'File with trigger inputs opened:',fname
        self.read()
        self.infile.close()
    except IOError:
       PrintError('I/O Error during loading file with trigger inputs:'+fname)
       sys.exit(0)
  #--------------------------------------------------------------------
  def read(self):
    # Read trigger inputs from the file
    #print 'Read trigger inputs...'
    while 1:
      line = self.infile.readline()
      if not line: break
      if line[0]=='#':
        if line[0:7]!='#InName': self.comments.append(line)
        continue
      inp = CtpInput(line)
      self.inputs.append(inp)
  #--------------------------------------------------------------------
  def write(self,stream):
    header=0
    for inp in self.inputs: 
      if header==0:
	inp.print_header(stream)
	header=1
      inp.print_input(stream)
  #--------------------------------------------------------------------
  def write_to_file(self,fname):
    new_file = open(fname,'w')
    for comment in self.comments:
      new_file.write(comment)
    self.write(new_file)
    new_file.close()
#======================================================================


#======================================================================
class CtpInput:
  def __init__(self,line,name=None,detectorname='',level=None,signature=None,inpnum=None,dimnum=None,configured=None,edge=None,delay=None,deltamin=None,deltamax=None):
    if line=='':
      self.name          = name
      self.detectorname  = detectorname
      self.level         = level
      self.signature     = signature
      self.inpnum        = inpnum
      self.dimnum        = dimnum
      self.configured    = configured
      self.edge          = edge
      self.delay         = delay
      self.deltamin      = deltamin
      self.deltamax      = deltamax
      self.l0fdefinition = None
      return
    self.line          = line
    self.name          = None
    self.detectorname  = None
    self.level         = None
    self.signature     = None
    self.inpnum        = None
    self.dimnum        = None
    self.configured    = None
    self.l0fdefinition = None
    self.edge          = 0
    self.delay         = 0
    self.deltamin      = 1000
    self.deltamax      = 1000
    # parse line (extract input name)
    self.line = string.expandtabs(self.line)
    self.line = string.strip(self.line,'\n')
    ninps = string.split(self.line,'=')
    if len(ninps)!=2:
      PrintError("CTP input definition (or detector it belongs to) missing for "+line)
      return
    self.name = string.strip(ninps[0])
    if self.name[:3]=='l0f' or self.name[:3]=='l0A' or self.name[:3]=='l0B':
      self.l0fdefinition = string.strip(ninps[1])
      return
    # parse the remaining line (input description)
    li = string.split(ninps[1],' ')
    for ix in range(len(li)-1, -1, -1):
      if li[ix]=='': del li[ix]
    
    self.detectorname = li[0]
    if len(li) == 1:
      self.configured = 0
    elif len(li) >= 9:
      self.level      = int(li[1])
      self.signature  = int(li[2])
      self.inpnum     = int(li[3])
      self.dimnum     = int(li[4])
      self.configured = int(li[5])
#    elif len(li) >= 9:
      self.edge       = int(li[6])
      self.delay      = int(li[7])
      self.deltamin   = int(li[8])
      self.deltamax   = int(li[9])
    else:
      PrintError("Bad CTP input definition:"+line)
  #--------------------------------------------------------------------
  def print_input(self,stream):
    if self.name==None: return
    stream.write(string.ljust(self.name,15)+' = ')
    if self.l0fdefinition==None:
      stream.write(string.ljust(self.detectorname,15))
      stream.write(string.rjust(str(self.level),10))
      stream.write(string.rjust(str(self.signature),10))
      stream.write(string.rjust(str(self.inpnum),10))
      stream.write(string.rjust(str(self.dimnum),10))
      stream.write(string.rjust(str(self.configured),12))
      stream.write(string.rjust(str(self.edge),10))
      stream.write(string.rjust(str(self.delay),10))
      stream.write(string.rjust(str(self.deltamin),10))
      stream.write(string.rjust(str(self.deltamax),10))
      stream.write('\n')
    else:   
      stream.write(self.l0fdefinition+'\n')
  #--------------------------------------------------------------------
  def print_header(self,stream):
    stream.write('#')
    stream.write(string.ljust('InName',15)+'= ')
    stream.write(string.ljust('Det',15))
    stream.write(string.rjust('Level',10))
    stream.write(string.rjust('Signature',12))
    stream.write(string.rjust('Inpnum',10))
    stream.write(string.rjust('Dimnum',10))
    stream.write(string.rjust('Configured',12))
    stream.write(string.rjust('Edge',10))
    stream.write(string.rjust('Delay',10))
    stream.write(string.rjust('DeltaMin',10))
    stream.write(string.rjust('DeltaMax',10))
    stream.write('\n')
#======================================================================



#======================================================================
class Switch:
  def __init__(self,line,nameweb=None,namectp=None,eq=None,sin=None,sout=None,ctpin=None):
    if line=='':
      self.nameweb=nameweb
      self.namectp=namectp
      self.eq=eq
      self.sin=sin
      self.sout=sout
      self.ctpin=ctpin
      self.l0inputId=-1
      return;
    self.line = string.expandtabs(line)
    self.line = string.strip(self.line,'\n')
    li = string.split(self.line,' ')
    for ix in range(len(li)-1, -1, -1):
      if li[ix]=='': del li[ix]
    if len(li)>6: PrintError('Failed to parse switch configuration line:'+line)
    self.nameweb       = li[0]
    self.namectp       = li[1]
    self.eq            = int(li[2])
    self.sin           = int(li[3])
    self.sout          = int(li[4])
    if len(li)>5:
      self.ctpin       = int(li[5])
    else:
      self.ctpin       = self.sout-1
    self.l0inputId=-1
  #--------------------------------------------------------------------
  def print_switch(self,stream):
    stream.write(self.nameweb+' ')
    stream.write(self.namectp+' ')
    stream.write(str(self.eq)+' ')
    stream.write(str(self.sin)+' ')
    stream.write(str(self.sout))
    if self.ctpin==self.sout-1:
      stream.write('\n')
    else:
      stream.write(' '+str(self.ctpin)+'\n')
#======================================================================


#======================================================================
class SwitchDB:
  def __init__(self,fname):
    self.switch = []
    self.comments=[]

    # Open file with CTP switch configuration 
    try:
      self.infile = open(fname,'r')
      if self.infile: 
        print 'File %s opened:'%fname
        self.read()
        self.infile.close()
    except IOError:
       PrintError('I/O Error during loading CTP switch configuration:'+fname)
       sys.exit(0)
  #--------------------------------------------------------------------
  def read(self):
    # Read trigger inputs from the file
    #print 'Read CTP switch configuration...'
    while 1:
      line = redline(self.infile)
      if not line: break
      if line[0]=='#': 
        self.comments.append(line)
	continue
      switch = Switch(line)
      self.switch.append(switch)
#      print line
  #--------------------------------------------------------------------
  def write(self,stream):
    for switch in self.switch: 
      switch.print_switch(stream)
  #--------------------------------------------------------------------
  def write_to_file(self,fname):
    new_file = open(fname,'w')
    for comment in self.comments:
      new_file.write(comment)
    self.write(new_file)
    new_file.close()
#======================================================================


#======================================================================
class SwitchOptionMenu:
  def __init__(self,master,i,detnames,ctpnames):
    self.master = master
    self.detnames = detnames
    self.ctpnames = ctpnames
    self.itemlist = ['none']
    self.previous_detname = ''
    self.var_det = Variable()
    self.var_ctpname = Variable()
    # create label
    self.label=Label(master,text=str(i+1)).grid(row=i+1,column=0,sticky=W+E)
    
    self.om_det = OptionMenu(self.master,self.var_det,command=self.updateInputsList,*self.detnames)
    self.om_det.grid(row=i+1,column=1,sticky=W+E)
    self.om_ctpname = OptionMenu(self.master,self.var_ctpname,*self.itemlist)
    self.om_ctpname.grid(row=i+1,column=2,sticky=W+E)
  #--------------------------------------------------------------------
  def setDefault(self,detname,inpname):
    self.var_det.set(detname)
    self.var_ctpname.set(inpname)
    self.previous_detname = detname
  #--------------------------------------------------------------------
  def updateInputsList(self,choice):
    detname = self.var_det.get()
    if detname==self.previous_detname and choice!=-1: return
    if choice!=-1: print 'detector name changed from', self.previous_detname, 'to',detname
    self.previous_detname = detname
    if detname=='none':
      self.itemlist=['none']
    # check detector name
    try:
      det_index = self.detnames.index(detname)
    except ValueError:
      print 'Detector name "'+detname+'" not found in a valid detector list'
      det_index = -1
    if det_index<0:
      self.itemlist = ['none']
    else:
      self.itemlist = self.ctpnames[det_index]
    self.updateItemList()
    if choice!=-1: self.var_ctpname.set('none')
  #--------------------------------------------------------------------
  def updateItemList(self):  
    self.om_ctpname["menu"].delete(0, END)
    for item in self.itemlist:
      self.om_ctpname["menu"].add_command(label=item,command=setit(self.var_ctpname, item))

#======================================================================


#======================================================================
class Switched:
  def __init__(self, master,switchInputsDB,switchDB,inputsDB):
    self.switchDB       = switchDB
    self.switch         = switchDB.switch
    self.inputsDB       = inputsDB
    self.switchInputsDB = switchInputsDB
    self.switchInputs   = switchInputsDB.switchInputs
    
    self.master  = master
    if master!=None:
      # Create "File" menu
      self.menubar = myw.MywMenu(self.master)
      self.master.config(menu=self.menubar)
      self.filemenu = self.menubar.addcascade('File')
      self.filemenu.addcommand('Save AND LOAD CTP SWITCH',self._save)
      self.filemenu.addcommand('Save only files (CTP SWITCH not loaded)',self._savefiles)
      self.filemenu.addcommand('Cancel',self._cancel)
      self.filemenu.addcommand('Quit',self._quit)
      # Create main GUI frame
      self.mainfr  = myw.MywFrame(self.master,side=TOP,expand='yes',fill='x')
      self.mainfr.columnconfigure(0,minsize=100)
      self.mainfr.columnconfigure(1,minsize=150)
      self.mainfr.columnconfigure(2,minsize=150)
      Label(self.mainfr,text='CTP input').grid(row=0,column=0,sticky=W+E)
      Label(self.mainfr,text='Detector').grid(row=0,column=1,sticky=W+E)
      Label(self.mainfr,text='Input name').grid(row=0,column=2,sticky=W+E)
    #else:
    self.detnames = self.detectorsDB()
    self.ctpnames = self.ctpnamesDB()
    #
    self.som = []
    if master!=None:
      # Create option menus
      for i in range(0,24):
        m = SwitchOptionMenu(self.mainfr,i,self.detnames,self.ctpnames)
        self.som.append(m)
      # Fill current values for option menus
      self.setDefaultInputs()
      for i in range(0,24):
        self.som[i].updateInputsList(-1)
  #--------------------------------------------------------------------
  def detectorsDB(self):
    det = []
    det.append('none')
    for switchInput in self.switchInputs:
      if switchInput.configured==0: continue
      if switchInput.detectorname=='none': continue
      if switchInput.detectorname not in det:
        det.append(switchInput.detectorname)
    return det
  #--------------------------------------------------------------------
  def ctpnamesDB(self): 
    ctpnames = []
    i=0
    for d in self.detnames:
      ctpnames.append([])
      ctpnames[i].append('none')
      i=i+1
    for switchInput in self.switchInputs:
      if switchInput.configured==0: continue
      if switchInput.namectp=='none': continue
      ctpnames[self.detnames.index(switchInput.detectorname)].append(switchInput.namectp)
    return ctpnames
  #--------------------------------------------------------------------
  def setDefaultInputs(self): 
    # set 'none'
    for som in self.som:
      som.setDefault('none','none')
    
    for switch in self.switchDB.switch:
      if switch.l0inputId==-1: continue
      detname = self.switchInputsDB.switchInputs[switch.l0inputId].detectorname
      inpname = switch.namectp
      inpno   = switch.ctpin
      if inpno>0 and inpno<=24:
        self.som[inpno-1].setDefault(detname,inpname)
#    for switchInput in self.switchInputs:
#      detname = switchInput.detectorname
#      inpno   = switchInput.ctpin
#      inpname = switchInput.namectp
#      if inpno>0 and inpno<=24:
#        self.som[inpno-1].setDefault(detname,inpname)
  #--------------------------------------------------------------------
  def _savefiles(self, minst=None, ix=None):
    print "updating CTP.SWITCH and VALID.CTPINPUTS in dbctp directory..."
    self.updateSwitch()
    self.updateCtpInputs()
    self.switchDB.write_to_file(fname_switch)
    self.inputsDB.write_to_file(fname_inputs)
  def _save(self, minst=None, ix=None):
    print "updating CTP.SWITCH, VALID.CTPINPUTS files..."
    self.updateSwitch()
    self.updateCtpInputs()
    if minst==None:
      print "CTP.SWITCH, VALID.CTPINPUTS (and more) downloaded from act used"
    else:
      print "saving CTP.SWITCH, VALID.CTPINPUTS in their .ARXIV/"
      date_time=time.strftime(".%Y-%m-%d.%H-%M-%S",time.localtime())
      #print 'saving:',fname_switch, fname_switch_arxiv+date_time
      os.rename(fname_switch,fname_switch_arxiv+date_time)
      self.switchDB.write_to_file(fname_switch)
      #print 'saving:',fname_inputs, fname_inputs_arxiv+date_time
      os.rename(fname_inputs,fname_inputs_arxiv+date_time)
      self.inputsDB.write_to_file(fname_inputs)
      print "saved..."
    #load only CTP switch (LTU.SWITCH is not in ACT):
    rc= exssh("ssh -q -2 trigger@alidcsvme004 loadswitch ctp");
    #print "exssh loadswitch rc:", rc, type(rc)
    if minst!=None:
      print """

if changed !=0 (above) for CTP.SWITCH, RESTART ctpproxy:
          ctpproxy restart

"""
    print "load switch rc (0x0 expected):0x%x"%rc
    return rc
  #--------------------------------------------------------------------
  def _cancel(self, minst=None, ix=None):
    print 'cancel'
  #--------------------------------------------------------------------
  def _quit(self, minst, ix):
    print 'quit'
    sys.exit(0)
  #--------------------------------------------------------------------
  def updateSwitch(self):
    del self.switchDB.switch[:]
    inpno=0
    for som in self.som:
      inpno=inpno+1
      detname = som.var_det.get()
      ctpname = som.var_ctpname.get()
      if detname=='none': continue
#     print inpno,detname,ctpname
      l0inputId=0
      for si in self.switchInputs:
#	print si.sin,
	if si.detectorname==detname and si.namectp==ctpname:
          si.sout=inpno+1
          si.ctpin=inpno
	  s = Switch('',
                si.nameweb,
                si.namectp,
                si.eq,
                si.sin,
                si.sout,
                inpno)
          s.l0inputId=l0inputId
	  self.switchDB.switch.append(s)
	  break
        l0inputId=l0inputId+1
#        print ''
      
#      for si in self.switchInputs:
#        print si.sin,
#	if si.detectorname==detname and si.namectp==ctpname:
#          si.sout=inpno+1
#          si.ctpin=inpno
#	  s = Switch('',
#                si.nameweb,
#                si.namectp,
#                si.eq,
#                si.sin,
#                si.sout,
#                si.ctpin)
#          self.switchDB.switch.append(s)
#	  break
#        print ''
  #--------------------------------------------------------------------
  def updateCtpInputs(self): 
    l0functions=[]
    l1inputs=[]
    l2inputs=[]
    for inp in self.inputsDB.inputs:
      if inp.l0fdefinition!=None: 
        l0functions.append(inp)
      elif inp.level==1:
        l1inputs.append(inp)
      elif inp.level==2:
        l2inputs.append(inp)
    del self.inputsDB.inputs[:]
    for switch in self.switch:
      if switch.ctpin<=0: continue
      if switch.l0inputId<0: continue
      si = self.switchInputs[switch.l0inputId]
#      print 'switch: ',switch.l0inputId,
#      print ' ctp in: ',switch.ctpin,
#      print 'namectp: ',si.namectp,
#      print 'detectorname: ',si.detectorname
      
      ci = CtpInput('',
          si.namectp,
          si.detectorname,
          si.level,
          si.signature,
          switch.ctpin,
          si.dimnum,
          1,
	  si.edge,
	  si.delay,
	  si.deltamin,
	  si.deltamax
	  )
      self.inputsDB.inputs.append(ci)
    self.inputsDB.inputs=self.inputsDB.inputs+l1inputs+l2inputs+l0functions
#======================================================================


#======================================================================
def consistencyCheck(switchInputsDB,switchDB,inputsDB):
  rccheck=0
  print 'Consistency check...'
  # Loop over CTP.SWITCH lines and check for consistency with L0.INPUTS
  L0inputs = switchInputsDB.switchInputs
  L0inputID=-1
  for switch in switchDB.switch:
    isOK=0
    id=0
    for L0input in L0inputs:
      if switch.sin==L0input.sin: 
        isOK=isOK+1
        L0inputID=id
        if (L0input.configured==0): 
          isOK=-1
	  continue
      id=id+1;
    if   (isOK==-1): 
      PrintError('Switch input '+str(switch.sin)+' not configured')
      rccheck= 1
    elif (isOK==0) : 
      PrintError('Switch input '+str(switch.sin)+' not found in L0 input database')
      rccheck= 1
    elif (isOK>1)  : 
      PrintError('Two switch configurations have identical swith input number:'+switch.sin)
      rccheck= 1
    #elif (isOK==1) : PrintInfo('Switch input '+str(switch.sin)+' succesfully found in L0 input database')
    
    if (isOK!=1): continue
    L0input = L0inputs[L0inputID]
    if switch.namectp!=L0input.namectp:
      PrintError('Ctpname ('+switch.namectp
        +') for switch input '+str(switch.sin)
	+' does not match ctpname in L0 input database ('
	+ L0input.namectp +')')
      rccheck= 1
      
    if switch.eq!=L0input.eq:
      PrintError('Equalized flag ('+str(switch.eq)
        +') for switch input '+str(switch.sin)
	+' does not match equalized flag in L0 input database ('
	+ str(L0input.eq) +')')
      rccheck= 1
      
    L0input.ctpin=switch.ctpin
    L0input.sout=switch.sout
    switch.l0inputId=L0inputID
#    print switch.l0inputId
  print 'Consistency check finished'
  return rccheck
#======================================================================


#======================================================================
def main(act=None):
  """
           from              actions
  act:     startClients.bash download from ACT + check + load SWITCH
  actload: ctpproxy.py       download from ACT + check
  checkonly:                 check
  """
  withgui='yes'
  if (act=='act') or ( act=='actload' ) or ( act=='checkonly' ):
    ftk=None
    ff=None
    if (act=='act') or ( act=='actload' ):
      #ftk= Tk()
      #ftk.title('CTP switch batch') ; ff=1
      withgui='no'
      rc= os.system("../ctp_proxy/linux_s/act.exe")
      print "Downloading config files from ACT. rc:0x%x"%rc
      rc= rc>>8
      if rc!=0:
        return rc
      os.system("../../bin/dos2unx.bash") 
  elif act==None:  
    ff=1
    ftk= Tk()
    ftk.title('CTP switch')
  else:
    print "Unknown parameter (only act allowed, if any)"
    return 8
  inputsDB = InputsDB(fname_inputs)
  #for input in inputsDB.inputs: input.print_input(sys.stdout)
  switchInputsDB = SwitchInputsDB(fname_switch_inputs, withgui)
  #switchInputsDB.write(sys.stdout)
  switchDB = SwitchDB(fname_switch)
  #switchDB.write(sys.stdout)
  rc= consistencyCheck(switchInputsDB,switchDB,inputsDB)
  if ( act=='actload' ) or ( act=='checkonly' ):
    return rc
  switched = Switched(ftk,switchInputsDB,switchDB,inputsDB)
  if ff:
    ftk.mainloop()
    rc= 0   # always 0 from interactive session
  else:
    rc= switched._save()
    pass
  return(rc)

if __name__ == "__main__": 
  if len(sys.argv) >= 2:
    rc=main(sys.argv[1])
  else:
    rc=main()
  sys.exit(rc)

