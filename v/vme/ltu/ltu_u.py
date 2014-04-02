#30.6. CtpEmulator -only 1 window allowed
#12.10.2005 loadseq: .seq file (old way) or .slm file -> compile
#           to /tmp/slmseq.seq and then load it
#examineslm: getfile added for dim-operation
#16.12.2013 lturun2 added
from Tkinter import *
import tkFileDialog
import os, os.path, glob, string, types
import myw,counters

def ffdummy(txtlines='ffdummy'):
  pass
  print "ff:", txtlines
#--------------------------- start CTP emulator
# 
ctpemuact=0   # 1->CtpEmulator window active, 0-> CtpEmulator w. not active
def CTP_Emulator(vb):
  class CtpEmulator:
    slmhelptext="""
Trigger sequences can be coded as text files with suffix .slm -
see v/vme/CFG/ltu/SLM/all.slm file for an example.

First line starting with "Errors:" label (this line can be skipped) sets
the Error flags for CTP emulator (i.e. the types of errors
deliberately generated during emulation for ErrProne flagged sequences).
The errors are forced by skipping one of these signals and messages:
PP      -prepulse
L0      -L0 signal
L1      -L1 signal
L1M     -L1 message
L1&L1M  -simultaneous suppression of L1 signal and L1 message
L2aM    -L2a message
L2rWord -L2 reject word

Lines starting with character '#' are comments

1 trigger sequence is coded per 1 line, following this format:

CODE L1classes L2classes Clusters flags

where:
CODE is one of: L0 L2A L2R CPP CL0 CL2A CLR
----
L1classes:
-------------- 
Bit pattern of L1 classes written as 25 (13 before 2014) hexa digits
The least signifiant bit is 1st class = 0x1.
The most significant bit is class 100:  0x8000000000000000000000000
Before 2014, only 50 classes were available, i.e. class 50: 0x2000000000000
All classes: 0xfffffffffffffffffffffffff 
(0x3ffffffffffff during run1)

L2classes:
------------
same as L1 classes, but in case of software trigger bits 48..25 are
"Participating L2 detectors" (at least one bit must be different from 0
in order to avoid generation of error in DAQ) 

Clusters: is 8 bits (6 bits during rn1) long string: e.g.
--------
   0x3f    - all clusters (1,2,3,4,5,6)
   0x7     - just clusters 1,2,3

These 4 parameteres mentioned above (code L1classes L2classes 
and Clusters) have to appear in each line for L2 sequences.
They have no meaning for L0, CPP and CL0 sequences.

In addition, following flags setting the bits in L1 and L2 messages,
are valid:
roc=N    N is decimal number 0-15 representing four Readout control
         bits. Codes 8-15 are reserved for trigger/daq 
         subsystem. Following codes were already allocated:
         
         14 -Start Of Data event (together with L1SwC, L2SwC flags)
         15 -End Of Data event   (together with L1SwC, L2SwC flags)

ESR      Enable Segmented Readout (part of ROI option)
CIT      (or CIT) Calibration trigger
L1SwC    Software Class status bit (in L1
L2SwC    or L2 message)
ErrProne the sequence is marked as 'error prone' 
Restart  sets the Restart bit (last sequence in the loop)
Last     last sequence in the list (set automatically)
spare1   2 unused bits in L1/L2 messages
spare2
spare3   = ESR bit in L2a message (word4.bit13)
"""
    seqhelptext="""
.seq file fomat:
First 2 lines (name and # of valid sequences in the file) not used
3. line contains 7 bits for ERROR_SELECTOR word in this order:
PP L0 L1 L1M L1&L1M L2aM L2rWord

Starting from 4th line, 8 lines are reserved for 1 sequence:
30....,...20....,...10....,....0
0SsC2XXXXXXXXcccSCrrrrsW99ELRxxx
xxx  seq. code: 1:L0 2:L2A 3:L2R 4:CPP 5:CL0 6:CL2A 7:CLR
ELR  3 bits: Error prone, Last and Restart    
99   L1class100..99
W    L1SwC
s    ESR
rrrr RoC 4 bits
C    Calibration trigger
S    spare
ccc  L2class100..98
XXXXXXXX L2Cluster8..1
2    L2SwC

CCCCCCCCCCCCCCCCcccccccccccccccc
cccccccccccccccc L1class98..83
CCCCCCCCCCCCCCCC L2class97..82

CCCCCCCCCCCCCCCCcccccccccccccccc
cccccccccccccccc L1class82..67
CCCCCCCCCCCCCCCC L2class81..66

CCCCCCCCCCCCCCCCcccccccccccccccc
cccccccccccccccc L1class66..51
CCCCCCCCCCCCCCCC L2class65..50

CCCCCCCCCCCCCCCCcccccccccccccccc  or
0dddddddddddddddcccccccccccccccc
cccccccccccccccc L1class50..35
CCCCCCCCCCCCCCCC L2class49..34
ddddddddddddddd  Detector24..10

CCCCCCCCCCCCCCCCcccccccccccccccc or
dddddddddCCCCCCCcccccccccccccccc
cccccccccccccccc L1class34..19
CCCCCCCCCCCCCCCC L2class33..18
ddddddddd Detector9..1

CCCCCCCCCCCCCCCCcccccccccccccccc
cccccccccccccccc L1class18..3 
CCCCCCCCCCCCCCCC L2class17..2 

C000000000000000cc00000000000000
cc L1class2..1
C  L2class1   
"""
    CLICKHERE='click here to choose the sequence'
    def __init__(self, vb):
      global ctpemuact
      #os.system('pwd') 
      if ctpemuact==1: return  #self.__del__ called after this return
      ctpemuact= 1
      #self.slmdir=os.path.join(os.environ['VMEWORKDIR'],"CFG","ltu","SLM")
      self.slmdir=os.path.join("CFG","ltu","SLM")
      self.vb=vb
      self.errsel= None
      self.sw2generate=0
      self.swLabel="Generate SW 'Start signal(s)'"
      self.tl=Toplevel(vb.master)
      self.tl.group(vb.master)
      #self.tl.bind("<Destroy>", self.delme, add="+")
      self.tl.bind("<Destroy>", self.delme)
      self.tl.title("CTP emulator")
      #print "hxbits1:",hxbits1,"<"
      #hxbits1= self.vb.io.execute("ERgetselector()",applout="<>")
      #hxbits=hxbits1[0]
      hxbits= self.vb.io.execute("ERgetselector()",applout="<>")[0]
      self.AllowedErrors= eval(hxbits)
      #print hxbits,self.AllowedErrors
      #self.BCDOWNdef= self.vb.io.execute(
      #  "vmeopr32(COUNT_PERIOD)")[:-1]
      self.f1= myw.MywFrame(self.tl,side=TOP)   #seq. editor frame
      self.f12= myw.MywFrame(self.f1,relief=FLAT,side=BOTTOM)
      self.doSeqNames()
      self.lsbex= myw.MywButton(self.f12, "Examine SLM",
        cmd= self.examineslm, side=LEFT, helptext=
        """Read LTU sequencer memory, if reasonable 
content save it in WORK/slmasci.seq file""")
      self.lsb= myw.MywButton(self.f12, "Load sequence",
        cmd= self.loadseq, side=LEFT, helptext=
        "Read sequence file and load it into LTU sequencer memory (SLM)")
      sed= myw.MywButton(self.f12, "Sequencer editor",
        cmd= self.seqeditor, side=RIGHT, helptext=
        "Edit sequence file")
      # 'selections & cmds' frame
      self.f2= myw.MywFrame(self.tl)   
      self.f22= myw.MywFrame(self.f2,      #errors selection frame
        borderwidth=6,relief=FLAT)
      self.normalcol=self.f22.cget("bg")
      self.ersr= None   # error signal rate
      erritems=[["Errors disabled","0x0",self.errEnaDis],
        ["Errors enabled","0x1",self.errEnaDis]]
      # always start with what is in LTU: 
      erenadis= self.vb.io.execute("getERenadis()",applout="<>")[0]
      #self.vb.io.execute("ERenadis(0)")
      self.errendi= myw.MywxMenu(self.f22, label='',
        defaultinx=eval(erenadis), side=TOP,
        helptext="""If enabled, 'error generation' (randomly with
choosen error signal rate) is applied to 
all the sequences with asserted 'Error Prone flag'. 
Choose 0x7fffffff for 'Error signal rate' to achieve complete
suppression of choosen signals/messages.
""", items=erritems)
      self.errEnaDis("butonly")
      #
      self.f23gap= myw.MywFrame(self.f2) # gap, L1_FORMAT
      l1formatitems= (("L1m suppressed","0x0",self.l1format),
        ("L1m header only","0x1",self.l1format),
        ("Complete L1m","0x3",self.l1format))
      l1formatdefix= self.findDefaultL1m("L1_FORMAT", l1formatitems)
      self.l1mfmt= myw.MywxMenu(self.f23gap, label=
        'L1 message\nformat:', defaultinx=l1formatdefix,side=LEFT,
        helptext="""L1 message format -
0, 1 or 5 words sent over TTC B-channel.""",
        items=l1formatitems)
      defstartsig= self.vb.io.execute("SLMgetstart()",applout="<>")[0]
      self.lgv= eval(defstartsig) & 0x8
      self.lhcgap= myw.MywButton(self.f23gap, 
        label="LHC Gap\nVeto ON",
        helptext="""All the START signals are vetoed during LHC Gap
interval""",
        cmd=self.gapveto,side=RIGHT)
      self.gapveto("setbuttononly"); self.gapveto("setbuttononly")
      self.f23= myw.MywFrame(self.f2, relief=FLAT) # START sig. selection frame
      items=(
        ("not selected","0",self.notselected),
        ("Pulser/level","1",self.external),
        ("Random","2",self.randomRate),
        ("BC","3",self.bcScaling),
        ("Pulser/edge","5",self.external))
        #('Automatic START\n signal selection:',"4"))
      defaultix= eval(defstartsig) & 0x7
      if defaultix==4: defaultix=0
      if defaultix==5: defaultix=4
      #print defstartsig,':',defaultix
      self.srbe= None   #Sw, Random, BC or External
      #
      self.startsel= myw.MywxMenu(self.f23, label='Automatic START\nsignal selection:',
        helptext="""START signal generation selection
i.e. the signal triggering the execution of next sequence
from the list of loaded sequences""",
        defaultinx=defaultix, side=LEFT, items=items)
      if defaultix==2: self.randomRate("onlybut")
      if defaultix==3: self.bcScaling("onlybut")
      #
      self.f23gss= myw.MywFrame(self.f2, relief=FLAT) # generate START signal
      self.start1= myw.MywButton(self.f23gss, 
        label= self.swLabel,
        helptext="""Generate software START signal by pressing this button
i.e. execute next sequence without waiting for 'START signal' from
automatic generation (if selected BC,Random or Pulser)""",
        cmd=self.swStart, side=LEFT, state=DISABLED)
      self.start1N= myw.MywEntry(self.f23gss,
        label="# of signals:",defvalue="1", width=5,
        helptext="""Number of START signals to be generated.
0- generate forever""")
      self.start1S= myw.MywEntry(self.f23gss,
        label="spacing[ms]:", defvalue="0", width=5,
        helptext="""The interval between the START signals
generated by SW in miliseconds aproximately. 
Actualy, the interval between triggers can be much more higher 
if time for generation of all signals is longer than 2 seconds or 
spacing is longer than 100ms -
in that case, the interval between 2 triggers is increased by time necessary
to transfer request for each trigger to server over network + overhead
caused by invoking python script.
Value 0 has special meaning: - generate START signal as fast as possible""")
      #
      self.f23res= myw.MywFrame(self.f2, relief=FLAT) # repetitive emulation 
      self.repetitions= myw.MywEntry(self.f23res,
        label="# of emulation starts:",defvalue="1", width=5,
        helptext="""Number of 'emulation starts'. E.g.:
 1 corresponds to an emulation started by a mouse click. 
10 corresponds to one 'mouse click' emu-start + 9 automatically started emu-starts
NOTE:
the loaded sequence should not have 'Restart' flag (very likely,
you want to have new sequence started, after the previous one finished) 
""")
      self.repspacing= myw.MywEntry(self.f23res,
        label="interval [ms]:",defvalue="1", width=5,
        helptext="""The time in miliseconds between 2 repetitions of
the emulation start. Minimal value is 1.""")
      #
      #repbreaks=
      self.f23reb= myw.MywFrame(self.f2, relief=FLAT) # repetitive breaks 
      self.repbreaks= myw.MywEntry(self.f23reb,
        label="# of repetitive breaks:",defvalue="0", width=5,
        helptext="""Number of 'Emulation break' repetitions.
NOTE1:
The loaded sequence should have  2 'Restart' flags denoting
2 instruction loops. When the inner one finishes with an automatic break command,
the outer loop after restarting from 1st sequence enters the inner loop again.

NOTE2:
A thread is started, in time of Emulation start, executing break command in loop.
It is finished automatically, when emulation stops.
""")
      self.breakspacing= myw.MywEntry(self.f23reb,
        label="interval [ms]:",defvalue="1", width=5,
        helptext="The time in miliseconds between 2 breaks. Min. value: 1")
      ###
      self.f24= myw.MywFrame(self.f2,side=BOTTOM,relief=FLAT) # cmds frame
      self.butquit=myw.MywButton(self.f24, label="Quit\nemulation",
        helptext="Generate QUIT command", state=DISABLED,
        side=RIGHT,cmd=self.quitCmd)
      self.butbreak=myw.MywButton(self.f24, label="Break\nemulation",
        helptext="Generate BREAK command", state=DISABLED,
        side=RIGHT,cmd=self.breakCmd)
      self.butstart=myw.MywButton(self.f24, label="Start\nemulation",
        helptext="""Start emulation.
i.e. 'enable choosen START SIGNAL' """,
        side=RIGHT,cmd=self.startCmd)
      self.butcheck=myw.MywButton(self.f24, label="Check emulation\nstatus",
        helptext="""Check emulation status, set buttons accordingly.
""",
        side=RIGHT,cmd=self.checkemuCmd)
      self.checkemuCmd()
      #self.f3= myw.MywFrame(self.tl)   #info frame
    def __del__(self):
      pass
      #print "CTP emulator.__del__:",self
    def delme(self,event):
      global ctpemuact
      #print "CTP emulator.delme:",self
      ctpemuact= 0
    def findDefaultL1m(self, vmeaddr, items):
      """ vmeaddr: string representing vme address -symbolic, or 0x23
      items - list of lists [[displayedString,value],...] used
              as parameter for MywxMenu
      Operation:
      - read value from vme
      - find corresponding value in items
      - return index to items (to be used as default for MywxMenu) 
      """
      l1formatdefix =-1
      l1formatdef= self.vb.io.execute(
        "vmeopr32("+vmeaddr+")")[:-1]
      for i in range(len(items)):
        if items[i][1]==l1formatdef:
          l1formatdefix= i
          break;
      if l1formatdefix==-1:
        print "bad value read:",l1formatdef,":",items
      return l1formatdefix
    def doSeqNames(self):
      #cwd= os.getcwd(); os.chdir(self.slmdir); 
      #names= glob.glob("*.seq") + glob.glob("*.slm");
      #os.chdir(cwd);
      names= string.split(self.vb.io.execute('prtfnames("'+self.slmdir+'",".seq")',"no"))
      names= names+ string.split(self.vb.io.execute('prtfnames("'+self.slmdir+'",".slm")',"no"))
      #print "doSeqNames:",names
      if len(names)>0:
        self.itn=[[self.CLICKHERE,'']]
        for n in names:
          #basen= string.split(n,'.'); self.itn.append([basen[0],n])
          self.itn.append([n,n])
        self.itn.append(["Refresh this list","removevalue"])
        self.sfile= myw.MywxMenu(self.f1, label="Sequence:",
          helptext="""The name of the .seq or .slm file with stored sequences.
Press 'Load sequence' button to load it into the sequence memory, or
      'Sequencer editor' to edit .seq disk file""", 
          side='left',items=self.itn, delaction=self.refreshSeqNames,
          cmd=self.filedownload)
      else:
        self.sfile= myw.MywEntry(self.f1, label="Sequence:",
          helptext="""The name of the file with stored sequences
-type path relative to VMEWORKDIR, full name (including .seq or .slm suffix)""", 
          side='left')
    def refreshSeqNames(self, sf1):
      #print "refreshSeqNames:self.f1:",self.f1,"sf1:",sf1
      self.sfile.destroy()
      self.doSeqNames()
    def filedownload(self, inst, ix):
      if ix>0 and myw.DimLTUservers.has_key(self.vb.boardName):
        print 'Downloading from DIM server:'+self.vb.boardName,' file:',\
          self.itn[ix][0], ix
        fnbase= self.itn[ix][0]
        #self.vb.io.write('Downloading: '+fnbase+'\n')
        fn= os.path.join(self.slmdir,fnbase)
        self.vb.io.execute('getfile("'+fn+'")')
    def frompercent(self, oldval, newval):
      if newval[-1:]=='%':   # probability given in %
        retval= newval[:-1]
        retval="0x%x"%(0x7fffffff/100.*int(retval))
      else:
        retval= str(newval)
      #print "frompercent:",oldval,type(oldval), newval,type(newval),retval
      return retval
    def errEnaDis(self,butonly=None):
      val=self.errendi.getEntry()
      #print "errendi ",val,self.ersr
      if val=="0x1":
        if self.ersr != None: return
        if butonly==None:
          self.vb.io.execute("ERenadis(1)")
        self.f22.config(bg=myw.COLOR_WARNING)
        self.f221= myw.MywFrame(self.f22,    #random errors
          borderwidth=2, side=BOTTOM)
        self.f222= myw.MywFrame(self.f22,    #errors on demand
          borderwidth=2, side=BOTTOM)
        self.f221.config(bg=myw.COLOR_WARNING)
        self.f222.config(bg=myw.COLOR_WARNING)
        self.ersr= myw.MywVMEEntry(self.f221, label="Error signal rate:",
          helptext="""Average rate of the randomly generated error signals.
0          -min. rate (no errors)
0x3fffffff -probability of an error is 0.5
0x7fffffff -max. rate (permanent error)
n          -probability of an error generation (0..1) is 
            float(n)/0x7fffffff
or
use % to specify probability of error generation:
30%        - error will be generated in 30% of all triggers 
""",
          vb=self.vb, vmeaddr= "ERROR_RATE", side=RIGHT,
          userupdate=self.frompercent)
        self.errsel=myw.MywBits(self.f221,label="Random error generation allowed for",
          defval= self.AllowedErrors, cmd=self.moderrsel,
          helptext="""Random errors are generated with given
'Error signal rate'. Errors are created by suppressing
the transmission of signals/messages selected with this button.""",
          bits=["PP -Pre_pulse", "L0", "L1", "L1m -L1 Message",
            "L1&L1m -Simultaneous L1 and L1 Message",
            "L2a Message", "L2r Word"],
          side=LEFT)
        self.moderrsel()   # no errors always after CTPemulator win. opened
        self.errsel.mb.config(anchor=W)
        hlpt="""Press one of the 'Error on demad' buttons to generate 
1 error request.
Error on demand can be generated in parallel with random errros."""
        self.errdemL=myw.MywLabel(self.f222, label="Error on demand:",
          helptext= hlpt)
        self.errdemL.config(anchor=W)
        self.errdem=myw.MywHButtons(self.f222, helptext=hlpt,
          buttons= [("Pre-pulse\nerror", self.errOnDemand,"1"),
            ("L0\nerror", self.errOnDemand,"2"),
            ("L1\nerror", self.errOnDemand,"3"),
            ("L1 Message\nerror", self.errOnDemand,"4"),
            ("L1&L1 Message\nerror", self.errOnDemand,"5"),
            ("L2a Message\nerror", self.errOnDemand,"6"),
            ("L2r Word\n error", self.errOnDemand,"7"),
            ])
      else:
        if butonly==None:
          self.vb.io.execute("ERenadis(0)")
        self.f22.config(bg=self.normalcol)
        if self.ersr: 
          self.ersr.destroy(); self.ersr=None
          self.errsel.destroy(); self.errsel= None
          self.errdemL.destroy()
          self.errdem.destroy()
          self.f221.destroy()
          self.f222.destroy()
    def moderrsel(self):
      #print "moderrsel:",self.errsel.getEntry()
      if self.errsel:    # only if it does exist
        tobeset= self.errsel.getEntry()
        hxbits= self.vb.io.execute("ERgetselector()",applout="<>")[0]
        inltu= eval(hxbits)
        if inltu != tobeset:
          self.vb.io.execute("ERsetselector("+str(tobeset)+")")
    def notselected(self):
      if self.srbe: self.srbe.destroy()
      self.srbe= None
      self.vb.io.execute("SLMsetstart("+str(self.lgv)+")")
    def randomRate(self,onlybut=None):
      if self.srbe: self.srbe.destroy()
      self.srbe= myw.MywVMEEntry(self.f23, label="rate:",
        helptext=myw.frommsRandomHelp,
        vb=self.vb, vmeaddr= "RANDOM_NUMBER", userupdate=myw.frommsRandom)
      if onlybut==None:
        self.vb.io.execute("SLMsetstart("+str(self.lgv | int(self.startsel.getEntry()))+")")
        afterid=self.f1.after(100, self.checkemuCmd)
    #def setrndrate(self):
    #  self.vb.io.execute("setrate("+self.srbe.getEntry()+")")
    def bcScaling(self,onlybut=None):
      if self.srbe: self.srbe.destroy()
      self.srbe= myw.MywVMEEntry(self.f23, label="BC downscaling\nfactor",
        helptext=myw.frommsHelp,
        vb=self.vb, vmeaddr= "COUNT_PERIOD", userupdate=myw.fromms)
      #  cmdlabel=self.setbcScaling, defvalue=self.BCDOWNdef)
      if onlybut==None:
        self.vb.io.execute("SLMsetstart("+str(self.lgv | int(self.startsel.getEntry()))+")")
        afterid=self.f1.after(100, self.checkemuCmd)
      #self.srbe.entry.bind("<Leave>", self.setbcScaling)
      #self.srbe.entry.bind("<Key-Return>", self.setbcScaling)
    #def setbcScaling(self, event=None):
    #  newbcdown= self.srbe.getEntry()
    #  if newbcdown != self.BCDOWNdef:
    #    self.BCDOWNdef= newbcdown
    #    self.vb.io.execute("setBCDOWN("+self.BCDOWNdef+")")
    def external(self):
      if self.srbe: self.srbe.destroy()
      self.srbe= None
      #print "external:",self.startsel.getEntry()
      self.vb.io.execute("SLMsetstart("+str(self.lgv | int(self.startsel.getEntry()))+")")
    def swStart(self):
      if self.sw2generate>0:
        self.tl.after_cancel(self.swafterid)
        self.sw2generate=0
        self.start1.resetColor() ; self.start1.setLabel(self.swLabel)
      else:
        n=self.start1N.getEntry()
        self.sw2generate= int(n)
        if self.sw2generate ==0: return
        spacing=self.start1S.getEntry()
        self.spacing= int(spacing)
        if int(spacing)>=100 or int(n)*(int(spacing)+1) > 2000:
          self.start1.setColor(myw.COLOR_WARNING)
          self.start1.setLabel("Stop SW 'Start signals'")
          self.gen1trig()
        else:
          rc=self.vb.io.execute("SLMswstart("+n+","+spacing+")")
          self.sw2generate=0
          #if int(n)*int(spacing) > 2000: >2secs, start it in the background
          #  rc=self.vb.io.execute("SLMswstart("+n+","+spacing+")",ff=ffdummy)
          # check em. status (for cmds without restart flag):
          afterid=self.f1.after(1000, self.checkemuCmd)
      #print 'swStart:',rc
    def gen1trig(self):
      if self.sw2generate<=0:
        self.start1.resetColor() ; self.start1.setLabel(self.swLabel)
        return
      rc=self.vb.io.execute("SLMswstart(1,0)",log="NO")
      self.sw2generate= self.sw2generate-1
      self.start1N.setEntry(str(self.sw2generate))
      self.swafterid= self.tl.after(self.spacing, self.gen1trig) 
    def l1format(self):
      self.vb.io.execute("vmeopw32(L1_FORMAT,"+self.l1mfmt.getEntry()+")")
      #print self.l1mfmt.getEntry()
    def gapveto(self,setbutonly=None):
      if self.lgv==0x8:
        self.lgv=0
        self.lhcgap.config(text="LHC Gap\nVeto OFF")
        self.lhcgap.config(bg=myw.COLOR_WARNING)
        self.lhcgap.newhelp("The START signals are NOT vetoed during LHC Gap interval")
      else:
        self.lgv=0x8
        self.lhcgap.config(text="LHC Gap\nVeto ON")
        self.lhcgap.config(bg=myw.COLOR_BGDEFAULT)
        #normcol= self.lhcgap.cget("bg")
        #print "gapveto:normcol:",normcol
        self.lhcgap.newhelp("""All the START signals are vetoed during LHC Gap
interval""")
      if setbutonly==None:
        self.vb.io.execute("SLMsetstart("+str(self.lgv | int(self.startsel.getEntry()))+")")
    def loadseq(self):
      import slmcomp,time
      fnbase= self.sfile.getEntry()
      #print "loadseq:fnbase:",fnbase
      if fnbase=='' or fnbase==' ' or fnbase==self.CLICKHERE or len(string.split(fnbase,'.'))<2:
        self.vb.io.write('Choose right sequence to be loaded first...\n')
        return
      fn= os.path.join(self.slmdir,fnbase)
      if string.split(fnbase,'.')[1]=='slm':   # new format, compile first
        import slmcmp
        if self.vb.lturun2:
          run="-run2"
        else:
          run="-run1"
        self.vb.io.write('Compiling:%s -> WORK/slmseq.seq (%s)\n'%(fn,run))
        sqbin= slmcmp.Cmpslm(fn, run); 
        #print "loadseq:",slmcmp.wartext
        if slmcmp.wartext!='':
          self.vb.io.thds[0].write(slmcmp.wartext)
        if slmcmp.errtext!='':
          self.vb.io.thds[0].write(slmcmp.errtext)
          return
        sqbin.savefile(slmcmp.slmseqpath)
        fn= slmcmp.slmseqpath
        if myw.DimLTUservers.has_key(self.vb.boardName):
          self.vb.io.execute('putfile("'+fn+'")')
          time.sleep(1)
      #check sequence file:
      if self.vb.lturun2:
        run="-run2"
      else:
        run="-run1"
      slm= slmcomp.Disslm(fn, run)
      #print 'loadseq: ',slm.fileok
      self.AllowedErrors=slm.getaerrs()
      if self.ersr: self.errsel.setEntry(self.AllowedErrors)
      #print "aerrs:",self.AllowedErrors
      self.vb.io.thds[0].write(slm.getlist())
      if slm.fileok:
        #print 'loading sequence:',fn
        self.vb.io.execute("SLMload(\""+fn+"\")")
        self.moderrsel()
    def examineslm(self):
      import slmcomp
      rc= self.vb.io.execute("SLMdump()",applout="<>")   # -> WORK/slmasci.seq
      #print "examineslm:",rc
      if len(rc)>0 and rc[0] == "0":
        fnasci= "WORK/slmasci.seq"
        if myw.DimLTUservers.has_key(self.vb.boardName):
          self.vb.io.execute('getfile("'+fnasci+'")')
        if self.vb.lturun2:
          run="-run2"
        else:
          run="-run1"
        slm= slmcomp.Disslm(fnasci, run)
        aerrors=slm.getaerrs()
        #print "aerrors:0x",hex(aerrors)
        if slm.fileok:
          self.vb.io.thds[0].write("SLM (%s content, saved in WORK/slmasci.seq):\n"%run)
          self.vb.io.thds[0].write(slm.getlist())
        else:
          self.vb.io.thds[0].write("SLM not loaded with valid sequences\n")
    def seqeditor(self):
      fnbase= self.sfile.getEntry()
      #print "seqeditor:",fnbase
      if len(string.split(fnbase,'.'))==1:   # new format
        # .slm default:
        #os.system("cp -u CFG/ltu/SLM/all.slm CFG/ltu/SLM/newslm.slm")
        #fnbase='newslm.slm'
        # .seq default:
        fnbase='blabla.seq'
      fname=string.split(fnbase,'.')[0]
      if string.split(fnbase,'.')[-1]=='slm':   # slm format
        if fname=='sod' or fname=='eod' or fname=='L2a':
          self.vb.io.thds[0].write(
            "Warning: If you are going to use sod.slm,eod.slm or L2a.slm with DAQ, do not modify class2 pattern!\n")
        fn= os.path.join(self.slmdir,fnbase)
        e=Editor(fn,self.slmhelptext,self.vb)
        #print "use a text editor to edit file:%s in SLM directory"%fnbase
        return
      # .seq format. Prevent edit of sod eod L2a
      if fname=='sod' or fname=='eod' or fname=='L2a':
        self.vb.io.thds[0].write(
          "sod.seq,eod.seq,L2a.seq sequences are read-only\n")
        return
      if vb.lturun2:
        #print "run2 .seq case..."
        fn= os.path.join(self.slmdir,fnbase)
        e=Editor(fn,self.seqhelptext,self.vb)
        #print "use a text editor to edit file:%s in SLM directory"%fnbase
        return
      #------------------------------- run1 .seq case
      import popen2
      #os.chdir(self.slmdir)  don't change current directory (which is SLM)
      ltu6path= os.path.join(os.environ['VMECFDIR'],"ltu","ltu6.tcl")
      #ltu6path= os.path.join("../../../","ltu","ltu6.tcl")
      snbix= self.sfile.getIndex()
      if snbix==0:
        snb=''
      else:
        snb= string.split(self.itn[snbix][0],'.')[0]
      cmd= "(cd "+self.slmdir+"; "+ltu6path+" "+snb+")"
      #print cmd
      #os.system(cmd+' &')
      popio= os.popen(cmd, "r")
      while 1:
        wfn= string.rstrip(popio.readline())
        #print "popio:",wfn
        if wfn=="": break
        fn=string.split(wfn,":")
        if len(fn)==2:
          if fn[0]=="written":
            putfn= os.path.join("CFG/ltu/SLM/", fn[1])
            if myw.DimLTUservers.has_key(self.vb.boardName):
              self.vb.io.execute('putfile("'+putfn+'")')
      popio.close()
    def checkemuCmd(self):
      #print "checkemuCmd:"
      statx= self.vb.io.execute("vmeopr32(EMU_STATUS)")[:-1]
      if statx:
        stat= eval(statx)&1
        if stat==1:
          self.buttonsEmuActive()
          msg="emulation still active...\n"
        else:
          self.buttonsEmuNotActive()
          msg="emulation not active.\n"
      else:
        self.buttonsEmuNotActive()
        msg="emulation not active ?\n"
      self.vb.io.thds[0].write(msg)
    def startCmd(self):
      rc= self.vb.io.execute("SLMstart()",applout="<>")
      #print "rc[]:",rc
      if len(rc)<1:
        print "startCmd rc:", rc 
        return
      else:
        if rc[0] == "0":   #self.checkemuCmd()
          afterid=self.f1.after(100, self.checkemuCmd)
        else:
          print "startCmd rc:", rc 
          return
      # Break loop:
      irs= self.repbreaks.getEntryBin(0)
      self.breaksp= self.breakspacing.getEntryBin(1)
      #print "breakloop:",irs,self.breaksp
      if irs >= 1:
        #plan next Break:
        afterid= self.f1.after(self.breaksp, self.checkbreakCmd)
      # Emu start loop:
      rs= self.repetitions.getEntry()
      try: #if rs.isdigit():
        irs= int(rs) #eval(rs)
      except:
        self.vb.io.thds[0].write("Incorrect repetition number:"+rs+"\n")
        irs=1
      srs=self.repspacing.getEntry()
      try: #if srs.isdigit():
        self.isrs= int(srs) #eval(srs)
      except:
        self.vb.io.thds[0].write("Incorrect spacing number:"+srs+"\n")
        isrs=1
      if irs > 1:
        #plan next Emulation start:
        afterid= self.f1.after(self.isrs, self.checkstartCmd)
      else:
        afterid=self.f1.after(100, self.checkemuCmd)
    def checkstartCmd(self):
      #print "checkemuCmd:"
      irs= eval(self.repetitions.getEntry())
      stat= eval(self.vb.io.execute("vmeopr32(EMU_STATUS)")[:-1])&1
      if irs > 1:
        self.buttonsEmuActive()
        irs= irs-1
        if stat==1:
          msg="emulation still active (not restarted), remaining restarts:%d.\n"%(irs-1)
        else:
          rc= self.vb.io.execute("SLMstart()",applout="<>")
          msg="emulation restarted, still to be restarted:%d.\n"%(irs-1)
        self.repetitions.setEntry(str(irs))
        afterid= self.f1.after(self.isrs, self.checkstartCmd)
      else:
        if stat==1:
          self.buttonsEmuActive()
          msg="emulation still active, to be restarted:%d.\n"%(irs-1)
        else:
          self.buttonsEmuNotActive()
          msg="emulation not active.\n"
      self.vb.io.thds[0].write(msg)
    def checkbreakCmd(self):
      brs= self.repbreaks.getEntryBin(0)
      #print "checkbreakCmd:",brs
      if brs>0:
        rcl= self.vb.io.execute("SLMbreak()",applout="<>")
        #print rcl, "type:", type(rcl)
        brs= brs-1
        self.repbreaks.setEntry(str(brs))
        if (eval(rcl[0]) & 0x1)!=0x1:
          msg="emulation not active, break thread stopped.\n"
          self.vb.io.thds[0].write(msg)
        else:
          afterid= self.f1.after(self.breaksp, self.checkbreakCmd)
    def buttonsEmuActive(self):
      self.butstart.disable(); self.lsb.disable(); self.lsbex.disable()
      self.butquit.enable()
      self.start1.enable()
      self.butbreak.enable()
    def buttonsEmuNotActive(self):
      self.butstart.enable(); self.lsb.enable(); self.lsbex.enable()
      self.butquit.disable()
      self.start1.disable()
      self.butbreak.disable()
    def breakCmd(self):
      print 'breakCmd:'
      rcl= self.vb.io.execute("SLMbreak()")
      self.checkemuCmd()
    def quitCmd(self):
      #print 'quitCmd:'
      rcl= self.vb.io.execute("SLMquit()")
      self.checkemuCmd()
      #self.quitCmdButs()
    #def quitCmdButs(self, emust='0'):
      #self.vb.io.thds[self.thdqc].plock(0)
      #self.checkemuCmd()
    def errOnDemand(self, tx):
      #print "errOnDemand",tx
      self.vb.io.execute("ERdemand("+tx+")")
    def hereiam(self):
      print "ltu6 hereiam"
      #ps -o pid,ppid,cmd
      #kill -SIGHUP 1954
  return(CtpEmulator(vb))

