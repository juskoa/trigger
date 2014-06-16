#!/usr/bin/python
import os,os.path,sys,string,time
#sys.path.append("./")
#import pdb

# working (r/w) directory
HOST=os.popen('hostname').read().rstrip()
if os.environ.has_key('VMEWORKDIR'):
  TRGWORKDIR= os.path.join(os.environ['VMEWORKDIR'], "WORK")
else:
  # apache or validate(for ACT) case: 
  if HOST=='avmes' or HOST=='pcalicebhm10' or HOST=='alidcscom835' or HOST=='alidcscom706':
    VMEWORKDIR="/home/alice/trigger/v/vme/"
  elif HOST=='alidcscom188':
    VMEWORKDIR="/home/alice/trigger/v/vme/"
  else:
    VMEWORKDIR="/tmp/"
  TRGWORKDIR="/tmp/"
#
if os.environ.has_key('VMECFDIR'):
  if os.environ.has_key('dbctp'):
     #validate in ACT shares ltuclient environment. dbctp has higher priority:
    TRGDBDIR= os.environ['dbctp']
  else:
    # DB (can be r/o) directory: VALID.LTUS ... files are here:
    TRGDBDIR= os.path.join(os.environ['VMECFDIR'], "CFG/ctp/DB")
  if os.environ.has_key('dbctp_pardefs'):
    CFGDIR= os.environ['dbctp_pardefs']
  else:
    # .partition .pcfg files go here:
    CFGDIR= os.path.join(os.getenv('VMECFDIR'), "CFG/ctp/pardefs")
else:
  # apache or validate(for ACT) case:  we need access to VALID.CTPINPUTS
  validate_case=None
  if HOST=='pcalicebhm05':
    os.environ['VMECFDIR']= "/data/ClientCommonRootFs/usr/local/trigger/vd/vme/"
  elif (HOST=='pcalicebhm10') or (HOST=='avmes')or (HOST=='alidcscom835') or (HOST=='alidcscom706'):
    #os.environ['VMECFDIR']= "/home/dl/root/usr/local/trigger/devel/v/vme/"
    os.environ['VMECFDIR']= "/home/dl6/local/trigger/v/vme/"
  elif HOST=='alidcscom188':
    os.environ['VMECFDIR']= "/data/dl/root/usr/local/trigger/stable/v/vme/"
  elif HOST=='alidcscom026':
    os.environ['VMECFDIR']= "/data/ClientCommonRootFs/usr/local/trigger/v/vme/"
  else:
    validate_case='yes'
  if validate_case=='yes':
    TRGDBDIR= os.path.join(os.environ['dbctp'])
    CFGDIR= os.path.join(os.environ['dbctp_pardefs'])
  else:
    TRGDBDIR= os.path.join(os.environ['VMECFDIR'], "CFG/ctp/DB")
    CFGDIR= os.path.join(os.getenv('VMECFDIR'), "CFG/ctp/pardefs")
#

def PrintError(fstr):
  print "Error:",fstr
def Arxiv(fname, arxdir):
  """ copy fname ->arxdir/date
  """
  import shutil
  date_time=time.strftime(".%Y-%m-%d.%H-%M-%S",time.localtime())
  #os.rename(fname_switch_inputs,fname_switch_inputs_arxiv+date_time)
  basefname= os.path.basename(fname)
  destname= os.path.join(arxdir,basefname+date_time)
  shutil.copyfile(fname, destname)
  print "Arxiv: %s -> %s/"%(fname,arxdir)
def pexecute(cmd):
  lsf= os.popen(cmd); ls=lsf.readlines(); lsf.close;
  print 'pexecute:',cmd
  for line in ls:
    if line==':\n': continue
    print line[:-1]
  return ls

