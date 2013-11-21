#!/usr/bin/python
# 1.4.2006 Error: 'incorrect Restore' now fixed
#    sig. names are now accompanied by bit number in SSM 
# 10.4.2006 - signals names now modified with the board change
from Tkinter import *
import tkFileDialog
import os, os.path, string, shelve #, glob
#sys.path.append("/home/alice/aj/v/vmeb")
#print "syspath:",sys.path
import myw

def DoSaveLoadWidgets(initialfile,filetypes):
  fts= filetypes + [("all files","*")]
  OpenLoad=tkFileDialog.Open(
    initialdir= os.environ['VMEWORKDIR'] +"/WORK/",
    initialfile=initialfile, filetypes=fts)
  OpenSave=tkFileDialog.SaveAs(
    initialdir= os.environ['VMEWORKDIR'] +"/WORK/",
    initialfile=initialfile, filetypes=fts)
  return (OpenSave, OpenLoad)

SavedSignals= DoSaveLoadWidgets("default.savedsigs", 
  [("SSM broswer signals","*.savedsigs")])

class BSnames:
  def __init__(self, siginst, brdix):
    """
    siginst -instance of SSMsig (SSMsig+BSnames -> 1 line in SSMbrowser win)
    brdix   - index into siginst.brinst.sms (STRING)
    """
    #print "BSnames brdix:",brdix, type(brdix), "siginst:", siginst
    self.siginst=siginst
    sms=self.siginst.brinst.sms[eval(brdix)]
    self.fbname=sms[1]
    self.brdix=brdix
    self.signal=None
    #print "BSnames ssmboards:",self.siginst.brinst.ssmboards
    if siginst.brinst.ssmboards!=None:
      self.board= myw.MywxMenu(self.siginst.topfr, label='SSM:',
        helptext="""
Board name + mode in which the SSM of this board
was recorded/generated.
The red color of this button indicates 'not synchronised SSM' 
(sms.syncflag differs from other boards), which means, 
that bc:0 corresponds to the physical start of SSM (address 0).

Default (grey) color of this button indicates 'synced SSM' -
i.e. bc:0 corresponds to sms[ix].offset (which differs for
synchronised boards).
""", 
        defaultinx=0, side=LEFT, 
        items=self.siginst.brinst.ssmboards,cmd=self.modboard,
        delaction=self.delsig)
        #delaction=self.siginst.brinst.delsig)
      #von boardn= self.siginst.brinst.ssmboards[ixdef][1]
      #von fbname= self.siginst.brinst.sms[eval(boardn)][1]  #mode is: fo_mode
      #fbname= self.siginst.brinst.ssmboards[ixx][0]
      #von self.sigitems=self.getsignals(self.fbname)
      #
      ixdef= eval(self.siginst.brinst.searchssmb(self.brdix))
      self.modboard(None, ixdef)
  def modboard(self, inst, ixx):
    """
    ixx:    integer -index into items in self.board
            (i.e. in self.siginst.brinst.ssmboards)
            TO BE SET in widget if inst==None
    boardn: string -index into self.siginst.brinst.sms[]
    """
    #print "modboard:ixx:",type(ixx),ixx
    if inst:
      boardn= self.board.getEntry()
      self.fbname= self.siginst.brinst.sms[eval(boardn)][1]  #mode is: fo_mode
    else:
      boardn= self.siginst.brinst.ssmboards[ixx][1]
      self.board.setEntry(ixx)
    self.sigitems=self.getsignals(self.fbname)
    #print "BSnames.modboard:boardn, ixx:",boardn,ixx,type(ixx)
    #print "BSnames.modboard:inst:",inst,ixx,self.fbname,self.sigitems
    self.siginst.ssmix= boardn
    # check syncflag for this board, and mark it 'synced/not synced'
    syncflag= string.split(self.siginst.brinst.vb.io.execute(
      "getsfSSM("+boardn+")","NO"),"\n")[0]
    if syncflag != self.siginst.brinst.syncflag:
      self.board.setColor(SSMsig.colNotSync)
    else:
      self.board.resetColor()
    #for ix in range(len(self.siginst.brinst.ssmboards)):
    #  if boardn==self.siginst.brinst.ssmboards[ix][0]: 
    #    self.brdix=ix
    #    break
    if self.signal: 
      self.signal.destroy()
    dfltix=self.bit2name(self.siginst.ssmixbit)
    #print "modboard.items:",dfltix,self.siginst.ssmixbit,self.sigitems
    self.signal= myw.MywxMenu(self.siginst.topfr, label='',
      helptext="""
The name of the signal obtained from corresponding .sig file
in %s directory accompanied by bit number (0-31). 
If .sig file is missing, bits are named 0-31.
"""%self.siginst.brinst.sigfdir, defaultinx=dfltix,
      side=LEFT, items=self.sigitems, cmd=self.modsignal)
    #self.siginst.ssmixbit= '0'; self.siginst.draw() '0'->bad idea
    self.modsignal()
  def bit2name(self,charbit):
    for relpos in range(len(self.sigitems)):
      if self.sigitems[relpos][1]==charbit: return relpos
    return 0   #show 1st. named bit if not found
  def modsignal(self,inst=None,ix=None):
    self.siginst.ssmixbit= self.signal.getEntry()
    self.siginst.draw()
    #print "BSnames.modsignal:ix:",ix, self.siginst.ssmixbit
  def delsig(self, frtodest):
    self.board.destroy()
    self.signal.destroy()
    self.siginst.canvas.destroy()
    for ix in range(len(self.siginst.brinst.sigs)):
    #for sgs in self.siginst.brinst.sigs:   blbina (treba ix)
      if self.siginst.brinst.sigs[ix] is self.siginst:
        #print "SSMbrowser.delsig: deleting", len(self.siginst.brinst.sigs)
        del self.siginst.brinst.sigs[ix]
        #print "SSMbrowser.delsig: after", len(self.siginst.brinst.sigs)
        break
    frtodest.destroy()
  def getsignals(self, fbname):
    """
    fbname: sms[].mode - filename without '.sig' suffix
    """
    #print "getsignals: fbname:",fbname
    fname= self.siginst.brinst.sigfdir+fbname+".sig"
    if fbname=='_nomode' or (os.access(fname, os.F_OK)==0):
      lines=[]
      for i in range(32):
        lines.append("%d %2d"%(i,i))
    else:
      f= open(fname,"r"); lines= f.readlines(); f.close()
    sigs=[]
    for line in lines:
      #print "getsignals:",line
      if line[0]=='#': continue
      if line[0]=='\n': continue
      bitsn= string.split(line)
      if (int(bitsn[0])>31) or (int(bitsn[0])<0): continue
      sigs.append([bitsn[1]+' '+bitsn[0], bitsn[0]])   # [name,bitnumber]
    return sigs
