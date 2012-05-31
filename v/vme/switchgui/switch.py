#!/usr/bin/python
# author: E.Kryshen
# mail: evgeny.kryshen@cern.ch
from Tkinter import *
import string,sys
import myw
import sets

fname_inputs = '/home/kryshen/alice/switch/DB/VALID.CTPINPUTS'
fname_switch = '/home/kryshen/alice/switch/DB/CTP.SWITCH'
TRGDBDIR = '/home/kryshen/alice/switch/DB/'


COLOR_PART="#006699"
COLOR_CLUSTER="#00cccc"
COLOR_NORMAL="#d9d9d9"
COLOR_TDSPFS="#ffff66"
COLOR_RARE="#99ff99"
COLOR_SHARED="#cc66cc"
COLOR_WARN="#ff9999"
COLOR_ACTIVE="#ff00cc"
COLOR_OK="#00ff00"
COLOR_WHITE="#ffffff"

#======================================================================
class SwitchOptionMenu(OptionMenu):
  def __init__(self, master,row,column,variable,*options):
    OptionMenu.__init__(self,master,variable,'None',command=self.change,*options)
    self.grid(row=row,column=column,sticky=W+E)
  def change(self,event):
    print 'bla'
#======================================================================


#======================================================================
class SwitchLabel(Label):
  def __init__(self, master,row,column,cnf={},**kw):
    Label.__init__(self,master,cnf={}, **kw)
    self.grid(row=row,column=column,sticky=W+E)
#======================================================================


#======================================================================
def PrintError(fstr):
  print "Error:",fstr
#======================================================================


#======================================================================
def redline(inf):                 # ignore empty lines
  while(1):
    cltds= inf.readline()
    if cltds!='\n': break    
  return cltds
#======================================================================


#======================================================================
class CtpInput:
  def __init__(self,line):
    self.line          = line
    self.name          = None
    self.detectorname  = ' '
    self.level         = None
    self.signature     = None
    self.inpnum        = None
    self.dimnum        = None
    self.configured    = None
    self.l0fdefinition = None
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
    if len(li) >= 3:
      self.level      = int(li[1])
      self.signature  = int(li[2])
      self.inpnum     = int(li[3])
      self.dimnum     = int(li[4])
      self.configured = int(li[5])
    elif len(li) == 1:
      self.configured = 0
    else:
      PrintError("Bad CTP input definition:"+line)
  def print_input(self,stream):
    stream.write(string.ljust(self.name,15)+' = ')
    if self.l0fdefinition==None:
      stream.write(string.ljust(self.detectorname,15))
      stream.write(str(self.level)+' \t')
      stream.write(string.rjust(str(self.signature),3)+' \t')
      stream.write(string.rjust(str(self.inpnum),3)+' \t')
      stream.write(string.rjust(str(self.dimnum),3)+' \t')
      stream.write(str(self.configured)+'\n')
    else:   
      stream.write(self.l0fdefinition+'\n')
#======================================================================


#======================================================================
class SwitchInput:
  def __init__(self,line):
    self.line = string.expandtabs(line)
    self.line = string.strip(self.line,'\n')
    li = string.split(self.line,' ')
    #print "lili:",li
    # remove empty spaces
    for ix in range(len(li)-1, -1, -1):
      if li[ix]=='': del li[ix]
    if len(li)>6: PrintError('Failed to parse switch configuration line:'+line)
    self.nameweb = li[0]
    self.namectp = li[1]
    self.eq      = int(li[2])
    self.sin     = int(li[3])
    self.sout    = int(li[4])
    self.ctpin = self.sout-1
    if len(li)==6: 
      li5 = li[5]
      self.ctpin = int(li5)
    self.ctpin_id   = -1
    self.detectorname  = 'None'
    self.level         = 0
    self.signature     = 0
    self.dimnum        = 0
  #--------------------------------------------------------------------
  def print_input(self,stream):
    stream.write(string.rjust(str(self.sin),3)+'    ')
    stream.write(string.ljust(self.detectorname,15))
    stream.write(string.ljust(self.nameweb,25))
    stream.write(string.ljust(self.namectp,15))
    stream.write(string.rjust(str(self.eq),10))
#    stream.write(string.rjust(str(self.sout),3)+'\t')
#    stream.write(string.rjust(str(self.ctpin),3)+'\t')
    stream.write(string.rjust(str(self.level),10))
    stream.write(string.rjust(str(self.signature),10))
    stream.write(string.rjust(str(self.dimnum),10))
    stream.write(string.rjust(str(1),10))
    
#    if stream==sys.stdout:
#      stream.write(string.rjust(str(self.ctpin_id),3)+'\n')
#    else: 
    stream.write('\n')
#======================================================================