class Table:
  def readtable(self, fn):
    """ read file of this format:
    a b c d #comment
    # comment
    
    Store the list of lists in self.ents:
    [a,b,c,d,#comment] or
    [#comment]
    """
    self.ents=[]
    self.filename=fn
    f= open(fn,"r")
    for line in f.readlines():
      if line[0] == "": continue
      if line[0] == "\n": continue
      if line[0] == "#": 
        self.ents.append([line])
        continue
      if line[-1] == "\n": line= line[:-1]
      ic= string.find(line, "#")
      if ic != -1:
        comm= [line[ic:]] ; line= line[:ic-1]
        self.ents.append(string.split(line)+comm)
      else:
        self.ents.append(string.split(line))
      #print self.ents[-1]
    f.close()
    return True
  def writetable(self, fn=None):
    if fn: self.filename= fn
    #f= open(self.filename+'n',"w")
    if hasattr(self, "ARXIV"):
      Arxiv(self.filename, self.ARXIV)
    f= open(self.filename,"w")
    print "Writing to:", self.filename
    for ix in range(len(self.ents)):
      if self.ents[ix][0][0]=='#':
        f.write(self.ents[ix][0])
      else:
        l=''
        for i in range(len(self.ents[ix])):
          l= l+' '+ self.ents[ix][i]
        f.write(l+'\n')
    f.close()
  def findixof(self, index, value):
    for ix in range(len(self.ents)):
      if self.ents[ix][0][0]=='#': continue
      if self.ents[ix][index]==value:
        return ix
    return None
    
class TrgLTU:
  def __init__(self, name, detnum, fo='0', focon='0', bsyinp='0'):
    self.name=name     # detector name
    self.detnum= int(detnum)  # 0.. (DAQ/ECS numbering)
    self.fo=int(fo)   # 1-6. 0: not connected
    self.focon=int(focon)   # 1-4
    self.bsyinp=int(bsyinp)   # 0: not connected
    self.vmecpu=None   #from ttcparts.cfg
    self.vmebase=None  #from ttcparts.cfg
    self.ttcitsw=None  #from ttcparts.cfg
  def getLogName26(self):
    #ln=/data/ClientLocalRootFs/alidcsvme007/home/alice/trigger/v/v0/WORK
    ln="/data/ClientLocalRootFs/%s/home/alice/trigger/v/%s/WORK/LTU-%s.log"%\
      (self.vmecpu, self.name.lower(), self.name)
    return ln
  #def show(self,master):
  #  ltubut= myw.MywButton(master, label=self.name, side= RIGHT)

class TrgLTUS:
  def __init__(self):
    self.ltus= readVALIDLTUS()
  def getLTUname(self, fo, focon):
    """return: LTUname connected to FO: fo(1..6)/focon(1..4)
             "" if not connected
"""
    for ltu in self.ltus:
      #print ltu.name, ltu.fo, ltu.focon
      if ltu.fo==fo and ltu.focon==focon:
        return ltu.name
    return ""
  def getLTUnameOfBusy(self, busyinput):
    """return: LTUname connected to BUSY input busyinput (1..24)
             "" if not connected
"""
    for ltu in self.ltus:
      #print ltu.name, ltu.fo, ltu.focon
      if ltu.bsyinp==0: return str(busyinput)
      if ltu.bsyinp==busyinput: 
        return ltu.name
    return ""
  def getdetnum(self,name):
    """return: ECS detector namber for LTU name"""
    for ltu in self.ltus:
      if ltu.name==name: return ltu.detnum
    return None
  def getTTCITSW(self,name):
    """return: ECS detector namber for LTU name"""
    for ltu in self.ltus:
      if ltu.name==name: return ltu.ttcitsw
    return None
  def findTTCITSW(self,chan):
    """return: LTU object TRGLTU connected to chan channel of TTCit switch"""
    for ltu in self.ltus:
      if  int(ltu.ttcitsw)== int(chan): return ltu
    return None

