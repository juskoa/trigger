#!/usr/bin/python
from Tkinter import *
import tkFileDialog
import string, os, types, time, threading
import os.path, glob
import myw
import counters

# name, rel.address, CGT, help
#
TTCITcnts={
"L0__OM":(0,"C","L0 triggers seen by Online Monitor"),
"L1__OM":(1,"C","L1 triggers seen by OM"),
"L1m__OM":(2,"C","L1m messages seen by OM"),
"L2a__OM":(3,"C","L2a messages seen by OM"),
"L2r__OM":(4,"C","L2r messages seen by OM"),
"L0S_err__OM":(5,"C","Surplus L0 by OM"),
"L1S_err__OM":(6,"C","Surplus L1 by OM"),
"L1m_missing__OM":(7,"C","Missing L1m by OM"),
"L1m_surplus__OM":(8,"C","Surplus L1m by OM"),
"L1m_incomplete__OM":(9,"C","Incomplete L1m by OM"),
"L1m_data_error__OM":(10,"C","L1m data error by OM"),
"L2m_missing__OM":(11,"C","L2m missing by OM"),
"L2m_surplus__OM":(12,"C","Surplus L2m by OM"),
"L2m_incomplete__OM":(13,"C","Incomplete L2m by OM"),
"L2m_data_error__OM":(14,"C","L2m data error by OM"),
"CALIBRATION_err__OM":(15,"C","Calibration error by OM"),
"BC_mismatch__OM":(16,"C","BC ID mismatch by OM"),
"PP_counter":(17,"C","PP counter")}

# "L0__SM":(17,"C","L0 triggers seen by Software Monitor"),
# "L1__SM":(18,"C","L1 triggers seen by SM"),
# "L1m__SM":(19,"C","L1m messages seen by SM"),
# "L2a__SM":(20,"C","L2a messages seen by SM"),
# "L2r__SM":(21,"C","L2e messages seen by SM"),
# "UNKNOWN__SM":(22,"C","Unknown item seen by SM"),
# "ACCEPTED__SM":(23,"C","L0-L1-L1m-L2a accepted triggers seen by SM"),
# "L1_rejected__SM":(24,"C","Triggers rejected by L1 absence seen by SM"),
# "L2_rejected__SM":(25,"C","Triggers rejecred by L2r seen by SM"),
# "RoI__SM":(26,"C","RoI messages seen by SM"),
# "L0_L1_fakes__SM":(27,"C","L0-L1 fake combinations seen by SM"),
# "L0S_err__SM":(28,"C","Surplus L0 seen by SM"),
# "L1S_err__SM":(29,"C","Surplus L1 seen by SM"),
# "L1T_err__SM":(30,"C","L0-L1 time violations seen by SM"),
# "L1Mo_err__SM":(31,"C","No L1m sent wihin programmable window seen by SM"),
# "L1F_err__SM":(32,"C","L1m format error seen by SM"),
# "L2T_err__SM":(33,"C","No L2a/L2r sent within pr..ble window seen by SM"),
# "L2TS_err__SM":(34,"C","L2a/L2r sent, but no L1, seen by SM"),
# "L2F_err__SM":(35,"C","L2a/L2r format error seen by SM"),
# "BC_mismatch_err__SM":(36,"C","BC ID mismatch seen by SM"),
# "RoIF_err__SM":(37,"C","RoI message format error seen by SM"),
# "RoIS_err__SM":(38,"C","Surplus RoI message without L1 signal seen by SM"),
# "RoIT_err__SM":(39,"C","RoI timeout seen by SM")}


