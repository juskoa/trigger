#! /usr/bin/env python
"""
22.11. -write2hw only modified ctp config. values
2 phases:
1.  Classes Klas,PFboard,PFcircuit,Fanout, Attr (Attr2, AttrLUT)
have variable 'writeme' and corresponding methods (dedicated from
class Genhw). Their methods readhw/writehw update writeme
variable automatically:
readhw(line) 
 - readhw (update writeme=0) or loadfrom line (writeme=1)
writehw(cf)
 - write2hw always and update hwwritten(1)
 - save2file always (don't touch writeme) 

Method writehw() should not be called if not necessary -this
can be found out by calling modified() method. E.g. see 
Ctpconfig.writehw()

Attributes of the instances of these classes can be changed:
- from the widget -> hwwritten(0) called from inside the class method
   (usually handler routine)
- from outside -> if modified not through readhw/writehw,
   the call hwwritten(0) should be arranged approppriately

I.e. by this arrangement, all the conf. parameters are divided
into groups (e.g. if only one attribute of PFcircuit object was
modified, when written to hw, all the attributes of this PFcircuit
will be written (the whole group))

2. phase (not implemented -we do not use shared memory):
shared memory and check in ctp.c is organised as follows:
read:  - don't read from hw (instead read from memory)
       - possible 'forced' read and check 
write: - don't write to hw, if the memory is the same
       - possible 'forced' write
10.1.2006
fixed: incorrect group-2 shared resources (INTSEL) reading from ctpcfg file 
1.4. BUSY board added
"""

from Tkinter import *
import string,os,types
import myw,txtproc
import trigdb

COLOR_SHARED="#cc66cc"
COLOR_FOS="#cc6600"
COLOR_BUSY= myw.VmeBoard.CTPcolors["busy"]
COLOR_PFS="#cc0066"
COLOR_LUT="#ffccff"
COLOR_BPATTERN="#00ccff"
ORBITLENGTH=3564

def IntErr(fstr):
  print "Internal error:",fstr
def PrintError(fstr):
  print "Error:",fstr
def prt(o, *pars):
  print "DBGPRT in ", o.__class__.__name__,":",pars

def InvertBit(w32,bit):   # bit: 0..31
  mi= w32 & (1<<bit)
  if mi:  
    iw32= w32 & (w32 & (~mi))
  else:   
    iw32= w32 | (1<<bit)
  #print "InverBit %d: 0x%x -> 0x%x"%(bit,w32,iw32)
  return iw32

vbexec= myw.vbexec
l0abtxt= vbexec.get2("l0AB()")
Gl0AB= int(l0abtxt[0])
if Gl0AB==0: Gl0AB= None   # None: L0 fi: >0xAB
print "Gl0AB:",Gl0AB

class Genhw:
  def __init__(self,attrs=None):
    self.attrs=attrs
    if attrs==None:
      self.writeme=1
  def modified(self):
    """ Test if HW != MEMORY
    rc: 1 if this class (descendant of this class) was changed,i.e.
           HW contains different value
        0 if HW registers are the same as attributes
    """
    if self.attrs==None:
      return self.writeme
    else:
      for atr in self.attrs:
        #print "Genhw.modified:",atr.modified(), atr.atrname
        if atr.modified():
          return 1
      return 0
  def hwwritten(self, newval=1):
    """ Set condition: if HW == MEMORY
    newval:
    1 -to be called when memory equals to HW, i.e. after:
       - reading HW
       - writing HW
    0 -to be called always after attribute value changed
       i.e. memory differs from HW -always after:
       - value modification from the screen
       - value modification by other program (not considered yet-
         perhaps after introducing 'shared memory')
       - value modification by reading from file
    """
    if self.attrs==None:
      self.writeme=1-newval
      if self.writeme==1: wcolor='red'
      else: wcolor='green'
      vbexec.setWarning(wcolor)
    else:
      for atr in self.attrs:
        #print "Genhw.hwwritten:",newval, atr.atrname
        atr.hwwritten(newval)