def readVALIDLTUS():
  """
  Read VALID.LTUS and ttcparts.cfg, create list of TrgLTU objects.
  return: list of created TrgLTU objects
  """
  ltus=[]
  f= open(os.path.join(TRGDBDIR, "VALID.LTUS"),"r")
  for line in f.readlines():
    if line[0] == "": continue
    if line[0] == "\n": continue
    if line[0] == "#": continue
    #print "readVALIDLTUS:",line
    #for ltu in string.split(line[:-1]):   26.9
    #  ltus.append(TrgLTU(ltu))       26.9
    nffc= string.split(line[:-1],'=')
    if len(nffc)==2:
      focon= string.split(nffc[1],' ')
      for ix in range(len(focon)-1,-1,-1):  # remove remains of double spaces
        if focon[ix]=='': del focon[ix]
      #focon: DAQnum fo focon bsyinp ltubase i2cchan i2cbran
      #                       >bsyinp... not processed
      #print "focon:",focon
      if len(focon)==1:        #not connected
        ltus.append(TrgLTU(nffc[0],focon[0]))
      elif len(focon)>1 and focon[1]=='0':        #not connected
        ltus.append(TrgLTU(nffc[0],focon[0]))
      elif len(focon)>=3:   #connected to FO
        bsy='0'
        if len(focon)>=4:   #busy input present
          if int(focon[3])>=0 and int(focon[3])<=24:   # and is valid
            if focon[3]!='0':
              bsy= focon[3]
          else:
            PrintError("Bad busy_input in VALID.LTUS file:\n"+line + '\n' + str(focon))
        if focon[1]>='1' and focon[1]<='6'and focon[2]>='1' and focon[2]<='4':
          #and focon[3]>='0' and focon[3]<='24':
          ltus.append(TrgLTU(nffc[0],focon[0], focon[1], focon[2], bsy))
        else:
          PrintError("Bad fanout.connector in VALID.LTUS file:\n"+line)
      else:
        PrintError("""Bad syntax.  
Only Detnum or
'Detnum FOnum ConnectorNum BSYnum ltubase i2cchan i2cbran' 
expected) in VALID.LTUS file:
"""+line)
    else:
      PrintError("Bad LTU=det#.fo#.con# in VALID.LTUS file:\n"+line)
  f.close() 
  # now update ltus with info from ttcparts.cfg:
  f= open(os.path.join(TRGDBDIR, "ttcparts.cfg"),"r")
  for line in f.readlines():
    if line[0] == "": continue
    if line[0] == "\n": continue
    if line[0] == "#": continue
    if line[-1] == "\n": line= line[:-1]
    nhb= string.split(line)   
    # name_small_caps vmecpu 0xbase rack dcs_crate ttcitSwitchChannel
    # 0               1      2      3    4         5
    if (nhb[1].find('altri')!=0) and  (nhb[1].find('alidcsvme')!=0):
      # case for TTCpartitions in various labs:
      PrintError("Line with CPU %s in ttcparts.cfg ignored"%nhb[1])
      continue
    notltu=True
    for ltu in ltus:
      if ltu.name == string.upper(nhb[0]):
        ltu.vmecpu= nhb[1]; ltu.vmebase= nhb[2] ; notltu=False
        if len(nhb)>5: ltu.ttcitsw= nhb[5]
        break
    if notltu:
      PrintError("%s in ttcparts.cfg not found in VALID.LTUS"%nhb[0])
  # mark 'not connected" ltus not present in ttcparts.cfg file:
  f.close() 
  for ltu in ltus:
    if ltu.vmecpu == None: ltu.fo=0
  return ltus

