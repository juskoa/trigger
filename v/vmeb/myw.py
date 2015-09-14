#!/usr/bin/python
'''
29.12.2002 -tear off menus now working (class FunStart)
           -funcs without pars are started directly 
 3.12.2003
- bubble help added for single function buttons
17.12.
vmeread modified (now a[-3])
9.1. started: new MywEntry (old is saved in myw.py.old)
13.1. Stdfuncs corrected (now correct width according to defaults)
 5.3. class MywBits 
 8.3. os.path.join used for paths creation (platform independent)
17.3. MywxMenu -if choosen option is the same as before, action
      is not invoked (nothing changed)
23.6. MywVMEEntry added
 3.10. "vmecrate nbi ..." -> don't initialise boards
27.1.2005 getCrate -adjusted so myw.py can be to be used with 'non vmecrate'
      modules
24.4. MywxMenu slightly modified (cmd added, bug 'only string not
      possible in items' fixed
17.5. NPARDESCTYPE/ID/NAME added, default for char * parameter is " "
11.9. MywEntry.getEntryBin() added
      MywBits -now possibility not to show some bits
21.9. MywHelp modified (now can be used as non inherite class)
11.11. NewToplevel() added
14.11. MywxMenu modified: now if cmd callback defined, it gets
       ix as integer (not string as before)
 1.12. MywEntry -bind="r" Return or Tab key pressed
16.12  bug fixed in MywBits (was incorret for bits=[(name,1),...
16. 1. MywMenuList -tear off implemented now
'''
import sys, os, os.path, string, types, time
from Tkinter import *

COLOR_RUNNING="#ff6666"
COLOR_VALCHANGED="#ff00ff"
COLOR_VALNORMAL="#99cc99"
COLOR_WARNING="#ff99cc"
COLOR_BGDEFAULT="#d9d9d9"
COLOR_DECIMAL="#d9d9ff"

DimLTUservers= {"tpc":0, "trg":0, "hmpid":0, "muon_trk":0, "ssd":0,
"daq":0, "tof":0, "fmd":0, "spd":0, "sdd":0, "trd":0, "phos":0, "cpv":0,
"muon_trg":0, "pmd":0, "t0":0, "v0":0, "zdc":0, "acorde":0, "emcal":0, "ad":0 }

def RiseToplevel(tlw):
    tlw.lift(); tlw.bell()
def NewToplevel(title, whenDestroyed=None, tlw=None,bg=None):
    if tlw==None: tlw=Toplevel(); 
    tlw.title(title)
    tlw.configure(bg=bg)
    tlm= getCrate()
    if tlm!=None:
      #tlw.transient(tlm.master)
      tlw.group(tlm.master)
    #tlw.transient(vbexec.vbinst.master)
    #print "NewToplevel: binding:", whenDestroyed
    tlw.bind("<Destroy>", whenDestroyed)
    return tlw
def compwidg(event, tlw):
  if event.widget == tlw:   # was str(tlw) before nov2013
    return 1
  else:
    return None

def dbgprint(o, *pars):
  #dbgclss={"MultRegs":1,"VmeBoard":1}
  #dbgclss={"MultRegs":1}
  dbgclss={}
  cnam=o.__class__.__name__
  if dbgclss.has_key(cnam):
    print o.__class__.__name__,":",pars
def errorprint(o, *pars):
  print "ERROR in ", o.__class__.__name__,":",pars
  #print "sys:",sys._getframe(1).f_code.co_name    see inspect
def gt(x,y):
  if(x>y):
    return(x)
  else:
    return(y)

frommsHelp="""
BC down-scaling factor is an integer number (32bits) giving the number
of BCs (25ns) between 2 triggers. 
Examples:
0          -> BC rate (40MHz)
1          -> 40/2 MHz rate
4000       -> generate 1 START signal per 100 micsecs
40000      -> generate 1000 START signals per second
40000000   -> generate 1 START signal per second
0x7fffffff -> generate 1 START signal per 53.7 seconds
n          -> generate 1 START signal per (n+1)/BC micsecs, where
              BC is BC rate (40*10**6)

or use: s  (seconds)  ms  (miliseconds)  us  (micsecroseconds)   for period or
        hz (Hertz)    khz (Kilohertz)    mhz (Megahertz)         for rate:
3s    -   3 seconds
10ms  -  10 miliseconds period
300us - 300 microseconds period
3khz  -   3 kilohertz rate
1mhz  -   1 megahertz rate
"""
def fromms(oldval, newval):
  """return: None if syntax error
  """
  bcfrekv=40.
  try:
    if string.lower(newval[-1])=='s':   # period given in s,ms, or us
      #  n= period*BC-1  n->VMEwrite, period[micsecs], BC[MHz]
      retval= newval[:-1]
      mult=1000000
      if string.lower(retval[-1])=='m':     # milisecs
        retval= retval[:-1]
        mult=1000
      elif string.lower(retval[-1])=='u':   # micsecs
        retval= retval[:-1]
        mult=1
      retval=str(int(int(retval)*mult*bcfrekv-1))
    elif string.lower(newval[-2:])=='hz':   # rate given: hz khz or mhz
      retval= newval[:-2]
      mult=1000000
      if string.lower(retval[-1])=='m':     # mhz
        retval= retval[:-1]
        mult=1
      elif string.lower(retval[-1])=='k':   # micsecs
        retval= retval[:-1]
        mult=1000
      retval=str(int(bcfrekv*mult/int(retval)-1))
    elif (len(newval)>2) and (newval[:2]=='0x'):  # direct hexa
      retval=newval
    else:     # n given directly as int
      intrep= int(newval)
      retval= str(newval)
  except:
    retval=None
  #print "fromms:",oldval,type(oldval), newval,type(newval),retval
  return retval
frommsL0prHelp="""
L0 down-scaling factor is an integer number (21 or 25 bits + flag in bit31).
Rate reduction (bit31:0, 21 LSB bits are valid):
-----------------------------------------------
0x1fffff -veto permanently asserted
0       -no downscaling
0xfffff -50% downscaled to half

Class busy (bit31:1, 25 least significant bits valid):
-----------------------------------------------------
Artificial class busy time is encoded in 25 LSBs, 1count is 10micsecs.

Example:
0x800001f4 means 'class busy' downscaling, class will be busy
with each trigger 5milsecs (0x1f4*10micsecs= 5ms).

For convenience, Class busy can be given in time units (us, ms, s)
or rate units (hz khz).
Rate reduction can be given as fraction of triggers to go through (%).
Examples:
0          -> no prescaling
1%         -> Rate reduction by 100 times
1.5%       -> Rate reduction to 1.5%
40ms       -> Class will be busy 40 miliseconds after each trigger
1khz       -> Class busy is set to 1 milisecond

Synchronous downscaling:
-----------------------
This option is valid, if L0pr if defined by symbolic name.
Symbolic name has to define n% value (0<=n<=100) -see Show ->
Synchronous downscaling groups
"""
def frommsL0pr(oldval, newval):
  """return: None if syntax error
  """
  try:
    if string.lower(newval[-1])=='s':   # period given in s,ms, or us
      #  n= period*BC-1  n->VMEwrite, period[micsecs], BC[MHz]
      retval= newval[:-1]
      mult=1000000
      if string.lower(retval[-1])=='m':     # milisecs
        retval= retval[:-1]
        mult=1000
      elif string.lower(retval[-1])=='u':   # micsecs
        retval= retval[:-1]
        mult=1
      retval=str(0x80000000 | int(int(retval)*mult/10.))
    elif string.lower(newval[-2:])=='hz':   # rate given: hz khz or mhz
      retval= newval[:-2]
      mult=1000000
      if string.lower(retval[-1])=='k':     # khz
        retval= retval[:-1]
        mult=1000
      retval=str(0x80000000 | int(mult/int(retval)/10.))
    elif string.lower(newval[-1])=='%':   # downscaling in %
      pr= newval[:-1]
      fpr= float(pr)
      if (fpr<0.0) or (fpr>100.0):
        retval=None
      else:
        retval= str(int(round((100-fpr)*0x1fffff/100)))
    elif (len(newval)>2) and (newval[:2]=='0x'):  # direct hexa
      retval=str(eval(newval))
    else:     # n given directly as int
      #intrep= int(newval)
      intrep= string.atoi(newval)
      retval= str(newval)
  except:
    retval=None
  #print "frommsL0pr:",oldval,type(oldval), newval,type(newval),retval
  return retval
frommsRandomHelp="""
Average rate of the randomly generated signals is coded in low 31 bits:
0x7fffffff -maximum rate
n          -the average rate will be set to n*BC/0x7fffffff where BC is
            bunch crossing rate (40MHz). I. e. for rate 1000 Hz,
            n= 1000*0x7fffffff/40/10**6 = 53687
For convenience, period can be given in micsecs [us] milisecs [ms]
or rate in megahertz [mhz] kilohertz [khz] or hertz [hz]
or seconds [s]. Example:
100us  -average period 100 micseconds
30ms   -average period 30 miliseconds
5s     -average period 5 seconds
3khz  -   3 kilohertz rate
1mhz  -   1 megahertz rate
"""
def frommsRandom(oldval, newval):
  """return: None if syntax error
  """
  try:
    if string.lower(newval[-1])=='s':   # period given in s,ms, or us
      retval= newval[:-1]
      mult=1000000
      if string.lower(retval[-1])=='m':     # milisecs
        retval= retval[:-1]
        mult=1000
      elif string.lower(retval[-1])=='u':   # micsecs
        retval= retval[:-1]
        mult=1
      rate= 1000000./(mult*int(retval))
      #print "frommsRandom:", rate, retval
      retval= str(int(0x7fffffff/40./10**6 * rate))
    elif string.lower(newval[-2:])=="hz":
      retval= newval[:-2]
      mult=1
      if string.lower(retval[-1])=='m':     # mhz
        retval= retval[:-1]
        mult=1000000
      elif string.lower(retval[-1])=='k':   # khz
        retval= retval[:-1]
        mult=1000
      rate= mult*int(retval)
      retval= str(int(0x7fffffff/40./10**6 * rate))
    elif (len(newval)>2) and (newval[:2]=='0x'):  # direct hexa
      retval=newval
    else:                 # n given directly as int
      intrep= int(newval)
      retval= str(newval)
  except:
    retval=None
  return retval

class curry:
  '''
  def __init__(self, fun, *args, **kwargs):
    self.fun = fun
    self.pending = args[:]
    self.kwargs = kwargs.copy()
  def __call__(self, *args, **kwargs):
    if kwargs and self.kwargs:
      kw = self.kwargs.copy()
      kw.update(kwargs)
    else:
      kw = kwargs or self.kwargs
    return self.fun(*(self.pending + args), **kw)
  '''
  def __init__(self, callback, *args, **kwargs):
    self.callback = callback
    self.args = args
    self.kwargs = kwargs
  def __call__(self):
    return apply(self.callback, self.args, self.kwargs)

class MywError:
 """Usage:
 myw.MywError("error message")
 """
 def __init__(self,errmsg="Unknown error",iow=None, fw=None):
   self.fw=fw
   self.tlm=Toplevel(bg="blue")
   cr= getCrate()
   if cr: self.tlm.group(cr.master)
   #print ":",iow,":"
   self.tlm.title("Error message")
   #self.tlm.lift()   # not very useful (the same with and without)
   if iow==None: # let error message overlap the 'crate' widget
     if cr:
       iow=cr.crframe.winfo_rootx(),cr.crframe.winfo_rooty()
     else: iow=(1,1)
   xystr= "+"+str(iow[0])+"+"+str(iow[1])
   self.tlm.geometry(xystr)
   self.tlm.bind("<Destroy>", self.cancel)
   l=MywLabel(self.tlm,errmsg)
   l.configure(background="red")
   MywButton(self.tlm, 'Cancel', bg="red",cmd=self.cancel)
   #self.tlm.focus_set()   #seems not to have sense with grab_set
   #self.tlm.grab_set()
   self.reps=0
   self.repeatErrorfocus()
 def repeatErrorfocus(self,cancel=None):
    if self.reps >= 10 or cancel:
      print "MywError: cancelling red error widget"
      self.tlm.after_cancel(self.afterid)
      self.reps=0
    else:
      print "MywError: starting 10x 1000ms"
      if self.fw:
        #von self.fw.lift(); 
        self.fw.event_generate("<Button-1>"); 
        self.fw.focus_set(); 
      else:
        self.tlm.focus_set(); 
      self.tlm.lift(); #self.tlm.bell()
      self.afterid=self.tlm.after(1000, self.repeatErrorfocus)
      self.reps= self.reps+1
 def cancel(self, ev=None):
   self.repeatErrorfocus(cancel='yes')
   self.tlm.destroy()