class Ctpconfig:
  NCLASS= 100
  clusterx0=25 #17  left top corner of cluster's rectangles 
  l0x0=clusterx0+13 # left top corner of classes L0 bit rectangles 
  l0y0=10         # class 0 -not class, instead reserved for text header
  hty0= 3 #was 10         # header text line
  int1bits=["INTfun1","BC1","BC2","RND1","RND2"]
  int2bits=["INTfun2","BC1","BC2","RND1","RND2"]
  #vonint2bits=[("i0",1),("i1",1),("i2",1),("i3",1),("i4",1),"INTfun2","BC1","BC2","RND1","RND2"]
  AllRarebits=["All"]
  lastshrgrp1=8   # index in self.sharedrs: eof LUTs
  if Gl0AB==None:
    firstshrgrp2= lastshrgrp1+8    # BCM1...
    lastshrgrp2= firstshrgrp2+12
    grp3start= lastshrgrp1+4
    dbgbits=12 #real version:12   debug:6 (change also shared.c LEN_l0f34=64)
    mskCLAMASK=0x80000000
    BCMASKN=12
  else:
    firstshrgrp2= lastshrgrp1+4
    lastshrgrp2= firstshrgrp2+4
    mskCLAMASK=0x10000
    BCMASKN=4
  def __init__(self, vb):
    self.l0AB= Gl0AB
    #print "Ctpconfig: l0AB:",self.l0AB
    self.caclstl= None   # Classes toplevel window not displayed
    self.canvas=None     # Canvas (1 class per line) not displayed
    self.allorenabled=0   # 1->all 0->only enabled classes shown in 'Classes'
    self.bcmasks=[];     # 4/12-bits words
    #cct= vbexec.get1("notInCrate(1)") ;print "Ctpconfig:",cct
    #cct= vbexec.get2("notInCrate(1)") ;print "Ctpconfig:",cct
    for i in range(ORBITLENGTH):
      self.bcmasks.append(0)
    self.shrtl=None      # Shared resources not displayed
    if self.l0AB:
      print "l0AB..."
      self.sharedrs= [
      AttrRndgen('RND1',0, TrgSHR.RNDxHelp+myw.frommsRandomHelp),
      AttrRndgen('RND2',0, TrgSHR.RNDxHelp+myw.frommsRandomHelp),  # 2 RND inputs
      Attr('BC1', 0, TrgSHR.BCxHelp+myw.frommsHelp),
      Attr('BC2', 0, TrgSHR.BCxHelp+myw.frommsHelp), # 2 BC scaled down inputs 
      AttrLUT('INTfun1',["0x0",4,0], TrgSHR.L0FUNxHelp),
      AttrLUT('INTfun2',["a|d",4,0], TrgSHR.L0FUNxHelp),
      AttrLUT('INTfunT',["a&b&(c|d)",4,0], TrgSHR.L0FUNxHelp),
      AttrLUT('L0fun1',["a|b|c|d",4,0], TrgSHR.L0FUNxHelp),
      AttrLUT('L0fun2',["a|b|c|d",4,0], TrgSHR.L0FUNxHelp),#eof grp1: lastshrgrp1
      AttrBits('INT1',0, helptext=TrgSHR.INTxHelp,bits=Ctpconfig.int1bits),
      AttrBits('INT2',0, helptext=TrgSHR.INTxHelp,bits=Ctpconfig.int2bits),
      AttrBits('All/Rare',0, helptext=TrgSHR.AllRareHelp,
        bits=Ctpconfig.AllRarebits),   # lastshrgrp1+3
      AttrBCmask('BCM1','bitmap',TrgSHR.BCMxHelp, self),  #firstshrgrp2
      AttrBCmask('BCM2','bitmap',TrgSHR.BCMxHelp, self),
      AttrBCmask('BCM3','bitmap',TrgSHR.BCMxHelp, self),
      AttrBCmask('BCM4','bitmap',TrgSHR.BCMxHelp, self)
      ]
    else: #firmAC
      print "l0AC..."
      self.sharedrs= [
      AttrRndgen('RND1',0, TrgSHR.RNDxHelp+myw.frommsRandomHelp),
      AttrRndgen('RND2',0, TrgSHR.RNDxHelp+myw.frommsRandomHelp),  # 2 RND inputs
      Attr('BC1', 0, TrgSHR.BCxHelp+myw.frommsHelp),
      Attr('BC2', 0, TrgSHR.BCxHelp+myw.frommsHelp), # 2 BC scaled down inputs 
      AttrLUT('INTfun1',["0x0",4,0], TrgSHR.L0FUNxHelp),
      AttrLUT('INTfun2',["a|d",4,0], TrgSHR.L0FUNxHelp),
      AttrLUT('INTfunT',["a&b&(c|d)",4,0], TrgSHR.L0FUNxHelp),
      AttrLUT('L0fun1',["a|b|c|d",4,0], TrgSHR.L0FUNxHelp),
      AttrLUT('L0fun2',["a|b|c|d",4,0], TrgSHR.L0FUNxHelp), #lastshrgrp1
      AttrBits('INT1',0, helptext=TrgSHR.INTxHelp,bits=Ctpconfig.int1bits),
      AttrBits('INT2',0, helptext=TrgSHR.INTxHelp,bits=Ctpconfig.int2bits),
      AttrBits('All/Rare',0, helptext=TrgSHR.AllRareHelp,
        bits=Ctpconfig.AllRarebits),   # lastshrgrp1+3
      AttrLUT('L0fun31',["a|b|c|d|e|f",Ctpconfig.dbgbits,0], TrgSHR.L0FUN34Help),
      AttrLUT('L0fun32',["a|b",Ctpconfig.dbgbits,0], TrgSHR.L0FUN34Help),
      AttrLUT('L0fun41',["a|b",Ctpconfig.dbgbits,0], TrgSHR.L0FUN34Help),
      AttrLUT('L0fun42',["a|b",Ctpconfig.dbgbits,0], TrgSHR.L0FUN34Help),#eof grp1, pointed by lastshrgrp1+7
      AttrBCmask('BCM1','bitmap',TrgSHR.BCMxHelp, self),  #firstshrgrp2
      AttrBCmask('BCM2','bitmap',TrgSHR.BCMxHelp, self),
      AttrBCmask('BCM3','bitmap',TrgSHR.BCMxHelp, self),
      AttrBCmask('BCM4','bitmap',TrgSHR.BCMxHelp, self),
      AttrBCmask('BCM5','bitmap',TrgSHR.BCMxHelp, self),
      AttrBCmask('BCM6','bitmap',TrgSHR.BCMxHelp, self),
      AttrBCmask('BCM7','bitmap',TrgSHR.BCMxHelp, self),
      AttrBCmask('BCM8','bitmap',TrgSHR.BCMxHelp, self),
      AttrBCmask('BCM9','bitmap',TrgSHR.BCMxHelp, self),
      AttrBCmask('BCM10','bitmap',TrgSHR.BCMxHelp, self),
      AttrBCmask('BCM11','bitmap',TrgSHR.BCMxHelp, self),
      AttrBCmask('BCM12','bitmap',TrgSHR.BCMxHelp, self),
      ]
    self.interactionsel=0
    self.ltus=trigdb.TrgLTUS()
    # PF stuff:
    self.pfbs= []   # 3 boards
    self.pfwcs= []   # references to self.pfbs.pfcs
    ixinbt=1
    for ix in range(3):   # 3 boards
      cct= vbexec.get2("notInCrate("+str(ixinbt)+")")
      if len(cct)<1:
        print "cctincfg:",ixinbt,cct
        print """ comment L0_INTERACT1/2 lines in ctp.cfg
at least when ctp boards are not configured. By other words:
ctp should be initailized always by ctp_proxy, while ctp expert software
should be started alwasy in nbi-mode (nO bOARD iNIT).
"""
      if cct[0]=='0':
        self.pfbs.append(PFboard(ix))
      ixinbt= ixinbt+1
    if len(self.pfbs)==0:
      print "Ctpconfig:lenpfbs:",len(self.pfbs), self.pfbs
      print "At least 1 board of L0/1/2 necessary, check /root/NOTES/boardsincrate"
      return
    for ix in range(5):   # 5 whole PF circuits (1,2,3,4,T)
      self.pfwcs.append(PFwholecircuit(ix+1, self))
    # L0/1/2:
    cct= vbexec.get2("notInCrate(1)")
    if len(cct)<1:
      print "Ctpconfig:cct:",cct
    if cct[0]=='0':   # L0 in the crate
      self.readShared()
      #dbgssmo= vbexec.get1("gettableSSM()")
      #print "dbgssm: before getClass",dbgssmo
      self.klasses=[]
      for ix in range(1,Ctpconfig.NCLASS+1):   #keep this order ! 
        k=Klas(self,ix)
        k.readhw()    # should be OK with shared memory
        self.klasses.append(k)
      #dbgssmo= vbexec.get1("gettableSSM()")
      #print "dbgssm: after getClass",dbgssmo
    else:
      self.klasses= [ Klas(self,31, 0xaa55, 0x3230),
        Klas(self, 8, 0xaa55, 0x3232)]
    # FOs:
    self.fanouts=[]; ixinbt=5; 
    self.fotl=None   # fanouts/busyclusters not displayed
    for ix in range(6):
      cct= vbexec.getline("getFO("+str(ixinbt)+")")   # cct: 'cl tcl'
      #print "Ctpconfig.fo:",cct, type(cct), len(cct)
      if len(cct)>0:
        self.fanouts.append(Fanout(self,ix+1,None))
        # let's read FO once more when initialised
      ixinbt= ixinbt+1
    #dbgssmo= vbexec.get1("gettableSSM()")
    #print "dbgssm: after getFO",dbgssmo
    #BUSY:
    cct= vbexec.get2("notInCrate(0)")
    if cct[0]=='0':   # BUSY in the crate
      self.busyboard= BusyBoard(self)
    else:
      self.busyboard=None
    #dbgssmo= vbexec.get1("gettableSSM()")
    #print "dbgssm: after BusyBoard",dbgssmo
  def findKlass(self,clnumber):
    for cl in self.klasses:
      if cl.clnumber==clnumber: return cl
    return None
  def getl0scalerx12(self):
    return [Klas.l0scalerx0, Klas.l1inputsx0]
  def doClasses(self):
    if self.caclstl:
      myw.RiseToplevel(self.caclstl); return
    self.caclstl= myw.NewToplevel("CTP classes",self.classesDestroyed)
    print "doClasses:", self.caclstl
    self.cmdbuts= myw.MywHButtons(self.caclstl, [
      ("blabla",self.cmdallenabled)
      ],side=TOP, helptext="""
All/Enabled -show all or only enabled classes. This button becomes red
             if some ENABLED classes are not shown
""")
    self.doCanvas()
  def doCanvas(self):
    if self.allorenabled==1:
      nclines= len(self.klasses)
      aetx=Klas.txenabled
    else:
      aetx=Klas.txall
      nclines=0
      for k in self.klasses:
        if (k.l0vetos & Ctpconfig.mskCLAMASK)==0:   #class enabled
          nclines= nclines+1
    self.cmdbuts.buttons[0].setLabel(aetx)
    self.cmdbuts.buttons[0].setColor(myw.COLOR_BGDEFAULT)
    if nclines>30: nclines=30   #show max. 30 classes (with scrollbar)
    canvash= (nclines+1)*myw.Kanvas.bitHeight + Ctpconfig.l0y0
    canvashmax= (Ctpconfig.NCLASS+1)*myw.Kanvas.bitHeight + Ctpconfig.l0y0
    canvasw= Klas.l2vetosx0 + myw.Kanvas.bitWidth*4 + myw.Kanvas.interspace   #980
    if self.canvas: print "error -canvs not deleted"
    self.freenumber=1   #line (from 1) on canvas
    self.canvas= myw.Kanvas(self.caclstl,self.canvasDestroyed, ctpcfg=self,
      width=canvasw,height=canvash,
      scrollregion=(0, 0, canvasw, canvashmax),
      background='yellow', borderwidth=1)
    id0=self.canvas.create_text(1, Ctpconfig.hty0,
      anchor=NW,text="Cl#")
    self.canvas.doHelp(id0, """Class number and Cluster it belongs to""")
    id1=self.canvas.create_text(Ctpconfig.l0x0, Ctpconfig.hty0,
      anchor=NW,text="L0 inputs")
    self.canvas.doHelp(id1, 
"""L0 inputs: 1-24, 2 special functions, 2 scaled down BC, 2 random triggers.
Mouse buttons clicks:
Left  -> modify
         red:         don't care (1)
         green:       enabled    (0)
         light green: enabled (0), inverted (1)
Middle-> modify the invert bit for classes 1-50 (45-50 with <AC version of L0-board)
""")
    id2=self.canvas.create_text(Klas.l0vetosx0, Ctpconfig.hty0,
      anchor=NW,text="L0 vetos sel.")
    self.canvas.doHelp(id2,"""L0 selectable vetos. As with L0 inputs:
         red:         don't care    (1)
         green:       veto selected (0)
Bit 'Class mask' must be selected (i.e. green) for active (triggering) classes
""")
    id3=self.canvas.create_text(Klas.l0scalerx0, Ctpconfig.hty0,
      anchor=NW,text="L0 pre-scaler")
    self.canvas.doHelp(id3,
""" L0 pre-scaler. 21 bits (0-no downscaling 0x1fffff-supress all triggers).
% character can be used to enter the down-scale rate. Examples:
%50 -reduce triggers by half
%1  -full trigger rate down-scaled 100 times
""")
    id4=self.canvas.create_text(Klas.l1inputsx0, Ctpconfig.hty0,
      anchor=NW,text="L1 inputs")
    self.canvas.doHelp(id4, """ L1 inputs: 1-24.
Mouse buttons clicks:
Left  -> modify
         red:         don't care (1)
         green:       enabled    (0)
         light green: enabled (0), inverted (1)
Middle-> modify the invert bit (only for classes 45-50)
""")
    id5=self.canvas.create_text(Klas.l1vetosx0, Ctpconfig.hty0,
      anchor=NW,text="L1vetos")
    self.canvas.doHelp(id5,"""L1 vetos
""")
    id6=self.canvas.create_text(Klas.l2inputsx0, Ctpconfig.hty0,
      anchor=NW,text=" L2 inputs")
    self.canvas.doHelp(id6, """ L2 inputs: 1-12.
Mouse buttons clicks:
Left  -> modify
         red:         don't care (1)
         green:       enabled    (0)
         light green: enabled (0), inverted (1)
Middle-> modify the invert bit (only for classes 45-50)
""")
    id7=self.canvas.create_text(Klas.l2vetosx0, Ctpconfig.hty0,
      anchor=NW,text="L2vetos")
    self.canvas.doHelp(id7,"""L2 vetos
""")
    sorted= self.klasses
    # sort klasses according to their cluster:
    # sorted.sort(self.compCluster)
    for k in sorted:
    #for k in self.klasses:
      if self.allorenabled==1:
        k.doClass()
      else:
        if (k.l0vetos & Ctpconfig.mskCLAMASK)==0:   #class enabled
          k.doClass()
  def canvasDestroyed(self,event):
    # this method is automatically invoked (by Tk) , when 
    # canvas' Toplevel window is destroyed
    #print "canvasDestroyed: marking classes 'not-visible'i event:",event
    for k in self.klasses:
      k.linenumber=0      # not visible
    self.freenumber=1   #line (from 1, line0 is text header) on canvas
    self.canvas=None
  def compCluster(self,k1,k2):
    c1= k1.getCluster(); 
    c2= k2.getCluster()
    if c1==0: c1=7   # unassigned classes at the end of the list
    if c2==0: c2=7
    if c1<c2: rc=-1
    elif c1>c2: rc=1
    else: 
      if k1.clnumber<k2.clnumber: rc=-1
      elif k1.clnumber>k2.clnumber: rc=1
      else: 
        rc=0
        IntErr("2 classes with equal numbers:%d %d"%(k1.clnumber,k2.clnumber))
    #print c1,c2,rc
    return rc
  def classesDestroyed(self,event):
    # canvasDestroyed is called automatically 
    # when destroyed. Called 2x, keycode the same 2nd time for more
    # repeatedly created/destryed canvases
    #print "classesDestroyed:",event 
    #print "classesDestroyed2:",event.widget, '\n:', self.caclstl 
    #print "classesDestroyed3:",type(event.widget), '\n:', type(self.caclstl)
    #print "keycode keysym:",event.keycode, event.keysym
    if myw.compwidg(event, self.caclstl):
      #print "==========",dir(event)
      self.caclstl=None
    #self.caclstl=None    # this line added 31.10.2013
    #if (event.widget==self.canvas) or (event.widget==self.caclstl):
    #always (when toplevel 'Classes' or only canvas destroyed):
  def doline(self, xy1, xy2, color="white",width=1):
    self.canvas.create_line(xy1[0],xy1[1],xy2[0],xy2[1],
      fill=color, width=width)
  def doEntry(self, xy, klas_method):
    entw= myw.MywEntry(self.caclstl,bind='lr', label='',
      cmdlabel=klas_method, width=10)
      #cmdlabel=self.entryhandler, width=10)
    #entw= Entry(self.caclstl, font=("Times",8))
    #entw.insert('end','defval'); entw.pack()
    id= self.canvas.create_window(xy[0], xy[1],window=entw,
      anchor=NW)
    #self.canvas.addtag_withtag("TAGl0pr",id)
    #self.canvas.tag_lower(id)
    #err entw.Frame.lower()
    #entw.entry.lower() #not enough (it is in the frame)
    return entw
  #def entryhandler(self,event):
  #  print "entryhandler:"
  def loadfile(self,fname=None):
    #if fname==None:
    #  fname= os.environ['VMECFDIR'] +"/WORK/ctp.cfg"
    cf= open(fname,"r")
    if cf: 
      vbexec.printmsg("Loading file:"+fname+"\n")
    else:
      vbexec.printmsg("Cannot load file:"+fname+"\n")
      return
    newcanvas=None
    if Gl0AB==None:   #firmAC
      llongs= ORBITLENGTH*3
    else:
      llongs= ORBITLENGTH
    for line in cf.readlines():
      lab,rest= string.split(line,None,1)
      rest= rest[:-1]
      #print lab,':',rest,':'
      if lab=='RBIF': 
        self.readSharedline(rest,0, Ctpconfig.lastshrgrp1,sep=':')
      elif lab=='INTSEL': 
        self.readSharedline(rest, 
          Ctpconfig.lastshrgrp1+1, Ctpconfig.lastshrgrp1+3)
      elif lab=='LUT34':
        if Gl0AB==None:
          self.str2L0f34(rest[:(2**Ctpconfig.dbgbits)])
      elif lab=='BCMASKS':
        self.str2masks(rest[:llongs])
      elif lab[:7]=='BCMASK.':
        bcix= int(lab[7])
        #ishr= Ctpconfig.lastshrgrp1+(4-1)+bcix
        ishr= Ctpconfig.firstshrgrp2+bcix-1
        self.sharedrs[ishr].setattrfo(rest, 0)
        #self.sharedrs[ishr].hwwritten(0) #anyhow already set with BCMASKS:
      elif lab[:4]=='CLA.':
        if len(lab)==6:
          clnmb= int(lab[4:6])
          print "Warning: .cfg file: 50 classes format %s..."%lab
        elif len(lab)==7:
          clnmb= int(lab[4:7])
          #print ".cfg file: 100 classes format"
        else:
          PrintError("bad line: %s"%(line))   
          continue
        kl= self.findKlass(clnmb)
        kl.readhw(rest)
        if kl.linenumber==0:     # not displayed
          if (kl.l0vetos&Ctpconfig.mskCLAMASK)==0:
            newcanvas=1          # but choosen
        else:
          kl.refreshClass();
      elif lab[:3]=='FO.':
        clix= int(lab[3])   # FO number (1..6)
        fo= self.findFO(clix)
        if fo:
          fo.readhw(rest)
        else:
          PrintError("FO%d board not in the crate"%(clix))   
      elif lab[:4]=='PFL.':
        pfl= string.split(lab[4:],'.')
        if len(pfl)==1:
          brd= int(lab[4])
          self.pfbs[brd].readhw(rest)
        else:
          pfcirc= (int(pfl[0]), int(pfl[1]))
          pfc= self.findPFC(pfcirc)
          if pfc:
            pfc.readhw(rest)
          else:
            PrintError("PFcircuit "+str(pfcirc)+" not in the crate")
      elif lab[:4]=='BUSY':
        self.busyboard.readhw(rest)
      else:
        PrintError("bad line: %s"%(line))   
    cf.close()
    if newcanvas and self.caclstl:
      #self.canvas.destroy(); self.doCanvas()
      self.cmdbuts.buttons[0].setColor('red')
  def save2file(self,fname=None):
    #workfname= os.environ['VMECFDIR'] +"/WORK/"+fname
    cf= open(fname,"w")
    if cf: vbexec.printmsg("Writing to file:"+fname+"\n")
    self.writeShared(cf)         # shared resources
    for k in self.klasses:       # class definitions
      k.writehw(cf)
    for k in self.fanouts:       # cluster definitions
      k.writehw(cf)
    self.busyboard.writehw(cf)
    for k in self.pfbs:           # pfs definitions
      k.writehw(cf)
      for circ in k.pfcs:                # circuits on 1 board
        circ.writehw(cf)
    cf.close()
  def readhwall(self):
    self.readShared()
    newcanvas=None
    vbexec.get1("hw2rates()")# has to be before following cycle:
    for k in self.klasses:   # class definitions
      k.readhw(); 
      if k.linenumber==0:    # class not displayed
        if (k.l0vetos&Ctpconfig.mskCLAMASK)==0:
          newcanvas=1        # and choosen
      else:
        k.refreshClass()
    #
    for k in self.fanouts:   # clusters defs (FOs)
      k.readhw()
    self.busyboard.readhw()  # busy board
    for k in self.pfbs:       # PF on L0,1,2 boards
      k.readhw()
      for circ in k.pfcs:                # circuits on 1 board
        circ.readhw()
    if newcanvas and self.caclstl:
      #self.canvas.destroy(); self.doCanvas()
      self.cmdbuts.buttons[0].setColor('red')
  def writehwall(self):
    self.writeShared()
    updaterates=None
    for k in self.klasses:   # class definitions
     if k.modified(): 
       k.writehw(); updaterates=1   #in case of at least 1 class changed
    if updaterates: vbexec.get1("rates2hw()")
    for k in self.fanouts:   # clusters defs
     if k.modified(): k.writehw()
    if self.busyboard.modified():self.busyboard.writehw()
    for k in self.pfbs:       # PF on L0,1,2 boards
      if k.modified(): k.writehw()
      for circ in k.pfcs:         # circuits on 1 board
        if circ.modified(): circ.writehw()
  def readSharedline(self, line, ix1, ix2, sep=' '):
    shr=string.split(line, sep)
    inx=0
    for ishr in range(ix1, ix2+1):
      if (len(shr)!=(Ctpconfig.lastshrgrp1+1)) and (inx==0): 
        # INTSEL group, INT1,2 in 1 word
        try:
          binval= eval(shr[0])
        except:
          print "readSharedline: Wrong INTSEL line:",line
          return
        if ishr==(Ctpconfig.lastshrgrp1+1): 
          binval= binval&0x01f
        if ishr==(Ctpconfig.lastshrgrp1+2): 
          binval= (binval>>5)&0x01f
          inx= inx+1
      else:
        binval= eval(shr[inx])
        inx= inx+1
      self.sharedrs[ishr].setattrfo(binval,0)
      #self.sharedrs[ishr].hwwritten(0)
  def readShared(self):
    shr= vbexec.getsl("getShared()")
    # get first (12) values of shared resources from hw
    if len(shr) != 13:      
      # e.g. expected: ['0x20c49b', '0x0', '0x7cf', '0x0', '0x8080', '0xc0c0', '0x0', '0x0', '0x0', '0x8', '0x1', '0x1', '']
      print "Error readShared:",shr
    for ishr in range(len(shr)-1):   #0..11
      v=shr[ishr]
      #print "readShared:",ishr, self.sharedrs[ishr].atrname, v
      self.sharedrs[ishr].setattrfo(eval(v), 1)
      #self.sharedrs[ishr].hwwritten(1)
    if Gl0AB==None:   #firmAC
      shr= vbexec.getsl("getSharedL0f34(4)")
      for ishr in range(12,16):
        v= shr[ishr-12]
        self.sharedrs[ishr].setattrfo(eval(v), 1)
        #print "readShared:value:%d:"%ishr,self.sharedrs[ishr].value
    longs=vbexec.getline("getBCmasks()");
    self.str2masks(longs)
    for ishr in range(Ctpconfig.firstshrgrp2, Ctpconfig.lastshrgrp2):
      self.sharedrs[ishr].setattrfo('bitmap', 1)
      #self.sharedrs[ishr].hwwritten(1)
    # where are pfcs?
  def writeShared(self, cf=None):
    """ cf!=None: write to cONFIG fILE
"""
    intselw= (self.sharedrs[9].getbinval()&0x01f) | \
             ((self.sharedrs[10].getbinval()&0x01f)<<5)
    allrare= self.sharedrs[11].getbinval()&1
    if Gl0AB==None:   #firmAC
      # we cannot do the following:
      lut34= vbexec.getsl("getSharedL0f34(1)")[0]
      #print "writeShared_shared:lut34:",lut34
      #for ishr in range(Ctpconfig.grp3start, Ctpconfig.grp3start+4):
      #  print "writeSharedishr:%x"%ishr, self.sharedrs[ishr].value
      # but we take modifications stored in python-code only:
      longs=[] ; lut34="" #; lut4=[]
      for ishr in range(Ctpconfig.grp3start, Ctpconfig.grp3start+4):
        #longs.insert(0, self.sharedrs[ishr].getbinval())
        longs.append(self.sharedrs[ishr].getbinval())
      #longs[0]:lut31   longs[1]:lut32
      #longs[2]:lut41   longs[3]:lut42
      for ix in range(2**Ctpconfig.dbgbits):
        bm= 2**ix   #bm= 1L<<ixk   ixk:0..4095
        if (longs[3] & bm) != 0: l41= 1L
        else: l41= 0L
        if (longs[2] & bm) != 0: bit= 0x1
        else: bit= 0
        l41= (l41<<1) | bit
        if (longs[1] & bm) != 0: bit= 0x1
        else: bit= 0
        l41= (l41<<1) | bit
        if (longs[0] & bm) != 0: bit= 0x1
        else: bit= 0
        l41= (l41<<1) | bit
        lut34= "%x"%l41 + lut34    #lut4.insert(0,l41)
        #print "bm:",bm,hex(l41)
      #print "writeShared:calut:",len(lut34),"chars,lut34:",lut34
    else:
      lut34= None
    if cf:
      #writeit=1   # write always to file
      cmd="RBIF "
      #for ishr in range(len(self.sharedrs)):
      for ishr in range(Ctpconfig.lastshrgrp1+1):
        if ishr>=4 and ishr<=8:
          cmd= "%s%s:"%(cmd, str(self.sharedrs[ishr].getattr()))
        else:
          cmd= "%s0x%x:"%(cmd, self.sharedrs[ishr].getbinval())
      cmd=cmd[:-1]+"\n"
      cf.write(cmd)
      cmd="INTSEL 0x%x 0x%x\n"%(intselw, allrare)
      cf.write(cmd)
      if lut34: cmd="LUT34 %s\n"%(lut34) ; cf.write(cmd)
      cmd="BCMASKS %s\n"%(self.masks2str())
      cf.write(cmd)
      #write mask-patterns if available (than they will be used
      # instead of bitmap):
      for bcix in range(1,Ctpconfig.lastshrgrp2-Ctpconfig.firstshrgrp2+1):
        ishr= Ctpconfig.firstshrgrp2+bcix-1
        if self.sharedrs[ishr].value!='bitmap':
          cmd="BCMASK.%1d %s\n"%(bcix,self.sharedrs[ishr].value)
          cf.write(cmd)
    else:
      writeit=0   # update hw only if at least 1 changed
      cmd="setShared("
      for ishr in range(Ctpconfig.lastshrgrp1+1):
        if self.sharedrs[ishr].modified(): writeit=1
        cmd= "%s0x%x,"%(cmd, self.sharedrs[ishr].getbinval())
        self.sharedrs[ishr].hwwritten(1)
      cmd=cmd[:-1]+")"
      if writeit==1: vbexec.get1(cmd)
      writeit=0   # INT1, INT2, All/Rare
      for ishr in range(Ctpconfig.lastshrgrp1+1,Ctpconfig.lastshrgrp1+4):
        if self.sharedrs[ishr].modified(): writeit=1
        self.sharedrs[ishr].hwwritten(1)
      cmd="setShared2(0x%x,0x%x)"%(intselw,allrare)
      if writeit==1: vbexec.get1(cmd)
      if lut34:
        writeit=0   # update lut34 hw only if at least 1 changed
        cmd="setSharedL0f34()\n"+lut34
        for ishr in range(Ctpconfig.lastshrgrp1+4, Ctpconfig.lastshrgrp1+4+4+1):
          if self.sharedrs[ishr].modified(): writeit=1
          self.sharedrs[ishr].hwwritten(1)
        #print "cmd setSharedL0f34:",cmd
        if writeit==1: 
          rcstr= vbexec.getline(cmd)
      writeit=0
      for ishr in range(Ctpconfig.firstshrgrp2, Ctpconfig.lastshrgrp2):
        if self.sharedrs[ishr].modified(): writeit=1
        self.sharedrs[ishr].hwwritten(1)
      cmd="setBCmasks()\n"+self.masks2str()
      if writeit: 
        rcstr=vbexec.getline(cmd)
        #print "writeShared1:",rcstr,':'
        #rcstr2=vbexec.getoutput()
        #print "writeShared2:",rcstr2,':'
        #print "writeShared thread"
        #self.dooutmskthd=vbexec.cmdthread(cmd, self.dooutmsk)
        #print "writeShared thread:",self.dooutmskthd
  #def dooutmsk(self, outstr):
  #  print "dooutmsk:",outstr,":"
  #  vbexec.pwrite0(self.dooutmskthd,self.masks2str()+'\n')
  #  print "dooutmsk2:",outstr,":"
  def showShared(self):
    if self.shrtl:
      myw.RiseToplevel(self.shrtl); return
    else:
      self.shrtl= myw.NewToplevel("CTP shared resources", self.hideShared)
    self.shrtl.configure(bg=COLOR_SHARED)
    for shrres in self.sharedrs:
      shrres.show(self.shrtl)
    #vonint12fr= myw.MywFrame(self.shrtl, side=TOP, relief=FLAT)
    # show buttons activating PF circuits:
    pffr= myw.MywFrame(self.shrtl, side=TOP, relief=FLAT)
    pfbuts=[]
    for pf in self.pfbs:
      pfbuts.append(["PF-L%1d"%pf.level, pf.show]) 
    myw.MywHButtons(pffr, pfbuts)   
    #
    pfwcfr= myw.MywFrame(pffr, side=TOP, relief=FLAT)
    for pfwc in self.pfwcs:
      pfwc.show(pfwcfr)
    pfwcfr.bind("<Destroy>", self.hidePFwholecircuit)
  def modint12(self, bitstoshift):
    if bitstoshift==5:
      intsel= self.int2b.getEntry()
      self.interactionsel= (self.interactionsel&0x01f) | (intsel<<5)
    elif bitstoshift==0:
      intsel= self.int1b.getEntry()
      self.interactionsel= (self.interactionsel&0x3e0) | intsel
    else:
      IntErr("bad parameter in Ctpconfig.modint12:"+str(bitstoshift))
  def hidePFwholecircuit(self, ev=None):
    for pfwc in self.pfwcs:
      pfwc.hidePFwholecircuit()
  def hideShared(self,event):
    #print "hideShared:"
    self.shrtl=None
  def showFOs(self):
    #self.hideds=0
    if self.fotl:
      myw.RiseToplevel(self.fotl); return
    else:
      self.fotl= myw.NewToplevel("CTP fanout boards", self.hideFOs)
    self.fotl.configure(bg=COLOR_FOS)
    allfosfr= myw.MywFrame(self.fotl, side=TOP, bg=COLOR_FOS)
    for fanout in self.fanouts:
      fofr= myw.MywFrame(allfosfr, side=LEFT, bg=COLOR_FOS)
      fanout.show(fofr)
    busyfr= myw.MywFrame(self.fotl, side=TOP, bg=COLOR_BUSY)
    self.busyboard.showClusters(busyfr)
  def hideFOs(self, event):
    #  self.hideds=self.hideds+1
    #print "hideFOs:" #,self.hideds
    self.fotl= None
    if self.busyboard.fr:
      self.busyboard.hideClusters()
    #print "hideFOs -BusyBoard not shown. event",event
  def findFO(self,fonumber):
    for fo in self.fanouts:
      if fonumber==fo.fonumber: return fo
    return None
  def findPFC(self,pfcirc):
    for pfb in self.pfbs:
      if pfcirc[0]==pfb.level:
        for pfc in pfb.pfcs:
          if pfcirc[1]==pfc.pfnumber[1]: return pfc
    return None
  def cmdallenabled(self, setv=None):
    if setv==None:
      self.allorenabled= 1-self.allorenabled
    else:
      if self.allorenabled==setv: return
      self.allorenabled= setv
    a=""" not necessary (done in doCanvas)
    if self.allorenabled==1:
      aetx=Klas.txenabled
    else:
      aetx=Klas.txall
    self.cmdbuts.buttons[0].setLabel(aetx)
    """
    self.canvas.destroy(); self.doCanvas()
  def masks2str(self):
    s=''
    if Gl0AB==None:   #firmAC
      for v in self.bcmasks:
        s= s+"%3.3x"%v
    else:
      for v in self.bcmasks:
        s= s+"%x"%v
    return s
  def str2masks(self,longs):
    if Gl0AB==None:   #firmAC
      llongs= ORBITLENGTH*3
      z13=3
    else:
      llongs= ORBITLENGTH
      z13=1
    if len(longs) != llongs:
      IntErr("short string (%d< [3*]3564) from getBCmasks()"%len(longs))
    else:
      for ix in range(len(self.bcmasks)):
        try:
          ix3= ix*z13
          self.bcmasks[ix]= eval('0x'+longs[ix3:(ix3+z13)])
        except:
          print "except str2masks:", ix, longs[ix],longs
    for ishr in range(Ctpconfig.firstshrgrp2,Ctpconfig.lastshrgrp2):
      self.sharedrs[ishr].setattrfo('bitmap', 0)
      #von self.sharedrs[ishr].hwwritten(0)
  def str2L0f34(self,lut41):
    l4= 2**Ctpconfig.dbgbits
    if len(lut41) != l4:
      IntErr("str2L0f34: short string (%d<%d)"%(len(lut41), l4))
    lutc= [0,0,0,0] ; l0f34xis= [0,0,0,0]  ; lut=["0x","0x","0x","0x"]
    for xis in range(0, 2**Ctpconfig.dbgbits, 4):    
      if xis>=len(lut41):   # let's use 0 if short file
        for lix in range(4):
          lutc[lix]= 0
      else:
        for lix in range(4):
          l0f34xis[lix]= eval("0x"+lut41[xis+lix])
        for lix in range(4):
          lutc[lix]= \
          (((l0f34xis[+0]>>lix)&0x1)<<3) | (((l0f34xis[+1]>>lix)&0x1)<<2) | \
          (((l0f34xis[+2]>>lix)&0x1)<<1) | (((l0f34xis[+3]>>lix)&0x1)<<0) 
          #printf(" lutc[%d]:0x%x", lix, lutc[lix]);
      for lix in range(4):
        lut[lix]= lut[lix] + "%x"%lutc[lix]
    longs=[] ; lut34="" #; lut4=[]
    for lix in range(4):
      ishr= Ctpconfig.grp3start+lix
      #self.sharedrs[ishr].updatevals((lut[lix], lut[lix]))   #("a&b", hexa) 
      self.sharedrs[ishr].setattr(eval(lut[lix]))   #(binary value) 