#------------------------------------------------------------- CTP
class TrgFilter(Table):
  def __init__(self, partname):
    if partname!="PHYSICS_1":   #filter applied ONLY on PHYSICS_1
      self.ents=[]; return
    fp= os.path.join(TRGDBDIR, "filter")
    if not os.path.exists(fp):
      self.ents=[]; return
    self.readtable(fp) 
    # 1. line: the list of detector names to be excluded. 
    # 2., 3.,... lines: Dx Dy       (Dx to be repalaced by Dy)
    # Classes using inputs from
    # these detectors will be ignored when .partition file loaded
  def isExcluded(self, ltuname):
    if len(self.ents)==0: return False
    for ent in self.ents:
      if ent[0][0]=='#': continue
      for name in ent:   # take first line (with the list of detectors)
        if ltuname==name: return True
      return False
    return False
  def getReplacement(self, dname):
    """ rc: new decriptor name or 
    None if dname not found or
    ""   if replacement not given (i.e. only 1 symbol in line)
    """
    for ent in self.ents:
      if ent[0][0]=='#': continue
      if ent[0]==dname:
        if len(ent)>1:
          return ent[1]
        return ""
    return None

class TrgCNAMES(Table):
  def __init__(self, conf="run2"):
    if conf=="run1":
      cnames= "cnames1.sorted2"
    else:
      cnames= "cnames.sorted2"
    fp= os.path.join(TRGDBDIR, cnames)
    self.readtable(fp) 
    # 0     1                2     3   4 
    # Name  RelPositionFrom0 Board CGT ctp
  def getRelPosition(self, name):
    for ent in self.ents:
      if ent[0][0]=='#': continue
      if ent[0]==name: return ent[1]
    return None
  def getRelPositions(self, names):
    positions=[]
    for ix in range(len(names)):
      p= self.getRelPosition(names[ix])
      positions.append(p)
    return positions

class TrgVALIDINPUTS(Table):
  def __init__(self):
    fp= os.path.join(TRGDBDIR, "VALID.CTPINPUTS")
    self.readtable(fp) 
    # 0    1 2   3     4   5      6      7    8    9     10       11
    # Name = Det Level Sig InpNum DimNum Conf Edge Delay Deltamin Deltamax
  def isL0f(self, ent):
    c3= ent[0][0:3]
    if (c3=='l0f') or (c3=='l0A') or (c3=='l0B'): return True
    return False
  def getL012inputs(self, levelc):
    """ levelc: '0','1' or '2'
    rc: 'Lx inputs: 1:xHWU 4:xEMC ...'
    """
    ins=[]
    for ix in range(24):
      ins.append(None)
    for ent in self.ents:
      if ent[0][0]=='#': continue
      if self.isL0f(ent): continue
      if (ent[3]==levelc) and (ent[7]=="1"):
        ins[int(ent[5])-1]= ent[0]
    rc='L%c inputs: '%levelc
    for ix in range(len(ins)):
      if ins[ix]: rc= rc+"%s:"%(ix+1)+ins[ix]+' '
    return rc
  def getdn(self, ent, niceout=True):
    if ent[0][0]=='#': return None
    if self.isL0f(ent): return None
    if ent[7]!="1": return None  # not configured
    detn= ent[2]
    if niceout:
      if ent[2]=='DAQ':
        detn= 'DAQ_TEST'
      elif ent[2]=='EMCAL':
        detn= 'EMCal'
      else:
        detn= ent[2]
    return detn
  def prtall(self,ofile):
    for ent in self.ents:
      detn= self.getdn(ent)
      if detn==None: continue
      line= "%s %s %s %s %s\n"%(ent[0], detn, \
        ent[3], ent[4], ent[5])
      ofile.write(line)
  def addl12(self,l0inps):
    for ent in self.ents:
      detn= self.getdn(ent, False)
      if detn==None: continue
      if ent[3]=='0': continue   # L0 already taken from L0.INPUTS
      if l0inps.has_key(detn):
        l0inps[detn]= l0inps[detn] + ' ' + ent[0]
      else:
        l0inps[detn]= ent[0]
 
  def getEdgesDelays(self):
    """ this method not used (ctp_alignment file created directly in ctpproxy)
    rc:
    input edge delay
    input edge delay
    ...
    where:
    input: L0/1/2 input name
    edge:  P or N  (0 or 1 in VALID.CTPINPUTS)
    delay: 0-15 in BCs
    """
    rc=''
    for ent in self.ents:
      if ent[0][0]=='#': continue
      if self.isL0f(ent): continue
      if ent[7]=='1':
        if ent[8]=='0': edge=' P ' 
        else: edge=' N '
        rc= rc+ent[0]+edge+ent[9]+'\n'
    return rc
  def log2tab(self, lf4):
    """ lf4: logical function of first four L0 inputs (see txtproc.log2tab)
    The names (e.g.: 0VGA...) are used in lf4.
    Operation:
    - check if all used inputs are first 4 L0 inputs
    - convert logical expression to lookuptable
    Return: 0xabcd lookuptable
            None if error (error message printed to stdout)
    """
    import txtproc
    lf4order=['','','',''] ; err=''
    ok,vie= txtproc.varsInExpr(lf4)
    if ok=='OK':
      for ixv in range(len(vie)):
        ix= self.findixof(0, vie[ixv])
        if ix!=None:
          if self.ents[ix][3]=='0':
            inpn= self.ents[ix][5] ; inpnint= int(inpn)
            #pdb.set_trace() 
            if (inpnint>=1) and (inpnint <= 4):
              lf4order[inpnint-1]= vie[ixv]
            else:
              err="%s%s is not one of the first four L0 CTP inputs. "%(err,vie[ixv])
          else:
            err="%s%s is not L0 CTP input. "%(err, vie[ixv])
        else:
          err="%s%s is not valid CTP input. "%(err, vie[ixv])
    else:
      err= vie
    if err != '':
      print "ERROR: %s"%err
      return None
    hx= txtproc.log2tab(lf4, lf4order)
    return hx
      
