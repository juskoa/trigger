#!/usr/bin/python
# 18.9. byoutT now ok, byin1-24 now OK (were shifted by 1)
# 18.9. INT counters added (previous int is now l0int)
# 24.11. CGT addded in findAddr 
from Tkinter import *
import string,os, types, time,threading #, os.path, glob
import myw

#following definition must agree with ctp/ctpcounters.h
NCOUNTERS_L0=300
NCOUNTERS_L0_SP1=17  # 2 spares
NCOUNTERS_L0_SP2=178 # 9 spares. run1: was commented out (spare99 becomes l0infun4)
NCOUNTERS_L0_SP3=298 # 2 spares
L1SH=NCOUNTERS_L0
#
NCOUNTERS_L1=300 #160
NCOUNTERS_L1_SP1=38 #38   2spares
NCOUNTERS_L1_SP2=250 #148   run2:50spares
#
L2SH=NCOUNTERS_L0+NCOUNTERS_L1
NCOUNTERS_L2=300 #134
NCOUNTERS_L2_SP1=25   # run1/2: 1 spare
NCOUNTERS_L2_SP2=236  # run2 only: 64 spares
#
NFOS=6
FOSH=NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2
NCOUNTERS_FO=72       # was 34 till 11.11.2008 run1:48(+4) run2:72
#NCOUNTERS_FOae=52    #from 5.7.2012
NCOUNTERS_FO_SP1=37   # run2:  3 spares
NCOUNTERS_FO_SP2=63   # run2: 9 spares
#
NCOUNTERS_BUSY=160
NCOUNTERS_BUSY_SP1=113    # run1: 105   run2: 113/47
#NCOUNTERS_BUSY_L2RS=129   # from 5.7.2012, not used in run2
NCOUNTERS_BUSY_TSGROUP=153
NCOUNTERS_BUSY_RUNX1=154
#
NCOUNTERS_INT=19
NCOUNTERS_SPEC=49
# 2 more for epoch_seconds and epoch_micseconds
# NCOUNTERS: different from ctpcounters.h (does not include NCOUNTERS_SPEC)
NCOUNTERS=NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+6*NCOUNTERS_FO+NCOUNTERS_BUSY+NCOUNTERS_INT
BYSH=NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+6*NCOUNTERS_FO
INTSH=NCOUNTERS_L0+NCOUNTERS_L1+NCOUNTERS_L2+6*NCOUNTERS_FO+NCOUNTERS_BUSY
SPECSH=INTSH+NCOUNTERS_INT

shmext=None
cntsactive=0

class CTPboards:
  def __init__(self, printnames=None):
    #self.bicname= os.environ['VMEWORKDIR'] +"/WORK/default.counters"
    #print "CTPboards:printnames:",printnames
    if printnames!=None:
      self.boards={"l0":"0x829000","l1":"0x82a000","l2":"0x82b000",
        "busy":"0x828000","int":"0x82c000",
        "fo1":"0x821000", "fo2":"0x822000", "fo3":"0x823000", 
        "fo4":"0x824000", "fo5":"0x825000", "fo6":"0x826000"}
      return
    self.bicname= "/root/NOTES/boardsincrate"
    cf= open(self.bicname,"r")
    #self.boards["fo1"]: base
    self.boards={}
    ltuix=1; ttcviix=1
    for line in cf.readlines():
      nb= string.split(line,"=")
      if nb[0]=="l0": self.boards["l0"]= nb[1]
      elif nb[0]=="l1": self.boards["l1"]= nb[1]
      elif nb[0]=="l2": self.boards["l2"]= nb[1]
      elif nb[0]=="busy": self.boards["busy"]= nb[1]
      elif nb[0]=="int": self.boards["int"]= nb[1]
      #elif nb[0]=="fo": self.boards["fo%1d"%(int(nb[1][4])+1)]= nb[1]
      elif nb[0]=="fo": self.boards["fo%1d"%(int(nb[1][4]))]= nb[1]
      elif nb[0]=="ltu": 
        self.boards["ltu"+str(ltuix)]= nb[1]
        ltuix= ltuix+1
      elif nb[0]=="ttcvi": 
        self.boards["ttcvi"+str(ttcviix)]= nb[1]
        ttcviix= ttcviix+1
      else:
        print "Error in %s:%s"%(self.bicname,line)       
    cf.close()
  def getBase(self,brd):
    if self.boards.has_key(brd):
      return self.boards[brd]
    else:
      return None
  def getSet(self,brd):
    """ return: None if board not in the crate or
    ["fo1","fo2",...]  -list with the names of the present boards
    """
    rc=[]
    if brd=='fo':
      for ix in (1,2,3,4,5,6):
        bn= 'fo'+ str(ix)
        if self.boards.has_key(brd):
          rc.append[bn]
    elif self.boards.has_key(brd):
      rc.append[bn]
    if rc==[]: rc=None
    return rc