class Kanvas(Canvas):
  bitWidth= 8   # 10   # including border lines
  bitHeight= 20 # 18 10
  bitBorder=1
  interspace=1
  colHelpBg='#ccffff'
  scalwidth=40
  def __init__(self, tlw, canvasDestroyed=None, ctpcfg=None, 
    scalerpos= (260,400), **kw):
    self.tlw=tlw
    self.ctpcfg=ctpcfg   # to get help window over L0 scaler entries
    self.scalerpos= scalerpos
    selfargs=(self,self.tlw)
    apply(Canvas.__init__,selfargs, kw)
    #self.pack(fill='y', side=RIGHT)
    #self.pack(expand='yes', fill='y', side=RIGHT)
    #self.pack(side=RIGHT)
    #frame= Frame(self)    # fr_version
    self.pack(side=LEFT)
    # create scrollbar
    # see http://effbot.org/zone/tkinter-scrollbar-patterns.htm
    self.scrollbar = Scrollbar(self.tlw)
    #self.sbfm= Frame(self.tlw)
    #self.scrollbar = Scrollbar(self.sbfm)
    self.scrollbar.pack(side=RIGHT, fill=Y)
    #print "Kanvasinit:", self.scrollbar, self.tlw
    #self.create_window((0,0),window=frame,anchor='nw')   # fr_version
    #frame.bind("<Configure>", #   fr_version
    # attach scrollbar to Canvas:
    self.config(yscrollcommand=self.scrollbar.set)
    self.scrollbar.config(command=self.yview)
    if canvasDestroyed!=None:
      self.bind("<Destroy>",canvasDestroyed)
  #def canvasDestroyed(self,ev):
  #  print "myw.Kanvas destroyed"
  def destroy(self):
    self.scrollbar.destroy()
    #print "myw Kanvasdestroy:"
    Canvas.destroy(self)
  def dobit(self, xy, color=None, helptext=None):
    """ Create box.
    rc: canvas box id
    """
    if color==None: color="white"
    bitid= self.create_rectangle(xy[0],xy[1],
      xy[0]+Kanvas.bitWidth-1, xy[1]+Kanvas.bitHeight-1, fill=color)
    # handlers:
    #if bithandler == Klas.modL0handler:
    #if isinstance(bithandler, Klas.modL0handler):
    #print "bithandler:",bithandler
    #handler= lambda e,s=self,k=bitposition:s.bithandler(e, k)
    if helptext: self.doHelp(bitid, helptext)
    return bitid
  #def bithandler(self,e,k):
  #  print "s.bithandler:",e,k
  def doEntry(self, xy, klas_method):
    entw= MywEntry(self.tlw,bind='lr', label='',
      cmdlabel=klas_method, width=10)
    #print "doEntry:",xy
    id= self.create_window(xy[0], xy[1],window=entw,
      anchor=NW, tags="TAGl0pr")
    #self.tag_bind(id, "<Enter>", self.winvisible)
    return entw
  def doHelp(self, id, helptxt, dynhelp=None):
    """ id: canvas object id
    dynhelp: callback function called with 1 parameter: helptxt
             in time when cursor enters the bit-field
    """
    enthandler= lambda e,s=self,k=helptxt,dh=dynhelp:s.enterbit(e, k, dh)
    pf3handler= lambda e,s=self,k=helptxt,dh=dynhelp:s.pf3help(e, k, dh)
    #print "Kanvas.doHelp:",type(id)
    self.tag_bind(id, "<Enter>", enthandler)
    self.tag_bind(id, "<Leave>", self.leavebit)
    self.tag_bind(id, "<Button-3>", pf3handler)
  def enterbit(self, event, hlptext="entering bit", dynhelp=None):
    obrd=5; cursd=20
    canh=int(self.cget("height"))
    canw=int(self.cget("width"))
    x= event.x; y=event.y
    #print "Kanvas.enterbit:x:",x," y:",y
    if hlptext==None: hlptext="Entering bit"
    if dynhelp!=None: 
      hlptext=dynhelp(hlptext)
    thlptxt= self.create_text(x,y,anchor=NW,
      tags="TAGhlptemp", text=hlptext)
    cs=self.bbox(thlptxt); txth= cs[3]-cs[1]; txtw=cs[2]-cs[0]
    # find the best position for text (NE SE SW NW)
    #if x>canw/2:    # tooltip on right side
    if not (((x>self.scalerpos[0]) and (x<(self.scalerpos[0]+Kanvas.scalwidth))) or\
            ((x>self.scalerpos[1]) and (x<(self.scalerpos[1]+Kanvas.scalwidth)))):
      if y<canh/2:   #NE
        x= event.x-cursd-txtw; y=event.y+cursd
      else:          #SE
        x= event.x-cursd-txtw; y=event.y-cursd-txth
    else:            # tooltip on left side
      if y<canh/2:   #NW
        x= event.x+cursd; y=event.y+cursd; #recreate=0
      else:          #SW
        x= event.x+cursd; y=event.y-cursd-txth
    #print "Kanvas.enterbit:new x:",x," y:",y
    ####if cs[2] > Klas.l0scalerx0:
    #print "canh canw x y:",canh,canw,x,y #type(canh),type(cs[3])
    # recreate:
    self.delete("TAGhlptemp")
    # x print "Kanvas:enterbit oldxy:",xorig,yorig," new x y:",x,y
    thlptxt= self.create_text(x,y,anchor=NW,
      tags="TAGhlptemp", text=hlptext)
    cs=self.bbox(thlptxt)
    t1= cs[0]-obrd 
    t2= cs[1]-obrd
    t3= cs[2]+obrd; t4= cs[3]+obrd
    # there is an error in following lines:
    #t1= self.ifnegorge(t1, canw, 0, canw/2)
    #t3= self.ifnegorge(t3, canw, canw-1, canw-1)
    #t2= self.ifnegorge(t2, canh, 0, canh/2)
    #t4= self.ifnegorge(t3, canh, canh-1, canh-1)
    #print "Kanvas.enterbit w h    x0 y0 x1 y1",canw,canh, t1,t2,t3,t4
    try:
      self.ovalhelp= self.create_rectangle(t1,t2,t3,t4,
        tags="TAGhlptemp", fill=Kanvas.colHelpBg)
    except:
      print "Kanvas.enterbit:", t1, t2, t3, t4
    #self.tag_raise(self.ovalhelp, "TAGl0pr")  not working
    self.toliftback=[]
    if self.ctpcfg: 
      sse= self.ctpcfg.getl0scalerx12()
      if ((sse[0]>t1) and (sse[0]<t3)) or \
         ((sse[1]>t1) and (sse[1]<t3)) or \
         ((sse[0]<t1) and (sse[1]>t3)):   # help covered by L0scaler entries
        for cls in self.ctpcfg.klasses:
          if cls.linenumber>0:
            #print "Kanvas-upper:", cls.linenumber
            #cls.scalentry.lower(self.ovalhelp) #not working
            #cls.scalentry.lower(self.tlw)     #not working
            pass
            #ok self.toliftback.append(cls.scalentry)
            #ok cls.scalentry.lower(self)         # scalentry disappears
    self.tag_raise(thlptxt, self.ovalhelp)
    #self.lift(thlptxt)
    #self.hide("TAGl0pr")
    #self.lift("TAGhlptemp")
    self.itemconfigure("TAGl0pr", state=HIDDEN)
  def ifnegorge(self,value, upperlimit, newifneg, newifge):
    if value<0:return newifneg
    if value>=upperlimit:return newifge
    return value
  #def winvisible(self, event):
  #  #print "winvisible:"
  #  self.itemconfigure("TAGl0pr", state=NORMAL)
  def leavebit(self, event):
    #print "leavebit:"
    self.itemconfigure("TAGl0pr", state=NORMAL)
    self.delete("TAGhlptemp")
    #self.delete(self.ovalhelp)
    for se in self.toliftback:
      se.lift(self)
    self.toliftback=[]
  def pf3help(self, event, hlptext="helppf3",dynhelp=None):
    if dynhelp: hlptext= dynhelp(hlptext)
    #blbina MywHelp(None, hlptext)
    print "Kanvas.pf3help:",hlptext

class MultRegs:
  def __init__(self, board):
    self.mrmaster=NewToplevel("multireg "+ board.boardName)
    self.regsframe= MywFrame(self.mrmaster)
    self.board=board; self.perrep=0
    self.regs=[]
    self.rgitems=[]
    self.addrgitems=[]
    for rg in self.board.vmeregs:
      #print 'MultRegs:',rg
      #htext=htext+ rg[0] + " = " + rg[1] + "\n"
      if rg[2]=='':   # take only 32 bit registers
        self.rgitems.append((rg[0],rg[1]))
        self.addrgitems.append((rg[0],rg[1],self.addReg))
    self.rgitems.append(("remove me","removevalue"))
    #self.cr= MywxMenu(self.mrmaster, items=self.rgitems)
    #readbut= MywButton(self.mrmaster, label='read', 
    #  cmd=self.allread, side='left')
    #writebut= MywButton(self.mrmaster, label='write', cmd=self.vmewrite, side='left')
    #okbut= MywButton(self.mrmaster, label='quit', 
    #  cmd=self.mrmaster.destroy, side='bottom')
    self.butsframe= MywFrame(self.mrmaster)
    self.addqframe= MywFrame(self.mrmaster)
    self.butts=MywHButtons(self.butsframe, 
      [("read", self.allread),
       ("periodic read", self.perRead),
       ("quit", self.mrmaster.destroy)])
    self.addr= MywxMenu(self.addqframe, items=self.addrgitems, 
      helptext="Choose reisters to be read",
      label='Addreg:',side=LEFT)   #, cmdlabel=self.chooseReg)
  def addReg(self, inx=None):
    #print "addReg:", self.rgitems
    #self.addr.printEntry('MultRegaddReg:')
    if inx==None: inx= self.addr.getIndex()
    r= MywEntry(self.regsframe, label=self.rgitems, 
      delaction=self.destroyReg, defaultinx=inx,
      side=TOP)   #, cmdlabel=self.chooseReg)
    dbgprint(self, "MultRegs.addReg:",r)
    #r.label.bind("<Enter>", self.cr.popup)
    #blika self.mrmaster.bind("<Leave>", self.cr.unpopup)
    self.regs.append(r)
  def destroyReg(self, entrywidget):
    dbgprint(self, "destroyReg:",self.regs,entrywidget)
    #delete the item from self.regs
    toremove=-1
    for i in range(len(self.regs)):
      if self.regs[i] is entrywidget:
        #print "destroyReg2:", self.regs[i]
        #print "          =:", entrywidget
        toremove=i
    if toremove!= -1:
      del self.regs[toremove]
    else:
      errorprint(self,"bad remove",self.regs)
    entrywidget.destroy()
  def doout(self,allvals):
    #print "allread3:",allvals,':'
    vlist=string.split(allvals); vll= len(vlist)
    if vll != len(self.regs):
      errorprint(self, "%d != %d"%(vll,len(self.regs)))
    #dbgprint(self,"allread2:",vlist)
    for i in range(vll):
      prevval= self.regs[i].getEntryHex()
      #print "allread:",prevval,vlist[i]
      if prevval != vlist[i]:
        self.regs[i].entry.configure(bg=COLOR_VALCHANGED)
      else:
        #self.regs[i].entry.configure(bg=COLOR_VALNORMAL)
        self.regs[i].setColor()
      self.regs[i].setEntry(vlist[i])
  def allread(self):
    addrs=[]
    cmd2=""
    for i in range(len(self.regs)):
      addrs.append(self.regs[i].label.posval.get())
      #self.regs[i].setEntry(addrs[i])
      cmd2=cmd2+','+str(addrs[i])
    #print 'allread  :', addrs
    if self.board.io==None: self.board.openCmd()
    #cmd="vmeopmr32("+ str(len(self.regs))+cmd2+')'
    cmd="vmeopmr32("+ str(len(self.regs))+')'
    # 1 line: 0x12 0xabcd ... expected
    #allvals=string.split(self.board.io.execute(cmd),'\n')[-2]
    thdrn=self.board.io.execute(cmd,ff=self.doout)
    #print 'MultRegs.cmd:',cmd, 'thrdn:',thdrn
    self.board.io.thds[thdrn].waitcmdw()
    for i in range(len(self.regs)):
      addrs.append(self.regs[i].label.posval.get())
      #self.regs[i].setEntry(addrs[i])
      self.board.io.thds[thdrn].pwrite(str(addrs[i])+"\n")
    #
  def repeatRead(self, cancel=None):
    if self.perrep >= 100 or cancel:
      self.mrmaster.after_cancel(self.afterid)
      self.butts.buttons[1].configure(bg=self.normalcolor[0],
        activebackground=self.normalcolor[1])
      self.perrep=0
    else:
      self.allread()
      self.afterid=self.mrmaster.after(1500, self.repeatRead)
      self.perrep= self.perrep+1
  def perRead(self):
    #print 'perRead.perrep:',self.perrep
    #self.allread()
    if self.perrep == 0:   #just starting
      self.normalcolor=self.butts.buttons[1].cget('bg'),\
        self.butts.buttons[1].cget('activebackground')
      #print 'perRead.normalcolor:',self.normalcolor
      self.butts.buttons[1].configure(bg=COLOR_RUNNING,
        activebackground=COLOR_RUNNING)
      self.repeatRead()
    else:                  # request to stop periodic read
      self.repeatRead(cancel='yes')

