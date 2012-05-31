# 13.9. 2005 - Scope outputs selection for CTP boards
# 15.9. 2006 INT board added
from Tkinter import *
#import os, os.path, glob, string
import myw
def findSigOnBoard(vb, boardix, ab):
  """
  boardix: index into ABsig.ctpboards
  ab: "A" or "B"
  rc: index into corresponding ABsig.signals dictionary or
      -1: board not in the crate 
  """
  sitxt=vb.io.execute("getScopeSignal("+str(boardix)+","+
  str(ord(ab))+")", log="out", applout="<>")[0]
  esi=eval(sitxt)
  if esi== -1: return esi
  esi=esi & 0x111f #Abis:0x1000 or Bbis:0x100 signal: 0x1f)
  # here, perhaps it would be better to search through
  # ABsig.signals[self.boardsig]. This would mean:
  # - not all items in that array would be necessary (now 
  #   0..23,24,...  all the items have to be present
  # - "1" or "0x1001" -should be codded as numbers (not strings)
  if esi>0x1f:            # bis signal  
    esi= (esi & 0x1f) + 32
  #print "esi:", esi
  return esi
def getSigNameOnBoard(boardix, ab, sigix):
  boardn= ABsig.ctpboards[boardix]
  if boardn[:2]=='fo':
    boardsig= 'fo'+ab
  else:
    boardsig= boardn+ab
  signame= ABsig.signals[boardsig][sigix]
  #print "getSigNameOnBoard:", boardn, boardsig, sigix, "->", signame
  return signame
def getixboard(board):
  for ix in range(len(ABsig.ctpboards)):
    if board==ABsig.ctpboards[ix]: 
      return ix          # pointer to ctpboards
  return None