class SHMCounter:
  pxwidth=500
  pxheight=46
  xresolution=2
  nmax= pxwidth/xresolution-1
  canvasbg="#ccffcc"
  COLDYNAMIC=canvasbg
  maxcounter=0x7fffffff
  entwidth=11
  def __init__(self, ctpcnts, cntname, cntaddr, color):
    self.ctpcnts= ctpcnts
    self.cntname=cntname
    self.cntaddr=cntaddr
    #self.ctpcnts.nx   - order number of measurement
    self.nx= ctpcnts.nx
    self.history=[]
    for ix in range(self.nx):
      self.history.append(0)
    self.minval=0
    self.maxval=1000
    self.dynminval=1   # 0:fixed limits   1: dynamic
    self.dynmaxval=1
    self.minvalfound=SHMCounter.maxcounter
    self.maxvalfound=0
    self.xpix=0; self.ypix=SHMCounter.pxheight-1
    self.f1= myw.MywFrame(ctpcnts.regsframe,side=TOP, expand='no',relief=FLAT,
      borderwidth=0)
    f11= myw.MywFrame(self.f1,side=LEFT)
    self.ccv= myw.MywEntry(f11,cntname,width=SHMCounter.entwidth,
      defvalue=' ', bg=color, side=TOP, helptext=
"""3 entry fields represent the last measured value of the counter
and 2 limits: Upper and Lower.
Both limits can be modified by entering new value and pressing

               ENTER or TAB key         <--------- !!!!!!!!!!!!

Modify the limit field to empty string or string
'auto'   to set 'AUTOMATIC limit' -i.e. it will be adjusted
         always, when counter overruns/underruns current 
         upper/lower limit.
number   to set 'FIXED limit' -i.e. the limit won't be adjusted.
         In this case, higher/lower values will be drawn as red line on 
         the top/bottom of the counter canvas.

To delete the counter, click its name by the middle mouse button.
""")   
    self.ccv.label.bind("<Button-2>",self.deletecnt)
    self.hisgra= myw.Kanvas(self.f1,width=SHMCounter.pxwidth, 
      height=SHMCounter.pxheight+1,
      background=SHMCounter.canvasbg, borderwidth=1)
    #self.hisgra.create_line(0,2,499,2,width=1)
    #yy=SHMCounter.pxheight+1
    #self.hisgra.create_line(0,yy,499,yy,width=1)
    f1mm= myw.MywFrame(self.f1,expand='no',side=RIGHT)
    self.cmax= myw.MywEntry(f1mm,'',defvalue='auto', #str(self.maxval),
      width=SHMCounter.entwidth,side=TOP,
      expandentry='no',bind='r',cmdlabel=self.newmax,helptext=
"""Upper limit. Modify this field to empty string or string
'auto'   to set 'AUTOMATIC limit' -i.e. it will be adjusted
         always, when counter overruns current limit
number   to set 'FIXED limit' -i.e. the limit won't be adjusted.
         Higher values will be drawn as red line on the top """)   
    self.cmin= myw.MywEntry(f1mm,'',defvalue='auto', #str(self.minval),
      width=SHMCounter.entwidth,side=TOP,
      expandentry='no',bind='r',cmdlabel=self.newmin)   
    #self.scale=0   # meaningful only after at least 2 diffrent measurements
    self.doscale()
    self.newmin(); self.newmax()
    #f13= myw.MywFrame(self.f1,side=LEFT)
  def updateShift(self, ixmaxdel):
    """ixmaxdel: how many entries to be deleted on the left
    """
    #print "update:",self.cntname,self.cntaddr
    #print "redrawing-maxxreached:"  
    del self.history[0:ixmaxdel]
    self.nx=len(self.history)
    # find new limits (if they are dynamic):
    if self.dynminval==1: 
      self.minval=self.findmin()
      self.cmin.setEntry(str(self.minval))
    if self.dynmaxval==1: 
      self.maxval=self.findmax()
      self.cmax.setEntry(str(self.maxval))
    self.redraw()
  def update(self):
    self.value=shmext.getcnt(self.cntaddr)
    self.history.append(self.value)
    self.ccv.setEntry(str(self.value))
    # always keep track of min/max in self.*valfound
    if self.value<self.minvalfound: 
      self.minvalfound=self.value
    if self.value>self.maxvalfound: 
      self.maxvalfound=self.value
    self.doLine(self.nx, self.value)
    if self.nx != self.ctpcnts.nx:
      print "SHMCounter: error ns != ctpcnts.nx"
    self.nx= self.nx+1
  def redraw(self):
    """ 0: first, self.nx: last point to be drawn
    """
    if len(self.history)<=2:
      #print "redraw: nothing to redraw"
      return
    self.hisgra.delete("TAGline")
    self.xpix,self.ypix= self.c2pix(0,self.history[0])
    #print "redraw:",self.cntname, self.history
    for ixval in range(len(self.history))[1:]:
      self.doLine(ixval, self.history[ixval])
  def c2pix(self,x,y):
    return x*SHMCounter.xresolution,\
      int(round((self.maxval-y)*self.scale)) +2
  def doLine(self, xax, val):
    redo=None; linecol='black'
    if val<self.minval: 
      if self.dynminval==0: 
        val= self.minval; linecol='red'
      else: 
        self.minval= val; redo=1
        self.cmin.setEntry(str(self.minval))
    if val>self.maxval: 
      if self.dynmaxval==0: 
        val= self.maxval; linecol='red'
      else: 
        self.maxval= val; redo=1
        self.cmax.setEntry(str(self.maxval))
    if redo:
      self.doscale()
      #print "redrawing-scalechanged:"  
      self.redraw()
    x1,y1= self.c2pix(xax, val)
    if xax<1:
      print "doLine: drawing point"
      x0= x1
      y0= y1
    else:
      x0= self.xpix
      y0= self.ypix
    #if y1==0: y1=2
    id=self.hisgra.create_line(x0,y0,x1,y1,width=1,fill=linecol)
    self.hisgra.addtag_withtag("TAGline",id)
    #print "doLine:",xax,":",val,x0,y0,x1,y1
    self.xpix=x1; self.ypix=y1
  def doscale(self):
    if self.maxval==self.minval:  #(or self.scale==None) #first measur.
      self.scale=0
    else:
      self.scale= SHMCounter.pxheight/float(self.maxval-self.minval)
    #print "doscale:",self.minval,self.maxval,self.scale
  def findmin(self):
    minval=SHMCounter.maxcounter
    for val in self.history:
      if val<minval: minval= val
    return minval
  def findmax(self):
    minval=0
    for val in self.history:
      if val>minval: minval= val
    return minval
  def newmin(self,e=None):
    mv=self.cmin.getEntry(); mvo=mv
    #print "newmin:",mv
    #min
    if mv=='' or mv=="auto":
      self.dynminval=1
      curmin= self.findmin()
      mv=str(curmin)
      self.cmin.setColor(SHMCounter.COLDYNAMIC)
    else:
      self.dynminval=0
      self.cmin.setColor()
    imv=int(mv)
    if imv != self.minval or mvo=='':
      self.minval= imv
      self.cmin.setEntry(str(self.minval))
      self.doscale()
      self.redraw()
  def newmax(self,e=None):
    #max:
    mv=self.cmax.getEntry(); mvo=mv
    if mv=='' or mv=="auto":
      self.dynmaxval=1
      curmax= self.findmax()
      mv=str(curmax)
      self.cmax.setColor(SHMCounter.COLDYNAMIC)
    else:
      self.dynmaxval=0
      self.cmax.setColor()
    imv=int(mv)
    if imv != self.maxval or mvo=='':
      self.maxval= imv
      self.cmax.setEntry(str(self.maxval))
      self.doscale()
      self.redraw()
  def deletecnt(self,event=None):
    #print "deletecnt:", self.cntname, event
    self.removecnt()
    ixreg= self.ctpcnts.find(self.cntname)
    del self.ctpcnts.regs[ixreg]
  def removecnt(self):
    #print "removing:", self.cntname
    self.f1.destroy()

class VMECounter:
  def __init__(self, ctpcnts, cntname, cntaddr, color, val=None, helptext="VME counter"):
    self.cntname=cntname
    self.cntaddr=cntaddr
    self.cntentry= myw.MywEntry(ctpcnts.getregsframe(), label=cntname,
      bg= color, defvalue= val, helptext=helptext,
      side="top",width=11,expandentry='no')
    #, cmdlabel=self.chooseReg)
    #delaction=self.destroyReg, defaultinx=inx,
    self.cntentry.convertStart()
  def removecnt(self):
    self.cntentry.destroyEntry()

#LTU: name, rel.address, CGT, help
LTUvicnts={
"time":(0,"T","Elapsed time (in 0.4micsecs counts)"),
"in_busy1":(1,"T","Subdetector BU/readTablesClient.cSY1 input timer"),
"in_busy2":(2,"T","Subdetector BUSY2 input timer"),
"sbusy":(3,"T","Subdetector BUSY timer (in_busy1 OR in_busy2)"),
"busy":(4,"T","LTU BUSY timer"),
"l1nearly_full":(5,"T","L1 message FIFO nearly full status timer"),
"l2nearly_full":(6,"T","L2 message FIFO nearly full status timer"),
"Interlock_timer":(7,"T","The timer for 'BUSY due to RATE_LIMIT condition'"),
"start_all":(8,"C","Start signal"),
"busy_ts":(9,"C","LTU BUSY on transition (25ns)"),
"l1nearly_full_ts":(10,"C","L1 FIFO nearly full on transition (25ns)"),
"l2nearly_full_ts":(11,"C","L2 FIFO nearly full on transition (25ns)"),
"any_error":(12,"C","ANY ERROR pulse"),
"Interlock_count":(13,"C","The number of RATE_LIMIT conditions"),
"BUSYover":(14,"C","BUSY over threshold (BusyProbe, 25ns)"),
"prepulse":(15,"C","Prepulse request signal (25ns), includes errors"),
"l0":(16,"C","L0 output, includes errors"),
"l1_only":(17,"C",""""L1 output. 
In case of 'L0 over TTC' it counts only L1 signals
Includes errors
"""),
"l1_strobe":(18,"C","L1 Strobe signali, includes errors"),
"l2a_strobe":(19,"C","L2a Strobe signal"),
"l2r_strobe":(20,"C","L2r Strobe signal")}
LTUcnts={
"time":(0,"T","Elapsed time (in 0.4micsecs counts)"),
"in_busy1":(1,"T","Subdetector BUSY1 input timer"),
"in_busy2":(2,"T","Subdetector BUSY2 input timer"),
"sbusy":(3,"T","Subdetector BUSY timer (in_busy1 OR in_busy2)"),
"busy":(4,"T","LTU BUSY timer"),
"l1nearly_full":(5,"T","L1 FIFO nearly full timer"),
"l2nearly_full":(6,"T","L2 FIFO nearly full timer"),
"bc_error":(7,"T","BC ERRor condition timer"),
"start_all":(8,"C","Start signal"),
"busy_ts":(9,"C","LTU BUSY on transition (25ns)"),
"l1nearly_full_ts":(10,"C","L1 FIFO nearly full on transition (25ns)"),
"l2nearly_full_ts":(11,"C","L2 FIFO nearly full on transition (25ns)"),
"any_error":(12,"C","ANY ERROR pulse"),
"bc_error_ts":(13,"C","BC Error on transition (25ns)"),
"mstrobe_ts":(14,"C","Master strobe on transition (25ns) or longbusy(LTUvi)"),
"prepulse":(15,"C","Prepulse signal (25ns)"),
"l0":(16,"C","L0 output"),
"l1_only":(17,"C","L1 output. In case of 'L0 over TTC' it counts only L1 signals"),
"l1_strobe":(18,"C","L1 Strobe signal"),
"l2a_strobe":(19,"C","L2a Strobe signal"),
"l2r_strobe":(20,"C","L2r Strobe signal")}