class SSMsig:
  bcw=9
  bch=6   # bit height, even number, minimum 2
  sigx0=1
  sigy0= 5   # left/top corner of signal line
  canvasw= 900  # canvas width and height in pixels
  canvash=30
  coordinates= 60
  bsframe=1
  colSignBg='#ffffcc'
  colHelpBg='#ccffff'
  colOffsetBg='#ccff00'
  colNotSync='#ff0000'
  def __init__(self,brinst,ssmix,ssmixbit='0'):
    """ brinst: SSMbrowser instance
        ssmix:  SSMindex (0:busy, 1:l0, ...)
        ssmixbit: SSM bit (0..31)
    """
    #self.fr=brinst.fsigs
    self.brinst=brinst
    self.ssmix= ssmix   # STRING -ssm identification (ix to sms)
    self.ssmixbit= ssmixbit  # first bit by default
    self.topfr= myw.MywFrame(brinst.fsigs,borderwidth=0,side=TOP,relief=FLAT)
    self.canvas= Canvas(self.topfr,width=SSMsig.canvasw,height=SSMsig.canvash,
      background=SSMsig.colSignBg, borderwidth=1)
    self.canvas.pack(fill='y',side=RIGHT)
    #myw.MywHelp(self.topfr,"blabla", self.canvas)  bavitiez
    myw.MywHelp(self.brinst.fsigs,"""
Signal values (0->green, 1->red).
1. The dragging mouse over green or red line displays 
   the from/to BCs (relative to 'offset' for SYNCED SSMs, or to 
                    'start of SSM' for unsynced SSMs) 

2. Left mouse button click shifts the signal to the BC where 
   the value of signal is changed and along with, all 
   the shown signals are shifted by the same number of BCs.
""", self.canvas)
    self.bsnames= BSnames(self,self.ssmix)
    incanvas="""
    bsfr= myw.MywFrame(self.fr,side=TOP,relief=FLAT)  # board/signals frame
    BSnames(bsfr,"A")
    id= self.canvas.create_window(0,0,window=bsfr,
      anchor=NW)
    """
    self.topfr.bind("<Destroy>", self.canvasDestroyed)
  def dobc(self, pos1, pos2, value):
    rc=None
    if value==0:
      #x1= SSMsig.sigx0+SSMsig.bcw*pos1; y1= SSMsig.sigy0+SSMsig.bch/2
      #x2= SSMsig.sigx0+SSMsig.bcw*(pos2+1); y2= SSMsig.sigy0+SSMsig.bch
      x1= SSMsig.sigx0+SSMsig.bcw*pos1; y1= SSMsig.sigy0+SSMsig.bch
      x2= SSMsig.sigx0+SSMsig.bcw*(pos2+1); y2= y1
      color='green'
    else:
      x1= SSMsig.sigx0+SSMsig.bcw*pos1; y1= SSMsig.sigy0
      x2= SSMsig.sigx0+SSMsig.bcw*(pos2+1); y2= y1
      color='red'
    xr1=x1; yr1= SSMsig.sigy0;
    xr2=x2; yr2= SSMsig.sigy0+SSMsig.bch
    if x1>SSMsig.canvasw:
      rc="canvas overflow"
      return rc
    if x2>SSMsig.canvasw:
      x2= SSMsig.canvasw; rc="canvas overflow"
    #bitidr= self.canvas.create_rectangle(xr1,yr1,xr2,yr2,tags="TAGsig")
    bitid= self.canvas.create_line(x1,y1,x2,y2,fill=color,
      width=3, tags="TAGsig")
    enthandler= lambda e,s=self,k=(pos1,pos2,value):s.enterbit(e, k)
    lmhandler= lambda e,s=self,k=(pos1,pos2,value,self):s.lmclick(e, k)
    self.canvas.tag_bind(bitid, "<Enter>", enthandler)
    self.canvas.tag_bind(bitid, "<Leave>", self.leavebit)
    self.canvas.tag_bind(bitid, "<Button-1>", lmhandler)
    return rc
  def enterbit(self, event, p12v):
    self.canvas.delete("TAGhlptemp")
    #bcatr= findbc(event.x)
    #print "enterbit:", event.x, event.y, "->", p12v
    obrd=1
    x= SSMsig.sigx0+SSMsig.bcw*p12v[0]+obrd; 
    if p12v[2]==1:
      y=SSMsig.bch*2; 
    else:
      y=SSMsig.bch*3; 
    #bc1-bc2:
    hili= p12v[1]+self.brinst.basebc
    if hili>1048575: hili=1048575 
    self.hlptxt= self.canvas.create_text(x,y,anchor=NW,
      tags="TAGhlptemp", text="%d-%d"%(p12v[0]+self.brinst.basebc, hili))
    cs=self.canvas.bbox(self.hlptxt)
    self.ovalhelp= self.canvas.create_rectangle(cs[0]-obrd,cs[1]-obrd,
      cs[2]+obrd,cs[3]+obrd,tags="TAGhlptemp", fill=SSMsig.colHelpBg)
    self.canvas.tag_raise(self.hlptxt, self.ovalhelp)
    #marker stuff:
    rr=2; middle= (p12v[1]-p12v[0])*SSMsig.bcw/2
    if p12v[2]==0:
      x1= SSMsig.sigx0+SSMsig.bcw*p12v[0] +middle -rr; 
      y1= SSMsig.sigy0+SSMsig.bch-rr; 
    else:
      x1= SSMsig.sigx0+SSMsig.bcw*p12v[0] +middle -rr; 
      y1= SSMsig.sigy0-rr
    x2= x1 +rr+rr
    y2=y1+rr+rr
    self.marker= self.canvas.create_oval(x1,y1,x2,y2,
      tags="TAGhlptemp", fill=SSMsig.colHelpBg)
  def leavebit(self, event):
    pass
    #print "leavebit:", event.x, event.y
    #self.canvas.delete(self.ovalhelp)
  def lmclick(self, event, p12vs):
    #print "lmclick:", p12vs[0], p12vs[1], p12vs[2]
    #print "lmclick2:", p12vs[3].ssmix, p12vs[3].ssmixbit,self.brinst.basebc
    bcntxt= str(self.brinst.basebc+p12vs[1])
    lastbit= self.brinst.vb.io.execute("finddifSSM("+self.ssmix+
      ','+ self.ssmixbit +',' + bcntxt+")", "NO")
    if lastbit[:-1] != '-1':
      self.brinst.bcent.setEntry(lastbit[:-1])
      #self.brinst.basebc= eval(lastbit[:-1])
      self.brinst.bcmodified(self.brinst)
  def findbc(self, xpix):
    # valid only in 'all' mode
    pass
  def draw(self):
    self.canvas.delete("TAGsig")
    self.canvas.delete("TAGhlptemp")
    bcntxt= str(self.brinst.basebc)
    #just +2 more than canvas can encompass
    #maxbc=(SSMsig.canvasw-SSMsig.sigx0)/SSMsig.bcw+2
    maxbc= 1024*1024
    dertxt= self.brinst.vb.io.execute("getsigSSM("+self.ssmix+
      ','+ self.ssmixbit +',' + bcntxt+","+str(maxbc)+")",
      "NO")    # out or no
    der= string.split(dertxt,'\n')
    #print "der:",der,":2"
    if der[0]=='-1':
      self.brinst.vb.io.write("SSM "+self.ssmix+ "not available")
      return
    val= eval(der[0])
    #der=[0,5,9,16,17,20, 300,320]; val=der[0]
    pos1=0
    #print 'draw:',der
    for ix in range(len(der)-1):
      if der[ix+1]=='':
        pos2=maxbc
      else:
        pos2= eval(der[ix+1])
      #print "SSMsig.draw:",bcntxt,pos1,pos2,val
      if self.dobc(pos1, pos2-1, val): break
      val=1-val
      pos1= pos2
    #offset:
    self.dooffset()
  def dooffset(self):
    self.smsoffset= self.brinst.vb.io.execute(
      "getoffsetSSM("+self.ssmix+ ")", "NO")[:-1]
    obrd=1
    x= SSMsig.sigx0 + SSMsig.canvasw/2; 
    y=SSMsig.bch*3; 
    self.entoff= myw.MywEntry(self.topfr, bind='lr',label='', 
      cmdlabel=self.offsetmod, width=10, 
      defvalue=self.smsoffset)    #, bg= SSMsig.colOffsetBg)
    id= self.canvas.create_window(x, y,window=self.entoff,
      anchor=NW)
  def offsetmod(self,ev):
    newoffset= self.entoff.getEntry()
    #print "offsetmod:", newoffset
    if self.smsoffset !=newoffset:
      self.brinst.vb.io.execute(
        "setoffsetSSM("+self.ssmix+','+newoffset+ ")")
      self.draw()
  def dooffset1(self): 
    """ not used (offset not modifiable)
    """
    smsoffset= self.brinst.vb.io.execute(
      "getoffsetSSM("+self.ssmix+ ")", "no")
    obrd=1
    x= SSMsig.sigx0 + SSMsig.canvasw/2; 
    y=SSMsig.bch*3; 
    offsettxt= self.canvas.create_text(x,y,anchor=NW,
      tags="TAGsig", text=smsoffset+'->')
    cs=self.canvas.bbox(offsettxt)
    ovalhelp= self.canvas.create_rectangle(cs[0]-obrd,cs[1]-obrd,
      cs[2]+obrd,cs[3]+obrd,tags="TAGsig", fill=SSMsig.colOffsetBg)
    self.canvas.tag_raise(offsettxt, ovalhelp)
  def canvasDestroyed(self,event):
    pass
    #print "canvasDestroyed:"