#--------------------------- Master interface test with TMX
def CheckWithTMX(vb):
  class st:
    def __init__(self, vb):
      self.vb=vb
      # check the presence of the TMX board:
      self.tmx= vb.crate.findBoard("tmx")
      if self.tmx==None:
        #print "CheckWithTMX:",xy
        myw.MywError("tmx board missing")
        return
      if self.tmx.io==None:
        self.tmx.openCmd()
      self.tl=Toplevel(vb.master)
      self.tl.title("Master interface check with TMX board")
      #
      self.f1= myw.MywFrame(self.tl,side=TOP)
      self.passes= myw.MywEntry(self.f1, label="Passes",
        helptext='i.e. Words*Passes write/read/check cycles in total',
        defvalue='1',side=TOP)
      self.words= myw.MywEntry(self.f1, label="Words",
        helptext='# of words tested in 1 pass (max. 0x20000)', 
        defvalue='0x20000',side=TOP)
      wrc= myw.MywButton(self.f1, "Start",
        cmd= self.start, side=BOTTOM, helptext=
        "Start write/read/check.\nEach pass starts with different seed for generator of random values")
    def start(self):
      #print "Starting"
      #clear address counter:
      self.tmx.io.execute("clearac()")
      #write through LTU master interface to tmx board:
      seed=3
      for pas in range(0,int(self.passes.getEntry())):
        #print 'seed:',seed
        self.vb.io.execute("MasterWrRnd("+str(seed)+","+self.words.getEntry()+")",
          "out","no") 
        #read/check words from tmx (and clearac() ):
        self.tmx.io.execute("rTMNcheck("+str(seed)+","+self.words.getEntry()+")",
          "out","no") 
        seed= seed+1
  return(st(vb))