class VmeRW:
  def __init__(self,vboard):
    #print "here VmeRW"
    self.vboard=vboard
    rwf=NewToplevel("r/w "+ vboard.boardName+' '+vboard.baseAddr)
    #
    adrframe=MywFrame(rwf)
    valframe=MywFrame(rwf)
    htext='base:%s\nVME regs:\n'%vboard.baseAddr
    items=[]
    for rg in vboard.vmeregs:
      #print 'VmeRW:',rg
      htext=htext+ rg[0] + " = " + rg[1] + "\n"
      items.append( (rg[0],rg[1]) )
    if len(items)>0:
      rt= vboard.findvmeregtyp(items[0][0])
      defx={"w8":0, "w16":1,"w32":2}[rt]
      helptext="""Symbolic name of VME register.
"""
      if vboard.boardName=='ctp':
        helptext= helptext+"""
For some CTP registers add:
0x8000  for BUSY board
0x9000  for L0   board
0xa000  for L1   board
0xb000  for L2   board
0xc000  for INT  board
0x1000  for FO1  board
0x*000  for FO*  board
0x6000  for FO6  board


NOTE1 for L0: 
Addresses ending r2 or lm0 (e.g. MASK_MODEr2) are different for
L0 vs. LM0 board. Choose correct one, according to the currently plugged board
(r2/lm0 to be used with LM0 board. Addresses without r2/lm0 suffix
are the same for both boards or they are valid ONLY for L0 board
if they have also r2/lm0 counterpart).

NOTE2 for L0: 
when working with LM0 board, the following block of registers
has addresses given below (the one appearing in addr: field is
correct for an old L0_100classes board):
#define L0_INTERACT1   0x9204    0x95bc-0x9204= 0x3b8 -> see L0LM0DIFF()
#define L0_INTERACT2   0x9208
#define L0_INTERACTT   0x920c
#define L0_INTERACTSEL 0x9210
                            
#define L0_FUNCTION1   0x9214
#define L0_FUNCTION2   0x9218
#define RANDOM_1       0x921c
#define RANDOM_2       0x9220
#define SCALED_1       0x9224
#define SCALED_2       0x9228
#define ALL_RARE_FLAG  0x922c

This is the price, we pay for working with both
L0+LM0 boards using the same software without recompilation and using
different addresses for L0/LM0.
"""
      self.adrmenu=MywMenu81632(adrframe, items=items,side='right', helptext=helptext)
      self.adrmenu.boardmod=self
      self.addrent= MywEntry(master=adrframe,label='addr:', side='left',\
        helptext=htext, textvariable=self.adrmenu.posval,width=8)
    else:
      # no registers defined:
      #print 'VmeRW:',items[0],self.findvmeregtyp(items[0][0])
      defx={"w8":0, "w16":1,"w32":2}['w32']
      self.addrent= MywEntry(master=adrframe,label='addr:', side='left',\
        helptext=htext, width=8)
    #
    self.v81632menu=MywxMenu(valframe,defaultinx=defx, 
      items=(("8b","8"),("16b","16"),("32b","32")),side='right')
    self.valent= MywEntry(master=valframe,label='value:', side='left',
      width=10, helptext="""
Value read/written to VME register. Value can be given in:
decimal notation   e.g. 250
hexadec. notation  e.g. 0xfa
Use RIGHT mouse button for decimal <-> hexadecimal conversion.
""")
    #
    readbut= MywButton(rwf, label='read', cmd=self.vmeread, side='left')
    writebut= MywButton(rwf, label='write', cmd=self.vmewrite, side='left')
    okbut= MywButton(rwf, label='quit', cmd=rwf.destroy, side='left')
  def vmeread(self):
    #print "wmeread from:",self.vboard.addrent.getEntry()
    #if self.vboard.adrmenu.posval.get()==self.addrent.getEntry():
    bits= self.v81632menu.posval.get()
    cmd= "vmeopr"+bits+"("+self.addrent.getEntry()+")" 
    if self.vboard.io==None: self.vboard.openCmd()
    alllines=string.split(self.vboard.io.execute(cmd),'\n')
    #print 'alllines:',alllines,'---'
    if len(alllines)>=2:
      a=alllines[-2]
      self.valent.setEntry(a)
    #print "vmeread:",a,"aa vmeread: ['0xf96f', ':', ''] aa"
  def vmewrite(self):
    bits= self.v81632menu.posval.get()
    cmd= "vmeopw"+bits+"("+self.addrent.getEntry()+", "+\
      self.valent.getEntry()+")" 
    if self.vboard.io==None: self.vboard.openCmd()
    self.vboard.io.execute(cmd)
  def modexit(self):
    #print "here modexit"
    if self.adrmenu.posval.get()==self.addrent.getEntry():
      #print "modexit:",self.adrmenu.radiobut['text']
      tx= self.vboard.findvmeregtyp(self.adrmenu.radiobut['text'])
      self.v81632menu.posval.set(tx[1:])
      self.v81632menu.radiobut['text']= tx

class MywHelp:
  """Usage:
  1. For classes inheriting MywHelp (see MywButton for example):
  myw.MywHelp(self.topfr,"help text")
  2. For classes not inheriting MywHelp:
  myw.MywHelp(self.topfr,"help text", widget)
  widget -widget to which Button-3 will be bound
  """
  maxheight=80
  def __init__(self,master,helptext,hlpwidget=None,baloon=None):
    self.fmaster=master
    self.newhelp(helptext)
    self.hw=None
    if hlpwidget!=None:
      sbnd= hlpwidget
    else:
      sbnd= self
    c= getCrate()
    if c and c.autohelps:
      sbnd.bind("<Enter>", self.entercb)
      sbnd.bind("<Leave>", self.leavecb)
    else:
      if baloon:
        self.baloonfun=baloon
        self.baloonfun2= lambda s=self:s.baloonfun()
        #sbnd.bind("<Enter>", self.entercb)
        #sbnd.bind("<Leave>", self.leavecb)
      sbnd.bind("<Button-3>",self.help)
  def help(self, event=None):
    if self.hw: return
    #self.hw=Toplevel(self.fmaster, width=0, height=0)
    self.hw=NewToplevel("help window")
    #cr= getCrate()
    #if cr: 
      #self.hw.transient(cr.master)
      #self.hw.group(cr.master)
      #print 'lower' -nohelp
      #self.hw.lower(cr.master)
    self.hw.config(bg="green")
    #main widget (crate) left top corner in pixels:
    cratex=self.fmaster.winfo_rootx(); cratey=self.fmaster.winfo_rooty()
    cratew=self.fmaster.winfo_width()
    self.hw.bind("<Destroy>",self.dest)
    scrollbar = Scrollbar(self.hw)
    wi,self.he= self.finwh(self.helptext)
    if self.he>self.maxheight:
      text=Text(self.hw, bg=self.hw.cget('bg'),width=wi, height=self.maxheight,
         yscrollcommand=scrollbar.set ); 
    else:
      text=Text(self.hw, bg=self.hw.cget('bg'),width=wi, height=self.he)
    #print self.hw.config()
    text.insert(END, self.helptext);
    text.config(state=DISABLED);
    text.pack(side=LEFT,expand='yes', fill='x');
    self.hw.update_idletasks()
    helpheight= self.hw.winfo_height() + 40
    helpwidth= self.hw.winfo_width()
    #print "cratex,y:",type(cratex),cratex,cratey
    #print "helpheight,Width:", helpheight, helpwidth,type(helpwidth)
    screenheight= self.hw.winfo_screenheight()
    screenwidth= self.hw.winfo_screenwidth()
    #print "screen height,width:",screenheight,screenwidth
    #x
    if cratex<screenwidth/2: x= cratex+cratew
    else: x= cratex-helpwidth
    if x<0 or x>screenwidth: x=0
    #y
    if (cratey+helpheight)<screenheight: y= cratey
    else: y= screenheight-helpheight
    if y<0: y= 0
    xy= '+%d+%d'%(x,y)
    #print "xy:",x,y
    self.hw.geometry(xy)
    if self.he>self.maxheight:
      scrollbar.config(command=text.yview)
      scrollbar.pack(side=RIGHT, fill='y')
    #okbut.pack(side=BOTTOM,expand='yes', fill='x') 
  def newhelp(self,helptext):
    self.helptext=helptext
  def finwh(self,s):
    #print "finwh:",s
    ss=string.split(s,'\n')
    h=len(ss)
    return(reduce(gt,map(len, ss)),h)
  def entercb(self,event):
    print "entercb", event.x, event.y, self.baloonfun
    self.baloonfun2()
    self.afterid=self.after(1000, self.help)
  def leavecb(self,event):
    print "leave", event.x, event.y 
    if self.hw:
      #self.MywHelp.hw.destroy() -nedobre
      if self.he<=self.maxheight:
        self.hw.destroy()
        self.hw=None
    else:
      self.after_cancel( self.afterid)
  def dest(self,event):
    #print "dest here"
    self.hw=None

class MywRadio:
  """
  Creates more Radiobuttons in 1 frame, together with a common Label
  button describing their purpose.
  cmdlabel: action started when Label button is pressed
  """
  def __init__(self, master=None,label="radio button",
    helptext=None, cmdlabel=None,
    items=(("text1","1"),("text2","2"))):
    self.posval= StringVar(); self.posval.set(items[0][1])
    self.radf=Frame(master, borderwidth=1,relief=RAISED)
    self.radf.pack(expand=YES,fill='x')
    self.label=MywEntry(self.radf,label=label, defvalue='',
      helptext=helptext,cmdlabel=cmdlabel)
    self.choices=[]
    for ch in items:
      self.choices.append( Radiobutton(self.radf,
        text=ch[0], variable=self.posval, value=ch[1]))
      self.choices[-1].pack()
  def getEntry(self):
    #print "radio:", self.posval.get()
    return(self.posval.get()) 
#  def config(self, state=NORMAL):
  def printEntry(self, text='printEntry:'):
    print text,self.getEntry()