class TrgSHR:
  """Shared resource properties:
  - probably single VME register for BC1/2,RND1/2
  - string for BCM1-4 (12), L0func1, L0func2
  """
  BCxHelp="""Scaled-down BC. 
Class L0 trigger condition is: L0input AND BCx AND RNDx.
BCx (x=1..2) and RNDx (x=1..2) is applicable for any Class
The number entered here represents number of BCs between two
pulses generated by BC-downscaled generator
0:         -always 1 (i.e. pulse in each BC)
0x7fffffff -pulses are not generated
"""
  RNDxHelp="""Random trigger. Value in this field:
The most significant bit (bit31) meaning:
1: the filter is applied to the random pattern removing neighbouring
   pulses (i.e. there wont be 2 pulses in 2 consecutive BCs)
0: filter is not applied

Note1:
Class L0 trigger condition is: L0input AND BCx AND RNDx.
BCx (x=1..2) and RNDx (x=1..2) are applicable for any Class.
Note2:
Both random triggers start with the same initial conditions, which means, even
if they programmed for different rate, the pattern of the slower one is
subset of another. 
To synchronise/resynchronise them use SimpleTests/RNDsync button.

Following special syntax can be used for this field:
bN  where N is the average number of BC between 2 triggers,
Examples:
b0   -always 1 (in each BC)
b3   -in average, 3 BC between 2 triggers

"""
  BCMxHelp="""BC mask. 
There are 4 (12) BC mask applicable as 4 (12) vetos for any Class.
If more BCmasks are enabled, the resulting BCmask is
logical OR of all of them.
One BC mask contains 3564 bits.
BCMx[bcnumber] (bcnumber=1..3564) bit set to 1 forbides possible
trigger in that BC.
"bitmap" written in BC mask entry field denotes \"bit map definition\" -it 
can be viewed by pressing Mask name (BCM1-12) button.
"bitmap" is result of reading the mask from CTP registers.

Otherwise, the BCmask entry field contains \"PATTERN definition\" 
with the following syntax:
L or l    -means '0' shown green (i.e. don't care)
H or h    -means '1' shown red (i.e. BC is masked = signal is vetoed)
()        -can be used for grouping repeated patterns
Examples:
2l4h      means 001111
2h 3(lhl) means 11010010010
1000L H 2562L   means masking just BC number 1001
The bitmap of resulting pattern can be seen by pressing "BCM*" button -
the fields of this "BCmask pattern" window are not updated -i.e., if
pattern definition is changed, the window should be closed/opened
to see the actual BCmask pattern.
"""
  L0FUNxHelp="""INTfun1, INTfun2, INTfunT (interaction functions) and
L0fun1, L0fun2 (L0 functions) are programmable functions of the first 
four CTP L0 inputs. 
These functions are defined by 16 bits Lookup table (LUT).
"""
  L0FUN34Help="""L0fun3, L0fun4 are programmable functions of all 24 L0 inputs.
It consists of OR of any logical combination of first L0 inputs (1-12)
and last L0 inputs (13-24):

L0fun3= l0f31 | l0f32
L0fun4= l0f41 | l0f42

l0fx1: (x:3 or 4) logical expression made from L0 inputs 1..12
l0fx2: (x:3 or 4) logical expression made from L0 inputs 13..24
"""
  INTxHelp="""Interaction selector
"""
  AllRareHelp="""
All/Rare: All        -take all events
All/Rare:            -kill classes marked by green All/Rare flag 
"""
class Klas(Genhw):
  txenabled="Show only enabled classes"
  txall="Show all classes"
  BCmask_mincabi=4
  if Gl0AB==None:
    l0allinputs=32   # numb. of valid bits in L0_CONDITION_n word (l0AC...)
    l0allvetos=18
    BCmask_maxcabi=15
    AR_cabi=16
    #iinvetos[i] -> position of bit in self.l0vetos
    #         4567 PF14, 8..11 BCmask1..4, 12:All/Rare 16:Classmask
    # firmAC: 4567 PF14, 8..19 BCmask1..12, 20:All/Rare 31:Classmask
    iinvetos= range(4,21)+[31]   # bit numbers in self.vetos
    MININVL0_45= 1
  else:
    l0allinputs=30   # numb. of valid bits in L0_CONDITION_n word for old versions
    l0allvetos=10    # numb. of L0 vetos in L0_VETO_n word
    BCmask_maxcabi=7
    AR_cabi=8
    iinvetos= range(4,13)+[16]   # bit numbers in self.vetos
    MININVL0_45= 45
  # L1:PF1-4, RoIflag,L2:PF1-4
  l12allvetos=[100,101,102,103,104,200,201,202,203]   
  l0inputs=24
  l0vetosx0=myw.Kanvas.bitWidth*l0allinputs + Ctpconfig.l0x0 + myw.Kanvas.interspace
  # 15 -2 vetos overwritten
  l0scalerx0= l0vetosx0+ myw.Kanvas.bitWidth*l0allvetos + myw.Kanvas.interspace
  l1inputsx0=  l0scalerx0+90   # 82
  l1vetosx0=  l1inputsx0 + myw.Kanvas.bitWidth*24 + myw.Kanvas.interspace
  l2inputsx0=  l1vetosx0 + myw.Kanvas.bitWidth*5 + myw.Kanvas.interspace
  l2vetosx0=  l2inputsx0 + myw.Kanvas.bitWidth*12 + myw.Kanvas.interspace
  colDontCare='#ff0000'    # red
  colValid='#009900'       #dark green
  colValidInv='#00ff66'
  #colCluster=("white",'#cc00ff','#ffff00','#ccffcc','#ccff00',
  #  '#66ffff','#6600ff')
  colCluster=('black','brown','red','orange','yellow','green','blue')
  def __init__(self, ctpcfg, number, inputs=0, vetos=0, scaler=0):
    Genhw.__init__(self)
    self.clnumber=number      # hw. number of the class (1-NCLASS)
    self.linenumber= 0        # line number on Canvas (1...) 0-not displayed
    self.ctpcfg=ctpcfg
    self.l0inputs=inputs
    self.l0inverted=0x2f
    self.l0vetos=vetos        # bit16:classmask
    self.l1definition=0
    self.l1inverted=0
    self.l2definition=0
    self.scaler=0
    self.clusbitid=None       # cluster bit id (canvas item)
  def readhw(self, line=None):
    if line:
      c5txt= string.split(line)
      self.hwwritten(0)
    else:
      c5txt= vbexec.get1("getClass("+str(self.clnumber)+")")
      self.hwwritten(1)
    #print "c5:",c5, self.clnumber
    c5= map(eval, c5txt)
    self.l0inputs= c5[0]    #30 bits (32 for firmAC)
    self.l0inverted= c5[1]  #24 bits
    self.l0vetos= c5[2]     #16 bits. bit16:CLassMask,bit0-12->see hw
    self.scaler= c5[3]
    if len(c5)<=4:   #old format -before L1.L2
      vbexec.printmsg("class%d:: L1,L2 definitions missing, taking defaults\n"%self.clnumber)
      self.l1definition= 0xfffffff
      self.l1inverted= 0
      self.l2definition= 0xf000fff
    else:
      self.l1definition= c5[4]
      self.l1inverted= c5[5]
      self.l2definition= c5[6]
  def writehw(self,cf=None):
    if cf:
      fmt="CLA.%03d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n"
      cmdout= cf.write
    else:
      fmt="setClass(%d,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x)"
      cmdout= vbexec.get2
      self.hwwritten(1)
    cmd=fmt%(self.clnumber, self.l0inputs,self.l0inverted, 
      self.l0vetos,self.scaler,self.l1definition,self.l1inverted,
      self.l2definition)
    #vbexec.get2(cmd)
    cmdout(cmd)
  def doClass(self):
    if self.linenumber!=0:
      IntErr("doClass:linenumber:%d !=0"%self.linenumber)
    self.linenumber= self.ctpcfg.freenumber   # line number on Canvas
    self.ctpcfg.freenumber= self.ctpcfg.freenumber+1
    #self.canvas.create_rectangle(0,0,200,10, fill='blue')
    self.ctpcfg.canvas.create_text(1, self.linenumber*myw.Kanvas.bitHeight
      - myw.Kanvas.bitBorder + Ctpconfig.l0y0, anchor=NW,
      text=str(self.clnumber))
    self.doScaler()
    self.doInputs(); self.doVetos();  # calls doCluster
  #def hideClass(self):
  #  if self.linenumber==0:
  #    IntErr("hideClass: linenumber %d!=0"%linenumber)   
  def refreshClass(self):
    for i in range(Klas.l0allinputs)+range(100,124)+range(200,212):
      self.refreshL0bit(i)
    for i in range(Klas.l0allvetos)+Klas.l12allvetos:
      self.refreshVetobit(i)
    self.refreshScaler()
    self.refreshCluster()
  def getCluster(self):
    rc= self.l0vetos&7
    if rc>6: 
      IntErr(" L0_VETO.ClusterCode >6 for class "+str(self.clnumber))
      rc=0
    #print "getCluster:",self.clnumber,self.linenumber,rc
    return rc
  def setCluster(self,cluster):
    self.l0vetos= self.l0vetos&0xfffffff8 | cluster&7
    self.l1definition= self.l1definition&0x8fffffff | ((cluster&7)<<28)
    self.l2definition= self.l2definition&0x8fffffff | ((cluster&7)<<28)
    self.refreshCluster()
    self.hwwritten(0)
  def refreshCluster(self):
    cluster=self.getCluster()
    if self.clusbitid:
      self.ctpcfg.canvas.itemconfigure(self.clusbitid,
        fill=Klas.colCluster[cluster])
    if cluster>0:
      self.clushlp="Cluster "+str(cluster)
    else:
      self.clushlp="Cluster not assigned"
  def doCluster(self):
    xy= (Ctpconfig.clusterx0, \
      self.linenumber*myw.Kanvas.bitHeight - myw.Kanvas.bitBorder + Ctpconfig.l0y0)
    cluster= self.getCluster()
    #color= Klas.colCluster[cluster]
    #self.clusbitid=self.ctpcfg.canvas.dobit(xy, Klas.colCluster[cluster], "XX"+str(self.clnumber),dynhelp=self.updtClusterHelp)
    self.clusbitid=self.ctpcfg.canvas.dobit(xy, Klas.colCluster[cluster])
    self.ctpcfg.canvas.doHelp(self.clusbitid, "XX"+str(self.clnumber),dynhelp=self.updtClusterHelp)
    handler= lambda e,s=self,k=cluster:s.clusterhandler(e, k)
    self.ctpcfg.canvas.tag_bind(self.clusbitid, "<Button-1>", handler)
    self.refreshCluster()
  def updtClusterHelp(self, htext):
    #print "updtClusterHelp:",htext
    k= self.ctpcfg.findKlass(int(htext[2:]))
    htext= "Cluster "+str(k.getCluster())
    return htext
  def clusterhandler(self,event, clustern):
    #print "clusterhandler: class# cluster",self.clnumber,clustern,self.linenumber
    xy= [Ctpconfig.l0x0-myw.Kanvas.bitWidth/2,
         Ctpconfig.l0y0+ myw.Kanvas.bitHeight*self.linenumber]
    #self.clentry= self.ctpcfg.doEntry(xy, self.modCluster)
    self.newclss=[]
    for newcl in range(7):
      bitid=self.ctpcfg.canvas.dobit(xy, Klas.colCluster[newcl], 
        "New cluster:"+str(newcl))
      xy[0]= xy[0]+ myw.Kanvas.bitWidth
      handler= lambda e,s=self,k=newcl:s.clha2(e, k)
      self.ctpcfg.canvas.tag_bind(bitid, "<Button-1>", handler)
      #self.ctpcfg.canvas.tag_bind(bitid, "<Leave>", handler,add='+')
      self.newclss.append(bitid)
  def clha2(self, event, newclust):
    #print "clha2:",newclust,dir(event)
    #print "clha2:",event.keysym_num,event.keycode,event.num,event.type
    # 0 0 0 8 <Leave>
    # 1 1 1 4 <Button-1>
    for bitid in self.newclss:
      self.ctpcfg.canvas.delete(bitid)
    if newclust != self.getCluster():
      self.setCluster(newclust)
  def doInputs(self, inps=None):
    """todo: doL0inpu1()
    """
    if inps: self.l0inputs=inps
    self.inpbitids={}
    for cabi in range(Klas.l0allinputs)+range(100,124)+range(200,212):
      if cabi>=200:
        startx0= Klas.l2inputsx0; i=cabi-200
        hlptext="L2 input "+str(i+1)
        minInverted= 1 #45
      elif cabi>=100:
        startx0= Klas.l1inputsx0; i=cabi-100
        hlptext="L1 input "+str(i+1)
        minInverted= 1 #45
      else:
        startx0= Ctpconfig.l0x0; i=cabi
        minInverted= Klas.MININVL0_45   # <AC or >=AC 
      xy= (i*myw.Kanvas.bitWidth - myw.Kanvas.bitBorder + startx0, \
        self.linenumber*myw.Kanvas.bitHeight - myw.Kanvas.bitBorder + Ctpconfig.l0y0)
      # 24,25: random,  26,27:Scaled-down BC, 28,29 L0function
      if cabi<24:
        hlptext="L0 input "+str(cabi+1)
      elif cabi==24:
        hlptext="L0 function 1"
      elif cabi==25:
        hlptext="L0 function 2"
      elif cabi==26:
        hlptext="L0 random 1"
      elif cabi==27:
        hlptext="L0 random 2"
      elif cabi==28:
        hlptext="BC scaled-down 1"
      elif cabi==29:
        hlptext="BC scaled-down 2"
      verACl0input= False
      if self.ctpcfg.l0AB==None:    #firmAC
        if cabi<24: verACl0input= True   # any L0 input for firmAC
        if cabi==26:
          hlptext="L0 function 3"
        elif cabi==27:
          hlptext="L0 function 4"
        elif cabi==28:
          hlptext="L0 random 1"
        elif cabi==29:
          hlptext="L0 random 2"
        elif cabi==30:
          hlptext="BC scaled-down 1"
        elif cabi==31:
          hlptext="BC scaled-down 2"
      bitid= self.ctpcfg.canvas.dobit(xy,helptext=hlptext)
      self.inpbitids[cabi]=bitid
      handler1= lambda e,k=cabi:self.modL0handler(e, 1, k)
      handler2= lambda e,k=cabi:self.modL0handler(e, 2, k)
      self.ctpcfg.canvas.tag_bind(bitid, "<Button-1>", handler1)
      # L1,2 different 
      if (self.clnumber>=minInverted and i<Klas.l0inputs) or verACl0input:   
        self.ctpcfg.canvas.tag_bind(bitid, "<Button-2>", handler2)
        #print "dobit %d:inverted"%(cabi)
      self.refreshL0bit(cabi)
    xy1= (24*myw.Kanvas.bitWidth - myw.Kanvas.bitBorder + Ctpconfig.l0x0, \
      self.linenumber*myw.Kanvas.bitHeight + myw.Kanvas.bitBorder + Ctpconfig.l0y0)
    xy2= (xy1[0], xy1[1]+myw.Kanvas.bitHeight - 3*myw.Kanvas.bitBorder)
    self.ctpcfg.doline(xy1, xy2, "white", 2*myw.Kanvas.bitBorder)
  def refreshL0bit(self,ibit):
    # ibit:0-29 even more with AC version, 100-123, 200-211
    bitnx= self.canv2hwinp(ibit)
    bitn= 1<<bitnx
    if ibit>=200:
      inputs= self.l2definition
      inverted= self.l2definition>>12
      minInverted= 1 #45
    elif ibit>=100:
      inputs= self.l1definition
      inverted= self.l1inverted
      minInverted= 1 #45
    else:
      inputs= self.l0inputs
      inverted= self.l0inverted
      minInverted= Klas.MININVL0_45   # <AC or >=AC 
    if inputs & bitn==0:
      # for L1,2 only from 45 with new version (AC):
      if (self.clnumber>=minInverted and bitnx<Klas.l0inputs):
        if inverted & bitn:
          color= Klas.colValidInv;
        else:
          color= Klas.colValid;
      else:
        color= Klas.colValid;
    else:
      color= Klas.colDontCare
    self.ctpcfg.canvas.itemconfigure(self.inpbitids[ibit],fill=color)
  def testhandler(self,event, klasbit):
    #print "testhandler. event x y:",event.x, event.y
    print "testhandler. event:",event
    print "testhandler. class# klasbit:",self.linenumber,klasbit
    #print self.ctpcfg.canvas,'\n',event.widget
  def modL0handler(self,event, eventkeycode, klasbit):
    """klasbit: 0..
    """
    #print "modL0handler1:", klasbit,hex(self.l0inputs), eventkeycode
    #print "modL0handler2:", event, dir(event)
    #print "modL0handler3:", event.keycode, event.keysym,event.keysym_num, eventkeycode
    #print "modL0handler4:", type(event.keycode)
    bit= self.canv2hwinp(klasbit)
    if klasbit>=200:
      if eventkeycode==1:
        self.l2definition= InvertBit(self.l2definition, bit )
      if eventkeycode==2:
        self.l2definition= InvertBit(self.l2definition, bit+12 )
    elif klasbit>=100:
      if eventkeycode==1:
        self.l1definition= InvertBit(self.l1definition, bit )
      if eventkeycode==2:
        self.l1inverted= InvertBit(self.l1inverted, bit )
    else:
      if eventkeycode==1:
        self.l0inputs= InvertBit(self.l0inputs, bit )
      if eventkeycode==2:
        self.l0inverted= InvertBit(self.l0inverted, bit )
    #print "modL0handlerX:", hex(self.l0inputs)
    self.refreshL0bit(klasbit)
    self.hwwritten(0)
  def doVeto1(self, cabi):
    #cabi: 0..9 100..104
    #print "doVeto1:",cabi
    if cabi>=200:
      startpix= Klas.l2vetosx0
      cabirel= cabi-200
    elif cabi>=100:
      startpix= Klas.l1vetosx0
      cabirel= cabi-100
    else:
      startpix= Klas.l0vetosx0
      cabirel= cabi
    i= self.canv2hwveto(cabi)
    xy= (cabirel*myw.Kanvas.bitWidth - myw.Kanvas.bitBorder + startpix, \
      self.linenumber*myw.Kanvas.bitHeight - myw.Kanvas.bitBorder + Ctpconfig.l0y0)
    if cabi>=0 and cabi<=3:
      hlptext="P/F "+str(cabi+1)
    elif cabi>=self.BCmask_mincabi and cabi<=self.BCmask_maxcabi:
      hlptext="BCmask "+str(cabi-3)
    elif cabi==self.AR_cabi:
      #if self.l0vetos & (1<<self.iinvetos[cabi]):
      hlptext="""All/Rare veto. 
red:1   -'rare' class (always taken)
green:0 -killed if AllRare is Rare"""
      #else:
      #  hlptext="All/Rare now Rare"
    elif cabi==(self.AR_cabi+1):
      hlptext="Class Mask"
    elif cabi>=200 and cabi<=203:
      hlptext="P/F "+str(cabi-199)
    elif cabi>=100 and cabi<=103:
      hlptext="P/F "+str(cabi-99)
    elif cabi==104:
      hlptext="RoI flag"
    else:
      IntErr("doVeto1")
    bitid= self.ctpcfg.canvas.dobit(xy, helptext=hlptext)
    handler= lambda e,s=self,k=cabi:s.modVetohandler(e, k)
    self.ctpcfg.canvas.tag_bind(bitid, "<Button-1>", handler)
    self.vetobitids[cabi]=bitid
    self.refreshVetobit(cabi)
  def refreshVetobit(self, ibit):
    vetobit= self.canv2hwveto(ibit) 
    if ibit>=200:
      vetoword= self.l2definition
    elif ibit>=100:
      vetoword= self.l1definition
    else:
      vetoword= self.l0vetos
    if vetoword & (1<<vetobit):
      color=Klas.colDontCare
    else:
      color=Klas.colValid
    #print "refreshVetobit: canvasbit:",ibit," vetobit:", vetobit
    self.ctpcfg.canvas.itemconfigure(self.vetobitids[ibit],fill=color)
  def doVetos(self, vetos=None):
    if vetos:self.l0vetos=vetos
    self.doCluster()   # let's keep it with vetos (1 common vme word)
    self.vetobitids={}
    #print "doVetos:",Klas.l0allvetos
    for ibitcanvas in range(Klas.l0allvetos)+Klas.l12allvetos:
      self.doVeto1(ibitcanvas)
  def modVetohandler(self,event,canvbit):
    #if klasbit==9:   #Class mask
    #print "modVetohandler:",event,canvbit, event.keycode
    # VME L0_MASK word will be updated according to self.l0vetos[31] bit
    if canvbit>=200:
      self.l2definition=InvertBit(self.l2definition, self.canv2hwveto(canvbit) )
    elif canvbit>=100:
      self.l1definition=InvertBit(self.l1definition, self.canv2hwveto(canvbit) )
    else:
      self.l0vetos= InvertBit(self.l0vetos, self.canv2hwveto(canvbit) )
    self.refreshVetobit(canvbit)
    self.hwwritten(0)
  def canv2hwveto(self,canvbit):
    if canvbit>=200:
      bit= canvbit-200 + 24
    elif canvbit==104:
      bit=31
    elif canvbit>=100:
      bit= canvbit-100+24
    else:
      bit=Klas.iinvetos[canvbit]
    return bit
  def canv2hwinp(self,canvbit):
    if canvbit>=200:
      bit= canvbit-200
    elif canvbit>=100:
      bit= canvbit-100
    else:
      bit=canvbit
    return bit
  def doScaler(self):
    xy= (Klas.l0scalerx0, \
      #self.linenumber*myw.Kanvas.bitHeight)
      self.linenumber*myw.Kanvas.bitHeight - myw.Kanvas.bitBorder + Ctpconfig.l0y0)
    #self.scalentry= self.ctpcfg.doEntry(xy, self.modScaler)
    self.scalentry= self.ctpcfg.canvas.doEntry(xy, self.modScaler)
    self.refreshScaler()
  def refreshScaler(self):
    percentrate=str(self.scaler)
    #if self.scaler==0: percentrate="%100"
    #percentrate= "%%%5.2f"%(100-self.scaler*100./0x1fffff)
    self.scalentry.setEntry(percentrate)
  def modScaler(self,event):
    ntv=self.scalentry.getEntry(); ratemsg=None
    try:
      if ntv[0]=='%':   # 21bits, 0->max. rate, 1fffff->min rate
        pr= float(ntv[1:])
        newscaler= int(round((100-pr)*0x1fffff/100))
        ratemsg="%s -> %d(0x%x) rate for class %d\n"%\
          (ntv, newscaler,newscaler,self.clnumber)
      else:
        newscaler=self.scalentry.getEntryBin()
    except:
      newscaler=0
      ratemsg="Error:%s -bad rate for class %d, 0 used instead\n"%\
        (ntv, self.clnumber)
    #print "modScaler:", self.scaler  #, dir(event)
    if newscaler!= self.scaler:
      self.scaler= newscaler
      self.refreshScaler()
      self.hwwritten(0)
      if ratemsg: vbexec.printmsg(ratemsg)