class VMEcnts(counters.CTPcnts):
  def __init__(self, ctpltu=None,tlw=None):
    #self.ctpltu=ctpltu
    #print "VMECnts__init__:",ctpltu,self.defcounters
    #ctpltu: None -> ctp
    #        dictionary -> LTU or LVDST
    self.perrep=0     # to be compliant with MultRegs
    self.mrmaster=tlw
    self.nx=0
    self.ctpltu= ctpltu
    self.saveconf = None
    #if self.mrmaster:
    #  myw.RiseToplevel(self.mrmaster); return
    #else:
    tllabel="TTCit counters"
    self.numberofcounters=len(ctpltu)
    deffile= os.environ['VMECFDIR'] +"/WORK/ttcit.counters"
    cmdbuttons= [
      ("Read", self.allread),
      ("Increments", self.changeacc),
      ("Periodic read", self.perRead),
      ("Clear counters", self.clearCounters)
      ]
    self.mrmaster=myw.NewToplevel(tllabel,
      self.hidemrm, tlw=self.mrmaster)
    self.defcounters= deffile
    #
    self.addw=None   #ADDREM TopLevel window: 'adding/removing' counters
    self.regsframe= myw.MywFrame(self.mrmaster)   #Shown counters (self.regs)
    # counters in more columns:
    self.regsframes=[]
    self.regsframe.bind("<Destroy>",self.hideregsframe)
    self.cmdframe= myw.MywFrame(self.mrmaster)
    self.showaccrual=1   # 0: show absolute vals (1:accruals)
    # regs: contains 1 item: pointer to VME/SHM Counter instance
    #       for all entries in self.regsframe
    # cnt_number: 0.. -order number of counter (0..158 for L0 counters)
    #             in array got from getCounters()
    self.regs=[]   # list of shown counters in self.regsframe
    self.allregs=[]   # last values read by getCounters() (only VMEcnts)
    for i in range(self.numberofcounters):
      self.allregs.append(" ")
    self.butts=myw.MywHButtons(self.cmdframe, 
      cmdbuttons , helptext="""
Read           -read counters from hw and update fields of all the counters
                (i.e. counters not shown are read too)
Increments     -showing increments
Abs. values    -showing absolute counter values 
Periodic read  -repeat updating once per second (roughly) 100 times
""")
       #("Quit", self.mrmaster.destroy)])
    self.normalcolor=self.butts.buttons[2].cget('bg'),\
      self.butts.buttons[2].cget('activebackground')
    #print 'perRead.normalcolor:',self.normalcolor
    #self.addReg(("BUSY_local_orbit"))
    if ctpltu==None:
      try:
        cf= open(self.defcounters,"r")
      except IOError:
        cf=None
      if cf:
        for line in cf.readlines():
          self.addReg(line[:-1])
        cf.close()
    else:
      keylist = ["L0__OM", "L1__OM", "L1m__OM", "L2a__OM", "L2r__OM",
      "L0S_err__OM", "L1S_err__OM", "L1m_missing__OM", "L1m_surplus__OM",
      "L1m_incomplete__OM", "L1m_data_error__OM", "L2m_missing__OM",
      "L2m_surplus__OM", "L2m_incomplete__OM", "L2m_data_error__OM",
      "CALIBRATION_err__OM", "BC_mismatch__OM", "PP_counter" ]
#      "L0__SM", "L1__SM", "L1m__SM", "L2a__SM", "L2r__SM", "UNKNOWN__SM",
#      "ACCEPTED__SM", "L1_rejected__SM", "L2_rejected__SM", "RoI__SM",
#      "L0_L1_fakes__SM", "L0S_err__SM", "L1S_err__SM", "L1T_err__SM",
#      "L1Mo_err__SM", "L1F_err__SM", "L2T_err__SM", "L2TS_err__SM",
#      "L2F_err__SM", "BC_mismatch_err__SM",
#      "RoIF_err__SM", "RoIS_err__SM", "RoIT_err__SM" ]
      for lc in keylist:
        if lc in ctpltu:
          # print "Key ",lc,"  exists"
          self.addLTUReg(lc)
        else:
          print "Key ",lc,"  missing"
    self.init()
  def init(self):
    self.printnames=None
  def finish(self):
    pass
  def allread(self):
    cmd2=""
    #for i in range(len(self.regs)):
    #  addr= self.regs[i][1]
    #  cmd2=cmd2+','+str(addr)
    #print 'allread  :', self.ctpltu, self.numberofcounters
    cmd="getCounters("+str(self.numberofcounters)+","+str(self.showaccrual)+")"
    thdrn=myw.vbexec.vbinst.io.execute(cmd,ff=self.doout)
    #print 'CTPcounters.cmd:',cmd,thdrn
    myw.vbexec.vbinst.io.thds[thdrn].waitcmdw()
    #print "allread finished"
  def doout(self,allvals):
    #print "allread3:",allvals,':'
    vlist=string.split(allvals,'\n'); vll= len(vlist)-1
    if vll != len(self.allregs):
      myw.errorprint(self, "doout: %d != %d"%(vll,len(self.allregs)))
    #print "allread2:",vll,len(self.allregs),"\n", vlist[706:724]
    for i in range(vll):
      self.allregs[i]= vlist[i]
    for i in range(len(self.regs)):
      prevval= self.regs[i].cntentry.getEntryHex()
      cntnum= self.regs[i].cntaddr
      newval= self.allregs[cntnum]
      #print "allread:",prevval,type(prevval),vlist[i],cntnum
      #print "allread:",prevval,type(prevval), newval, type(newval)
      if prevval != newval:
        self.regs[i].cntentry.setColor(myw.COLOR_VALCHANGED)
        self.regs[i].cntentry.setEntry(newval)
      else:
        self.regs[i].cntentry.setColor()


