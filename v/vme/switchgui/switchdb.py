#!/usr/bin/python
# author: E.Kryshen
# mail: evgeny.kryshen@cern.ch
from Tkinter import *
import string,sys
from switch import CtpInput,GUI
from time import strftime, localtime
import os
from os import rename

#TRGDBDIR = '/home/kryshen/alice/switch/DB/'
#TRGDBDIR = os.environ.get('VMECFDIR')
TRGDBDIR = os.environ.get('dbctp')+'/'
fname_inputs = TRGDBDIR+'VALID.CTPINPUTS'
fname_switch = TRGDBDIR+'CTP.SWITCH'
fname_switch_inputs = TRGDBDIR+'SWITCH.INPUTS'
fname_switch_inputs_arxiv = TRGDBDIR+'SWITCH.INPUTS.ARXIV/'+'SWITCH.INPUTS'

sys.path.append(os.environ.get('VMECFDIR')+'/TRG_DBED/')
import myw

COLOR_WHITE="#ffffff"
COLOR_NOTCONFIGURED="#ff9999"

#======================================================================
def redline(inf):
  # ignore empty lines
  while(1):
    cltds= inf.readline()
    if cltds!='\n': break
  return cltds
#======================================================================

#======================================================================
def PrintError(fstr):
  print "Error:",fstr

#======================================================================

#======================================================================
class SwitchInputEntry(Entry):
  # Entry widget tuned for switch configuration usage
  def __init__(self,master,i,j,v,cmdlabel,justify=RIGHT,bg=COLOR_WHITE):
    Entry.__init__(self,master,textvariable=v,justify=justify,bg=bg,width=10)
    self.cmdlabel = cmdlabel
    self.grid(row=i,column=j,sticky=W+E)
    self.bind("<Key-Return>", self.updateentry)
    self.bind("<Key-Tab>", self.updateentry)
  #--------------------------------------------------------------------
  def updateentry(self,event=None):
    ne = self.get()
    self.cmdlabel(ne)
#======================================================================