class TrgL0INPUTS(Table):
  def __init__(self):
    fp= os.path.join(TRGDBDIR, "L0.INPUTS")
    self.readtable(fp) 
    # 0   1   2     3    4  5   6      7    8   9    10       11       12
    # sin det cable name eq sig dimnum edge del conf deltamin deltamax comment
  #def findixof(self, index, value):
  #  for ix in range(len(self.ents)):
  #    if self.ents[ix][0][0]=='#': continue
  #    if self.ents[ix][index]==value:
  #      return ix
  #  return None
  def prtnames(self):
    all={}
    print "\nAvailable L0 inputs:"
    print "---------------------"
    for en in self.ents:
      if en[0][0]=='#': continue
      if en[9]!='1': continue      # what does it mean 2?
      if all.has_key(en[1]):
        all[en[1]]= all[en[1]]+' '+ en[3]
      else:
        all[en[1]]= en[3]
      #if en[1]=='PHOS': print '!:',en
    for en in all.keys():
      print "%s:%s"%(en, all[en])
    return all

class Trgctpcfg(Table):
  def __init__(self):
    fp= os.path.join(TRGDBDIR, "ctp.cfg")
    self.readtable(os.path.join(TRGDBDIR, "ctp.cfg"))
    # 0       1  2  
    # parname v1 v2 ...
  def getTIMESHARING(self):
    ixts= self.findixof(0, "TIMESHARING")
    if ixts !=None:
      return self.ents[ixts][1:]
    else:
      return ['1','2','3','4','5','6','7','8','9']
  def getINT12(self,int12):
    ixts= self.findixof(0, "L0_INTERACTSEL")
    if ixts==None: return None
    intsel= eval(self.ents[ixts][1])
    if int12==1:
      intsel= intsel&0x1f
    else:
      intsel= intsel>>5
    ixts= self.findixof(0, "L0_INTERACT%d"%int12)
    if ixts==None:
      intdef=''
    elif len(self.ents[int(ixts)])<=1:
      intdef=''
    else:
      intdef= string.join(self.ents[ixts][1:])
    intsym=''
    if (intsel & 1)==1:     intsym= intdef
    if (intsel & 0x2)==0x2: intsym= intsym+"|BC1"
    if (intsel & 0x4)==0x4: intsym= intsym+"|BC2"
    if (intsel & 0x8)==0x8: intsym= intsym+"|RND1"
    if (intsel & 0x10)==0x10: intsym= intsym+"|RND2"
    if len(intsym)>0:
      if intsym[0]=='|': intsym= intsym[1:]
    #print ":%s:"%intsym
    if intsym=='': intsym=None
    return intsym
     