def setCurrents(vb):
  import cfgedit
  curdefs= cfgedit.Variables(vmeb=vb)
  curdefs.show()

def setDefaults(vb):
  fn="CFG/ltu/ltuttc.cfg"
  helptext="""Default values saved in database, loaded only once -when the
crate is powered on.
"""
  if myw.DimLTUservers.has_key(vb.boardName):
    vb.io.execute('getfile("'+fn+'")')
  e=DefaultsEditor(fn,helptext,vb,initialdir="CFG/ltu",suffix="*.cfg")
  return
def setGLOBAL(vb):
  class setGLOBAL:
    def __init__(self, vb):
      vb.io.execute("setglobalmode()")
      vb.setColor(myw.COLOR_WARNING)
  return(setGLOBAL(vb))
def setSTDALONE(vb):
  class setSTDALONE:
    def __init__(self, vb):
      vb.io.execute("setstdalonemode(1)")
      vb.setColor("original")
  return(setSTDALONE(vb))

class SOFTLEDS:
  SWLEDS=(("R","VME read"), ("W","VME write"), ("A","SCOPE-A"), 
          ("B","SCOPE-B"),
          (".","""'continuous update' indication, i.e. there is 
SW VME read every half second if this label is moving.
Blue color indicates 'no update' -i.e. the color of R W A B fields
shows the status from last update.
Click on this button to get 4 associated fields (VMEread, 
VMEwrite and 2 signals) updated for another minute. 
The color of 4 fields (R W A B) is red if choosen signal
is changing during update.
"""))
  #ooo=("-","\\","|","/")
  ooo=("\\","/")
  def __init__(self, vb, frame):
    self.vb=vb
    self.ooo=0
    self.active=0   # 1: update active   0: update not active
    self.afteridCount= 0
    if frame==None:
      self.f1=myw.NewToplevel("Soft LEDS")
    else:
      self.f1= frame
    self.labels=[]
    for ix in range(4):
      self.labels.append(myw.MywLabel(self.f1, SOFTLEDS.SWLEDS[ix][0],
        SOFTLEDS.SWLEDS[ix][1], fill='both', side=LEFT))
    self.labels.append(myw.MywButton(self.f1, SOFTLEDS.SWLEDS[4][0],
      helptext=SOFTLEDS.SWLEDS[4][1], side=LEFT, cmd=self.updateOnOff))
    self.updateOnOff()
  def updateOnOff(self):
    self.active=1-self.active
    if self.active:
      self.labels[4].resetColor()
      self.update()
    else:
      self.labels[4].setColor("blue")
      self.afteridCount= 0
  def update(self):
    self.sls= self.vb.io.execute("getSWLEDS()", log="NO")   # "1011"
    #print self.sls
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
      #if self.afteridCount > 240:   # stop after 2 minutes
      if self.afteridCount > 8:   # stop soon
        self.updateOnOff()

