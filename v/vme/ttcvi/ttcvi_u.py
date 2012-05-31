from Tkinter import *
import string
import myw, cmdlin2

#-------------------------------------- An example of user defined function:
def guiexample(vb):
  class st:
    def __init__(self, vb):
      self.vb=vb
      self.f=Toplevel(vb.master)
      self.f.title("guiexamle")
      self.freq=myw.MywRadio(self.f)
      self.exbutton= myw.MywButton(self.f,label="set",cmd=self.setTrig)
      self.cmdout=myw.MywEntry(self.f, label='cmd output',helptext=
      """
This is the output entry
where the 1st line of the command output
is given
      """)
    def setTrig(self):
      freq= self.freq.getEntry()
      print "value:", freq
      print "=========="
      apout= self.vb.io.execute("a="+freq, "giveoutput")
      print apout,"=========="
      self.cmdout.setEntry(string.split(apout,'\n',1)[0])
  st(vb)

#--------------------------------------set trigger source and  frequency
def setTrigRadio(vb):
  class st:
    hlptxt=\
    """
Set trigger source (L1* -external input, VME -from program,
Random -internal generator) and frequency in case of Random trigger
    """
    def __init__(self, vb):
      self.vb=vb
      self.f=Toplevel(vb.master)
      self.f.title("setTrigRadio")
      self.trgsrc=myw.MywRadio(self.f, label="trig. source",
        helptext=self.hlptxt,
        items=(("L1A0",0),("L1A1",1),
        ("L1A2",2),("L1A3",3),("VME",4),("Random",5)))
      self.freq=myw.MywRadio(self.f, label="frequency", helptext=self.hlptxt,
        items=(("   1 Hz",0),
        (" 100 Hz",1),
        ("  1 kHz",2),("  5 kHz",3),(" 10 kHz",4),(" 25 kHz",5),
        (" 50 kHz",6),("100 kHz",7)))
      self.exbutton= myw.MywButton(self.f,label="set",cmd=self.setTrig)
    def setTrig(self):
      cmd= "settrig(%s,%s)" % \
        (self.trgsrc.getEntry(),self.freq.getEntry())
      print "cmd:", cmd
      self.vb.io.execute(cmd)
  st(vb)

#--------------------------------------set trigger source and  frequency
def setTrigMenu(vb):
  class st:
    hlptxt=\
    """
Set trigger source (L1* -external input, VME -from program,
Random -internal generator) and frequency in case of Random trigger
    """
    def __init__(self, vb):
      self.vb=vb
      self.tl=Toplevel(vb.master)
      self.f= Frame(self.tl, borderwidth=1); self.f.pack()
      self.tl.title("setTrigMenu")
      lbl= myw.MywEntry(self.f, label="Triger source & frequency",
        helptext=self.hlptxt, defvalue='',side='top')
      self.trgsrc=myw.MywxMenu(self.f, label="trig. source",
        helptext=self.hlptxt, side= 'right',
        items=(("L1A0","0"),("L1A1","1"),
        ("L1A2","2"),("L1A3","3"),("VME","4"),("Random","5")),defaultinx=5)
      self.freq=myw.MywxMenu(self.f, label="frequency", helptext=self.hlptxt,
        defaultinx=4, side= 'right',
        items=(("   1 Hz","0"), (" 100 Hz","1"),
        ("  1 kHz","2"),("  5 kHz","3"),(" 10 kHz","4"),(" 25 kHz","5"),
        (" 50 kHz","6"),("100 kHz","7")))
      self.exbutton= myw.MywButton(self.f,label="set",cmd=self.setTrig,
        side='bottom')
    def setTrig(self):
      cmd= "settrig(%s,%s)" % \
        (self.trgsrc.getEntry(),self.freq.getEntry())
      #print "cmd:", cmd
      #if self.trgsrc.getEntry() == 4:
      #  self.freq. 
      self.vb.io.execute(cmd)
  st(vb)
