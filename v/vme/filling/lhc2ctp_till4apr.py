#!/usr/bin/python
import sys,string,types
ORBITL=3564
CALIBbc=3009   #1st bc in orbit is 0
SHIFTS=(345, 3018)
#SHIFTS=(2610, 1719)   BEAM2 clock
BCS_BEFORE=85   # was 16, TRD request: 85
DIS_BEFORE=3
DIS_AFTER_AC=32
DIS_AFTER_B=128
def suborbit(a, b):
  """ cal. diffrence between 2 BCs b->a
  """
  if a>=b:
    rc= a-b
  else:
    rc= a + ORBITL - b 
  return rc
def prevbc(bc, bcs=1):
  #if bc>=1: return bc-1
  #return ORBITL-1
  if bc>=bcs: return bc-bcs
  return ORBITL+bc-bcs
def nextbc(bc,bcs=1):
  #if bc==(ORBITL-1): return 0
  #return bc+1
  if (bc+bcs)<=(ORBITL-1): return (bc+bcs)
  return bc+bcs-ORBITL
class BcAttrs:
  def __init__(self, buc1, buc2, parasit,trainlen):
    self.buc= [buc1, buc2]
    self.parasit= parasit
    self.train= trainlen # None or [lentgth,spacing] of train starting here
    self.isincomb= None  # True if this bunch is part of the comb
  def update(self,beam,buc):
    self.buc[beam-1]= buc
    #buckets[bc].buc= buckets[bc].buc+','+str(buc)
  def bucstr(self):
    if self.buc[1]==None: return "%d"%(self.buc[0])
    if self.buc[0]==None: return "%d"%(self.buc[1])
    return "%d,%d"%(self.buc[0],self.buc[1])
class trainset:
  def __init__(self,name):
    self.name= name
    self.trains=[]   # item: [StartingBC, LastBC]
  def add(self, train):
    self.trains.append(train)
  def isin(self, bc, bc2=None):
    for train in self.trains:
      if (bc>=train[0]) and (bc<=train[1]):return True
      if bc2:
        if (bc2>=train[0]) and (bc2<=train[1]):return True
    return False
  def getlen(self):
    lng=0
    for trainix in range(len(self.trains)):
      train= self.trains[trainix]
      lng= lng+train[2]
    return lng
  def prt(self):
    totbs= self.getlen()
    print "%s (%d bunches) trains:"%(self.name, totbs)
    for trainix in range(len(self.trains)):
      train= self.trains[trainix]
      print "%d:%d-%d (%d bnches)"%(trainix,train[0],train[1],train[2])
class ACGROUP:
  def __init__(self, len, nbs, firstb,lastb,leftg,rightg):
    """ 
A: 1.1.1.1.1.1...1.1.11..
C: .........1.1.........1.1
               <---len->
                 F   L
    """
    self.len=len
    self.nbs= nbs        # 3 -number of bunches
    self.firstb= firstb  # F -BCnumber of 1st and last bunch
    self.lastb= lastb    # L
    self.leftg= leftg    # 3
    self.rightg= rightg  # 2
class biggaps:
  GAPBEFORE=1
  BC=0
  def __init__(self, lhcfs,size=2):
    self.lhcfs= lhcfs # LHCinfo() instance
    self.gaps=[]   # Bgaps in descending order, sorted by Bgap length
    self.size=size # number of Bgaps stored
    for x in range(size):
      self.gaps.append([-1,-1])   # bc(0..ORBITL-1), Bgap before this bc
      # i.e. (bc-Bgap) == BCnumber of Bbunch preceeding bc bunch 
    # find A,C non interleaving groups:
    self.nigs= {'A':[], 'C':[]}
    self.findNIG('A')
    #self.findNIG('C')
    # find combs
  def findNIG(self, beam):
    # find Non Interleaving Groups. beam: A or C
    self.lhcfs.getnext(beam, 0)
  def prt(self):
    for x in range(self.size):
      print "biggap %d: "%x, self.gaps[x]
  def update(self, bc, dif):   # dif: bc-bc_previous
    #print "biggaps:",bc,dif
    for ix in range(self.size):
      if dif > self.gaps[ix][self.GAPBEFORE]:
        #print "biggaps_update:",ix,bc,dif
        bcold= self.gaps[ix][self.BC]         
        gapold= self.gaps[ix][self.GAPBEFORE]         
        self.gaps[ix][self.BC]= bc
        self.gaps[ix][self.GAPBEFORE]= dif
        bc= bcold; dif= gapold
  def getempty(self, ix, bcbefore=BCS_BEFORE):
    """ rule: place bcbefore bcs before B in largest gaps.
    ix: gap number
    """
    ##global abcs
    msg=""
    bc= self.gaps[ix][self.BC]
    gap= self.gaps[ix][self.GAPBEFORE]
    #e2b=bcbefore
    #print "getempty:%d: from %d to %d"%(ix,bcbefore, gap)
    for e2b in range(bcbefore,gap):
      if gap<=e2b:
        e2b= gap/2
        msg= msg+" E->B is %d <70."%(gap/2)
      bc_e= suborbit(bc, e2b)
      if not self.lhcfs.abcs.has_key(bc_e): break
    b2e= gap-e2b
    if (b2e)<100:
      msg= msg+" B->E is %d <100."%(b2e)
    if msg !="": msg= "#"+msg
    #print "getempty:",ix, bc_e, msg
    return bc_e, msg
  #def getemptyM16(self, ix):
  #  """rule: place E into middle between 'start of gap' and 'end of gap -16'
  #  ix: gap number
  #  This method is called only for large gaps (>600)
  #  """
  #  msg=""
  #  bc= self.gaps[ix][self.BC]
  #  len2= (self.gaps[ix][self.GAPBEFORE]-16)/2
  #  bc_e= suborbit(bc, len2)
  #  return bc_e