def TTCitCounters(vb):
  st= VMEcnts(TTCITcnts)
  return

#--------------------------- Scope outputs selection:
def Scope_Signals(vb):
  class st:
    def __init__(self, vb):
      self.vb=vb
      self.tl=myw.NewToplevel("Oscilloscope signals selection")
      ab=self.vb.io.execute("ScopeGet_AB(2)")[:-1]
      aa = eval(ab) / 1000;
      bb = eval(ab) % 1000;
      # print "ab:",ab,"   aa = ",aa,"  bb = ",bb
      if aa == 100:
        aa = 20
      if bb == 100:
        bb = 20
      itemsA=(
        ("On-board clock","0",self.selectAB),
        ("clk40des1 from TTCrx chip","1",self.selectAB),
        ("clk40des1 delayed inside FPGA","2",self.selectAB),
        ("BCNT reset from TTCrx chip","3",self.selectAB),
        ("L0 from LVDS cable","4",self.selectAB),
        ("L1 accept from TTC Rx","5",self.selectAB),
        ("Data out strobe","6",self.selectAB),
        ("Data bit 0","7",self.selectAB),
        ("TTC address bit 0","8",self.selectAB),
        ("Bunch Counter strobe (TTC Rx)","9",self.selectAB),
        ("Bit 0 of Bunch counter (TTC Rx)","10",self.selectAB),
        ("Reset TTC Rx","11",self.selectAB),
        ("Reset for Front End electronics","12",self.selectAB),
        ("Serial B channel","13",self.selectAB),
        ("TTC Ready","14",self.selectAB),
        ("Clock40Des2 from TTCrx","15",self.selectAB),
        ("Clock40","16",self.selectAB),
        ("QPLL locked","17",self.selectAB),
        ("PrePulse","18",self.selectAB),
        ("Broadcast status strobe 1","19",self.selectAB),
        ("not selected","20",self.selectAB))
      self.selA= myw.MywxMenu(self.tl, label='A:',
        helptext="""Scope A: """,
        defaultinx=aa, side=TOP, items=itemsA)
      itemsB=(
        ("On-board clock","0",self.selectAB),
        ("clk40des1 from TTCrx chip","1",self.selectAB),
        ("clk40des1 delayed inside FPGA","2",self.selectAB),
        ("BCNT reset from TTCrx chip","3",self.selectAB),
        ("L0 from LVDS cable","4",self.selectAB),
        ("L1 accept from TTC Rx","5",self.selectAB),
        ("Data out strobe","6",self.selectAB),
        ("Data bit 0","7",self.selectAB),
        ("TTC address bit 0","8",self.selectAB),
        ("Bunch Counter strobe (TTC Rx)","9",self.selectAB),
        ("Bit 0 of Bunch counter (TTC Rx)","10",self.selectAB),
        ("Reset TTC Rx","11",self.selectAB),
        ("Reset for Front End electronics","12",self.selectAB),
        ("Serial B channel","13",self.selectAB),
        ("TTC Ready","14",self.selectAB),
        ("Clock40Des2 from TTCrx","15",self.selectAB),
        ("Clock40","16",self.selectAB),
        ("QPLL locked","17",self.selectAB),
        ("PrePulse","18",self.selectAB),
        ("Broadcast status strobe 1","19",self.selectAB),
        ("not selected","20",self.selectAB))
      self.selB= myw.MywxMenu(self.tl, label='B:',
        helptext="""Scope B: """,
        defaultinx=bb, side=TOP, items=itemsB)
      f1= myw.MywFrame(self.tl,side=TOP)
    def selectAB(self):
      self.vb.io.execute("ScopeSelect_AB("+ self.selA.getEntry()+","+
        self.selB.getEntry()+")", "out","no")
  return(st(vb))