LVDSTcnts={
"time":(0,"T","Elapsed time (in 0.4micsecs counts)"),
"time1sterr1":(1,"T","Time until first error, cable1"),
"time1sterr2":(2,"T","Time until first error, cable2"),
"gnd3":(3,"C","ground"),
"gnd4":(4,"C","ground"),
"gnd5":(5,"C","ground"),
"gnd6":(6,"C","ground"),
"gnd7":(7,"C","ground"),
"Cable1errs":(8,"C","number of different BCs measured on cable 1"),
"Cable2errs":(9,"C","number of different BCs measured on cable 2"),
"PatternRate":(10,"C","Number of '1' BCs (BCs where data==1)"),
"SeqStrobes":(11,"C","Number of sequence strobes")}

class CTPcnts:
  COLSIZE=10
  c150=("1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32","33","34","35","36","37","38","39","40","41","42","43","44","45","46","47","48","49","50")
  c1100=("1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32","33","34","35","36","37","38","39","40","41","42","43","44","45","46","47","48","49",\
  "50","51","52","53","54","55","56","57","58","59",\
  "60","61","62","63","64","65","66","67","68","69",\
  "70","71","72","73","74","75","76","77","78","79",\
  "80","81","82","83","84","85","86","87","88","89",\
  "90","91","92","93","94","95","96","97","98","99","100")
  i124=("1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24")
  i48=("1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32","33","34","35","36","37","38","39","40","41","42","43","44","45","46","47","48")
  i112=("1","2","3","4","5","6","7","8","9","10","11","12")
  i16=("1","2","3","4","5","6","7","8")
  iT6=("T", "1","2","3","4","5","6","7","8")
  i14=("1","2","3","4")
  t7=("1","2","3","4","5","6","T")
  t9=("1","2","3","4","5","6","7","8","T")
  pf5=("1","2","3","4","T")
  specfname="spec.py"
  #def __init__(self,tlw=None, vb=None):   #see myw.vbexec
  def __init__(self, ctpltu=None, tlw=None, printnames=None):
    global cntsactive
    #print "CTPcnts:printnames:",printnames
    #ctpltu: None -> ctp
    #        dictionary -> LTU or LVDST
    if cntsactive==1:return #self.__del__ called after this return
    cntsactive=1
    self.perrep=0     # to be compliant with MultRegs
    self.mrmaster=tlw
    self.nx=0
    self.ctpltu= ctpltu
    #if self.mrmaster:
    #  myw.RiseToplevel(self.mrmaster); return
    #else:
    if ctpltu!=None:
      self.saveconf=None
      tllabel="LTU counters"
      self.numberofcounters=len(ctpltu)
      deffile= os.environ['VMEWORKDIR'] +"/WORK/ltu.counters"
      cmdbuttons= [
        ("Read", self.allread),
        ("Increments", self.changeacc),
        ("Periodic read", self.perRead)
        #("Clear counters", self.clearCounters)
      ]
      #addCTPorLTUReg= self.addLTUReg
    else:
      tllabel="CTP counters"
      self.saveconf='yes'
      self.numberofcounters=NCOUNTERS
      deffile= os.environ['VMEWORKDIR'] +"/WORK/default.counters"
      self.ctpboards= CTPboards(printnames)
      self.boardfs={} # boardsfs{board}->corresponding frame in ADDREM window
      cmdbuttons= [
        ("Read", self.allread),
        ("Increments", self.changeacc),
        ("Periodic read", self.perRead),
        #("Clear counters", self.clearCounters),
        ("Add/Remove counter", self.addnewcnt)
      ]
      #addCTPorLTUReg= self.addReg
    #print "ttlabel:", self.numberofcounters
    self.mrmaster=myw.NewToplevel(tllabel,
      self.hidemrm, tlw=self.mrmaster)
    self.defcounters= deffile

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
                NOTE about LTU counters: minimum time between 2 clicks
                is 1sec (if shorter, the same reading is returned,
                in case the same ltuproxy time slot hit)