#======================================================================
class InputsDB:
  def __init__(self,fname):
    self.inputs = []
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
    print 'Read trigger inputs...'
    while 1:
      line = self.infile.readline()
      if not line: break
      if line[0]=='#': continue
      input = CtpInput(line)
      self.inputs.append(input)
#======================================================================


#======================================================================
class SwitchDB:
  def __init__(self,fname):
    self.switch = []
    # Open file with CTP switch configuration 
    try:
      self.infile = open(fname,'r')
      if self.infile: 
        print 'File with CTP switch configuration opened:',fname
        self.read()
        self.infile.close()
    except IOError:
       PrintError('I/O Error during loading CTP switch configuration:'+fname)
       sys.exit(0)
  #--------------------------------------------------------------------
  def read(self):
    # Read trigger inputs from the file
    print 'Read CTP switch configuration...'
    while 1:
      line = redline(self.infile)
      if not line: break
      if line[0]=='#': continue
      switch_input = SwitchInput(line)
      self.switch.append(switch_input)
#      print line
  #--------------------------------------------------------------------
  def updateCTPinputs(self,inputsDB):
    for switch in self.switch:
      if switch.ctpin==-1:
	break
      ictpin=0
      #print switch.ctpin,
      for ctp_input in inputsDB.inputs:
        #print ctp_input.inpnum,
	if switch.ctpin==ctp_input.inpnum:
          switch.ctpin_id = ictpin
	  switch.detectorname = ctp_input.detectorname
	  switch.level        = ctp_input.level
	  switch.signature    = ctp_input.signature
	  switch.dimnum       = ctp_input.dimnum
	  break
        ictpin=ictpin+1
      #print ' '
#======================================================================


#======================================================================
class GUI:
  def __init__(self, master,switchDB,inputsDB):
    self.switchDB = switchDB
    self.inputsDB = inputsDB
    self.master = master
    self.mainfr = myw.MywFrame(self.master,side=BOTTOM)
    self.menubar = myw.MywMenu(self.master)
    self.master.config(menu=self.menubar)
    self.filemenu = self.menubar.addcascade('File')
    self.showmenu = self.menubar.addcascade('Show')
    self.filemenu.addcommand('Save',self.save)
    self.filemenu.addcommand('Save as',self.saveAs)
    self.filemenu.addcommand('Cancel',self.cancel)
    self.filemenu.addcommand('Quit',self.quit)
    
    self.fr_table = myw.MywFrame(self.master,side=TOP,expand='yes',fill='x')
    """
    self.fr_ctpin    = myw.MywFrame(self.master,side=LEFT)
    self.fr_det      = myw.MywFrame(self.master,side=LEFT)
    self.fr_ctpname  = myw.MywFrame(self.master,side=LEFT)
    """
    
#    self.fr_inputs = []
    
#    for i in range(0,24):
#      self.fr_inputs.append(myw.MywFrame(self.master,side=TOP))
    
    # create labels for columns
    #self.l_ctpin   = myw.MywLabel(self.fr_ctpin   ,label='CTP input No',side=TOP)
    #self.l_det     = myw.MywLabel(self.fr_det     ,label='Detector'    ,side=TOP)
    #self.l_ctpname = myw.MywLabel(self.fr_ctpname ,label='Input name'  ,side=TOP)
  
    det = []
    for switch_input in switchDB.switch:
      if switch_input.detectorname=='None': continue
      if switch_input.detectorname not in det:
        det.append(switch_input.detectorname)
      
    ctpnames = []
    for d in det:
      ctpnames.append([])
      print d
  
    for switch_input in switchDB.switch:
      if switch_input.namectp=='none': continue
      ctpnames[det.index(switch_input.detectorname)].append(switch_input.namectp)
    
    for d in ctpnames:
      for i in d:
        print i,
      print ' '
  
  
    self.l_ctpins = []
    self.om_det = []
    self.var_det = []
    self.om_ctpname = []
    self.var_ctpname = []
    
    self.list_det = det
    self.list_ctpname = []
    
    
    
    for i in range(0,24):
      self.list_ctpname.append([])
    
    self.fr_table.columnconfigure(0,minsize=100)
    self.fr_table.columnconfigure(1,minsize=150)
    self.fr_table.columnconfigure(2,minsize=150)
    self.fr_table.configure(padx=10)
    
    self.l_ctpins.append(SwitchLabel(self.fr_table,0,0,text='CTP input'))
    self.om_det.append(SwitchLabel(self.fr_table,0,1,text='Detector'))
    self.om_ctpname.append(SwitchLabel(self.fr_table,0,2,text='Input name'))
    
    
    for i in range(0,24):
      print i
      self.var_ctpname.append(Variable())
      self.var_det.append(Variable())
      self.var_det[i].set('None')
    
    for switch_input in switchDB.switch:
      if switch_input.ctpin:
        self.var_det[switch_input.ctpin-1].set(switch_input.detectorname)
	if switch_input.detectorname=='None': continue
	print switch_input.detectorname, det.index(switch_input.detectorname)
        self.list_ctpname[switch_input.ctpin-1]=ctpnames[det.index(switch_input.detectorname)]
        self.var_ctpname[switch_input.ctpin-1].set(switch_input.namectp)

    for i in range(0,24):
      self.l_ctpins.append(SwitchLabel(self.fr_table,i+1,0,text=str(i+1)))
      self.om_det.append(SwitchOptionMenu(self.fr_table,i+1,1,self.var_det[i],*self.list_det))
      self.om_ctpname.append(SwitchOptionMenu(self.fr_table,i+1,2,self.var_ctpname[i],*self.list_ctpname[i]))
      #self.om_det[i].bind('<ButtonRelease>',self.change)
      
  #--------------------------------------------------------------------
  def save(self, minst, ix):
    print 'save'
    new_switch_file = open(fname_switch+'.new','w')
    for switch in self.switchDB.switch: 
      switch.print_input(new_switch_file)
    new_switch_file.close()  
  
    new_inputs_file = open(fname_inputs+'.new','w')
    for input in self.inputsDB.inputs:
      input.print_input(new_inputs_file)
    new_inputs_file.close()  
  #--------------------------------------------------------------------
  def saveAs(self, minst, ix):
    print 'saveAs'
  #--------------------------------------------------------------------
  def cancel(self, minst=None, ix=None):
    print 'cancel'
  #--------------------------------------------------------------------
  def quit(self, minst, ix):
    print 'quit'
    sys.exit(0)