#======================================================================
class SwitchInput:
  def __init__(self,line):
    # create switch input from a line, which is read from SWITCH.INPUTS file
    self.line = string.expandtabs(line)
    self.line = string.strip(self.line,'\n')
    li = string.split(self.line,' ')
    for ix in range(len(li)-1, -1, -1):
      if li[ix]=='': del li[ix]
    if len(li)>9: PrintError('Failed to parse switch configuration line:'+line)
    self.sin           = int(li[0])
    self.detectorname  =     li[1]
    self.nameweb       =     li[2]
    self.namectp       =     li[3]
    self.eq            = int(li[4])
    self.level         = int(li[5])
    self.signature     = int(li[6])
    self.dimnum        = int(li[7])
    self.configured    = int(li[8])
    self.sout          = 0
    self.ctpin         = 0
    
    # variables
    self.v_sin           = StringVar()
    self.v_detectorname  = StringVar()
    self.v_nameweb       = StringVar()
    self.v_namectp       = StringVar()
    self.v_eq            = StringVar()
    self.v_level         = StringVar()
    self.v_signature     = StringVar()
    self.v_dimnum        = StringVar()
    self.v_configured    = StringVar()
    
    # set values
    self.v_sin.set(self.sin)
    self.v_detectorname.set(self.detectorname)
    self.v_nameweb.set(self.nameweb)
    self.v_namectp.set(self.namectp)
    self.v_eq.set(self.eq)
    self.v_level.set(self.level)
    self.v_signature.set(self.signature)
    self.v_dimnum.set(self.dimnum)
    self.v_configured.set(self.configured)
  
  #--------------------------------------------------------------------
  def print_input(self,stream):
    stream.write(string.rjust(str(self.sin),3)+'   ')
    stream.write(string.ljust(self.detectorname,10))
    stream.write(string.ljust(self.nameweb,25))
    stream.write(string.ljust(self.namectp,15))
    stream.write(string.rjust(str(self.eq),5))
    stream.write(string.rjust(str(self.level),8))
    stream.write(string.rjust(str(self.signature),12))
    stream.write(string.rjust(str(self.dimnum),8))
    stream.write(string.rjust(str(self.configured),12))
    stream.write('\n')
  #--------------------------------------------------------------------
  def print_header(self,stream):
    stream.write('#')
    stream.write(string.rjust('sin',3)+'  ')
    stream.write(string.ljust('detector',10))
    stream.write(string.ljust('nameweb',25))
    stream.write(string.ljust('namectp',15))
    stream.write(string.rjust('eq',5))
    stream.write(string.rjust('level',8))
    stream.write(string.rjust('signature',12))
    stream.write(string.rjust('dimnum',8))
    stream.write(string.rjust('configured',12))
    stream.write('\n')
  #--------------------------------------------------------------------
  def createEntries(self,mainfr,i):
      self.mainfr=mainfr
      bgd=COLOR_WHITE
      self.e=[]
      if self.configured==0: bgd=COLOR_NOTCONFIGURED
      self.e_sin=SwitchInputEntry(self.mainfr,i+1,0,self.v_sin,self.changed_sin,bg=bgd)
      self.e_detectorname=SwitchInputEntry(self.mainfr,i+1,1,self.v_detectorname,self.changed_detectorname,justify=LEFT,bg=bgd)
      self.e_nameweb=SwitchInputEntry(self.mainfr,i+1,2,self.v_nameweb,self.changed_nameweb,justify=LEFT,bg=bgd)
      self.e_namectp=SwitchInputEntry(self.mainfr,i+1,3,self.v_namectp,self.changed_namectp,justify=LEFT,bg=bgd)
      self.e_eq=SwitchInputEntry(self.mainfr,i+1,4,self.v_eq,self.changed_eq,bg=bgd)
      self.e_level=SwitchInputEntry(self.mainfr,i+1,5,self.v_level,self.changed_level,bg=bgd)
      self.e_signature=SwitchInputEntry(self.mainfr,i+1,6,self.v_signature,self.changed_signature,bg=bgd)
      self.e_dimnum=SwitchInputEntry(self.mainfr,i+1,7,self.v_dimnum,self.changed_dimnum,bg=bgd)
      self.e_configured=SwitchInputEntry(self.mainfr,i+1,8,self.v_configured,self.changed_configured,bg=bgd)
      self.e.append(self.e_sin)
      self.e.append(self.e_detectorname)
      self.e.append(self.e_nameweb)
      self.e.append(self.e_namectp)
      self.e.append(self.e_eq)
      self.e.append(self.e_level)
      self.e.append(self.e_signature)
      self.e.append(self.e_dimnum)
      self.e.append(self.e_configured)

  #--------------------------------------------------------------------
  def changed_sin(self,choice):
    if self.sin==int(self.v_sin.get()): return
    print 'changed sin'
    self.sin=int(self.v_sin.get())
  #--------------------------------------------------------------------
  def changed_detectorname(self,choice):
    if self.detectorname==self.v_detectorname.get(): return
    print 'changed detectorname'
    self.detectorname=self.v_detectorname.get()
  #--------------------------------------------------------------------
  def changed_nameweb(self,choice):
    if self.nameweb==self.v_nameweb.get(): return
    print 'changed nameweb'
    self.nameweb=self.v_nameweb.get()
  #--------------------------------------------------------------------
  def changed_namectp(self,choice):
    if self.namectp==self.v_namectp.get(): return
    print 'changed namectp'
    self.namectp=self.v_namectp.get()
  #--------------------------------------------------------------------
  def changed_eq(self,choice):
    if self.eq==int(self.v_eq.get()): return
    print 'changed eq'
    self.eq=int(self.v_eq.get())
  #--------------------------------------------------------------------
  def changed_level(self,choice):
    if self.level==int(self.v_level.get()): return
    print 'changed level'
    self.level=int(self.v_level.get())
  #--------------------------------------------------------------------
  def changed_signature(self,choice):
    if self.signature==int(self.v_signature.get()): return
    print 'changed signature'
    self.signature=int(self.v_signature.get())
  #--------------------------------------------------------------------
  def changed_dimnum(self,choice):
    if self.dimnum==int(self.v_dimnum.get()): return
    print 'changed dimnum'
    self.dimnum=int(self.v_dimnum.get())
  #--------------------------------------------------------------------
  def changed_configured(self,choice):
    if self.configured==int(self.v_configured.get()): return
    print 'changed configured'
    self.configured=int(self.v_configured.get())
    bgd=COLOR_WHITE
    if self.configured==0: 
      bgd=COLOR_NOTCONFIGURED
    for e in self.e:
      e.config(bg=bgd)