Increments     -showing increments
Abs. values    -showing absolute counter values 
Periodic read  -repeat updating once per second (roughly) 10 times
Add/remove     -add new/remove shown counter field (only for ctp counters)
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
      for lc in ctpltu.keys():
        self.addLTUReg(lc)
    self.init()
  def printSPEC(self):
    print "epochsecs %d ctp G ctp"%SPECSH
    print "epochmics %d ctp G ctp"%(SPECSH+1)
    print "l2orbit %d l2 G ctp"%(SPECSH+2)
    specix=SPECSH+3
    for bn in ['busy','l0','l1','l2','int','fo1','fo2','fo3','fo4','fo5','fo6']:
      print "%stemp %d %s G ctp"%(bn, specix, bn) ; specix=specix+1
      print "%svolts %d %s G ctp"%(bn, specix, bn) ; specix=specix+1
    for ltuix in range(24):
      ltun= self.findLTUdetnum(ltuix)
      print "ltuvolts%d %d ltu G %s"%(ltuix+1, specix, ltun)
      specix=specix+1
    #spares:
    # see: v/DOC/CTPreadme
    lix= 0+NCOUNTERS_L0_SP1
    # 2 spares
    for irp in range(lix,lix+2):
      print "spare%d %d l0 S N"%(irp, irp)
    lix= 0+NCOUNTERS_L0_SP2
    #9 spares
    for irp in range(lix,lix+9):
      print "spare%d %d l0 S N"%(irp, irp)
    lix= 0+NCOUNTERS_L0_SP3
    #2 spares
    for irp in range(lix,lix+2):
      print "spare%d %d l0 S N"%(irp, irp)
    lix= L1SH+NCOUNTERS_L1_SP1
    # 2 spares
    for irp in range(lix,lix+2):
      print "spare%d %d l1 S N"%(irp, irp)
    lix= L1SH+NCOUNTERS_L1_SP2
    hix= L1SH+NCOUNTERS_L1
    # run2: 50 spares
    for irp in range(lix,hix):
      print "spare%d %d l1 S N"%(irp, irp)
    lix= L2SH+NCOUNTERS_L2_SP1
    for irp in range(lix,lix+1):   # 1 spare
      print "spare%d %d l2 S N"%(irp, irp)
    lix= L2SH+NCOUNTERS_L2_SP2
    for irp in range(lix,lix+64):   # 64 spares
      print "spare%d %d l2 S N"%(irp, irp)
    #for irp in range(763, 812):
    lix= BYSH+NCOUNTERS_BUSY_SP1
    #hix= BYSH+NCOUNTERS_BUSY_L2RS in run1
    hix= BYSH+NCOUNTERS_BUSY_TSGROUP
    for irp in range(lix, hix):
      print "spare%d %d busy S N"%(irp, irp)
    # 6x4 FO counters:
    #irp= BYSH+NCOUNTERS_BUSY_L2RS
    #for fon in range(1, 7):
    #  for conn in range(1, 5):
    #    print "fo%dl2rout%d %d fo%d C N"%(fon, conn, irp, fon)
    #    irp= irp + 1
    #
    hix= BYSH+NCOUNTERS_BUSY_TSGROUP
    print "spare%dTSGROUP %d busy G N"%(hix, hix)
    # 6 counters reserved for 'RUNX COUNTERS' (see:
    # ctp_proxy/dimservices.c:#define RUNXCOUNTERSSTART 812
    lix= BYSH+NCOUNTERS_BUSY_RUNX1
    for irp in range(lix, lix+6):
      print "spare%drunx %d busy S N"%(irp, irp)
    #
    for foix in range(NFOS):
      fosh= FOSH + foix*NCOUNTERS_FO
      lix= fosh+NCOUNTERS_FO_SP1
      for irp in range(lix,lix+3):   # 3 spares
        print "spare%d %d fo%d S N"%(irp, irp, foix+1)
      lix= fosh+NCOUNTERS_FO_SP2
      hix= fosh+NCOUNTERS_FO
      # run2: 10 spares
      for irp in range(lix,hix):
        print "spare%d %d fo%d S N"%(irp, irp, foix+1)
  def addnewcnt(self):
    if self.addw:
      #return
      myw.RiseToplevel(self.addw); return
    else:
      self.addw=myw.NewToplevel("Adding new CTP counter", self.hideaddw)
    if self.printnames!=None:
      import trigdb
      self.ltus=trigdb.readVALIDLTUS()
    # see comment in self.indexmode()
    #------------------------------------------------------ L0
    self.l0frame= myw.MywFrame(self.addw,side=LEFT); 
    self.boardfs["l0"]=self.l0frame
    l0label= myw.MywLabel(self.l0frame,label="L0 counters",
      bg=myw.VmeBoard.CTPcolors["l0"], side=TOP)
    self.makeit1("l0","l0byclst", CTPcnts.t9, "Test, 1-8 cluster BUSY")
    self.makeit1("l0","l0pf", CTPcnts.pf5, "Test, 1-4 P/F output")
    self.makeit1("l0","l0timers", ["allrare","l0time","l0rate28"],
      """allrare, elapsed time and down-scaling veto ON for L0 class28
in 16BCs intervals""")
    #self.makeit1("l0","l0inp", CTPcnts.i124, "L0 input")
    self.makeit1("l0","l0classB", CTPcnts.c1100, "Class before vetos")
    self.makeit1("l0","l0classA", CTPcnts.c1100, "Class after vetos")
    self.makeit1("l0","l0inp", CTPcnts.i48, "L0 inputs")
    #self.makeit1("l0","l0ifun", ("1","2"), "L0 input functions")
    self.makeit1("l0","l0ifun", ("1","2","3","4"), "L0 input functions")
    self.makeit1("l0","l0int", ["1","2","T","A","B", "D"],
      "Interaction signals: 1,2,T and  P/F Interaction signals A,B,D")
    #self.makeit1("l0","l0int", ["1","2","T","A","B"],
    #  "Interaction signals: 1,2,T and  P/F Interaction signals A,B")
    self.makeit1("l0","l0counters", ["l0strobe0","prepulse","s_soft"],
      "L0 strobe (ANYCLST), Prepulse and SW trigger counters")
    self.makeit1("l0","l0clst", CTPcnts.t9, "Test, 1-8 cluster trigger")
    #------------------------------------------------------ L1
    self.l1frame= myw.MywFrame(self.addw,side=LEFT); 
    self.boardfs["l1"]=self.l1frame
    l1label= myw.MywLabel(self.l1frame,label="L1 counters",
      bg=myw.VmeBoard.CTPcolors["l1"], side=TOP)
    self.makeit1("l1","l1timers", ["l1pf1","l1pf2","l1pf3","l1pf4",
      "l1pfT","l1time"],
      """L1 timers in 16BCs intervals""")
    self.makeit1("l1","l1inp", CTPcnts.i124, "L1 inputs")
    self.makeit1("l1","l1int", ["1","2","A","B", "D"] , 
"""
1,2:   2 interaction inputs
A,B,D: 3 interaction P/F signals
""")
    self.makeit1("l1","Strobes,ESR", ["l0strobeIN","l1strobeOUT",
      "esrflag"],"""
l0strobeIN  -L0strobe input (any L0 cluster)
l1strobeOUT -L1strobe output (any L1 cluster)
esrflag     -Enable Segmented Readout flag
""")
    self.makeit1("l1","l1classB", CTPcnts.c1100, "Class triggers before vetos")
    self.makeit1("l1","l1classA", CTPcnts.c1100, "Class triggers after vetos")
    self.makeit1("l1","l1clst", ("1","2","3","4","5","6","7","8","T","0"),"""
1-8   -L1 cluster 1-8 triggers
T     -L1 test cluster (after vetos)
0     -L0 test cluster (before vetos)
""")
    #------------------------------------------------------ L2
    self.l2frame= myw.MywFrame(self.addw,side=LEFT); 
    self.boardfs["l2"]=self.l2frame
    l2label= myw.MywLabel(self.l2frame,label="L2 counters",
      bg=myw.VmeBoard.CTPcolors["l2"], side=TOP)
    self.makeit1("l2","l2timers", ["l2pf1","l2pf2","l2pf3","l2pf4",
      "l2pfT","l2time"],
      """L2 timers in 16BCs intervals""")
    self.makeit1("l2","l2inp", CTPcnts.i112, "L2 inputs")
    self.makeit1("l2","l2int", ["1","2","A","B", "D"] , 
"""
1,2:   2 interaction inputs
A,B,D: 3 interaction P/F signals
""")
    self.makeit1("l2","Strobes", ["l1strobeIN","l2strobeOUT"],
      """l1strobeIN -any L1 cluster
l2strobeOUT -any L2 cluster""")
    self.makeit1("l2","l2classB", CTPcnts.c1100, "Class triggers before vetos")
    self.makeit1("l2","l2classA", CTPcnts.c1100, "Class triggers after vetos")
    self.makeit1("l2","l2clst", ("1","2","3","4","5","6","7","8","T","X"),"""
1-8   -l2clst[1-8]. L2 cluster1-8 trigger
T     -l2clstt.     L2 test cluster (after vetos)
X     -l1clstt.     L1 test cluster (before vetos)
""")
    #------------------------------------------------------ FO
    for foix in [1,2,3,4,5,6]:
      fona = ["zero","fo1","fo2","fo3","fo4","fo5","fo6"][foix]
      # if printing all counter names to stdout, do it for ALL boards
      if self.printnames==None:
        if not self.ctpboards.boards.has_key(fona): continue  #FO not in crate
      self.boardfs[fona]= myw.MywFrame(self.addw,side=LEFT); 
      folabel= myw.MywLabel(self.boardfs[fona],label="FO%d counters"%foix,
        bg=myw.VmeBoard.CTPcolors[fona[:2]], side=TOP)
      #self.makeit1(fona,fona+"others", ["time","l1strIN",
      #  "l2strIN","ppi", "l0clstt","l1clstt"],
      self.makeit1(fona,fona+"others", [fona+"time",fona+"l1strIN",
        fona+"l2strIN",fona+"ppi", fona+"l0clstt",fona+"l1clstt"],
        """fotime		-elapsed time (timer)
fol1strIN	-L1 strobe input (any L1 cluster)
fol2strIN	-L2 strobe input
foppi		-Prepulse input
fol0clstt	-L0 test cluster trigger
fol1clstt	-L1 test cluster trigger""")
      self.makeit1(fona,fona+"l0clst", CTPcnts.i16, "L0 cluster1-8 trigger")
      self.makeit1(fona,fona+"l1clst", CTPcnts.i16, "L1 cluster1-8 trigger")
      self.makeit1(fona,fona+"glitch", CTPcnts.iT6, "Glitch for cluster T,1-8")
      self.makeit1(fona,fona+"l1spurious", CTPcnts.iT6, "L1spurious cluster T,1-8")
      self.makeit1(fona,fona+"ppout", CTPcnts.i14, "PP output 1-4")
      self.makeit1(fona,fona+"l0out", CTPcnts.i14, "L0 output 1-4")
      self.makeit1(fona,fona+"l1out", CTPcnts.i14, "L1 output 1-4")
      self.makeit1(fona,fona+"l2stro", CTPcnts.i14, "L2 strobe output 1-4")
      self.makeit1(fona,fona+"l2rout", CTPcnts.i14, "L2r output 1-4")
    #------------------------------------------------------ BUSY
    self.busyframe= myw.MywFrame(self.addw,side=TOP); 
    self.boardfs["busy"]=self.busyframe
    busylabel= myw.MywLabel(self.busyframe,label="BUSY counters",
      bg=myw.VmeBoard.CTPcolors["busy"], side=TOP)
    self.makeit1("busy","byin", CTPcnts.i124,
      "24 subdetector BUSY input timers")
    self.makeit1("busy","byin_end", CTPcnts.i124,
      """24 subdetector BUSY input counters counting the number of 
busy_end signals. For single-event buffering detectors, this counter
should be equal to received triggers.
""")
    self.makeit1("busy","byin_last", CTPcnts.i124,
      """24 subdetector BUSY input counters counting the number of 
busy_last signal -number of occurences when this detector was slowest
in its cluster.
""")
    self.makeit1("busy","byout", CTPcnts.t9,
      "9 Cluster BUSY output timers for Cluster1-8 and Test cluster")
    self.makeit1("busy","byout_end", CTPcnts.t9,
      """9 Cluster BUSY output counters for Cluster1-8 and Test cluster.
The number of 'cluster busy' signals.""")
    self.makeit1("busy","bydaq", CTPcnts.i16,
      "8 Cluster DAQ BUSY timers")
    self.makeit1("busy","bytimers", ("CTPdeadtime","CTPbusy","bytime"),
      "Other BUSY timers")
    self.makeit1("busy","bycounters", ("byanyclu","byclu1","byclu2",
      "byclu3","byclu4","byclu5","byclu6","byclu7","byclu8","bytestclass",
      "byendCTPbusy", "bylongbusy"),
      """BUSY counters:
byanyclu    -Any L0 Cluster trigger
byclu1-8    -L0 Cluster trigger 1-8
bytestclass -L0 Test class trigger
byendCTPbusy-End of CTPbusy (CTPbusy transitions)
bylongbusy  -Long BUSY (BusyProbe).Counts cases, when busy>MAXIMIM_LIMIT
""")
    #------------------------------------------------------ INT
    self.intframe= myw.MywFrame(self.addw,side=TOP); 
    self.boardfs["int"]=self.intframe
    busylabel= myw.MywLabel(self.intframe,label="INT counters",
      bg=myw.VmeBoard.CTPcolors["int"], side=TOP)
    self.makeit1("int","inttimers", ("intCTPbusy","fiLF",
      "inttime"), """INT timers:
intCTPbusy   -CTP busy (DDL FIFO nerly full)
fiLF         -Link Full (DDL signal)
inttime      -elapsed time
""")
    self.makeit1("int","intcounters", ("int1","int2","intL1S",
      "intL2S","L2rseq","L2aseq","rd_block","rc_block",
      "orbit_err", "ctpbusy_pulse", "lf_pulse", "L2a_tc", "tc_input1",
      "tc_input2", "incomplete", "orc_error"),
      """INT counters:
int1       -Interaction 1 input
int2       -Interaction 2 input
intL1S     -L1 Strobe input
intL2S     -L2 Strobe input
L2rseq     -L2r sequence
L2aseq     -L2a sequence
rd_block   -DDL CTP Readout block
rc_block   -DDL Interaction Record block
orbit_err  -Orbit Counter Synchronisation Error
ctpbusy_pulse - CTPbusy 0->1 transition
lf_pulse   -Link Full 0->1 transition
L2a_tc     -L2a Test Class sequence
tc_input1  -Test Class Counter 1 input (programmable)
tc_input2  -Test Class Counter 2 input (programmable)
incomplete -Incomplete Orbit error
orc_error  -Orbit record with error
""")
    if self.printnames!=None:
      self.printSPEC()
  def findLTUfo(self, foboard, focon):
    for ltu in self.ltus:
      if ltu.fo==foboard and ltu.focon==focon:
        return ltu.name
    return 'N'
  def findLTUbusy(self, busyinp):
    for ltu in self.ltus:
      if ltu.bsyinp==busyinp:
        return ltu.name
    return 'N'
  def findLTUdetnum(self, ecsdaqn):
    for ltu in self.ltus:
      if ltu.detnum==ecsdaqn:
        return ltu.name
    return 'N'
  def findAddr(self, cntlabel, col5=None):
    """
    rc: Counter address,board,CGT,det5c
        or None for non-existing counter
    Counter address is position in shared memory (0,...)
    L0 counters: 0-159 (158:l0clst6)
    ...
    CGT: Counter, Gauge, Timer
    det5c: ctp or detector name from VALID.LTUS if applicable (from 24.10.2007)
    """
    n=None; board=None; det5c='ctp'
    #det5c: 5th column for cnames file
    CGT='C'   # Counter (default), Gauge or Timer
    #print "findAddr:",cntlabel
    #------------------------------------------------------ L0
    if string.find(cntlabel,"l0byclst")==0:
      board="l0"; c= cntlabel[8]; CGT='T';
      if c=="T": n= 0
      else: n= int(c)
    elif string.find(cntlabel,"l0pf")==0:
      board="l0"; c= cntlabel[4]; CGT='T';
      if c=="T": n= 9
      else: n= int(c)+9
    elif string.find(cntlabel,"allrare")==0:
      board="l0"; n= 14; CGT='T';
    elif string.find(cntlabel,"l0time")==0:
      board="l0"; n= 15; CGT='T';
    elif string.find(cntlabel,"l0rate28")==0:
      board="l0"; n= 16; CGT='T';
    elif string.find(cntlabel,"l0classB")==0:
      board="l0"; n= (19-1)+ int(cntlabel[8:])
    elif string.find(cntlabel,"l0inp")==0:   #l0inp1: 116
      board="l0"; n= (119-1)+ int(cntlabel[5:])
    elif string.find(cntlabel,"l0ifun")==0:  # l0ifun1: 140
      board="l0"; 
      c1234= cntlabel[6]
      if c1234>="1" and c1234<="4":   # 140..141
        n= (167-1)+ int(cntlabel[6:])
      else: 
        myw.errorprint(self,"Bad int counter name:"+cntlabel)
    elif string.find(cntlabel,"l0int")==0:
      board="l0"; c= cntlabel[5];
      if c=="1": n= 173 #50+92
      elif c=="2": n= 174 #50+93
      elif c=="T": n= 172 #50+94
      elif c=="A": n= 175 #50+95
      elif c=="B": n= 176 #50+96
      elif c=="D": n= 177 #50+97
      else: 
        myw.errorprint(self,"Bad int counter name:"+cntlabel)
    elif string.find(cntlabel,"l0strobe0")==0:   # 148
      board="l0"; n= 171
    elif string.find(cntlabel,"l0classA")==0:    # run2: l0classA1: 152
      board="l0"; n= (187-1)+ int(cntlabel[8:])   # run1: 99
    elif string.find(cntlabel,"s_soft")==0:
      board="l0"; n= 287 #102+150
    elif string.find(cntlabel,"prepulse")==0:
      board="l0"; n= 288 #102+151
    elif string.find(cntlabel,"l0clst")==0:
      board="l0"; c= cntlabel[6];
      if c=="T": n= 289 # 102+152
      else: n= (290-1) + int(c)  #102+152+int(c)
    #------------------------------------------------------ L1
    elif string.find(cntlabel,"l1pf")==0:
      board="l1"; c= cntlabel[4]; CGT='T';
      if c=="T": n= 0 +L1SH
      else: n= int(c) +L1SH
    elif string.find(cntlabel,"l1time")==0:
      board="l1"; n= 5+L1SH; CGT='T';
    elif string.find(cntlabel,"l1inp")==0:
      board="l1"; n= 5+L1SH+ int(cntlabel[5:])
    elif string.find(cntlabel,"l1int")==0:
      board="l1"; c= cntlabel[5];
      if c=="1": n= 30+L1SH
      elif c=="2": n= 31+L1SH
      elif c=="A": n=32+L1SH
      elif c=="B": n=33+L1SH
      elif c=="D": n=34+L1SH
      else: 
        myw.errorprint(self,"Bad l1int counter name:"+cntlabel)
    elif string.find(cntlabel,"l0strobeIN")==0:
      board="l1"; n= 35+L1SH
    elif string.find(cntlabel,"l1strobeOUT")==0:
      board="l1"; n= 36+L1SH
    elif string.find(cntlabel,"esrflag")==0:
      board="l1"; n= 37+L1SH
    elif string.find(cntlabel,"l1classB")==0:
      board="l1"; n= 39+L1SH+ int(cntlabel[8:])
    elif string.find(cntlabel,"l1classA")==0:
      board="l1"; n= 139+L1SH+ int(cntlabel[8:])   #run1:89
    elif string.find(cntlabel,"l1clst")==0:
      board="l1"; c= cntlabel[6];
      if   c=="T": n= 241+L1SH   #141
      elif c=="0": n= 240+L1SH   #140
      else: n= 241+L1SH+int(c)   #141/6 or run2:241/8
    #------------------------------------------------------ L2
    elif string.find(cntlabel,"l2pf")==0:
      board="l2"; c= cntlabel[4]; CGT='T';
      if c=="T": n= 0 +L2SH
      else: n= int(c) +L2SH
    elif string.find(cntlabel,"l2time")==0:
      board="l2"; n= 5+L2SH; CGT='T';
    elif string.find(cntlabel,"l2inp")==0:
      board="l2"; n= 5+L2SH+ int(cntlabel[5:])
    elif string.find(cntlabel,"l2int")==0:
      board="l2"; c= cntlabel[5];
      if c=="1": n= 18+L2SH
      elif c=="2": n= 19+L2SH
      elif c=="A": n=20+L2SH
      elif c=="B": n=21+L2SH
      elif c=="D": n=22+L2SH
      else: 
        myw.errorprint(self,"Bad l2int counter name:"+cntlabel)
    elif string.find(cntlabel,"l1strobeIN")==0:
      board="l2"; n= 23+L2SH
    elif string.find(cntlabel,"l2strobeOUT")==0:
      board="l2"; n= 24+L2SH
    elif string.find(cntlabel,"l2classB")==0:
      board="l2"; n= 25+L2SH+ int(cntlabel[8:])
    elif string.find(cntlabel,"l2classA")==0:
      board="l2"; n= 125+L2SH+ int(cntlabel[8:])   # run1:75
    elif string.find(cntlabel,"l2clst")==0:
      board="l2"; c= cntlabel[6];
      if   c=="T": n= 227+L2SH   # 127
      elif c=="X": n= 226+L2SH   # l1clstt. run1:126
      else: n= 227+L2SH+int(c)   # 127
    #------------------------------------------------------ BUSY
    # have to be in this order (byin_last checked before byin)
    elif string.find(cntlabel,"byin_end")==0:
      board="busy"; bsyin= int(cntlabel[8:])
      n= 63+ BYSH+bsyin-1; CGT='C';  #run1:55
      if col5: det5c= self.findLTUbusy(bsyin) 
    elif string.find(cntlabel,"byin_last")==0:
      board="busy"; bsyin= int(cntlabel[9:])
      n= 87+ BYSH+bsyin-1; CGT='C';  #run1:79
      if col5: det5c= self.findLTUbusy(bsyin) 
    elif string.find(cntlabel,"byin")==0:
      board="busy"; bsyin=int(cntlabel[4:])
      n= 0 + BYSH+bsyin-1; CGT='T';
      if col5: det5c= self.findLTUbusy(bsyin) 
    elif string.find(cntlabel,"byout_end")==0:
      board="busy"; CGT='C';
      if cntlabel[9]=='T':
        n= 62+ BYSH  #run1: 54
      else:
        n= 53+ BYSH+int(cntlabel[9])  #run1: 47
    elif string.find(cntlabel,"byout")==0:
      board="busy"; CGT='T';
      if cntlabel[5]=='T':
        n= 32+ BYSH   #run1: 30
      else:
        n= 23+ BYSH+int(cntlabel[5])   #run1/2:23
    elif string.find(cntlabel,"bydaq")==0:
      board="busy"; n= 32+ BYSH+int(cntlabel[5:]); CGT='T';   #run1 30
    elif string.find(cntlabel,"CTPdeadtime")==0:
      board="busy"; n= 41+ BYSH; CGT='T';   #run1: 37
    elif string.find(cntlabel,"CTPbusy")==0:
      board="busy"; n= 42+ BYSH; CGT='T';   #run1: 38
    elif string.find(cntlabel,"bytime")==0:
      board="busy"; n= 43+ BYSH; CGT='T';   #run1: 39
    elif string.find(cntlabel,"byanyclu")==0:
      board="busy"; n= 44+ BYSH   #run1: 40
    elif string.find(cntlabel,"byclu")==0:
      board="busy"; n= 44+ BYSH+ int(cntlabel[5:])   #run1: 40
    elif string.find(cntlabel,"bytestclass")==0:
      board="busy"; n= 53+ BYSH   #run1: 47
    elif string.find(cntlabel,"byendCTPbusy")==0:
      board="busy"; n= 111+ BYSH   #run1: 103
    elif string.find(cntlabel,"bylongbusy")==0:
      board="busy"; n= 112+ BYSH   #run1: 104
    #------------------------------------------------------ FO
    elif self.findFO(cntlabel,"time"):
      foix= self.findFO(cntlabel,"time")
      board="fo"; n= 0+FOSH +(foix-1)*NCOUNTERS_FO; CGT='T';
    elif self.findFO(cntlabel,"l1strIN"):
      foix= self.findFO(cntlabel,"l1strIN")
      board="fo"; n= 17+FOSH+(foix-1)*NCOUNTERS_FO   #run1:13
    elif self.findFO(cntlabel,"l2strIN"):
      foix= self.findFO(cntlabel,"l2strIN")
      board="fo"; n= 18+FOSH+(foix-1)*NCOUNTERS_FO   #run1:14
    elif self.findFO(cntlabel,"glitch"):
      foix=self.findFO(cntlabel,"glitch")
      board="fo"; c= cntlabel[9]; 
      if c=='T': c='0'
      n= 19 + int(c) +FOSH+(foix-1)*NCOUNTERS_FO   #run1:15
    elif self.findFO(cntlabel,"l1spurious"):
      foix=self.findFO(cntlabel,"l1spurious")
      board="fo"; c= cntlabel[13]; 
      if c=='T': c='0'
      n= 28 + int(c) +FOSH+(foix-1)*NCOUNTERS_FO   #run1:22
    elif self.findFO(cntlabel,"ppi"):
      foix= self.findFO(cntlabel,"ppi")
      board="fo"; n= 40+FOSH+(foix-1)*NCOUNTERS_FO   #run1:29
    elif self.findFO(cntlabel,"l0clstt"):
      foix= self.findFO(cntlabel,"l0clstt")
      board="fo"; n= 41+FOSH+(foix-1)*NCOUNTERS_FO   #run1:30
    elif self.findFO(cntlabel,"l1clstt"):
      foix= self.findFO(cntlabel,"l1clstt")
      board="fo"; n= 42+FOSH+(foix-1)*NCOUNTERS_FO   #run1:31
    elif self.findFO(cntlabel,"l0clst"):
      foix=self.findFO(cntlabel,"l0clst")
      board="fo"; c= cntlabel[9]   # foNl0clstX
      n= 0 + int(c) +FOSH+(foix-1)*NCOUNTERS_FO   #run1/2:0
      #print "foix:",foix, c, n
    elif self.findFO(cntlabel,"l1clst"):
      foix=self.findFO(cntlabel,"l1clst")
      board="fo"; c= cntlabel[9]
      n= 8 + int(c) +FOSH+(foix-1)*NCOUNTERS_FO   #run1:6
    elif self.findFO(cntlabel,"ppout"):
      foix=self.findFO(cntlabel,"ppout")
      board="fo"; c= cntlabel[8]
      n= 42 + int(c) +FOSH+(foix-1)*NCOUNTERS_FO   #run1:31
      if col5: det5c= self.findLTUfo(foix, int(c))
    elif self.findFO(cntlabel,"l0out"):
      foix=self.findFO(cntlabel,"l0out")
      board="fo"; c= cntlabel[8]
      n= 46 + int(c) +FOSH+(foix-1)*NCOUNTERS_FO   #run1:35
      if col5: det5c= self.findLTUfo(foix, int(c)) 
    elif self.findFO(cntlabel,"l1out"):
      foix=self.findFO(cntlabel,"l1out")
      board="fo"; c= cntlabel[8]
      n= 50 + int(c) +FOSH+(foix-1)*NCOUNTERS_FO   #run1:39
      if col5: det5c= self.findLTUfo(foix, int(c)) 
    elif self.findFO(cntlabel,"l2stro"):
      foix=self.findFO(cntlabel,"l2stro")
      board="fo"; c= cntlabel[9]
      n= 54 + int(c) +FOSH+(foix-1)*NCOUNTERS_FO   #run1:43
      if col5: det5c= self.findLTUfo(foix, int(c)) 
    elif self.findFO(cntlabel,"l2rout"):
      foix=self.findFO(cntlabel,"l2rout")
      board="fo"; c= cntlabel[9]
      #n= 29+14 + int(c) +FOSH+(foix-1)*NCOUNTERS_FO
      #n= BYSH + NCOUNTERS_BUSY_L2RS -1 + int(c) + (foix-1)*4 #run1:
      n= 58 + int(c) +FOSH+(foix-1)*NCOUNTERS_FO
      if col5: det5c= self.findLTUfo(foix, int(c)) 
    #------------------------------------------------------ INT
    elif string.find(cntlabel,"intCTPbusy")==0:
      board="int"; n= 0+ INTSH; CGT='T';
    elif string.find(cntlabel,"fiLF")==0:
      board="int"; n= 1+ INTSH; CGT='T';
    elif string.find(cntlabel,"inttime")==0:
      board="int"; n= 2+ INTSH; CGT='T';
    elif string.find(cntlabel,"int1")==0:
      board="int"; n= 3+ INTSH
    elif string.find(cntlabel,"int2")==0:
      board="int"; n= 4+ INTSH
    elif string.find(cntlabel,"intL1S")==0:
      board="int"; n= 5+ INTSH
    elif string.find(cntlabel,"intL2S")==0:
      board="int"; n= 6+ INTSH
    elif string.find(cntlabel,"L2rseq")==0:
      board="int"; n= 7+ INTSH
    elif string.find(cntlabel,"L2aseq")==0:
      board="int"; n= 8+ INTSH
    elif string.find(cntlabel,"rd_block")==0:
      board="int"; n= 9+ INTSH
    elif string.find(cntlabel,"rc_block")==0:
      board="int"; n= 10+ INTSH
    elif string.find(cntlabel,"orbit_err")==0:
      board="int"; n= 11+ INTSH
    elif string.find(cntlabel,"ctpbusy_pulse")==0:
      board="int"; n= 12+ INTSH
    elif string.find(cntlabel,"lf_pulse")==0:
      board="int"; n= 13+ INTSH
    elif string.find(cntlabel,"L2a_tc")==0:
      board="int"; n= 14+ INTSH
    elif string.find(cntlabel,"tc_input1")==0:
      board="int"; n= 15+ INTSH
    elif string.find(cntlabel,"tc_input2")==0:
      board="int"; n= 16+ INTSH
    elif string.find(cntlabel,"incomplete")==0:
      board="int"; n= 17+ INTSH
    elif string.find(cntlabel,"orc_error")==0:
      board="int"; n= 18+ INTSH
    else:
      myw.errorprint(self, "unknown counter name:"+cntlabel)
    if col5: 
      return n,board,CGT,det5c
    else:
      return n,board,CGT
  def findFO(self, cntlabel,cname):
    if cntlabel[:2]=='fo' and cntlabel[3:3+len(cname)]==cname:
      return int(cntlabel[2])
    else:
      return None
  def makeit1(self, board,label, items, helptext):
    l0inp= myw.MywMenuList(self.boardfs[board], items=items, side=TOP,
      label=label, cmd=self.indexmod, #showactive=1,
      defaults=self.findCnts(label, items),
      helptext=helptext)
    #if label[0:2]=='fo': print label,items
    if self.printnames==None: return
    # following code executed only when printing all the counter names 
    # to stdout i.e.:
    # cd $VMEBDIR ; ./counters.py printnames >~/cnames
    #
    #print "makeit1:", label,"\n",items
    for ix in range(len(items)):
      if self.directNames(label):
        cname= items[ix]
      else:
        cname= label+items[ix]
      cntaddr,bname,cgt,col5=self.findAddr(cname, "Give_Column5")
      #ibname vs. board explanation: e.g.: bname: fo board: fo5
      #print cname, cntaddr, bname
      print cname, cntaddr, board, cgt, col5
  def findLTUAddr(self, cntlabel):
    """
    rc: Counter address,"ltu",CGT
        or None for non-existing counter
    Counter address is position in shared memory (0,...)
    CGT: Counter, Gauge, Timer
    """
    if self.ctpltu.has_key(cntlabel):
      rc= self.ctpltu[cntlabel][0], "ltu", self.ctpltu[cntlabel][1],\
        self.ctpltu[cntlabel][2]
    else:
      rc=None
    return rc
  def addReg(self, cntname):
    """
    cntname -counter name (label appearing in counter widget)
    """
    cntaddr,bname,cgt=self.findAddr(cntname)
    #print "addReg1:", cntname, cntaddr, bname, len(self.allregs), len(self.regs)
    #self.addr.printEntry('MultRegaddReg:')
    if cntaddr==None: return
    if bname==None: color= myw.COLOR_BGDEFAULT
    else:
      color=myw.VmeBoard.CTPcolors[bname]
    if self.__class__.__name__ =='VMEcnts':
      newval= self.allregs[cntaddr]
      r= VMECounter(self, cntname, cntaddr, color, newval)
    else:
      r= SHMCounter(self, cntname, cntaddr, color)
    self.regs.append(r)
  def addLTUReg(self, cntname):
    """
    cntname -counter name (label appearing in counter widget)
    """
    cntaddr,bname,cgt,htext=self.findLTUAddr(cntname)
    #print "addLTUReg1:", cntname, cntaddr, bname
    #print "addReg2:", self.rgitems
    #self.addr.printEntry('MultRegaddReg:')
    if cntaddr==None: return
    if bname==None: color= myw.COLOR_BGDEFAULT
    else:
      color=myw.VmeBoard.LTUcolor
    if self.__class__.__name__ =='VMEcnts':
      newval= self.allregs[cntaddr]
      r= VMECounter(self, cntname, cntaddr, color, newval,htext)
    else:
      r= SHMCounter(self, cntname, cntaddr, color)
    self.regs.append(r)
  def directNames(self, butlab):
    """ Return: 1 if group with direct names
                None if the name is to be created: butlab + item[]
    """
    if butlab=="l0counters" or butlab=="l0timers" or\
      butlab=="l1timers" or butlab=="l2timers" or\
      butlab=="bytimers" or butlab=="bycounters" or\
      butlab=="inttimers" or butlab=="intcounters" or\
      butlab=="Strobes,ESR" or\
      butlab=="Strobes" or\
      self.findFO(butlab,"others")!=None:
      return 1
    else:
      return None
  def indexmod(self,inst=None,ix=None):
    """ 
    2 types of MywMenuList (both optionaly with showactive=1):
    1. items=["1","2","3"...]
    2. items=["name1","name2",...] -> to BE INCLUDED BELOW:
    """
    #print "indexmod:...",inst,ix
    cntname= inst.getLabel()
    newvalue= inst.getEntry(ix)
    #print "indexmod:",cntname,'<',ix,':', newvalue
    cntgroup= string.split(cntname,":")
    # use cntgroup[0], ix to find out:
    # cntname -name of the counter (as it appears in 'CTP counters' widget)
    # address of the counter
    # board it belongs to (busy,l0,l1,l2,fo,int)
    if self.directNames(cntgroup[0]):
      cntname= inst.getItem(ix) 
    else:
      #cntname= cntgroup[0] + str(ix+1)
      cntname= cntgroup[0] + inst.getItem(ix)
    cntaddr,board,cgt=self.findAddr(cntname)
    if newvalue==1:   # add counter
      self.addReg(cntname)
    else:             # remove counter
      ixreg= self.find(cntname)
      #print "indexmod.ixreg:",ixreg
      self.regs[ixreg].removecnt()
      del(self.regs[ixreg])
  def repeatRead(self, cancel=None):
    #print "repeatReead:",self.perrep
    if self.perrep >= 10 or cancel:
      self.mrmaster.after_cancel(self.afterid)
      self.setColor()
      self.perrep=0
    else:
      self.allread()
      self.afterid=self.mrmaster.after(1000, self.repeatRead)
      self.perrep= self.perrep+1
  def setColor(self,color=None):
    if color==None:
      color=self.normalcolor
    self.butts.buttons[2].configure(bg=color[0], 
      activebackground=color[1])
  def perRead(self):
    #print 'perRead.perrep:',self.perrep
    #self.allread()
    if self.perrep == 0:   #just starting
      self.setColor((myw.COLOR_RUNNING,myw.COLOR_RUNNING))
      self.repeatRead()
    else:                  # request to stop periodic read
      self.repeatRead(cancel='yes')
      self.setColor()
  def find(self, cntlabel):
    """ among shown counters """
    for ix in range(len(self.regs)):
      if cntlabel== self.regs[ix].cntname:
        return ix
    return None
  def findCnts(self, nbase, items):
    """
    2 types, according to items.
    if items[0]=="1":
       -counter names: nbase1,nbase2,... -these are chceked
        for 'being shown/notshown'
    else:
       -counter names: items[0],items[1],...
        nbase is not used, just check 'which of the items[*]'
        are shown/notshown
    return (0,0,1,1,0,...) for all counters 
    starting with string nbase, where:
    0 -counter is not shown
    1 -counter is shown
    I.e. if counters 
    L0_inp2 and L0_inp4 are choosen, findCnts(L0_inp) returns:
    (0,1,0,1) -doesn't have to be the same # of items as
    counters of given type (if less, the remaining are 0).
    """
    maxn=len(items)
    dfs=[]
    for ix in range(maxn):
      dfs.append(0)
    if items[0]=="1":
      for ix in range(len(self.regs)):
        lbl= self.regs[ix].cntname   #name1, nameT
        if string.find(lbl, nbase)==0:
          #lbl= int(lbl[len(nbase):])-1
          # dfs[n]= 1
          for ix2 in range(len(items)):    # 1, T
            if nbase+items[ix2]==lbl:
              dfs[ix2]=1
    else:   # items (list of strings -counters names)
      for ix in range(len(self.regs)):
        lbl= self.regs[ix].cntname
        for ix2 in range(len(items)):
          if items[ix2]==lbl:
            dfs[ix2]=1
    return dfs
  def clearCounters(self):
    myw.vbexec.vbinst.io.execute("clearCounters()")
  def hideregsframe(self,event):
    global cntsactive
    cntsactive=0
    #print "hideregsframe...",event.widget
    pass
  def hideaddw(self,event):
    #print "hideaddw...",event.widget
    self.addw=None
  def hidemrm(self,event):
    #print "hidemrm...",event.widget
    #print "       ...", self.mrmaster
    if self.perrep>0:
      #self.finish()
      self.perrep=0
    #if event.widget== self.mrmaster:   #invoked as the very last
    if myw.compwidg(event, self.mrmaster):
      print "hidemrm-mrmaster...",event.widget
      self.finish()
      if self.saveconf:
        cf= open(self.defcounters,"w")
        #myw.vbexec.vbinst.io.execute(cmd,ff=self.doout)
        myw.vbexec.printmsg("Writing choosen counters names to file:"+self.defcounters+"\n")
        for ix in range(len(self.regs)):
          cntlabel= self.regs[ix].cntname
          cf.write(cntlabel+'\n')
        cf.close()
      if self.addw:
        print "destroying addreg-widget:",self.addw
        self.addw.destroy()
  def changeacc(self):
    self.showaccrual=1-self.showaccrual
    if self.showaccrual==1:
      self.butts.buttons[1].setLabel("Increments")
    else:
      self.butts.buttons[1].setLabel("Abs. values")
  def getregsframe(self):
    """ self.regsframe: valid for SHMcounter...
    For VMEcounter, getregsframe() is used to get right frame
    where to place the counter entry
    """
    #print "getregsframe:",len(self.regs)
    if (len(self.regs) % CTPcnts.COLSIZE) == 0:
      self.regsframes.append( myw.MywFrame(self.regsframe, side=LEFT))
    return self.regsframes[-1]