#
"""
class MywEntry(Frame, MywHelp):
  def __init__(self, master=None, text='butlabel', cmd=None, 
    bg=None,side=None,anchor=None,funchelp=None,relief=RAISED):
    # zda sa ze nemusi byt self.but:
    #but= Button(master, text=text, command=cmd, bg=bg, activebackground=bg)
    #but.pack(side=side, expand='yes', fill='x')
    Button.__init__(self,master, text=text, command=cmd, bg=bg, activebackground=bg, relief=relief)
    Button.pack(self,side=side, expand='yes', fill='x', anchor=anchor)
    #if funcdescr and funcdescr[VmeBoard.NFUSAGE]:
    #  MywHelp.__init__(self,master,funcdescr[VmeBoard.NFUSAGE])
    if funchelp: MywHelp.__init__(self,master,funchelp)
"""
class MywEntry(Frame,MywHelp):
  """
  label: string -presented together with the Entry (on the left side)
         list   -see MywxMenu. In this case, delaction can be supplied
                 too, and defaultinx parameter for 'initial' item
  bind:  lr or r -cmdlabel is activated when cursor leaves entry (l)
                  or 'return' or 'Tab' is pressed (r)
  cmdlabel: command to be executed when label pressed
            or (Enter key pressed when entry field is active -to be done)
  defvalue: if '', Entry part is not shown
  vme32: string representing VME32 address (symbolic name or
         constant)
  external methods:
  setEntry()
  getEntry(), getEntryBin(), getEntryHex()
  """
  def __init__(self, master, label='label', defvalue=' ',bind=None,
    side='left', helptext="", cmdlabel=None, width=None, bg=None,
    textvariable=None, delaction=None,defaultinx=0,name=None,
    relief=SUNKEN, expandentry='yes'):
    self.cmdlabel=cmdlabel
    self.nointcheck= 1   # to do: as parameter + updateentry + self.value 
    Frame.__init__(self,master, bg=bg)
    Frame.pack(self,side=side, expand=expandentry, fill='x')
    #if bind=='lr': 
    #  Frame.bind(self,"<Leave>", cmdlabel)
    #  Frame.bind(self,"<Key-Return>", cmdlabel)
    #if bind=='r': 
    #  Frame.bind(self,"<Key-Return>", cmdlabel)
    #self.master=master         # for MywxMenu -destroyReg()
    #1711Frame.pack(self,side=side)
    self.bind("<Destroy>", self.dummycmd)
    self.labelname='' ; self.label= None
    self.helptext=helptext
    if helptext: 
      MywHelp.__init__(self,master, helptext, baloon=self.printEntry)
    if label!='':
      if type(label) is types.ListType:
        self.label= MywxMenu(self, items=label,side=LEFT,
        helptext=helptext, delaction=delaction,defaultinx=defaultinx)
      if type(label) is types.StringType:
        self.labelname=label
        if self.cmdlabel and bind==None:
          self.label=MywButton(self,label=label,helptext=helptext, 
            side='left', relief=FLAT, cmd=self.updateentry, bg=bg)
        else:
          self.label=MywLabel(self,label=label,helptext=helptext, 
            side='left', relief=FLAT, bg=bg)
      #self.label.bind("<Destroy>", self.dummycmd)
    #else:
    #print "MywEntry:bind:",bind,"defvalue:",defvalue
    self.normalcolor= COLOR_BGDEFAULT
    if defvalue:
      self.conv2dec=2   #0: dec2hex  1: hex2dec   2: no conversion
      self.entry=Entry(self, width=width,textvariable=textvariable,
        name=name, relief=relief,bg=bg)
      self.entry.insert('end',defvalue)
      self.entry.pack(side='left', expand=expandentry, fill='x')
      self.entry.bind("<Button-3>",self.convertStart)
      if bind=='lr': 
        self.entry.bind("<Leave>", self.updateentry)
        self.entry.bind("<Key-Return>", self.updateentry)
        self.entry.bind("<Key-Tab>", self.updateentry)
      if bind=='r': 
        self.entry.bind("<Key-Return>", self.updateentry)
        self.entry.bind("<Key-Tab>", self.updateentry)
    else:
      self.entry=None
    #print "MywEntry: label, defavalue:",label, defvalue
  def lower(self, widg):
    Frame.lower(self, widg)
  def lift(self, widg):
    Frame.lift(self, widg)
  def dummycmd(self,event):
    pass
    #print "dummycmd:",event
  def updateentry(self, event=None):
    """event is None in case of activation by button..."""
    #print "MywEntry:",self.getEntry(),event
    ne= self.entry.get()
    if self.nointcheck:
      if self.cmdlabel: self.cmdlabel(ne)
      return
    try:
      ne_b= eval(ne)
    except:
      MywError("bad value (int or hex expected):"+ne, fw=self.entry)
    else:
      if self.cmdlabel: self.cmdlabel(ne)
  def setEntry(self, text):
    if self.entry==None:
      print "myw.setEntry None...?"
      return
    self.entry.delete(0, 'end')
    if self.conv2dec==1:
      text= self.hex2dec(text)
    else:
      if self.conv2dec==0:
        text= self.dec2hex(text)
    #print 'Here setEntry:',text
    self.entry.insert(0, text)
  def getLabel(self):
    return self.labelname
  def getEntry(self):
    tx= self.entry.get()
    return(tx)
  def getEntryBin(self, defbin=0xdeadbeaf):
    tx= self.entry.get()
    if tx=='': tx='0'
    try:
      txbin= eval(tx)
    except:
      print "Bad value (int or 0x... expected). Using: 0xdeadbed"
      txbin= defbin
    return(txbin)
  def getEntryHex(self):
    tx= self.entry.get()
    if tx[0:2]!='0x':
      try:
        tx= int(tx)
        tx= "0x%x"%(tx)
      except:
        pass
    return(tx)
  def setColor(self, color=None):
    if color==None:
      color= self.normalcolor
    self.entry.configure(bg=color)
    #self.label.configure(bg=color)
  def convertStart(self,event=None):
    # make this entry visible as decimal number:
    #print "convertStart:",
    t=self.getEntry();
    if self.conv2dec==0 or self.conv2dec==2:
      self.conv2dec=1
      self.setColor(COLOR_DECIMAL)
      self.setEntry(self.hex2dec(t))
    else:
      self.conv2dec=0
      self.setColor(COLOR_BGDEFAULT)
      self.setEntry(t)
  def hex2dec(self,text):
    #print "hex2dec:",text, range(len(text)-1,1,-1)
    #text=int(text,16)    -in python 2.2.2
    """
    hexdig={'0':0,'1':1,'2':2,'3':3,'4':4,'5':5,'6':6,'7':7,
            '8':8,'9':9,'a':10,'b':11,'c':12,'d':13,'e':14,'f':15,
            'A':10,'B':11,'C':12,'D':13,'E':14,'F':15}
    d=0L; exp=1
    if text[0:2]=="0x":
      for i in range(len(text)-1,1,-1):
        c= text[i]
        d= d+ hexdig[c]*exp
        print "hex2dec:",c,d
        exp=exp*16
    """
    try:
      d=eval(text)
      if d<0:
        texto=text+'L'
        d= eval(texto)
        texto= str(d)   #[:-1]
      else:
        texto= str(d)
    except:
      texto=text
      #pass
    #print "hex2dec:",text, texto
    return texto
  def dec2hex(self,text):
    #print "dec2hex:",text
    if text=="" or text==" ":
      return text
    try:
      if text > "2147483647":
        text= text+'L'
        #print "big n:",text
        d= eval(text)
        text= hex(d)[:-1]
      else:
        d=eval(text)
        text= "0x%x"%(d)
    except:
      print 'except'
      pass
    return text
  def printEntry(self, text='MywEntry.printEntry:'):
    print text,self.getEntry()
  #def destroyEntry(self):
  def destroy(self):
    if self.label: self.label.destroy()
    if self.entry:
      self.entry.destroy()
    #self.Frame.destroy() nebavi
    #self.destroy()

class MywVMEEntry(MywEntry):
  """ See MywEntry. This class in addition to MywEntry:
  - is initialised by VME value (VME read in __init__)
  - when entry field modified, VME register vmeaddr is updated 
    when: mouse cursor leaves the entry, ENTER is pressed 
    or when label button pressed
  - userupdate(oldval,newval) method: converts the string given by user to
    hex/dec number to be written into vme
  """
  def __init__(self, master, label='label',vmeaddr='VMEADDR',vb=None,
    side='left', helptext="", width=None, userupdate=None):
    self.vb= vb
    self.userupdate= userupdate
    self.vmeaddr= vmeaddr
    self.getvme()
    MywEntry.__init__(self, master, label=label, defvalue=self.vmeval,
    side=side, helptext=helptext, cmdlabel=self.updateentry, width=width,
    textvariable=None, delaction=None,defaultinx=0,name=None)
    self.entry.bind("<Leave>", self.updateentry)
    self.entry.bind("<Key-Return>", self.updateentry)
  def getvme(self):
    if self.vb:
      self.vmeval= self.vb.io.execute("vmeopr32("+self.vmeaddr+")")[:-1]
    else:
      print "MywVMEEntry.getvme(): vb not supplied, returning 0x55aa"
      self.vmeval="0x55aa"
  def updateentry(self, event=None):
    newentry= self.getEntry()
    #print "MywVMEreg:",newentry
    if newentry == self.vmeval: return
    if self.userupdate:
      strforvme= self.userupdate(self.vmeval, newentry)
    else:
      #strfotvme= str(self.vmeval)
      strforvme= newentry   #.split()[0] only 1st part from '234 ~23us' perhaps later
    self.vmeval= newentry
    #print "MywVMEreg:updating",newentry
    if self.vb:
      self.vb.io.execute("vmeopw32("+self.vmeaddr+", "+
        strforvme+")")
    else:
      print "MywVMEEntry.updateentry(): vb not supplied, writing ", self.vmeval
    
class MywButton(Button, MywHelp):
  """
  If cmd is not supplied, the button appearance resembles Label
  widget. See MywLabel for 'true' label widget.
  label -used as positional parameter sometimes (2nd one)
  state:
  """
  def __init__(self, master, label='butlabel', cmd=None, state=None,
    bg=None,side=None,anchor='center',helptext=None,expand='yes',
    fill='x', relief=RAISED):
    # zda sa ze nemusi byt self.but:
    #but= Button(master, text=label, command=cmd, bg=bg, activebackground=bg)
    #but.pack(side=side, expand='yes', fill='x')
    Button.__init__(self,master,text=label, command=cmd, bg=bg, 
      relief=relief,state=state, anchor=anchor)
    self.normalcolor= Button.cget(self,'background'),\
      Button.cget(self,'activebackground')
    self.label=label
    #print "MywButton:",abg,pbg
    if cmd == None:
      Button.configure(self,activebackground=self.normalcolor[0],takefocus=0)
    Button.pack(self,side=side, expand=expand, fill=fill) #, anchor=anchor)
    #if funcdescr and funcdescr[VmeBoard.NFUSAGE]:
    #  MywHelp.__init__(self,master,funcdescr[VmeBoard.NFUSAGE])
    if helptext: MywHelp.__init__(self,master, helptext)
    #print "MywButton:",self.helptext
  def disable(self):
    self.config(state=DISABLED)
  def enable(self):
    self.config(state=NORMAL)
  def setBackground(self, color):
    Button.configure(self,background=color)
    #Button.configure(self,activebackground=color)
  def setColor(self, color):
    self.setBackground(color)
  def resetColor(self):
    #self.checkbut.config(bg=self.normalcolor[0])
    Button.configure(self,bg=self.normalcolor[0])
  def getLabel(self):
    return(self.cget('text'))
  def setLabel(self,name):
    self.configure(text=name)
class MywFrame(Frame):
  def __init__(self, master=None, borderwidth=1, relief=RAISED, side=None,
      bg=None,name=None,expand='yes',fill='x'):
    Frame.__init__(self,master, borderwidth=borderwidth, relief=relief,
      bg=bg,name=name);
    Frame.pack(self,side=side, expand=expand, fill=fill)
    #if helptext: MywHelp.__init__(self,master, helptext)

class MywHButtons(MywFrame):
  """
  Creates more buttons in 1 line. Parameters:
  buttons: list of [label, command, arg] items where:
    label:   the button label
    command: command to be executed after the button press
    arg:     parameter supplied to command (optional).
  """
  def __init__(self, master, buttons, side=BOTTOM,helptext=None):
    MywFrame.__init__(self,master, side=side);
    self.buttons=[]
    for i in range(len(buttons)):
      but=buttons[i]
      #self.buttons.append(MywButton(master, label=but[0], cmd=but[1], side=LEFT))
      if len(but)>2:
        self.buttons.append(MywButton(self, label=but[0], 
          cmd=curry(but[1],but[2]), side=LEFT, helptext=helptext))
      else:
        self.buttons.append(MywButton(self, label=but[0], 
          cmd=but[1], side=LEFT, helptext=helptext))
    MywFrame.pack(self,side=side, expand='yes', fill='x')
    #addregbut= MywButton(self.mrmaster, label='add reg.', 
    #  cmd=self.addReg, side='bottom')
  def destroy(self):
    for b in self.buttons:
      b.destroy()

