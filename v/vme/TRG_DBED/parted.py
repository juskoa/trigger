#!/usr/bin/python
"""
grep -e'^  def ' -e'^class Trg' parted.py >parted.contents

The goal: edit pname.partition file (pname is the name of the partition)
Operation: load/edit/save pname.partition file

history:
23.2.2005 P/F protection circuits added
12.3. parted.py pname   now OK
16.3. TODO:
 - TrgCTP class (bc1/2 rnd1/2 BCM1-4)
   DONE. Insteda TrgTCP TrgSHR class created
 - ctp load button (from ctp.py?), or different program for CTP load?
 - better colors (yellow is not working corectly now)
   DONE. P/F -yellow, Rare -green (new color is kept as long as possible)
 - if partition use shared resource (rnd, bc, l0fun intfun?), 
   it HAS TO BE DEFINED i.e.:
   - warning if shrcs defined but not used
   - error if used but not defined
   DONE 7.9.2006
23.4.
 - TrgCLass.clsbut points to button in the list of classes in
   Cluster window
24.4.
 - 'show:' button is now MywxMenu (was MywMenuList)
 - RARE has the color assigned
 - full list of class properties implemented in 'show:' button
27.5. bug bcms fixed, 
 - cluster names added 
 - TrgClass.mycluster points now to cluster wherre class is assigned to
 - top level window for class properties disables Class button
   (see TrgClass.hideclass)
28.5. l0prescaler settinag (int) now checked for syntax, better help texts
11.5. bug 'save partition' fixed (before rnd1,2 was saved as rnd3,4)
22.5. Menubar added instead of buttons save/cancel/show/quit
      Class name (initialised to TDS name) added
      Shared resources now in separate top level window
23.5. Save/load now valid for shared resources too
      'Save as' option added
24.5. Selected cluster: last edited cluster is selected (i.e. for
      duplicate )
25.7. cluster deletion (from menu and directly by middle button) now OK
 - VALID.CTPINPUTS supplemented with real CTP inputs connection
26.9. VALID.LTUS format changed. Now line per LTU=fo#.connector#
 1. 4. 2006: TRIGDBDIR and TRIGWORKDIR now used
25.8. .pcfg file created too
19.8.2008:
todo:
-do not save if error
-hz,s + baloon window for BC1,2 RND1,2
11.9.
Version: 2 now:
- class mask handled
- BCM used instead of bcm
- classgroup attribute added to TrgClass
4.5.2009
part.activeclasses dictionary for DAQdb update
4.9.2011
Version: 3   L0 firmware AC  (12 BCmasks, 50 inverted L0 inputs, complex l0f)
23.2.2012
VERSION: 4 (.rcfg)
Version: 4 (.partition) just to be the same with .rcfg
23.9.2012
VERSION: 5 (both .rcfg .partition): sync downscaling
22.10.
VERSION: 6 LINDF REPL added
"""
from Tkinter import *
import os,sys,glob,string,shutil,types
import pdb
sys.path.append("./")
if hasattr(sys,'version_info'):
  if sys.version_info >= (2, 3):
    # sick off the new hex() warnings, and no time to digest what the
    # impact will be!
    import warnings
    warnings.filterwarnings("ignore", category=FutureWarning, append=1)
    print "warnings ignored\n\n"
import myw, txtproc, trigdb, syncdg, preproc
VERSION="6"
COLOR_PART="#006699"
COLOR_CLUSTER="#00cccc"
COLOR_NORMAL="#d9d9d9"
COLOR_TDSPFS="#ffff66"
COLOR_RARE="#99ff99"
COLOR_SHARED="#cc66cc"
COLOR_WARN="#ff9999"
COLOR_ACTIVE="#ff00cc"
COLOR_OK="#00ff00"
symchars = 'abcdfeghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_'


# .partition .pcfg files go here:
CFGDIR=trigdb.CFGDIR       # .partition & .pcfg files are here
WORKDIR=trigdb.TRGWORKDIR  # archive (PCFG/...)
TRGDBDIR=trigdb.TRGDBDIR   # trigger DB files (VALID.PFS,...)

PF_NUMBER=4        # 4 PFs
PF_COMDEFSIX=9
BCMASKS_START=4    # from here, TrgSHR_BCM starts in global SHRRSRCS[]
BCM_NUMBER=12       # 4 before AC, 12: firmAC
if BCM_NUMBER==4:
  L0F_NUMBER=4     # number of inputs in l0f function
  PFS_START=8      # from here, TrgSHR_BCM for PF starts in global SHRRSRCS[]
else:
  L0F_NUMBER=12     # 12 (debug: 6)
  PFS_START=16
lutzero="0x"+ '0'*(2**(L0F_NUMBER-2))
TRGCTPCFG= trigdb.Trgctpcfg()
clgtimes= TRGCTPCFG.getTIMESHARING()
symbols= preproc.symbols()

def IntErr(fstr):
  print "Internal error:",fstr
def PrintError(fstr, selfmsg=None):
  if selfmsg:
    if fstr[-1]=="\n":
      selfmsg.loaderrors= selfmsg.loaderrors + fstr
    else:
      selfmsg.loaderrors= selfmsg.loaderrors + fstr + "\n"
  else:
    print "Error:",fstr
def PrintWarning(fstr, selfmsg=None):
  if selfmsg:
    if fstr[-1]=="\n":
      selfmsg.loadwarnings= selfmsg.loadwarnings + fstr
    else:
      selfmsg.loadwarnings= selfmsg.loadwarnings + fstr + "\n"
  else:
    print "Warning:",fstr
def PrintInfo(fstr):
  print "Info:",fstr
def redline(inf):                 # ignore empty lines
  cltds=""
  while(1):
    cl= inf.readline()
    #print "redline:",cl[:-1]
    if cl=='\n': continue    
    if len(cltds)>0 and cltds[0]=='#':continue
    if cl[-2:]=='\\\n': 
      cltds= cltds + cl[:-2]
      continue
    else:
      cltds= cltds + cl
      break
  #replace symbols, if any:
  while True:
    istart=None
    for ic in range(len(cltds)):
      ch= cltds[ic]
      if ch=='$':   # start of $sym or $sym$sym
        if istart!=None:   # $sym$sym -end of first $sym found
          istop=ic-1 ; break
        istart= ic         # $sym
        continue
      if istart!=None:   # looking for end of $sym
        if not (ch in symchars):
          istop=ic-1 ; break   # end of $sym found
    if istart==None: break     # $... not found
    # istart:istop -first:last character of sym name
    key= cltds[istart+1:istop+1]
    val= symbols.get(key)
    if val==None:
      PrintError("Unknown symbol %s in line:"%(key)+cltds)
      break
    cltds= cltds[:istart] + val + cltds[istop+1:]
  #print "redline rc:",cltds[:-1]
  return cltds
def Parse1(clstring):
  """ 
  clstring: DescriptorName(a=b,c,d=e,f)           -used in 'Clusters:'
    section of .partition file to define 1 class
  rc: name,{'a':'b','c':'ON','d':'e','f':'ON'}
      name,{}     -also valid
  rc: None in case of error
  Reserved names (keys in rc[1]):
  cn -class name
  L0pr  -L0prescaler
  Rare, All, pf[1-4], bcm[1-4]
  """
  rc={}
  nameits= string.split(clstring,'(')
  #print "Parse1:",nameits
  name=nameits[0]
  if len(nameits)>1:
    if nameits[1][-1]!=')':
      PrintError(") expected at the end of line:"+clstring)
      rc=None
    else:
      nameits[1]= nameits[1][:-1]  # strip ')'
      for par in string.split(nameits[1],','):
        # abc=def,par
        par=par.strip()
        #print "Parse1:",par,":"
        if par=='': continue
        valpar=string.split(par,'=')
        if len(valpar)==2:
          rc[valpar[0]]=valpar[1]
        elif len(valpar)==1:
          rc[valpar[0]]="ON"
        else:
          PrintError("bad class definition:"+clstring)
          rc=None
          break
  #print "Parse1:", name,rc
  return name,rc         

def ones(allres):
  n=0
  for res1 in allres:
    if res1==1: n=n+1
  return n
def findRR(allres, res):
  """ allres: list of indexes into SHRRSRCS e.g. [4,5,8] (for BCM)
  """
  #print "findRR looking for:",res.name, res
  for res1 in allres:
    #print "findRR:",res1.name,res1
    if res1 == res: return res1
  return None

def dorcfgname(runnumber, rcfg):
  try:
    name=os.path.join(WORKDIR, "RCFG/r%s.%s"%(runnumber,rcfg))
  except:
    name="badRunNumber%s"%runnumber
  return name

def list2str(lutval):
  lutx=['','']
  if type(lutval) == types.ListType:
    for ix in (0,1):
      lutx[ix]= lutval[ix];
      if lutx[ix]=='0': lutx[ix]= lutzero
    lutval=lutx[0]+lutx[1]
  return lutval

class TrgInput:
  def __init__(self, name):
    '''
      new syntax used in VALID.CTPINPUTS file: 
      InName = Det Level Signature  InpNum  Dimnum Configured Edge Delay
      where:
      Edge: 0 -positive, 1 -negative
      Delay: 0..15 in BCs
    name:
      -input name or l0fdefinition name (starting with l0f) 
    self.ctpinp:
      None   -> no wiring info, or l0f definition
      (0,1)  -> first L0 input, 
      (1,24) -> last L1 input
      (x,0)  -> x:0 1 or 2. Not connected now
    self.l0fdefinition:
      None   -> standard input 
      text   -> l0function definition (simple or complex)
    '''
    self.ctpinp=None   # no wiring info
    self.name=None     # not valid input (error), or input name
    self.l0fdefinition= None   # None: not l0f input definition
    self.l0fAB=None     # !=None: this is l0f34 definition defined by
    # list [l0Axxx_input, l0Bxxx_input]. l0fdefinition in this case is 
    # "(l0Axxx.l0fdefinition) | (l0Bxxx.l0fdefinition)"  -it is compound of both.
    # such l0fxxx is virtual -not existing in VALID.CTPINPUTS, but created 
    # after reading VALID.CTPINPPUTS
    # todo: dynamic transition l0fsimple -> l0f34 if needed
    self.signature=None
    self.detectorname=None
    self.dim=None
    self.edge=None
    self.delay=None
    ninps= string.split(name,'=')
    #print "TrgInput:",ninps
    if len(ninps)==2:
      self.name= string.strip(ninps[0])
      if self.name[:3]=='l0f' or self.name[:3]=='l0A' or self.name[:3]=='l0B':
        # in parted, lookup table can't be computed (real
        # inputs not known yet)
        # but the presence of required ctp input names can be checked now
        self.l0fdefinition= ninps[1]
      else:   # CTP input
        #li= map(string.strip, string.split(ninps[1],' '))
        li= string.split(ninps[1],' ')
        #print "lili:",li
        for ix in range(len(li)-1, -1, -1):
          if li[ix]=='': del li[ix]
        self.detectorname= li[0]
        # Det Level Signature InpNum Dimnum Configured Edge Delay
        #print "TrgInput:", self.name,":", li, self.detectorname
        if len(li) == 1:   # not connected input
          self.ctpinp=None
        elif len(li) >= 8:
          # Det Level Signature InpNum Dim Conf Edge Delay
          # 0   1     2         3      4   5    6    7
          # example of input from L0.INPUTS (not connected):
          #0ASL = ACORDE 0 1 0 1 0 0 12
          if li[5]=='1':
            self.signature= li[2]
            self.dim= li[4]
            self.edge= li[6]
            self.delay= li[7]
            self.ctpinp= int(li[1]), int(li[3])   
          #if li[5]!='1':
          #  self.ctpinp= int(li[1]),0   # not connected
        else:
          self.name=None
          PrintError("Bad CTP input definition:"+name)
    else:
      PrintError("CTP input definition (or detector it belongs to) missing for "+name)
      #pass
    #print "TrgInput:",self.name, self.ctpinp
  def check(self, line):
    #check: detname sig dimnum edge delay
    my="%s %s %s %s %s %s"%(self.name, self.detectorname, self.signature, 
      self.dim, self.edge, self.delay)
    f= string.split(line)
    #print "TrgInput check:",line
    #       0    2    4    6    8  9
    #line= "%s = %s 0 %s 0 %s 0 %s %s"%(inpname, detname, l0in[5], l0in[6], \
    #    l0in[7], l0in[8]) 
    sfl0i="%s %s %s %s %s %s"%(f[0], f[2], f[4], f[6], f[8], f[9])
    if my != sfl0i:
     if self.signature:
      PrintError("discrepancy found: name detname sig dimnum edge delay\n"+\
        "VALID.CTPINPUTS:"+my+"   <-this is taken\n"+\
        "L0.INPUTS:      "+sfl0i)
  def prt(self):
    if self.ctpinp:
      pass
      #print "TrgInput:",self.name,'=',self.detectorname, self.ctpinp
    else:
      if self.l0fdefinition:
        print "TrgInput(l0f):",self.name,'=',self.l0fdefinition
      else:
        print "TgrInput(not connected):", self.name
class TrgInpDet:
  def __init__(self, name):
    self.name=name   # one of TrgLTU.name or 'LTU' or 'SOFT'
    # inps=[] as default parameter: BAD (inputs would be the same pointer
    # for different TrgInpDet objects)
    self.inputs= []   # list of TrgInput objects of self.name detector
    #print "TrgInpDet:",name,self.getinputnames()
  def findInput(self, name):
    for inp in self.inputs:
      if inp.name==name:
        return inp
    return None
  def addInput(self, inp):
    nin= self.findInput(inp.name)
    if nin!= None:
      PrintError("""TrgInpDet %s has already defined input:%s.
Input: %s not added.""" %(self.name, inp.name, str(inp.ctpinp)))
    else:
      self.inputs.append(inp)
      #print "TrgInpDet:addInput",self.name,'=',inp.name
  def getinputnames(self):
    inp= ""
    for i in self.inputs:
      inp= inp+" "+i.name
    return string.strip(inp)
  def tidprt(self):
    print "----------------- TrgInpDet:",self.name, "inputs:",len(self.inputs)
    for tinp in self.inputs: tinp.prt() 
  def makeVirtL0f(self):
    """ create l0fxxx for all pairs l0[AB]xxx.
    Checks:
    - delete l0[AB]xxx if there is l0fxxx
    - one of l0[AB]xxx is enough (i.e one can be missing)
    """
    newl0fs={}
    for i in self.inputs:
      pair=None
      if i.name[:3]=="l0A": pair= 0
      if i.name[:3]=="l0B": pair= 1
      if pair==None: continue
      newname= "l0f"+i.name[3:]
      #print "makeVirtL0f:inp:", i.name, newname
      if self.findInput(newname):
        PrintError("deleting %s. %s already exist."%(i.name, newname))
        i.name= None
        continue
      if not newl0fs.has_key(newname):
        newl0fs[newname]= [None, None]
      newl0fs[newname][pair]= i
      #print "makeVirtL0f:inp:", i.name, newname, newl0fs[newname]
    # newl0fs{ l0fxxx:[Ainp, Binp],...}
    #print "makeVirtL0f:", newl0fs.keys()
    for newname in newl0fs.keys():
      txtdef=''
      if newl0fs[newname][0]:
        txtdef= newl0fs[newname][0].l0fdefinition
      if newl0fs[newname][1]:
        if txtdef=='':
          txtdef= newl0fs[newname][1].l0fdefinition
        else:
          txtdef = "(" + txtdef + ") | (" + newl0fs[newname][1].l0fdefinition + ")"
      l0finp= TrgInput(newname + " = " + txtdef)
      l0finp.l0fAB=  newl0fs[newname]
      self.inputs.append(l0finp)