class ABsig:
  ctpboards=["busy","l0","l1","l2","int",
    "fo1","fo2","fo3","fo4","fo5","fo6",
    "not selected"]
  # in signals: all items have to be present (no holes)
  signals={
    "foA":[ ["bp_bc","0"],
     ["bp_orbit","1"], ["bp_PP","2"], ["l0clstt","3"], ["l0clst1","4"],
     ["l0clst2","5"], ["l0clst3","6"], ["l0clst4","7"], ["l0clst5","8"],
     ["l0clst6","9"], ["l1clstt","10"], ["l1clst1","11"], ["l1clst2","12"],
     ["l1clst3","13"], ["l1clst4","14"], ["l1clst5","15"], ["l1clst6","16"],
     ["l1str(25ns,1bc delay)","17"], ["l2data1","18"], ["l2data2","19"], 
     ["out_l2strobe-1","20"], ["out_l2strobe-2","21"], 
     ["out_l2strobe-3","22"], ["out_l2strobe-4","23"],
     ["DelayedL1Cluster","24"], ["VMEstrobe","25"], ["L2str+60InputData","26"]],
    "foB":[
     ["bc fromPLL","0"], ["out_orbit-1","1"], ["out_prepulse-1","2"],
     ["out_prepulse-2","3"], ["out_prepulse-3","4"], ["out_prepulse-4","5"],
     ["out_l0-1","6"], ["out_l0-2","7"], ["out_l0-3","8"],
     ["out_l0-4","9"], ["out_l1-1","10"], ["out_l1-2","11"],
     ["out_l1-3","12"], ["out_l1-4","13"], ["out_l1data-1","14"],
     ["out_l1data-2","15"], ["out_l1data-3","16"], ["out_l1data-4","17"],
     ["l1data","18"], ["l2strobe(25ns,1bc delay)","19"], ["out_l2data-1","20"],
     ["out_l2data-2","21"], ["out_l2data-3","22"], ["out_l2data-4","23"],
     ["AnyL1Cluster","24"], ["VMEwriteStr","25"], ["VMEreadStr","26"],
     ["L1strobe+48InputData","27"], ["GND","28"], ["GND","27"], ["GND","29"], ["GND","30"], 
     ["GND","31"] ],
    "l0A":[
     ["pll_bc","0"], ["orbit","1"], ["byclstt","2"], ["byclst1","3"],
     ["byclst2","4"], ["byclst3","5"], ["byclst4","6"], ["byclst5","7"],
     ["byclst6","8"], ["fun1","9"], ["fun2","10"], ["int1","11"],
     ["int2","12"], ["int_test","13"], ["scaled_bc1","14"], ["scaled_bc2","15"],
     ["pf1","16"], ["pf2","17"], ["pf3","18"], ["pf4","19"],
     ["pf5(testclass)","20"], ["adc_in","21"], 
     ["deadtime","22"], ["strobe","23"],
     ["CLASS48afterVetos","24"], ["CLASS28afterVetos","25"], 
     ["TestClBUSY","26"], ["PPrequest","27"],
     ["L0request","28"], ["L0ack","29"], ["L0strobe(25ns)","30"], ["L0f3","31"],
     ["l0-i1","0x1000"], ["l0-i2","0x1001"], ["l0-i3","0x1002"],
     ["l0-i4","0x1003"], ["l0-i5","0x1004"], ["l0-i6","0x1005"],
     ["l0-i7","0x1006"], ["l0-i8","0x1007"], ["l0-i9","0x1008"],
     ["l0-i10","0x1009"], ["l0-i11","0x100a"], ["l0-i12","0x100b"],
     ["l0-i13","0x100c"], ["l0-i14","0x100d"], ["l0-i15","0x100e"],
     ["l0-i16","0x100f"], ["l0-i17","0x1010"], ["l0-i18","0x1011"],
     ["l0-i19","0x1012"], ["l0-i20","0x1013"], ["l0-i21","0x1014"],
     ["l0-i22","0x1015"], ["l0-i23","0x1016"], ["l0-i24","0x1017"]
                 ],
    "l0B":[ ["bcAfterPLL","0"],
                  ["orbit","1"],
                  ["l0clstt","2"],
                  ["l0clst1","3"],
                  ["l0clst2","4"],
                  ["l0clst3","5"],
                  ["l0clst4","6"],
                  ["l0clst5","7"],
                  ["l0clst6","8"],
                  ["l0strobe+24bitsInputData","9"],
                  ["l0data","10"],
                  ["prepulse","11"],
                  ["rndtrg1","12"],
                  ["rndtrg2","13"],
                  ["BCMASK1","14"],
                  ["BCMASK2","15"],
                  ["BCMASK3","16"],
                  ["BCMASK4","17"],
                  ["SWtrigger(25ns)","18"],
                  ["GND","19"],
                  ["GND","20"],
                  ["bc_count offset","21"], # was (13.3.2008): ["bc_ff","21"],
                  ["strobe & write","22"],
                  ["strobe & !write","23"],
                  ["CLASS48beforeVetos","24"],
                  ["CLASS28beforeVetos","25"],
                  ["Class28rate","26"],
                  ["CLASS28mask","27"],
                  ["INTa","28"],
                  ["INTb","29"],
                  ["DelaydINTbeforeDelay","30"],
                  ["GND","31"],
                  ["l0-i1s","0x100"],
                  ["l0-i2s","0x101"],
                  ["l0-i3s","0x102"],
                  ["l0-i4s","0x103"],
                  ["l0-i5s","0x104"],
                  ["l0-i6s","0x105"],
                  ["l0-i7s","0x106"],
                  ["l0-i8s","0x107"],
                  ["l0-i9s","0x108"],
                  ["l0-i10s","0x109"],
                  ["l0-i11s","0x10a"],
                  ["l0-i12s","0x10b"],
                  ["l0-i13s","0x10c"],
                  ["l0-i14s","0x10d"],
                  ["l0-i15s","0x10e"],
                  ["l0-i16s","0x10f"],
                  ["l0-i17s","0x110"],
                  ["l0-i18s","0x111"],
                  ["l0-i19s","0x112"],
                  ["l0-i20s","0x113"],
                  ["l0-i21s","0x114"],
                  ["l0-i22s","0x115"],
                  ["l0-i23s","0x116"],
                  ["l0-i24s","0x117"]
                 ],
    "l1A":[
     ["pll_bc","0"], ["orbit","1"], ["TestClassBV","2"], ["GND","3"],
     ["GND","4"], ["GND","5"], ["GND","6"], ["GND","7"], ["GND","8"],
     ["L0strBeforeDelay+24L0inputData","9"], ["L0datainBeforeDelay","10"], 
     ["INT1","11"], ["INT2","12"],
     ["L1strobe(25ns)","13"], ["GND","14"], ["GND","15"],
     ["PF1","16"], ["PF2","17"], ["PF3","18"], ["PF4","19"], ["PFT","20"],
     ["ADCinput_overlap","21"], ["GND","22"], ["strobe","23"], 
     ["Class48AfterVetos","24"], ["Class28AfterVetos","25"], 
     ["GND","26"], ["GND","27"], ["GND","28"],
     ["L1ack(testclass)","29"],
     ["INT_DAfterDelay","30"], ["GND","31"],
     ["l1-i1 before synch.","0x1000"], ["l1-i2","0x1001"], ["l1-i3","0x1002"],
     ["l1-i4","0x1003"], ["l1-i5","0x1004"], ["l1-i6","0x1005"],
     ["l1-i7","0x1006"], ["l1-i8","0x1007"], ["l1-i9","0x1008"],
     ["l1-i10","0x1009"], ["l1-i11","0x100a"], ["l1-i12","0x100b"],
     ["l1-i13","0x100c"], ["l1-i14","0x100d"], ["l1-i15","0x100e"],
     ["l1-i16","0x100f"], ["l1-i17","0x1010"], ["l1-i18","0x1011"],
     ["l1-i19","0x1012"], ["l1-i20","0x1013"], ["l1-i21","0x1014"],
     ["l1-i22","0x1015"], ["l1-i23","0x1016"], ["l1-i24","0x1017"]
        ],
    "l1B":[ ["bcAfterPLL","0"], ["orbit","1"],
     ["l1clstt","2"], ["l1clst1","3"], ["l1clst2","4"],
     ["l1clst3","5"], ["l1clst4","6"], ["l1clst5","7"],
     ["l1clst6","8"], ["l1strobe+48InputData","9"], ["l1data","10"],
     ["L0strobe after delay+24L0inpData","11"], ["ESRflag","12"],
     ["L0str(25ns,1bc delay)","13"], ["GND","14"], ["GND","15"],
     ["GND","16"], ["GND","17"], ["GND","18"],
     ["GND","19"], ["GND","20"], ["GND","21"],
     ["Strobe&write","22"], ["Strobe&!write","23"],
     ["Class48 Before Vetos","24"], ["Class28 Before Vetos","25"], 
     ["TestClass Before Vetos","26"], 
     ["GND","27"], ["INTA signal","28"], ["INTB signal","29"],
     ["INTD delayed before INT","30"], ["GND","31"],
     ["l1-i1s after synch/alignment","0x100"],
     ["l1-i2s","0x101"],
     ["l1-i3s","0x102"],
     ["l1-i4s","0x103"],
     ["l1-i5s","0x104"],
     ["l1-i6s","0x105"],
     ["l1-i7s","0x106"],
     ["l1-i8s","0x107"],
     ["l1-i9s","0x108"],
     ["l1-i10s","0x109"],
     ["l1-i11s","0x10a"],
     ["l1-i12s","0x10b"],
     ["l1-i13s","0x10c"],
     ["l1-i14s","0x10d"],
     ["l1-i15s","0x10e"],
     ["l1-i16s","0x10f"],
     ["l1-i17s","0x110"],
     ["l1-i18s","0x111"],
     ["l1-i19s","0x112"],
     ["l1-i20s","0x113"],
     ["l1-i21s","0x114"],
     ["l1-i22s","0x115"],
     ["l1-i23s","0x116"],
     ["l1-i24s","0x117"]
        ],
    "l2A":[
     ["pll_bc","0"], ["orbit","1"], ["TestClassBV","2"], ["GND","3"],
     ["GND","4"], ["GND","5"], ["GND","6"], ["GND","7"], ["GND","8"],
     ["L1strBeforeDelay(+48bitsInputData)","9"], ["L1datainBeforeDelay","10"], 
     ["INT1","11"], ["INT2","12"],
     ["L2strobe(25ns)","13"], ["GND","14"], ["GND","15"],
     ["PF1","16"], ["PF2","17"], ["PF3","18"], ["PF4","19"], ["PFT","20"],
     ["ADCinput_overlap","21"], ["GND","22"], ["strobe","23"], 
     ["Class48AfterVetos","24"], ["Class28AfterVetos","25"], 
     ["GND","26"], ["GND","27"], ["L2Rack(testclass)","28"],
     ["L2Aack(testclass)","29"],
     ["INT_DAfterDelay","30"], ["GND","31"],
     ["l2-i1 before synch.","0x1000"], ["l2-i2","0x1001"], ["l2-i3","0x1002"],
     ["l2-i4","0x1003"], ["l2-i5","0x1004"], ["l2-i6","0x1005"],
     ["l2-i7","0x1006"], ["l2-i8","0x1007"], ["l2-i9","0x1008"],
     ["l2-i10","0x1009"], ["l2-i11","0x100a"], ["l2-i12","0x100b"],
     ["GND","0x100c"]
        ],
    "l2B":[ ["bcAfterPLL","0"], ["orbit","1"],
     ["l2clstt","2"], ["l2clst1","3"], ["l2clst2","4"],
     ["l2clst3","5"], ["l2clst4","6"], ["l2clst5","7"],
     ["l2clst6","8"], ["l1strobeAfterDelay","9"], ["l2strobeOut(+60bitsData)","10"],
     ["L2data1out","11"], ["L2data2out","12"],
     ["L1strobe(25ns,1bcdelay)","13"], ["GND","14"], ["GND","15"],
     ["GND","16"], ["GND","17"], ["GND","18"],
     ["GND","19"], ["GND","20"], ["GND","21"],
     ["Strobe&write","22"], ["Strobe&!write","23"],
     ["Class48 Before Vetos","24"], ["Class28 Before Vetos","25"], 
     ["TestClass Before Vetos","26"], 
     ["GND","27"], ["INTA signal","28"], ["INTB signal","29"],
     ["delayed INT before delay","30"], ["GND","31"],
     ["l2-i1s after synch/alignment","0x100"],
     ["l2-i2s","0x101"],
     ["l2-i3s","0x102"],
     ["l2-i4s","0x103"],
     ["l2-i5s","0x104"],
     ["l2-i6s","0x105"],
     ["l2-i7s","0x106"],
     ["l2-i8s","0x107"],
     ["l2-i9s","0x108"],
     ["l2-i10s","0x109"],
     ["l2-i11s","0x10a"],
     ["l2-i12s","0x10b"],
     ["GND","0x10c"]
        ],
    "busyA":[
    ["in_bc","0"], ["inOrbit","1"],
    ["EdgeFlag","2"],
    ["l0clstt","3"],
    ["l0clst1","4"],
    ["l0clst2","5"],
    ["l0clst3","6"],
    ["l0clst4","7"],
    ["l0clst5","8"],
    ["l0clst6","9"],
    ["ctpbusy","10"],
    ["anyCluster","11"],
    ["strobe","12"],
    ["WriteStrobe","13"],
    ["ReadStrobe","14"],
    ["GND","15"],
    ["GND","16"],
    ["GND","17"],
    ["GND","18"],
    ["BUSY over threshold(25ns)","19"],
    ["in_busy1","20"],
    ["in_busy2","21"],
    ["in_busy3","22"],
    ["in_busy4","23"],
    ["in_busy5","24"],
    ["in_busy6","25"],
    ["in_busy7","26"],
    ["in_busy8","27"],
    ["in_busy9","28"],
    ["in_bus10","29"],
    ["in_bus11","30"],
    ["in_bus12","31"]],
           "busyB":[
    ["bc","0"], ["delayed bc input","1"],
    ["out_bc","2"],
    ["orbit","3"],
    ["byclstt","4"],
    ["byclst1","5"],
    ["byclst2","6"],
    ["byclst3","7"],
    ["byclst4","8"],
    ["byclst5","9"],
    ["byclst6","10"],
    ["ctp_time.busy (CTP deadtime)","11"],
    ["clst_timet.busy (Test cluster L0-L1time)","12"],
    ["clst_time1.busy","13"],
    ["clst_time2.busy","14"],
    ["clst_time3.busy","15"],
    ["clst_time4.busy","16"],
    ["clst_time5.busy","17"],
    ["clst_time6.busy","18"],
    ["BUSY over threshold(25ns)","19"],
    ["in_busy13","20"],
    ["in_busy14","21"],
    ["in_busy15","22"],
    ["in_busy16","23"],
    ["in_busy17","24"],
    ["in_busy18","25"],
    ["in_busy19","26"],
    ["in_busy20","27"],
    ["in_busy21","28"],
    ["in_busy22","29"],
    ["in_busy23","30"],
    ["in_busy24","31"]],
    "intA":[
    ["pll_bc","0"], ["orbit","1"], ["vme_clock","2"], ["Strobe&Write","3"],
    ["Strobe&!Write","4"], ["GND","5"], ["L2str in+60bitInpData","6"], ["int1","7"],
    ["int2","8"], ["L1strobe in+48InpData","9"], ["L1data in","10"], ["L2str(25ns)","11"],
    ["fiBEN_N (DDL bus enable)","12"], ["fiDIR (DDL Direction)","13"], 
    ["ifiLF_N (DDL Link full)","14"], ["DDL L2a seq.","15"],
    ["DDL L2r seq.","16"], ["DDL L2a_tc seq.","17"], ["TestCounter1 in","18"], 
    ["GND","19"],
    ["I2C SCL","20"], ["I2C SDA out","21"], ["I2C SDA in","22"], 
    ["I2C BUSY","23"],
    ["I2C error","24"], ["DDL Int.record block","25"], 
    ["GND","26"], ["GND","27"],
    ["GND","28"], ["GND","29"], ["GND","30"], ["GND","31"]
    ],
    "intB":[
    ["bc","0"], ["orbit","1"],
    ["orbit delayed (offset)","2"],
    ["strobe","3"],
    ["CTP busy out","4"],
    ["GND","5"],
    ["L2str in+60bitInpData","6"],
    ["L2data1 in","7"],
    ["L2data2 in","8"],
    ["RoI strobe out","9"],
    ["RoIdata out","10"],
    ["L2str(25ns)","11"],
    ["DDLbusy","12"],
    ["DDL EOB busy","13"],
    ["DDL readout grant","14"],
    ["DDL fbTEN_N out (Trans. en.)","15"],
    ["DDL fbCTRL_N out (Control)","16"],
    ["DDL fbD[15] -data bit","17"],
    ["GND","18"],
    ["GND","19"],
    ["mux_write","20"],
    ["mux_read","21"],
    ["adc_write","22"],
    ["adc_read","23"],
    ["Test Counter 2 in","24"],
    ["DDL CTP readout block","25"],
    ["Incomplete orbit record","26"],
    ["ddl.rc_error Orbit rec. with error","27"],
    ["Orbit counter sync. error","28"],
    ["GND","29"],
    ["GND","30"],
    ["GND","31"]]
    }
  def __init__(self, fr, ab, vb=None):
    self.updateSWLED=None
    self.vb=vb
    self.fr=fr
    self.ab=ab
    self.signal=None
    selboard= self.findBoard()
    self.board= myw.MywxMenu(self.fr, label=ab+': ',
      helptext="Board choosen for scope "+ab+" output ",
      defaultinx=selboard, side=LEFT, items=ABsig.ctpboards,cmd=self.modboard)
    self.modboard(None, selboard)
  def findBoard(self, bnix=None):
    """ 
    bnix==None: 
    Find 1st Board sending scope self.ab and
    check if there is other boards with output enabled. Disable
    the output of these boards.
    bnix!=None:
    Enable just the requested board (self.ab signal), disable the rest
    """
    if bnix!=None:
      bntxt=self.vb.io.execute("setScopeBoard('"+self.ab+"',"+str(bnix)+")",
        applout="<>")[0]
    else:
      bntxt=self.vb.io.execute("checkScopeBoard('"+self.ab+"')",
        applout="<>")[0]
    if bntxt== '-1':
      bntxt=str(len(ABsig.ctpboards)-1)   #not selected
    #print "bntxt:",type(bntxt),bntxt
    return eval(bntxt)
  def findSignal(self,six=None):
    """six:
    rc: index into ABsig.signals[self.boardix], which is:
        =six (if six was !=None on Input)
        =index (if six was ==None on Input ) to signal which was/is set
           on the board
        = -1 board not in the crae
    """
    if six!=None:
      #print "ABsig.findSignal:boarsig:",self.boardsig,six
      #print "ABsig.findSignal:",ABsig.signals[self.boardsig][six]
      #sixval=str(six)
      sixval= ABsig.signals[self.boardsig][six][1]
      sitxt=self.vb.io.execute("setScopeSignal("+str(self.boardix)+","+
      str(ord(self.ab))+","+sixval+")", applout="<>")[0]
      self.esi= six
    else:
      self.esi= findSigOnBoard(self.vb, self.boardix, self.ab)
      #sitxt=self.vb.io.execute("getScopeSignal("+str(self.boardix)+","+
      #str(ord(self.ab))+")", log="out", applout="<>")[0]
      #self.esi=eval(sitxt) &0x111f #Abis:0x1000 or Bbis:0x100 signal: 0x1f)
      # here, perhaps it would be better to search through
      # ABsig.signals[self.boardsig]. This would mean:
      # - not all items in that array would be necessary (now 
      #   0..23,24,...  all the items have to be present
      # - "1" or "0x1001" -should be codded as numbers (not strings)
      #if self.esi>0x1f:            # bis signal  
      #  self.esi= (self.esi & 0x1f) + 32
    #print "self.esi:", self.esi
    return self.esi
  def modboard(self, inst, ixx):
    boardn= self.board.getEntry()
    self.boardix= getixboard(boardn)
    #print "ABsig.modboard:ixx ix:",ixx,ix
    if boardn[:2]=='fo':
      self.boardsig= 'fo'+self.ab
    else:
      self.boardsig= boardn+self.ab
    if self.signal: 
      self.signal.destroy()
    self.findBoard(self.boardix)
    if boardn=='not selected': return
    selsignal= self.findSignal()
    self.signal= myw.MywxMenu(self.fr, label='',
      helptext="Signal choosen for scope "+self.ab+" output ",
      defaultinx=selsignal, side=LEFT, items=ABsig.signals[self.boardsig],
      cmd=self.modsignal)
    if self.updateSWLED:
      self.updateSWLED[self.boardix](self.ab,
        ABsig.signals[self.boardsig][selsignal][0])
  def modsignal(self,inst,ix):
    print "ABsig.modsignal:ix:",ix
    #ABsig.signals[self.boardsig][ix][1]
    selsig= self.findSignal(ix)
    if self.updateSWLED:
      self.updateSWLED[self.boardix](self.ab,
        ABsig.signals[self.boardsig][selsig][0])
  #def getsigname(self, esi):
  #  print "scpab:",esi, self.signal.getIndex(), self.signal.getEntry()
  #  signame= self.signal.items[self.signal.getIndex()][0]
  #  print "scpab2:",signame
  #  return signame