#--------------------------------------get/set CSR1
def InpSelTiming(vb):
  class st:
    hlptxt=\
    """
- Read CSR1 register
- show current values
- write back (set button)
    """
    def __init__(self, vb):
      self.vb=vb
      # read current CSR1:
      line= string.split(self.vb.io.execute("pcsr1()"),'\n',1)[0]
      try:
        exec('evorb,frequ,bcdel,vmetp,frese,fempt,ffull,exorb,trsrc='+line) in locals()
      except:
        print "wrong line:",line
        return None
      #print evorb,frequ,bcdel,vmetp,frese,fempt,ffull,exorb,trsrc
      #
      self.tl=Toplevel(vb.master)
      self.tl.title("Input selection and Timing")
      #
      self.f1= myw.MywFrame(self.tl)
      lbl= myw.MywEntry(self.f1, label="Counting:",
        helptext=self.hlptxt, defvalue='',side='left')
      cur=evorb
      self.evorb=myw.MywxMenu(self.f1, label="",
        helptext=self.hlptxt, side= 'right',
        items=(("events","0"),("orbits","1")),
        defaultinx=cur)
      #
      cur=trsrc
      self.f5= myw.MywFrame(self.tl)
      #lbl= myw.MywEntry(self.f5, label="Trigger src:",
      #  helptext=self.hlptxt, defvalue='',side='left')
      self.trsrc=myw.MywxMenu(self.f5, label="Trigger src:", helptext=self.hlptxt,
        defaultinx=cur, side= 'right',
        items=( ("L1A0","0"),("L1A1","1"),
        ("L1A2","2"),("L1A3","3"),("VME","4"),("Random","5"),
        ("Calibration","6"), ("Disabled","7")))
      #
      cur=frequ
      self.f2= myw.MywFrame(self.tl)
      lbl= myw.MywEntry(self.f2, label="Frequency:",
        helptext=self.hlptxt, defvalue='',side='left')
      self.frequ=myw.MywxMenu(self.f2, label="", helptext=self.hlptxt,
        defaultinx=cur, side= 'right',
        items=(("   1 Hz","0"), (" 100 Hz","1"),
        ("  1 kHz","2"),("  5 kHz","3"),(" 10 kHz","4"),(" 25 kHz","5"),
        (" 50 kHz","6"),("100 kHz","7")))
      #
      lbl= myw.MywLabel(self.tl, label="BC delay: "+str(bcdel))
      #
      if vmetp==1:
        curlab="VME request pending"
      else:
        curlab="VME OK"
      lbl= myw.MywLabel(self.tl, label= curlab)
      #
      if fempt==1:
        curlab="L1A FIFO empty"
      else:
        curlab="L1A FIFO not empty"
      lbl= myw.MywLabel(self.tl, label= curlab)
      #
      if ffull==1:
        curlab="L1A FIFO full"
      else:
        curlab="L1A FIFO not full"
      lbl= myw.MywLabel(self.tl, label= curlab)
      #
      self.f3= myw.MywFrame(self.tl, borderwidth=3, relief=RAISED); self.f3.pack()
      lbl= myw.MywEntry(self.f3, label="L1A FIFO:",
        helptext=self.hlptxt, defvalue='',side='left')
      cur=frese
      self.frese=myw.MywxMenu(self.f3, label="",
        helptext=self.hlptxt, side= 'right',
        items=(("don't reset","0"),("reset","1")),
        defaultinx=0)
      #
      self.f4= myw.MywFrame(master=self.tl)
      lbl= myw.MywEntry(self.f4, label="Orbit:",
        helptext=self.hlptxt, defvalue='',side='left')
      cur=exorb
      self.exorb=myw.MywxMenu(self.f4, label="",
        helptext=self.hlptxt, side= 'right',
        items=(("external","0"),("internal","1")),
        defaultinx=cur)
      #
      self.okbutton= myw.MywButton(self.tl,label="quit",cmd=self.tl.destroy,
        side='bottom')
      self.exbutton= myw.MywButton(self.tl,label="set CSR1",cmd=self.setCSR1,
        side='bottom')
    def setCSR1(self):
      cmd= "setcsr1(%s,%s,%s,%s,%s)" % \
        (self.evorb.getEntry(),
         self.frequ.getEntry(),
         self.frese.getEntry(),
         self.exorb.getEntry(),
         self.trsrc.getEntry(),
        )
      #print "cmd:", cmd
      #if self.trgsrc.getEntry() == 4:
      #  self.freq. 
      self.vb.io.execute(cmd,applout=None)
  st(vb)

#--------------------------------------send L1/L2
def sendL1L2(vb):
  class st:
    hlptxt=\
    """
- Read CSR1 register
- show current values
- write back (set button)
    """
    def __init__(self, vb):
      self.vb=vb
      self.L1L2set=[0,0]
      tl=Toplevel(vb.master)
      tl.title("Send trigger")
      l12frame= myw.MywFrame(tl,side='top')
      # L1 message
      fl1= myw.MywFrame(l12frame,side='left')
      l1lab=myw.MywLabel(fl1, label="L1 message",side='top')
      self.l1defs=('0x111','0x222','0x333','0x444',
                   '0x555')
      self.l1m=[]
      for i in range(len(self.l1defs)):
        self.l1m.append( myw.MywEntry(fl1, label="w"+str(i)+":",
        defvalue=self.l1defs[i], side='top'))
      exl1= myw.MywButton(fl1,label="sendL1",cmd=self.sendL1,side='bottom')
      # L2 message:
      fl2= myw.MywFrame(l12frame,side='right')
      l1lab=myw.MywLabel(fl2, label="L2 message",side='top')
      self.l2defs=('0xfff','0xfff','0xfff','0xfff',
                   '0xfff','0xfff','0xfff','0xfff')
      self.l2m=[]
      for i in range(len(self.l2defs)):
        self.l2m.append( myw.MywEntry(fl2, label="w"+str(i)+":",
        defvalue=self.l2defs[i], side='top'))
      exl1= myw.MywButton(fl2,label="sendL2",cmd=self.sendL2,
        side='bottom')
      self.trigs= myw.MywEntry(tl,label="# of L1/L2 sequences:",
        defvalue='1',width=10)
      self.extr= myw.MywButton(tl,label="send L1/L2 seq.",
        cmd=self.sendtrigger)
      self.extr.configure(state="disabled")
    def execcmd(self, l12, vals):
      cmd= "sendL"+l12+"M("
      for i in range(len(vals)):
        ent= vals[i].getEntry()
        cmd= cmd+ent+","
      #? cmd=string.rstrip(cmd,',')+')'
      cmd=cmd[0:-1]+')'
      #print "cmd:", cmd
      #if self.trgsrc.getEntry() == 4:
      #  self.freq. 
      self.vb.io.execute(cmd)
    def sendL1(self):
      self.L1L2set[0]=1
      self.execcmd("1", self.l1m)
    def sendL2(self):
      self.L1L2set[1]=1
      if self.L1L2set[0] == 1:
        self.extr.configure(state="normal")
      self.execcmd("2", self.l2m)
    def sendtrigger(self):
      #print "extr:",dir(self)
      ntrig= self.trigs.getEntry()
      cmd= "sendtrigger("+ntrig+")"
      self.vb.io.execute(cmd)
  st(vb)