class VMEcnts(CTPcnts):
  def __init__(self, ctpltu=None, printnames=None):
    #print "hereVMEcnts_init_:",printnames
    self.printnames=printnames
    CTPcnts.__init__(self, ctpltu, printnames=printnames)
    #self.ctpltu=ctpltu
    #print "VMECnts__init__:",ctpltu,self.defcounters
  def init(self):
    #self.printnames=None
    pass
  def finish(self):
    pass
  def allread(self):
    cmd2=""
    #for i in range(len(self.regs)):
    #  addr= self.regs[i][1]
    #  cmd2=cmd2+','+str(addr)
    #print 'allread  :', self.ctpltu, self.numberofcounters
    # sep2014 change: getCounters returns always abs. values. Calculate increments here.
    cmd="getCounters("+str(self.numberofcounters)+","+str(self.showaccrual)+",2)"
    thdrn=myw.vbexec.vbinst.io.execute(cmd,ff=self.doout)
    #print 'CTPcounters.cmd:',cmd,thdrn
    # doing nothing: myw.vbexec.vbinst.io.thds[thdrn].waitcmdw()
    #print "allread finished"
  def doout(self,allvals):
    #print "allread3:",allvals,':'
    vlist=string.split(allvals,'\n'); vll= len(vlist)-1
    if vll != len(self.allregs):
      myw.errorprint(self, "doout: %d != %d"%(vll,len(self.allregs)))
      myw.errorprint(self, allvals)
    #print "allread2:",vll,len(self.allregs),"\n", vlist[152:253]
    for i in range(vll):
      self.allregs[i]= vlist[i]
    for i in range(len(self.regs)):
      #prevent= self.regs[i].cntentry.getEntryHex()
      prevent= self.regs[i].cntentry.getEntry()
      cntnum= self.regs[i].cntaddr
      if prevent==" " or prevent=="": 
        newval= self.allregs[cntnum]
        #print "allread4:",prevent,type(prevent),cntnum, newval
        self.regs[i].cntentry.setEntry(newval)
        self.regs[i].prevbin= eval(newval)
      else:
        if self.ctpltu!=None:   # LTU; accrual not valid
          if self.showaccrual==1:
            prevbin= self.regs[i].prevbin; newbin= eval(self.allregs[cntnum])
            newval= "%d"%(self.dodif32(prevbin, newbin))
          else:
            newval= "%d"%(eval(self.allregs[cntnum]))
            newbin= eval(self.allregs[cntnum])
          #print "allread:",prevval,type(prevval),vlist[i],cntnum
          #if (cntnum==0) and (self.showaccrual==1):
          #  secs= eval(newval)*0.4/1000000.
          #  print "allread-time:%d/%d"%(i,cntnum),prevbin, newval, "secs:",secs
          #print "allread5:%s:%s:"%(prevent, newval)
        else:                   # CTP: accrual valid
          newval= self.allregs[cntnum]
          newbin= eval(self.allregs[cntnum])
        if prevent != newval:
          self.regs[i].cntentry.setColor(myw.COLOR_VALCHANGED)
          self.regs[i].cntentry.setEntry(newval)
        else:
          self.regs[i].cntentry.setColor()
        self.regs[i].prevbin= newbin
  def dodif32(self, before, now):
    if now >= before: 
      dif= now-before
    else: 
      dif= now+ (0xffffffff-before) +1
    #if(DBGcnts) printf("dodif32:%d\n", dif)
    return dif