#--------------------------- Scope outputs selection:
def ScopeAB(vb):
  class st:
    def __init__(self, vb):
      self.vb=vb
      ltuver= self.vb.io.execute("getgltuver()",applout="<>")[0]
      #print 'ScopeAB:',ltuver, type(ltuver)
      self.tl=myw.NewToplevel("Oscilloscope signals selection")
      #self.f1= myw.MywFrame(self.tl,side=TOP)
      #self.f12= myw.MywFrame(self.f1,relief=FLAT,side=BOTTOM)
      ab=self.vb.io.execute("vmeopr32(SCOPE_SELECT)")[:-1]
      defa= eval(ab)&0x1f
      defb= (eval(ab)&0x3e0)>>5
      if(int(ltuver) & 0xf0)==0xb0:
        #print "LTUvi"
        itemsA=(
          ("BC(LTU,delayed) also at B","0",self.selectAB),
          ("Orbit, 1us pulse, also at B","1",self.selectAB),
          ("Pre_Pulse, 25ns, no errors","2",self.selectAB),
          ("L1 strobe, including errors","3",self.selectAB),
          ("L2 Strobe, including errors","4",self.selectAB),
          ("Pulser(input)","5",self.selectAB),
          ("BUSY1(input)","6",self.selectAB),
          ("BUSY2(input)","7",self.selectAB),
          ("VME Read","8",self.selectAB),
          ("VME Write","9",self.selectAB),
          ("VME STROBE (read or write)","10",self.selectAB),
          ("TTCchannel B output, no delay","11",self.selectAB),
          ("BusyProbe BUSY_LONG (25ns) also a B","12",self.selectAB),
          ("ADC(input)","13",self.selectAB),
          ("Err Request (set by ErrOnDemand Option)","14",self.selectAB),
          ("GND","15",self.selectAB),
          ("GND","16",self.selectAB),
          ("START ALL signal(emulator, sel. START option)","17",self.selectAB),
          ("L0, no errors","18",self.selectAB),
          ("VMEclock (16MHz)","19",self.selectAB),
          ("TTCchannel A output, no delay","20",self.selectAB),
          ("L1only (L0 over TTC), no errors","21",self.selectAB),
          ("Orbit inhibit,44BCs,precedes ORBITtransmit at B","22",self.selectAB),
          ("PP inhibit,44BCs,precedes PPtransmit at B","23",self.selectAB),
          ("VME FIFO write strobe (25ns)","24",self.selectAB),
          ("TTC CahnnelB transmission in progress","25",self.selectAB),
          ("Interlock BUSY (RATE_LIMIT condition)","26",self.selectAB))
        itemsB=(
          ("BC(LTU,delayed) also at A","0",self.selectAB),
          ("BC(input), no delay","1",self.selectAB),
          ("Channel B out, delayed, includes errors","2",self.selectAB),
          ("PPrequest, includes errors (LTU output not used)","3",self.selectAB),
          ("L0 (output), includes errors","4",self.selectAB),
          ("Channel A output, delayed, includes errors,L0+L1 if L0overTTC","5",self.selectAB),
          ("L1 Data","6",self.selectAB),
          ("L2 Data","7",self.selectAB),
          ("START of the emulation sequence","8",self.selectAB),
          ("LTU internal BUSY, all constituents ","9",self.selectAB),
          ("L1 FIFO Empty status","10",self.selectAB),
          ("L2 FIFO Empty status","11",self.selectAB),
          ("L1 FIFO Nearly Full status","12",self.selectAB),
          ("L2 FIFO Nearly Full status","13",self.selectAB),
          ("Any Error","14",self.selectAB),
          ("L1 Strobe (from emulator)","15",self.selectAB),
          ("L2 Data (from emulator)","16",self.selectAB),
          ("Orbit, 1us pulse, also at A","17",self.selectAB),
          ("L2A strobe","18",self.selectAB),
          ("L2R strobe","19",self.selectAB),
          ("BCerror flag","20",self.selectAB),
          ("L1, includes errors","21",self.selectAB),
          ("L1 DATA","22",self.selectAB),
          ("Pending PP request","23",self.selectAB),
          ("Orbit transmission in progress (17BCs)","24",self.selectAB),
          ("PP transmission in progress (17BCs)","25",self.selectAB),
          ("L1word transmission in progress (43BCs)","26",self.selectAB),
          ("L2word transmission in progress (43BCs)","27",self.selectAB),
          ("VME TTC transmission in progress (43BCs)","28",self.selectAB),
          ("VME TTC FIFO empty status","29",self.selectAB),
          ("BusyProbe BUSY_LONG (25ns) also a A","30",self.selectAB))
      else:
        itemsA=(
          ("BC(LTU,delayed)","0",self.selectAB),
          ("Orbit(input from CTP or LTU emulator)","1",self.selectAB),
          ("Pre_Pulse(input from CTP or LTU emulator)","2",self.selectAB),
          ("L1 strobe","3",self.selectAB),
          ("L2 Strobe","4",self.selectAB),
          ("Pulser(input)","5",self.selectAB),
          ("BUSY1(input)","6",self.selectAB),
          ("BUSY2(input)","7",self.selectAB),
          ("VME Read","8",self.selectAB),
          ("VME Write","9",self.selectAB),
          ("VME STROBE","10",self.selectAB),
          ("DWB(Device Wants Bus)","11",self.selectAB),
          ("MSTROBE","12",self.selectAB),
          ("ADC(input)","13",self.selectAB),
          ("Err Request (set by ErrOnDemand Option)","14",self.selectAB),
          ("TTC MS(serial TTCvi out, 1+8 most sign. bits)","15",self.selectAB),
          ("Leaky Bucket Halt","16",self.selectAB),
          ("START ALL signal(emulator, sel. START option)","17",self.selectAB),
          ("L0","18",self.selectAB),
          ("VMEclock (16MHz)","19",self.selectAB),
          ("GND","20",self.selectAB),
          ("L1only (L0 over TTC)","21",self.selectAB),
          ("GND","22",self.selectAB),
          ("not selected","23",self.selectAB))
        itemsB=(
          ("BC(LTU,delayed)","0",self.selectAB),
          ("BC(input)","1",self.selectAB),
          ("Orbit(output)","2",self.selectAB),
          ("Pre_Pulse(output)","3",self.selectAB),
          ("L0 (output)","4",self.selectAB),
          ("L1 (output)","5",self.selectAB),
          ("L1 Data","6",self.selectAB),
          ("L2 Data","7",self.selectAB),
          ("Trigger sequence START","8",self.selectAB),
          ("LTU BUSY (output)","9",self.selectAB),
          ("L1 FIFO Empty","10",self.selectAB),
          ("L2 FIFO Empty","11",self.selectAB),
          ("L1 FIFO Nearly Full","12",self.selectAB),
          ("L2 FIFO Nearly Full","13",self.selectAB),
          ("Any Error","14",self.selectAB),
          ("L1 Strobe (from emulator, before delay)","15",self.selectAB),
          ("L2 Data (from emulator, before delay)","16",self.selectAB),
          ("TTC LS(serial TTCvi out, 1+8 least sign. bits","17",self.selectAB),
          ("L2A","18",self.selectAB),
          ("L2R","19",self.selectAB),
          ("BCerror flag","20",self.selectAB),
          ("L1only (L0 over TTC)","21",self.selectAB),
          ("GND","22",self.selectAB),
          ("not selected","23",self.selectAB))
      self.selA= myw.MywxMenu(self.tl, label='A:',
        helptext="""Scope A output """,
        defaultinx=defa, side=TOP, items=itemsA)
      self.selB= myw.MywxMenu(self.tl, label='B:',
        helptext="""Scope B output """,
        defaultinx=defb, side=TOP, items=itemsB)
      f1= myw.MywFrame(self.tl,side=TOP)
      SOFTLEDS(self.vb, f1)
    def selectAB(self):
      #print "selectAB:",self.selA.getEntry(),self.selB.getEntry()
      self.vb.io.execute("setAB("+ self.selA.getEntry()+","+
        self.selB.getEntry()+")", "out","no")
  return(st(vb))