class FanoutCon:
  def __init__(self,fo,connector, clusters=0, roc=0, detname=''):
    self.fo= fo                # Fanout instance it belongs to
    self.connector=connector   # 1..4
    # Toggle bit + clusters it belongs to (bits:XT654321)
    self.clusters=clusters     
    self.roc= roc                # RoC bits 4..1
    if detname=='':
      self.detname="ltu"+ str((self.fo.fonumber-1)*4 + connector)
    else:
      self.detname=detname
    #print "detname:", self.detname
    self.mb=None               # not shown
  def showrefresh(self, fr=None):
    """ Show connector, or refresh the hw and widget (fr==None)
    fr==None: we are called from readhw 
              - refresh widget if self.mb!=None
    fr!=None: display new widget only
    """
    if fr !=None:
      if self.mb:
        print "FanoutCon.show: self.mb already exists, but fr!=None"
        return
      else:         # refresh
        # refresh hw part 
        pass
    if self.mb:   #refresh
      self.mb.setEntry(self.clusters)
      self.rocw.setEntry(self.roc)
    else:
      if fr:
        connfr= myw.MywFrame(fr, relief=FLAT)
        self.mb= myw.MywBits(connfr, side=LEFT, relief=FLAT,
          defval= self.clusters, label=self.detname,
          cmd= self.modclusters, bits=["1","2","3","4","5","6","T","X"],
          helptext="""Clusters this fanout connector belongs to:
- 1,2,3,4,5,6 and T-test cluster and 
- TOGGLE option (X), if this option is choosen, the L1 data signal
  of this connector will toggle
  The toggle option is valid only with 'real_output' enabled -i.e. 
  SSMEnable[0] (SSM Output flag bit) is 0.

Assigning ltuN to ClusterX involves FO and BUSY boards programming -
i.e. corresponding FO_CLUSTER/FO_TESTCLUSTER (on FO board)
and BUSY_CLUSTER (on BUSY board) VME registers are modified.

The entry field on the right, the 4 RoC bits transmitted in L1 header, 
can be set as number 0-15 (or hexadecimaly 0x0-0xf).
""")
        self.rocw= myw.MywEntry(connfr,label='',
          bind='lr', defvalue=str(self.roc), relief=SUNKEN,
          width=3, side=RIGHT, cmdlabel= self.modroc)
  def modclusters(self):
    newclus= self.mb.getEntry()
    #print "FanoutCon:",hex(newclus)
    if newclus!=self.clusters:
      self.fo.hwwritten(0)
      #diff= self.clusters ^ newclus   (there is FO-BUSY button for this)
      #clstr=8
      #if diff &0x1: clstr=1
      #if diff &0x2: clstr=2
      #if diff &0x4: clstr=3
      #if diff &0x8: clstr=4
      #if diff &0x10: clstr=5
      #if diff &0x20: clstr=6
      #if diff &0x40: clstr=0
      self.clusters= newclus
      #if clstr != 8:   # not X
      #  detn= (self.fo.fonumber-1)*4 + self.connector
      #  self.fo.ctp.busyboard.refreshClusters(detn,clstr)
  def modroc(self,ev):
    newroc= self.rocw.getEntry()
    #print "FanoutCon.modroc:",newroc
    try:
      enewroc= eval(newroc)
    except:
      self.rocw.setEntry(str(self.roc))
    else:
      if enewroc!=self.roc:
        self.fo.hwwritten(0)
        self.roc= enewroc
        #detn= (self.fo.fonumber-1)*4 + self.connector
        #self.fo.ctp.busyboard.refreshClusters(detn,clstr)