class TrgSwitchLTU(Table):
  ALL={"9":"muon_trk muon_trg", "8":"v0", "10":"tpc", "11":"trd", 
       "12":"spd", "13":"acorde hmpid emcal","7":"sdd","6":"v0fee",
       "2":"ctp_l1_18"}
  # ctp_l1_18: special connection to CTP L1-18 for ZDC (1ZED)
  ARXIV=os.path.join(TRGDBDIR,"CTP.SWITCH.ARXIV")
  def __init__(self):
    fp= os.path.join(TRGDBDIR, "LTU.SWITCH")
    self.readtable(fp)   # cable name equaliser swin ltun 
    # ltun: one of keys in ALL
    #print "varshaskey:",hasattr(self, "ARXIV")
  def ltu2so(self, ltuname):
    """rc: switch out
    """
    for so in self.ALL.keys():
      if string.find(self.ALL[so], ltuname)>=0:
        return so
    return None
  def getltuix(self, ltuname):
    """rc: index to self.ents[] -list corresponding to ltuname
           or -1 if ltuname is not connected
    """
    so= self.ltu2so(ltuname)
    if so==None: return None   # bad name
    for ix in range(len(self.ents)):
      if self.ents[ix][0][0]=='#': continue
      if self.ents[ix][4]==so:
        print "getltuix:",ix
        return ix
    print "getltuix:-1"
    return -1                  # not connected
  def getltu(self, ltuname):
    ix= self.getltuix(ltuname)
    if ix: 
      if ix != -1: ix= self.ents[ix]
      else: ix= []
    return ix
  def setltu(self, ltuname, cable, ctpname, sin):
    ix= self.getltuix(ltuname)
    if ix ==None: return None
    if ix==-1:   # new entry
      so= self.ltu2so(ltuname)
      newe=[cable, ctpname, '0', sin, so]
      self.ents.append(newe)
      ix= len(self.ents)-1
    else:
      self.ents[ix][0]= cable
      self.ents[ix][1]= ctpname
      self.ents[ix][3]= sin
    return self.ents[ix]
  #def prt1(self):
  def prtall(self):
    print "\nCTPin -> LTUout(ltuname)"
    print  "-------------------------"
    for en in self.ents:
      #print "prtall:",en
      if en[0][0]=='#': continue
      if len(en) != 5:
        print "Error: wrong line in LTU.SWITCH file:",en
        continue
      if self.ALL.has_key(en[4]):
        ltuname= self.ALL[en[4]]
        print "%s -> %s(%s)"%(en[3],en[4],ltuname)
      else:
        print "Error: bad line in LTU.SWITCH file:",en

class TrgMasks(Table):
  def __init__(self):
    fp= os.path.join(TRGDBDIR, "VALID.BCMASKS")
    self.readtable(fp)   # cable name equaliser swin ltun 
  def getmask(self, name):   # name: 'bcmA', 'bcmEMPTY',...
    for ix in range(len(self.ents)):
      if self.ents[ix][0][0]=='#': continue
      if self.ents[ix][0]==name:
        #print "getmask:",name,self.ents[ix][1]
        return self.ents[ix][1]
    return ""
    