class TrgDescriptor:
  """
  name: name of Trigger descriptor
  inps: list of trigger inputs
        NEGATED inputs (classes 44-50) are prefixed with '*'
        (i.e. *name)
  return: instance of TrgDescriptor, self.allinputsvalid is True
    In case of error
      self.name is None
    In case descriptor is not loadable (some inputs not connected
    or not connected to 1..4 in case of l0f used)
      self.allinputsvalid is False
  """
  def __init__(self, name, inps=[]):
    self.name= name     # None: error when descriptor created
    self.inputs=[]      # list of NAMES of valid inputs and l0functs
    # ['0VBA', '*0SMB','l0fINT1',...]
    allinputsvalid=True    # all inputs correct
    baddesc=False       # if True, this TRGDESC cannot be referenced
    self.connected=[0,0]     # [connected, unconnected]
    self.unconnectedNames= ''
    self.loaderrors=''   # ok if ''
    # check if the connections of all the inputs are defined:
    #print "---TrgDescr:",self.name,inps
    # calculate self.xxx. Used for .pcfg:
    #   Final.l1def= self.l1def & l1def_cretaed in TrgPartition.savepcfg
    self.l0inps=self.l1def= 0xffffffff
    self.l2def= 0xff000fff
    self.l0inv=self.l1inv= 0
    self.class4550flag=0   # 1 if this TDS requires class 45..50
    # 1 TDS can use as many l0funs as available, i.e. 2
    # NOTE: The final ordering of l0funs is in partition's l0funs !
    self.l0funs=[None,None]  # l0funs definitions (LUTs), if used, for this TDS
    # 1 item is: [integer, string_l0definition]
    self.l0funs34=[None,None] # l0funs definitions (LUTs), if used, for this TDS
    # 1 item is: [[hexastringA, hexastringB], string_complete_l0definition] or A, B separately???
    for inpnegname in inps:
      #von validinpl0= None
      if inpnegname[0]=='*':
        self.class4550flag=1   # necessary only in loader
      inp= TDLTUS.findInput(inpnegname)
      if not inp:
        allinputsvalid=False ; baddesc= True
        PrintError(inpnegname+" -unknown CTP input in "+self.name+" descriptor", self)
        continue
      if inpnegname[0:3]=='l0f': 
        if inp.l0fAB: # complex l0f
          abok=0
          #for l0ab in ("l0A","l0B"):
          for ixab in (0,1):
            #abname= l0ab+ inpnegname[3:]
            #inp= TDLTUS.findInput(abname)
            inp2= inp.l0fAB[ixab]
            if not inp2:   # None: A or B not given
              #allinputsvalid=False ; baddesc= True
              #PrintError(inpnegname+" -bad l0f input in "+self.name+" descriptor", self)
              abok= abok+1
              continue
            err_rc= self.check_l0f(inp2)      # check A or B
            if err_rc:
              allinputsvalid=False ; baddesc= True
            else:
              abok= abok+1
          if abok==2:
            err_rc= self.checkForPcfg(inp)
            if err_rc:
              allinputsvalid=False ; baddesc= True
        else:
          err_rc= self.check_l0f(inp)
          if err_rc==None:
            err_rc= self.checkForPcfg(inp)
          if err_rc:
            allinputsvalid=False ; baddesc= True
            continue
        self.inputs.append(inpnegname)
        continue
      #print "inp.name:",inp.name
      #if name=='DSMHWU':
      #  pdb.set_trace()
      #inp.prt()
      if inp.ctpinp==None:   # not_connected input
        #print "---TrgDes %s:l0f:%s"%(self.name,inp.l0fdefinition),self.l0funs
        self.connected[1]= self.connected[1]+1
        self.unconnectedNames= self.unconnectedNames + inp.name+' '
        PrintError("%s not connected now (%s)"%(inp.name,self.name),self)
        allinputsvalid=False  # baddesc=True-i.e.allow ppreparation in advance
        self.inputs.append(inpnegname)
        continue
      if inp.ctpinp[1]==0:   # not connected input
        self.connected[1]= self.connected[1]+1
        self.inputs.append(inpnegname)
        PrintError("%s not connected now(%s)"%(inp.name,self.name),self)
        allinputsvalid=False  # baddesc=True
        continue
      self.connected[0]= self.connected[0]+1
      # valid input (L0/1/2):
      validinpl0=1
      bit= 1<<(inp.ctpinp[1]-1)
      if inp.ctpinp[0]==0:   # L0 input
        self.l0inps= self.l0inps & ~bit
        if inpnegname[0]=='*': self.l0inv= self.l0inv | bit
      elif inp.ctpinp[0]==1:   # L1 input
        self.l1def= self.l1def & ~bit
        if inpnegname[0]=='*': self.l1inv= self.l1inv | bit
      elif inp.ctpinp[0]==2:   # L2 input
        self.l2def= self.l2def & ~bit
        if inpnegname[0]=='*': self.l2def= self.l2def | (bit<<12)
      else:
        validinpl0=None
        allinputsvalid=False ; baddesc= True
        PrintError("Bad CTP input level(internal error):%s level:%s"%
          (inp.name, str(inp.ctpinp[0]), self))
      if validinpl0:
        self.inputs.append(inpnegname)
      #NOTE: check for class 45..50 later, in CTP loader
    # 2 l0funs bits in self.l0inps will be set from trde.l0funs later
    # in savepcfg()
    self.allinputsvalid= allinputsvalid
    if baddesc==True:
      self.name=None
    #self.trdprt()
    #print "TrgDescriptor:inputs:",self.inputs
  def check_l0f(self, inp):
    """
    inp: instance of l0f, l0A or l0B TrgInput
    """
    # check/find max. 4 l0-inputs in expression:
    oe, vars= txtproc.varsInExpr(inp.l0fdefinition)
    # we should consider 1/2 type or 3/4 type. 1/2type can be
    # loaded to 3/4 but not vice versa.
    err_rc= None
    if oe!="OK":
      PrintError(vars + " in l0f definition: %s = %s"%(inp.name,inp.l0fdefinition), self)
      err_rc= 1
     #print "oe:",oe,"vars:",vars
    elif len(vars)>L0F_NUMBER:
      PrintError("Too many inputs (max. %d) in:"%L0F_NUMBER\
        +inp.l0fdefinition, self) 
      err_rc= 2
    else:
      # vars-> inputs in right order:
      inputs=[]
      minL0F_NUMBER=1
      if L0F_NUMBER==4:
        maxL0F_NUMBER= 4
        maxinputs= 4
      else:
        maxinputs=L0F_NUMBER
        if inp.name[:3]=='l0A':
          maxL0F_NUMBER= L0F_NUMBER
        elif inp.name[:3]=='l0B':
          maxL0F_NUMBER= 12+L0F_NUMBER ; minL0F_NUMBER=12+1
        else:
          maxinputs= 4
          maxL0F_NUMBER= 4
      for ix in range(maxinputs): inputs.append(None)
      for ixl0f in range(len(vars)):
        inl0f= TDLTUS.findInput(vars[ixl0f])
        if inl0f==None:
          PrintError("%s is not CTP input in l0fun definition:%s= %s (%s)"\
            % (vars[ixl0f], inp.name, inp.l0fdefinition, self.name), self)
          err_rc= 3
        elif inl0f.ctpinp==None:   # todo should be legal (can be connected later)
          PrintError("%s is not CTP input in l0f def:%s=%s (%s) right now"\
            %(vars[ixl0f], inp.name, inp.l0fdefinition, self.name), self)
          err_rc=4 
        elif (inl0f.ctpinp[0] != 0) or (inl0f.ctpinp[1] > maxL0F_NUMBER) \
              or (inl0f.ctpinp[1]<minL0F_NUMBER):   
          #print "ctpinp:",ixl0f,inl0f.ctpinp
          # must be 1-4/1-12/13-24 L0 input
          PrintError("""%s (%d) is not (allowed) L0 input in l0fun:%s=%s right now. 
%d should be between %d and %d.
.pcfg file will not be written, or incorrectly written if this
descriptor/l0f is used with any class in this partition.
"""% (inl0f.name, inl0f.ctpinp[1], inp.name, inp.l0fdefinition,
  inl0f.ctpinp[1], minL0F_NUMBER, maxL0F_NUMBER), self)
          err_rc= 5
        else:
          ixinp= inl0f.ctpinp[1]-1 -(minL0F_NUMBER-1)
          #print "check_l0f:", inl0f.ctpinp, "ixinp:", ixinp, "inputs:",inputs
          inputs[ixinp]= inl0f.name
    if err_rc==None:
      inp.l0finputs= inputs   # inputs used for l0A/B or l0f
      #print "check_l0f:", inp.name,":", inputs
    return err_rc
  def checkForPcfg(self, inp):
    """
    To be called in time of .pcfg generation
    inp: instance of l0f TrgInput (simple or complex (i.e. l0fAB defined))
    rc: None if ok
    """
    #print "txtlog2tab:",inp.l0fdefinition, inputs
    err_rc= None
    if inp.l0fAB:
      txtlut2= ["0","0"]
      for ix in (0,1):
        if inp.l0fAB[ix]==None:   # A or B not supplied in VALID.CTPINPUTS
          continue
        inputs= inp.l0fAB[ix].l0finputs
        #print "ABinputs:",inputs
        txtlut= txtproc.log2tab(inp.l0fAB[ix].l0fdefinition, inputs)
        if txtlut==None: break
        txtlut2[ix]= txtlut     #txtlut[2:] 
      l0funs= self.l0funs34
      if txtlut:
        lut= txtlut2
    else:
      inputs= inp.l0finputs
      txtlut= txtproc.log2tab(inp.l0fdefinition, inputs)
      l0funs= self.l0funs
      if txtlut:
        lut= eval(txtlut) 
    #print "txtlog2ta2:",txtlut, self.name
    if txtlut==None:
      PrintError("l0f: %s \n inputs:%s) -cannot create LUT"%
        (inp.l0fdefinition, inputs), self)
      err_rc= 6
    else:
      defstored= self.check2or34funs(l0funs, lut, inp)
      if not defstored:    
        err_rc= 7
        PrintError("Number of used l0funs for TDS %s >2. %s not used"%
          (self.name, inp.l0fdefinition), self)
      #if self.name=="DTEST":
      #  print "TrgDescLUT:DTEST", inp.l0fdefinition, inputs
      #  for ixx in (0,1):
      #    if self.l0funs[ixx]:
      #      print "l0f  :",self.l0funs[ixx][0],'=',self.l0funs[ixx][1].l0fdefinition
      #  for ixx in (0,1):
      #    if self.l0funs34[ixx]:
      #      print "l0f34:",self.l0funs34[ixx][0],'=',self.l0funs34[ixx][1].l0fdefinition
    return err_rc
  def check2or34funs(self, l0funs, lut, inp):
    """ check if already used by this TDS, if not
     add this l0fun definition in l0funs (or self.l0funs34])
    In: lut: integer -for simple l0f
             ["hexa","hexa"] -for complex l0f 2 LUTs in list
    firmAC considerations:
    check: # of l0f <=2 in TDS
    ???
"""
    defstored=None
    for i01 in (0,1):
      if l0funs[i01] == None:
        l0funs[i01]= (lut, inp)
        if i01==1:   #always keep ascending order of LUTs!
          if l0funs[1][0]< l0funs[0][0]: # for right allocation
            lowerlut= l0funs[1];
            l0funs[1]= l0funs[0]
            l0funs[0]= lowerlut
        defstored=1; break
      else:
        if l0funs[i01][0] == lut:
          defstored=1; break
    return defstored
  def show(self,master):
    tdbut= myw.MywButton(master, label=self.name, side=RIGHT, cmd=self.prt)
  def trdprt(self):
    inps=""
    for inp in self.inputs:
      inps= inps+" "+inp
    print "trdprt:"+str(self.name)+":"+inps,"  l0funs:", self.l0funs,"\n"
  def getInputs(self):
    inps=""
    for inpname in self.inputs:
      if inpname[:3]=='l0f':
        inp= TDLTUS.findInput(inpname)
        if inp:
          if inp.l0fdefinition!=None:
            inpname= inpname+"("+str(inp.l0fdefinition)+")"
      if inps:
        inps= inps+" "+inpname
      else:
        inps= inpname
    #inps= inps+" l0funs:"+str(self.l0funs)
    return(inps)
  def getInpDets(self):
    inpdets=[] ; dkeys={}
    #print "xx1:",self.inputs
    td= TDLTUS.findTD(self.name)
    #print "xx2:";td.trdprt()
    for i in td.inputs:
      if i[0]=='*': i= i[1:]   # inverted input
      detn= TDLTUS.getInpDets(i)
      #print "xx3:",detn xx3: ['V0', 'SPD', 'V0', 'SPD', 'V0', 'V0']
      for det in detn:
        if dkeys.has_key(det): continue
        dkeys[det]=1
        inpdets.append(det)
    return inpdets
class TrgPF:
  def __init__(self, name, vals):
    self.name= name
    self.vals= vals
class TrgSHR:
  """Shared resource:
  value:
  - 32 bits int for BC1/2,RND1/2
    string for BC1/2, RND1/2
  - string for BCM1-12, PF1-4 
  - value 'None' (or '') means: not used in this partition
  """
  BCxHelp="""Scaled-down BC. 
Class L0 trigger condition is: L0input AND BCx AND RNDx.
BCx (x=1..2) and RNDx (x=1..2) is applicable for any Class
"""
  RNDxHelp="""Random trigger generator. 
Class L0 trigger condition is: L0input AND BCx AND RNDx.
BCx (x=1..2) and RNDx (x=1..2) is applicable for any Class
"""
  L0FUNxHelp="""
INTfun1, INTfun2, INTfunT (interaction functions) and
L0fun1, L0fun2 (L0 functions) are programmable functions of the first 
four CTP L0 inputs. 
These functions are defined by 16 bits Lookup table (LUT).
The definitions are given in VALID.DESCRIPTORS file: L0fun* entries
are read/only -value should appear only in case L0fun* is used
by this partition.

"""
  def __init__(self, name, helptext=None):
    self.name=name
    self.helptext=helptext
    #if name[:3]=='BCM':
    #  self.value="3654L"
    #else:
    #  self.value=None
    self.value="None"
    self.used=0   # if used, # of references (filled by savepcfg)
    self.ebut=None
  def show(self,master):
    self.ebut= myw.MywEntry(master, label=self.name,side=TOP,
      helptext=self.helptext, bind='lr',
      bg=COLOR_SHARED,
      defvalue=str(self.value), cmdlabel=self.updt)
  def hide(self):
    if self.ebut:
      self.ebut.destroy()
      self.ebut=None
  def updt(self, event=None):
     self.setValue(self.ebut.getEntry())
  def getValue(self):
    return self.value
  def setValue(self, newvaluetxt):
    #print "shrsetValue:",newvaluetxt, type(newvaluetxt)
    if newvaluetxt=='None' or newvaluetxt=='': 
      strorNone=newvaluetxt="None"
    # check for correctness
    elif self.name=="BC1" or self.name=="BC2":
      strorNone= myw.fromms('', newvaluetxt)
    elif self.name=="RND1" or self.name=="RND2":
      strorNone= myw.frommsRandom('', newvaluetxt)
    # elif self.name=="L0fun1" or self.name=="L0fun2":
    #  PrintWarning("setValue: L0fun* are defined in VALID.DESCRIPTORS, not here")
    #  #strorNone= txtproc.log2tab(newvaluetxt)
    #  strorNone=None
    else:
      strorNone=newvaluetxt
    if strorNone==None:
      PrintError("%s syntax:%s"%(self.name, newvaluetxt));
    else:
      self.value=newvaluetxt
    if self.ebut:
      self.ebut.setEntry(self.value)
  def getValueHexa(self):
    if self.value=="None":
      val=''
    else:
      if self.name=="BC1" or self.name=="BC2":
        val= int(myw.fromms('', self.value))
        val ="0x%x"%val
      elif self.name=="RND1" or self.name=="RND2":
        val= int(myw.frommsRandom('', self.value))
        val ="0x%x"%val
      #von
      #elif self.name=="L0fun1" or self.name=="L0fun2":
      #  val= int(txtproc.log2tab(newvaluetxt))
      #  val ="0x%x"%val
      #  print "getValHexL0f:",val
      else:
        val= self.value
    return val
  def save(self, outfile):
    if self.value!="None":
      outfile.write(self.name+' '+self.value+'\n')
      
class TrgSHR_BCM(TrgSHR):
  """Shared resource for BCMx:
  value:
  - string for BCM1-4(12) or for PF1-4
  """
  def __init__(self, name, helptext=None):
    self.name=name
    self.helptext=helptext
    self.value= 'None'  # name from TDLTUS.BCM_DB/PF_DB (not value!)
    self.used=0   # if used, # of references (filled by savepcfg)
    self.ebut=None
    if self.name[:2]=="PF":
      self.BCMPFitems= TDLTUS.PF_DB
      self.bcmpf="PF"
    else:
      self.BCMPFitems= TDLTUS.BCM_DB
      self.bcmpf="BCM"

  def show(self,master):
    #print self.value
    ix= TDLTUS.findBCMPFname(self.value, self.bcmpf)
    self.ebut= myw.MywxMenu(master, label=self.name,
      helptext=self.helptext, items=self.BCMPFitems,
      bg=COLOR_SHARED,
      defaultinx=ix, side="top", cmd=self.updt)
  def setValue(self, newbcm):
    """ newbcm: the name of the BCmask (given in VALID.BCMASKS file)
    """
    #An example of the BCM syntax check:
    #locbcmasks=[]; 
    #for i in range(ORBITLENGTH):
    #  locbcmasks.append(0)
    #bm=txtproc.BCmask(newvaluetxt) ; errmsg= bm.checkSyntax()
    #ix=TDLTUS.findBCMPFname(newvaluetxt, self.bcmpf)
    #if errmsg:
    #  #print self.name+" syntax:", errmsg
    #  if errmsg[0:4]=='Warn':
    #    print errmsg     
    #    strorNone=newvaluetxt
    #  else:
    #    strorNone=None
    #else:
    #  strorNone=newvaluetxt
    ix= TDLTUS.findBCMPFname(newbcm, self.bcmpf)
    self.value= self.BCMPFitems[ix][0]
    #print "setValueshr:", newbcm, "is: ",ix,self.value
    if self.ebut:
      self.ebut.setEntry(ix)
   
  def updt(self, tkobj=None, event=None):
    #Value= self.ebut.getEntry()
    inx= self.ebut.getIndex()
    print "updtshr:", inx, "is: ",self.BCMPFitems[inx]
    self.setValue(self.BCMPFitems[inx][0])
  def getDefinition(self):
    """rc: None   -if not defined
       "25H2L... "
    or "PF definition string"
    """ 
    #print "getDefinition:",self.value, self.bcmpf
    if self.value==None: return None
    inx= TDLTUS.findBCMPFname(self.value, self.bcmpf)
    return self.BCMPFitems[inx][1]
  def isPFDefined(self,level):
    """ "0 0 0" ->
    PF on given level not defined (i.e. not to be used as veto in class def)
    """
    pfd= string.split(self.getDefinition())
    if len(pfd)!=12:
      IntErr("isPFDefined: bad definition of PF:%s"%self.getDefinition())
      return False
    for ix in range(3):
      if eval(pfd[ix+(3*level)])!=0:
        return True
    return False
  def prtBits(self):
    ix= TDLTUS.findBCMPFname(self.value, self.bcmpf)
    logdef= self.BCMPFitems[ix][1]
    print "prtBits:",self.value, " is ", logdef
    if self.value==None:
      return
    if self.name[:2]=="PF":
      PrintWarning("prtBits() invoked for %s"%self.name)
      return
    bm=txtproc.BCmask(logdef) ; 
    print "prtBits bm:", bm
    errmsg= bm.checkSyntax()
    print "prtBits err:", errmsg
    bits,strl= bm.bcm2bits(0)
    print len(bits),"strl:",strl
  def getBits(self):
    ix= TDLTUS.findBCMPFname(self.value, self.bcmpf)
    logdef= self.BCMPFitems[ix][1]
    if self.name[:2]=="PF":
      PrintWarning("getBits() invoked for %s"%self.name)
      return []   # integer array of bits (in case of BCM)
    bm=txtproc.BCmask(logdef) ; 
    bits,strl= bm.bcm2bits(0)
    return bits