class Fanout(Genhw):
  def __init__(self, ctp, fonumber, ctc=None):
    """ fonumber: 1..6   ctc="0xcluster 0xtestcluster"
    """
    self.ctp=ctp
    self.fowidget=None   # not shown
    self.fonumber= fonumber   # 1..6 (+4 to get position in ctpboards)
    self.cons=[]              # list of 4 connectors
    self.calflag=0
    for ix in range(4):
      detname= self.ctp.ltus.getLTUname(self.fonumber, ix+1)
      self.cons.append(FanoutCon(self, ix+1, detname=detname))
    Genhw.__init__(self)
    self.readhw(ctc)
  def readhw(self,line=None):
    """Set Fanout attributes according to ctc. If ctc==None, read HW
    refresh displayed widget, if any
    """
    if line:
      ctctx= string.split(line)
      self.hwwritten(0)
    else:
      ctctx= vbexec.get1("getFO("+str(self.fonumber+4)+")")   # cct: [cl, tcl]
      self.hwwritten(1)
    ctc= map(eval, ctctx)
    self.calflag= (ctc[1]&0x100000)>>20      # cal. flag (0 or 1)
    if self.fowidget: self.cfw.setEntry(self.calflag)
    for ix in range(4):   #all front panel connectors
      clu= (ctc[0] & (0x3f<< (ix*8))) >>(ix*8)    # clusters
      clu= clu | (( (ctc[1]>>(ix+16)) & 1) <<6)   # test cluster
      clu= clu | (( (ctc[1]>>(ix+28)) & 1) <<7)   # toggle
      roc= (ctc[1] >> 4*ix) & 0xf
      self.cons[ix].clusters= clu
      self.cons[ix].roc= roc
      self.cons[ix].showrefresh()
  def writehw(self, cf=None):
    """ prepare cluster and testcluster words for this FO
    and write them to the board
    """
    clu=0; tclu= 0;
    for ix in range(4):
      clu= clu | (( self.cons[ix].clusters & 0x3f) << (8*ix))
      tclu= tclu | \
        (( self.cons[ix].roc & 0xf) << (4*ix)) |\
        (( self.cons[ix].clusters>>6)&1) << (16+ix) |\
        (( self.cons[ix].clusters>>7)&1) << (28+ix) 
    tclu= tclu | (self.calflag<<20)
    if cf:
      fmt= "FO.%d 0x%x 0x%x\n"
      fonum= self.fonumber
      cmdout=cf.write
    else:
      fmt= "setFO(%d,0x%x,0x%x)"
      fonum= self.fonumber+4
      cmdout=vbexec.get1
      self.hwwritten(1)
    cmd= fmt%(fonum,clu,tclu)
    cmdout(cmd)
  def show(self, fr):
    """fr -Frame in which the Fanout attributes will be shown
    """
    if self.fowidget:
      print "Fanout.show: self.fowidget already exists"
      return
    self.fowidget= fr
    self.hideds=0
    self.fowidget.bind("<Destroy>", self.hideFO)
    self.title= myw.MywLabel(self.fowidget,label="fo"+str(self.fonumber),
      bg=COLOR_FOS,
      helptext="""Fanout widget.
For each of 4 fanout connectors, the following information is shown: 
- the clusters to which the connector is assigned (1,2,3,4,5,6,T)
- the 'toggling' option (X), if choosen for the connector
- RoC flags (4 bits)""")
    self.cfw= myw.MywBits(self.fowidget, relief=FLAT,
          defval= self.calflag, label='CalFlag',
          cmd= self.modcflag, bits=["yes"],
          helptext="""Calibration flag for this FO. This flag
should be the same for all FOs.
""")
    fr2= myw.MywFrame(self.fowidget, relief=FLAT)
    for ix in range(4):
      self.cons[ix].showrefresh(fr2)   # FanoutCon
  def modcflag(self):
    newcflag= self.cfw.getEntry()
    if newcflag!= self.calflag:
      self.hwwritten(0)
      self.calflag= newcflag
  def hideFO(self, event):
    self.hideds=self.hideds+1
    #print "hideFO:",self.fonumber,self.hideds
    for c in self.cons:
      c.mb=None   #garbage collector frees the rest
    self.fowidget= None