class TrgInp:
  def __init__(self, VCI=None, LI=None):
    # ctpinputs.cfg:
    #InName = Det Level Signature Inpnum Dimnum Configured Edge Delay DeltaMin  
    #0      1 2   3     4         5      6      7          8    9     10
    #DeltaMax ppn nameweb eq dimdns dimservice #anycomment
    # 11      12  13      14   15      16       17
    self.inp= []
    if (VCI==None) and (LI=None):
      for i in range(len(16)): self.inp.append("NA")
    elif VCI!=None:
      if len(VCI.ents)==12:
        for i in range(len(11)): self.inp.append(VCI.ents[i])
      else:
        print "Error: TrgInp VCIlen:",len(VCI.ents)
    else:   # adding LI, i.e. not found in VALID.CTPINPUTS
      if len(LI.ents)=>12:
        self.inp[0]= LI.ents[3]   # InName
        self.inp[1]= "="
        self.inp[2]= LI.ents[1]   # Det
        self.inp[3]= "0"          # Level
        self.inp[4]= LI.ents[5]   # Signature
        self.inp[5]= "0"          # Inpnum
        self.inp[6]= LI.ents[6]   # Dimnum
        self.inp[7]= LI.ents[0]   # Configured todo: set 0 if >48
        self.inp[8]= LI.ents[7]   # Edge
        self.inp[9]= LI.ents[8]   # Delay 
        self.inp[10]= LI.ents[10]   # DeltaMin
        self.inp[11]= LI.ents[11]   # DeltaMax
        self.inp[12]= "NA"         # ppn   
        self.inp[13]= LI.ents[2]   # nameweb
        self.inp[14]= LI.ents[4]   # eq
        self.inp[15]= "NA"         # dimdns
        self.inp[16]= "NA"         # dimservice
        self.inp[17]= LI.ents[12]   # anycomment
      else:
        print "Error: TrgInp LIlen:",len(LI.ents)
      
class TrgInputs:
  def __init__(self):
    inps= []   # array of TrgInp objects
  def addVCI(ent):
    inps.append(TrgInp(VCI=ent))
  def modifyVCI(ent):
    # add info from L0.INPUTS to existing info from VALID.CTPINPUTS:
  def addLI(ent):
    inps.append(TrgInp(LI=ent))
  def find(self, det, inpname):
    for ix in range(len(inps)): 
      if (inps[ix][1]==det) && (inps[ix][0]==inpname):
        return ix
    return None
    
#------------------ following classes used from TRG_DBED/scanrcfg.py
class TrgRcfgVals:
  def __init__(self):
    self.vals={}   # item: key:Nusages
  def regvalue(self, value):
    if self.vals.has_key(value):
      self.vals[value]= self.vals[value]+1
    else:
      self.vals[value]= 1   # first time use
    #print "value:",value,"registered. Usage:",self.vals[value]
  def prt(self):
    #print "prt:", self.vals
    for v in self.vals.keys():
      print " '%s':%d"%(v, self.vals[v])