class MywMenubutton(Menubutton, MywHelp):
  def __init__(self, master=None, label='noname',helptext=None,side=TOP,
    state=None, bg=None, width=None,expand='yes', fill='x',
    relief=RAISED,anchor='center'):
    Menubutton.__init__(self,master,text=label,state=state,
      relief=relief, bg=bg,anchor=anchor, width=width)
    Menubutton.pack(self,side=side, expand=expand, fill=fill)
    if helptext: MywHelp.__init__(self,master,helptext)

class MywMenu(Menu, MywHelp):
  def __init__(self,master=None,helptext=None,cmd=None):
    Menu.__init__(self, master, postcommand=cmd, relief=RAISED)
    if helptext: MywHelp.__init__(self,self,helptext)
    #self.master=master
    self.ccs=[]
    self.ixchoosen=None
  def addcascade(self,cname, columnbreak=False):
    self.ccs.append(MywMenu())
    self.add_cascade(label=cname, menu=self.ccs[-1], columnbreak=columnbreak)
    return self.ccs[-1]
  def addcommand(self,cname, cmd,disabled=None, columnbreak=False):
    self.ccs.append([cname,cmd])
    ix= len(self.ccs)
    self.add_command(label=cname, command=curry(cmd,self,ix), columnbreak=columnbreak)
    if disabled:
      self.disable(ix)
  def setcommand(self,cname, cmd,disabled=None):
    ix= self.findLabel(cname)
    #print "setcommand:",cname,ix
    self.entryconfigure(ix, command=curry(cmd,self,ix))
    #self.entryconfigure(ix, command=cmd)
    if disabled:
      self.disable(ix)
  def findLabel(self, label):
    for i in range(1,self.index(END)+1):
       l= self.entrycget(i, "label")
       #print "MywMenu.findLabel:",l
       if label== l:
         return i
    return None
  def disable(self, iix):
    self.enabdisa(iix, DISABLED)
  def enable(self, iix):
    self.enabdisa(iix, NORMAL)
  def enabdisa(self, iix, state):
    if type(iix) is types.IntType:
      self.entryconfigure(iix,state=state)
    elif type(iix) is types.ListType:
      for x in iix:
        if type(x) is types.IntType:
          indx= x
        elif type(x) is types.StringType:
          indx= self.findLabel(x)
        else: continue
        if indx: self.entryconfigure(indx,state=state)

#class MywMBar(Menu, MywHelp):
#  def __init__(self, master=None

class MywBits:
  """
  bits[] -array of items corresponding to bits 0,1,2,...,31
          1 item is:
          -simple string (label naming the corresponding bit) or
          -tuple ("label",0)   ->corresponding bit is DISABLED i.e. 
             GUI-read/only, but allowed to be changed from program
          -tuple ("label",1)   ->corresponding bit is not shown
  cmd    -callback activated after any modification by mouse
  defval -default value. If changed programmably, call setEntry() to
          upgrade the menu
  allro  -'yes' -> all bits are DISABLED
  """
  def __init__(self, master=None, helptext=None, defval=0,
    side=TOP, label="?", cmd=None, relief=RAISED, bg=None,
    allro=None,
    bits=["PP Pre-pulse", "L0", "L1", "L1m L1 message"]):
    self.ilabel=label
    self.cmd=cmd
    self.bits=bits
    #print "MywBits:type:",type(defval), defval
    self.value= defval
    #self.value= eval(defval)
    self.mb= MywMenubutton(master, label=self.ilabel,side=side,
      helptext=helptext, relief=relief, bg=bg, anchor='w');
    self.normalcolor= self.mb.cget('background'),\
      self.mb.cget('activebackground')
    self.menu= MywMenu(self.mb,   #cmd=self.hereiam,
      helptext=helptext)
    #self.menu.config(tearoff=0)
    self.cv=[]
    for bitn in range(0,len(self.bits)):
      self.cv.append(IntVar())
      chb= self.bits[bitn]
      endistate= None; shown=1
      if type(chb) is types.TupleType:
        if chb[1]==0: endistate= DISABLED
        if chb[1]==1: shown=None
        chb= chb[0]
      else:
        if allro=='yes': endistate= DISABLED
      if self.value & (1<<bitn): self.cv[bitn].set(1)
      else: self.cv[bitn].set(0)
      if shown:
        self.menu.add_checkbutton(label=chb, command=self.mcmd,
        variable=self.cv[bitn], state=endistate)  
    self.mb.config(menu=self.menu)
    self.dolabel()
  #def hereiam(self):
  #  print "hereiam:",self.bits
  def mcmd(self):
    #print "MywBits mcmd:"
    self.value=0
    for ix in range(len(self.cv)):
      cv= self.cv[ix].get()
      #print ix,":",self.menu.entrycget(ix,'label'),cv
      if cv==1: self.value= self.value | (1<<ix)
    #print "END:",self.menu.index(END)
    self.dolabel()
    # activate user callback (if present)
    if self.cmd: self.cmd()
  def dolabel(self):
    lbl=self.ilabel
    if self.ilabel!='':
      lbl=lbl+':'
    for bitn in range(0,len(self.bits)):
      chb= self.bits[bitn]
      #endistate= None
      if type(chb) is types.TupleType:
        #if chb[1]==0: endistate= DISABLED
        if chb[1]==1: continue   #not shown
        chb= chb[0]
      #if self.value & (1<<bitn):
      if  self.cv[bitn].get():
        lbl= lbl+' '+string.split(chb,None,2)[0]
      #print "dolabel:",bitn,self.cv[bitn].get(),lbl
    self.mb.config(text=lbl)
  def setEntry(self, val):
    #print "setEntry:",val
    self.value= val
    #self.value= eval(val)
    for bitn in range(0,len(self.cv)):
      chb= self.bits[bitn]
      if self.value & (1<<bitn):
        self.cv[bitn].set(1)
      else:
        self.cv[bitn].set(0)
    self.dolabel()
  def getEntry(self):
    #print "MywBits value:%x"%(self.value)
    return self.value
  def getEntryBin(self):
    return self.value
  def destroy(self):
    self.mb.destroy()
    self.menu.destroy()
  def setColor(self, color=None):
    if color==None:
      color= self.normalcolor[0]
    self.mb.configure(bg=color)
    #self.label.configure(bg=color)

class MywMenuList:
  '''
  Button expanding to checkbutton list of buttons.
  label: optional. The label of this menu. If showactive==1, label
         is automatically modified with activating checkbutton:
          label:active1 active2 ...
  items: list of items. 1 item is a string 
         or list of 2 (0,1) items:
    0   -text label (to be displayed), 
    1   -optional. Function to be called, when this entry choosen,
         i.e. set/unset (not implemented for tear off menus)
         This function is activated before cmd (if both present)
  defaults: list of 0/1 for default items to be set
  cmd: function called when any entry changed. 2 parameters
       are passed: self pointer to current MywMenuList instance and
                   index pointing to item which was just
                   modified (set/reset)
  '''
  def __init__(self, master=None,label=None, cmd=None,
    helptext=None, defaults=None, side= 'bottom', state=None,
    bg=None,
    showactive=None, items=("text1","text2")):
    self.showactive=showactive
    self.items=items
    self.master= master
    self.cmd=cmd
    self.onoff=[]
    for i in range(len(self.items)):
      self.onoff.append(0)
    if defaults:
      if (type(defaults) is types.ListType) or (type(defaults) is types.TupleType):
        for i in range(len(defaults)):
          if defaults[i]!=0:
            self.onoff[i]= 1
    self.posval=[]
    for i in range(len(self.onoff)):
      self.posval.append(IntVar()); 
      self.posval[i].set(self.onoff[i])
    #
    if label != None:
      labeltxt=label
    else:
      labeltxt=self.items[0]
    self.name=labeltxt
    self.checkbut= MywMenubutton(self.master, helptext=helptext,
      label=labeltxt, state=state, side=side, bg=bg)
    self.normalcolor=self.checkbut.cget('bg'),\
      self.checkbut.cget('activebackground')
    self.rbm= Menu(self.checkbut)
    ixitem=0
    for ch in self.items:
      ixitem= ixitem+1
      if ixitem%20 == 0 and ixitem>0:
        cbreak=1
      else:
        cbreak=None
      #print 'onoff:',self.onoff,ixitem
      if type(ch) is types.StringType:
        chname=ch
      else:
        chname=ch[0]
      if chname=='-':
        self.rbm.add_separator()   # position counted!
      else:
        self.rbm.add_checkbutton(label=chname,
          variable=self.posval[ixitem-1], command=self.modifname, 
          columnbreak=cbreak)
    self.checkbut['menu']= self.rbm
    if self.showactive: self.dolabel()
    #
    #bad self.rbm.entryconfigure(defaultinx+1, "ACTIVE")
    #self.rbm.invoke(defaultinx+1)
  def modifname(self):
    #print "ACTIVE:",self.rbm.index(ACTIVE)
    inx= self.rbm.index(ACTIVE)
    if inx!=None:
      #print "act. label:",self.rbm.entrycget(self.rbm.index(ACTIVE),'label')
      inx=inx-1
      self.onoff[inx]= self.getEntry(inx)
    else:
      # teared off menu (ACTIVE doesn't work -is None for teared-off menu):
      #raise NameError, "tearoff not implemented for MywMenuList"
      ndiff=0    # number of diffrent items (has to be 1)
      #curv= self.getEntry()
      for ix in range(1, self.rbm.index(END)+1):
        #print 'modifname,ix,label:',ix,":",self.rbm.entrycget(ix,'label')
        cvar= self.getEntry(ix-1)
        #print 'state,value:', self.rbm.entrycget(ix,'state'),cvar
        if self.onoff[ix-1]!= cvar:
          inx=ix-1; ndiff= ndiff+1
          self.onoff[ix-1]= cvar
          if ndiff>1:
            print "Interror in torn off MywMenuList (ndiff:%d ix:%d)"%(ndiff,ix)
      if ndiff != 1:
        raise "Internal error in torn off MywMenuList (ndiff:%d ix:%d)"%(ndiff,ix)
    #if self.getEntry(inx)==1:
    #  self.setColor("RED")
    #else:
    #  self.resetColor()
    pvg= self.posval[inx].get()
    #print 'pvg:',pvg
    #self.checkbut['text']= self.items[inx][0]
    if type(self.items[inx]) is types.StringType:
      pass
    else:
      #print "calling action for ",self.items[inx][0]
      self.items[inx][1]()
    if self.cmd:
      #print "calling global action for ",self.items[inx]
      self.cmd(self, inx)
    if self.showactive: self.dolabel()
    return
  def dolabel(self):
    lbl= self.name+':'
    for ich in range(len(self.items)) :
      if self.posval[ich].get():
        if type(self.items[ich]) is types.StringType:
          chname=self.items[ich]
        else:
          chname=self.items[ich][0]
        lbl= lbl+' '+chname
    self.checkbut.config(text=lbl)
  def getLabel(self):
    return self.checkbut.cget("text")
  def setLabel(self, newlabel):
    return self.checkbut.config(text=newlabel)
  def ons(self):
    """
    return number of check buttons switched ON
    """
    non=0
    for index in range(len(self.items)):
      if self.posval[index].get()==1: non=non+1
    return non 
  def enable(self):
    self.checkbut.config(state=NORMAL)
  def disable(self):
    self.checkbut.config(state=DISABLED)
  def setColor(self, color):
    self.color=color
    self.checkbut.config(bg=self.color)
    #self.ButConnect.setBackground(color=self.color)
  def resetColor(self):
    self.checkbut.config(bg=self.normalcolor[0])
  def getEntry(self, index):
    #print "butm:", self.posval.get()
    return self.posval[index].get()  
  def getItem(self, index):
    return self.items[index]
  def setEntry(self, index, val):
    #print "MywMenuList.setEntry",index,":", self.posval[index].get(),'->',val
    return self.posval[index].set(val)  
  #def printEntry(self, text='MywMenuList.printEntry:'):
  #  print text,'ix:',self.index,'Entry:',self.getEntry()
  #def popup(self, event):
  #  self.rbm.post(event.x_root, event.y_root)
  #def unpopup(self, event):
  #  self.rbm.unpost()
  def destroy(self):
    self.checkbut.destroy()   # garbage collector does the rest...