class TdsLtus:
  """TdsLtus keeps information about defined
- Trigger descriptors (also 'possible' TD, i.e. defined by not connected inputs)
- p/f circuits
- LTUs
- BCmasks
"""
  def __init__(self):
    self.validtds= []    # ["MB","SC",...]
    self.pfsnames= []    # ["pf1","pf2",...] von
    self.PF_DB=[['None',None]]       # list of possible [pfName,pfDefinition]
    self.tdinps= []      # [["TD_NAME1", "inp1", ...]...]
    self.tds= []         # list of possible TrgDescriptor objects
    self.ltus= []        # list of possible TrgLTU objects
    self.indets= []      # list of possible TrgInpDet objects
    self.pfs= []         # list of possible TrgPF objects
    self.csName= "csname"
    self.BCM_DB=[['None',None]]       # list of possible [bcmName,bcmDefinition]
    self.loaderrors=""
    self.tdshelptext="""Predefined Trigger descriptors in VALID.DESCRIPTORS file:
"""
    self.pfshelptext="""Possible P/F protection circuits:
"""
    self.bcmhelptext="""BC mask. There are 4/12 BC masks applicable for any Class.
One BC mask contains 3654 bits -one for each BC in the Orbit.
BCMx[bcnumber] (bcnumber=1..3654) bit set to 1(or H) vetoes possible
triggers in that BC.

Predefined BC masks in VALID.BCMASKS file:
"""
    self.load_BCMs()
    self.load_PFs()
    self.ltus= trigdb.readVALIDLTUS()
    f= open(os.path.join(TRGDBDIR, "VALID.CTPINPUTS"),"r")
    # goal: create self.indets list
    # 1. go through VALID.CTPINPUTS, adding l0/1/2 inputs
    # 2. go through l0inputsdb -info for l0 inputs (we need to know
    #    about unconnected inputs to prepare partition). Actions
    #    -add unconfigured L0 input
    #    -warning message 'discrepancy between L0.INPUTS and VALID.CTPINPUTS
    #InName=Det Level Signature  InpNum  Dimnum Configured Edge Delay
    # the lines starting with l0f like:
    # l0fxxx= l0function definition
    # are l0function definitions, and will be saved as "L0FUNS" detector
    # inputs
    #VON l0finputs=[]   #list of possible L0fun*
    PrintError("-------------------------------------------- VALID.CTPINPUTS:",self)
    for line in f.readlines():
      if line[0] == "\n": continue
      if line[0] == "#": continue
      #if line[-1]== '\n': line= line[:-1]    # does not work properly!
      inp1= string.split(line[:-1],'=')
      inpname= string.strip(inp1[0])
      inpAlreadyIn= self.findInput(inpname)
      if inpAlreadyIn:
        #2 definitions with the same name not allowed!
        PrintError("""Trigger input %s already defined. VALID.CTPINPUTS line:
%s discarded.  """%(inpname, line), self)
        continue
      if (line[:3]=='l0f') or (line[:3]=='l0A') or (line[:3]=='l0B'):
        #print "l0fun* special function in VALID.CTPINPUTS:",line
        detname= "L0FUNS"
      else:
        detname= string.strip(string.strip(string.split(inp1[1])[0]))
      newTID=self.findTrgInpDet(detname)
      if not newTID:
        newTID= TrgInpDet(detname)
        self.indets.append(newTID)
      newInput= TrgInput(line[:-1])
      if newInput.name != None:
        newTID.addInput(newInput)
    f.close() 
    # create virtual l0fxxx for all l0[AB]xxx
    newTID=self.findTrgInpDet("L0FUNS")
    if newTID: 
      newTID.makeVirtL0f()
      #newTID.tidprt()
    #
    alll0s= trigdb.TrgL0INPUTS()   
    for l0in in alll0s.ents:
      if l0in[0][0]=='#': continue
      if l0in[9]!='1': continue   # not configured in L0.INPUTS
      #if len(l0in)<13: print "alll0s:", l0in
      inpname= l0in[3]
      detname= l0in[1]
      line= "%s = %s 0 %s 0 %s 0 %s %s"%(inpname, detname, l0in[5], l0in[6], \
        l0in[7], l0in[8]) 
      inpAlreadyIn= self.findInput(inpname)
      newTID=self.findTrgInpDet(detname)
      if inpAlreadyIn:
        #check: detname sig dimnum edge delay
        inpAlreadyIn.check(line)
      else:
        #add not configured input:
        if not newTID:
          newTID= TrgInpDet(detname)
          self.indets.append(newTID)
        # Det Level Signature InpNum Dim Conf Edge Delay
        # 0   1     2         3      4   5    6    7
        newInput= TrgInput(line)
        if newInput.name != None:
          # 0ASL = ACORDE 0 1 0 1 0 0 12
          #print "added L0.INPUT input:",line
          newTID.addInput(newInput)
    #print "\n-----------------------------------------------------TdsLtus:"
    PrintError("\n-----------------------------------------------------TdsLtus:",self)
    #for tid in self.indets: tid.prt() 
    #print "---------------------------------------------------- eof TdsLtus"
    #print "TdsLtus:",self.tds
    #print "TdsLtus:\n",self.tdinps,"\n",self.getltunames(),"\n",self.pfs
  def initTDS(self,reload=None):
    #if reload: 
    #  self.tds= []         # list of possible TrgDescriptor objects
    #  self.validtds= []    # ["MB","SC",...]
    f= open(os.path.join(TRGDBDIR, "VALID.DESCRIPTORS"),"r")
    PrintError("\n-----------------------------------------------VALID.DESCRIPTORS:",self)
    baddescriptors= 0 ; notreadydescriptors= 0
    for line in f.readlines():
      self.tdshelptext=self.tdshelptext+line
      if len(line) <= 3: continue
      if line[0] == "\n": continue
      if line[0] == " ": continue
      if line[0] == "#": continue
      #remove text after #:
      ihk= string.find(line,'#')
      if ihk>0:
        line= line[:ihk]
      stripedline= string.strip(line)
      self.tdinps.append(string.split(stripedline))
      #
      #print "initTDS:%s:"%line, "len:", len(line)
      tdname= string.split(stripedline)[0]    
      tdinps= string.split(stripedline)[1:]
      #if tdname=="DDG1":
      #  import pdb ; pdb.set_trace()
      newtrgdes= TrgDescriptor(tdname, tdinps)
      #if tdname=="DDG1":
      #  print "initTDS:",line
      #  newtrgdes.trdprt()
      if newtrgdes.name != None:   # usable (but can have unconneted inputs)
        if newtrgdes.allinputsvalid==False:
          PrintError(newtrgdes.loaderrors, self)
          notreadydescriptors= notreadydescriptors+1
        self.tds.append(newtrgdes)
        self.validtds.append(string.split(stripedline)[0])    
      else:
        PrintError(newtrgdes.loaderrors, self)
        baddescriptors= baddescriptors+1
    f.close() 
    PrintError("Bad descriptors:%d not ready:%d"%\
      (baddescriptors,notreadydescriptors), self)
    self.tdsbuthelptext="""
LEFT mouse button   -> add/remove Trigger class in/from this cluster.
MIDDLE mouse button -> delete this cluster.

LEFT button displays 2 lists (separated by ------------):
l. The list of possible trigger descriptor names - one of them has
   to be choosen to ADD new class.
   EMPTY option menas: no trigger inputs for this class.
2. The list of classes (class names) already assigned to that cluster.
   By choosing one of the them, the corresponding class is REMOVED
   from this cluster.

""" + self.tdshelptext
  def findInput(self, nameintds):
    """ nameintds: if proceeded with * -> inverted
    return: None if not found, or instance of TrgInput object found
    """
    #print "TdsLtus.findInput:", nameintds
    if nameintds[0]=='*':
      name=nameintds[1:]
    else:
      name=nameintds
    for det in self.indets:
      inp= det.findInput(name)
      if inp: return inp
    return None
  def findTrgInpDet(self, name):
    for det in self.indets:
      if det.name==name: return det
    return None
  def findPF(self, name):
    for pf in self.pfs:
      if pf.name == name: return pf
    return None
  def findTD(self, name):
    for td in self.tds:
      if td.name == name: return td
    return None
  def findLTU(self, name):
    for td in self.ltus:
      if td.name == name: return td
    return None
  def getltunames(self):
    ln=[]
    for l in self.ltus:
      ln.append(l.name)
    return ln
  def getInpDets(self, inpname):
    """rc: return the list of input Detectors.
    -The list contains just 1 item if simple input.
    -empty list if error
    """
    odets=[]
    for det in self.indets:
      inp= det.findInput(inpname)
      if inp:
        #print "getInpDets2:",inp.name, det 
        #if det.name=="L0FUNS":
        if inp.l0fdefinition:
          #os= string.split(inp.l0fdefinition,"()|&~")
          os= inp.l0fdefinition
          for cop in "()|&~":
            os=os.replace(cop," ")
          os= string.split(os)
          #print "os:",os
          for inpfname in os:
            inp= TDLTUS.findInput(inpfname)
            if inp:
              odets.append(inp.detectorname)
            else:
              PrintError("%s not found"%inpfname)
          return odets
        else:
          return [det.name]
    return []   # inpname doesn't belong to any detector
  def doTDSbut(self, master, cmd, trdeactive):
    #for ix in range(len(self.validtds)):
    #  if self.validtds[ix]== trdeactive.name:
    #    break
    #xmenu= myw.MywxMenu(master,cmd=cmd,defaultinx=ix,
    #  items=self.validtds, label="Trigger descriptor:")
    xmenu= myw.MywLabel(master, label="Trigger descriptor: "+trdeactive.name)
    return xmenu
  def load_PFs(self):
    PrintError("----------------------------------------------- TRIGGER.PFS:",self)
    f= open(os.path.join(TRGDBDIR, "TRIGGER.PFS"),"r")
    linen=0
    for line in f.readlines():
      linen= linen+1
      self.pfshelptext=self.pfshelptext+line
      if line[0] == "#": continue
      if line[0] == "\n": continue
      (bcm_name, bcm_definition) = string.split(line," ",1)
      bcm_definition= string.strip(bcm_definition)
      #errmsg= bm.checkSyntax()
      errmsg=None
      if errmsg!= None:
        PrintError(errmsg, self)
      else:
        self.PF_DB.append([bcm_name,bcm_definition]);	
    f.close()
  def load_BCMs(self):
    PrintError("----------------------------------------------- VALID.BCMASKS:",self)
    f= open(os.path.join(TRGDBDIR, "VALID.BCMASKS"),"r")
    linen=0
    for line in f.readlines():
      linen= linen+1
      if len(line) > 100:
        rest= line
        while 1 :
          self.bcmhelptext=self.bcmhelptext+ rest[:100] + "\n"
          rest= rest[100:]
          if len(rest)<100:
            self.bcmhelptext=self.bcmhelptext+ rest +"\n"
            break
      else:
        self.bcmhelptext=self.bcmhelptext+line
      if line[0] == "#": 
        if linen==1:   # first line is giving collision schedule name
          # 525ns_80b+4small_68_64_32_8bpi14inj-3556
          lione= string.split(line)
          if len(lione)>1:
            self.csName= lione[1]
        continue
      if line[0] == "\n": continue
      (bcm_name, bcm_definition) = string.split(line)
      #print "load_BCMs:",bcm_name, bcm_definition
      bm=txtproc.BCmask(bcm_definition) ; 
      #print "prtBits bm:", bm
      errmsg= bm.checkSyntax()
      if errmsg!= None:
        PrintError(errmsg, self)
      else:
        self.BCM_DB.append([bcm_name,bcm_definition]);	
    f.close()
  def findBCMPFname(self,bcmname, bcmpf):
    """
    In: "BCM..." or "PF..."
    rc: None or index pointing to: PF_DB or BCM_DB[rc][]
    """
    if bcmpf=="BCM":
      selfdb= self.BCM_DB
    else:
      selfdb= self.PF_DB
    rv=0   # 1st item is ['None', None]
    for i in range(len(selfdb)):
      #if selfdb[i][1]==None: continue
      if selfdb[i][0]==bcmname:
        #rv= self.BCM_DB[i][0]
        rv= i
        break
    if rv==None:
      print "BCM/PF %s not in VALID.BCMASKS/.PFS"%(bcmname)
    #print "findBCMPFname:", bcmname, rv
    return rv

os.chdir(trigdb.TRGWORKDIR)
print "working dir:",trigdb.TRGWORKDIR, "\nTRGDBDIR:", TRGDBDIR, \
  "\nCFGDIR:", CFGDIR
TDLTUS= TdsLtus()
TDLTUS.initTDS() 
#shared resources (order is important for savepcfg, getTXTbcrnd):
SHRRSRCS= [
  TrgSHR('RND1',TrgSHR.RNDxHelp+myw.frommsRandomHelp),
  TrgSHR('RND2',TrgSHR.RNDxHelp+myw.frommsRandomHelp),        # 2 RND inputs
  TrgSHR('BC1', TrgSHR.BCxHelp+myw.frommsHelp),
  TrgSHR('BC2',TrgSHR.BCxHelp+myw.frommsHelp)] # 2 BC scaled down inputs 
  #TrgSHR('L0fun1',TrgSHR.L0FUNxHelp),  see TrgPartition.l0funs
  #TrgSHR('L0fun2',TrgSHR.L0FUNxHelp),
for ix in range(BCM_NUMBER):         #4(12) BCM vetos, start from BCMASKS_START
  SHRRSRCS.append(TrgSHR_BCM('BCM%d'%(ix+1),TDLTUS.bcmhelptext))
for ix in range(PF_NUMBER):         #4 PFs, start from PFS_START=16/8
  SHRRSRCS.append(TrgSHR_BCM('PF%d'%(ix+1),TDLTUS.pfshelptext))

def findSHR(shrname):
  for ix in range(len(SHRRSRCS)):
    if SHRRSRCS[ix].name == shrname:
      return SHRRSRCS[ix]
  return None