def bu2bc(beamac12,buc):
  if type(beamac12)==types.StringType:
    if beamac12=='A': beam=1
    elif beamac12=='C': beam=2
    else:
      return 4000
  else: beam= beamac12
  bc= ((buc+10)/10 + SHIFTS[beam-1])%ORBITL
  return bc
def bc2buc_shift(shift, bc):
  if shift==bc:
    buc=bucz= ORBITL*10-9
  else:
    buc= ((bc + ORBITL - shift)%ORBITL)*10-10+1
    bucz= ((bc - shift)%3564)*10-9
    if buc!=bucz:
      print "bc2buc: %d != %d"%(buc,bucz)
  return buc
def bc2buc(beamac,bc):
  if (beamac=='A') or (beamac=='B') or (beamac=='E'):
    beam= 1
  elif beamac=='C':
    beam= 2
  else:
    return 'ERROR' 
  buc= bc2buc_shift(SHIFTS[beam-1], bc)
  if beamac=='A':
    #return str(buc)
    return buc,None
  elif beamac=='C':
    return None,buc
  else:
    buc2= bc2buc_shift(SHIFTS[1], bc)
    #buc2= ((bc + ORBITL - SHIFTS[1])%ORBITL)*10-10+1
  return buc,buc2
def bc2bucstr(beamac,bc):
  b1,b2= bc2buc(beamac,bc)
  if b1==None: return str(b2)
  if b2==None: return str(b1)
  return "%d,%d"%(b1,b2)

class LHCinfo:
  def __init__(self, fsname):
    self.fsname= fsname
    self.abcs={} ;    # self.abcs[bc] is A C B or E
    self.buckets={}   # the same keys as abcs, item:BcAttrs
    self.sortedABCE= None  # will be done later by do_sortedABCE()
  def getnext(self, abce, fromthis):
    """ find next  A B C or E bunch starting from fromthis+1
        return None if not found
    """
    ixbc= nextbc(fromthis)
    ix=0
    #while 1:
    # we probably cannot use yet self.sortedABCE....  
  def __storebc(self,bc,beam,buc,train):
    err=None
    if beam==1: beamac='A'
    if beam==2: beamac='C'
    if self.abcs.has_key(bc):
      beamac= beamac+self.abcs[bc]   # AC or CA for colliding
      if beamac=="AC" or beamac=="CA": beamac="B"
      else: 
        beamac= "X"+beamac   # error in input file
        #print "Error: buc:%d bc: %d beam:%d"%(buc,bc,beam)
        err= "#Error: buc:%d bc:%d beam:%d"%(buc,bc,beam)
    self.abcs[bc]= beamac
    if self.buckets.has_key(bc):
      #self.buckets[bc].buc= self.buckets[bc].buc+','+str(buc)
      self.buckets[bc].update(beam,buc)
    else:
      if beam==1:
        self.buckets[bc]= BcAttrs(buc,None, '', train)
      else:
        self.buckets[bc]= BcAttrs(None,buc, '', train)
    return err
  def read_sch(self, fn):
    """ rc: err message or ""
lines in fn:
Old scheme (.sch), before 19.9.2010: ring_1/2 in ll[3]
 inj  3  2  ring_1     2001     1250   4
            Y          Y         Y     Y             Y       Y
            beam       bucFirst buc    repetitions SPSspace PStrains
  0   1  2   3         4         5     6            7        8

New scheme (web txt), after 19.9.2010: ring_1/ring_2 in ll[2]
   1    1  ring_1      1     150      8         0       1 100
   2    1  ring_2      1     150      8         0       1 100
   3    2  ring_1   8941     150      8         0       1 100
           beam     bucFirst buc  repetitions  SPSspace PStrains
   0    1   2          3     4        5         6       7

Y -this column is used for ALICE collisions calculation
i.e.:
#... comment ignored
empty line: ignored
len(splitted line)<7: ignore

Goal: prepare:
1. abcs{}: abcs[bc] is one of: 'A' 'B' 'C' 'E'   bc:0..3563
2. buckets: buckets[bc].buc: n or n1,n2 LHC bucket number(s) 1..35641
                       .parasit: '*': ignored for triggering (.mask)
    """
    errs=""
    trainlenmax=0
    for line in string.split(fn,"\n"):
      if line == "": continue
      if line[0] == "\n": continue
      if line[0] == "#": continue
      ll= string.split(line)
      if len(ll)<7: continue
      if ll[3][:5]=='ring_':   
        ixring=3; ixbeam=3; ixbucFirst=4; ixbuc=5; ixrepetitions=6 
        ixSPSspace= 7; ixPStrains=8
      elif ll[2][:5]=='ring_':  #new
        ixring=2; ixbeam=2; ixbucFirst=3; ixbuc=4; ixrepetitions=5 
        ixSPSspace= 6; ixPStrains=7
      else:
        errs= errs+ "#Error: 'ring_1/2' expected in 3rd or 4th column\n"
        break
      ring= string.split(ll[ixring],"_")
      if len(ring)!=2:
        errmsg= "#Bad input line:%s"%line
        return errmsg
      PStrains= int(ll[ixPStrains]); SPSspace= int(ll[ixSPSspace])
      repetitions= int(ll[ixrepetitions])
      if repetitions>trainlenmax:
        trainlenmax= repetitions
      beam= int(ring[1]) ; bucFirst= int(ll[ixbucFirst])
      for ibcsps in range(PStrains):
        #print "ibcsps:",ibcsps, SPSspace
        for ibc in range(repetitions):   # ibc>0: multi bunch injections
          bc_spacing= int(ll[ixbuc])/25
          buc= (bucFirst + 10*ibcsps*SPSspace/25 + \
            ibc*(10*bc_spacing))%(ORBITL*10)
          bc=bu2bc(beam, buc)
          if ibc==0:
            trainStart= [repetitions, bc_spacing]
          else:
            trainStart= None
          #print "bcbuc:", beam, buc,"->", bc
          err=self.__storebc(bc, beam, buc,trainStart)
          if err: errs= errs+ err + "\n"
    print "Max. train:", trainlenmax
    return errs
  def read_dip(self, fn):
    """ rc: err message or ""
    """
    errs=""
    for line in string.split(fn,"\n"):
      if (line == ""): continue
      if (line[0] == "\n") or (line[0] == "#"): continue
      ll= string.split(line)
      if len(ll)<2: 
        errs= errs+ "Bad line:%s\n"%line
        continue
      beamac= ll[0] ; buc= int(ll[1])
      if (beamac=='A'): beam=1
      elif (beamac=='C'): beam=2
      else:
        errs= errs+ "Bad beam in line:%s\n"%line
        continue
      bc=bu2bc(beam, buc)
      err=self.__storebc(bc, beam, buc,None)
      if err: 
        errs= errs+ err
        continue
    return errs
  def do_sortedABCE(self):
    self.sortedABCE= self.abcs.keys(); self.sortedABCE.sort()   # A, C, B, E
  def locate_combs(self):
    self.ACsatellites= 0
    self.BvsACs= 0
    for ixabc in range(len(self.sortedABCE)):
      abc= self.sortedABCE[ixabc]
      if ixabc == len(self.sortedABCE)-1:
        abc_next= self.sortedABCE[0]
      else:
        abc_next= self.sortedABCE[ixabc+1]
      testcomb=False
      if abc==(ORBITL-1):
        if abc_next == 0: testcomb=True
      else:
        if abc_next == abc+1: testcomb=True
      if testcomb:
        # AC CA -normal     BA BC AB CB -B vs. AC satellite
        # AA CC -not comb
        if self.abcs[abc]=="E" or self.abcs[abc_next]=="E": continue
        if ((self.abcs[abc]=="A") and (self.abcs[abc_next]=="C")) or \
           ((self.abcs[abc]=="C") and (self.abcs[abc_next]=="A")):
          self.ACsatellites= self.ACsatellites+1
          self.buckets[abc].isincomb= True
          self.buckets[abc_next].isincomb= True
          continue
        # one of 2 neighbouring bunches (or both) is B
          self.BvsACs= self.BvsACs+1
          self.buckets[abc].isincomb= True
          self.buckets[abc_next].isincomb= True
    # if found, count also last one:
    if self.BvsACs>0: self.BvsACs= self.BvsACs+1
    if self.ACsatellites>0: self.ACsatellites= self.ACsatellites+1