class MywxMenu:
  '''
  Button expanding to radio-type menu.
  label: optional. The label of this menu
  delaction(): to be supplied only if one of the items[] contains
    item with value "removevalue" -action to be done, when 
    this item is selected
  items: list of items. 1 item is a string 
         or list of 2 or 3 values (0,1,2):
    0   -text label (to be displayed), 
    1   -text value (can be obtained by getEntry(), 'removevalue' has
                     special meaning)
    2   -optional. Function to be called, when this entry choosen.
         This parameter is ignored, if cmd supplied
  cmd: function to be called when modification done (activated
       before items[2] functions if any). cmd is called
       with 2 arguments: this instance and the index (0..) of choosen item
  defaultinx: index of item in items[] to be taken as default
  '''
  def __init__(self, master=None,label=None, delaction=None,
    helptext=None, defaultinx=0, side= 'bottom', state=None, cmd=None,
    width=None, bg=None,
    items=[("text1","1"),("text2","2")]):
    self.label=label   # None (was until 15.1.2006)
    self.items=[]
    for itm in items:
      if type(itm) is types.StringType:
        self.items.append((itm,itm))
      else:
        self.items.append(itm)
    self.master= master
    self.cmd= cmd
    self.delaction= delaction
    if defaultinx>=len(items):
      print "myw.MywxMenu: deafaultinx too high:",defaultinx, "setting 0"
      defaultinx=0
    self.index=defaultinx
    #print "MywxMenu:items:", items, defaultinx
    self.posval= StringVar(); self.posval.set(self.items[defaultinx][1])
    self.posvalbefore= self.posval.get()
    self.posinx= defaultinx
    #
    self.radf=Frame(master, borderwidth=0)
    expandentry='yes'
    self.radf.pack(side=side, expand=expandentry, fill='x')
    if label != None:
      #1711self.label=MywEntry(self.radf,label=label, defvalue='', helptext=helptext,side=LEFT)
      self.label=MywLabel(self.radf,label=label, helptext=helptext,side=LEFT,
        bg=bg, expand='no', fill='y')
    #self.radiobut= Menubutton(self.radf, text=self.items[defaultinx][0])
    self.radiobut= MywMenubutton(self.radf, helptext=helptext,
      label=self.items[defaultinx][0], state=state,bg=bg,
      expand=expandentry, fill='x',
      width=width) 
    #self.radiobut.config(bg=bg)
    #self.radiobut.pack(side=RIGHT)
    self.rbm= Menu(self.radiobut)
    ixitem=0
    for ch in self.items:
      ixitem= ixitem+1
      if ixitem%20 == 0 and ixitem>0:
        cbreak=1
      else:
        cbreak=None
      self.rbm.add_radiobutton(label=ch[0], value=ch[1],
        variable=self.posval, command=self.modifname, columnbreak=cbreak)
    self.radiobut['menu']= self.rbm
    #
    #bad self.rbm.entryconfigure(defaultinx+1, "ACTIVE")
    #self.rbm.invoke(defaultinx+1)
  def modifname(self):
    #print "ACTIVE:",self.rbm.index(ACTIVE),"posval:",self.posval #or END
    inx= self.rbm.index(ACTIVE)
    if inx:
      #print "act. label:",self.rbm.entrycget(self.rbm.index(ACTIVE),'label')
      inx=inx-1
    else:
      # teared off menu (ACTIVE doesn't work -is None for teared-off menu):
      inx=None; curv= self.getEntry()
      for ix in range(1, self.rbm.index(END)+1):
        #print 'curv:',curv,ix,":",self.rbm.entrycget(ix,'label')
        if curv==self.items[ix-1][1]: inx=ix-1
    self.index= inx   # keep it for getIndex()
    txt="internal error"
    pvg= self.posval.get()
    if pvg == "removevalue":
      #print "MywxMenu: remove me",self.master
      #bavi (iba label) self.radf.destroy()
      #bavi (aj entry removne) self.master.destroy()
      #bavi (aj frame o vsetkymy regs emovne) self.master.master.destroy()
      #?bad idea
      self.master.after_idle(self.delaction, self.master)
    #elif pvg != self.posvalbefore:
    else:
      self.posvalbefore= pvg
      self.posinx= inx
      self.radiobut['text']= self.items[inx][0]
      if self.cmd:
        self.cmd(self, inx)
      elif len(self.items[inx])>2:
        #print "calling action for ",self.items[inx][0]
        self.items[inx][2]()
    return
  def backEntry(self):
    self.posval.set(self.posvalbefore)
    print "backEntry:",self.posval.get(), self.posinx
    self.radiobut['text']= self.items[self.posinx][0]
    #still makeactive:
  def enable(self):
    self.radiobut.config(state=NORMAL)
  def disable(self):
    self.radiobut.config(state=DISABLED)
  def getEntry(self):
    #print "butm:", self.posval.get()
    return(self.posval.get()) 
  def setEntry(self, newactiveix):
    #print 'setEntry:',newactiveix
    self.posvalbefore= self.posval.get()
    self.posinx= newactiveix
    self.radiobut['text']= self.items[newactiveix][0]
    self.posval.set(newactiveix)
  def getIndex(self):
    """ returns index of active item (0,1,...) in supplied items
    """
    return(self.index)
  def printEntry(self, text='MywxMenu.printEntry:'):
    print text,'ix:',self.index,'Entry:',self.getEntry()
  #def popup(self, event):
  #  self.rbm.post(event.x_root, event.y_root)
  #def unpopup(self, event):
  #  self.rbm.unpost()
  def destroy(self):
    self.radf.destroy()   # garbage collector does the rest...
  def setColor(self, color):
    #self.color=color
    self.radiobut.config(bg=color)
    #self.ButConnect.setBackground(color=self.color)
  def resetColor(self):
    #self.normalcolor= COLOR_BGDEFAULT
    #self.normalcolor= Button.cget(self,'background'),\
    #  Button.cget(self,'activebackground')
    self.radiobut.config(bg=COLOR_BGDEFAULT)
  def getLabel(self):
    return self.label.cget("text")
class MywLabel(Label,MywHelp):
  def __init__(self, master=None, label=None, helptext=None,
      borderwidth=0, width=None, relief=RAISED, fill='x',
      side=None, bg=None, expand='yes',anchor='center'):
    Label.__init__(self,master, text=label,borderwidth=borderwidth, 
      width=width,relief=relief,bg=bg, anchor=anchor);
    Label.pack(self,side=side, expand=expand, fill=fill)
    #1711Label.pack(self,side=side, expand=expand, fill='y')
    if helptext: MywHelp.__init__(self,master, helptext)
  def label(self, newlabel):
    self.configure(text=newlabel)
  def getLabel(self):
    return self.cget("text")
  def setColor(self, color):
    #self.color=color
    self.configure(bg=color)

class FunStart:
  def fcallstart(self):
    cmdline=self.fn+"("
    #print "fcallstart:"
    if len(self.ient)>0:
      for e1 in self.ient:
        #print 'FunStart.cmdline:',cmdline
        cmdline=cmdline+e1.getEntry()+","
    else:
      cmdline=cmdline+" "
    #if len(self.ient)>0:
    cmdline=cmdline[:-1]+")"
    #else:
    #  cmdline=cmdline+")"
    if self.vb.io == None: self.vb.openCmd()
    #self.vb.io.cmd.setEntry(cmdline); 
    self.vb.io.execute(cmdline)
  def fcallstop(self):
    if self.inxMenu:
      self.inxMenu.entryconfigure(self.ginx, state='normal')
    else:
      self.inx.configure(state='normal')
    self.rwf.destroy()
  def __init__(self, vb, fnix):
    """
    vb    -VmeBoard instance
    fnix  - index info vb.funcs
    """ 
    #print "FunStart:", fnix," ",vb.funcs[fnix], vb.funcsm[fnix]
    # vb.funcs[fnix]: [GroupName,HelpText,FunType,FunName,
    #                   [[ParName1,ParType1,IdirectFlag],...]]
    #print "FunStart.vb:", vb.boardName,vb.funcsm
    self.fn= vb.funcs[fnix][VmeBoard.NFNAME]
    self.vb= vb
    self.ient= []
    #inx   - index (1..) in corresponding Menu, or pointer to Button:
    if vb.funcs[fnix][0] == None:   #button (no group func)
      self.inxMenu= None
      self.inx= vb.ngf[vb.funcsm[fnix][0]]
    else:
      #print "entrycget:",vb.gmbs[vb.funcsm[fnix][0]]
      self.inx= vb.funcsm[fnix][0]
      self.ginx= vb.funcsm[fnix][1]
      self.inxMenu= vb.gmbs[self.inx*2+1]
      # following returns None if menu was torn off:
      #self.inx= self.inxMenu.index(ACTIVE)
      #print "entrycget ",self.inx," ",self.inxMenu.entrycget(self.inx,"label")
      #print "entrycget ACTIVE ",self.inxMenu.entrycget(ACTIVE,"label")
    #print "FunStart...",self.inx, self.inxMenu
    if len(vb.funcs[fnix][VmeBoard.NPARDESC])==0:
      #self.ient.append(MywEntry(master=self.rwf,label='help', defvalue='',
      #  side='left', helptext=vb.funcs[fnix][VmeBoard.NFUSAGE]))
      self.fcallstart()   
    else:
      self.rwf=Toplevel(vb.vmebf)
      cr= getCrate()
      if cr: self.rwf.group(cr.master)
      self.rwf.title(self.vb.boardName+'.'+self.fn)
      # following not OK if menu was torn off:
      #self.inxMenu.entryconfigure(ACTIVE, state='disabled')
      if self.inxMenu:
        self.inxMenu.entryconfigure(self.ginx, state='disabled')
      else:
        self.inx.configure(state='disabled')
      for ip in vb.funcs[fnix][VmeBoard.NPARDESC]:
        #print "PAR",ip
        pname=ip[VmeBoard.NPARDESCNAME]
        dflval=" "
        if ip[VmeBoard.NPARDESCTYPE]=='char' and ip[VmeBoard.NPARDESCIND]=="*":dflval="\" \""
        self.ient.append(MywEntry(master=self.rwf,label=pname, 
          side='top', defvalue=dflval,
          helptext=vb.funcs[fnix][VmeBoard.NFUSAGE]))
      startbut= MywButton(self.rwf, label='start', cmd=self.fcallstart, side='left')
      okbut= MywButton(self.rwf, label='quit', cmd=self.fcallstop, side='left')

# this can be used only through VmeBoard (boardmod)
class MywMenu81632(MywxMenu):
  def modifname(self):
    MywxMenu.modifname(self)
    #print "ACTIVE:",self.rbm.index(ACTIVE)       #or END
    self.boardmod.modexit()
    #self.radiobut['text']= self.rbm.entrycget(
    #  self.rbm.index(ACTIVE),'label')

