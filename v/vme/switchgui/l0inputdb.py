#!/usr/bin/python
# author: E.Kryshen
# mail: evgeny.kryshen@cern.ch
from Tkinter import *
import string,sys,os,time

sys.path.append(os.environ.get('VMECFDIR')+'/TRG_DBED/')
import myw

TRGDBDIR = os.environ.get('dbctp')+'/'

fname_inputs = TRGDBDIR+'VALID.CTPINPUTS'
fname_switch = TRGDBDIR+'CTP.SWITCH'
fname_switch_inputs = TRGDBDIR+'L0.INPUTS'
fname_switch_inputs_arxiv = TRGDBDIR+'L0.INPUTS.ARXIV/'+'L0.INPUTS'


COLOR_WHITE="#ffffff"
COLOR_NOTCONFIGURED="#ff9999"


#======================================================================
def PrintInfo(fstr):
  print "Info:",fstr
#======================================================================


#======================================================================
def PrintError(fstr):
  print "Error:",fstr
#======================================================================


#======================================================================
def PrintFatal(fstr):
  print "Fatal:",fstr
  sys.exit(0)
#======================================================================


#======================================================================
class ScrollableFrame(Frame):
  def __init__(self, root):
    vscrollbar = Scrollbar(root)
    vscrollbar.grid(row=0, column=1, sticky=N+S)
    hscrollbar = Scrollbar(root, orient=HORIZONTAL)
    hscrollbar.grid(row=1, column=0, sticky=E+W)

    canvas = Canvas(root,yscrollcommand=vscrollbar.set,xscrollcommand=hscrollbar.set,width=1140,height=800)
    canvas.grid(row=0, column=0, sticky=N+S+E+W)
    vscrollbar.config(command=canvas.yview)
    hscrollbar.config(command=canvas.xview)

    # make the canvas expndable
    root.grid_rowconfigure(0, weight=1)
    root.grid_columnconfigure(0, weight=1)

    # create canvas contents

    Frame.__init__(self,canvas,borderwidth=1)
    self.rowconfigure(1, weight=1)
#    self.columnconfigure(1, weight=1)

    canvas.create_window(0, 0, anchor=NW, window=self)
    self.canvas=canvas

  def Update(self):
    self.update_idletasks()
    self.canvas.config(scrollregion=self.canvas.bbox("all"))
#======================================================================



#======================================================================
def redline(inf):
  # ignore empty lines
  while(1):
    cltds= inf.readline()
    if cltds!='\n': break
  return cltds
#======================================================================


#======================================================================
class SwitchInputEntry(Entry):
  # Entry widget tuned for switch configuration usage
  def __init__(self,master,i,j,v,cmdlabel,justify=RIGHT,bg=COLOR_WHITE,width=5):
    Entry.__init__(self,master,textvariable=v,justify=justify,bg=bg,width=width)
    self.cmdlabel = cmdlabel
    self.grid(row=i,column=j,sticky=W+E)
    self.bind("<Key-Return>", self.updateentry)
    self.bind("<Key-Tab>", self.updateentry)
    self.bind("<Leave>", self.updateentry)
  #--------------------------------------------------------------------
  def updateentry(self,event=None):
    ne = self.get()
    self.cmdlabel(ne)
#======================================================================