class VmeRW2Scope:
  def __init__(self,fr,vb=None):
    self.vb=vb
    self.getrwscope()   # boards in 'VMERW-Scope' mode
    self.rwab= myw.MywBits(fr, label="VMERW-Scope mode for",
      defval=self.boardsinAB, cmd=self.modboardsinAB,
      bits=[("busy"),("l0"),("l1"),("l2"),("int"),
        ("fo1"), ("fo2"), ("fo3"), ("fo4"), ("fo5"), ("fo6")],
      side=LEFT,
      helptext="""CTP boards in 'VMERW-Scope' mode, i.e.
VME R/W LEDs corresponds to Scope A/B signal of this board
(signal is choosen, but it doesn't necessarily mean that
the signal has to be enabled for Scope A/B
backplane signals in the same time).
By default, CTP boards are in 'VMERW' mode -i.e. VME R/W LEDS
indicate VME read/write operations on this board
""")
  def modboardsinAB(self):
    newval=self.rwab.getEntry()
    self.vb.io.execute("setVMERWScope("+hex(newval)+","+
      hex(self.boardsinAB)+")")
    self.boardsinAB=newval
    print "modboardsinAB:",newval
  def getrwscope(self):
    self.boardsinAB=int(self.vb.io.execute("getVMERWScope()",
      applout="<>")[0])