class TrgRcfg:
  ''' Holds:
  self.inputs.vals: { inpname+sig:Nusages (should be always 1) }
  '''
  def __init__(self, basedir=None):
    self.section= self.ignore
    self.inputs= {}
    self.descriptors= {}
    self.classes= {}
    self.basedir= basedir
  def addrcfgall(self):
    import glob
    os.chdir(self.basedir)
    rnames= glob.glob('*.rcfg')
    #print rnames, len(rnames)
    for fn in rnames: self.addrcfg(fn)
  def addrcfg(self, fname):
    f= open(os.path.join(self.basedir, fname),"r")
    for line in f.readlines():
      if line[0]=='#': continue
      lsp= string.split(string.strip(line))
      if len(lsp)==0: continue
      #print "lsp:",lsp
      if string.find(lsp[0],':')>=0: self.setSection(lsp[0])
      else:
        self.section(lsp)   #process 1line of the current section
    f.close()
  def prtsection(self, section, head):
    print head
    for iname in section.keys():
      section[iname].prt()
  def prtrcfg(self):
    self.prtsection(self.inputs, "inputs:\n'InputName Signature':Usage")
    self.prtsection(self.descriptors, "descriptors:\n'DescName Inputs':Usage")
    self.prtsection(self.classes, "classes:\n'ClassName Desc Clust PF BCMASK Prescaler AllRare':Usage")
    
  def setSection(self, sect):
    if sect=='INPUTS:': self.section= self.sec_inputs
    elif sect=='L0FUNCTIONS:': self.section= self.sec_l0functions
    elif sect=='DESCRIPTORS:': self.section= self.sec_descriptors
    elif sect=='CLUSTERS:': self.section= self.sec_clusters
    elif sect=='BCMASKS:': self.section= self.sec_bcmasks
    elif sect=='CLASSES:': self.section= self.sec_classes
    else: sect= self.ignore
  
  def registerKeyValue(self,section, ki, value):
    """ register pair ki: value in disctionary section
    """
    if not section.has_key(ki):
      section[ki]= TrgRcfgVals()
    section[ki].regvalue(value)
  def ignore(self, lsp):
    print "ignore:", lsp 
    pass
  def sec_inputs(self,lsp):
    # value is: InputName+signature
    self.registerKeyValue(self.inputs, lsp[0], lsp[0]+' '+lsp[3])
  def sec_l0functions(self,lsp):
    print "l0functions:", lsp 
  def sec_descriptors(self,lsp):
    #print "descriptors:", lsp 
    # value: all the inputs defined for this descriptor
    dvalue= lsp[0]
    for ix in range(1, len(lsp)):
      dvalue= dvalue+' '+lsp[ix]
    self.registerKeyValue(self.descriptors, lsp[0], dvalue)
  def sec_clusters(self, lsp):
    #print "clusters:", lsp 
    pass
  def sec_bcmasks(self,lsp):
    #print "bcmasks:", lsp 
    pass
  def sec_classes(self,lsp):
    #print "classes:", lsp 
    # value is: ClassName+Desc+Clust+PF+BCM+Prescaler+Allrare
    self.registerKeyValue(self.classes, lsp[0], 
      lsp[0]+' '+lsp[2]+' '+lsp[3]+' '+lsp[4]+' '+lsp[5]+' '+lsp[6]+' '+lsp[7])

def main(argv):
  if len(argv) < 2:
    print """
  trigdb.py log2tab 'logical expression from L0inputs'
  trigdb.py prtinps     source: VALID.CTPINPUTS taken
  trigdb.py cables      source: L0.INPUTS (L0) + VALID.CTPINPUTS (L1+L2) 
  trigdb.py joininputs  L0.INPUTS+VALID.CTPINPUTS -> ctpinputs.cfg
"""
    return
  if argv[1]=='log2tab':
    vcis= TrgVALIDINPUTS()
    hx= vcis.log2tab(argv[2])
    if hx!= None: print hx
  elif argv[1]=='prtinps':
    a=TrgVALIDINPUTS()
    print a.getL012inputs('0')
    print a.getL012inputs('1')
  elif argv[1]=='cables':
    #print "todo..."
    a=TrgL0INPUTS()
    allds= a.prtnames()
    #print allds
    vci= TrgVALIDINPUTS()
    vci.addl12(allds)
    print # print allds
    for det in allds.keys():
      print "%s: %s"%(det, allds[det])
  elif argv[1]=='joininputs':
    vci= TrgVALIDINPUTS()
    l0i= TrgL0INPUTS()
    # first add all VALID.CTPINPUTS
    cis= TrgInputs()
    for ent in vci.ents:
      cis.addVCI(ent)
    # add L0.INPUTS (modifying already existing inputs in ci:
    for ent in l0i.ents:
      ixi= cis.find(ent[1], ent[3])   # check if already in ci:
      if ixi:
        cis.modifyVCI(ixi, ent)
      else:
        cis.addLI(ent)
    # check if CTP.SWITCH consistent
    print "rewriting ctpinputs.cfg..."
if __name__ == "__main__":
    main(sys.argv)