#--------------------------- Snapshot memory
def Snapshot_memory(vb):
  class st:
    def __init__(self, vb):
      self.wrkdir=os.path.join(os.environ['VMEWORKDIR'],"WORK")
      self.vb=vb
      self.tl=myw.NewToplevel("Snapshot memory")
      self.fss= myw.MywFrame(self.tl,side=TOP)   #start/schedule frame
      self.fsq= myw.MywFrame(self.tl,side=TOP)   #stop/quit/show memory fr.
      self.Safter=myw.MywButton(self.fss, label="Start After (26ms)",
        helptext="Start recording in AFTER mode (26ms)", state=NORMAL,
        side=TOP,cmd=self.Cafter)
      self.SafterTTCrxreset=myw.MywButton(self.fss, label="Start After + TTCrxreset",
        helptext="Start recording in AFTER mode (26ms) + TTCrxreset", 
        state=NORMAL, side=TOP,cmd=self.CafterTTCrxreset)
      self.Sbefor=myw.MywButton(self.fss, label="Start Before (continuous)",
        helptext="""Start recording in BEFORE mode i.e. continuous recording.
To stop 'Before mode' press 'Quit emulation' in
CTP_emulator window""", state=NORMAL,
        side=TOP,cmd=self.Cbefore)
      schopts=(
        ("not scheduled","0",self.Cschedule), 
        ("Start After when StartSignal Selected","2",self.Cschedule), 
        ("Start Before when StartSignal Selected","3",self.Cschedule))
      self.Sschedule=myw.MywxMenu(self.fss, label="Scheduled start: ",
        helptext="""Start of recording (in After/Before mode)
is postponed until:
- Start Signal Selection (pulser,random, BC scaled down) or
  1st SW Start Signal generated. 
  'After mode' stops by itself after about 26 ms,
  'Before mode' stops by pressing 'Quit emulation' button
 
""", 
        items=schopts)
      self.Sfp2ssm=myw.MywButton(self.fss, label="Wait L0, startAfter FP2ssm",
        helptext="""Wait for a change of L0 or PP counters, then
- set FP2ssm mode (7 front panel connector signals caught in SSM
  regardless of the LTU global/stdalone mode), and
- start recording in AFTER mode (26ms)
""", state=NORMAL, side=TOP,cmd=self.CL0after)
      text="""Human readable file. Syntax is following:
BC name/length message
BC - bunch crossing number measured from the begining of snapshot memory
name - signal names for ltu: 
       ORBIT,PP,L0,L1S,L1DATA,L2S,L2DATA,SBUSY,ALLBUSY,
       L1NF,L2NF,LBHALT,MSTROBE,TTC LS,TTC MS,VMES,ALLSTART,ANYERR
       signal names for ltuvi: 
       ORBIT,PP,L0,L1S,L1DATA,L2S,L2DATA,SBUSY,ALLBUSY,
       L1NF,L2NF,ChanA,ChanB,TTCBUSY,PPT,VMES,ALLSTART,ANYERR
       
length - for some signals indicates the length of the signals
message - for some signals contains data
ORBIT/40  - orbit length is 40 BC
PP - prepulse request
L0 - L0 output
L1S - L1 strobe
L1DATA:008.000.000.000.004 TrCl: 0x2000000000001 -   
      58 serial data bits        Trigger Classes
L2S - L2 strobe
L2DATA:4e1.e1d.d13.006.000.000.000.001 TrCl: 0x2000000000001 - 
       97 serail data bits                   Trigger classes
SBUSY - subdetector busy
ALLBUSY - LTU internal busy (all constituents)
L1NF - L1 message FIFO nearly full status
L2NF - L2 message FIFO nearly full status
ChanA/1 - channel A (L0) 
ChanA/2 - channel A (L1)
ChanB OP:0x1 0x13  - channel B orbit
ChanB OP:0xfc 0x12  - channel B pre pulse
ChanB L1h :0x0000 1 0x1 0x008 0x77 - channel B
           data   E      data check
                   header
TTCBUSY/43 - TTC busy and its length
PPT - PP transmision in progress (length=17)
VMES - vme strobe
ALLSTART - All emulation start
ANYERR - any error signal
"""
      #self.Sschedule.enable()
      #self.Sschedule.radf.config(anchor=S)
      self.Sstop=myw.MywButton(self.fsq, label="Stop",
        helptext="Stop recording", state=NORMAL,
        side=LEFT,cmd=self.Cstop)
      self.Sdump=myw.MywButton(self.fsq, label="Dump",
        helptext="""Dump snapshot memory to the WORK/ssm.dump 
and create human readable file""", state=NORMAL,
        side=LEFT,cmd=self.Cdump)
      self.Sshow=myw.MywButton(self.fsq, label="Show",
        helptext=text, state=NORMAL,
        side=LEFT,cmd=self.Cshow)
    def Cafter(self):
      #print 'Cafter:'
      rcl= self.vb.io.execute("SSMstartrec(2)")
      #self.quitCmdButs()
    def CafterTTCrxreset(self):
      rcl= self.vb.io.execute("SSMstartrec(0x102)")
    def CL0after(self):
      print 'CL0after:'
      self.vb.io.execute("SSMstartrec(0x12)",ff=ffdummy)
      #self.quitCmdButs()
    def Cbefore(self):
      #print 'Cbefore:'
      rcl= self.vb.io.execute("SSMstartrec(3)")
      #self.quitCmdButs()
    def Cstop(self):
      #print 'Cstop:'
      rcl= self.vb.io.execute("SSMstoprec()")
    def Cschedule(self):
      #print 'Cschedule:'
      #following thread has to be #0 thread (the same for 
      # buttons startsel/star1 in CTP_emulator class above) 
      # see SLMsetstart(),SLMswstart() routines
      p=self.Sschedule.getEntry()
      self.vb.io.execute("SSMschedule("+p+")")
      #self.quitCmdButs()
    def Cdump(self):
      import ssman,platform
      if platform.machine()=="x86_64":
        lindir= "lin64"
      else:
        lindir= "linux"
      #print 'Cshow:'
      rcl= self.vb.io.execute("SSMdump()",applout='<>')
      #print rcl
      if len(rcl)==1 and rcl[0]=='1':
        #log= ssman.makelst()
        fnasci= "WORK/SSM.dump"
        if myw.DimLTUservers.has_key(self.vb.boardName):
          self.vb.io.execute('getfile("'+fnasci+'")')
        #get ltu version
        output=self.vb.io.execute("getgltuver()",applout='<>')
        ltuver=hex(int(output[0]))
        #print ltuver
        if ltuver >= '0xb7':
          plog= os.popen(os.path.join(os.environ['VMECFDIR'],"SSMANA",lindir,"ssmanls1.exe"),"r")
        else:
          plog= os.popen(os.path.join(os.environ['VMECFDIR'],"SSMANA",lindir,"ssmanv.exe"),"r")
        log= plog.read()
        rc= plog.close()
        self.vb.io.thds[0].write(log)
        self.vb.io.thds[0].write("rc="+str(rc)+"\n")
    def Cshow(self):
      #fe= Editor(self.vb.master,"WORK/SSMa.txt",self.vb)
      fe= Editor("WORK/SSMa.txt",vb=self.vb)
  return(st(vb))