class VmeBoard:
  NGUINAME=5
  NPARDESC=4
  NFNAME=3
  NFTYPE=2
  NFUSAGE=1
  NFGROUP=0
  NPARDESCNAME=0
  NPARDESCTYPE=1
  NPARDESCIND=2
  CTPcolors={"busy":"#ccff00", "l0":"#cc9933","l1":"#cc33cc",
    "l2":"#ccff99", "fo":"#cccccc","int":"#ccffff"}
  LTUcolor="green"
  def ZNCdestroybf(self, event):
    #try:
    #  print "destroybf ", event.widget, ":",self.io.f, ":", self.io.tp
    #except:
    #  print "exception ", sys.exc_info()[0]
    #if event.widget == self.io.f:
    #  print "OK, cmdlin2 widget destroyed"
    del self.io; self.io= None; self.ButConnect.configure(fg="black")
  def delio(self):
    self.io= None; 
    try:  #if destroyed by X window manager, ButConnect doesn't have to exist 
      if self.ButConnect:
        self.ButConnect.configure(fg="black")
    except:
      #print "TclError:",self.ButConnect
      pass
  def openCmd(self):
    """ Start 'board interface'
    - popen2
    - create top level window for stdin/stdout
    """
    import cmdlin2
    #print "here openCmd ",self.boardName
    #self.ButConnect.configure(state="disabled")
    if self.io==None:
      bdir= os.environ['VMECFDIR']
      if self.iamltu('dim'):
        import platform
        if platform.machine()=="x86_64":
          exdir= "lin64"
        else:
          exdir= "linux"
        cm= os.path.join(bdir, "ltudim",exdir,"ltuclient")+" " + self.boardName
      else:
        cm= os.path.join(bdir, self.boardName, self.boardName)
        cm= cm+'.exe '+self.baseAddr
      #print "openCmd:",cm,":"
      self.io= cmdlin2.cmdlint(cmd=cm, board=self)
      if self.io.thds[0].pidexe==None:
        #raise "exiting..."
        self.master.after(3000, self.exit)
      #self.io.tp.bind("<Destroy>", self.destroybf)
      self.ButConnect.configure(fg="red")
    else:
      #self.io.stop()
      #del self.io; self.io= None; self.ButConnect.configure(fg="black")
      #print "myw:openCmd:nop"  
      self.io.execute("nop",ff= self.emptyfun)
  def exit(self):
    #print "VMeBoardexit:"
    sys.exit(4)
  def emptyfun(self,txt):
    pass
    #print 'emptyfun:',txt,'---'
  def funvmerw(self):
    multregs= VmeRW(self)
  def multreg(self):
    multregs= MultRegs(self)
  def findvmeregtyp(self, reg):
    tx="badregtype"
    for rg in self.vmeregs:
      if reg==rg[0]:
        if rg[2] == '': tx="w32"
        if rg[2] == '0x04000000': tx="w16"
        if rg[2] == '0x02000000': tx="w8"
        break
    return(tx)
  def startFun(self, fnix):
    fn= self.funcs[fnix][VmeBoard.NFNAME]
    #print "here startFun F:", fn, "fnix:", fnix
    if self.funcs[fnix][VmeBoard.NFTYPE]=='GUI':
      if self.iamltu():
        modname= "ltu_u"
      else:
        modname= self.boardName+"_u"
      #print modname
      #__import__(modname)
      exec('import '+modname)
      #print 'cmdeval:',modname,':',fn
      # let self.io is always defined for user graphics:
      if self.io== None: self.openCmd()                 
      exec('rc='+modname+'.'+fn+'(self)')
      #print "startFun:rc:",rc
      #rc.hereiam()
    else:
      #znc self.fust.append(FunStart(self, fnix))
      FunStart(self, fnix)
  def funbuts(self):
    #print "here funbuts"
    #self.funcs= [[None, '','w32', 'init', [['preg2', 'w32', '']]], [None, 'usagehelp','void', 'Doit', []], ['MBcard','usghlp', 'void', 'setGlobalReg', [['reg', 'int', ''], ['val', 'w32', '*']]]]
    self.groups=[]
    for f in self.funcs:
      gn=f[VmeBoard.NFGROUP]
      if gn in self.groups or gn==None:
        continue
      self.groups.append(gn)
    # order numbers of the func in ngf (N,0) or gmbsi (N,I) I->order number
    # in correponding menu:
    self.funcsm={}
    #--------------------- TOP (or no GROUPsfuncs):
    self.ngf=[]
    for ix in range(len(self.funcs)):
      if self.funcs[ix][VmeBoard.NFGROUP] == None:   # TOP fun
        if len(self.funcs[ix])>=(VmeBoard.NGUINAME+1):
          fn= self.funcs[ix][VmeBoard.NGUINAME]
        else:
          fn= self.funcs[ix][VmeBoard.NFNAME]
        cmdh= lambda s=self,x=ix:s.startFun(x)
        self.ngf.append(MywButton(self.vmebf, label=fn,
          # funchelp -help for just this function
          cmd= cmdh, side='top', helptext=self.funcs[ix][VmeBoard.NFUSAGE]))
        # order number (0,...) in correspondig/top group
        self.funcsm[ix]= (len(self.ngf)-1,0)
        #print "funbuts1:",fn,"self.funcm[",ix,"]=",self.funcsm[ix]
    #---------------------- GROUP functions:
    self.gmbs=[]
    for gn in self.groups:
      if string.find(self.hiddenfuncs," "+gn)!=-1: continue
      # funchelp -for just this group (see comp.py, self.groups)
      self.grouphelp={}
      self.grouphelp[gn]=''
      for ix in range(len(self.funcs)):
        if gn == self.funcs[ix][VmeBoard.NFGROUP]:
          self.grouphelp[gn]= self.grouphelp[gn]+\
            '========== '+\
            self.funcs[ix][VmeBoard.NFNAME]+'\012'+\
            self.funcs[ix][VmeBoard.NFUSAGE]
        if self.grouphelp[gn] and (self.grouphelp[gn][-1] != '\012'):
           self.grouphelp[gn]= self.grouphelp[gn] + '\012'
      self.gmbs.append(MywMenubutton(self.vmebf, label=gn,\
        helptext=self.grouphelp[gn]))
      #
      #prepare help from all the helps in the group:
      self.gmbs.append( MywMenu(self.gmbs[-1],
        helptext=self.grouphelp[gn]))
      self.gmbs[-1].config(tearoff=1)
      #self.gmbs[-2]["menu"]= self.gmbs[-1]    alebo:
      self.gmbs[-2].config(menu= self.gmbs[-1])
      gix=0   # func. number in corresponding group (for disable/normal state)
      # not working properly?
      # named groups (not TOP):
      for ix in range(len(self.funcs)):
        if gn == self.funcs[ix][VmeBoard.NFGROUP]:
          #print "funbuts1:",gn
          if len(self.funcs[ix])>=(VmeBoard.NGUINAME+1):
            labname= self.funcs[ix][VmeBoard.NGUINAME]
          else:
            labname= self.funcs[ix][VmeBoard.NFNAME]
          #print "funbuts2:",self.funcs[ix],labname
          # lambda isn't correct for tear off menu
          cmdh= lambda s=self,x=ix:s.startFun(x)
          self.gmbs[-1].add_command(label=labname, command=cmdh)
          gix=gix+1
          self.funcsm[ix]= (len(self.gmbs)/2-1, gix)
          #print "funbuts3:",fn,"self.funcm[",ix,"]=",self.funcsm[ix]
  def getcf(self):
    self.funcs=[]
    if self.iamltu():
      usedir="ltu"
    else:
      usedir=self.boardName
    try:
      bdir= os.path.join( os.environ['VMECFDIR'],
        usedir, usedir)
      cff= bdir+'_cf.py'
      execfile(cff)
      # self.baseAddr,spaceLength,vmeregs,funcs,hiddenfuncs
    except:
      raise usedir+'_cf.py'+" doesn't exist or incorrect"
    else:
      #cff.close()
      pass
  def delBoard(self):
    if self.io:
      self.io.stop()
    self.vmebf.destroy()
    self.ButConnect=None
  def setColor(self, color):
    if color=="original": color=self.color
    self.vmebf.configure(bg=color)
    self.ButConnect.setBackground(color=color)
  def iamltu(self, dim=None):
    """
    self.boardName  dim  rc
    -----------------------
    ltu            'dim' False
    ltu            None  True
    hmpid           -    True
    blbina          -    ZDOCHNE
    """
    if DimLTUservers.has_key(self.boardName): 
      return True
    elif self.boardName=='ltu':
      if dim=='dim':
        return False
      else:
        return True
    return False
  def __init__(self, crate=None, boardName="abc", baseAddr="",color=None,
    initboard="yes"):
    #Frame.__init__(self, master, borderwidth=3)
    self.crate= crate
    self.master=crate.crframe;
    self.boardName= boardName
    self.initboard= initboard
    self.vmeregs=None; 
    self.getcf()   # self.(vmeregs[],baseAddr,spaceLength,funcs[])
    self.baseAddr= baseAddr   # has to be after getcf()
    #if baseAddr != "": self.baseAddr= baseAddr   # has to be after getcf()
    self.fungrps=[]

    if color==None:
      if self.iamltu(): color=VmeBoard.LTUcolor
      elif boardName=='ttcvi': color="#66cc00"
      elif VmeBoard.CTPcolors.has_key(boardName):
        color= VmeBoard.CTPcolors[boardName]
      else: color="#ff9900"
      
    self.color= color
    self.io= None   #user interace not opened
    # create work directory if not existing -only for LTU:
    if self.iamltu():
      #if self.baseAddr[0:6]=='VXI0::':
      #  self.workdir= os.path.join( os.environ['VMEWORKDIR'],"WORK", self.boardName+self.baseAddr[6:])
      #else:
      self.workdir= os.path.join( os.environ['VMEWORKDIR'],"WORK")
      #  self.boardName+self.baseAddr)
      if os.access(self.workdir, os.W_OK)!=1:
        try:
          os.mkdir(self.workdir) 
        except:
          print "Cannot create work directory:", self.workdir
          print "exception:", sys.exc_info()[0]
    #self.vmefuns= {}
    self.vmebf= Frame(self.master, borderwidth=4, relief="raised",bg=color)
    self.vmebf.pack(side="left", expand='yes', fill='y')
    self.ButConnect= MywButton(self.vmebf, bg=color, \
      label=boardName+' '+self.baseAddr, \
      cmd= self.openCmd, side="top", helptext="Base addr: "+self.baseAddr+"""
This button starts main log/cmd window 
if it is not started yet.
Main log/cmd window is started by itself if necessary.
""") 
    # standard funcs:
    self.ButFunctions= MywMenubutton(self.vmebf, label="Stdfuncs",
      helptext="access to VME registers") 
    self.FunMenu= Menu(self.ButFunctions)
    #self.ButFunctions.config(menu= self.FunMenu)
    self.ButFunctions['menu']= self.FunMenu
    self.FunMenu.add_command(label="VME r/w", command=self.funvmerw)
    #self.FunMenu.add_command(label="multiregs", command=self.multreg)
    """ attempt to solve problem in cygwin by replacing with MywxMenu:
        which didn't rectify the behaviour after opening ioWindow
    self.ButFunctions= MywxMenu(self.vmebf, label="Stdfuncs",
      items=[["VME rw", "0", self.funvmerw],
        ["Multiregs", "1", self.multreg]],
      helptext="access to VME registers") 
    """
    #
    # user funcs:
    self.funbuts()
    if vbexec==None:
      self.openCmd() ; vbinit(self)
    if self.iamltu('dim') or (self.initboard=='nbi' and self.iamltu()):
      #print ":", self.io.execute("getsgmode()",applout='<>')[0], ":"
      rcs= self.io.execute("getsgmode()",applout='<>')
      if len(rcs)==1:
        rc= rcs[0]
      else:
        rc= '-2'
      if rc == '-2':
        self.setColor(COLOR_WARNING)
        errmsg="rc:-2, cannot contact server"
        print errmsg
        return
      elif rc == '-1':
        self.setColor(COLOR_WARNING)
        errmsg="""
- LTU not in the crate or 
- bad LTU base address given or
- obsolete FPGA in LTU or
- unsuccessfull FPGA load from Flash memory on the board after power off/on"""
        self.io.write(errmsg)
        print errmsg
        return
      elif rc=='0':
        self.io.write("LTU in global mode !\n")
        self.setColor(COLOR_WARNING)
      #rcs= self.io.execute("vmeopr32(LTUVERSION_ADD)",applout='<>')
    #
    if self.iamltu():
      rcs= string.strip(self.io.execute("vmeopr32(LTUVERSION_ADD)"))
      print "VmeBoard:",rcs
      rc= eval(rcs)
      if rc>=0xb7:
        self.io.write("LTU version %s, (i.e. run2)\n"%rcs)
        self.lturun2= True
      else:
        self.lturun2= False
        self.io.write("LTU version %s, (i.e. run1)\n"%rcs)
    """ from 23.6. init.mac is called directly from cmdbase.c
    initmac= os.path.join( os.environ['VMEWORKDIR'],"CFG",
      self.boardName, "init.mac")
    if os.access(initmac, os.R_OK):
      self.openCmd()
      #ToDo: execute(init() only:
      # if /tmp/BoardName_busy.PID == self.io.thds[0].pidexe OR
      #    /tmp/boardName_busy doesn't exist
      # OR (seems better) do it in cmdbase (modify macro processing)
      self.io.execute("init()")
    """
    #print "myw.VmeBoard:", boardName