def bu2bcstr(fn, fsname='not given', parasitic=None,\
  empty1=None,empty2=None, format="from sch"):
  ''' parasitic: 
N:        automatically mark parasitic bunches (closer than N)
None:     take all bunches
   0:     mark parasitic all A C bunches
"leave22" -leave only 2A and 2C trains   
"leave4B" -leave only 2*N A and 2*N C bunches (N: number of B)  
"leaveB" -leave only N/2 A and N/2 C bunches (N: number of B)  DEFAULT in 2011
"rnd4B"   -leave randomly (2*N of A + 2*N of C bunches)
"combAC"  -disable all B (head-head), show all neighbouring AC as B enabled
           disable all the others AC
  '''
  ##global abcs
  errs=''
  alice=fsname+'\n'; 
  lhcfs= LHCinfo(fsname)
  if format == "from sch":
    errmsg= lhcfs.read_sch(fn)
  if format == "from dip":
    errmsg= lhcfs.read_dip(fn)
  if errmsg!="":
    return alice+format+'\n'+errmsg
  abcs= lhcfs.abcs; buckets= lhcfs.buckets
  sortedB=[]
  for abc in abcs.keys():
    if abcs[abc]=="B": sortedB.append(abc)
  sortedB.sort()                # B-bunches only, sorted
  bgap=biggaps(lhcfs)
  #print "abcs,sortedB:",abcs,sortedB
  if len(sortedB)==1:   # just 1 colliding bunch
    bgap.update(sortedB[0], ORBITL)
  elif len(sortedB)==0:   # no colliding bunch
    bgap.update(3564, ORBITL)
  else: 
    bc_prev= sortedB[-1] 
    for bc in sortedB:   # find 2 longest gaps B - B
      dif= suborbit(bc, bc_prev)   # (dif-1): BCs between bc_prev and bc
      bgap.update(bc, dif)
      bc_prev= bc
  #bgap.prt()
  # we know largest gaps, so can create Empty BCs (-70bcs -TRD request):
  #print "BC:%d 1.gap before:%d"%(bgap.gaps[0][0], bgap.gaps[0][1])
  #print "BC:%d 2.gap before:%d"%(bgap.gaps[1][0], bgap.gaps[1][1])
  # if bigger gap is >200, put both E bunches into this gap
  if bgap.gaps[0][1] >200:
    e1, err= bgap.getempty(0)
    if err !="" : errs= errs + err
    e32= bgap.gaps[0][0]-e1+BCS_BEFORE
    #print "e32:",e32
    e2, err= bgap.getempty(0, e32)
    if err !="" : errs= errs + err
  else:
    e1, err= bgap.getempty(0)
    if err !="" : errs= errs + err
    e2, err= bgap.getempty(1)
    if err !="" : errs= errs + err
  Eerror=None
  if abcs.has_key(e1):
    errs=errs+"bc %d is already: %s. "%(e1,abcs[e1]) 
    Eerror=1
  if abcs.has_key(e2):
    errs=errs+"bc %d is already: %s. "%(e2,abcs[e2]) 
    Eerror=1
  if empty1:
    print "Empty1:%d forced to %d"%(e1,empty1)
    e1= empty1
  if empty2:
    print "Empty2:%d forced to %d"%(e2,empty2)
    e2= empty2
  if not Eerror:
    abcs[e1]="E"; abcs[e2]="E";
    buc1,buc2 = bc2buc('E', e1); buckets[e1]= BcAttrs(buc1,buc2, '', None)
    buc1,buc2 = bc2buc('E', e2); buckets[e2]= BcAttrs(buc1,buc2, '', None)
  lhcfs.do_sortedABCE()   # we have already E-bunches also
  lhcfs.locate_combs()
  #for bc in abcs.keys():   # not sorted!
  #
  nbb=0   # numb. of colliding bcs
  for bc in lhcfs.sortedABCE:
    btyp= abcs[bc]
    if btyp=='B': nbb= nbb+1
  if parasitic!=None:
    if parasitic==0:   # mark all A,C,parasitic
      for bc in lhcfs.sortedABCE:
        if abcs[bc]=='A' or abcs[bc]=='C':
          #print "ACpara:",bc
          buckets[bc].parasit='*'
    elif parasitic=="leave4B" or parasitic=="rnd4B" or parasitic=="leaveB" or parasitic=="combAC":
      #"rnd4B"   :  mark all A,C parasitic but 2*N+2*N randomly
      # "leave4B":  # mark all A,C parasitic but 
      #    leave 2N A-bunches and 2N C-bunmches N:number of B bunches
      #    in largest Bgap
      nbb=nbA=nbC=0;
      for bc in lhcfs.sortedABCE:
        btyp= abcs[bc]
        if btyp=='B': nbb= nbb+1
        if btyp=='A':
          buckets[bc].parasit='*' ; nbA=nbA+1
        if btyp=='C':
          buckets[bc].parasit='*' ; nbC= nbC+1
      if parasitic=="leaveB":
        nbb= (nbb+1)/2
      else:
        nbb= nbb*2
      Aenabled= 0; Cenabled= 0;
      #if nbb>0:   # leave all A,C if no B-bunches
      if parasitic=="combAC":
        # disable all B bunches:
        nbb=0
        for bc in lhcfs.sortedABCE:
          btyp= abcs[bc]
          if btyp=='B': 
            buckets[bc].parasit='*'
            nbb= nbb+1
        print "combAC required, %d main B-bunches disabled"%nbb
        # all AC CA neighbourgs will be enabled and shown as B later
        pass
      elif parasitic=="leave4B" or parasitic=="leaveB":
        bc= bgap.gaps[0][0]-4; nbc1= bgap.gaps[0][1]-4-40 ; nbc= nbc1
        #print "Bbunches:%d"%nbb
        while 1:
          if abcs.has_key(bc):
            btyp= abcs[bc]
            if btyp=='A' and Aenabled<nbb:
              buckets[bc].parasit=''
              Aenabled= Aenabled+1
            elif btyp=='C' and Cenabled<nbb:
              buckets[bc].parasit=''
              Cenabled= Cenabled+1
            #print "nbc:%d btyp:%s bc:%d"%(nbc,btyp,bc)
          if Aenabled==nbb and Cenabled==nbb: break
          bc= prevbc(bc); nbc= prevbc(nbc)
          if nbc==nbc1: break
      else:   #rnd4B
        import random
        #print "Bbunches:%d"%nbb
        rndattempts=0
        while 1:
          bc= random.randint(0, ORBITL-1)
          rndattempts= rndattempts+1
          if abcs.has_key(bc):
            btyp= abcs[bc]
            if btyp=='A' and Aenabled<nbb and buckets[bc].parasit=='*':
              buckets[bc].parasit=''
              Aenabled= Aenabled+1
            elif btyp=='C' and Cenabled<nbb and buckets[bc].parasit=='*':
              buckets[bc].parasit=''
              Cenabled= Cenabled+1
            #print "nbc:%d btyp:%s bc:%d"%(nbc,btyp,bc)
          if ((Aenabled==nbb) or (Aenabled==nbA)) and \
             ((Cenabled==nbb) or (Cenabled==nbC)): break
        print "rnd4B: attempts:%d A:%d C:%d"%(rndattempts,Aenabled,Cenabled)
    elif parasitic=="leave22":   # mark all A,C parasitic but 2+2 trains
      # mark all * and find B- areas
      Bareas= trainset('B'); Aareas= trainset('A'); Careas= trainset('C')
      for bc in lhcfs.sortedABCE:
        btyp= abcs[bc]
        if btyp=='A' or btyp=='C':
          #print "ACpara:",bc
          buckets[bc].parasit='*'
          if buckets[bc].train != None:
            Nbb= buckets[bc].train[0]
            if btyp=='A':
              Aareas.add([bc, bc+(Nbb-1)*buckets[bc].train[1], Nbb])
            else:
              Careas.add([bc, bc+(Nbb-1)*buckets[bc].train[1], Nbb])
        #elif abcs[bc]=='B' and buckets[bc].train != None:
        elif btyp=='B' and buckets[bc].train != None:
          # find B-areas:
          Nbb= buckets[bc].train[0]
          Bareas.add([bc, bc+(Nbb-1)*buckets[bc].train[1], Nbb])
      Bareas.prt() # Aareas.prt() Careas.prt()
      enabled_trains=0; enabling=0; aorc='A'
      for bc in lhcfs.sortedABCE:
        if abcs[bc]==aorc and buckets[bc].train != None:
          start=bc; end= bc+(buckets[bc].train[0]-1)*buckets[bc].train[1]
          if Bareas.isin(start,end): continue
          else:
            enabling= buckets[bc].train[0]
            bbnches= Bareas.getlen()
            if enabling<bbnches:
              #print "rather number of B-bunches=12 than train length %d."%(enabling)
              #enabling=12
              print "train length %d -skipped."%(enabling)
              continue
        if enabling>0:
          if abcs[bc]!=aorc: continue
          buckets[bc].parasit=''
          enabling= enabling-1
          if enabling==0:
            enabled_trains= enabled_trains+1
            if enabled_trains==2:
              aorc='C'
            if enabled_trains==4:
              break
          continue
      print "enabled trains:", enabled_trains 
    else:
      # parasitic bunch: if preceeded in <=5 bcs by another A,C, or B:
      bc_prev= lhcfs.sortedABCE[-1] 
      for bc in lhcfs.sortedABCE:
        dif= suborbit(bc, bc_prev)   # (dif-1): BCs between bc_prev and bc
        if (dif-1) <= parasitic:
          buckets[bc].parasit='*'
        bc_prev= bc
  # do not allow (any B,A,C, or E) in calib. bc. Here, only A,C E can
  # happen (B should not happen in LHC gap):
  print "bu2bcstr:buckets:",buckets.keys()
  if buckets.has_key(CALIBbc):
    buckets[CALIBbc].parasit='*'
    beamac= abcs[CALIBbc] 
    print "%c-bunch %d marked parasitic"%(beamac, CALIBbc)
    if beamac!='A':
      print
      print "ERROR: B-bunch (only A bunch allowed here) in LHC C-gap in calib. BC %d"%CALIBbc
      print
  bc_prev= lhcfs.sortedABCE[-1] 
  satelliteB=0
  for bc in lhcfs.sortedABCE:
    beamac= abcs[bc] 
    dif= suborbit(bc, bc_prev)
    spdphase= bc%4
    bucket= buckets[bc].bucstr()
    if buckets[bc].parasit=='*':
      parasit= ' *'
    else:
      parasit= '' #' N'
    #if buckets[bc].train:
    #  parasit= parasit+" Tr: %d*%d"%(buckets[bc].train[0],buckets[bc].train[1])
    #buccalc= bc2bucstr(beamac, bc)
    #print bc, beamac
    # .xls format:
    #alice= alice + "%d %s %d %d\n"%(bc, beamac, dif, spdphase)
    # .xls, but replace dif by bucket# format:
    #alice= alice + "%d %s %s %d%s %s\n"%(bc, beamac, bucket, spdphase, parasit, buccalc)
    if parasitic=="combAC":
      if buckets[bc].isincomb and beamac!='B':
        #all main B-bunches were marked parasitic already
        buc12= bc2bucstr('B',bc)
        alice= alice + "%d B %s %d\n"%(bc, buc12, spdphase)
        satelliteB= satelliteB+1
      else:
        alice= alice + "%d %s %s %d%s\n"%(bc, beamac, bucket, spdphase, parasit)
    else:
      alice= alice + "%d %s %s %d%s\n"%(bc, beamac, bucket, spdphase, parasit)
    #alice= alice + "%d %s %s\n"%(bc, beamac, bucket)
    bc_prev= bc
  if parasitic=="combAC":
    print "combAC: A/C->B: %d"%satelliteB
  return alice+errs