class TrgClass:
  #def __init__(self, td=None, L0pr='0', bcms=[0,0,0,0],pf=None):
  def __init__(self, clstring, mycluster, clusname4clsname):
    """ mycluster: 
    None -take clusname4clsname for cluster part of cluster name
          -case when TrgClass instantiated from TrgPartition.loadfile
    TrgCluster - take mycluster.name ...
          -case when new TrgClass added interactively to cluster

    ret: self.trde: None -serious problem with this class (syntax...)
         self.trde...    -class is available with current config
    """
    self.tlw=None     # i.e. 'properties window' is hidden
    self.clsbut=None  #i.e. button not created (in Cluster window)=
                      # class not assigned to any cluster
    tdsname,pars=Parse1(clstring)
    if pars==None:
      self.trde= None
      return
    self.trde= TDLTUS.findTD(tdsname) # pointer to trigger descriptor or None
    if not self.trde:
      PrintError("Unknown Trigger descriptor:"+tdsname)
      return
    if mycluster==None:
      cluspart= clusname4clsname
    else:
      cluspart= mycluster.name
    self.updateClassName(cluspart,composeName=None) # self.clsnamepart[] only
    cn_name=None
    self.clanumlog= 0   # 1..50 assigned in TrgPartition.loadfile()
    #print "TrgClass:",self.clsname,pars
    #print "TrgClass:", self.clsname," trde:" ; if self.trde: self.trde.prt()
    #VON self.clsl0funs=[None,None]
    self.setCluster(mycluster)   #
    self.L0pr='0';   # n, 0xhexa, n%, or syncdg symb. name
    self.bcms=[0,0,0,0,0,0,0,0,0,0,0,0]
    self.optinps=[0,0,0,0]   # rnd1, rnd2, bc1, bc2
    self.pfs=[0,0,0,0]   # [0,1,0,0]: TrgSHR[PFS_START+1].getDefinition()
    self.allrare=1   # 0-> rare   1-> all
    self.classgroup=0   # 0..9    0: class is always active 9: never active?
    # 1..9: class is active only in certain time slot (1..9)
    #for k,vv in pars.iteritems():
    #print "trgclassinit:", pars
    for k in pars.keys():
      vv=pars[k]
      if k=='cn':
        cn_name= pars['cn']
        #self.clsname= pars['cn']
        continue
      if k=='cg':
        self.classgroup= int(pars['cg'])
        continue
      if k=='L0pr':
        if vv!='ON': self.L0pr=vv
        continue
      if k=='bc1':
        self.optinps[2]=1; continue
      if k=='bc2':
        self.optinps[3]=1; continue
      if k=='rnd1':
        self.optinps[0]=1; continue
      if k=='rnd2':
        self.optinps[1]=1; continue
      if k[:3]=='bcm':
        PrintWarning("bcm in .partition file converted to BCM (valid from 11.9.2008)")
        k= 'BCM'+ k[3:]
      if k[:3]=='BCM':
        bcmix= int(k[3:])   # 1..12
        if (bcmix>=1) and (bcmix<=12):
          self.bcms[bcmix-1]= 1
          x=self.get_clsnamepart1(k)
          if x==None:
            PrintError("Undefined BCM%s in:"%bcmix,clstring)
            self.trde= None
            break
          self.clsnamepart[1]= x
          continue
        else:
          PrintError("Bad BCM%s in:"%bcmix,clstring)
          self.trde= None
          break
      if k[:2]=='PF':
        bcmix= int(k[2:])   # 1..4
        if (bcmix>=1) and (bcmix<=4):
          self.pfs[bcmix-1]= 1
          x=self.get_clsnamepart2(k)
          if x==None:
            PrintError("Undefined PF%s in:"%bcmix,clstring)
            self.trde= None
            break
          self.clsnamepart[2]= x
          continue
        else:
          PrintError("Bad PF%s (PF1..4 expected) in:"%bcmix,clstring)
          self.trde= None
          break
      if k=='all':
        self.allrare=1; continue
      if k=='rare':
        self.allrare=0; continue
      #if vv=='ON':
      #  pf= TDLTUS.findPF(k)             # find pfname
      #  if pf: self.pfs.append(pf)
      #  else:
      #    print "Unknown P/F circuit:", k
      #  continue
      self.trde= None
      PrintError("Bad TD:"+clstring); break
    # leave None if not defined in .partition (from 3.6.2012):
    if (cn_name!=None) and (cn_name!=self.getclsname()):
      print "TrgClass:", cn_name, "default name:", self.getclsname()
      self.clsname=cn_name
  def get_clsnamepart1(self, k):
    bcm= findSHR(k)
    if bcm.value==None: return None
    # bcmEMPTY -> E, bcmS -> S...
    x= bcm.value[3:]
    #print "get_clsnamepart1:", x
    if x=="EMPTY": x="E"
    return x
  def get_clsnamepart2(self, k):
    bcm= findSHR(k)
    if bcm.value==None: return None
    # bcmEMPTY -> E, bcmS -> S...
    x= bcm.value
    #print "get_clsnamepart2:", x
    return x
  def prtClass(self):
    lfs=''
    for ix in (0,1):
      if self.trde.l0funs[ix]:
        lfs= lfs + "l0f%d:0x%x= %s "%(ix, self.trde.l0funs[ix][0],
          self.trde.l0funs[ix][1].l0fdefinition)
    print "prtClass:",self.getclsname(), " clsl0funs:", lfs 
    lfs=''
    for ix in (0,1):
      if self.trde.l0funs34[ix]:
        lfs= lfs + "l0f%d:0x%x= %s\n"%(ix, str(self.trde.l0funs[ix][0]),
          self.trde.l0funs[ix][1].l0fdefinition)
    print "prtClass:",self.getclsname(), " clsl0funs34:", lfs 
    print self.trde.getInpDets()
  def updateClassName(self, cluspart,composeName="yes"):
    #needed for new classes (after click on cluster button)
    if composeName==None:
      self.clsname=''; 
      p0= self.trde.name.replace('D','C',1)   #default class name
      if p0=='CEMPTY': p0='CTRUE'
      #if cluspart!='CENT ALL MUON TPC FAST':
      if string.find('CENT ALL ALLNOTRD MUON TPC FAST FASTNOTRD CFAST CENTNOTRD',cluspart)<0:
        # only warning pprinted:
        print "Strange cluster name:%s"%cluspart
      self.clsnamepart=[p0, "ABCE", "NOPF", cluspart]
    else:
      self.clsname= self.buildname()
    #print("updateClassName:", self.clsname, self.clsnamepart)
  def buildname(self,name_part=None):
    if name_part==None: name_part= self.clsnamepart[0]
    return "%s-%s-%s-%s"%(name_part, self.clsnamepart[1],\
      self.clsnamepart[2], self.clsnamepart[3])
  def updateClassName1(self):
    dname= self.trde.name
    ixdash= self.clsname.find('-')
    self.clsname= 'C'+dname[1:]+self.clsname[ixdash:]
  def setCluster(self, mycluster):
    """ invoked:
    1. new TrgCluster -TrgParttion.loadfile()
    2. when class added -TrgClass.__init__()
    3. when Trigger desc. changed -TrgClass.modtds
    """
    self.mycluster= mycluster   # pointer to controlled cluster
    #print "setCluster:" ; self.trde.prt()
    if mycluster:
      if self.trde:
        lfs,error=self.mycluster.partition.allocShared(self.trde.l0funs34, lf34=1)
        if error:
          PrintError("setCluster: Class %s: %s"%(self.getclsname(), error))
        lfs,error=self.mycluster.partition.allocShared(self.trde.l0funs)
        if error:
          PrintError("setCluster:Class %s: %s"%(self.getclsname(), error))
        else:
          #print "setCluster: allocating:", lfs
          #self.clsl0funs= lfs   # INCORRECT-DEEP COPY IS MUST:
          #VON self.clsl0funs[0]= lfs[0]   #these should be used for l0fun
          #VON self.clsl0funs[1]= lfs[1]
          pass 
          # bits calculation in CLA line
  def getrcfgdesname(self):
    """return: rcfgdescname,oins
    rcfgdesname: "EMPTY_BC1_RND2"
    oins: " BC1 RND2"
    """
    nm= self.trde.name
    oins=''
    app= self.getTXTbcrnd(text='rcfgdesname')
    if len(app)>0:
      nm= nm+app
      oins=string.replace(app,'_',' ')
    return nm,oins
  def getTXTpfs(self):
    pfsl=''
    for ibcm in range(len(self.pfs)):
      if self.pfs[ibcm]!=0:
        pfsl=pfsl+"PF%d,"%(ibcm+1)
    return pfsl
  def getPFs(self):
    pfs='{'   # get P/Fs:
    if self.pfs!=None:
      for ixpfpc in range(len(self.pfs)):
        if self.pfs[ixpfpc]==1:
          pfs=pfs+"PF%d"%(ixpfpc+1)+','
    if pfs=='{': pfs='{NONE,'
    return pfs[:-1]+'}'
  def getTXTbcrnd(self, text=None):
    """ rc: 
    "bc1,rnd1,...,"                           (text=='saving')        
    "_BC1_BC2_RND1_RND2"                       (text=='rcfgdesname')
    {"bc1":(hwvalue_text, human_text),...}    (other text) 
    """
    bcrnd=''  # get bc,rnd
    rcfgdesname=''
    dict={}
    for ibcm in range(len(self.optinps)):     # opt. inputs
      if self.optinps[ibcm]!=0:
        val=SHRRSRCS[ibcm].value
        if ibcm<2:
          bcrndname="rnd%1d"%(ibcm+1)
          hwval= myw.frommsRandom("", val)
        else:
          bcrndname="bc%1d"%(ibcm-1)
          hwval= myw.fromms("", val)
        bcrnd=bcrnd+"%s,"%(bcrndname)
        rcfgdesname= rcfgdesname+'_'+string.upper(bcrndname)
        dict[bcrndname]=(hwval, val)
    if text=="saving":
      return bcrnd
    elif text=="rcfgdesname":
      return rcfgdesname
    else:
      return dict
  def getTXTbcms(self):
    bcms=''
    for ibcm in range(len(self.bcms)):        # bcms
      if self.bcms[ibcm]!=0:
        bcms=bcms+"BCM%d,"%(ibcm+1)
    return bcms
  def getBCMASKs(self):
    bcms='{'
    for ibcm in range(len(self.bcms)):        # bcms
      if self.bcms[ibcm]!=0:
        if SHRRSRCS[ibcm+BCMASKS_START].getDefinition()==None:
          PrintError("Undefined %s referenced by class %s, reference discarded(in .rcfg)"%\
            (SHRRSRCS[ibcm+BCMASKS_START].name, self.getclsname()))
          continue   
        bcms=bcms+"BCM%d,"%(ibcm+1)   #firmAC
    if len(bcms)==1: bcms=bcms+','
    if bcms=='{,': bcms='{NONE,'
    return bcms[:-1]+'}'
  def getTXTcls(self):
    """
    default: all/rare: if all -> '' to ouptut
             L0pr='0'         -> '' to output
             cn=tdsname       -> ''
    """
    CLSNAME="skip"   # skip or oldway
    # prepare pfs, bcm*, allrare (3x empty or finished with ',')
    pfs= self.getTXTpfs()                     # pfs
    bcrnd= self.getTXTbcrnd(text='saving')
    bcms= self.getTXTbcms()
    if self.allrare==1:                       # all/rare
      #allrare='all,'
      allrare=''
    else:
      allrare='rare,'
    if self.L0pr=='0':
      l0pr=''
    else:
      l0pr="L0pr=%s,"%self.L0pr
    if self.classgroup==0:
      cg=''
    else:
      cg="cg=%d,"%self.classgroup
    if self.trde == None:
      trdename="None"
    else:
      trdename= self.trde.name
    if CLSNAME=="oldway":   # was till 3.6.2012:
      cn="cn=%s,"%self.getclsname()
    elif CLSNAME=="skip": # from 3.6.2012:
      if self.clsname=='':
        cn=''
      else:
        cn="cn=%s,"%self.clsname
    rctxt=trdename+"(%s%s%s%s%s%s%s"%(cn,l0pr,pfs,bcrnd,bcms,allrare,cg)
    if rctxt[-1]==',':
      rctxt=rctxt[:-1]+')'
    else:
      rctxt=rctxt+')'
    #print "TrgClass.getTXT:",rctxt
    return rctxt
  def hide(self):
    #print "TrgClass.hide",self.tlw
    if self.tlw!=None:
      self.tlw.destroy()
      self.tlw=None
  def showClass_BCMPF(self, bcmorpf):
    tdscolor=None; onoffs=[]
    if bcmorpf=="BCM":
      bcmpfs= self.bcms
      label="BCmasks"     # must be (checked later)
      items=['BCM1','BCM2','BCM3','BCM4','BCM5','BCM6','BCM7','BCM8','BCM9','BCM10','BCM11','BCM12']
      helptextma="""BC masks. 4/12 possible masks can be choosen 
simultaneously: BCM1 BCM2 BCM3 or BCM4 .. BCM12
"""
    else:
      bcmpfs= self.pfs
      label="PFs"
      items=['PF1','PF2','PF3','PF4']
      helptextma="""PF protection circuits.
"""
    onoffs=[]
    for ixoi in range(len(bcmpfs)):
      if bcmpfs[ixoi]==1:
        onoffs.append(1)
        tdscolor= COLOR_TDSPFS
      else:
        onoffs.append(0)
    mask_pf_but=myw.MywMenuList(self.tlw,
      label=label, cmd=self.modadi, showactive=1,
      items=items, defaults=onoffs, side=TOP, helptext=helptextma)
    if bcmorpf=="BCM":
      self.bcmbut= mask_pf_but
      pass
    else:
      self.pfsbut= mask_pf_but
      if tdscolor: mask_pf_but.setColor(tdscolor)
    return
  def showClass(self):
    """ show attributes of the class
    """
    #deactivate corresponding class button:
    self.clsbut.disable()
    self.tlw=Toplevel()
    self.tlw.bind("<Destroy>", self.hideclass)
    self.tlw.title(self.mycluster.name+":Class "+self.getclsname())
    #print "TrgClass.show.len(pfs):",len(self.pfs)
    if self.clsname=='': defclsname=' '
    else: defclsname= self.clsname
    self.clsnameent= myw.MywEntry(self.tlw, label="Class name:",
      helptext="""Class name is user defined identifier for this class.
It is not referred from any part of CTP configuration.
""", bind="lr",
      defvalue= defclsname, cmdlabel=self.clsnamecmd, side=TOP)
    #P/F circuits:
    self.showClass_BCMPF("PF")
    vonpf="""
    onoffs=[]; tdscolor=None
    helptext1=TDLTUS.pfshelptext   # +"Active P/F for this TDS:"
    for ixpfsn in range(len(TDLTUS.pfsnames)):   #for all PFS
      onoffs.append(0)
      for pf in self.pfs:
        if pf.name==TDLTUS.pfsnames[ixpfsn]:
          onoffs[-1]=1
          #helptext1= helptext1+" "+ pf.name
          tdscolor= COLOR_TDSPFS
          break;
    self.pfsbut=myw.MywMenuList(self.tlw,
      label="PFs", cmd=self.modpfs, showactive=1,
      items=TDLTUS.pfsnames, defaults=onoffs, side=TOP,
      helptext=helptext1)
"""
    # bc1,bc2,rnd1,rnd2:
    helptextoi="""Optional inputs. 
- Scaled-down BC trigger. 2 possible inputs: bc1 or bc2
- Random trigger. 2 possible inputs: rnd1 or rnd1
NOTE:
   ONLY DEEFINED RESOURCES (click Show->Shared resources) CAN BE CHOOSEN
"""
    onoffs=[]
    for ixoi in range(len(self.optinps)):
      if self.optinps[ixoi]==1:
        onoffs.append(1)
      else:
        onoffs.append(0)
    self.adibut=myw.MywMenuList(self.tlw,
      label="Optional inputs", cmd=self.modadi, showactive=1,
      items=['rnd1','rnd2', 'bc1','bc2'], defaults=onoffs, side=TOP,
      helptext=helptextoi)
    # BCM1-4/12
    self.showClass_BCMPF("BCM")
    # rare
    self.rarebut= myw.MywButton(self.tlw, label="rare", side= TOP,
      cmd=self.rarecmd, helptext="blabla")
    self.rarecmd(); self.rarecmd()
    # classgroup
    cgitems= []
    for ix in range(10): cgitems.append(("%d"%ix, "%d"%ix, self.cgupdate))
    self.cgbut= myw.MywxMenu(self.tlw, label=
      'Class group:', defaultinx=self.classgroup,side=TOP,
      helptext="""Class group:
0 class is always active
1..9 class is active only during the given time slot (in seconds). 
   In one time, there are active:
   - not grouped classes (class group 0) and
   - ONLY 1 Classgroup (0<class group<=9).
   9 time slots are defined in $dbctp/ctp.cfg in TIMESHARING line

Example:
0 1 1 1 1 1 6 7 8 9  
Groups 1 2 3 4 5 become 'not active' after 1 second of active time.
Only one group is active in one time.
Group 6 becomes 'not active' after 6 seconds of active time
...

Currently, these times [in seconds] are defined for groups 1..9:
%s
"""%str(clgtimes),
      items=cgitems)
    # L0prescaler
    self.l0prbut= myw.MywEntry(self.tlw, label="L0prescaler:",
      helptext=myw.frommsL0prHelp, bind="lr",
      defvalue= self.L0pr, cmdlabel=self.l0prcmd, side=TOP)
    #self.l0prbut.bind("<Leave>", self.l0prcmd)
    #self.l0prbut.bind("<Key-Return>", self.l0prcmd)
    # TDSxx:
    #vonself.tdsbut= TDLTUS.doTDSbut(self.tlw,self.modtds, self.trde)
    TDLTUS.doTDSbut(self.tlw,self.modtds, self.trde)
    # class name:
  def applyClassFilter(self,filter):
    """
    msg,err= trgclass.applyClassFilter(filter)
    rc: msg,err where
    err: skipped
         ok   replacement done (new descriptor, class name changed)
         "" nothing done (i.e. 2. class skip' to be tried...)
         error message (e.g. replacement not found)
    """
    msg= self.trde.name
    repl= filter.getReplacement(self.trde.name)
    if repl==None:   # not found
      return msg,""
    if repl=="":   # skip this class
      return msg,"skipped"
    # repl: name of the descriptor to be used instead of current one
    #print "applyClassFilter:",self.trde.name,"->",repl
    msg= msg+' -> '+repl
    # change descriptor:
    trdenew= TDLTUS.findTD(repl) # pointer to trigger descriptor or None
    if trdenew==None:
      err="unknown replacement %s"%repl
      return msg,err
    self.trde= trdenew
    # modify class name:
    self.updateClassName1()
    return msg, "ok"
  def cgupdate(self, event=None):
    self.classgroup= int(self.cgbut.getEntry())
  def l0prcmd(self, event=None):
    #print "l0prcmd.event:",dir(event),event['keycode']
    #for k in dir(event).keys():
    #  print k,":",event[k]
    #print "l0prcmd.event:",dir(event),event.keycode,event.keysym
    l0txt= self.l0prbut.getEntry()
    # check if syncdg:
    strorNone= self.mycluster.partition.sdgs.find(l0txt)
    if strorNone==None:   # is it syncdg name?
      strorNone= myw.frommsL0pr('', l0txt)
    if strorNone:
      self.L0pr=l0txt
    else:
      print l0txt," -bad syntax, L0 prescaler value set to 0 (no downscaling)"
      self.L0pr='0'
    #try:
    #  self.L0pr=int(l0txt)
    #except:
    # if l0txt[:3]=='0x8':
    #   self.L0pr=eval('0x'+l0txt[3:]) +0x40000000+0x40000000
    #   print "L0 prescaler value set to %d(0x%x)"%(self.L0pr,self.L0pr)
    # else:
    #   print l0txt," -bad integer, L0 prescaler value set to 0"
    #   self.L0pr=0
  def getclsname(self, clustpart='yes'):
    """
    cluspart: None -trailing cluster part not given in returned name
    new (from 11.5.2012:
    return: buit-name i.e. built from clsnamepart[]
    """
    if self.clsname!='': 
      return self.clsname
      #return self.trde.name   # DESCRIPTOR name if not exists
    #r= str(self.clsname)
    r= self.buildname()
    #print "getclsname:", clustpart
    if clustpart==None: # cut off '-clusterName' if present
      for i in range(len(r),0,-1):
        if r[i-1]=='-': 
          r= r[:i-1] ; break
    return r
  def clsnamecmd(self, event=None):
    newcn= self.clsnameent.getEntry().strip()
    if len(newcn.split())> 1:
      PrintError("'"+newcn+"'"+" is ncorrect. Class name cannot contain spaces")
      self.clsnameent.setEntry(self.clsname)
      return
    self.clsname= newcn
    self.clsnameent.setEntry(self.clsname)
    self.clsbut.setLabel(self.getclsname())
    self.tlw.title(self.mycluster.name+":Class "+self.getclsname())
  def modadi(self, adibutinst, ixoi):
    #print "modadi:",ixoi, adibutinst
    if adibutinst.getEntry(ixoi)==1:
      onoff=1
    else:
      onoff=0
    shrname= findSHR(string.upper(adibutinst.getItem(ixoi)))
    #print "modadi2:",shrname
    if shrname==None:
      IntErr("modadi:"+str(ixoi))
      return
    #print "modadi:", shrname.name, shrname.value
    #check if shared resource defined:
    if shrname.value == 'None':
      PrintError("Shared resource %s not defined, define it before making the reference"%(shrname.name))
      if onoff==1: onoff=0   # reset usage
      adibutinst.setEntry(ixoi, onoff)
    if adibutinst==self.adibut:
      self.optinps[ixoi]=onoff
    elif adibutinst==self.pfsbut:
      self.pfs[ixoi]=onoff
    elif adibutinst==self.bcmbut:
      self.bcms[ixoi]=onoff
    else:
      IntErr("modadi: not optinps/bcm/pf:"+str(ixoi))
  def rarecmd(self):
    #print "rarecmd:",self.allrare
    if self.allrare==1:
      self.allrare=0
      self.rarebut.config(text="Rare")
      #self.rarebut.config(bg=myw.COLOR_WARNING)
      self.rarebut.newhelp("""
Rare veto is ON, i.e. this class is sensitive to global All/Rare flag
""")
    else:
      self.allrare=1
      self.rarebut.config(text="All")
      self.rarebut.newhelp("""
Rare veto is OFF, i.e. this class is not sensitive to global All/Rare flag
""")
    von="""
  def modpfs(self, mli, ix):
    #update P/F info
    newasspfs=[]
    for ixass in range(len(TDLTUS.pfs)):
      if mli.getEntry(ixass)==1:
        newasspfs.append(TDLTUS.pfs[ixass])
    if len(newasspfs)==0: 
      mli.resetColor()
    else:
      mli.setColor(COLOR_TDSPFS)
    self.pfs= newasspfs
    self.modButton()
"""
  def modtds(self, mywxinstance, ix):
    self.trde= TDLTUS.findTD(self.tdsbut.getEntry())
    #if self.clsname=='':
    #print "modtds0: %s, modifying class name label"%self.clsname
    self.clsbut.setLabel(self.getclsname(clustpart=None))
    self.setCluster(self.mycluster) 
    self.mycluster.refreshTDShead()
  def modButton(self):
    if ones(self.pfs)>0:   
      tdscolor= COLOR_TDSPFS
    else:
      tdscolor= COLOR_NORMAL
    self.clsbut.setColor(tdscolor)
    self.clsbut.setLabel(self.getclsname(clustpart=None))
  def hideclass(self,event):
    #print "hideclass:", self.clsbut,event
    self.tlw=None
    # activate corresponding class button:
    if self.clsbut: self.clsbut.enable()