class VmeCrate:
  _instance= None
  def __init__(self, master=None):
    self.boards=[]
    self.autohelps=None
    if self.autohelps==None:
      print "Use right mouse button to get help"
    self.master=master
    self.crframe= Frame(master)
    self.crframe.pack(side="top")
    self.crbuts= Frame(master)
    self.crbuts.pack(side="bottom", expand='yes', fill='x')
    okbut= MywButton(self.crbuts, label='quit', cmd= self.quit,side="bottom") 
    self._instance= self
  def addBoard(self, board):
    self.boards.append(board)
  def findBoard(self, boardname):
    for b in self.boards:
      if b.boardName==boardname:return(b)
    return(None)
  def quit(self):
    for b in self.boards:
      b.delBoard()
    self.master.destroy()

def getCrate(f=None):
  if VmeCrate._instance==None:
    if f==None:
      #there has to be VmeCrate._instance for vmeboards software
      # i.e. this is the call when used outside of 'vmecrate' sw
      return None
    else:
      # first (and only call with f!=None) from crate.py
      VmeCrate._instance= VmeCrate(f)
  return VmeCrate._instance

#------------------------------------- Vbexec (used in CTP sw)
vbexec=None
def vbinit(vb):
  global vbexec
  if not vbexec:
    if vb:
      vbexec= Vbexec(vb)
    else:
      vbexec= Vbexecvoid()

class Vbexec:
  """
  Access to popened ctp.exe + 2 command buttons Readhw, Write2hw
  """
  #vbinst=None
  def __init__(self, vb):
    #if Vbexec.vbinst:
    #  print 'ctpcfg.vbexec: attempt to reinitialise vbinst'
    #else:
    self.vbinst= vb
    #self.printvbnames()
    self.cmdbuts= [ self.getcmdbut("Write2hw"),
      self.getcmdbut("Readhw"), self.getcmdbut("File")]
  def setWarning(self,color):
    #print "mywsetWarning:",len(self.cmdbuts), self.cmdbuts
    for ixb in self.cmdbuts:
      if ixb!=None:
        if ixb[1]==-1:   #groupname
          ixb[0].configure(background=color)
        else:
          ixb[0].entryconfigure(ixb[1],background=color)
  def getcmdbut(self,name):
    """rc:
    None            not found
    menu,ix         name found as function name in a group
    menubutton, -1  name found as a groupname
    """
    rc=None
    for ixfg in range(len(self.vbinst.gmbs)):
      fg= self.vbinst.gmbs[ixfg]
      if ixfg%2 ==0:
        grpname= fg.cget("text")
        if grpname==name:
          rc= fg,-1 
          break
      else:
        ixbt= fg.findLabel(name)
        #print "getcmdbut:",ixbt
        if ixbt!=None: 
          rc= fg,ixbt
          break
    #if rc: print "getcmdbut:",name,rc
    return rc
  def printvbnames(self):
    """An example of searching through all the function names 
    in board instance
    """
    for ixfg in range(len(self.vbinst.gmbs)):
      fg= self.vbinst.gmbs[ixfg]
      if ixfg%2 ==0: 
        print "Vbexec:groupname:type:",type(fg)
        grpname= fg.cget("text")
        print "Vbexec:groupname:",grpname
        continue
      print "Vbexec.fg:",type(fg), fg.type(0),"\n",dir(fg)
      ixfunmax =fg.index(END)+1
      print "Vbexec.ixfunmax>",ixfunmax
      for ixfun in range(1, ixfunmax):   # 0 -is tearoff type
        entype= fg.type(ixfun)
        enlabel= fg.entrycget(ixfun,"label")
        print "Vbexec:ix",ixfun,"entype:",entype,"name:",enlabel
  def printmsg(self, msg):
    try:
      self.vbinst.io.write(msg)
    except:
     exctype, value= sys.exc_info()[:2]
     #print "exc_info:",exctype,value
     #exc_info: exceptions.AttributeError 'None' object has no attribute 'write'
  def getline(self, cmd):
    """ return line printed to stdout by cmd, trailing NL is peeled off
    If cmd is: "setBCmasks()\n+data_passed_through_stdin"
    then self.getoutput has to be called to sychronise cmdlin2 i/o
    """
    l= self.vbinst.io.execute(cmd)
    if len(l)>0: l= l[:-1]
    return l
  def getoutput(self):
    l= self.vbinst.io.thds[0].getOutput()
    return l
  def get1(self, cmd):
    """ return list of strings (printed to stdout by cmd as 1 line)"""
    return string.split(self.vbinst.io.execute(cmd))
  def get2(self, cmd, log="out"):
    """ return list of strings (items were found in <> 
    printed to stdout by cmd)"""
    if log!="out": log= "no"
    a= self.vbinst.io.execute(cmd, log,"<>")
    #print "get2:",a
    return a
    #return self.vbinst.io.execute(cmd, log, "<>")
  def getsl(self, cmd):
    """ return list of strings -string/line (printed to stdout by cmd as more lines)"""
    return string.split(self.vbinst.io.execute(cmd),'\n')

class Vbexecvoid:
  def printmsg(self,msg):
    print msg

def checklabel(arg1=None):
  print "here checklabel. arg1:", arg1
def checklabel2():
  print "here checklabel2"
  #print event,b
def prerror():
  global butsFrame
  xy=butsFrame.winfo_rootx(),butsFrame.winfo_rooty()
  MywError("this error message\n overlaps button Frame",xy)
def efdestroy(event):
  print "efdestroy:",event
def fdestroy(event):
  print "fdestroy:",event

class Testcfg:
  def __init__(self):
    self.caclstl= NewToplevel("CTP classes",self.k_destroyed)
    self.canvas= Kanvas(self.caclstl,self.canvasDestroyed, ctpcfg=self,
      width=200,height=100,
      scrollregion=(0, 0, 200, 300),
      background='yellow', borderwidth=1)
    for ixl in range(20):
      id0=self.canvas.create_text(1, 3+10*ixl,
        anchor=NW,text="Cl#%d"%ixl)
      self.canvas.doHelp(id0, """Class number and Cluster it belongs to
help text %d"""%ixl)
  def k_destroyed():
    print "k_destroyed:"
  def canvasDestroyed(self,event):
    print "canvasDestroyed:",event
    self.canvas=None

def Test_Kanvas():
  cfg= Testcfg()

def main():
  global butsFrame
  f = Tk()
  f.bind("<Destroy>", fdestroy)
  f.title("Myw examples")
  #----------------------------------------------------entry:
  #f.option_add("bLUEcolor*background","LimeGreen")
  # seems not to work with the equal name for more instances
  # (look for bLUEcolor)
  #entriesFrame=MywFrame(f, side=LEFT,name="bLUEcolor"); 
  entriesFrame=MywFrame(f, side=LEFT); 
  entriesFrame.bind("<Destroy>", efdestroy)
  entriesFrame.config(bg="blue")
  lb=MywEntry(entriesFrame,label="MywEntry with help/cmd:",
    side=TOP, defvalue='abc',cmdlabel=checklabel,helptext="abc\n\
  2 riadok\ntreti\n\
  stvrty\n\
piaty")
  lb3=MywEntry(entriesFrame,label="MywEntry with help/nocmd:",
    side=TOP, defvalue='abc',helptext="abc help")
  lb2=MywEntry(entriesFrame,label="MywLUT", 
    defvalue='a|b&c',side='top', bind='r',cmdlabel=checklabel2)
  #lb4=MywVMEEntry(entriesFrame, label="VMEreg", helptext="automatic update of VME register", vmeaddr="0x88")
  lb.getEntry()
  #but=Button(entriesFrame, text="print entry", command=lb.printEntry) 
  #but.pack(side=TOP, expand='yes', fill='x')
  #----------------------------------------------------buttons:
  butsFrame=MywFrame(f, side=LEFT); 
  butsFrame.config(bg="yellow")
  #
  doerrbut=MywButton(butsFrame,label="Make error",cmd=prerror)
  desbut=MywButton(butsFrame,label="destroy 1st entry",cmd=lb.destroy)
  radiob=MywRadio(butsFrame,label='MywRadio\nbutton')
  #
  #passing parameter to callback function (lambda):
  prent= lambda i=radiob,x='callback printEntry': i.printEntry(x)
  prent(i=radiob,x='direct printEntry')
  but=MywButton(butsFrame, label="MywButton-print radio", 
    cmd=prent) 
  #another approach to callback with parameters (positional par is OK too):
  ccbpe='curry callback printEntry'
  but=MywButton(butsFrame, label="MywButton-print radio\ncurry", 
    cmd=curry(radiob.printEntry,text=ccbpe)) 
  butExit=MywButton(butsFrame, label="exit", cmd=f.destroy)
  #-----------------------------------------------------menus:
  ##f.mainloop(); return
  #menusFrame=MywFrame(f, side=LEFT,name="bLUEcolor"); 
  menusFrame=MywFrame(f, side=LEFT); 
  menusFrame.config(bg="pink")
  #wid= menusFrame.winfo_id(); widp=menusFrame.winfo_pathname(wid)
  #print "wid:",wid,"widp:",widp,"winfo_name:",menusFrame.winfo_name()
  menub=MywxMenu(menusFrame,label="MywxMenu", items=(("txt1","b1"),
    ("txt2","b2"), ("txt3","b2")))
  menubl=MywMenuList(menusFrame,label="MywMenuList", 
    items=("txt1","b1", "txt3"), defaults=[0,1,1])
  #menubl.disable()
  simplebut= MywButton(menusFrame, label="MywButton", 
    cmd=menub.printEntry,
    helptext="print MywxMenu value") 
  bitsmenu= MywBits(menusFrame, 
    bits=["PP Pre-pulse", "L0", "L1", "L1m L1 message"],
    helptext="MywBits demonstration")
  #-------------------------------------------- Error messages
  xy=butsFrame.winfo_rootx(),butsFrame.winfo_rooty()
  #MywError("butsFrame is not overlaped (mainloop?)\nby this window",xy)
  #--------------------------------------------BitsPlay:
  biplFrame=MywFrame(entriesFrame,bg="pink");
  #mask: 0x3
  b1=MywxMenu(biplFrame, items=(("bit0","0x1"),("bit1","0x2")),
    side=LEFT,helptext="bits 0x3")
  b1.radiobut.configure(relief=FLAT)
  #mask: 0xc
  b2=MywxMenu(biplFrame, items=(("bit2","0x4"),("bit23","0xc"),("bit23=0","0")),
    side=LEFT,helptext="bits 0xc")
  b2.radiobut.configure(relief=FLAT)
  b1b2e=MywEntry(biplFrame,label="bits:", 
    defvalue=' ',side='top')
  lb.getEntry()
  #
  #for n in range(1,20):
  #  but.flash()
  #----------------------------------------------------canvas test:
  canvFrame=MywFrame(entriesFrame, side=TOP); 
  canvFrame.config(bg="green")
  docanvbut= MywButton(canvFrame,label="Test Kanvas",cmd=Test_Kanvas)
  #----------------------------------------------------switched test:
  swiFrame=MywFrame(entriesFrame, side=BOTTOM); 
  swiDet=MywxMenu(swiFrame, 
    items=(("acorde","ACO"),("SPD","SPD"), ("MUON_TRG","MTR")),
    side=LEFT,helptext="choose detector")
  #swidet.radiobut.configure(relief=FLAT)
  swiInp=MywxMenu(swiFrame, 
    items=(("None","0"), ("0MUH","16"),("0MUL","17"), ("0MSH","18")),
    side=LEFT,helptext="choose input to be connected")
  #swidet.radiobut.configure(relief=FLAT)
  f.mainloop()

if __name__ == "__main__":
    main()