class FilScheme:
  def __init__(self, fsname="", fsxls=None):
    #self.mask=[]
    #for ix in range(len(ORBITL)):
    #  self.mask[]=1   # 'H' i.e. ignored
    self.mask={}   # contains only meaningful (BACE) BCs
    self.fsname= fsname
    self.bx= {'B':[], 'A':[], 'C':[], 'E':[], 'AorC':[]}
    # self.bx['B']: list of ints representing colliding BCs
    self.ignore={}; ixline=-1
    for line in string.split(fsxls,"\n"):
      ixline= ixline+1
      if ixline==0:
        if fsname=="":
          self.fsname= line
        print "fsname:",self.fsname
      if line == "": continue
      if line[0] == "\n": continue
      if line[0] == "#": continue
      #uuprint "btdp:", btdp,":"
      btdp= string.split(line)   # BX Type[BACE] Delta SPD_phase *
      # if * present in 5th column, ignore for A/C/AorC/B but take into account for D
      if btdp==None: continue
      if len(btdp)<2: continue
      if btdp[0] == 'BX': continue
      bxn= int(btdp[0])   # 0..ORBITL-1
      bace= btdp[1]
      if len(btdp)==2: 
        buckets= bc2bucstr(bace, bxn)
        spdph= bxn%4
        print "%d %s %s %d"%(bxn, bace, buckets, spdph)
      if len(btdp)>=5:
        if btdp[4]=='*':   # ignore this BC for A/C/B/AorC but not for D
          self.ignore[bxn]=1
      #von self.bx[bace].append(bxn)
      self.insert(bace, bxn)   # isert bxn before bace
      self.mask[bxn]= bace
      if (bace=='A') or (bace=='C'):
        #self.bx['AorC'].append(bxn)   # A + C
        self.insert('AorC', bxn)
  def insert(self, bace, bxn):
    for ix in range(len(self.bx[bace])):
      if bxn<self.bx[bace][ix]:
        self.bx[bace].insert(ix, bxn)
        return
    self.bx[bace].append(bxn)
  def eN(self, bx):
    """ bx: SORTED list of ints representing meaningfull (significant) bunches
    Notes:
    - self.ignore is taken into account
    rc: "3563H1L"
"""
    lastbx=-1; msk=""; lrep=0
    #print "eN:bx:",bx
    for n in range(len(bx)):
      bxn= bx[n]
      if self.ignore.has_key(bxn): continue
      rep= bxn-lastbx-1
      if rep>0:
        if lrep>0:
          msk= msk + "%dL" % lrep; lrep=0
        msk= msk + "%dH1L" % (rep)
      else:
        lrep= lrep+1
        #msk= msk + "1L"
      lastbx= bxn
    if lrep>0:
      msk= msk + "%dL" % lrep; lrep=0
    rep=ORBITL-lastbx-1
    if rep>0:
      msk= msk + "%dH" % (ORBITL-lastbx-1)
    return msk
  def H19around(self,bx,dis_after):
    ixa=[]   # bcs around bx
    ixbc= prevbc(bx,DIS_BEFORE)
    for bc in range(dis_after+DIS_BEFORE+1):
      ixa.append(ixbc)
      ixbc= nextbc(ixbc)
    return ixa 
  def H2(self,bx,plus1):
    ixa=[]   # bcs: bx, bx+1
    ixbc= bx
    for bc in range(0,plus1):
      ixa.append(ixbc)
      ixbc= nextbc(ixbc)
    return ixa 
  def cosmicAllowed(self):
    """ 
    """
    maar=[]   # 0..3563 L-ok(triggering) H: disabled
    for ix in range(ORBITL):
      maar.append('L')
    for bx in self.mask.keys():   # not sorted!
      bt= self.mask[bx]
      if bt=='E': continue   # ignore E-bunches
      # B C or A
      if bt!= 'A' and bt !='C' and bt !='B':
        print "Internal error BC type in %d is "%bx,bt
        continue
      if bt== 'B':
        dis_after= DIS_AFTER_B
      else:
        dis_after= DIS_AFTER_AC
      ixa= self.H19around(bx, dis_after)
      #print ixa
      for bc19 in ixa:
        maar[bc19]= 'H'
    maar[CALIBbc]= 'H'   # bug in ctp fy (cal. cannot be together with physics)
    cmask=""; rep=0; lh= maar[0];
    for ix in range(ORBITL):
      if maar[ix]==lh:
        rep= rep +1
      else:
        cmask= cmask + "%d%c"%(rep, lh)
        #print "    mask:",cmask
        rep=1; lh= maar[ix]
    cmask= cmask + "%d%c"%(rep, lh)
    #print "lastmask:",cmask
    return cmask
  def reversed(self,plus1):
    """rc: reversed mask: all but 'B' bunches 
    plus1: None: disable just B
           1:    disable B and the bunch after
    """
    maar=[]   # 0..3563 L-ok(triggering) H: disabled
    for ix in range(ORBITL):
      maar.append('L')
    for bx in self.mask.keys():   # not sorted!
      bt= self.mask[bx]
      if bt=='E': continue   # ignore E-bunches
      # B C or A
      if bt!= 'A' and bt !='C' and bt !='B':
        print "Internal error BC type in %d is "%bx,bt
        continue
      if bt== 'B':  # disable B
        maar[bx]='H'
        if plus1!=None:
          ixa= self.H2(bx, plus1)  # disable also following bunch
          #print "reversed ixa:",ixa
          for bc2 in ixa:
            maar[bc2]= 'H'
    maar[CALIBbc]= 'H'   # bug in ctp fy (cal. cannot be together with physics)
    cmask=""; rep=0; lh= maar[0];
    for ix in range(ORBITL):
      if maar[ix]==lh:
        rep= rep +1
      else:
        cmask= cmask + "%d%c"%(rep, lh)
        #print "    mask:",cmask
        rep=1; lh= maar[ix]
    cmask= cmask + "%d%c"%(rep, lh)
    #print "reversed:",cmask
    return cmask
  def cosmicAllowed_old(self):
    """ bcmD mask: cosmic. Disabled in all possible bunches -3 +16 BCs
    """
    cmask="" ; bxprev=bxprev1=0; 
    firstoff=3   # SPD signals are 4BCs wide
    ordn=0
    #for bx in self.mask.keys():   # not sorted!
    keys= self.mask.keys() ; keys.sort()
    #print keys
    for bx in keys:   # now sorted!
      #if self.mask[bx]=='E': continue   # ignore E-bunches
      #cmask= cmask+"%dL16H"%(bx-bxprev)
      #bxprev= bx+16
      # enabled: length of area before BC bx:
      if bx<bxprev: 
        #print "bxA:%d bxprev:%d"%(bx, bxprev)
        bxprev= bx+16
        continue   #bx anyhow disabled
      if (bxprev-bxprev1)>0:
        msk= "%dH"%(bxprev-bxprev1)
        cmask= cmask+msk
        print " %d H:  "%bx,msk
      enabled=(bx-firstoff)-bxprev
      #print "bxB:%d bxprev:%d enabled:%d"%(bx, bxprev,enabled)
      if enabled<=0:
        msk= "%dH"%(16+firstoff+enabled)
        cmask= cmask+msk
        #bxprev= bxprev + 16+firstoff + enabled:
        bxprev= bx-firstoff+16 + firstoff
      else:
        msk= "%dL%dH"%(enabled, 16+firstoff)
        cmask= cmask+msk
        #bxprev= bxprev + enabled + 16+firstoff:
        bxprev= bx+16
      bxprev1= bxprev
      ordn=ordn+1
      print ordn,"D bx:%d %d bxprev(next allowed):%d msk:%s"%(bx,enabled,bxprev,msk)
    if (bxprev-bxprev1)>0:
      msk= "%dH"%(bxprev-bxprev1)
      cmask= cmask+msk
      print "lastH:  ",msk
    cmask= cmask+"%dL"%(ORBITL-bxprev)
    return cmask
  def print1(self,bace):
    ostr="#%s"%bace
    for n in range(len(self.bx[bace])):
      bxn= self.bx[bace][n]
      if self.ignore.has_key(bxn): continue
      ostr= ostr + " " + str(bxn)
    return ostr
  def print2(self,name, arr):
    """ analogy of print1, using explicit buffer
"""
    ostr="#%s"%name
    for n in range(len(arr)):
      bxn= arr[n]
      if self.ignore.has_key(bxn): continue
      ostr= ostr + " " + str(bxn)
    return ostr
  def getMasks(self):
    om= "# %s"%self.fsname
    om= om+"\n" + self.print1('E')
    if len(self.bx['E']) <2:
      om= om+"\n#LESS THAN 2 EMPTY BUNCHES!"
      om= om+"\n#LESS THAN 2 EMPTY BUNCHES!"
    om= om+"\n" + "bcmEMPTY" +" "+ self.eN(self.bx['E'])
    om= om+"\n" + self.print1('B')
    om= om+"\n" + "bcmB" +" "+ self.eN(self.bx['B'])
    om= om+"\n" + self.print1('A')
    om= om+"\n" + "bcmA" +" "+ self.eN(self.bx['A'])
    om= om+"\n" + self.print1('C')
    om= om+"\n" + "bcmC" +" "+ self.eN(self.bx['C'])
    om= om+"\n" + "bcmAC" +" "+ self.eN(self.bx['AorC'])
    # prepare AC+E: (both AorC, E are sorted!)
    ace=[] ; ecopy=[]
    for ix in range(len(self.bx["E"])):
      ecopy.append(self.bx['E'][ix])
    #print "ecopy:",ecopy
    for ix in range(len(self.bx["AorC"])):
      ac= self.bx["AorC"][ix]
      for ixe in range(len(ecopy)):
        e= ecopy[ixe]
        if e==None: continue
        if e>ac:
          ace.append(ac) ; ac=None
          break   # take next ac
        elif e<ac:
          ace.append(e) ; ecopy[ixe]= None
          # continue to check next e
        else:
          ace.append(e) ; ecopy[ixe]= None ; ac=None
          break   # take next ac
      if ac!=None:
        ace.append(ac) ; ac=None
    #print "ace:",ace      
    om= om+"\n" + self.print2('ACE', ace)
    om= om+"\n" + "bcmACE" +" "+ self.eN(ace)
    # not working with trains...
    om= om+"\n" + "bcmR" +" "+ self.reversed(None)   #None or 1
    om= om+"\n" + "#bcmR2" +" "+ self.reversed(2)   #B +1
    #om= om+"\n" + "bcmD" +" "+ self.cosmicAllowed()
    # for technical runs -using this mask to disable triggering in CALib bc
    #om= om+"\n" + "bcmD" +" "+ "3009L1H554L"   # see CALIBbc
    return om