#======================================================================
class SwitchInput:
  def __init__(self,line,withgui='yes'):
    # create switch input from a line, which is read from SWITCH.INPUTS file
    self.line = string.expandtabs(line)
    self.line = string.strip(self.line,'\n')
    # Split input comments
    self.comments = ''
    comment_split_result = string.split(self.line,'#')
    if len(comment_split_result)==0: 
      PrintError('Parse error: empty line? '+line)
      sys.exit(0)
    elif len(comment_split_result)>2: 
      PrintError('Parse error: too many comment marks'+line)
    elif len(comment_split_result)==2:
      # Assign comments (strip spaces)
      self.comments = string.strip(comment_split_result[1],' ')
    
    li = string.split(comment_split_result[0],' ')
    for ix in range(len(li)-1, -1, -1):
      if li[ix]=='': del li[ix]
    if len(li)>13: PrintError('Failed to parse switch configuration line:'+line)
    self.sin           = int(li[ 0])
    self.detectorname  =     li[ 1]
    self.nameweb       =     li[ 2]
    self.namectp       =     li[ 3]
    self.eq            = int(li[ 4])
    self.signature     = int(li[ 5])
    self.dimnum        = int(li[ 6])
    self.edge          = int(li[ 7])
    self.delay         = int(li[ 8])
    self.configured    = int(li[ 9])
    self.deltamin      = int(li[10])
    self.deltamax      = int(li[11])
    self.level         = 0
    self.sout          = 0
    self.ctpin         = 0
    if withgui!='yes': return
    # variables
    self.v_sin           = StringVar()
    self.v_detectorname  = StringVar()
    self.v_nameweb       = StringVar()
    self.v_namectp       = StringVar()
    self.v_eq            = StringVar()
    self.v_signature     = StringVar()
    self.v_dimnum        = StringVar()
    self.v_edge          = StringVar()
    self.v_delay         = StringVar()
    self.v_configured    = StringVar()
    self.v_deltamin      = StringVar()
    self.v_deltamax      = StringVar()
    self.v_comments      = StringVar()
    
    # set values
    self.v_sin.set(self.sin)
    self.v_detectorname.set(self.detectorname)
    self.v_nameweb.set(self.nameweb)
    self.v_namectp.set(self.namectp)
    self.v_eq.set(self.eq)
    self.v_signature.set(self.signature)
    self.v_dimnum.set(self.dimnum)
    self.v_edge.set(self.edge)
    self.v_delay.set(self.delay)
    self.v_configured.set(self.configured)
    self.v_deltamin.set(self.deltamin)
    self.v_deltamax.set(self.deltamax)
    self.v_comments.set(self.comments)
  
  #--------------------------------------------------------------------
  def print_input(self,stream):
    stream.write(string.rjust(str(self.sin),3)+'   ')
    stream.write(string.ljust(self.detectorname,10))
    stream.write(string.ljust(self.nameweb,25))
    stream.write(string.ljust(self.namectp,15))
    stream.write(string.rjust(str(self.eq),5))
    stream.write(string.rjust(str(self.signature),12))
    stream.write(string.rjust(str(self.dimnum),8))
    stream.write(string.rjust(str(self.edge),8))
    stream.write(string.rjust(str(self.delay),8))
    stream.write(string.rjust(str(self.configured),12))
    stream.write(string.rjust(str(self.deltamin),9))
    stream.write(string.rjust(str(self.deltamax),9))
    stream.write(' #'+self.comments)
    stream.write('\n')
  #--------------------------------------------------------------------
  def print_header(self,stream):
    stream.write('#')
    stream.write(string.rjust('sin',3)+'  ')
    stream.write(string.ljust('detector',10))
    stream.write(string.ljust('nameweb',25))
    stream.write(string.ljust('namectp',15))
    stream.write(string.rjust('eq',5))
    stream.write(string.rjust('signature',12))
    stream.write(string.rjust('dimnum',8))
    stream.write(string.rjust('edge',8))
    stream.write(string.rjust('delay',8))
    stream.write(string.rjust('configured',12))
    stream.write(string.rjust('deltamin',9))
    stream.write(string.rjust('deltamax',9))
    stream.write(string.ljust(' #comments',12))
    stream.write('\n')
  #--------------------------------------------------------------------
  def createEntries(self,mainfr,i):
      self.mainfr=mainfr
      bgd=COLOR_WHITE
      self.e=[]
      if self.configured==0: bgd=COLOR_NOTCONFIGURED
      self.e_sin         =SwitchInputEntry(self.mainfr,i+1, 0,self.v_sin         ,self.changed_sin         ,width=  5,bg=bgd)
      self.e_detectorname=SwitchInputEntry(self.mainfr,i+1, 1,self.v_detectorname,self.changed_detectorname,width= 12,bg=bgd,justify=LEFT)
      self.e_nameweb     =SwitchInputEntry(self.mainfr,i+1, 2,self.v_nameweb     ,self.changed_nameweb     ,width= 20,bg=bgd,justify=LEFT)
      self.e_namectp     =SwitchInputEntry(self.mainfr,i+1, 3,self.v_namectp     ,self.changed_namectp     ,width= 12,bg=bgd,justify=LEFT)
      self.e_eq          =SwitchInputEntry(self.mainfr,i+1, 4,self.v_eq          ,self.changed_eq          ,width=  5,bg=bgd)
      self.e_signature   =SwitchInputEntry(self.mainfr,i+1, 5,self.v_signature   ,self.changed_signature   ,width= 10,bg=bgd)
      self.e_dimnum      =SwitchInputEntry(self.mainfr,i+1, 6,self.v_dimnum      ,self.changed_dimnum      ,width= 10,bg=bgd)
      self.e_edge        =SwitchInputEntry(self.mainfr,i+1, 7,self.v_edge        ,self.changed_edge        ,width=  7,bg=bgd)
      self.e_delay       =SwitchInputEntry(self.mainfr,i+1, 8,self.v_delay       ,self.changed_delay       ,width=  8,bg=bgd)
      self.e_configured  =SwitchInputEntry(self.mainfr,i+1, 9,self.v_configured  ,self.changed_configured  ,width= 10,bg=bgd)
      self.e_deltamin    =SwitchInputEntry(self.mainfr,i+1,10,self.v_deltamin    ,self.changed_deltamin    ,width=  8,bg=bgd)
      self.e_deltamax    =SwitchInputEntry(self.mainfr,i+1,11,self.v_deltamax    ,self.changed_deltamax    ,width=  8,bg=bgd)
      self.e_comments    =SwitchInputEntry(self.mainfr,i+1,12,self.v_comments    ,self.changed_comments    ,width= 30,bg=bgd,justify=LEFT)
      self.e.append(self.e_sin)
      self.e.append(self.e_detectorname)
      self.e.append(self.e_nameweb)
      self.e.append(self.e_namectp)
      self.e.append(self.e_eq)
      self.e.append(self.e_signature)
      self.e.append(self.e_dimnum)
      self.e.append(self.e_edge)
      self.e.append(self.e_delay)
      self.e.append(self.e_configured)
      self.e.append(self.e_deltamin)
      self.e.append(self.e_deltamax)
      self.e.append(self.e_comments)

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
  def changed_edge(self,choice):
    if self.edge==int(self.v_edge.get()): return
    print 'changed edge'
    self.edge=int(self.v_edge.get())
  #--------------------------------------------------------------------
  def changed_delay(self,choice):
    if self.delay==int(self.v_delay.get()): return
    print 'changed dimnum'
    self.delay=int(self.v_delay.get())
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
  #--------------------------------------------------------------------
  def changed_deltamin(self,choice):
    if self.deltamin==int(self.v_deltamin.get()): return
    print 'changed deltamin'
    self.deltamin=int(self.v_deltamin.get())
  #--------------------------------------------------------------------
  def changed_deltamax(self,choice):
    if self.deltamax==int(self.v_deltamax.get()): return
    print 'changed deltamax'
    self.deltamax=int(self.v_deltamax.get())
  #--------------------------------------------------------------------
  def changed_comments(self,choice):
    if self.comments==self.v_comments.get(): return
    print 'changed comments'
    self.comments=self.v_comments.get()