class BusyCluster:
  def __init__(self,bb,cluster, dets, daqbusy):
    self.cluster=cluster   #0->Test cluster or 1,2,3,4,5,6
    self.dets= dets
    self.daqbusy= daqbusy
    self.bb= bb   # parrent BusyBoard instance
  def showCluster(self, fr):
    fr1c= myw.MywFrame(fr, relief=FLAT, side=TOP)
    if self.cluster==0:
      clusnam= 'T'
    else:
      clusnam= str(self.cluster)
    self.dbb= myw.MywButton(fr1c, side=LEFT, label=clusnam,
      expand='no',
      cmd= self.daqbsycmd, helptext="""DAQBUSY button. 
This button controls the DAQBUSY bit for corresponding cluster (1-6,T).
If in red color, DAQBUSY is set -i.e. triggers for this cluster are disabled

If at least 1 cluster is in DAQBUSY state, DAQ LED on L0 board should be ON.
""")
    if self.daqbusy==1: self.dbb.setColor('red')
    self.db= myw.MywBits(fr1c, side=LEFT, label='', bits=self.bb.detConnected,
      cmd= self.detscmd, defval= self.dets, allro='yes',
      #cmd= self.detscmd, defval= self.dets,
      helptext="""CLUSTER word as programmed on BUSY board. 
You can see LTU names or numbers.
The numbers represents NOT CONNECTED busy inputs (1-24) 
belonging to this cluster -i.e. something is wrong (all busy inputs
used should be CONNECTED!).
Always when cluster is choosen for a FO output connector (upper part of
the widget), the corresponding registers on FO boards are modified, BUT
BUSY BOARD IS LEFT UNTOUCHED.

That is the reason why numbers(1-24) of this button,
corresponding to bits in CLUSTER word on BUSY board, are 'read only'.
The right value can be obtained by pressing 'FOs-->BUSY' button.
(if you want to play with BUSY board setting independently,
remove allro='yes' option for self.db button in ctpcfg.py file)
""")
  def daqbsycmd(self):
    if self.cluster==0:return   # no DAQBUSY for Test cluster
    self.bb.hwwritten(0)
    if self.daqbusy==1: 
      self.daqbusy=0 
      self.bb.daqbusys= self.bb.daqbusys & (~(1<<(self.cluster-1)))
      self.dbb.resetColor()
    else:
      self.daqbusy=1 
      self.bb.daqbusys= self.bb.daqbusys | (1<<(self.cluster-1))
      self.dbb.setColor('red')
  def detscmd(self):
    self.bb.hwwritten(0)
    self.dets= self.db.getEntry()
  def updateCluster(self, dets,daqbusy=None):
    #print "updateCluster:", hex(dets), daqbusy
    self.dets= dets
    if daqbusy!=None: self.daqbusy= daqbusy
    if self.bb.fr:
      if daqbusy!=None: 
        if self.daqbusy==1: self.dbb.setColor('red')
        else: self.dbb.resetColor()
      self.db.setEntry(self.dets)
      
class BusyBoard(Genhw):
  #det124=("1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24")
  def __init__(self, ctp):
    Genhw.__init__(self)
    self.ctp= ctp
    self.fr=None   #not shown
    self.bcs=[]
    self.detConnected=[]
    for ib in range(1,25):
      ltuname= self.ctp.ltus.getLTUnameOfBusy(ib)
      if ltuname == '': ltuname= str(ib)
      self.detConnected.append(ltuname)
    for ibc in (0,1,2,3,4,5,6):
      self.bcs.append(BusyCluster(self,ibc, 0, 0))
    self.readhw()
  def showClusters(self, fr):
    self.fr= fr
    self.f2b= myw.MywButton(self.fr, side=TOP, label="FOs --> BUSY",
      cmd= self.fos2busy, helptext="""Update BUSYs cluster words (bottom
part of the 'CTP fo boards' widget) to conform with FOs setting (upper part).
NOTES:
1. INT_TC_SET word (INT board i.e. RoC and CIT bits in L2a message) 
   is left untouched.
2. HW (i.e. BUSY board) is not programmed, unless the next File->Write2hw
   button pressed
""")
    for ibc in (0,1,2,3,4,5,6):
      self.bcs[ibc].showCluster(fr)
    #detsfr= myw.MywFrame(fr, relief=FLAT)
    #for ibc in (0,1,2,3,4,5,6):
    #  daqbusy= (self.dbs >> ibc) &1
  def hideClusters(self):
    #for ibc in (0,1,2,3,4,5,6):
    #  self.bcs[ibc].dbb.destroy()
    #  self.bcs[ibc].db.destroy()
    self.fr=None
  def refreshClusters(self, det124,clu06):
    """ invert det124 detector bit for clu06 cluster """
    detbit= 1<<(det124-1)  # 1..24 (busy input)
    olddets= self.bcs[clu06].dets
    if olddets & detbit:   # 1->0
      newdets= olddets & (~detbit)
    else:                  # 0->1
      newdets= olddets | detbit
    #print "refreshClusters:"
    self.bcs[clu06].updateCluster(newdets)
  def writehw(self, cf=None):
    if cf:
      fmt= "BUSY 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n"
      cmdout=cf.write
    else:
      fmt= "setClusters(0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x)"
      cmdout=vbexec.get1
      self.hwwritten(1)
    cmd= fmt%(self.daqbusys,self.bcs[0].dets, self.bcs[1].dets,
         self.bcs[2].dets, self.bcs[3].dets, self.bcs[4].dets,
         self.bcs[5].dets, self.bcs[6].dets)
    cmdout(cmd)
  def fos2busy(self):
    #print "fos2busy"
    for ibc in (0,1,2,3,4,5,6):
      self.bcs[ibc].updateCluster(0)
    for fo in self.ctp.fanouts:
      #print "fos2busy fo:",fo.fonumber
      for focon in fo.cons:
        detn= (fo.fonumber-1)*4 + focon.connector   # 1..24
        #print "fos2busy detn clusters:",detn, focon.clusters
        for clstr in range(0,7):
          if focon.clusters & (1<<clstr):
            if clstr==6: clstr=0   # T cluster
            else: clstr= clstr+1   # 1..6
            # detn belongs to clstr   
            bsyinp= vbexec.get2("findBUSYINP("+str(fo.fonumber)+","+\
              str(focon.connector)+")")[0]
            ibsyinp=int(bsyinp)
            if ibsyinp==0: continue   # not connected bsy input
            #print "bsyinp:", detn, ibsyinp
            #self.refreshClusters(detn, clstr)
            self.refreshClusters(ibsyinp, clstr)
            self.hwwritten(0)
    #cl06=vbexec.get1("updateBusyClusts()")
    #print "fos2busy:", cl06
  def readhw(self, line=None):
    if line:
      #print "readhwline:",line
      c8= string.split(line)
      db= c8[0]; c7=c8[1:8]
      self.hwwritten(0)
    else:
      db= vbexec.get1("getDAQbusy()")[0]
      c7= vbexec.get1("getClusters()")
      #print "c7:",c7,type(c7)
      self.hwwritten(1)
    self.daqbusys= eval(db)
    dbs= self.daqbusys<<1
    for ibc in (0,1,2,3,4,5,6):
      daqbusy= (dbs >> ibc) &1
      self.bcs[ibc].updateCluster(eval(c7[ibc]), daqbusy)

class MywLUT(myw.MywEntry):
  """ value=("LUT expression", number_of_inputs)
  """
  expnames=["a","b","c","d","e","f","g","h","i","j","k","l"]
  #expnames=["a","b","c","d","e","f"]
  def __init__(self,master, value, **kw):
    #print "mywLUT:", master, value,kw
    selfargs=(self,master)
    self.ninputs= MywLUT.expnames[0:value[1]]
    kw['helptext']= kw['helptext']+"""

Look up table can be given:
- directly as hexa or decimal number (e.g. 0xfa or 250) when input 
  field is GRAY or as
- logical function of input variables"""+str(self.ninputs)+"""
  when input field is of PINK color.

  Use following logical operators for logical function definition:
|  -OR                        &  -AND
^  -exclusive OR              ~  -negation
() -for grouping operations with higher priority
0  = 0 on output for any input combination (the same result
     can be reached by entering 0x0 in gray field
1  = 1 on output for any input combination (the same result
     can be reached by entering 0xffff in GRAY field (for LUT with 4 inputs,
     i.e. 16 possible output values. For LUT wit 12 inputs you need
     to put 2**12 bits which is 0xfff...fffL -hexadecimal constant
     with 1024 'f' characters)

Example:
a &b | (a|b)     (should result in 0xe LUT given as 4 bits)  
"""
    kw['defvalue']=value[0]
    apply(myw.MywEntry.__init__,selfargs, kw)
    #von self.entry.bind("<Button-3>",self.convertStart)
    #MywEntry.__init__(self, master, kw)
    #self.setEntry(value[0])
  def updateentry(self, event=None):
    newlut= self.getEntry()
    #print "MywLUT.updateentry: ninputs,oldLUT",self.ninputs,newlut
    # check newlut, if OK call cmdlabel, if not, set back old value
    newluthex= txtproc.log2tab(newlut, self.ninputs)
    #print "MywLUT.updateentry: newLUT",newluthex
    if newluthex:
      if self.conv2dec==1:
        # no error, LUT expression
        self.lutexp= newlut
        self.luthex= newluthex
      else:
        # no error, LUT hexadecimally
        self.lutexp= newlut    # expression is lost
        self.luthex= newluthex
      if self.cmdlabel: self.cmdlabel((self.lutexp,self.luthex))
    else:
      # error, LUT expression or hexadecimally
      PrintError("bad LUT expression:%s inputs %s, not assigned"%(newlut,str(self.ninputs)))
    self.showEntry()
  def showEntry(self):
    self.entry.delete(0, 'end')
    if self.conv2dec==1:
      self.setColor(COLOR_LUT)
      self.entry.insert(0, self.lutexp)
    else:
      self.setColor(myw.COLOR_BGDEFAULT)
      self.entry.insert(0, self.luthex)
  #def getEntry(self):
  #  return self.luthex
  def setEntry(self,luttext):
    #self.conv2dec: 0:          1:           2: no conversion
    #                  show hex    show LUT     not allowed
    # text: valid LUT-expression
    newluthex= txtproc.log2tab(luttext, self.ninputs)
    if newluthex:
      self.lutexp= luttext
      self.luthex= newluthex
    else:
      PrintError("bad LUT expression:%s inputs:%s, not assigned"%(luttext,str(self.ninputs)))
      self.lutexp= "n/a"
      self.luthex= 0
    self.conv2dec=1      # show LUT expression
    self.showEntry()
    #errorprint(self,"bad self.hec2dex:%d"%self.conv2dec)
  def convertStart(self,event=None):
    #print "lutconvertStart:"
    if self.conv2dec==0:
      self.conv2dec=1
    else:
      self.conv2dec=0
    self.showEntry()
  def updateHelp(self, newhelp):
    self.label.helptext= newhelp
class MywBCmask(myw.MywEntry):
  def updateentry(self, event=None):
    """event is None in case of activation by button..."""
    #print "MywEntry:",self.getEntry(),event
    ne= self.entry.get()
    self.cmdlabel(ne, event)
  def convertStart(self,event=None):
    print "MywBCmask.convertStart"
    pass