class TrgCluster:
  def __init__(self, clusname, partition=None, cls=None, outdets=None):
    # cls: list of classes
    self.partition= partition   # to which it belongs to
    if clusname==None:   # invent non ambiguous name
      clusname= self.partition.inventClusterName()
    self.name= clusname
    if cls==None: cls=[]    # list of TrgClass objects
    if outdets==None: outdets=[]    # list of TrgClass objects
    self.outdets= outdets  # list of TrgLTU objects
    self.clfr=None         # cluster not shown
    self.cls=[]
    self.tdshead=None
    for cls1 in cls:
      self.cls.append(cls1)
      if self.cls[-1].mycluster != None:
        IntErr("TrgCluster: assigning already assigned class")
        self.prt()
        self.clusname=None
        return
      self.cls[-1].setCluster(self)
  def getCLS(self):
    """ret. string: 'tds(L0pr=8ms, pf1,all)'
    """
    cls=''
    for ixtd in range(len(self.cls)):
      cls=cls+self.cls[ixtd].getTXTcls()+' '
    #print "getCLS():",tds
    return string.strip(cls)
  def prt(self):
    tds=self.getCLS()
    print "---Cluster:",self.name
    ltus=''
    for ltu in self.outdets:
      ltus=ltus+ltu.name+' '
    print "    TDS:",tds
    print "   LTUS:",ltus
  def hide(self):
    # first destroy all the Class' Toplevel windows
    for cls in self.cls: 
      #print "TrgCluster.hide:",cls.getclsname()
      cls.hide()
      cls.clsbut=None
    self.clfr.destroy()
    self.clfr=None
  def doTDSbuttons(self):
    for ixtds in range(len(self.cls)):   #for all the classes
      #tds.prt()
      self.doCLSbutton(ixtds)
  def getFreeFrame(self):
    for ixlinefrm in range(len(self.tdslines)):
      if len(self.tdslines[ixlinefrm])==6: continue
      frm= myw.MywFrame(self.tdslines[ixlinefrm][0],side=LEFT, relief=FLAT)
      self.tdslines[ixlinefrm].append(frm)
      return frm
    linefrm= [myw.MywFrame(self.tdsfr,side=TOP, relief=FLAT)]
    frm= myw.MywFrame(linefrm[0],side=LEFT)
    linefrm.append(frm)
    self.tdslines.append(linefrm)
    return frm
  def doCLSbutton(self, ixcls):
    """
    self.cls[ixcls] exists already
    """
    #print "doCLSbutton:",ixcls #, self.cls[ixcls].pfs
    onoffs=[]; tdscolor=None
    htclsbut="""Trigger class.
Click on this button to modify class definition. To add/remove class click
on Cluster button (the leftmost one).
In addition to the shown class name, the additional information
about each class can be arranged by pressing:
TopMenu->Show->classes'

"""+TDLTUS.tdshelptext
    if self.cls[ixcls].clsbut!= None:
      IntErr("TrgClass.clsbut != None")
    #self.cls[ixcls].clsbut= myw.MywButton(self.tdsfr,
    self.cls[ixcls].clsbut= myw.MywButton(self.getFreeFrame(),
      #label=self.cls[ixcls].trde.name, side=LEFT,
      label=self.cls[ixcls].getclsname(clustpart=None), side=LEFT,
      cmd=myw.curry(self.modcls,self.cls[ixcls]),
      helptext=htclsbut)
    #self.cls[ixcls].clsbut.bind("<Button-2>",
    #  self.delcls)
    # myw.curry(self.delcls, self.cls[ixcls]))
    self.cls[ixcls].modButton()
  def refreshClassNames(self):
    # cluster was renamed:
    for clsactive in self.cls:
      clsactive.updateClassName(self.name)
      if clsactive.clsbut:
        clsactive.clsbut.setLabel(clsactive.getclsname())
      else:
        print "? refreshClassNames: clsactive.clsbut empty?..."
  def doTDShead(self):
    if self.tdshead: self.tdshead.destroy()
    self.tdshead=myw.MywMenuList(self.tdsheadfr,
      label="Cluster "+self.name+"\n classes:", 
      side=LEFT, bg=COLOR_CLUSTER,
      defaults= self.tdsonoffs, items= self.tdsnames,
      cmd=self.modtdsl, helptext=TDLTUS.tdsbuthelptext)
    self.tdshead.checkbut.bind("<Button-2>",self.delete)
    self.tdshead.checkbut.bind("<Button-1>", self.setActiveCluster)
  def refreshTDShead(self):
    self.doTDSheadLists()
    self.doTDShead()
  def refreshTDSheadLabel(self):
    label="Cluster "+self.name+"\n classes:" 
    self.tdshead.setLabel(label)
  def doTDSheadLists(self):
    self.tdsonoffs=[]; self.tdsnames=[]; max=0;
    for tds in TDLTUS.tds:   # first add TD (for adding new classes)
      self.tdsonoffs.append(0)
      self.tdsnames.append(tds.name)
      self.tdsnames.sort()
      max=max+1
    self.tdsonoffs.append(0); self.tdsnames.append('-'); max=max+1 # separator
    self.maxTD=max-1   # 0,1,2,...
    for clsactive in self.cls:   # then add classes
      self.tdsonoffs.append(1)
      #self.tdsnames.append(clsactive.trde.name)  better class names
      #print "doTDSheadLists: adding 1 ", clsactive.getclsname()
      self.tdsnames.append(clsactive.getclsname())
  def show(self,master):
    self.clfr= myw.MywFrame(master,side=BOTTOM, bg=COLOR_CLUSTER)
    # prepare allclassesN/5 Frames (each for 5 classes):
    #print "TrgCluster.show:", len(self.cls)
    self.tdsfrA= myw.MywFrame(self.clfr)
    self.tdslines=[]
    self.tdsfr= myw.MywFrame(self.tdsfrA,side=RIGHT)
    self.tdsheadfr= myw.MywFrame(self.tdsfrA,side=LEFT)
    outfr= myw.MywFrame(self.clfr)
    #tdsl= myw.MywLabel(tdsfr,label=" TDS:", side=LEFT)
    #self.tdshead=myw.MywBits(self.tdsfr,label="TDS", side=LEFT,
    #  defval= self.dobits(TDLTUS.tds, self.tds), cmd=self.modtdsl,
    #  helptext=TDLTUS.tdsbuthelptext,
    #  bits= TDLTUS.validtds)
    # let's assume, 1 TDS can be present only once in 1 cluster
    self.doTDSheadLists()
    self.doTDShead()
    self.doTDSbuttons()
    self.outl=myw.MywBits(outfr,label="Cluster of",
      defval= self.dobits(TDLTUS.ltus, self.outdets), cmd=self.modoutl,
      helptext="""Click to add/remove LTU. 

Shown LTUs (Output Detectors) are triggered when at least 
1 class driving this cluster becomes active.

See VALID.LTUS file for available LTUs.""",
      bits= TDLTUS.getltunames())
  def modcls(self, cls):
    """ Activated when class-button in TDS: list is pressed.
    cls: pointer to class, to be modified
    """
    #print "TrgCluster.modcls:",cls.trde.name
    cls.showClass()
    self.setActiveCluster()
  #def delcls(self, cls):
  #  cls.prtClass()
  def modtdsl(self, mwl, modix):
    #print "modtdsl:",modix,self.maxTD #, mwl.items
    if mwl.getEntry(modix)==1:   # adding TDS 
      if modix>self.maxTD:
        IntErr("modtdsl error:%s,%s,%s"%(modix,self.maxTD, mwl.items))
      else:
        self.tdshead.setEntry(modix,0)   # reset check button
      totclasses= self.partition.getRR(None)[0]
      if totclasses>=50:
        PrintError("Number of classes in this partition reached maximum (50)")
        return
      newcls=TrgClass(mwl.items[modix], self, None)
      #here we should check l0fun (if >2 are used, do not allow new class)
      self.cls.append(newcls)
      self.doCLSbutton(len(self.cls)-1)
      self.tdsonoffs.append(1)
      #self.tdsnames.append(newcls.trde.name)  
      #use className (not tdsName)
      self.tdsnames.append(newcls.getclsname())
      self.doTDShead()
    else:                        # removing TDS from this cluster
      found=None
      ind=modix   # to be removed from tdshead
      #indclsbut= len(self.clsbuts)-(modix-self.maxTD)   #from clsbuts
      indclsbut= modix-self.maxTD-1   #from clsbuts
      #print "modtdsl3.ind:",ind,"indclsbut:",indclsbut
      #if self.cls[indclsbut].trde.name!= mwl.items[ind]:
      if self.cls[indclsbut].getclsname()!= mwl.items[ind]:
        raise NameError, "in TrgCluster.modtdsl:"+ \
          self.cls[indclsbut].getclsname() + '!=' + mwl.items[ind]
      #todo: remove from self.tdslines[lineix][frameix]
      self.cls[indclsbut].clsbut.destroy()
      del self.cls[indclsbut].clsbut
      self.cls[indclsbut].clsbut= None
      #
      if self.cls[indclsbut].tlw:
         self.cls[indclsbut].tlw.destroy()
         self.cls[indclsbut].tlw= None
      del self.cls[indclsbut]
      del self.tdsonoffs[ind]
      del self.tdsnames[ind]
      self.doTDShead()
      self.setActiveCluster()
  def modoutl(self):
    tdbits= self.outl.getEntry()
    self.outdets=[]
    for i in range(32):
      bit= 1<<i
      if tdbits & bit:
        self.outdets.append(TDLTUS.ltus[i])
    #print "modoutl:", self.outl.getEntry(), self.outdets
  def delete(self, event=None):
    if self.partition:
      self.partition.rmCluster(self)
    self.tds= []
    self.outdets= []
    #self.clusterLabel.destroy()
    if self.clfr: 
      self.clfr.destroy()
      self.clfr=None
  def save(self, outfile):
    tds= self.getCLS()
    if not tds:
      PrintError("Cluster "+self.name+" without classes, not written to .partition")
      return
    ltunames= self.getltunames()
    if tds != '' or ltunames != '' :   #non empty cluster
      outfile.write(self.name+'\n')
      outfile.write(tds+'\n')
      outfile.write(ltunames+'\n')
  def getltunames(self):
    ltunames=" "
    for outdet in self.outdets:
      #print "TrgCluster:save:",outdet.name
      ltunames= ltunames+outdet.name+' '
    ltunames= string.strip(ltunames)
    return ltunames
  def getinpdets(self):
    """rc: list of triggering detectors in this cluster"""
    detnames= []
    for td in self.cls:
      detnames= detnames + td.trde.getInpDets()
    #print "dbggid:",detnames
    return detnames
  def dobits(self, bitlist, ltl):
    bits=0
    #for i in range(32):   -unfortunatelly, only 32 max. (to be improved!)
    for i in range(len(bitlist)):
      bit= 1<<i
      for ltuORtd in ltl:
        name= ltuORtd.name
        #print "dobits2:", name, bitlist[i].name
        if bitlist[i].name == name:
          if bits & bit:
            PrintError(name+" occurs more than once")
          bits= bits | bit
    #print "dobits:", bits
    return bits
  def setActiveCluster(self, event=None):
    self.partition.setActiveCluster(self)
  def activeCluster(self,event=None):
    #print 'activeCluster:', self.name
    self.tdshead.setColor(COLOR_ACTIVE)
    self.clfr.configure(bg=COLOR_ACTIVE)
  def deactiveCluster(self,event=None):
    #print 'deactiveCluster:', self.name
    self.tdshead.resetColor()
    self.clfr.configure(bg=COLOR_CLUSTER)
