# 20.11.2006 testclassLowLevel.py is obsolete now, means 
#    low level (old) approach, not using setswtrig/startswtrig
# testclass.py -new approach using setswtrig/startswtrig
from Tkinter import *
#import os, os.path, glob, string
import myw

class TestCl:
  def __init__(self, vb):
    self.vb=vb
    self.tl=None
    self.getbs()
    #self.roc=0
    warning="""Do not forget to set Cal. flag and ROC bits on right FO
in FOs/Cluster window
"""
    print warning; self.vb.io.write(warning)
    if self.tl:
      myw.RiseToplevel(self.tl)
    else:
      #self.tl=Toplevel(vb.master)
      self.tl=myw.NewToplevel("Test class")
    self.fbusy= myw.MywFrame(self.tl,side="top")   # busys frame
    self.fbcnbcm= myw.MywFrame(self.tl,side="top") # entries frame
    self.fcq= myw.MywFrame(self.tl,side="top")   # check/quit
    self.busys=myw.MywBits(self.fbusy,label="RO flags",
      defval= self.tcstatus, 
      helptext="""RO Flags:
explanation later...
""",
    bits=[
        ("ClusterBUSY",0),
        ("PPreq",0),
        ("L0req",0),
        ("L0ack",0),
        ("ClassBUSY",0),
        ("L1ack",0),
        ("L2Rack",0),
        ("L2Aack",0)],
      side="top")
    self.rwflags=myw.MywBits(self.fbusy,label="RW flags",
      defval= self.tcset, cmd=self.modRWflags,
      helptext="""RW Flags:
Choosen Flgas (red) are set to '1' which means:
  -> don't care for VETOs flags (bcm*, PFveto)
  -> CAL or SYNCH triggers for 'non-veto' flags
Notes:
1. When CAL flag is set, SYNCH flag is set automatically too and
   PP is generated ib BC 128 (the final exact shift is set on TTCvi)
2. BCmask (bcm1..bcm4) flags are ignored for SYNCH triggers -i.e.
   SYNCH triggers can be issued in any BC
3. The FO and INT boards are not programmed. For CAL trigger,
   corresponding FO should be flagged with CalFlag and Test class (T)
   has to be set for detector(s) which are supposed to get SW trigger.
   INT board: word INT_TC_SET should be correctly set with
              RoC and CalFlag for CTP readout.
""",
      bits=[("bc0",1), ("bc1",1), ("bc2",1), ("bc3",1), ("bc4",1),
        ("bc5",1), ("bc6",1), ("bc7",1), ("bc8",1), ("bc9",1),
        ("bc10",1), ("bc11",1),
        "SYNCH  -off ->ASYNCHRONOUS trigger",
        "CAL    -off ->not calibration trigger",
        "bcm1", "bcm2", "bcm3", "bcm4",
        "L0 PFveto -P/F protection veto",
        "L1 PFveto -P/F protection veto",
        "L2 PFveto -P/F protection veto",],
      side="top")
    self.bbcn= myw.MywEntry(self.fbcnbcm,label="BC number",
      helptext="""BC number. Relevant only for synchronous triggers.
Valid values are: 0..3563
3563:last BC in the Orbit
0:   1st BC in the Orbit. If L0_BCOFFSET is set to 0, it corresponds
     to BC 3 when counted from Orbit signal. 
     Or, set L0_BCOFFSET=3562 to have numbering conforming with Orbit signal.
""",
      side="left", bind='lr', cmdlabel=self.bcmod,
      defvalue= str(self.tcset&0xfff))
      # Commented, FOs are managed in FOs/Clusters window.
    self.broc= """myw.MywEntry(self.fbcnbcm,label="ROC",
      helptext="4 ROC bits. The combinations with most significant one (0x8) set to 1 are reserved for CTP use.
      side="left", bind='lr', cmdlabel=self.brocmod,
      defvalue= str(self.roc&0xf))"""
    self.detectorsw=myw.MywBits(self.fbusy,label="Detectors",
      defval= self.detectors, cmd=self.moddetectors,
      helptext="""Detectors: 1-24, (DAQdet numbers). The set choosen here
is writen to L2_TCSET word. BUSY board is not programmed.
""",
      bits=["1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24"],
      side="top")
    self.butstart=myw.MywButton(self.fcq, label="Generate\nsw. trigger",
      helptext="""Clear flags and generate SW trigger.
Only L0_TCCLEAR (clear flags) and L0_TCSET (generate) written.""",
      side="left",cmd=self.gensw)
    self.butssmstart=myw.MywButton(self.fcq, label="Start SSM+\nsw. trigger",
      helptext="""Clear flags, start L0-SSM (27ms) in outmon mode, 
and generate SW trigger""",
      side="left",cmd= myw.curry(self.gensw, "1"))
    self.butcheck=myw.MywButton(self.fcq, label="Check\nflags",
      helptext="Check L0_TCSTATUS register",side="left",cmd=self.checkbs)
    self.butclear=myw.MywButton(self.fcq, label="Clear\nflags",
      helptext="Clear L0_TCSTATUS register",side="left",cmd=self.clear)
    self.butquit=myw.MywButton(self.fcq, label="Quit",
      helptext="QUIT this window", side="right",cmd=self.quitcb)
  def modRWflags(self):
    nval= self.rwflags.getEntry()
    if ((nval ^ self.tcset) & 0x1fffff) == 0: # +2 bits (L0,L1,L2 boards)
      return         # no change in flags
    #print "modRWflags.rwflags:",hex(nval)
    # set automatically SYNCHRONOUS for CALIB. triggers
    if (nval&0x2000)==0x2000:
      nval= nval | 0x1000      # set SYNC flag
      self.rwflags.setEntry(nval)
    calflag= (nval ^ self.tcset) & 0x2000
    # Commented, FOs are managed in FOs/Clusters window.
    #if calflag != 0: # cal. flag changed
    #  calflag= nval & 0x2000
    #  self.vb.io.execute("setFOrocs("+hex(calflag)+','+str(self.roc)+")")
    nbc= self.bbcn.getEntryBin() & 0xfff
    nval= nval | nbc
    #print "modRWflags:tcset:%x new value:%x"%(oval, nval)
    self.tcset= nval
    self.vb.io.execute("setTCSET("+hex(self.tcset)+','+str(self.detectors)+")")
    self.checkbs()
  def getbs(self):
    ab= self.vb.io.execute("getTCSTATUS()",applout='<>')
    self.tcstatus= eval(ab[0])
    ab= self.vb.io.execute("getTCSET()", applout='<>')
    self.tcset= eval(ab[0])
    ab= self.vb.io.execute("vmeopr32(L2_TCSET)")[:-1]
    self.detectors= eval(ab) & 0xffffff
    #print "getbs:0x%x 0x%x"%(self.tcstatus, self.tcset)
  def bcmod(self,event=None):
    nbc= self.bbcn.getEntryBin() & 0xfff
    if nbc != self.tcset&0xfff:
      nval= self.rwflags.getEntry()
      self.tcset= (self.tcset&0xfffff000) | nbc
      self.vb.io.execute("setTCSET("+hex(self.tcset)+','+str(self.detectors)+")")
  #def brocmod(self,event=None):
    #  self.roc= self.broc.getEntryBin()&0xfff
    #  #print "brocmod:",self.roc
    #  calflag= self.tcset & 0x2000
    #  self.vb.io.execute("setFOrocs("+hex(calflag)+','+str(self.roc)+")")
  def moddetectors(self,event=None):
    self.detectors= self.detectorsw.getEntryBin()
    print "moddetectors:",self.detectors
    self.vb.io.execute("setTCSET("+hex(self.tcset)+','+str(self.detectors)+")")
  def gensw(self,ssm=None):
    if ssm==None: ssm='0'
    status=self.vb.io.execute("swtrigger("+ssm+")", applout='<>')
    #self.checkbs()
    self.tcstatus= eval(status[0])
    self.busys.setEntry(self.tcstatus)
  def checkbs(self):
    self.getbs()
    self.busys.setEntry(self.tcstatus)
  def clear(self):
    #self.vb.io.execute("vmeopw32(L0_TCCLEAR,0xffffffff)")
    self.vb.io.execute("clearTC()")
    self.checkbs()
  def quitcb(self):
    self.tl.destroy()
#def PFcircuits(vb):
#  ctpmem.showPF()

if __name__ == "__main__":
    #main()
  master=Tk()
  #ABsig(master,"B",vb)
  master.mainloop()