class Attr(Genhw):
  """Info about 'hw written' is set by:
     1. class itself (if modified from the keyboard) or 
     2. from outside (if set from outside -i.e. from program)
     3. in setattrfo(val, X) = setattr + hwwritten(X) if necessary
  """
  def __init__(self, atrname, value, helptext=None, cci=None):
    """
    value: 
      number (hexa or dec) (simple value)
      list:  of 2 values, or 
      list: ["LUT expression", number_of_inputs, 0] or
      string "3l45H"
      <long> -for big LUTS
    cci: ctpcfg instance
    """
    Genhw.__init__(self)
    self.atrname=atrname     
    #prt(self,"atrname:",atrname, "value:",value)
    self.value=value     
    self.helptext=helptext     
    self.cci=cci        # usually ctpcfg instance
    self.atrw= None     # associated widget not shown
  #  self.atrspecific()
  #def atrspecific(self):
  #  pass
  def setattr(self, value):
    #print "setattr.atrw:",self.atrw
    self.value=value     
    if self.atrw: self.atrw.setEntry(str(value))
  def setattrfo(self, value, hww):
    changed=None
    if self.value != value:
      self.setattr(value); changed=1
    self.hwwritten(hww)
    if self.atrw: 
      if hww==0: 
        if changed:
          self.setColorChanged(myw.COLOR_VALCHANGED)
        else:
          pass   # better to keep old color even if not changed
      else:
          self.setColorChanged(None)
  
  def setColorChanged(self,color):
    self.atrw.setColor(color)
  def getattr(self):
    return self.value
  def getbinval(self):
    #print "Attr.getbinval:", self.value
    return self.value
  def bindparent(self, fr):
    fr.bind("<Destroy>", self.hideAttr, add='+')
  def printAttr(self):
    print "Attr:", self.atrname,":", self.value, " of type:",type(self.value)
  def show(self, fr,side=TOP):
    self.bindparent(fr)
    self.atrw= myw.MywEntry(fr,label=self.atrname,helptext=self.helptext,
      bind='lr', defvalue=str(self.value), relief=SUNKEN,
      width=8, side=side, cmdlabel= self.modatr)
    #following should not be here (actual only if shown already)
    #von if self.writeme==0: 
    #  self.atrw.setColor(myw.COLOR_VALCHANGED)
    #else:
    #  self.atrw.setColor()
  def hideAttr(self,ev):
    #print "Attr.hideAttr"
    self.atrw=None
  def modatrcommon(self, entwidget, oldbinvalue):
    newval= entwidget.getEntryBin()
    if newval=='': newval='0'
    #print "Attr.modatrcommon:",newval,oldbinvalue
    #nbvalue= self.conv2bin(newval, oldbinvalue)
    nbvalue= newval
    if nbvalue != oldbinvalue:
      self.hwwritten(0)
      self.setColorChanged(myw.COLOR_VALCHANGED)
    return nbvalue
  def modatr(self, ev=None):
    #print "Attr.modatr1:",self.value," of typ:", type(self.value)
    ntv= self.atrw.getEntry() 
    if ntv[-1]=='z' or ntv[-1]=='s':
       self.atrw.setEntry(myw.fromms("blabla",ntv))
    ntv= self.atrw.getEntryBin() ; ntv= str(ntv)
    #print "Attr.modatr:",ntv, "of type:",type(ntv)
    nbvalue= self.modatrcommon(self.atrw, self.value)
    self.value=nbvalue
  def conv2bin(self, newval, oldval):
    newvalint=oldval
    try:
      newvalint= eval(newval)
    except:
      myw.MywError("bad value: %s, integer or 0x... expected"%newval)
      #self.atrw.setEntry(str(self.value))
    return newvalint
  #def save2file(self, cf):
  #  line= "%s %s\n"%(self.atrname, str(self.value))
  #  cf.write(line)
#class AttrRate(Attr):
#  def modatr(self, ev=None):
#    ntv= self.atrw.getEntry()
#    if ntv[0]=='%':   # 21bits, 0->max. rate, 1fffff->min rate
#      pr= float(ntv[1:])
#      rr= int(round((100-pr)*0x1fffff/100))
#      self.atrw.setEntry(hex(rr))
#    nbvalue= self.modatrcommon(self.atrw, self.value)
#    self.value=nbvalue
class AttrRndgen(Attr):
  def modatr(self, ev=None):
    ntv= self.atrw.getEntry()
    if len(ntv)>0 and ntv[0]=='b':
      rr= int(ntv[1:])+1
      rr= 0x7fffffff/rr
      self.atrw.setEntry(hex(rr))
    elif ntv[-1]=='z' or ntv[-1]=='s':
      self.atrw.setEntry(myw.frommsRandom("blabla",ntv))
    ntv= self.atrw.getEntryBin() ; ntv= str(ntv)
    nbvalue= self.modatrcommon(self.atrw, self.value)
    self.value=nbvalue
class AttrBCmask(Attr):
  x0=35
  y0=1
  def __init__(self,*fixedarg, **kw):
    fa= tuple([self]+list(fixedarg))
    #self.ctpcfg=kw['ctpcfg']
    #del kw['ctpcfg']
    apply(Attr.__init__, fa, kw)
    self.bcmbit=int(self.atrname[3:])-1   #BCM1-4 (or 12)
    self.bpw=None   # associated bit pattern widget not shown
  def show(self, fr,side=TOP):
    #print "show:",self.value
    self.bindparent(fr)
    #cmd=myw.curry(self.modint12, 0) ,helptext=
    self.atrw= MywBCmask(fr,label=self.atrname,helptext=self.helptext,
      defvalue=self.value, relief=SUNKEN,
      width=8, side=side, cmdlabel= self.modatr)
    self.atrw.entry.bind("<Leave>", self.atrw.updateentry)
    self.atrw.entry.bind("<Key-Return>", self.atrw.updateentry)
  def modatr(self, newval, ev=None):
    #nmask= newval   # or self.atrw.getEntry()
    #print "AttrBCmask.modatr:",newval,self.value,ev
    if ev==None:
      #print "AttrBCmask: label cmd"
      #vbexec.printmsg(self.cci.masks2str()+'\n')
      self.showbp()
      return
    if newval == 'bitmap': return
    if newval != self.value:
      #check correctness:
      bm=txtproc.BCmask(newval)
      errmsg= bm.setbits([self.cci.bcmasks, self.bcmbit])
      if errmsg!=None:
        vbexec.printmsg(errmsg)
        if errmsg[:4]=="Warn": errmsg=None
      if errmsg==None:
        #print "->modatr:",self.cci.bcmasks[0:9]
        self.value= newval
        self.hwwritten(0)
    #return nbvalue
  def showbp(self):
    if self.bpw:
      myw.RiseToplevel(self.bpw); return
    else:
      self.bpw= myw.NewToplevel("Bit pattern "+self.atrname, self.hidebpw,
        bg=COLOR_BPATTERN)
    canvasw= 100*myw.Kanvas.bitWidth + AttrBCmask.x0
    canvash= 36*myw.Kanvas.bitHeight
    self.canvas= myw.Kanvas(self.bpw, #self.canvasDestroyed,
      width=canvasw,height=canvash,
      background=COLOR_BPATTERN, borderwidth=1)
    for ix0 in range(ORBITLENGTH/100+1):
      id0=self.canvas.create_text(1, ix0*myw.Kanvas.bitHeight, 
        anchor=NW,text=str(ix0*100+1))
  #colDontCare='#ff0000'    # red
  #colValid='#009900'       #dark green
      for ix1 in range(0,100):
        bn= 100*ix0+ix1
        if bn >= ORBITLENGTH: break
        self.showbit(ix0, ix1, bn)
  def showbit(self, ix0, ix1, bn):
        ix0c= bn/(ORBITLENGTH/100+1)
        #print "showbitix0:", ix0c, ix0
        ix1c= bn- 100*ix0
        #print "showbitix1:", ix1c, ix1
        xy=(AttrBCmask.x0+ ix1*myw.Kanvas.bitWidth,
            AttrBCmask.y0+ ix0*myw.Kanvas.bitHeight)
        if self.cci.bcmasks[bn] & (1<<self.bcmbit):
          col= Klas.colDontCare
        else:
          col= Klas.colValid
        bitid=self.canvas.dobit(xy,col,"BC "+str(bn+1))
        handler= lambda e,s=self,bn=bn:s.modmask(e, bn)
        self.canvas.tag_bind(bitid, "<Button-1>", handler)
  def modmask(self, event, bn):
    print "modmask: %d-bits word bcmasks[%d]:0x%x"%\
      (Ctpconfig.BCMASKN, bn, self.cci.bcmasks[bn])
  def hidebpw(self, event):
    self.bpw=None
class AttrBits(Attr):
  def __init__(self,*fixedarg, **kw):
    fa= tuple([self]+list(fixedarg))
    #print "AttrBits.init:",fixedarg,fa
    #print "AttrBits.init:",kw
    self.bits=kw['bits']
    del kw['bits']
    #fixedarg= [self]+fixedarg
    apply(Attr.__init__,fa, kw)
    #self.printAttr();
  def modatr(self, ev=None):
    ntv= self.atrw.getEntry()
    #newval= self.atrw.getEntryBin()
    #print "AttrBits.modatr:",ntv,"oftype:",type(ntv), self.value, " of type:", type(self.value)
    #nbvalue= self.modatrcommon(self.atrw, self.value)
    if ntv != self.value:
      self.hwwritten(0)
    self.value=ntv
  def show(self, fr,side=TOP):
    #print "show:",self.value
    self.bindparent(fr)
    #cmd=myw.curry(self.modint12, 0) ,helptext=
    self.atrw=myw.MywBits(fr, side=TOP, defval= self.value,
      label= self.atrname, bits=self.bits,
      cmd=self.modatr ,helptext=self.helptext) 
  def setattr(self, value):
    #print "Attrbits.setattr.atrname,value,atrw:",self.atrname,hex(value),self.atrw
    self.value=value     
    if self.atrw: self.atrw.setEntry(self.value)
class Attr2(Attr):
  """ value is list"""
  def getattr(self, ix=None):
    #print "getattr:",self.value
    if ix!=None:
      return self.value[ix]
    else:
      return self.value
  def setattr(self, value):
    #print "setattr.atrw:",self.atrw
    self.value=value     
    if self.atrw: 
      self.atrw.setEntry(str(value[0]))
      self.atrw2.setEntry(str(value[1]))
  def setColorChanged(self, color):
    self.atrw.setColor(color)
    self.atrw2.setColor(color)
  def show(self, fr,side=TOP):
    #print "show:",self.value
    self.bindparent(fr)
    fratr= myw.MywFrame(fr, relief=FLAT, borderwidth=0, side=side)
    self.atrl= myw.MywLabel(fratr,label=self.atrname,expand='no',
      relief=SUNKEN,
      width=12, side=LEFT, helptext=self.helptext)
    self.atrw= myw.MywEntry(fratr,label='',expandentry='no',
      bind='lr', defvalue=str(self.value[0]), relief=SUNKEN,
      width=8, side=LEFT, cmdlabel= self.modatr)
    self.atrw2= myw.MywEntry(fratr,label='',expandentry='no',
      bind='lr', defvalue=str(self.value[1]), relief=SUNKEN,
      width=8, side=LEFT, cmdlabel= self.modatr2)
  def modatr(self, ev):
    #print "Attr.modatr:",self.value,newval
    nbvalue= self.modatrcommon(self.atrw, self.value[0])
    self.value[0]=nbvalue
  def modatr2(self, ev):
    #print "Attr.modatr2:",self.atrname,self.value,newval, type(newval)
    nbvalue= self.modatrcommon(self.atrw2, self.value[1])
    self.value[1]=nbvalue
class AttrLUT(Attr):
  def show(self, fr,side=TOP):
    #prt(self,'AttrLUT.show:',self.value)
    self.bindparent(fr)
    self.atrw= MywLUT(fr,self.value, label=self.atrname,helptext=self.helptext,
      bind='lr', relief=SUNKEN,cmdlabel= self.updatevals,
      width=8, side=side)
  def updatevals(self,new2):
    """new2 ("a&b", 0xde)   i.e. (text,hexa)
    """
    #prt(self,"updatevals:", new2)
    ixc= string.find(new2[0],'#')   # allow only short comment
    if ixc >=0: 
      val0= new2[0][:ixc+20]
    else:
      val0= new2[0]
    self.value[0]= val0
    newbin= self.conv2bin(new2[1], self.value[2])   #binary value of LUT table
    if newbin != self.value[2]:
      self.value[2]= newbin
      self.hwwritten(0)
      lenn= len(new2[1])
      if lenn<102:
        newhlptxt= val0+"= "+new2[1]
      else:
        newhlptxt= val0+"= 0x\n" ; ix=2
        while ix<=lenn:
          newhlptxt= newhlptxt + new2[1][ix:ix+100] + "\n"
          ix=ix+100
      self.atrw.updateHelp(newhlptxt)
  #def setbinval(self, nv):
  #    self.value[2]= nv
  def setattr(self, val):
    # val: if val[], then val[0] is preferred to val[3]
    #print "LUTsetattr.atrw:",self.atrname,self.atrw
    if (type(val)== types.IntType) or \
        (type(val)== types.LongType):   # added for long LUTs (firmAC)
      self.value[2]= val        # bin. value of LUT
      self.value[0]= hex(val)   # text value of LUT
      if self.atrw: self.atrw.setEntry(self.value[0])
    elif type(val)== types.StringType:
      self.value[2]= val        # bin. value of LUT
      self.value[0]= "0x"+val
    elif type(val)== types.ListType:
      if len(val)==3:   # ! val[0] has to conform with val[2] !
        self.value=val
        #print "setattr:",self.value
        self.value[2]= eval(txtproc.log2tab(self.value[0], self.value[1]))
        if self.atrw: self.atrw.setEntry(self.value[0])
      else:
        PrintError('For LUT expected e.g.: ["a&b",4,0x0f] but got:'+\
          str(type(val))+' '+str(val))
    else:
      PrintError("bad value for LUT:"+str(type(val))+' '+str(val))
  def getbinval(self):
    #print "LUTgetbinval:", self.value
    return self.value[2]   #binary value of LUT table