class SHMcnts(CTPcnts):
  FIFONAME="/tmp/dataready"
  def init(self):
    global shmext
    shmext=__import__("shmpyext")
    self.stophistorygram=None   # thread stopping
    self.historythd=None        # thread not started
  def finish(self):
    try:
      self.setColor()
    except:
      pass
    self.stophistorygram=1
    if self.historythd: self.historythd.join()
    #myw.vbexec.vbinst.io.thds[thdrn].waitcmdw()
  def changeacc(self):
    print "no effect for shared memory counters (always accruals)"
  def checkShift(self):
    if self.nx>=SHMCounter.nmax:
      ixmaxdel=SHMCounter.nmax/2
      for i in range(len(self.regs)):
        self.regs[i].updateShift(ixmaxdel)
      self.nx=SHMCounter.nmax-ixmaxdel
  def allread(self):
    self.checkShift()
    for i in range(len(self.regs)):
      #addr= self.regs[i][1]
      addr= self.regs[i].cntaddr
      val=shmext.getcnt(i)
      #print "allreadshm:", addr,val
      self.regs[i].update()
    self.nx= self.nx+1
  def repeatRead(self, cancel=None):
    print "shmrepeatRead.cancel:",cancel
    if cancel:
      self.stophistorygram=1   # thread stopping
      #n=shmext.startstopfw(0)
      #print "RC from stopping fif:",n, "closing fifo..."
      #self.fifo.close()
      #self.perrep=0
      return
    #thread= threading.Thread(self.historygram,arg=(1))
    self.historythd= threading.Thread(target=self.historygram)
    self.stophistorygram=None
    self.historythd.start()
  def historygram(self, first=None):
    """ started as thread
    """
    if shmext.startstopfw(1) != 1:
      myw.errorprint(self,"Cannot open shm")
      self.setColor()
      return
    print "Opening", SHMcnts.FIFONAME   #, "(is hwreader started?)"
    try:
      self.fifo= open(SHMcnts.FIFONAME,"r")
    except:
      myw.errorprint(self, SHMcnts.FIFONAME+" cannot be opened for reading")
      self.setColor()
      return
    emptylines=0
    #for n in range(10):
    while 1:
      #time.sleep(1)
      line= self.fifo.readline()
      if line=='':
         emptylines= emptylines+1
         print "got empty line (max. 10x)"
         if emptylines<10: continue
      if line=='' or self.stophistorygram:
        n=shmext.startstopfw(0)
        print "RC from stopping fifo:",n," closing fifo..."
        self.fifo.close()
        self.perrep=0
        if line=='':
          print "got empty line"
          #time.sleep(0.3)
        break
      print "hg:", line[:-1]
      self.perrep= self.perrep+1
      self.checkShift()
      for i in range(len(self.regs)):
        self.regs[i].update()
      if os.path.exists(CTPcnts.specfname):
        try:
          execfile(CTPcnts.specfname, globals(), locals())
        except:
          print "exception caught executing:",CTPcnts.specfname
      self.nx= self.nx+1
    print "hg: finishing"
    self.setColor() # nedobre?

def main():
  myw.vbinit(None)
  f = Tk()
  p1=''
  if len(sys.argv)>1: 
    p1=sys.argv[1];
    if p1=='printnames':
      cv=VMEcnts(printnames='Yes')
      #cv.printnames='Yes'
      cv.addnewcnt()
  else:
    print """Usage:
./counters.py                   -to get 'counters over time' graph
./counters.py printnames        -print counter names to stdout
Usually:
./counters.py printnames >~/cnames
sort -n -k 2 -t ' ' ~/cnames >~/cnames.sorted2

"""
  return
  #f.bind("<Destroy>", fdestroy)
  SHMcnts(tlw=f)
  #VMEcnts(f)
  f.mainloop()

if __name__ == "__main__":
  main()