class Editor:
  """
  Usage:
  fe= Editor("WORK/SSMa.txt", helptext)
  """
  def __init__(self, filename, hlptext=None, vb=None, master=None,
    initialdir="CFG/ltu/SLM", suffix="*.slm"):
    #self.vb.io.thds[0].write("Use any text editor\n")
    try:
      f= open(filename);
    except:
      print "File %s cannot be opened"%filename  
      return
    self.filename= filename
    self.vb= vb
    self.suffix= suffix
    self.initialdir= initialdir
    self.tl=Toplevel(master); self.tl.title(filename)
    self.tl.transient(master)
    f1= myw.MywFrame(self.tl, side=TOP)
    self.f2= myw.MywFrame(self.tl, side=TOP)
    self.textview   = Text(f1, bg = "#ccffff")
    scrollview = Scrollbar(f1, command = self.textview.yview)
    self.textview.pack(side = "left")
    scrollview.pack(side = "right", fill = "y")
    myw.MywHelp(f1, hlptext, self.textview)
    self.textview.configure(yscrollcommand = scrollview.set)
    self.textview.delete("1.0", "end")
    self.textview.insert("end", f.read())
    f.close()
    self.doButtons()
  def doButtons(self):
    sq= myw.MywHButtons(self.f2, buttons= [
      ("Save as",self.save),
      ("Quit",self.quit)]
    )
  def save(self):
    #f= open(self.filename,"w");
    OpenSave=tkFileDialog.SaveAs(
      #initialdir= os.environ['VMEWORKDIR'] +"/CFG/ltu/SLM/",
      initialdir= self.initialdir,
      initialfile=os.path.basename(self.filename),
      filetypes=[("CTPemu text files",self.suffix), ("all files","*")])
    f2save= OpenSave.show()
    #print "Editor:",f2save,type(f2save)
    # Editor: /ram/home/alice/trigger/v/vme/CFG/ltu/SLM/zzz.slm <type 'unicode'>
    if (type(f2save)==types.StringType) or (type(f2save)==types.UnicodeType):
      cfgltuslmName=os.path.join(self.initialdir,os.path.basename(f2save))
      f= open(f2save,"w")
      #txt=self.textview.get("1.0","end")
      #print "Editor:saving:%s:%s:\n"%(f2save, txt)
      f.write(self.textview.get("1.0","end"))
      f.close()
      if myw.DimLTUservers.has_key(self.vb.boardName):
        self.vb.io.execute('putfile("'+cfgltuslmName+'")')
      self.vb.io.write('File '+cfgltuslmName+' was saved.\n')
  def quit(self):
    self.tl.destroy()
