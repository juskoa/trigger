#from Tkinter import *
import tkFileDialog
import ctpcfg,scopeab,ssmbrowser,ssmcontrol,counters,testclass
import os   #, os.path, glob, string
import myw
#ctphw= ctpcfg.Ctpconfig()    # the copy of hw registers
ctpmem= None   # initialised in  vmeb/crate.py (Ctpconfig instance)
OpenLoad=tkFileDialog.Open(
  initialdir= os.environ['VMEWORKDIR'] +"/WORK/",
  initialfile='ctp.cfg',
  filetypes=[("CTP config","*.cfg"), ("all files","*")])
OpenSave=tkFileDialog.SaveAs(
  initialdir= os.environ['VMEWORKDIR'] +"/WORK/",
  initialfile='ctp.cfg',
  filetypes=[("CTP config files","*.cfg"), ("all files","*")])

def CTP_Classes(vb):
  #von class Ctpclasses:
  #von  def __init__(self, vb):
  ctpmem.doClasses()
  #von return(Ctpclasses(vb))
def CTP_Clusters(vb):
  ctpmem.showFOs()
  #return
def ScopeAB(vb):
  class Scope:
    def __init__(self, vb):
      self.vb=vb; self.tl=None
      swleds=[]
      if self.tl:
        myw.RiseToplevel(self.tl)
      else:
        self.tl=myw.NewToplevel("CTP scope signals")
      f1= myw.MywFrame(self.tl, side="top")
      f11= myw.MywFrame(f1,relief="flat",side="left")
      f12= myw.MywFrame(f1,relief="flat",side="left")
      f3= myw.MywFrame(self.tl, side="top")
      a=scopeab.ABsig(f11,"A", vb)
      b=scopeab.ABsig(f12,"B", vb)
      scopeab.VmeRW2Scope(f3, vb)
      startallbut= myw.MywButton(f3, "StartAll",
        helptext="""Start SW LEDs for all boards
""", side='left', cmd=self.updateOnOffAll)
      self.swll=[]
      for ix in range(len(scopeab.ABsig.ctpboards)-1):
        # find signame for board ix:
        siga= scopeab.findSigOnBoard(vb, ix, "A")
        #print "sigab2:", siga
        if siga==-1: continue
        sigb= scopeab.findSigOnBoard(vb, ix, "B")
        #print "sigab:", siga, sigb
        aname=scopeab.getSigNameOnBoard(ix, "A", siga)[0]
        bname=scopeab.getSigNameOnBoard(ix, "B", sigb)[0]
        f4= myw.MywFrame(self.tl, side="top")
        self.swll.append(scopeab.SOFTLEDS(self.vb, f4, 
          scopeab.ABsig.ctpboards[ix], activeA= aname, activeB= bname))
        swleds.append(self.swll[-1].updateLabel)
      a.updateSWLED= swleds
      b.updateSWLED= swleds
    def updateOnOffAll(self):
      for swlo in self.swll:
        swlo.updateOnOff(action="start")
  return(Scope(vb))

def Resources(vb):
  ctpmem.showShared()
def SSMbrowser(vb):
  ssmbrowser.main(vb)
def SSMcontrol(vb):
  ssmcontrol.main(vb)
def SaveFile(vb):
  fn= OpenSave.show()
  if fn: ctpmem.save2file(fn)
def LoadFile(vb):
  fn= OpenLoad.show()
  #print "fn:",fn
  if fn: ctpmem.loadfile(fn)
def Readhw(vb):
  ctpmem.readhwall()
def Write2hw(vb):
  ctpmem.writehwall()
def Counters(vb):
  #print "ctp_u.py:Counters"
  cnts=counters.VMEcnts()   # is vb-independent if with shared dmemory

#------------------------------------------ Test Class
def CheckTestClass(vb):
  return(testclass.TestCl(vb))

