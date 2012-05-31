# 13.9. 2005 - Scope outputs selection for CTP boards
from Tkinter import *
#import os, os.path, glob, string
import myw

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
     ["l1strobe","17"], ["l2data1","18"], ["l2data2","19"], 
     ["out_l2strobe-1","20"], ["out_l2strobe-2","21"], 
     ["out_l2strobe-3","22"], ["out_l2strobe-4","23"]],
    "foB":[
     ["bc fromPLL","0"], ["out_orbit-1","1"], ["out_prepulse-1","2"],
     ["out_prepulse-2","3"], ["out_prepulse-3","4"], ["out_prepulse-4","5"],
     ["out_l0-1","6"], ["out_l0-2","7"], ["out_l0-3","8"],
     ["out_l0-4","9"], ["out_l1-1","10"], ["out_l1-2","11"],
     ["out_l1-3","12"], ["out_l1-4","13"], ["out_l1data-1","14"],
     ["out_l1data-2","15"], ["out_l1data-3","16"], ["out_l1data-4","17"],
     ["l1data","18"], ["l2strobe","19"], ["out_l2data-1","20"],
     ["out_l2data-2","21"], ["out_l2data-3","22"], ["out_l2data-4","23"] ],
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
     ["L0request","28"], ["L0ack","29"], ["n/a","30"], ["n/a","31"],
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
                  ["l0strobe","9"],
                  ["l0data","10"],
                  ["prepulse","11"],
                  ["rndtrg1","12"],
                  ["rndtrg2","13"],
                  ["0","14"],
                  ["0","15"],
                  ["0","16"],
                  ["0","17"],
                  ["SWtrigger","18"],
                  ["0","19"],
                  ["0","20"],
                  ["bc_ff","21"],
                  ["strobe & write","22"],
                  ["strobe & !write","23"],
                  ["CLASS48beforeVetos","24"],
                  ["CLASS28beforeVetos","25"],
                  ["GND","26"],
                  ["CLASS28mask","27"],
                  ["INTa","28"],
                  ["INTb","29"],
                  ["DelaydINTbeforeDelay","30"],
                  ["n/a","31"],
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
     ["L0str Before delay","9"], ["L0datain Before delay","10"], 
     ["INT1","11"], ["INT2","12"],
     ["GND","13"], ["GND","14"], ["GND","15"],
     ["PF1","16"],
     ["PF2","17"],
     ["PF3","18"],
     ["PF4","19"],
     ["PFT","20"],
     ["ADCinput overlap","21"], 
     ["GND","22"], 
     ["strobe","23"], 
     ["Class48 After Vetos","24"], 
     ["Class28 After Vetos","25"], 
     ["GND","26"], ["GND","27"], ["GND","28"],
     ["L1ack (test class)","29"]
     ["INT_D after delay","30"]
     ["n/a","31"],
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
            ["l1clst6","8"], ["l1strobe","9"], ["l1data","10"],
            ["L0strobe after delay","11"], ["ESRflag","12"],
            ["GND","13"], ["GND","14"], ["GND","15"],
            ["GND","16"], ["GND","17"], ["GND","18"],
            ["GND","19"], ["GND","20"], ["GND","21"],
            ["Strobe&write","22"], ["Strobe&!write","23"],
            ["Class48 Before Vetos","24"], 
            ["Class28 Before Vetos","25"], 
            ["TestClass Before Vetos","26"], 
            ["GND","27"],
            ["INTA signal","28"],
            ["INTB signal","29"],
            ["INTD delayed before INT","30"],
            ["GND","31"],
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
    "busyA":[
    ["in_orbit","0"], ["in_bc","1"],
    ["out_bc","2"],
    ["orbit","3"],
    ["busyclst1","4"],
    ["busyclst2","5"],
    ["busyclst3","6"],
    ["busyclst4","7"],
    ["busyclst5","8"],
    ["busyclst6","9"],
    ["busyclstT","10"],
    ["busy01","11"],
    ["busy02","12"],
    ["busy03","13"],
    ["busy04","14"],
    ["busy05","15"],
    ["busy06","16"],
    ["busy07","17"],
    ["busy08","18"],
    ["busy09","19"],
    ["busy10","20"],
    ["busy11","21"],
    ["busy12","22"],
    ["GND","23"]],
           "busyB":[
    ["in_orbit","0"], ["bc_afterPLL","1"],
    ["scopeA","2"],
    ["scopeB","3"],
    ["l0clst1","4"],
    ["l0clst2","5"],
    ["l0clst3","6"],
    ["l0clst4","7"],
    ["l0clst5","8"],
    ["l0clst6","9"],
    ["l0clstT","10"],
    ["busy13","11"],
    ["busy14","12"],
    ["busy15","13"],
    ["busy16","14"],
    ["busy17","15"],
    ["busy18","16"],
    ["busy19","17"],
    ["busy20","18"],
    ["busy21","19"],
    ["busy22","20"],
    ["busy23","21"],
    ["busy24","22"],
    ["ctpbusy","23"]
    ]}
  def __init__(self, fr, ab, vb=None):
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
    """rc: index into ABsig.signals[self.boardix], which is:
          =six (if six was !=None)
          =index (if six was ==None) to signal which was/is set
           on the board
    """
    if six!=None:
      #print "ABsig.findSignal:boarsig:",self.boardsig,six
      #print "ABsig.findSignal:",ABsig.signals[self.boardsig][six]
      #sixval=str(six)
      sixval= ABsig.signals[self.boardsig][six][1]
      sitxt=self.vb.io.execute("setScopeSignal("+str(self.boardix)+","+
      str(ord(self.ab))+","+sixval+")", applout="<>")[0]
      esitxt= six
    else:
      sitxt=self.vb.io.execute("getScopeSignal("+str(self.boardix)+","+
      str(ord(self.ab))+")", log="out", applout="<>")[0]
      esitxt=eval(sitxt) &0x111f #Abis:0x1000 or Bbis:0x100 signal: 0x1f)
      # here, perhaps it would be better to search through
      # ABsig.signals[self.boardsig]. This would mean:
      # - not all items in that array would be necessary (now 
      #   0..23,24,...  all the items have to be present
      # - "1" or "0x1001" -should be codded as numbers (not strings)
      if esitxt>0x1f:            # bis signal  
        esitxt= (esitxt & 0x1f) + 32
    return esitxt
  def modboard(self, inst, ixx):
    boardn= self.board.getEntry()
    for ix in range(len(ABsig.ctpboards)):
      if boardn==ABsig.ctpboards[ix]: 
        self.boardix=ix          # pointer to ctpboards
        break
    print "ABsig.modboard:ixx ix:",ixx,ix
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
  def modsignal(self,inst,ix):
    print "ABsig.modsignal:ix:",ix
    #ABsig.signals[self.boardsig][ix][1]
    self.findSignal(ix)

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