class SSMbrowser:
  saveactions= [["Save signals","names"], ["Save also the content of SSMs", "ssms"]]
  restactions= [["Restore signals","names"], ["Restore also the content of SSMs", "ssms"]]
  def __init__(self,vb):
    #import pdb ; pdb.set_trace()
    self.vb=vb
    self.sigfdir= os.environ['VMECFDIR'] +"/CFG/ctp/ssmsigs/"
    workfdir= os.environ['VMEWORKDIR'] +"/WORK/"
    self.tl= myw.NewToplevel("SSM browser")
    self.basebc=0
    self.savedsigs=[]   # list of saved signals
    self.fsigs= myw.MywFrame(self.tl,side=TOP,relief=FLAT)  # signals frame
    self.fbuts= myw.MywFrame(self.tl,side=TOP,relief=FLAT)  # contr. buttons frame
    #self.sms=[] done in findAllSMS()
    self.findAllSMS()   # fill in self.sms (the copy of sms from ssm.c)
    self.ssmboards=[]   #items for BSnames.ssmboards
    self.dossmboards()
    # find SSMs with highest syncflag:
    self.syncflag=None   # STRING
    self.sigs=[]; self.findSynced()   # only relevant SSMs
    self.newsigbut= myw.MywButton(self.fbuts, label="Add new signal",
      cmd=self.addnewsig, side=LEFT, helptext="""
Add new signal:
The new signal widget is created in SSMbrowser window.
The SSM and its bit can be choosen by 2 left most buttons.
The SSM can be choosen from boards, from which SSM was read -
i.e. unsynchronised SSMs can be viewed too.
""")
    self.refreshbut= myw.MywButton(self.fbuts, label="Refresh shown signals",
      cmd=self.refreshsigs, side=LEFT, helptext="""
Redraw the signals on the screen. Note: SSM on the board is not read
into computer memory -the refresh is done only from computer memory
""")
    #self.savebut= myw.MywButton(self.fbuts, label="Save shown signals",
    self.savebut= myw.MywxMenu(self.fbuts, side=LEFT, defaultinx=0,
      label='', items=SSMbrowser.saveactions,
       cmd=self.savesigs,helptext="""
Save choosen boards/bits names, possibly with SSMs content,
for future restoration with new instance of SSMbrowser and pressing Restore button
""")

    self.restorebut= myw.MywxMenu(self.fbuts, label="",
      defaultinx=0, items=SSMbrowser.restactions,
      cmd=self.restoresigs, side=LEFT, helptext="""
Restore shown signals, optionally with SSMs content, from file(s)
saved before by 'Save shown signals' button.
""")
    llbut= myw.MywButton(self.fbuts, label="<<",
      cmd=myw.curry(self.shiftsigs,0), side=LEFT)
    lbut= myw.MywButton(self.fbuts, label="<",
      cmd=myw.curry(self.shiftsigs,-100), side=LEFT)
    rbut= myw.MywButton(self.fbuts, label=">",
      cmd=myw.curry(self.shiftsigs,100), side=LEFT)
    self.bcent= myw.MywEntry(self.fbuts, label="bc:", 
      defvalue=str(self.basebc),bind='lr',side=LEFT, 
      cmdlabel=self.bcmodified, helptext="""
0 corresponds to synchronised SSM beginning (if more synced SSM
are available)
""")
    synctxt='SyncFlag:'+self.syncflag+' SSMs:'
    for nt in self.sigs:
      ix= eval(nt.ssmix)
      #print "bla1:", type(ix), ix
      #print "bla:", type(nt.ssmix), type(self.sms[ix]), ix
      synctxt= synctxt+ ' '+nt.ssmix+ '-' + self.sms[ix][0]
    self.synclab= myw.MywLabel(self.fbuts, label=synctxt,side=LEFT,
      helptext="""
Highest syncflag (has to be >0 for synced boards) and
the numbers-boardnames of boards with synchronised SSMs.
""")
    #for s in self.sigs: s.draw() done in BSnames.modboard()
  def bcmodified(self,event=None):
    #print "bcmodified:"
    newbase= self.bcent.getEntry()
    if newbase=='' or newbase==' ': 
      newbase='0'
    try:
      newbase= eval(newbase)
    except:
      print "Error: integer expected in bc field:",self.bcent.getEntry(),":"
      newbase= self.basebc
    if self.basebc != newbase:  
      self.basebc= newbase
      for sig in self.sigs:
        sig.draw()
  def shiftsigs(self,shiftbc):
    if shiftbc==0: shiftbc= -self.basebc
    newbc= self.basebc+shiftbc
    if newbc<0: newbc=0
    if newbc>(1024*1024-100): newbc= 1024*1024-101
    self.basebc=newbc; self.bcent.setEntry(str(newbc))
    for sig in self.sigs:
      sig.draw()
  def addnewsig(self):
    # let's take the first one in self.sigs
    if len(self.sigs)>0:
      self.sigs.append(SSMsig(self,self.sigs[0].ssmix))
  def refreshsigs(self):
    for nt in self.sigs:
      nt.draw()
  def savesigs(self, inst, ix):
    om= self.savebut.getEntry()
    print "savesigs:",ix, om, type(om)
    if om=="names":
      self.savesigs_ssm(False)
    elif om=="ssms":
      self.savesigs_ssm(True)
  def savesigs_ssm(self, dumpssm):
    """ savedsigs[] -list of ['ssmix', 'ssmixbit', mode]
    """
    sssfile= SavedSignals[0].show()
    print "savesigs_file:",type(sssfile),sssfile
    if sssfile=='': return
    basenam= os.path.splitext( os.path.basename(sssfile))
    savedsigs=[]
    for nt in range(len(self.sigs)):
      #itm= [self.sigs[nt].ssmix, self.sigs[nt].ssmixbit]
      mode= self.sms[int(self.sigs[nt].ssmix)][1]
      itm= [self.sigs[nt].ssmix, self.sigs[nt].ssmixbit, mode]
      savedsigs.append(itm)
      #print "savesigs:",self.sms[int(self.sigs[nt].ssmix)],':', savedsigs[nt]
      print "savesigs_itm:", itm
      if dumpssm:
        #  (ssmixbit, ssmix, self.sms[int(ssmix)][0])
        ssmname= "%s_%s.ssm"%(basenam[0],itm[0])
        rc= self.vb.io.execute('dumpSSM(%s,"%s")'%(itm[0],ssmname), applout="<>")
    sf=shelve.open(sssfile)
    sf['savedsigs']= savedsigs
    #print "savesigs:",savedsigs
    sf.close()
  def restoresigs(self, inst, ix):
    om= self.restorebut.getEntry()
    print "restsigs:",ix, om, type(om)
    if om=="names":
      self.restoresigs_ssm(False)
    elif om=="ssms":
      self.restoresigs_ssm(True)
  def restoresigs_ssm(self, restssm):
    sssfile= SavedSignals[1].show()
    if not sssfile: return
    sf=shelve.open(sssfile)
    if not sf.has_key('savedsigs'): return
    savedsigs=sf['savedsigs']
    sf.close()
    basenam= os.path.splitext( os.path.basename(sssfile))
    #for sig in self.sigs:
    for isig in range(len(self.sigs)):
      #print "restoresigs:", self.sigs[0].ssmix, len(self.sigs)
      #sig.bsnames.delsig(sig.topfr)
      self.sigs[0].bsnames.delsig(self.sigs[0].topfr)
      #del sig
    #print "restoresigs2:",self.sms
    #print "restoresigs3:",self.ssmboards
    self.findAllSMS()
    for nt in range(len(savedsigs)):
      ssmix= savedsigs[nt][0]
      ssmixbit= savedsigs[nt][1]
      if len(savedsigs[nt])>=3:
        modeix= savedsigs[nt][2]   # intended mode saved before
      else:
        modeix= ""
      brmode= self.sms[int(ssmix)][1]
      #print "restoresigs:", ssmix, ssmixbit, modeix, "brmode:",brmode, "restssm:", restssm
      #   mode= self.sms[int(self.sigs[nt].ssmix)][1]
      #itm= [self.sigs[nt].ssmix, self.sigs[nt].ssmixbit, mode]
      #print "restoresigs_itm:", self.sigs[nt]
      if brmode=='nossm':
        if not restssm:
          msg="SSM not read, bit %s for board:%s(%s) not restored\n"%\
            (ssmixbit, ssmix, self.sms[int(ssmix)][0])
          self.vb.io.write(msg)
          continue
        #if SSM to be restored also, try to overwrite their content anyhow
      if brmode!=modeix:
        if not restssm:
          msg="W: Bit %s for board:%s(%s) mode:%s but browser is using: %s\n"%\
            (ssmixbit, ssmix, self.sms[int(ssmix)][0], modeix, brmode)
          self.vb.io.write(msg)
          continue
        # if SSM see comment above
      if restssm:
        # 1. check if SSM available:
        #  (ssmixbit, ssmix, self.sms[int(ssmix)][0])
        ssmname= "WORK/%s_%s.ssm"%(basenam[0],ssmix)
        #brmode= self.sms[int(ssmix)][1]
        # modeix: "l2_inmon"  -> modeonly: "inmon"
        # netreba modeonly= modeix[ string.find(modeix,"_")+1:]
        #print "restoresigs2:", ssmname   #, modeonly
        # change also mode according to restored mode:
        rc= self.vb.io.execute('readSSMDumpMode(%s,"%s","%s")'%\
          (ssmix, ssmname,modeix), applout="<>")
        #rc= self.vb.io.execute('readSSMDump(%s,"%s")'%(ssmix, ssmname))
        # rc: '<1>\n' if file does not exist
        #rc= self.vb.io.execute('readSSMDump(%s,"%s")'%(ssmix, ssmname), applout="<>")
        # rc: ['1'] if file does not exist
        #print "restoresigs3:%s:"%rc, rc
        if rc[0]=='0':
          # 2. SSM available and restored. Now show the signal row in browser:
          # refresh:
          self.findAllSMS(); self.dossmboards()
          #self.sms[ssmix]   "_nomode"
          self.sigs.append(SSMsig(self, ssmix, ssmixbit))
        else:
          self.vb.io.write("%s not restored\n"%(ssmname))
      else:
        # restore only a signal row in browser:
        self.sigs.append(SSMsig(self, ssmix, ssmixbit))
    self.refreshsigs()
  def findAllSMS(self):
    """ Return:
    self.sms: list of [name,mode]
    name: busy,l0,l1,l2,fo1,fo2,...,fo6,ltu1,...,ltu4
    mode: nossm    -ssm not read (or board missing)
          notin    -board not in
          _nomode  -empty mode
    """
    self.sms=[]
    lines= string.split(self.vb.io.execute("gettableSSM()","NO"),"\n")
    #print "findAllSMS:",lines,':'
    for ix in range(len(lines)):
      nm= string.split(lines[ix])
      if len(nm)==0: break
      #if nm[1]=='nossm': continue  sms is copy of sms in C !
      if nm[1]=='notin': nm[1]='nossm'
      self.sms.append(nm)
  def findSynced(self):
    line= string.split(self.vb.io.execute("getsyncedSSM()","NO"))
    self.syncflag= line[0]
    #print 'findSync:',line,':'
    # highest_syncflag n1 n2 n3 ...
    # n1,... -> numbers of ssm synced with this syncflag
    for ix in range(len(line)-1):
      #if ix >2: break
      ix2= eval(line[ix+1])   # board number
      #print "ix2:",ix2, "line[ix+1]:",line[ix+1]
      if self.sms[ix2][1]=='nossm': continue
      self.sigs.append(SSMsig(self, line[ix+1]))
    for ixdef in range(len(self.sigs)):
      #ixdef= eval(self.siginst.brinst.searchssmb(self.brdix))
      #self.modboard(None, ixdef)
      self.sigs[ixdef].bsnames.modboard(None, ixdef)
  def dossmboards(self):
    #ml=[["busy_i","0"],["fo1_o","5"], ["l0_i","2"],["l0_o","3"]]
    ml=[]
    # find ssms: in self.siginst.brinst.sms & file sms.mode exists:
    for ix in range(len(self.sms)):
      ssm= self.sms[ix]
      if ssm[1]=='nossm': continue
      #namo= ssm[0]+'_'+ssm[1]      # "name_mode"
      # mode is: name_mode (e.g. : fo_o1   for fo2 board)
      # create name: fo2_o1      (from above example) 
      iu= string.find(ssm[1],'_')
      if iu==-1:
        msg="Bad mode in sms["+str(ix)+"] (missing _):"+ssm[1]+'\n'
        self.vb.io.write(msg)
        continue
      namo= ssm[0]+ssm[1][iu:]
      fb= self.sigfdir
      if ssm[1] != '_nomode' and os.access(fb+ssm[1]+'.sig', os.F_OK)==0:
        print "SSM signals file", namo, "not found"
      else:
        ml.append([namo, str(ix)])   #n_m, ix2sms
    #print "dossmboards:", ml
    if len(ml)>0:
      ml.append(["delete this signal","removevalue"])
      self.ssmboards=ml
  def searchssmb(self,ix2sms):
    """
    go through self.ssmboards
    """
    for ixb in range(len(self.ssmboards)):
      if self.ssmboards[ixb][1]==ix2sms: return str(ixb)
    print "searchssmb error. board:"+ix2sms+" not found (probably not read)"
    return None
class vbio2:
  def __init__(self):
    pass
  def execute(self,cmd):
    print "vbio:", cmd
    return ["0"]
class vbio:
  def __init__(self,master):
    self.master=master
    self.io=vbio2()
def main(vb):
  SSMbrowser(vb)
if __name__ == "__main__":
  master=Tk()
  vb= vbio(master)
  cfdir= os.environ['VMECFDIR']
  os.chdir(cfdir)
  sys.path.append(cfdir+'/ctp')
  main(vb)
  master.mainloop()