class TrgPartition:
  clustnames=["one","two","three","four","five","six"]
  def __init__(self, relpname, strict=None):
    """
    relpname: relpath/name
    Create partition object from file 'name.partition'
    if relname=='empty_partition', file is not read, but empty part. object
    is created.
    strict: strict: luminosity DIM service (see preproc.py) has to be available
    """
    self.strict= strict
    if strict=="strict":
      preproc.getlumi()
    print "initPartition:", strict, preproc.lumi_source
    self.version='0'
    self.loaderrors=''   # ok if ''
    self.loadwarnings=''   # ok if ''
    self.relpath= os.path.dirname(relpname)
    self.name= os.path.basename(relpname)
    # Important! the use of clusters=[] as default parameter
    # (if clusters is passed as parameter) is incorrect (reference) 
    # -use None instead
    self.clusters= []
    self.activeCluster= None
    # shared resources by this partition. The following should be
    # checked always when new resource added:
    # - number of l0funs <=2   todo: 4 if we allow transition to l0f34
    self.l0funs=[None, None]    # used l0funs (16 bits stored if used)
    self.l0funs34= [None, None] # complex l0f. pointer to l0fxxx TrgInput with
    # defined l0fAB pointing to 2 l0[AB]xxx TrgInput objects
    # list of pointers to P/F objects utilised by this partition (max. 4)
    self.activepfs=[]   
    #
    self.pclfr=None    # partition 'not shown'
    self.shmaster=None # shared rsrcs not shown
    self.activeclasses= {}  # filled in self.savercfg (for DAQ sqldb)
    self.sdgs= syncdg.TrgSDG()
    fname=self.name+".partition" ; fname=os.path.join(CFGDIR,self.relpath,fname)
    #if self.name=="empty_partition" or (not os.path.exists(fname)):
    if self.name=="empty_partition":
      self.clusters.append(TrgCluster(None, partition=self))
    else:
     print "NAME:%s:"%fname
     try:
      infile= open(fname, "r")
      if infile: 
        self.loadfile(infile)
        infile.close()
     except IOError:
       PrintError('I/O Error during loading partition:'+fname, self)
       print sys.exc_info()[0]
    #print "TrgPartition:",fname, "created. clusters:", self.clusters
  def donewtd(self):
    print "MywMenuList -using VALID.CTPINPUTS file"
  def show(self,master,name=None):
    #print "show2:",self.partfr
    if name:
      self.pclfrlab= myw.MywFrame(master)
      self.partnamelab=myw.MywLabel(self.pclfrlab, self.name, relief=FLAT, bg=COLOR_PART)
    else:
      self.pclfrlab=None
    self.pclfr= myw.MywFrame(master)
    #print "nclusts:", len(self.clusters)
    for cl in self.clusters:
      #print "nclusttds length:", len(cl.tds)
      cl.show(self.pclfr)
  def hide(self):
    # destroy windows if shown, including Class Toplevel windows
    #print "TrgPartition.hide"
    for i in range(len(self.clusters)):
      #print "TrgPartition.hide:",i
      self.clusters[i].hide()
    if self.pclfrlab:
      self.pclfrlab.destroy(); self.pclfrlab= None;
    self.pclfr.destroy(); self.pcflfr= None;
    #zle del(self.partfr)
  def destroy(self):
    print "destroying:", len(self.clusters)
    if self.pclfr:   # destroy windows if shown:
      self.hide()
    if self.shmaster:
      self.hideShared()
    for i in range(len(self.clusters)):
      del self.clusters[0]
  def prt(self):
    print "------- ", self.name, "loaderrors:"
    print self.loaderrors
    print " see also /tmp/parted.log"
    print "----------------------------- Partition",self.name+"'s clusters:"
    for cl in self.clusters:
      #print "nclusttds length:", len(cl.tds)
      cl.prt()
  def prtInputDetectors(self):
    allindets={} ; detsline=""   #alloutdets={}
    for cluster in self.clusters:
      if cluster==None:
        PrintError("cluster None")
        break
      cids= cluster.getinpdets()
      #print "cluster:",cluster.name, cids
      for id in cids:
        if id=="L0FUNS": continue
        if id=="": continue   # no input (e.g.:ctp generator)
        if not allindets.has_key(id):
          allindets[id]= id
          detsline= id + " " + detsline
    print detsline
  def getRR(self, minst, ix=0):   # get Required Resources
    """ 
    rc: (classes,pfs,bcms,clusters,outdets, indets)
    Total numbers (i.e. number of uses)
    minst: !=None  -print to stdout in addition 
    minst: None -do not print text line to stdout
    """
    allcls=[]    #pointers to all the classes used in this partition
    negcls=0     #number of negating classes
    alloutdets={} ;allindets={}
    allpfs=[]    # list of used resources. values: 0..3
    allbcms=[]   # 0..3/11
    for cluster in self.clusters:
      #print "getRR cluster:",cluster.prt()
      if cluster==None:
        PrintError("cluster None")
        break
      cids= cluster.getinpdets()
      #print "cluster:",cluster.name, cids
      for id in cids:
        if not allindets.has_key(id):
          allindets[id]= id
      allcls=allcls+cluster.cls
      for clas in cluster.cls: 
        if clas.trde.class4550flag==1: negcls= negcls+1
        # check PF:
        for ix in range(len(clas.pfs)):
         if clas.pfs[ix]==1:
           if findRR(allpfs, ix)!=None: continue
           allpfs.append(ix)
        # check BCM:
        for ix in range(len(clas.bcms)):
         if clas.bcms[ix]==1:
           if findRR(allbcms, ix)!=None: continue
           allbcms.append(ix)
      for det in cluster.outdets:
        if not alloutdets.has_key(det):
          alloutdets[det]= 'yes'
    if minst!=None:
      print self.name, "total resources: Classes:",len(allcls),\
        "Negating classes:", negcls,"BCMs:", len(allbcms),\
        "PFs:",len(allpfs), "Clusters:", len(self.clusters),\
        "Output dets:",len(alloutdets), "Input dets:",len(allindets) 
      #print "Input dets:", allindets.keys()
    return (len(allcls),negcls,len(allpfs),len(allbcms),len(self.clusters),len(alloutdets), len(allindets))
    #return (allcls,allpfs,self.clusters,alloutdets,allindets)
  def save(self, partname):
    """ savepcfg() has to be called before save()
    """
    rc=0
    fnw=os.path.join(self.relpath, partname)
    if os.access(CFGDIR, os.W_OK)==0:
      print "Cannot write into "+CFGDIR
      return 4
    else:
      fnw=os.path.join(CFGDIR, fnw)
    self.savepcfg(name=partname) # has to be called prior .partition creation!
    outfile= open(os.path.join(fnw+".partition"),"w") # (not used resources)
    outfile.write("Version: %s\n"%(VERSION))
    for shrr in SHRRSRCS:
      shrr.save(outfile)
    self.sdgs.save(outfile)
    outfile.write("Clusters:\n")
    for cluster in self.clusters:
      cluster.save(outfile)
    outfile.close()
    print fnw+".partition written"
    return rc
  def checkPartition(self):
    """rc: None if file saved
    """
  def l0fUpdate(self, tl0funs, l0inps):
    for ix in (0,1):
      #print  cls.trde.l0funs[ix]
      lf= tl0funs[ix]
      if lf:
        lutv= tl0funs[ix][0]    # >=AC
        if type(lutv) == types.ListType:
          lutv=list2str(lutv)
          lfpos= 26
          ixall= self.l0funs34[lutv][1]
        else:                   # <=AB
          lfpos= 24
          ixall= self.l0funs[lutv][1]
        #print "LUTixall:lutv:",lutv,ixall
        #cls.prtClass()
        l0inps= l0inps & ~(1<<(lfpos+ixall))   # set l0funs
    return l0inps
  def findfirst(self, phclasses=None):
    """ Find first class in SDG group. All the classes in SDG group will
    point to first class.
    Invoked from:
    pcfg (phclasses is None), instead of hw-allocated classes, clanumlog
         is used. In this case, the result is used for the check if
         given SDG group was used.
    rcfg phclasses are known and used. In this case, the result is used
         for .rcfg info -SDG column in class lines
    """
    for sdgn in self.sdgs.sdgs.keys():   # all SDG groups
      firstc=51
      for ixclu in range(len(self.clusters)):
        cluster= self.clusters[ixclu]
        for cla in cluster.cls:
          if cla.L0pr==sdgn:
            if phclasses==None:
              phcla= cla.clanumlog
            else:
              ixclasses= cla.clanumlog-1
              phcla= phclasses[ixclasses]
            if int(phcla)<firstc:
              firstc= int(phcla)
      if firstc<51:
        self.sdgs.setl0prsdg(sdgn, firstc)
  def savepcfg(self,wdir=CFGDIR,name=None):
    """wdir:
    is CFGDIR for interactive use (.pcfg just save, NOT USED LATER!)
    is WORKDIR for use from pydimserver -i.e. when saved+copied to ctpproxy-CPU
       at the time of LOAD partition
    return: "": ok, .pcfg saved
            errormessage: the same error message written in .pcfg file
                          preceded with line "Errors:"
    Notes: savepcfg() has to be called prior save() !
    """
    if name==None: name=self.name
    #print "\n---------------- savepcfg: ",name
    errormsg='Errors:\n'   # has to be (checked later and in ctp-proxy!)
    outfilename= os.path.join(wdir, name)+".pcfg"
    clanum=clunum=1
    alldets=[]   # preparation for FO. lines:
    for ix in range(24):
      alldets.append(0)
    for sr in SHRRSRCS:   # preparation for 'check if rsrcs is used'
      sr.used=0
    CLAlines=[]
    # allocate l0f* usage in all classes (check if <=2 used was done already)
    lfs,error=self.allocShared([None,None], lf34=1)   #just update self.l0funs
    if error: errormsg= errormsg+error+'\n'
    lfs,error=self.allocShared([None,None])   #just update self.l0funs
    if error: errormsg= errormsg+error+'\n'
      #PrintError(error)
    # calculate lines for all classes in all clusters:
    pf_comdef=None
    for cluster in self.clusters:
      if len(cluster.cls)==0:
        errormsg=errormsg+"Cluster "+cluster.name+" without classes, not written to .pcfg\n"
        continue
      #print "savepcfg:%s"%(cluster.name)," :",cluster.getltunames()
      if string.find(cluster.getltunames(), "TRD")== -1:
        check0HWU= False
      else:
        check0HWU= True
      for cls in cluster.cls:
        if cls.trde.connected[1]>0:   # at least 1 unconnected input
          unconnectedNames= cls.trde.unconnectedNames
          errormsg=errormsg+"Number of not connected inputs:%d (%s) for %s in cluster %s\n"%\
            (cls.trde.connected[1], unconnectedNames, cls.trde.name, cluster.name)
          continue
        if cls.trde.allinputsvalid==False:   # at least 1 input not valid
          errormsg=errormsg+"Some of %s inputs are not valid in cluster %s\n"%\
            (cls.trde.name, cluster.name)
          continue
        if check0HWU:
          # check if 0HWU in all classes
          #print "savepcfg:   ", cls.trde.getInputs()
          if string.find(cls.trde.getInputs(), "0HWU")== -1:
            errormsg= errormsg+"Cluster: %s triggered by non-0HWU class %s\n"%\
              (cluster.name, cls.trde.name)
            continue
        l0inv=0
        if BCM_NUMBER==4:
          l0vetos=0xfff0 | clunum # 0x10000 has to be 0 (active class)
        else:
          l0vetos=0x7ffffff0 | clunum
        # see ctp_proxy/readme
        if cls.allrare==0: 
          if BCM_NUMBER==4:
            l0vetos= l0vetos & 0xffffefff
          else:
            l0vetos= l0vetos & 0xffefffff
        l0scaler= cls.L0pr
        l0pr= self.sdgs.find(l0scaler)
        if l0pr==None:
          #print "l0scaler:", l0scaler,":"
          rcnone= myw.frommsL0pr('', l0scaler)
          if rcnone:
            l0scaler= "0x%x"%(int(rcnone))
          else:
            errormsg= errormsg+"Bad L0pr:%s\n"%l0scaler
        l1def= 0x0fffffff | (clunum<<28)
        l1inv= 0
        l2def= 0x0fffffff | (clunum<<28)
        if BCM_NUMBER==4:
          l0inps=0x3fffffff   # bc2 bc1 rnd2 rnd1 l0f2 l0f1 i24..i1
          r12b12=1<<26
        else:
          l0inps=0xffffffff   # bc2 bc1 rnd2 rnd1 l0f4..l0f1 i24..i1
          r12b12=1<<28
        #print "savepcfg1:",cls.optinps
        for ix4 in range(4):        # rnd1/2 bc1/2
          if cls.optinps[ix4]==1:   # cls uses ix4 resource
            l0inps= l0inps & ~r12b12
            SHRRSRCS[ix4].used= SHRRSRCS[ix4].used+1
          r12b12= r12b12<<1
        r12b12=1<<8
        for ix4 in range(BCM_NUMBER):
          if cls.bcms[ix4]==1:   # cls uses bcm[ix4+1] resource
            # check if corresponding BCM is defined:
            #print "bcm%ddef:"%ix4,SHRRSRCS[ix4+BCMASKS_START].getDefinition()
            if SHRRSRCS[ix4+BCMASKS_START].getDefinition()==None:
              errormsg= errormsg+\
                "Undefined %s referenced by class %s, reference discarded in .pcfg file\n"%\
                (SHRRSRCS[ix4+BCMASKS_START].name, cls.getclsname())
              continue   
            l0vetos= l0vetos & ~r12b12
            SHRRSRCS[ix4+BCMASKS_START].used= SHRRSRCS[ix4+BCMASKS_START].used+1
          r12b12= r12b12<<1
        l1def= l1def & cls.trde.l1def
        l2def= l2def & cls.trde.l2def
        r12b12=1<<4 ; pfl1def= pfl2def = 1<<24
        for ix4 in range(PF_NUMBER):
          if cls.pfs[ix4]==1:   # cls uses pfs[ix4+1] resource
            #print "pf%ddef:"%ix4,SHRRSRCS[ix4+PFS_START].getDefinition()
            if SHRRSRCS[ix4+PFS_START].getDefinition()==None:
              errormsg= errormsg+\
                "Undefined %s referenced by class %s, reference discarded in .pcfg file\n"%\
                (SHRRSRCS[ix4+PFS_START].name, cls.getclsname())
              continue   
            #print "PFdef:",SHRRSRCS[ix4+PFS_START].getDefinition()
            # check if common definition agrees (should be improved, can happen not all PFs used):
            comdef= map(eval,string.split(SHRRSRCS[ix4+PFS_START].getDefinition())[PF_COMDEFSIX:])
            if pf_comdef==None:
              pf_comdef= comdef
            else:
              err=False
              for i in range(3):
                if comdef[i]!= pf_comdef[i]:
                  errormsg= errormsg+\
                    "PF%d common definition does not agree,not used, class:%d\n"%((ix4+1),clanum)
                  err=True ; break
              if err:
                continue
            if SHRRSRCS[ix4+PFS_START].isPFDefined(0): 
              l0vetos=l0vetos & ~r12b12
            if SHRRSRCS[ix4+PFS_START].isPFDefined(1): 
              l1def= l1def & ~pfl1def
            if SHRRSRCS[ix4+PFS_START].isPFDefined(2): 
              l2def= l2def & ~pfl2def
            SHRRSRCS[ix4+PFS_START].used= SHRRSRCS[ix4+PFS_START].used+1
          r12b12= r12b12<<1
          pfl1def= pfl1def<<1
          pfl2def= pfl2def<<1
        #print "CLA:%d"%clanum ; cls.trde.prt()
        l0inps= l0inps & cls.trde.l0inps    # take l0 inputs
        l0inv= cls.trde.l0inv
        l1inv= cls.trde.l1inv
        #print "savepcfg:",cls.trde.l0funs
        l0inps= self.l0fUpdate(cls.trde.l0funs, l0inps)
        l0inps= self.l0fUpdate(cls.trde.l0funs34, l0inps)
        clgrouptx=''
        if cls.classgroup>0: clgrouptx=" %d"%cls.classgroup
        if cls.clanumlog != clanum:
          # see comment in self.loadfile.
          PrintWarning("Class %s CLA%2.2d != %d (clanumlog)"%\
           (cls.getclsname(), clanum, cls.clanumlog))
        line="CLA.%2.2d 0x%x 0x%x 0x%x %s 0x%x 0x%x 0x%x%s\n"%\
          (clanum, l0inps, l0inv, l0vetos, l0scaler, l1def, l1inv, 
          l2def, clgrouptx)
        CLAlines.append(line)
        if l0inps==0x3fffffff:
          PrintWarning("""No input choosen(class will be triggered all the time). 
Logical class """+str(clanum)+", cluster:"+cluster.name+", class name:"+ cls.getclsname())
        clanum= clanum+1
      # preparation for FO. lines:
      for outd in cluster.outdets:
        #print "Cluster(1-6):",clunum,outd.name, outd.detnum
        alldets[outd.detnum]= alldets[outd.detnum] | (1<<(clunum-1))
      #print "alldets:",alldets
      clunum= clunum+1
    #
    self.findfirst()
    # check if all used SHRRSRCS are defined and vice versa:
    for sr in SHRRSRCS:
      if sr.used>0:
        if sr.getValue()=="None":
          errormsg=errormsg+sr.name+" used, but not defined\n"
      else:
        srval= sr.getValue()
        if srval!="None":
          PrintWarning("Value %s of %s ignored (not referenced)"%\
            (srval, sr.name))
          sr.setValue("None")
    #
    # write shared now (after their usage was checked):
    outfile= open(outfilename,"w")
    if errormsg!='Errors:\n':
      outfile.write(errormsg)
      outfile.close()
      #print outfilename," written"
      print errormsg
      return errormsg
    line='RBIF '
    for ix in range(4):    # order: RND1,2 BC1,2 L0f1,2,T
      line=line+str(SHRRSRCS[ix].getValueHexa())+':'
    for ix in (0,1):
      for shrlut in self.l0funs.keys():
        if self.l0funs[shrlut][1]==ix:
          #line=line + "0x%x"%shrlut + " " + str(self.l0funs[shrlut][0])
          line=line + "0x%x"%shrlut + " "+self.l0funsdefs[shrlut]
      line=line+':'
    outfile.write(line+"\n")
    if BCM_NUMBER!=4:
      line='L0F34 '
      for ix in (0,1):
        for shrlut in self.l0funs34.keys():
          #print "shrlut:%d%d:"%(ix,self.l0funs34[shrlut][1]), "shrlut:", shrlut
          if self.l0funs34[shrlut][1]==ix:
            #line=line + "0x%x"%shrlut + " " + str(self.l0funs[shrlut][0])
            line=line + str(shrlut) + " "+self.l0funsdefs34[shrlut]
        line=line+':'
      outfile.write(line+"\n")
    line='BCMASKS '
    allmasks=[]; 
    for i in range(txtproc.ORBITLENGTH):
      allmasks.append(0)   # default is: 0 = L = don't care
    atleast1bcm= 0
    for i in range(BCM_NUMBER):
      #print "bcm%d"%i,SHRRSRCS[BCMASKS_START+i].getDefinition()
      bcmdef= SHRRSRCS[BCMASKS_START+i].getDefinition()
      if bcmdef==None: continue
      #SHRRSRCS[BCMASKS_START+i].prtBits()
      atleast1bcm= atleast1bcm+1
      b= SHRRSRCS[BCMASKS_START+i].getBits()
      for ibc in range(txtproc.ORBITLENGTH):
        if b[ibc]==1:
          allmasks[ibc]= allmasks[ibc] | (1<<i)
    if atleast1bcm>0:
      if BCM_NUMBER==4:
        bcm_format="%1.1x"
      else:
        bcm_format="%3.3x"
      for ibc in range(txtproc.ORBITLENGTH):
        line=line + bcm_format%allmasks[ibc]
      outfile.write(line+"\n")
    #print line+"\n"
    #bm=txtproc.BCmask(newvaluetxt) ; errmsg= bm.checkSyntax()
    #
    # PF:
    for i in range(PF_NUMBER):
      #print "bcm%d"%i,SHRRSRCS[BCMASKS_START+i].getDefinition()
      pfdef= SHRRSRCS[PFS_START+i].getDefinition()
      if pfdef==None: continue
      line='PF.%d %s'% (i+1, pfdef)
      outfile.write(line+"\n")
    #
    self.sdgs.save(outfile, "SDG ")
    # now we can write 'CLA' lines:
    for ix in range(len(CLAlines)): 
      outfile.write(CLAlines[ix])   
    for ix in range(6):   # do FO.1 FO.2 ... FO.6 lines
      flag=0; clusters=0; ix1=0
      for ixx in range(ix*4,ix*4+4):   # 0,1,2,3 or 4,5,6,7 or 8,9,10,11 ...
        clusters1= alldets[ixx]
        clusters= clusters | (clusters1<<(8*ix1))
        #print "clusters1:", ixx, hex(clusters1), hex(clusters)
        ix1=ix1+1
        if clusters1!=0: flag=1
      if flag==1:
        line= "FO.%1d 0x%x\n"%(ix+1, clusters)
        #print "FO.%1d 0x%x"%(ix+1, clusters)
        outfile.write(line)   
    outfile.close()
    print outfilename," written"
    return ""
  def savercfg(self, line=""):
    """line: info from ctp_proxy. Format:
    partitionName runNumber detectorMask phys_clusters phys_classes
      INT1lookup INT1def INT2lookup INT2def
    if line=='': just testing  (i.e.: parted PHYSICS_1 r)
    detectorMask: 0x820  -allow just 2 detecors (0x800 + 0x20)
    phys_clusters: 1 2 3 4 5 6
    phys_classes:  1 2 ... 49 50
    INT*lookup: 0x...  -4 hexadigits
    INT*def:    0x...  -5 bits (0x1f= LUT |BC1|BC2|RND1|RND2)
                                 bit:  0    1   2   3    4
    now the sequence of calls is: 
      1. ctp_proxy sends line to vme/pydim/dimserver.py(service CTPRCFG/RCFG)
      2. 'parted.py.savercfg for partName' -> WORK/RCFG/.rcfg on server
         'full masking' of detectors is implemented (like in ctp_proxy)
    TODO:
    """
    if self.name[:5]!="DBGDB":
      rcfg= 'rcfg'   # was debug
    else:
      rcfg= 'rcfg'   # real partitions start with ALICE
    lixorig= string.split(line)
    if len(lixorig)<4:
      rcfg= 'debug'
      line="%s 1234 0xffffff 6 5 4 3 2 1"%(self.name)
      #line="%s 1234 0x2008 1 2 3 4 5 6"%self.name
      #line="%s 1234 0xffffff 0 0 1 2 0 0"%self.name
      for i in range(1,51,1):
        line= line+" "+str(i)
      # INT1: 0x1234 LUT|BC1    INT2: 0x1234 LUT|BC2
      line= line+" 0x1234 0x3 0x1234 0x5"
    runnumber=1234 ; 
    detmask=0xffffff   # all dets by default
    #detmask= 0x5       # only SSD+SPD
    #detmask= 0x10010   # only ACORDE TRD
    #detmask= 0x4       # SSD only
    #detmask= 0x80000   # DAQ only
    if len(lixorig)>=2: runnumber= lixorig[1]
    if len(lixorig)>=3: detmask= eval(lixorig[2])
    lix= string.split(line)
    phclusters=[]
    cc650=len(lix)   # should be 3+6+50=59
    for ix in range(6):
      if (ix+3)>= cc650: phclu='0'   #not given = not assigned
      else: phclu=lix[ix+3]
      phclusters.append(phclu)
    phclasses=[]   #phclasses[logN]= physN, logN:0..49 physN:1:50
    for ix in range(50):
      if (ix+9)>= cc650: phcla='0'   #not given = not assigned
      else: phcla=lix[ix+9]
      phclasses.append(phcla)
    interactions=[(0,0), (0,0)]
    for ixint in [0,1]:
      if (ixint*2+59)>=cc650: lookupt= 0
      else: lookupt= eval(lix[ixint*2+59])
      if (ixint*2+60)>=cc650: intdef= 0
      else: intdef= eval(lix[ixint*2+60])
      interactions[ixint]= (lookupt, intdef)
    #print "line:",line
    #print "phclusters:",phclusters, "int1:", interactions[0], "int2:", interactions[1]
    #print "phclasses:",phclasses
    print "Applying mask:", hex(detmask), 'phclusters:',phclusters
    # we have to descend with indexes because of deleting
    for ixclu in range(len(self.clusters)-1,-1,-1):
      print "-----------Cluster:"+self.clusters[ixclu].name
      self.clusters[ixclu].prt()
      for ixdet in range(len(self.clusters[ixclu].outdets)-1,-1,-1):
        det= self.clusters[ixclu].outdets[ixdet]
        ltu= TDLTUS.findLTU(det.name)
        #print "savercfg2:",det.detnum
        if ((1<<det.detnum)&detmask)==0:
          print "Cluster:"+self.clusters[ixclu].name+ " detector:"+det.name + " masked out"
          del self.clusters[ixclu].outdets[ixdet]
          #print "reamining dets:",self.clusters[ixclu].outdets
          print "Cluster:"+self.clusters[ixclu].name+ " # ofLTUS:",len(self.clusters[ixclu].outdets)
          if len(self.clusters[ixclu].outdets)==0:
            # empty cluster. Do not remove it. Corresponding
            # phclusters[] should be 0 when invoked from ctp_proxy -is happens
            # after classes/clusters were allocated. 
            # Invocation 'parted.py partname r': Let's put 0 with warning:
            #self.clusters[ixclu].delete()
            print "Empty clust [0-5]:%d phclust[0-6]:%s"%(ixclu, phclusters[ixclu])
            if phclusters[ixclu] != '0':
              PrintWarning("phclusters[%d] set to '0'"%(ixclu))
              phclusters[ixclu]= '0'
    outfilename= dorcfgname(runnumber,rcfg)
    of= open(outfilename,"w")
    if self.downscaling!=None:
      #line='VERSION: 4 %s\n'%self.downscaling.v0and
      line='VERSION: %s %s\n'%(VERSION,self.downscaling.v0and)
    else:
      #line='VERSION: 4\n'
      line='VERSION: %s\n'%VERSION
    of.write(line)
    line='PARTITION: %s\n'%(self.name); of.write(line)
    line='INPUTS:\n' ; of.write(line)
    usedinputs={} ; optinputs={}
    useddescriptors={}
    usedbcmasks={}
    usedclasses=[] ; atleast1classNONEbcm=0   # ixclasses=0
    atleast1pfnone=0
    l0defs=[]
    trgctpcfg= trigdb.Trgctpcfg(); clgtimes= trgctpcfg.getTIMESHARING()
    # find first sync downscaling class
    self.findfirst(phclasses)
    #prepare 4 sections: CLASSES DESCRIPTORS BCMASKS INPUTS(directly written)
    for ixclu in range(len(self.clusters)):
      cluster= self.clusters[ixclu]
      #if phclusters[ixclu] =='0': continue   # masked out
      if len(cluster.outdets)==0:continue #empty cluster,do not take its classes
      #print "Classes preparation:" ; cluster.prt()
      for cla in cluster.cls:
        #---------------- prepare optional inputs (for INPUTS: section):
        bcrnd= cla.getTXTbcrnd()
        #print "savercfg:", bcrnd
        for key in bcrnd.keys():
          if not optinputs.has_key(key):
            optinputs[key]= bcrnd[key]
        #
        #---------------------------------- prepare classes section: 
        ixclasses= cla.clanumlog-1
        phcla= phclasses[ixclasses]
        #print "cla.trde.name:",cla.trde.name,phcla ; cla.trde.trdprt()
        desname,oins=cla.getrcfgdesname()
        #fix for offline (they can cope with EMPTY, DEMPTY is not valid)
        if desname=='DEMPTY': desname= 'EMPTY'
        msk= cla.getPFs()
        if string.find(msk,"NONE") >=0:
          atleast1pfnone= atleast1pfnone+1
        msk= cla.getBCMASKs()
        if string.find(msk,"NONE") >=0:
          atleast1classNONEbcm= atleast1classNONEbcm+1
        clasname= cla.getclsname()
        if string.find(clasname,'-') <0:
          # DTRUE(cn=C0TVX) -convenient for techn. runs
          clasname= cla.buildname(clasname)
        #line="#%s %s %s %s %s %s %s %d\n"%(clasname, phcla, desname, \
        #  cluster.name, cla.getPFs(), cla.getBCMASKs(), \
        #  myw.frommsL0pr('',cla.L0pr), \
        #  cla.allrare)
        #print "rcfgline:",line
        if cla.classgroup==0:
          clgtime='0'
        else:
          clgtime= clgtimes[int(cla.classgroup)]  # was -1
        l0scaler= self.sdgs.find(cla.L0pr)
        if l0scaler!=None:   #sync downscaling
          sdg= self.sdgs.getl0prsdg(cla.L0pr)
          l0pr= myw.frommsL0pr('',l0scaler)
        else:
          l0pr= myw.frommsL0pr('',cla.L0pr)
          if l0pr==None:
            print "ERROR:savercfg: claL0pr:", cla.L0pr, clasname, l0pr, type(l0pr)
          if int(l0pr) & 0x80000000:
            sdg= 0   # not sync, busy way
          else:
            sdg= int(phcla)   # not syncdownscaling, rnd. way
        #if len(self.sdgs.sdgs.keys())>0:
        lineclg="%s %s %s %s %s %s %s %d %d %s %d\n"%(
          clasname, phcla, desname, cluster.name, 
          cla.getPFs(), cla.getBCMASKs(), l0pr, 
          cla.allrare, cla.classgroup, clgtime, sdg)
        #else:
        #  lineclg="%s %s %s %s %s %s %s %d %d %s\n"%(
        #    clasname, phcla, desname, cluster.name, 
        #    cla.getPFs(), cla.getBCMASKs(), l0pr, 
        #    cla.allrare, cla.classgroup, clgtime)
        if phcla!='0': 
          #usedclasses.append(line); 
          usedclasses.append(lineclg)
          self.activeclasses[phcla]= (clasname, cla.classgroup, clgtime, l0pr)   # used in pydim
          #print "activeclasses:", str(self.activeclasses)
        #--------------------------------- prepare descriptors section:
        if not useddescriptors.has_key(desname):
          #print "cla.trde:"
          #cla.trde.trdprt()
          line=''
          for inp in cla.trde.inputs:
            line= line + ' ' + inp
          line= line + oins
          # add optional inputs (bad idea to do it here):
          #line= line + ' ' + cla.getTXTbcrnd()
          line= line+'\n'
          useddescriptors[desname]= desname+' '+line
        #
        #--------------------------------- create usedbcmasks for BCMASKS: section
        # todo: if cla.bcms not in usedbcmasks: add it to usedbcmasks
        #print "clabcms:",clasname,cla.bcms
        for ixm in range(BCM_NUMBER):
          if cla.bcms[ixm]==0: continue
          name= SHRRSRCS[BCMASKS_START+ixm].name
          if usedbcmasks.has_key(name): continue
          usedbcmasks[name]= 1
          #print "usedbcmasks:", name
        #
        #--------------------------------- create INPUTS: section
        takewholevalidctpinputs="""
        for inpname in cla.trde.inputs:
          inp= TDLTUS.findInput(inpname)
          #note: inpname is [*]name, inp.name is: name
          if usedinputs.has_key(inp.name): continue
          usedinputs[inp.name]= 'ok'
          if inp.ctpinp==None:            # l0f definition
            if inp.l0fdefinition:
              l0defs.append("%s %s\n"%(inpname, inp.l0fdefinition))
              ok,l0funvars= txtproc.varsInExpr(inp.l0fdefinition)
              for inpname2 in l0funvars:
                inp2= TDLTUS.findInput(inpname2)
                if usedinputs.has_key(inpname2): continue
                usedinputs[inpname2]= 'ok'
                if inp2==None:
                  IntErr("savercfg: %s input used in %s not found"%(inp2,inpname))
                  continue
                self.prtinput1(inp2, of)
            else:
              PrintError("%s is not CTPinput neither L0 definition:"%inpname)
            continue
          self.prtinput1(inp, of)
"""
        # we need to prepare l0defs. Go through all inputs, and
        # create usedinputs, l0defs. Only l0defs will be used 
        # in DESCRIPTORS: section
        for inpname in cla.trde.inputs:
          inp= TDLTUS.findInput(inpname)
          #note: inpname is [*]name, inp.name is: name
          if usedinputs.has_key(inp.name): continue
          usedinputs[inp.name]= 'ok'
          if inp.ctpinp==None:     # l0f -simple or complex definition
            if inp.l0fdefinition:
              l0defs.append("%s %s\n"%(inpname, inp.l0fdefinition))
              ok,l0funvars= txtproc.varsInExpr(inp.l0fdefinition)
              for inpname2 in l0funvars:
                inp2= TDLTUS.findInput(inpname2)
                if usedinputs.has_key(inpname2): continue
                usedinputs[inpname2]= 'ok'
                if inp2==None:
                  IntErr("savercfg: %s input used in %s not found"%(inp2,inpname))
                  continue
                self.prtinput1(inp2, of)
            else:
              PrintError("%s is not CTPinput neither L0 definition:"%inpname)
            continue
          #self.prtinput1(inp, of)
    vi= trigdb.TrgVALIDINPUTS() ; vi.prtall(of) ; vi=None
    #------------------------------------- optional inputs:
    for key in optinputs.keys():
      line= "%s CTP 0 %s %s\n"%(string.upper(key), optinputs[key][0],
        optinputs[key][0])
        #optinputs[key][1])    #khz ms...
      of.write(line)
    #     
    # moved below
    #if len(l0defs)>0:    # no section at all if no l0f definitions
    #  line='L0FUNCTIONS:\n' ; of.write(line)
    #  for line in l0defs: of.write(line)
    #------------------------------------- interactions:
    line='INTERACTIONS:\n' ; of.write(line) ; noneline='NONE\n'
    #New way (22.1.2010): symb.lookupt | BC1/2 RND1/2 from ctp.cfg
    for ixint in [1,2]:
      int12line= TRGCTPCFG.getINT12(ixint)
      if int12line!=None:
        of.write("INT%d %s\n"%(ixint,int12line))
        noneline=None
    #Old way (lookupT + BC1/2 RND1/2 if defined):
    for ixint in [0,1]:
      iline=''
      intdef= interactions[ixint][1]
      if (intdef & 1)==1:     iline=' '+hex(interactions[ixint][0])
      if (intdef & 0x2)==0x2: iline= iline+" BC1"
      if (intdef & 0x4)==0x4: iline= iline+" BC2"
      if (intdef & 0x8)==0x8: iline= iline+" RND1"
      if (intdef & 0x10)==0x10: iline= iline+" RND2"
      if iline!='':
        of.write("# INT%d%s\n"%(ixint+1, iline))
        noneline=None
    #if noneline:
    #  of.write(noneline)
    #D= (lookupt, intdef)
    line='DESCRIPTORS:\n' ; of.write(line)
    if len(l0defs)>0:    # no section at all if no l0f definitions
      #line='L0FUNCTIONS:\n' ; of.write(line)
      for line in l0defs: of.write(line)
    for desname in useddescriptors.keys():
      of.write(useddescriptors[desname])
    line='CLUSTERS:\n' ; of.write(line)
    for ixclu in range(len(self.clusters)):
      cluster= self.clusters[ixclu]
      #phclu= str(ixclu+1)
      phclu= phclusters[ixclu]
      if phclu=='0': continue
      line="%s %s"%(cluster.name, phclu)
      for det in cluster.outdets:
        ltu= TDLTUS.findLTU(det.name)
        #print "savercfg2:",det.detnum
        #if (1<<det.detnum)&detmask:
        #fix
        if det.name=='DAQ':
          line= line+' '+'DAQ_TEST'
        elif det.name=='EMCAL':
          line= line+' '+'EMCal'
        else:
          line= line+' '+det.name
      line= line+'\n'
      of.write(line)
    line='PFS:\n' ; of.write(line)   # !Reminder: do the same as for BCMASKS (atleast1classNONEbcm)
    atleast1pf=0 
    for ixpfpc in range(PF_NUMBER):
      pfdef= SHRRSRCS[PFS_START+ixpfpc].getDefinition()
      if pfdef==None: 
        continue
      line="PF%d %s\n"%(ixpfpc+1, pfdef)
      of.write(line)
      atleast1pf= atleast1pf+1
    if atleast1pfnone>0:      #if atleast1pf==0:
      line='NONE\n' ; of.write(line)
    line='BCMASKS:\n' ; of.write(line)
    line='#cs %s\n'%(TDLTUS.csName) ; of.write(line)
    atleast1bcm= 0
    for ixm in range(BCM_NUMBER):
      #print "bcm%d"%ixm,SHRRSRCS[BCMASKS_START+ixm].getDefinition()
      name= SHRRSRCS[BCMASKS_START+ixm].name
      if not usedbcmasks.has_key(name): continue
      bcmdef= SHRRSRCS[BCMASKS_START+ixm].getDefinition()
      if bcmdef==None: continue
      atleast1bcm= atleast1bcm+1
      of.write("%s %s\n"%(name, bcmdef))
    if atleast1bcm==0 or (atleast1classNONEbcm>0): line='NONE\n' ; of.write(line)
    line='CLASSES:\n' ; of.write(line)
    for ix in range(len(usedclasses)):
      of.write(usedclasses[ix])
    of.close()
    if rcfg=='debug': return
    srcname=os.path.join(CFGDIR,self.relpath,self.name+".partition")
    desname= os.path.join(WORKDIR, "PCFG/r%s.%s"%(runnumber,'partition'))
    shutil.copyfile(srcname,desname)
    #take .pcfg cretaed in LOAD time (i.e. WORKDIR).
    srcname=os.path.join(WORKDIR,self.relpath,self.name+".pcfg")
    desname= os.path.join(WORKDIR, "PCFG/r%s.%s"%(runnumber,'pcfg'))
    shutil.copyfile(srcname,desname)
    print outfilename," written. .partition and .pcfg files copied to PCFG/"
  def prtinput1(self, inp, of):
    """not used, instead  trigdb.TrgVALIDINPUTS.prtall() is used
    if inp==None:
      IntErr("savercfg: %s input not found"%inpname)
      return
    #fix:
    if inp.detectorname=='DAQ':
      detn= 'DAQ_TEST'
    elif inp.detectorname=='EMCAL':
      detn= 'EMCal'
    else:
      detn= inp.detectorname
    line= "%s %s %d %s %s\n"%(inp.name, detn, \
      inp.ctpinp[0], inp.signature, inp.ctpinp[1])
    #  inp.ctpinp[0], inp.signature, inp.signature)
    of.write(line)
    """
  def allocShared(self, pl0funs, lf34=None):
    """
    pl0funs: list of l0funcs ([16bitsvalue,inp_ref] or 
      [string_2hexa, in_pref] in case of lf34!=None)
      to be used in addition to already used in self.l0funs[34]
    If l0funs not used: [None, None]
    1 l0fun used:   [(LUT,inp), None]
    2 l0funs used:   [(LUT1,inp), (LUT2,inp)]
    rc: 
    l0funsall, None -> ok, allocated. l0funsall is
      list of given LUTs (i.e. NOT all the LUTs allocated for
      partition) [lut1,lut2], as they are allocated
      for this partition, None if not used.
    Possible l0funsall:
    [LUT, None]
    [None, LUT]
    [LUT1, LUT2]
    [LUT2, LUT1]
    None, errmsg -> not allocated
    """
    #print "---- allocShared:l0funs:", pl0funs
    newlf= [None,None] ; errmsg=None
    #self.l0funs= {}  
    #self.l0funsdefs= {}  # corresponding l0 definitions
    plfs= {} ; plfsdefs= {}
    ixalloc=0
    for cluster in self.clusters:
      for cls in cluster.cls:
        for ix in (0,1):
          if lf34:
            trdel0funs= cls.trde.l0funs34
          else:
            trdel0funs= cls.trde.l0funs
          if trdel0funs[ix]:   # l0f[ix] used
            #von if lf34: lutval= list2str(trdel0funs[ix][0])
            #von else:    lutval=          trdel0funs[ix][0]
            lutval= list2str(trdel0funs[ix][0])
            l0def=  trdel0funs[ix][1].l0fdefinition
            #print "allocSharednew:",ix, lutval,'=',l0def
            #cls.prtClass()
            if plfs.has_key(lutval):
              # number of uses:
              #print 'lutval:',plfs[lutval]
              plfs[lutval][0]= plfs[lutval][0]+1
            else:
              plfs[lutval]= [1, ixalloc]; ixalloc= ixalloc+1
              plfsdefs[lutval]= l0def
    #print "allocShared3:", plfs, ixalloc
    for i in range(len(pl0funs)):
      if pl0funs[i]==None: break   # nothing to allocate
      lutval= list2str(pl0funs[i][0])
      if plfs.has_key(lutval):
        #print "allocShared3:allocated already. ", lutval
        plfs[lutval][0]= plfs[lutval][0]+1
      else:
        #print "allocShared5:new_allocation.:", lutval, plfs
        plfs[lutval]= [1, ixalloc]; ixalloc= ixalloc+1
      if ixalloc<=2:
        newlf[plfs[lutval][1]]= lutval
    if lf34:
      self.l0funs34= plfs
      #print "aS_l0f34:"
      self.l0funsdefs34= plfsdefs  # corresponding l0 definitions
    else:
      self.l0funs= plfs
      #print "aS_l0f:"
      self.l0funsdefs= plfsdefs  # corresponding l0 definitions
    #print plfs,"\n",plfsdefs
    #print "newlf:",newlf,"\nixalloc:%d"%ixalloc
    if ixalloc>2:
      errmsg="not allocated (>2 l0fs used in partition):"
      newlf=None
    #print "allocShared:rc::", newlf,errmsg
    return newlf,errmsg
  def prt_FIXrc(self, rc, cltds, symbols, shrname):
    if rc:
      PrintError(rc,self)
    else:
      print string.strip(cltds) + "   -> " + symbols.get(shrname)
    if (self.strict=="strict") and (preproc.lumi_source != "dim"):
      #PrintError("strict required, but luminosity DIM service not available.lumi_source:"+str(preproc.lumi_source),self)
      PrintWarning("Luminosity not available, using default "+\
        str(preproc.lumi)+"Hz/ub for automatic calculation of downscale factor.",self)
    return
  def loadfile(self, inf):
    """example of .partition file (spaces only between TDs, LTUs!):
    BC1 2
    BCM1 356L 25H
    Clusters:
    Clustername
    MB(L0pr=356ms,BCM1,all,pf1) SC(pf1)
    TPC
    Clust1name
    MB(pf1,pf2) SC
    TPC TRD SPixelD
"""
    section='Shared'
    filter= trigdb.TrgFilter(self.name)
    self.downscaling=None
    while 1:
      cltds= redline(inf)
      #print "cltds:",cltds
      if not cltds: break
      #if cltds[0]=='#':continue   moved to redline
      if string.find(cltds, 'Version:')==0:   # has to be first line
        verline= string.split(cltds,' ')
        print "verline:",verline
        self.version= string.strip(verline[1])
        if len(verline)>2:
          if "downscaling" == string.strip(verline[2]):
            # replace 12345678[789] and 0.0% strings in this .partition
            import downscaling
            v0rate= downscaling.getactv0rate()
            if v0rate==None:
              PrintError("V0 rate not choosen in ACT", self); break
            self.downscaling= downscaling.Downscaling(v0rate)
            if self.downscaling.v0and==None:
              PrintError("downscaling.rates not found", self); break
        print "version:",self.version, "ds:",self.downscaling
        continue
      if string.find(cltds, 'Clusters:')==0:
        section= 'Clusters'
        continue
      if section == 'Shared':
        #print "Shared:",cltds
        if self.downscaling!=None:
          cltds= self.downscaling.replace_inline(cltds)
        cltdsa= string.split(cltds)
        if len(cltdsa)<2:
          PrintError("at least 2 items in shared section expected:%s"%cltds,self)
          break
        shrname= cltdsa[0]
        #print "Shared2:",cltdsa
        if cltdsa[1]=="REPL":               # replacement definition
          ixstart= cltds.find(" REPL ")+6
          symbols.add(shrname, cltds[ixstart:-1])
          print string.strip(cltds)
        elif (cltdsa[1]=="FIXLUM") or (cltdsa[1]=="FIXLOSS") or \
          (cltdsa[1]=="FIXPOWER"):
          for ch in shrname:
            if not (ch in symchars):
              PrintError("Bad FIX* name:%s. Allowed chars:%s"%(shrname, symchars))
          # shrname FIXPOWER n floatlum
          fixll= " " + cltdsa[1] + " "
          ixstart= cltds.find(fixll)+len(fixll)
          # cltds[ixstart:] lum
          rc= symbols.add_FIXLL(shrname, cltds[ixstart:], cltdsa[1])
          self.prt_FIXrc(rc, cltds, symbols, shrname)
        else:                               # BC1/2 RND1/2 BCM* PF* or SDG
          sr= findSHR(shrname)
          #print "Shared4:", sr
          if sr!=None: 
            sr.setValue(string.strip(cltds[len(sr.name)+1:-1]))
          else:
            sr= self.sdgs.find(shrname)
            if sr!=None: 
              PrintError("incorrect (double def?) sync downscaling resource line:"+cltds, self)
            else:
              rc=self.sdgs.add(shrname, cltdsa[1])
              if rc:
                PrintError(rc,self)
          #print "Shared selfloaderrors:",self.loaderrors
        continue
      # Clusters section 
      if self.version!='0': # (Version>1: 3 lines per cluster):
        clusname= string.strip(cltds)      # cluster name
        if clusname=='':
          PrintWarning("Empty line  in Clusters: section ignored")
          continue
          #clusname=str(len(self.clusters))   # "0, 1... ", read from file
        cltds= redline(inf)                   # classes line
        if self.downscaling!=None:
          cltds= self.downscaling.replace_inline(cltds)
      else:                 # Version 0: 2 lines per cluster (no name stored)
        clusname=str(len(self.clusters))   # "0, 1... "
      if len(cltds)<2:
        PrintError("Bad or missing Classes line  in Clusters: section",self)
        break
      #if line[0] == "\n": continue
      #if line[0] == "#": continue
      totclasses= self.getRR(None)[0]
      asscls=[]; ltus=[]; 
      for clstring in string.split(cltds):    #-------------TDS
        # clstring: tds(pf1,pf2,...) or tds or tds()
        if (totclasses+len(asscls))>=50:
          PrintError(clstring+" -ignored (>50 classes)", self)
          continue
        else:
          trgclass=TrgClass(clstring, None, clusname)
          if trgclass.trde==None:
            PrintError(clstring+" -invalid class definition", self)
            continue
          #trgclass.prtClass()
          inds= trgclass.trde.getInpDets()
          #print "fcls:",trgclass.getclsname(),"input dets:",inds
          filteredout= False
          # 1. apply 'class replacement/skip' filter
          msg,err= trgclass.applyClassFilter(filter)
          if err=="skipped":   #skip to be done
            PrintWarning(msg+':'+err)
            continue
          if err=="ok":   #replacement done
            PrintWarning(msg+':'+err)
          elif err!="":   # error when replacing...
            PrintError(msg+":"+err, self)
            continue      # try to find more errors
          else:
            # 2. apply 'class skip' filter (not filtered in 1.)
            for det in inds:
              if filter.isExcluded(det):
                PrintWarning("%s filtered out (provided by %s)"%\
                  (trgclass.getclsname(),det))
                filteredout=True; break
            if filteredout: continue
        #
        if trgclass.trde:      # EMPTY or known descriptor !
          asscls.append(trgclass)
        else:
          # test for L0pr,BCM1/2/3/4, all, rare:
          PrintError(clstring+" -unknown Trigger descriptor", self)
      clltus= redline(inf)         # LTUs line
      for name in string.split(clltus):   #-------------LTUS
        ltu= TDLTUS.findLTU(name)
        #print "loadfile:",name, ltu.name
        if ltu:
          ltus.append(ltu)
        else:
          PrintError(name+" -unknown LTU  (missing in VALID.LTUS file)", self)
      #print "loadfile2:",len(asscls), len(ltus)
      if len(asscls)==0:
        PrintWarning("Empty cluster %s ignored"%clusname)
      else:
        self.clusters.append(TrgCluster(clusname,self,asscls,ltus))
    if len(self.clusters)==0:
      PrintError("Empty partition:%s. At least 1 cluster must be defined."%self.name, self)
    rsrcs= self.getRR("print to stdout")
    if BCM_NUMBER==4:
      if rsrcs[1]>6:
        PrintError("More than 6 negating classes",self)
    #print "selfloaderrors:",self.loaderrors
    # following assignment causes .pcfg saved from 'parted interactive session'
    # and by programaticaly by load/savepcfg INCOMPATIBLE !
    # I.e.: ALWAYS before LOAD_PARTITION, do programatic load/savepcfg
    # cls.clanumlog is needed in savercfg() -see comment there. 
    clanum=1
    for cluster in self.clusters:
      for cls in cluster.cls:
        cls.clanumlog=clanum
        clanum= clanum+1
  def showShared(self, minst, ix):
    if self.shmaster: 
      myw.RiseToplevel(self.shmaster); return
    self.shmaster=myw.NewToplevel(self.name+":shared resources", 
      self.hideShared, bg=COLOR_SHARED)
    for shrres in SHRRSRCS:
      if shrres.name[:3]=='l0f':
        print "showShared: l0fun* should not be shown"
      shrres.show(self.shmaster)
    #  newtd= myw.MywButton(self.master, label="New TD", side= RIGHT,
    #    cmd=self.donewtd)
  def hideShared(self, event=None):
    for shrres in SHRRSRCS:
      shrres.hide()
    #self.shmaster.destroy()
    self.shmaster=None
  def addCluster(self, minst,ix ):
    #if len(self.clusters) < CTP.getmaxclusters():
    if len(self.clusters) < 6:
      #clusname=str(len(self.clusters))
      clusname= self.inventClusterName()
      self.clusters.append(TrgCluster(clusname,self))
      self.clusters[-1].show(self.pclfr)
      self.setActiveCluster(self.clusters[-1])
  def findCluster(self, clu):
    ix=0
    for ix in range(0,len(self.clusters)):
      if self.clusters[ix]==clu:
        return ix
    return None
  def findClusterName(self, cluname):
    ix=0
    for ix in range(0,len(self.clusters)):
      print "findClusterName:", self.clusters[ix].name
      if self.clusters[ix].name==cluname:
        return ix
    return None
  def rmCluster(self, clu):
    ix= self.findCluster(clu)
    del self.clusters[ix]
  def renActiveCluster(self, minst, ix):
    print "renActiveCluster:", minst, ix
    if self.activeCluster==None: return
    self.satlw=Toplevel()
    self.satlw.title("Rename cluster "+self.activeCluster.name)
    self.saentry= myw.MywEntry(self.satlw, label="New name:",side=TOP,
      helptext="""Type new cluster name (no blank characters!) and ENTER key.
""",
      bg=COLOR_SHARED, bind='r',
      defvalue=self.activeCluster.name, cmdlabel=self.clusterRenamed)
  def clusterRenamed(self, event=None):
    newname1= string.strip(self.saentry.getEntry())
    if self.findClusterName(newname1)!=None:
      newname= self.inventClusterName()
      PrintWarning("Ambiguous cluster name %s. %s used instead"%(newname1, newname))
    elif newname1.rfind(' ')>=0:
      newname= newname1.replace(" ","_")
      PrintWarning("Spaces in cluster name replaced by '_'")
    else:
      newname=newname1
    self.activeCluster.name= newname
    self.activeCluster.refreshTDSheadLabel()
    self.activeCluster.refreshClassNames()
    self.saentry.destroy()
    self.satlw.destroy()
    #self.satlw=None
  def delActiveCluster(self, minst, ix):
    if self.activeCluster==None:
      return
    self.activeCluster.prt()
    self.activeCluster.delete()
    self.activeCluster=None
  def setActiveCluster(self, actclu):
    # only shown cluster can become active!
    for clu in self.clusters:
      if clu==None: continue
      #print "setActiveCluster:",clu.name
      if actclu==clu:
        clu.activeCluster()
        self.activeCluster=clu
      else:
        clu.deactiveCluster()
  def inventClusterName(self):
    for clun in TrgPartition.clustnames:
      rc=clun
      for clu in self.clusters:
       if clu==None: PrintWarning("None in self.clusters")
       if clun== clu.name: 
         rc=None
         break     
      if rc: break
    if rc==None: 
      IntErr("inventClusterName: cluster name not assigned")
      rc=None
    return rc
  def sdgGroups(self, minst,ix):
    self.sdgs.show()