class SOFTLEDS:
  SWLEDS=(("R","VME read"), ("W","VME write"), ("A","SCOPE-A"), 
          ("B","SCOPE-B"),
          (".","""'continuous update' indication, i.e. there is 
SW VME read every half second if this label is moving.
Blue color indicates 'no update'.
Click on this button to get 4 associated fields (VMEread, 
VMEwrite and 2 signals) updated for another minute. """))
  #ooo=("-","\\","|","/")
  ooo=("\\","/")
  def __init__(self, vb, frame, board=None, activeA=None, activeB=None):
    self.vb=vb
    self.ooo=0
    self.active=0   # 1: update active   0: update not active
    self.afteridCount= 0
    if frame==None:
      self.f1=myw.NewToplevel("Soft LEDS")
    else:
      self.f1= frame
    self.labels=[];
    if board:
      self.ixboard=getixboard(board)
      self.boardlabel= myw.MywLabel(self.f1, board,
        "CTP board", fill='both', side=LEFT)
    for ix in range(4):
      if (ix==2) and activeA:
        actSigName= activeA
      elif (ix==3) and activeB:
        actSigName= activeB
      else:
        actSigName=SOFTLEDS.SWLEDS[ix][0]
      self.labels.append(myw.MywLabel(self.f1, actSigName,
        SOFTLEDS.SWLEDS[ix][1], fill='both', side=LEFT))
    self.labels.append(myw.MywButton(self.f1, SOFTLEDS.SWLEDS[4][0],
      helptext=SOFTLEDS.SWLEDS[4][1], side=LEFT, cmd=self.updateOnOff))
    self.updateOnOff()
  def updateOnOff(self, action=None):
    if action=='start': self.active=1
    elif action=='stop': self.active=0
    else: self.active=1-self.active
    if self.active:
      self.labels[4].resetColor()
      self.afteridCount= 0
      self.update()
    else:
      self.labels[4].setColor("blue")
  def update(self):
    self.sls= self.vb.io.execute("getSWLEDS(%d)"%self.ixboard, log="NO")#"1011"
    #print "scopeab.py SOFTLEDS:",self.sls
    for ix in range(4):
      if self.sls[ix]=='1':
        self.labels[ix].setColor("red")
      else:
        self.labels[ix].setColor("white")
    self.labels[4].setLabel(SOFTLEDS.ooo[self.ooo])
    self.ooo= self.ooo+1
    if self.ooo >= len(SOFTLEDS.ooo): self.ooo=0
    if self.active:
      self.afterid=self.f1.after(500, self.update)
      self.afteridCount= self.afteridCount+1
      if self.afteridCount > 10:   # 240: stop after 2 minutes
        self.updateOnOff()
  def updateLabel(self, ab, newlabel):
    if ab=='A': ix=2
    elif ab=='B': ix=3
    else: ix=0
    self.labels[ix].label(newlabel)

class vbio2:
  def __init__(self):
    pass
  def execute(self,cmd):
    print "vbio:", cmd
    return ["0"]
class vbio:
  def __init__(self):
    self.io=vbio2()

if __name__ == "__main__":
    #main()
  master=Tk()
  vb= vbio()
  ABsig(master,"A",vb)
  ABsig(master,"B",vb)
  master.mainloop()

