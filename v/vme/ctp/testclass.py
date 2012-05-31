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
    self.trigtype='c'
    self.ntrigs=1; self.ntrigs_attempts= 0; self.afterid= None
    self.bcmask=0
    self.t2tms=60000  # 1 minute
    self.roc=0x7
    #print warning; self.vb.io.write(warning)
    if self.tl:
      myw.RiseToplevel(self.tl)
    else:
      #self.tl=Toplevel(vb.master)
      self.tl=myw.NewToplevel("Test class")
    self.rodets= myw.MywFrame(self.tl,side="top")   # busys frame
    self.fbcnbcm= myw.MywFrame(self.tl,side="top") # entries frame
    self.fcq= myw.MywFrame(self.tl,side="top")   # check/quit
    self.ttb= myw.MywxMenu(self.rodets, side=LEFT, defaultinx=2,
      label='Trigger type', items=[("Async. trigger","a"),
      ("Sync. trigger","s"),("Calibration trigger","c")],
      cmd=self.ttaction, helptext="""Trigger type.
""")

    self.busys=myw.MywBits(self.rodets,label="RO flags",
      defval= self.tcstatus, 
      helptext="""RO Flags:
The status bits of L*_TCSTATUS word:
ClusterBUSY L0 test cluster is busy, if participating detectors
               are busy or CTP is busy
PPreq       L0
L0req       L0
L0ack       L0
ClassBUSY   L0 -"sw trigger in progress". Cleared by sw 
                before each sw trigger.
L1ack       L1
L2Rack      L2
L2ack       L2
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
      side=LEFT)
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
    self.bbcn= myw.MywEntry(self.fbcnbcm,label="BC/mask",
      helptext="""BC number or BCmask.
BC number: relevant only for synchronous triggers.
   Valid values are: 0..3563
   3563:last BC in the Orbit
   0:   1st BC in the Orbit. If L0_BCOFFSET is set to 0, it corresponds
        to BC 3 when counted from Orbit signal.  Or, 
        set L0_BCOFFSET=3562 to have the numbering conforming 
        with Orbit signal.

   0xfff: set this value for Calibration triggers if you want
        to read it from detector's database (i.e. LTU defaults).
        By default it is 3011, but T0 uses different settings

BCmask: Valid for asynchr. triggers. 0: don't use any mask
        1: use BCmask1
        8: use BCmask4
""",
      side="left", bind='lr', cmdlabel=self.bcmod,
      defvalue= "0xfff")
#      defvalue= str(self.tcset&0xfff))
    self.ntrigsb= myw.MywEntry(self.fbcnbcm,label="N of trigers:",
      helptext="""Number of triggers.
""",
      side="left", bind='lr', cmdlabel=self.nmod,
      defvalue= self.ntrigs)
      # 
    self.t2tmsb= myw.MywEntry(self.fbcnbcm,label="ms between triggers:",
      helptext="""time between 2 triggers in milseconds.
""",
      side="left", bind='lr', cmdlabel=self.t2tmsmod,
      defvalue= str(self.t2tms))
      # 
    self.broc= myw.MywEntry(self.fbcnbcm,label="ROC:",
      helptext="""4 ROC bits. The combinations with most 
significant one (0x8) set to 1 are reserved for CTP use.""",
      side="left", bind='lr', cmdlabel=self.brocmod,
      defvalue= self.roc)
    self.detectorsw=myw.MywBits(self.rodets,label="Detectors",
      defval= self.detectors, cmd=self.moddetectors,
      helptext="""Detectors: 0-23, (ECS/DAQ numbers). The set choosen here
is writen to L2_TCSET word. BUSY board is programmed according
to current cabling (in VALID.LTUS file).
""",
      bits=["0=SPD","1=SDD","2=SSD","3=TPC","4=TRD","5=TOF","6=HMPID","7=PHOS","8=CPV","9=PMD","10=MUON_TRK","11=MUON_TRG","12=FMD","13=T0","14=V0","15=ZDC","16=ACORDE","17TRG","18=EMCAL","19=DAQ","20","21","22","23"],
      side=LEFT)
    self.butstart=myw.MywButton(self.fcq, label="Generate\nsw. trigger(s)",
      helptext="""GenSwtrg() routine is invoked to set and
generate SW triggers after pressing this button. 
If this window is closed, the generation of triggers is interrupted.
The settings are set with each trigger: i.e.
the modifications of entry fields in this widget will be valid for
next trigger(s).
After the generation of required sw triggers, the message
appears in log window about the number of all attempts to generate trigger
and successful triggers. 
For each sw trigger all the CTP boards are programmed using current cabling
from VALID.LTUS. NB: if there is 'busy-input-connection' declared
in VALID.LTUS, and the corresponding LTU is 'busy', the SW triger
will be rejected at L0 level.
""",
      side="left",cmd=self.gensw)
    #self.butssmstart=myw.MywButton(self.fcq, label="Start SSM+\nsw. trigger",
    #  helptext="""Clear flags, start L0-SSM (27ms) in outmon mode, 
#and generate SW trigger""",
    #  side="left",cmd= myw.curry(self.gensw, "1"))
    self.butcheck=myw.MywButton(self.fcq, label="Check\nflags",
      helptext="""Check L?_TCSTATUS/L?_TCSET registers
""",side="left",cmd=self.checkbs)
    self.butclear=myw.MywButton(self.fcq, label="Clear\nflags",
      helptext="Clear L0_TCSTATUS register",side="left",cmd=self.clear)
    self.butquit=myw.MywButton(self.fcq, label="Quit",
      helptext="QUIT this window", side="right",cmd=self.quitcb)
  def getbs(self):
    ab= self.vb.io.execute("getTCSTATUS()",applout='<>')
    self.tcstatus= eval(ab[0])
    ab= self.vb.io.execute("getTCSET()", applout='<>')
    self.tcset= eval(ab[0])
    ab= self.vb.io.execute("vmeopr32(L2_TCSET)")[:-1]
    self.detectors= eval(ab) & 0xfdffff   # never CTP (bit17)
    # in spite of having it (DAQ wants to have this bit on when
    # our readout is on -L2TCSET bit 17 is remaining from SOD/EOD).
    #print "getbs:0x%x 0x%x"%(self.tcstatus, self.tcset)
  def checkbs(self):
    self.getbs()
    self.busys.setEntry(self.tcstatus)
  def bcmod(self,event=None):
    self.bcmask= self.bbcn.getEntryBin()
  def nmod(self,event=None):
    self.ntrigs= self.ntrigsb.getEntryBin()
  def t2tmsmod(self,event=None):
    self.t2tms= self.t2tmsb.getEntryBin()
  def ttaction(self,ttbinst,ix):
    self.trigtype= ttbinst.getEntry()
  def brocmod(self,event=None):
    self.roc= self.broc.getEntryBin()
    #print "brocmod:",self.roc
  def moddetectors(self,event=None):
    self.detectors= self.detectorsw.getEntryBin()
    #print "moddetectors_hex:",hex(self.detectors)
    #self.vb.io.execute("setTCSET("+hex(self.tcset)+','+str(self.detectors)+")")
  def gensw1(self,ssm=None):
    cmd="GenSwtrg(1, '%s', %d, %d, 0x%x, 2)"%(self.trigtype,
      self.roc, self.bcmask, self.detectors)
    status=self.vb.io.execute(cmd, log="out",applout='<>')
    #status=1; print "gensw:",cmd
    #print "Successful triggers:", status[0], type(status[0])
    self.checkbs()
    self.ntrigs_attempts= self.ntrigs_attempts + 1
    if status[0]=="1": self.ntrigs_ok= self.ntrigs_ok+1
    if status[0]=="12345678": 
      msg="Cal. triggers stopped, some of cal. detectors not in global run(s)\n"
    else:
      msg="All attepmts:%d Successful:%d\n"%(self.ntrigs_attempts,
      self.ntrigs_ok)
    self.vb.io.write(msg)
    if (self.ntrigs_attempts>= self.ntrigs) or status[0]=="12345678":
      self.butstart.enable()
      self.afterid=None
    else:
      self.afterid= self.tl.after(self.t2tms, self.gensw1)
  def gensw(self,ssm=None):
    if self.bcmask==0xfff:
      # find CALIBRATION_BC, from corresponding ltuproxy
      cmd="getCALIBBC2(0x%x)"%(self.detectors)
      rc=self.vb.io.execute(cmd, log="out",applout='<>')
      #print "gensw:",rc
      if rc[0]=='-1':
        msg= 'Cannot contact ltuproxy, using 3011\n'
        self.bcmask=3011
      else:
        msg= "Using CALIBRATION_BC:%s\n"%rc
        self.bcmask=int(rc[0])
      self.vb.io.write(msg)
    if ssm==None: ssm='0'
    self.butstart.disable()
    self.ntrigs_attempts= 0
    self.ntrigs_ok= 0
    if self.t2tms==0:
      self.vb.io.write("WARNING: other sw triggers (i.e. SOD/EOD too) not allowed")
      cmd="GenSwtrg(%d, '%s', %d, %d, 0x%x, 2)"%(self.ntrigs,self.trigtype,
        self.roc, self.bcmask, self.detectors)
      status=self.vb.io.execute(cmd, log="out",applout='<>')
      if status[0]=="12345678": 
        msg="Cal. triggers stopped, some of cal. detectors are not in global run(s)\n"
      else:
        msg="All attepmts: %d Successful:%s\n"%(self.ntrigs, status[0])
      self.vb.io.write(msg)
      self.butstart.enable()
    else:
      self.gensw1()
  def clear(self):
    #self.vb.io.execute("vmeopw32(L0_TCCLEAR,0xffffffff)")
    self.vb.io.execute("clearTC()")
    self.checkbs()
  def quitcb(self):
    if self.afterid:
      self.tl.after_cancel(self.afterid)
    self.tl.destroy()
#def PFcircuits(vb):
#  ctpmem.showPF()

if __name__ == "__main__":
    #main()
  master=Tk()
  #ABsig(master,"B",vb)
  master.mainloop()