def getNames():
  """
  return: the list of names of all the partitions in trigger database
  """
  cwd= os.getcwd(); os.chdir(CFGDIR); 
  fnames= glob.glob("*.partition")   ; os.chdir(cwd);
  def getn(x):return string.split(x,'.')[0]
  ar= map(getn,fnames)
  ar.sort()
  #print "ar:",ar
  return ar

class TrgLoadSave:
  """ Create empty/loaded TrgPartition object
  """
  def __init__(self, master, partname):
    """partname: None -new partition to be created
    rel_path/part_name
    """
    self.lsmaster= master
    if partname:
      self.lsmaster.title(partname)
    else:
      self.lsmaster.title("No partition")
    self.part=None
    self.partsbut=None
    #xx self.clusfr=None
    self.clusfr= myw.MywFrame(self.lsmaster,side=BOTTOM)
    #checkbut= myw.MywButton(master,label="Check",side=RIGHT,cmd=self.checkall)
    self.menubar= myw.MywMenu(self.lsmaster,helptext="""
File    -save in file/abandon current work
-------------------------------------------------------------
Show    -the way, how/what the information is shown
  Shared resources:
    In separate window, show the 'shared resources' required by this partition.

  classes':
    Each class in the cluster is identified by its name.
    The class name is derived automaticaly from trigger 
    descriptor name used by the class, and can be changed 
    in 'class definition' window.
    The following information can be choosen to be shown together with
    the class name:
    TDSname 
    p/fs             -shows the names of used past future protection circuits
    optional inputs  -scaled-down BC and Random trigger generators
    BCmasks          -BC masks used with the class
    All/Rare
    L0prescaler      -the value of prescaler to be set for the class
-------------------------------------------------------------
Cluster -add new/delete active cluster
""")
    self.lsmaster.config(menu=self.menubar)
    self.filemenu= self.menubar.addcascade('File')
    self.showmenu= self.menubar.addcascade('Show')
    self.clustermenu= self.menubar.addcascade('Cluster')
    self.losafr= myw.MywFrame(self.lsmaster,side=BOTTOM) 
    #menubar.entryconfigure(1, background=COLOR_WARN)
    #menubar.entryconfigure(2, background=COLOR_OK)
    #self.filemenu.addcommand('Load', self.savePart, 'disable')
    self.filemenu.addcommand('Save', self.savePart,"disable")
    self.filemenu.addcommand('Save as', self.savePartAs,"disable")
    self.filemenu.addcommand('Cancel', self.cancelPart)
    self.filemenu.addcommand('Quit', self.quitPart)
    self.showmenu.addcommand('Shared resources', None)
    self.showmenu.addcommand('Sync downscaling groups', None)
    self.showmenu.addcommand('Shared resources reservation', None)
    tdsnames=[]
    selshow= self.showmenu.addcascade('Descriptor')
    for tds in TDLTUS.tds:
      tdsnames.append(tds.name)
    tdsnames.sort()
    for ix in range(len(tdsnames)):
      if ix%20==0: cb=True  
      else: cb= False
      selshow.addcommand(tdsnames[ix], self.showDescriptor, columnbreak=cb)
    #
    # following to be  implemented in ctp.py:
    #self.showmenu.addcommand('CTP configuration', None)
    selshow= self.showmenu.addcascade("classes'") #,columnbreak=4)
    selshow.addcommand('Class name', self.changeShown)
    selshow.addcommand('TDSname', self.changeShown)
    selshow.addcommand('p/fs', self.changeShown) #, columnbreak=True)
    selshow.addcommand('opt. inputs', self.changeShown)
    selshow.addcommand('BCmasks', self.changeShown)
    selshow.addcommand('All/Rare', self.changeShown)
    selshow.addcommand('L0prescaler', self.changeShown)
    selshow.addcommand('Class group', self.changeShown)
    selshow.addcommand('Inputs', self.changeShown)
    self.clustermenu.addcommand('Add new', None)
    self.clustermenu.addcommand('Delete active', None)
    self.clustermenu.addcommand('Rename active', None)
    if partname:
      self.loadPartition(partname)
    else:
      self.cancelPart(self.filemenu)
      #self.doNames()
  def doNames(self):
    #xx self.clusfr= myw.MywFrame(self.lsmaster,side=BOTTOM)
    #self.itn=[ ["Refresh this list","removevalue"],
    #xx if self.partsbut!=None: return
    self.itn=[
    ["empty_partition","empty_partition", self.loadPartition]]
    #pnames= getNames(); print "pnames:",pnames
    for basen in getNames():
      self.itn.append([basen,basen+'.partition',self.loadPartition])
    #xx
    self.partsbut= myw.MywxMenu(self.clusfr, label="Load partition:",
      helptext="""Choose the name of the partition to load, or 
'empty_partition' to start with empty partition""", 
      side='left',items=self.itn, delaction=self.refreshNames)
  def refreshNames(self, sf1):
    #print "refreshSeqNames:self.f1:",self.f1,"sf1:",sf1
    self.partsbut.destroy(); self.partsbut=None
    self.doNames()
  def doSaveEntry(self):
    if self.partsbut:
      self.partsbut.destroy()
      self.partsbut=None
    #print "doSaveEntry"
    self.filemenu.enable(["Save","Save as","Cancel"])
    self.menubar.enable(["Show","Cluster"])
  def cancelPart(self, minst=None, ix=None):
    minst.disable(["Save","Save as","Cancel"])
    self.menubar.disable(["Show","Cluster"])
    if self.part: 
      self.part.destroy()    # partition object itself
      self.part=None
    #xx  self.clusfr.destroy(); self.clusfr=None
    #self.hideshared()
    if self.partsbut==None: self.doNames()
  def quitPart(self, minst, ix):
    sys.exit(0)
  def savePart(self, minst, ix):
    #print "ix:",ix
    self.part.save(self.part.name)
  def savePartAs(self, minst, ix):
    #print "ix:",ix
    self.satlw=Toplevel()
    self.satlw.title("Old part. name: "+self.part.name)
    self.saentry= myw.MywEntry(self.satlw, label="New name:",side=TOP,
      helptext="""Type new part. name (without .partition suffix) and ENTER key.
The window will be closed after saving current configuration.
""",
      bg=COLOR_SHARED, bind='r',
      defvalue=self.part.name, cmdlabel=self.savePartAs2)
  def savePartAs2(self, event=None):
    newname= string.strip(self.saentry.getEntry())
    rc=self.part.save(newname)
    if rc==0:
      self.saentry.destroy()
      self.satlw.destroy()
      #self.satlw=None
    self.part.name= newname
    self.lsmaster.title(self.part.name)
  def loadPartition(self,partname=None):
    #print "loadPartititon:"
    if partname==None:
      ix= self.partsbut.getIndex()
      partname=self.itn[ix][0]
    self.part= TrgPartition(partname)
    if self.partsbut:
      self.partsbut.destroy(); 
      self.partsbut=None
    self.part.show(self.clusfr)
    self.lsmaster.title(self.part.name)
    self.doSaveEntry()
    self.showmenu.setcommand('Shared resources', self.part.showShared)
    self.showmenu.setcommand('Shared resources reservation', 
      self.part.getRR)
    self.showmenu.setcommand('Sync downscaling groups', self.part.sdgGroups)
    self.clustermenu.setcommand('Add new', self.part.addCluster)
    self.clustermenu.setcommand('Delete active', self.part.delActiveCluster)
    self.clustermenu.setcommand('Rename active', self.part.renActiveCluster)
    #self.lsmaster.after_idle(self.doSaveEntry)
  def showDescriptor(self,menuinstance, ix):
    tosee= menuinstance.entrycget(ix, "label")
    #print "showDescriptor:",tosee
    TDLTUS.findTD(tosee).trdprt()
  def changeShown(self,menuinstance, ix):
    if menuinstance.ixchoosen!= None:
      menuinstance.entryconfigure(menuinstance.ixchoosen, 
        background=COLOR_NORMAL)
    menuinstance.entryconfigure(ix, background=COLOR_OK)
    menuinstance.ixchoosen= ix
    tosee= menuinstance.entrycget(ix, "label")
    for clus in self.part.clusters:
      for ixcl in range(len(clus.cls)):
        clas2mod= clus.cls[ixcl]
        #newname= clas2mod.clsbut.getLabel()
        newname= 'leave'
        newcolor=None
        #print "changeShown: name:",newname
        if tosee=='p/fs':
          newname= clas2mod.getTXTpfs()[:-1]
          if newname:
            newcolor=COLOR_TDSPFS
        elif tosee=='Inputs':
          newname= clas2mod.trde.getInputs()
        elif tosee=='Class group':
          newname= str(clas2mod.classgroup)
        elif tosee=='Class name':
          newname= clas2mod.clsname
        elif tosee=='TDSname':
          newname= clas2mod.trde.name
        elif tosee=='opt. inputs':
          newname= clas2mod.getTXTbcrnd(text='saving')[:-1]
        elif tosee=='BCmasks':
          newname= clas2mod.getTXTbcms()[:-1]
        elif tosee=='All/Rare':
          if clas2mod.allrare==0:
            newcolor= COLOR_RARE
          else:
            newcolor= COLOR_NORMAL
        elif tosee=='L0prescaler':
          newname= str(clas2mod.L0pr)
        if newcolor:
          clus.cls[ixcl].clsbut.setColor(newcolor)
        if newname!='leave':
          # always show class name:
          newname= clas2mod.getclsname()+'\n'+newname
          clus.cls[ixcl].clsbut.setLabel(newname)
        #cl.configure(name=...)
  def checkall(self):
    print "checkall:",self.part