#======================================================================


#======================================================================
class SwitchInputsDB:
  def __init__(self,fname):
    print 'Init SwitchInputsDB'
    self.comments=[]
    self.switchInputs=[]
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
    print 'Reading configuration of switch inputs from a file'
    while 1:
      line = redline(self.infile)
      if not line: break
      if line[0]=='#': 
        if line[0:4]!='#sin': self.comments.append(line)
	continue
      switchInput = SwitchInput(line)
      self.switchInputs.append(switchInput)
  #--------------------------------------------------------------------
  def write(self,stream):
    header = 0
    for switchInput in self.switchInputs: 
      if header==0: 
	switchInput.print_header(stream)
	header=1
      switchInput.print_input(stream)
  #--------------------------------------------------------------------
  def write_to_file(self,fname):
    new_file = open(fname,'w')
    for comment in self.comments:
      new_file.write(comment)
    self.write(new_file)
    new_file.close()
#======================================================================


#======================================================================
class Switch_GUI:
  def __init__(self, master,switchInputsDB):
    self.master = master
    self.switchInputsDB = switchInputsDB
    self.switchInputs = switchInputsDB.switchInputs
    self.mainfr=Frame(master,borderwidth=1,relief=RAISED,width=10)
    self.mainfr.pack(side=TOP,expand='no')
    self.menubar = myw.MywMenu(self.master)
    self.master.config(menu=self.menubar)
    self.filemenu = self.menubar.addcascade('File')
    self.showmenu = self.menubar.addcascade('Show')
    self.filemenu.addcommand('Save',self._save)
    self.filemenu.addcommand('Cancel',self._cancel)
    self.filemenu.addcommand('Quit',self._quit)
    self.mainfr.columnconfigure(0,minsize=20,weight=1)
    self.mainfr.columnconfigure(1,minsize=80,weight=3)
    self.mainfr.columnconfigure(2,minsize=150,weight=3)
    self.mainfr.columnconfigure(3,minsize=100,weight=3)
    self.mainfr.columnconfigure(4,minsize=20,weight=1)
    self.mainfr.columnconfigure(5,minsize=20,weight=1)
    self.mainfr.columnconfigure(6,minsize=20,weight=1)
    self.mainfr.columnconfigure(7,minsize=20,weight=1)
    self.mainfr.columnconfigure(8,minsize=20,weight=1)
    Label(self.mainfr,text='sin'       ).grid(row=0,column=0,sticky=W+E)
    Label(self.mainfr,text='detector'  ).grid(row=0,column=1,sticky=W+E)
    Label(self.mainfr,text='nameweb'   ).grid(row=0,column=2,sticky=W+E)
    Label(self.mainfr,text='namectp'   ).grid(row=0,column=3,sticky=W+E)
    Label(self.mainfr,text='eq'        ).grid(row=0,column=4,sticky=W+E)
    Label(self.mainfr,text='level'     ).grid(row=0,column=5,sticky=W+E)
    Label(self.mainfr,text='signature' ).grid(row=0,column=6,sticky=W+E)
    Label(self.mainfr,text='dimnum'    ).grid(row=0,column=7,sticky=W+E)
    Label(self.mainfr,text='configured').grid(row=0,column=8,sticky=W+E)

    i=0
    for inp in self.switchInputs:
      inp.createEntries(self.mainfr,i)
      i=i+1
  #--------------------------------------------------------------------
  def _save(self, minst, ix):
    print 'save'
    date_time=strftime(".%Y-%m-%d.%H-%M-%S", localtime())
    rename(fname_switch_inputs,fname_switch_inputs_arxiv+date_time)
    self.switchInputsDB.write_to_file(fname_switch_inputs)
  #--------------------------------------------------------------------
  def _cancel(self, minst=None, ix=None):
    print 'cancel'
  #--------------------------------------------------------------------
  def _quit(self, minst, ix):
    print 'quit'
    sys.exit(0)
#======================================================================


#======================================================================
def main():
  f= Tk()
  
  switchInputsDB = SwitchInputsDB(fname_switch_inputs)
  switchInputsDB.write(sys.stdout)
  f.title('Switch Inputs Config')
  gui = Switch_GUI(f,switchInputsDB)
  f.mainloop()
#======================================================================

if __name__ == "__main__": main()