class DefaultsEditor(Editor):
  def doButtons(self):
    sq= myw.MywHButtons(self.f2, buttons= [
      ("Save to Defaults DB and quit",self.save),
      ("Quit",self.quit)]
    )
  def save(self):
    f2save=self.filename
    f= open(f2save,"w")
    f.write(self.textview.get("1.0","end"))
    f.close()
    cfgltuslmName=os.path.join(self.initialdir,os.path.basename(f2save))
    if myw.DimLTUservers.has_key(self.vb.boardName):
      self.vb.io.execute('putfile("'+cfgltuslmName+'")')
    self.vb.io.write('File '+cfgltuslmName+' saved.\n')
    self.quit()
   
#--------------------------- Counters
# to be removed (old version valid for F2 LTU fpga)
#class Counts(myw.MultRegs):
#  def __init__(self,vb):
#    myw.MultRegs.__init__(self, vb)
#  def addReg(self, inx=None):
#    #print "addReg:", self.rgitems
#    #self.addr.printEntry('MultRegaddReg:')
#    if inx==None: inx= self.addr.getIndex()
#    r= myw.MywEntry(self.regsframe, label=self.rgitems, 
#      delaction=self.destroyReg, defaultinx=inx,
#      side=TOP)   #, cmdlabel=self.chooseReg)
#    r.convertStart()
#    self.regs.append(r)
#  def addCCB(self):
#    ccb= myw.MywButton(self.addqframe,label='Clear',cmd=self.cc,
#      helptext= 'Clear counters', side=RIGHT)
#  def cc(self):
#    self.board.io.execute("SLMclearcnts(0xfff)")
def Counters(vb):
#  class st:
#    def __init__(self, vb):
#      cnts= Counts(vb); cnts.addCCB()
#      for i in range(50,62):   # ! L1FIFO_COUNTER is 50
#        cnts.addReg(i)
  ltuver= vb.io.execute("getgltuver()",applout="<>")[0]
  if (int(ltuver) & 0xf0)==0xb0:
    st=counters.VMEcnts(counters.LTUvicnts) 
  else:
    st=counters.VMEcnts(counters.LTUcnts) 
  return

def ADC_Scan(vb):
  class st:
   def __init__(self,vb):
    import mywrl
    self.vb=vb
    self.tl=myw.NewToplevel("ADC")
##########################################################
    if(self.checkbcclock()):
     self.f1=myw.MywFrame(self.tl,side=TOP)
     cmd="rndtest()"
     output=self.vb.io.execute(cmd,log="no",applout="<>")
     xy=self.xy(output)
     #print 'xy=',xy
     max=self.finddelay(xy)

     self.c1=mywrl.Graph(self.f1,x0=0.,y0=0.,
           xgraph=32.,nxtick=8,ygraph=130.,nytick=10)
     self.c1.plot(xy)
     self.c1.xlabel(text='Delay [ns]')
     self.c1.ylabel(text='ADC')
     self.c1.pack()
     f2=myw.MywFrame(self.tl)
     b0=myw.MywButton(f2,label='Cancel',cmd=self.tl.destroy,side=LEFT,
        helptext='Close the window without accepting the value.')
     b2=myw.MywButton(f2,label="Measure",cmd=self.measure,side=LEFT,
        helptext="""Measure points again, i.e. read ADC for randomly
choosen values for BC_DEALY_ADD  (100 values).""")       
     self.en=myw.MywEntry(f2,label="DELAY:",defvalue=str(max), helptext=\
""" Value in this entry field is: Delay-2 [ns] where
Delay corresponds to the edge (sharp declination of the ADC count).
WARNING:
This value corresponds to right setting of BC_DEALY_ADD register
for the negative edge sampling of the front panel signals.
Due to requirement of having good strobing in both modes (global and stdalone),
BC_DELAY_ADD value has to be corrected according to 
TTC_INTERFACE register setting -i.e. BC_DELAY_ADD should be set
by calling setBC_DEALY_ADD(). See 'Timing of the ALICE TTC partition -
 LTUvi version' and setBC_DELAY_ADD() routine where the correction 
is implemented.
""")
     b1=myw.MywButton(f2,label="OK",cmd=self.ok,side=LEFT,
       helptext="""
Before this widget is closed, the DELAY: entry field is
taken to calculate corrected value
and saving it into BC_DELAY_ADD register (setBC_DEALY_ADD).
I.e.: if you want to set different value in this register, use Cancel button
to close this widget, and then set it in stdfuncs->BC_DELAY_ADD->write.
""")
     b3=myw.MywButton(f2,label='Save plot',cmd=self.save,side=LEFT,
        helptext="Save plot in directory $VMEWORKDIR/WORK") 
############################################################################     
   def checkbcclock(self):
    """
       Check id BC clock is present and ready.
    """
    cmd="getbcstatus()"
    output=self.vb.io.execute(cmd,log="no",applout="<>")
    #print "output=",output,' ',output[0]
    if (output[0] != '0x2'):
       myw.MywError(errmsg="BC clock is not present, staus="+output[0])
       self.tl.destroy()
       return 0
    return 1
   def save(self):
    """
       Save the postscript file of the plot to WORK directory.
    """
    fn=os.path.join(os.environ['VMEWORKDIR'],
        "WORK","adc.ps")
    rc=self.c1.postscript(file=fn)
    if rc is not '':
     myw.MywError(errmsg="Directory WORK does not exist.")
     print "rc=",rc,len(rc)
   def measure(self):
    """
      Measure and plot points again.
    """
    import mywrl
    if(self.c1):
      self.c1.destroy()
      self.c1=None
    if(self.checkbcclock()):
     cmd="rndtest()"
     output=self.vb.io.execute(cmd,log="no",applout="<>")
     xy=self.xy(output)
     max=self.finddelay(xy)
     self.c1=mywrl.Graph(self.f1,x0=0.,y0=0.,xgraph=32.,nxtick=8,
                   ygraph=130.,nytick=10)
     self.c1.plot(xy)
     self.c1.xlabel(text='Delay [ns]')
     self.c1.ylabel(text='ADC')
     self.c1.pack()
     self.en.setEntry(str(max))
   def ok(self):
    """
       Accept delay value and destroy ADC window.
    """
    import re; isNumber=re.compile(r"^[-+]\d+$").match;
    delay=self.en.getEntry()
    if isNumber(delay):
      cmd="setbcdelay("+delay+",0)"   # set BC_DELAY_ADD with corrections!
      self.vb.io.execute(cmd)
      #print 'Value accepted ',delay
      #self.setbcdefault(delay)
    else:
      self.vb.io.write("%s -not int, not saved\n"%delay)  
    self.tl.destroy()
   #def setbcdefault(self, bcd):
   #  mn= os.path.join(os.environ['VMEWORKDIR'],"CFG","ltu","init.mac")
   #  mf= open(mn); lines= mf.readlines(); mf.close()
   #  i=0
   #  for l in lines:
   #    funcsp= string.split(l,"(")
   #    #print ":",funcsp
   #    if funcsp[0]=="setbcdelay":
   #      lines[i]= funcsp[0]+"("+bcd+")\n"
   #      #print i,":",lines[i]
   #    i=i+1
   #  #print lines
   #  mf= open(mn,'w'); mf.writelines(lines); mf.close()
   #  self.vb.io.thds[0].write("BC delay "+bcd+" Stored in CFG/ltu/init.mac file\n")
   def xy(self,output):
    """
       Transforms self.vb.io output (list) to list of 2-tuples.

       One 2-tuples correspond to (delay,ADC).There
       may be many points with the same dealy.
       Convert strings to floats.
    """
    #output[-2] -average in micsec waiting for PLL unlock
    #output[-1] -average in micsec waiting for PLL lock
    self.vb.io.write("average unlock wait:%s average total lock wait:%s\n"
      %(output[-2],output[-1]))
    ll=len(output)-2
    if ll:
      xy=[]
      for i in range(0,ll,2):
        xynow=(float(output[i]),float(output[i+1]))
        flag=1
        for j in xy:
          if (j == xynow):
           flag=0
        if flag:
          xy.append(xynow)
    else:
       xy=None
    return xy
   def finddelay(self,xy):
     """ Try to find delay from measured data.
        xy: [[delay1,adc1], [delay2,adc2],...]
        This version works only with 2 mirror curves.
     """
     if xy:
      listx,listy=self.sort(xy)
      #print 'listx=',listx
      #print 'listy=',listy
      delay=[]
      n=len(listx)
      for i in range(1,n,1):
        der=(listy[i][0]-listy[i-1][0])/(listx[i]-listx[i-1])
        if(abs(der)>30.):
          delay.append(int(listx[i]-1))
      print "delay=",delay
      if (delay==[]): delay=None
      return delay
     else:
      return None
   def sort(self,xy):
     """
        1.) Find x  2.) For every x find all y
     """
     xy.sort()
     #print xy
     x0=xy[0][0]   # x of first tuple
     listy=[]      # list of list of y values for given x
     listx=[]      # list of x values
     ll=[]
     for i in xy:
       if(i[0] == x0):      # change of x
         ll.append(i[1])
       else:
         listy.append(ll)
         listx.append(x0)
         ll=[]
         ll.append(i[1])
         x0=i[0]
     listy.append(ll)
     listx.append(x0)
     return listx,listy
   def evaluate(self,listx,listy):
    """
       Find numberof delays with 0,1 and more than 1 entries:
       numof0,numof1,numof2. This will be used for the decision
       how to find delay.
    """
    numof0,numof1,numof2=0,0,0
    for i in range(0,32,1): #i is never 32
      if listx.count(i):
        j=listx.index(i)
        ll=len(listy[j])
        if ll == 1: numof1=numof1+1
        else: numof2=numof2+1
      else:
        numof0=numof0+1
    return numof0,numof1,numof2
   def find2(self,listx,listy):
     j,gmax,jmax=0,0,0
     for i in listy:
       numofy=len(i)    # number of points for given delay
       _min,_max=1.e23,-1.e23
       if numofy>1:
          for l in i:
            if(_min>l): _min=l
            if(_max<l): _max=l
   #       print listx[j],_max-_min
   #    elif numofy == 1:
   #       if( (i[0] == 0) | (i[0] >128)): 
   #	     _max,_min=129,0 

       if( (_max-_min) > gmax):
           gmax=_max-_min
           jmax=j
       j=j+1
     return jmax
  st(vb)    
  return None