class PFcircuit(Genhw):
  """ 15 instances (5 per board)
  """
  L0_L1time=(6.4-0.8)*1000       # L0 - L1 time in ns
  def __init__(self, pfnumber):
    """ pfnumber: (0..2,1..5)    5 -is dedicated for Test class only  
    """
    self.pfwidget=None   # not shown
    self.pfnumber= pfnumber # (0..2,1..5)
    self.attrs=[ Attr2("TH1",[0,0],            # 0
"Threshold1:  2x 6 bits for blocks A, B"), 
      Attr2("TH2",[0,0],                       # 1
"Threshold2:  2x 6 bits for blocks A, B"),
      Attr2("DeltaT",[0,0],                    # 2
"Protection interval: 2x 8 bits"), 
      Attr2("Delay",[0,0],                     # 3 (including flag in bit11)
"""2x 12 bits Delay for blocks A,B
Left most bit (0x800) represents the NO DELAY FLAG a/b
11 right bits represent delay a/b (they have no sense for L0 board)
"""),
      Attr2("Scaled-down",[0,0],               # 4
"Clock scaled-down factor: 2x 5 bits for blocks A,B"),
      AttrLUT("P signal LUT",["0x0",3,0],                   # 5
"Output P signal look-up table: 8 bits")]
    Genhw.__init__(self, self.attrs)
    self.readhw()
  def getwidths(self):
    widths=[]
    for ix in (0,1):
      dtx= self.attrs.value[2][ix]+1
      if dtx==1: dtx=257
      n= self.attrs.value[4][ix]+1
      w= dtx*n*25/40.    # in micsecs
      widths.append(w)
    print "PFcircuit:", self.pfnumber, widths, " micsecs"
  def readhw(self,line=None):
    """Set PFcircuit attributes according to pfpc. If pfpc==None, read HW
    """
    if line==None:
      #pftx= vbexec.get1("getPF("+str(self.pfnumber[0])+","+
      # str(self.pfnumber[1])+  ")")   # + board
      #pfpc= map(eval, pftx)
      #pfpc=[1,2,3,4,5,6,7,8,9,10,("P-lut",3)]
      line= vbexec.getline("getPFc(%d,%d)"%(self.pfnumber[0]+1, 
        self.pfnumber[1]))
      hww=1 #self.hwwritten(1)
    else:
      hww=0 #self.hwwritten(0) # from file (likely different from what is in hw)
    pfpctx= string.split(line)
    #prt(self, "getPFc():",pfpctx)
    p3= map(eval, pfpctx)
    self.attrs[0].setattrfo([p3[0]&0x3f, p3[1]&0x3f], hww)
    self.attrs[1].setattrfo([(p3[0]>>6)&0x3f, (p3[1]>>6)&0x3f], hww)
    self.attrs[2].setattrfo([(p3[0]>>12)&0xff, (p3[1]>>12)&0xff], hww)
    self.attrs[3].setattrfo([(p3[0]>>20)&0xfff, (p3[1]>>20)&0xfff], hww)
    self.attrs[4].setattrfo([(p3[2]>>8)&0x1f, (p3[2]>>13)&0x1f], hww)
    self.attrs[5].setattrfo([hex(p3[2]&0xff),3,0], hww)
    #if self.pfwidget:
    #  print "PFcircuit.readhw: updating pfwidget todo ->done in setattr"
  def conv2abl(self):
    blockab=[0,0,0];   # A, B, LUT
    for ix in (0,1):
      blockab[ix]= self.attrs[0].getattr(ix)
      blockab[ix]= blockab[ix] | ( self.attrs[1].getattr(ix)<<6)
      blockab[ix]= blockab[ix] | ( self.attrs[2].getattr(ix)<<12)
      blockab[ix]= blockab[ix] | ( self.attrs[3].getattr(ix)<<20)
    blockab[2]= self.attrs[5].getbinval()
    blockab[2]= blockab[2] | (self.attrs[4].getattr(0)<<8)
    blockab[2]= blockab[2] | (self.attrs[4].getattr(1)<<13)
    return blockab
  def writehw(self, cf=None):
    """ prepare PFBLOCK_A/B and PFLUT words
    and write them to the board
    """
    blockab= self.conv2abl()
    if cf:
      fmt= "PFL.%d.%d 0x%x 0x%x 0x%x\n"
      lvl= self.pfnumber[0]
      cmdout=cf.write
    else:
      fmt= "setPFc(%d,%d,0x%x,0x%x,0x%x)"
      lvl= self.pfnumber[0]+1
      cmdout= vbexec.get1
      self.hwwritten(1)
    cmd= fmt%(lvl, self.pfnumber[1],blockab[0],blockab[1],blockab[2])
    cmdout(cmd)
  def show(self, fr):
    """fr -Frame in which the PFcircuit attributes will be shown
    """
    if self.pfwidget:
      print "PFcircuit.show: self.pfwidget already exists"
      return
    self.pfwidget= fr
    self.pfwidget.bind("<Destroy>", self.hidePF)
    self.title= myw.MywLabel(self.pfwidget, side=TOP,
      label="PF"+str(self.pfnumber)+"              A                  B       ",
      helptext="""PFcircuit widget.
1 PF circuit consists of 2 identical blocks A and B.
""")
    for ix in range(len(self.attrs)):
      self.attrs[ix].show(self.pfwidget)
  #def refresh(self):
    #for ix in range(len(self.attrs)):
    #  self.cons[ix].showrefresh(fr2)
  def hidePF(self, event):
    self.pfwidget= None
class PFwholecircuit:
  """ wholecircuit: all 3 levels, circuit: 1,2,3,4,5=T"""
  def __init__(self, circuit, ctp):
    self.ctp=ctp
    self.circuit=circuit # 1..5
    self.pfwidget=None   # not shown
    self.levels=[]   # 3 pointers to PFcircuit objects (for 3 levels)
    self.pfinbc= None 
    for ix in range(3):
      #print "PFwholecircuit:", ix, self.circuit
      self.levels.append(self.ctp.pfbs[ix].pfcs[self.circuit-1])
  def setpfinbc(self, pfinbc):
    """ set corresponding fields in PFboard objects, 
        update on the screen if necessary
    """
    print "pfinbc:", pfinbc
  def show(self, fr):
      #pfwcbuts.append(myw.MywEntry(pfwcfr, pfwc.name(), 'None')
    if self.pfwidget:
      print "PFwholecircuit%d.show: self.pfwidget already exists"%self.circuit
      return
    self.pfwidget= fr
    #von -see def showShared(
    #self.pfwidget.bind("<Destroy>", self.hidePFwholecircuit)
    if self.pfinbc:
      defval= str(self.pfinbc)
    else:
      defval='None'
    name= str(self.circuit)
    if name=='5': name='T'
    name = 'PF'+name
    self.entry= myw.MywEntry(self.pfwidget, name, defval, 
      bind='lr', cmdlabel= self.newpfinbc, side=TOP,
      helptext="""
The half of the past-future protection interval in BCs or
in microseconds if followed by 'us'. E.g.: 5us means 
the whole length of the past future prot. interval is 10 microseconds
""")
    #for ix in range(len(self.attrs)):
    #  self.attrs[ix].show(self.pfwidget)

  def hidePFwholecircuit(self, ev=None):
    #self.pfinbc.hideAttr()
    #print "hidePFwholecircuit%d"%self.circuit
    self.entry.destroyEntry()
    self.pfwidget= None
  def newpfinbc(self, newpf):
    #newpf= self.entry.getEntry()
    if newpf=='None': return
    try:
     if (len(newpf)>2) and (newpf[-2:]=='us'):
       newpfbc= int(pf[:-2])*40
     else:
       newpfbc= int(newpf)
    except:
       return
    print "newpfinbc:icircuit+1:", self.circuit+1, \
      "oldpf:",self.pfinbc,"newpf:",newpf, newpfbc
    if self.pfinbc == newpfbc: return
    #if (self.pfinbc==None) or (self.pfinbc!= newpfbc)) and newpfbc:
    # compute and modify corresponding values for all levels:
    #self.atrs[3].setattr((pfc>>12)&0xfff)
    # p3: hexadecimal numbers: PFBLOCK_A, PFBLOCK_B, PFLUT
    #.self.attrs[0].setattr([p3[0]&0x3f, p3[1]&0x3f])
    #.self.attrs[1].setattr([(p3[0]>>6)&0x3f, (p3[1]>>6)&0x3f])
    #.self.attrs[2].setattr([(p3[0]>>12)&0xff, (p3[1]>>12)&0xff])
    #.self.attrs[3].setattr([(p3[0]>>20)&0xfff, (p3[1]>>20)&0xfff])
    #.self.attrs[4].setattr([(p3[2]>>8)&0x1f, (p3[2]>>13)&0x1f])
    #.self.attrs[5].setattr([hex(p3[2]&0xff),3,0])
    v12str= vbexec.get1("printPFwc(%d)"%(newpfbc))
    if v12str[0]=='0xffffffff': 
      vbexec.printmsg("bad p/f prot interval. < 3472 expected\n")
      if self.pfinbc==None: sb='None'
      else: sb= str(self.pfinbc)
      self.entry.setEntry(sb)
      return
    self.pfinbc = newpfbc
    v12= map(eval, v12str)
    print "v12str:", v12str
    for il in (0,1,2):
      th1ab=[ v12[il*4]&0x3f, v12[il*4+1]&0x3f ]
      th2ab=[ (v12[il*4]>>6)&0x3f, (v12[il*4+1]>>6)&0x3f ]
      DeltaT=[ (v12[il*4]>>12)&0xff, (v12[il*4+1]>>12)&0xff ]
      Delay=[ (v12[il*4]>>20)&0xfff, (v12[il*4+1]>>20)&0xfff ]
      PsignalLUT= v12[il*4+2]&0xff
      ScaledDown=[ (v12[il*4+2]>>8)&0x1f, (v12[il*4+2]>>13)&0x1f ]
      #INTsignaldelay=v12[il*4+3]&0xfff
      self.levels[il].attrs[0].setattrfo(th1ab,0)   # 0-> TH1
      self.levels[il].attrs[1].setattrfo(th2ab,0)
      self.levels[il].attrs[2].setattrfo(DeltaT,0)
      self.levels[il].attrs[3].setattrfo(Delay,0)
      self.levels[il].attrs[4].setattrfo(ScaledDown,0)
      self.levels[il].attrs[5].setattrfo(PsignalLUT,0)
      #von -don ein setattrfo above. self.levels[il].attrs[0].hwwritten(0)
  def getdeltaN(self, ns):
    """
    ns: interval in nanosecs
    returns: (number_of_intervals, resolution_in_25ns_steps) 
             ->(0..255,0..31,) 
    """
    if ns==0: return (0, 0)
    resolution=1;
    while(1):
      nmax= (ns-1)/25*resolution+1   # number of 25ns intervals
      if nmax<=255: 
        if self.pfnumber[0]==0:
          if nmax*resolution> PFcircuit.L0_L1time:
            print "pf-L2 to be set for %d (>%d)"%(ns,PFcircuit.L0_L1time)
        return (nmax, resolution-1, delay)
      resolution= resolution + 1
    PrintError("Too large prot. interval %d (max. 204000ns)"%(ns))
class PFboard(Genhw):
  """ 3 instances of this for 3 boards: L0, L1, L2"""
  def __init__(self, level):
    self.level= level     # 0,1,2
    self.thispftl=None    # this level not shown
    self.pfcs=[]
    for pfonboard in (1,2,3,4,5):
      self.pfcs.append(PFcircuit((level,pfonboard)))
    self.atrs=[
      AttrLUT("INTa LUT",["0x0",2, 0],
"INTa function look-up table: 4 bits"),
      AttrLUT("INTb LUT",["0x0",2, 0],
"INTb function look-up table: 4 bits"),
      AttrLUT("Delayed INT LUT",["0x0", 2, 0],
"Delayed INT signal look-up table: 4 bits"),
      Attr("INT signal delay",0,
"Delayed INT signal delay: 12 bits. These 12 bits are always 0 for L0 board")
      ]
    Genhw.__init__(self, self.atrs)
    self.readhw()   # read the whole board
  def readhw(self,line=None):
    """Set PFboard attributes according to ctc. If ctc==None, read HW
    """
    if line:
      ctctx=line
      hww=0 #self.hwwritten(0)
    else:
      ctctx= vbexec.getline("getPF("+str(self.level+1)+")") # ctctx: 'PF_COMMON'
      hww=1 #self.hwwritten(1)
    pfc= eval(ctctx)
    self.atrs[0].setattrfo([hex(pfc&0xf), 2,0], hww)
    self.atrs[1].setattrfo([hex((pfc>>4)&0xf), 2,0], hww)
    self.atrs[2].setattrfo([hex((pfc>>8)&0xf),2,0], hww)
    self.atrs[3].setattrfo((pfc>>12)&0xfff, hww)
    #if self.thispftl:
    #  print "PFboard.readhw: to do -update widget -> done in setattr"
    #if line==None:
    #  for ix in range(5):
    #    self.pfcs[ix].readhw()
  def writehw(self, cf=None):
    """ prepare PF_COMMON word, write it and then 
    update all the 5 circuits
    """
    pfc= self.conv2pfc()
    if cf:
      fmt= "PFL.%1d 0x%x\n"
      lvl= self.level
      cmdout= cf.write
    else:
      fmt= "setPF(%d,0x%x)"
      lvl= self.level+1
      cmdout= vbexec.get1
    cmd= fmt%(lvl,pfc)
    if cf:
      cmdout(cmd)
      #for ix in range(len(self.pfcs)):
      #  self.pfcs[ix].writehw(cf)
    else:
      if self.modified():
        cmdout(cmd)
        self.hwwritten(1)
        #for ix in range(len(self.pfcs)):
        #  if self.pfcs[ix].modified():
        #    self.pfcs[ix].writehw()
  def conv2pfc(self):
    pfc= self.atrs[0].getbinval()
    pfc= pfc | (self.atrs[1].getbinval()<<4)
    pfc= pfc | (self.atrs[2].getbinval()<<8)
    pfc= pfc | (self.atrs[3].getattr()<<12)
    return pfc
  def show(self,frbrd=None):
    if frbrd==None:
      if self.thispftl:
        myw.RiseToplevel(self.thispftl); return
      else:
        self.thispftl= myw.NewToplevel("PF-L"+str(self.level), self.hidePF)
      self.thispftl.configure(bg=COLOR_PFS)
      frbrd= self.thispftl
    pfsfr= myw.MywFrame(frbrd, side=TOP,relief=FLAT, bg=COLOR_PFS)
    for ix in range(len(self.pfcs)):
      pffr= myw.MywFrame(pfsfr, side=LEFT, bg=COLOR_PFS)
      self.pfcs[ix].show(pffr)
    pfcfr= myw.MywFrame(frbrd, side=TOP, relief=FLAT,bg=COLOR_PFS)
    for ix in range(len(self.atrs)):
      self.atrs[ix].show(pfcfr,LEFT)
  def hidePF(self,ev=None):
    #print "hidePF:",ev
    self.thispftl=None
def main():
  root=Tk()

  sys.path.append(cfdir+'/'+board[0])   # to find user gui routines
  ctp= Ctpconfig()
  ctp.doClasses() 
  #ctp.save2file() 
  root.mainloop()

if __name__ == '__main__':
    main()