#======================================================================


#======================================================================
class Entries:
  def __init__(self, master,switch_inputs,ctp_inputs):
    self.master = master
    self.ctp_inputs    = ctp_inputs
    self.switch_inputs = switch_inputs
    # create collections of SWITCH inputs data
    self.v_nameweb    = []
    self.v_namectp    = []
    self.v_eq         = []
    self.v_sin        = []
    self.v_sout       = []
    self.v_ctpin      = []
    self.v_det        = []
    self.v_level      = []
    self.v_signature  = []
    self.v_dimnum     = []
    self.v_configured = []
    self.nameweb      = []
    self.namectp      = []
    self.eq           = []
    self.sin          = []
    self.sout         = []
    self.ctpin        = []
    self.det          = []
    self.level        = []
    self.signature    = []
    self.dimnum       = []
    self.configured   = []
    # create frames (columns with SWITCH inputs data)
    self.fr_nameweb    = myw.MywFrame(self.master,side=LEFT)
    self.fr_namectp    = myw.MywFrame(self.master,side=LEFT)
    self.fr_eq         = myw.MywFrame(self.master,side=LEFT)
    self.fr_sin        = myw.MywFrame(self.master,side=LEFT,expand='no')
    self.fr_sout       = myw.MywFrame(self.master,side=LEFT,expand='no')
    self.fr_ctpin      = myw.MywFrame(self.master,side=LEFT,expand='no')
    self.fr_det        = myw.MywFrame(self.master,side=LEFT,expand='no')
    self.fr_level      = myw.MywFrame(self.master,side=LEFT,expand='no')
    self.fr_signature  = myw.MywFrame(self.master,side=LEFT,expand='no')
    self.fr_dimnum     = myw.MywFrame(self.master,side=LEFT,expand='no')
    self.fr_configured = myw.MywFrame(self.master,side=LEFT,expand='no')
    # create labels for columns
    self.l_nameweb    = myw.MywLabel(self.fr_nameweb    ,label='Switch name',side=TOP)
    self.l_namectp    = myw.MywLabel(self.fr_namectp    ,label='CTP name'   ,side=TOP)
    self.l_eq         = myw.MywLabel(self.fr_eq         ,label='Eq'         ,side=TOP)
    self.l_sin        = myw.MywLabel(self.fr_sin        ,label='Switch in'  ,side=TOP)
    self.l_sout       = myw.MywLabel(self.fr_sout       ,label='Switch out' ,side=TOP)
    self.l_ctpin      = myw.MywLabel(self.fr_ctpin      ,label='CTP input'  ,side=TOP)
    self.l_det        = myw.MywLabel(self.fr_det        ,label='Detector'   ,side=TOP)
    self.l_level      = myw.MywLabel(self.fr_level      ,label='Level'      ,side=TOP)
    self.l_signature  = myw.MywLabel(self.fr_signature  ,label='Signature'  ,side=TOP)
    self.l_dimnum     = myw.MywLabel(self.fr_dimnum     ,label='DIM number' ,side=TOP)
    self.l_configured = myw.MywLabel(self.fr_configured ,label='Configured' ,side=TOP)

  #--------------------------------------------------------------------
  def create(self):  
    self.v_nameweb    = []
    self.v_namectp    = []
    self.v_eq         = []
    self.v_sin        = []
    self.v_sout       = []
    self.v_ctpin      = []
    self.v_det        = []
    self.v_level      = []
    self.v_signature  = []
    self.v_dimnum     = []
    self.v_configured = []
    
    for switchInput in self.switch_inputs:
      e_nameweb    = SwitchEntry(self.fr_nameweb   ,textvariable=self.v_nameweb   ,cmdlabel=self.changed_nameweb,justify=LEFT)
      e_namectp    = SwitchEntry(self.fr_namectp   ,textvariable=self.v_namectp   ,cmdlabel=self.changed_namectp,justify=LEFT)
      e_eq         = SwitchEntry(self.fr_eq        ,textvariable=self.v_eq        ,cmdlabel=self.changed_eq)
      e_sin        = SwitchEntry(self.fr_sin       ,textvariable=self.v_sin       ,cmdlabel=self.changed_sin)
      e_sout       = SwitchEntry(self.fr_sout      ,textvariable=self.v_sout      ,cmdlabel=self.changed_sout)
      e_ctpin      = SwitchEntry(self.fr_ctpin     ,textvariable=self.v_ctpin     ,cmdlabel=self.changed_ctpin)
      e_det        = SwitchEntry(self.fr_det       ,textvariable=self.v_det       ,cmdlabel=self.changed_det)
      e_level      = SwitchEntry(self.fr_level     ,textvariable=self.v_level     ,cmdlabel=self.changed_level)
      e_signature  = SwitchEntry(self.fr_signature ,textvariable=self.v_signature ,cmdlabel=self.changed_signature)
      e_dimnum     = SwitchEntry(self.fr_dimnum    ,textvariable=self.v_dimnum    ,cmdlabel=self.changed_dimnum)
      e_configured = SwitchEntry(self.fr_configured,textvariable=self.v_configured,cmdlabel=self.changed_configured)
      if switchInput.ctpin_id>=0:
        ctp_input = self.ctp_inputs[switch_input.ctpin_id]
        e_det.delete(0,END)
        e_level.delete(0,END)
	e_signature.delete(0,END)
	e_dimnum.delete(0,END)
	e_configured.delete(0,END)
	e_det.insert('end',ctp_input.detectorname)
        e_level.insert('end',ctp_input.level)
        e_signature.insert('end',ctp_input.signature)
	e_dimnum.insert('end',ctp_input.dimnum)
	e_configured.insert('end',ctp_input.configured)
      self.nameweb.append(e_nameweb)
      self.namectp.append(e_namectp)
      self.eq.append(e_eq)
      self.sin.append(e_sin)
      self.sout.append(e_sout)
      self.ctpin.append(e_ctpin)
      self.det.append(e_det)
      self.level.append(e_level)
      self.signature.append(e_signature)
      self.dimnum.append(e_dimnum)
      self.configured.append(e_configured)
  #--------------------------------------------------------------------
  def changed_nameweb(self,event=None):
      print 'SWITCH input name changed'
  #--------------------------------------------------------------------
  def changed_namectp(self,event=None):
      print 'CTP input name changed'
  #--------------------------------------------------------------------
  def changed_eq(self,event=None):
      print 'Eq changed'
  #--------------------------------------------------------------------
  def changed_sin(self,event=None):
      print 'SWITCH in changed'
  #--------------------------------------------------------------------
  def changed_sout(self,event=None):
      print 'SWITCH out changed'
  #--------------------------------------------------------------------
  def changed_ctpin(self,event=None):
      print 'CTP input changed'
  #--------------------------------------------------------------------
  def changed_det(self,event=None):
      print 'Detector changed'
  #--------------------------------------------------------------------
  def changed_level(self,event=None):
      print 'Level changed'
  #--------------------------------------------------------------------
  def changed_signature(self,event=None):
      print 'Signature changed'
  #--------------------------------------------------------------------
  def changed_dimnum(self,event=None):
      print 'DIM number changed'
  #--------------------------------------------------------------------
  def changed_configured(self,event=None):
      print 'CTP input changed'
#======================================================================


#======================================================================
def main():
    
  inputsDB = InputsDB(fname_inputs)
  for input in inputsDB.inputs:
    input.print_input(sys.stdout)
  
  switchDB = SwitchDB(fname_switch)
  switchDB.updateCTPinputs(inputsDB)
  for switch in switchDB.switch: 
    switch.print_input(sys.stdout)
  print 'done'
  
  f= Tk()
  f.title('CTP switch')
  gui = GUI(f,switchDB,inputsDB)
  """
  entries = Entries(gui.mainfr,switchDB.switch,inputsDB.inputs)
  entries.create()
  """
  f.mainloop()
    

if __name__ == "__main__": main()