def main():
  #1000ns_50b_35_14_35
  d=""" inj  1      1     ring_1           1        1000       4
 inj  2      1     ring_2           1        1000       4
 inj  3      2     ring_1        1601        1000       3
 inj  4      2     ring_2        1601        1000       3
 inj  5      3     ring_1        2801        1000       4
 inj  6      3     ring_2        2801        1000       4
 inj  7      4     ring_1        4401        1000       3
 inj  8      4     ring_2        4401        1000       3
 inj  9      5     ring_1        8941        1000       4
 inj 10      5     ring_2        6601          25       1
 inj 11      6     ring_1       10541        1000       3
 inj 12      6     ring_2        8911        1000       4
 inj 13      7     ring_1       11741        1000       4
 inj 14      7     ring_2       10511        1000       3
 inj 15      8     ring_1       13341        1000       3
 inj 16      8     ring_2       11741        1000       4
 inj 17      9     ring_1       17301          25       1
 inj 18      9     ring_2       13341        1000       3
 inj 19     10     ring_1       17851        1000       4
 inj 20     10     ring_2       14541        1000       4
 inj 21     11     ring_1       19451        1000       3
 inj 22     11     ring_2       16141        1000       3
 inj 23     12     ring_1       20681        1000       4
 inj 24     12     ring_2       17851        1000       4
 inj 25     13     ring_1       22281        1000       3
 inj 26     13     ring_2       19451        1000       3
 inj 27     14     ring_1       23481        1000       4
 inj 28     14     ring_2       20681        1000       4
 inj 29     15     ring_1       25081        1000       3
 inj 30     15     ring_2       22281        1000       3
"""
  e="""
   1    1  ring_1      1     150         4               1500           2             99
   2    1  ring_2      1     150         4               1500           2             99
   3    2  ring_1   2001     150         4               1500           4             100
   4    2  ring_2   1941     150         4               1500           4             100
   5    3  ring_1   4401     150         4               1500           4             100
   6    3  ring_2   4341     150         4               1500           4             100
   7    4  ring_1   8941     150         4               1500           2             99
   8    4  ring_2   8911     150         4               1500           2             99
   9    5  ring_1  10881     150         4               1500           4             100
  10    5  ring_2  10881     150         4               1500           4             100
  11    6  ring_1  13281     150         4               1500           4             100
  12    6  ring_2  13281     150         4               1500           4             100
  13    7  ring_1  17851     150         4               1500           2             99
  14    7  ring_2  17851     150         4               1500           2             99
  15    8  ring_2  19761     150         4               1500           4             100
  16    8  ring_1  19761     150         4               1500           4             100
  17    9  ring_2  22161     150         4               1500           4             100
  18    9  ring_1  22161     150         4               1500           4             100
  19   10  ring_1  28701     150         4               1500           4             100
  20   10  ring_2  28701     150         4               1500           4             100
  21   11  ring_1  31101     150         4               1500           4             100
  22   11  ring_2  31101     150         4               1500           4             100
"""

  # LHC -> alice.xls
  if len(sys.argv)>=2:
    schname= sys.argv[1]
    if schname=='bc2buc':
     acbe= sys.argv[2]
     bc= int(sys.argv[3])
     spdphase= bc%4
     print bc2bucstr(acbe,bc),spdphase
     return
    else:
      namsuf= string.split(schname,'.')
      if len(namsuf)==2:
        if namsuf[1]=="alice":
          alicef="from alice"
          schname= namsuf[0]
        elif namsuf[1]=="dip":
          alicef="from dip"
          schname= namsuf[0]
          lsf= open(schname+".dip"); ee=lsf.read(); lsf.close;
        elif namsuf[1]=="sch":
          alicef="from sch"
          schname= namsuf[0]
          lsf= open(schname+".sch"); ee=lsf.read(); lsf.close;
        else:
          print "Probably bad filename (.alice expected)"
          return
      else:
        alicef= "from sch"
        lsf= open(schname+".sch"); ee=lsf.read(); lsf.close;
      empty1=None; empty2=None
      if len(sys.argv)>2:
        empty1= int(sys.argv[2])
      if len(sys.argv)>3:
        empty2= int(sys.argv[3])
  else:
    schname="notgiven"
    print """
Usage:
1.
lhc2ctp.py lhc_fs.sch [empty1] [empty2]
 e.g.: ~/lhc2ctp.py 150ns_200b_186_8_186_8+8bpi17inj
where:
lhc_fs: LHC filling scheme name
empty1/2: BCs of E-bunches. If given, E-bunches will be placed in these
positions

prepares 2 files, taking file lhc_fs.sch, in working directory:
lhc_fs.alice (ALICE collisions schedule)
   -to be copied to $dbctp/fs:
   alidcscom188:/data/dl/root/usr/local/trigger/v/vme/CFG/ctp/DB/fs
lhc_fs.mask (VALID.BCMASK instance for ACT)

2.
lhc2ctp.py lhc_fs.dip [empty1] [empty2]
prepares 2 files (see case 1. above)

3.
lhc2ctp.py lhc_fs.alice
prepares lhc_fs.mask (VALID.BCMASK for ACT) from lhc_fs.alice file
in working directory

4.
lhc2ctp.py bc2buc ABCE bc
shows the bucket(s) for 'ABC or E-bunch' bc
"""
    return
  if (alicef=="from sch") or (alicef=="from dip"):
    alice= bu2bcstr(ee, schname, empty1=empty1,empty2=empty2, format=alicef, parasitic=None)
    lsf= open(schname+".alice","w"); lsf.write(alice); lsf.close;
    print "Alice collisions schedule written in file:\n",schname+".alice"
  #return
  # take it directly from file:
  #lsf= open("/home/aj/act/domsk/125ns_48b_36_16_36Fromxls.alice"); 
  lsf= open(schname+".alice","r"); alice=lsf.read(); lsf.close;
  # VALID.BCMASKS:
  mask= FilScheme("", alice); mask= mask.getMasks()
  #print "ignored bcs:",str(mask.ignore)
  lsf= open(schname+".mask","w"); lsf.write(mask); lsf.close;
  print "Alice masks written in file:\n",schname+".mask"
  #print "VALID.BCMASKS:\n",mask.getMasks()
if __name__ == "__main__":
    main()