def main():
  global partw
  """
  parted.py partname        -open GUI to edit partition in $dbctp/pardefs
  parted.py partname r      -create test version of .rcfg file (.debug)
    test version of .rcfg file contains:
    - logical clusters and classes (not hardware numbers!)
    See pydim/pydimserver.py, how .rcfg file is created
  """
  #TDLTUS.initTDS() 
  errfile= open("/tmp/parted.log","w")
  errfile.write(TDLTUS.loaderrors)
  errfile.close()
  #f= Tk()
  partname=None
  if len(sys.argv)>=2:
    partname=sys.argv[1]
  if len(sys.argv)>2:
    if sys.argv[2]=='r':   # create .rcfg file for given partition
      # see v/vme/pydim -production code invoking parted -> .rcfg
      part= TrgPartition(partname, "strict")
      #part= TrgPartition(partname)
      if part.loaderrors:
        print "Errors:"
        print part.loaderrors
      elif len(sys.argv)>3:
        part.savercfg("%s 1234 %s"%(partname, sys.argv[3]))
      else:
        part.savercfg()
        #part.savercfg("a 115812 0x5008a 1 0 2 0 0 0 1 45 2 46 3 0 0 0 0 4 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0x0 0x0 0x0 0x0")
      return
    else:                 # srgv[2]: source directory with .partition/.pcfg)
      print "bad argv[2]:",argv[2]
      return
  else:     # edit partition
    f= Tk()
    partw= TrgLoadSave(f,partname)
    # print load errors + info what was loaded:
    partw.part.prt()
  #P1.show(f)
  #P2.show(f)
  f.mainloop()
  
if __name__ == "__main__":
    main()