#======================================================================


#======================================================================
class SwitchInputsDB:
  def __init__(self,fname, withgui='yes'):
    #print 'Init SwitchInputsDB'
    self.withgui= withgui
    self.comments=[]
    self.switchInputs=[]
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
    #print 'Reading configuration of switch inputs from a file'
    while 1:
      line = redline(self.infile)
      if not line: break
      if line[0]=='#': 
        if line[0:4]!='#sin': self.comments.append(line)
	continue
      switchInput = SwitchInput(line, self.withgui)
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
#    self.mainfr=Frame(master,borderwidth=1,relief=RAISED,width=10)
#    self.mainfr.pack(side=TOP,expand='no')
    self.mainfr=ScrollableFrame(self.master)
    self.menubar = myw.MywMenu(self.master)
    self.master.config(menu=self.menubar)
    self.filemenu = self.menubar.addcascade('File')
    self.showmenu = self.menubar.addcascade('Show')
    self.filemenu.addcommand('Save',self._save)
    self.filemenu.addcommand('Cancel',self._cancel)
    self.filemenu.addcommand('Quit',self._quit)
    #self.mainfr.columnconfigure( 0,minsize=10,weight=10)
    #self.mainfr.columnconfigure( 1,minsize=80,weight=80)
    #self.mainfr.columnconfigure( 2,minsize=150,weight=150)
    #self.mainfr.columnconfigure( 3,minsize=100,weight=150)
    #self.mainfr.columnconfigure( 4,minsize=10,weight=10)
    #self.mainfr.columnconfigure( 5,minsize=20,weight=50)
    #self.mainfr.columnconfigure( 6,minsize=20,weight=50)
    #self.mainfr.columnconfigure( 7,minsize=10,weight=10)
    #self.mainfr.columnconfigure( 8,minsize=10,weight=10)
    #self.mainfr.columnconfigure( 9,minsize=20,weight=50)
    Label(self.mainfr,text=' sin '       ).grid(row=0,column= 0,sticky=W+E)
    Label(self.mainfr,text=' detector '  ).grid(row=0,column= 1,sticky=W+E)
    Label(self.mainfr,text=' nameweb '   ).grid(row=0,column= 2,sticky=W+E)
    Label(self.mainfr,text=' namectp '   ).grid(row=0,column= 3,sticky=W+E)
    Label(self.mainfr,text=' eq '        ).grid(row=0,column= 4,sticky=W+E)
    Label(self.mainfr,text=' signature ' ).grid(row=0,column= 5,sticky=W+E)
    Label(self.mainfr,text=' dimnum '    ).grid(row=0,column= 6,sticky=W+E)
    Label(self.mainfr,text=' edge '      ).grid(row=0,column= 7,sticky=W+E)
    Label(self.mainfr,text=' delay '     ).grid(row=0,column= 8,sticky=W+E)
    Label(self.mainfr,text=' configured ').grid(row=0,column= 9,sticky=W+E)
    Label(self.mainfr,text=' deltamin   ').grid(row=0,column=10,sticky=W+E)
    Label(self.mainfr,text=' deltamax   ').grid(row=0,column=11,sticky=W+E)
    Label(self.mainfr,text=' comments   ').grid(row=0,column=12,sticky=W+E)

    i=0
    for inp in self.switchInputs:
      inp.createEntries(self.mainfr,i)
      i=i+1
    self.mainfr.Update()
  #--------------------------------------------------------------------
  def _save(self, minst, ix):
    print 'save'
    date_time=time.strftime(".%Y-%m-%d.%H-%M-%S",time.localtime())
    os.rename(fname_switch_inputs,fname_switch_inputs_arxiv+date_time)
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
  #switchInputsDB.write(sys.stdout)
  f.title('L0 inputs database')
  gui = Switch_GUI(f,switchInputsDB)
  f.mainloop()
#======================================================================

if __name__ == "__main__": main()