def CheckBusy(vb):
  class Cb:
    def __init__(self, vb):
      self.vb=vb
      self.getbs()
      self.tl=myw.NewToplevel("Check busy sources")
      self.fbusy= myw.MywFrame(self.tl,side=TOP)   # busys frame
      self.fcq= myw.MywFrame(self.tl,side=TOP)   # check/quit
      self.busys=myw.MywBits(self.fbusy,label="Active BUSYs",
        defval= self.busystatus, cmd=self.modbusys,
        helptext="""BUSY_STATUS bits:
B1ENA, B2ENA -LTU BUSY* input is ENABLED if B*ENA is ON (red)
L1, L2       -L1 or L2 fifo full
SW           -busy set by software
BUSY1, BUSY2 -current status of LTU BUSY inputs
BUSY         -current status of LTU BUSY output. The YELLOW LED
              on fron panel shows the status of the signal
              - on the BUSY output connector in GLOBAL mode
              - inside the LTU, before GLBOAL/STDALONE selector
                in STDALONE mode (in STDALONE mode, the BUSY
                output connector is always HIGH which means 'busy'
B1ENA, B2ENA, SW bits can be set by software.

Yellow BUSY LED on LTU front panel is reflecting the resulting
BUSY from all possible sources (BUSY1/2 + internal leaky-bucket
mechanism preventing the TTCvi overflow).
""",
        bits=["B1ENA -BUSY1 enabled", "B2ENA -BUSY2 enabled", 
          ("L1    -L1fifo full",0),
          ("L2    -L2fifo full",0),
          "SW    -SW BUSY",
          ("BUSY1 -BUSY1 input status",0), 
          ("BUSY2 -BUSY2 input status",0),
          ("BUSY  -BUSY output status",0)],
        side=TOP)
      self.butquit=myw.MywButton(self.fcq, label="Check",
        helptext="""
Check BUSY_STATUS register (i.e. update 'Active BUSYs' button)""",
side=LEFT,cmd=self.checkbs)
      self.butquit=myw.MywButton(self.fcq, label="Quit",
        helptext="QUIT this window", side=RIGHT,cmd=self.quitcb)
      self.vb=vb
    def modbusys(self):
      #print "No sense -BUSY_STATUS is read only VME register"
      nval= self.busys.getEntry()
      oval= self.busystatus
      #print "is:%x new value:%x"%(oval, nval)
      modbits= nval ^ oval
      if modbits & 0x10:   # SW BUSY set/clear
        if nval & 0x10:    # set 
          self.vb.io.execute("vmeopw32(SW_BUSY,1)")
        else:              # clear
          self.vb.io.execute("vmeopw32(SW_BUSY,0)")
      if modbits & 0x03:   # BUSY1,2 ENABLE/DISABLE
        self.vb.io.execute("vmeopw32(BUSY_ENABLE,%d)"%(nval&0x3))
      self.checkbs()
    def getbs(self):
      ab= self.vb.io.execute("vmeopr32(BUSY_STATUS)")[:-1]
      #print "CheckBusy ab:",ab
      self.busystatus= eval(ab)&0xff
      #print "getbs:%x"%(self.busystatus)
    def checkbs(self):
      self.getbs()
      self.busys.setEntry(self.busystatus)
    def quitcb(self):
      self.tl.destroy()
  Cb(vb)    
  return None

def CheckRateLimit(vb):
  class rl:
    def __init__(self, vb):
      self.vb=vb
      self.tl=myw.NewToplevel("Check/set rate limit in LTU")
      fef= myw.MywFrame(self.tl,side=TOP)   # entry fields frame
      self.interval=myw.MywEntry(fef,label="Interval (us):",
        defvalue="bla", helptext=\
"""Time interval in micsecs. During this time, triggers are counted
and compared at the end of the interval with 'Max. triggers' limit.
Interval is set in 820/100 micsecs slots, starting from 1640/200 micsecs.
Meaningfull values: 1640..209100/200..204700.
Max. interval is 209100/204700 micsecs (any higher value results in 209100/204700).

This window allows to check/set values directly in LTU. Use
RATE_LIMIT hexadecimal value shown in log window for default setting
(loaded with each new run) in Defaults editor.
""")
      self.trigs=myw.MywEntry(fef,label="Max. triggers:",
        defvalue="bla2", helptext=\
"""
Max. number of triggers allowed in any time interval
Allowed values: 0..63

""")
      self.fbuts= myw.MywFrame(self.tl,side=TOP)   # buttons
      self.buts=myw.MywHButtons(self.fbuts, helptext="""
Read or change current Rate limit.
RATE_LIMIT value shown in log window should be used for RATE_LIMIT
setting in Deafults editor window.
""",
        buttons= [("Read", self.readrl),
          ("Write", self.writerl),
          ("Disabled\Enabled", self.disenarl),
          ("Quit", self.quit)
          ])
      self.readrl()
    def readrl(self):
      #self.rali= string.split(self.vb.io.execute("RateLimit(0,0)")[:-1])
      self.rali= self.exrali("999999","0","0")
      #print "rali:", self.rali
      self.updateWindow()
    def updateWindow(self):
      self.interval.setEntry(self.rali[0])
      self.trigs.setEntry(self.rali[1])
      if self.rali[2]=='1':
        self.buts.buttons[2].setLabel('Enabled')
      else:
        self.buts.buttons[2].setLabel('Disabled')
    def writerl(self):
      self.rali[0]= self.interval.getEntry()
      self.rali[1]= self.trigs.getEntry()
      if self.buts.buttons[2].getLabel()=='Disabled':
        self.rali[2]='0'
      else:
        self.rali[2]='1'
      #self.rali= string.split(self.vb.io.execute("RateLimit(%s)"%pars)[:-1])
      self.updateHW()
      self.updateWindow()
    def updateHW(self):
      self.rali= self.exrali(self.rali[0], self.rali[1], self.rali[2])
    def disenarl(self):
      lbl= self.buts.buttons[2].getLabel()
      #print "disenarl:%s:"%lbl;
      if lbl=='Disabled':
        self.rali[2]='1'
        self.buts.buttons[2].setLabel('Enabled')
      else:
        self.rali[2]='0'
        self.buts.buttons[2].setLabel('Disabled')
      self.updateHW()
      self.readrl()
    def quit(self):
      self.tl.destroy()
    def exrali(self, interval, trigs, enadis):
       pars= interval+','+trigs+','+enadis
       outs= self.vb.io.execute("RateLimit(%s)"%pars)
       #print outs,"<<"
       o= string.split(string.split(outs,"\n")[-2])
       if len(o) != 3: o=["Internal","error", "0"]
       #print o,"<<"
       return o
       #output=self.vb.io.execute(cmd,log="no",applout="<>")

  rl(vb)    
  return None
