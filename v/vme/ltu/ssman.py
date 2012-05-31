#!/usr/bin/python
# history:
# 22.5.2004 'unexpected strob' warning added
# export PYTHONPATH=~/FTP/psyco-1.2 (improves to 1:45->37 sec)
# 1.6. ACTIVESa - no improvement 37-> 38 with psyco
# Jun/2004 -this way abandoned, -much more slower than C version, which
#           is in VMECFDIR/SSMANA directory.
import sys,os
class tBC:
  """Pulse. Duration: 1BC
  """
  def __init__(self, ssmbit, name):
    self.name=name
    self.ssmbit=ssmbit       # 0-17 (bit occupied in snapshot memory)
    self.bc=None   # None: item is empty (not active)
  def update(self,bc, wrd):
    bit= (wrd>>self.ssmbit)&0x1
    if bit==1:
      self.bc=bc
      rctxt= "%7d: %s"%(self.bc,self.name)
      #print self.name
      self.clear()
    else: rctxt=""
    return rctxt
  def clear(self):
    self.bc=None
  def logit(self, bc, text):
    global ssm
    txt= "%7d: %s"%(bc, text)
    ssm.lstf.write(txt+"\n")
  def flush(self):
    if self.bc!=None:
      return "%7d: %s"%(self.bc,self.name)
    else:
      return ""

class tDATA(tBC):
  """Data. Duration: fixed by parameter length
  There are 2 types of DATA signals:
  data              -strobed by '1' in bit strobbit
  self-strobed data -data follows just after the strobe (1st 1)
                     strobbit==bit for self-strobed date
  ERRORS checked:
  - 'unexpected strob' -during active data, another strob found
  - 'rubbish data'     -data found (0->1) without a preceding strob
  """
  def __init__(self, bit, name, length, strobbit):
    tBC.__init__(self,bit, name)
    self.length= length
    self.strobbit= strobbit
    self.data=[]   # data bits
  def update(self, bc, wrd):
    """returns ( and tBC,tSIGNAL too):
    empty string if nothing to be logged
    bc: name signal-dependent info
    """
    global ACTIVES,ACTIVESa
    rettxt= ""
    if self.bc==None:
      #DATA not active
      if ((wrd>>self.strobbit)&1) == 1:
        # strob found, DATA just activated
        ACTIVES=ACTIVES+1
        ##ACTIVESa[self.ssmbit]=1
        if self.strobbit==self.ssmbit:
          # self-strobed data
          self.bc= bc+1 # data starts with the next bit
          return ""
        else:
          self.bc= bc   # data starts with strob bit
      else: 
        if (wrd>>self.ssmbit)&1 == 1:   # rubbish
          self.logit(bc, "WARNING rubbish data bit %s in word:%x"%(self.name,wrd))
        return rettxt
    else:
      #DATA already active
      if self.strobbit!=self.ssmbit:
        # check only for NON self-strobed data
        if ((wrd>>self.strobbit)&1) == 1:
          self.logit(bc,"WARNING unexpected strob find, while "+self.name+" active")
    databit= (wrd>>self.ssmbit)&0x1
    return self.appendbit(databit)
  def appendbit(self, bit):
    global ACTIVES,ACTIVESa
    #if self.name=="TTCLS":
    #  print "ttclsA:%x"%(bit),len(self.data),self.length
    self.data.append(bit)
    if len(self.data)>=self.length:
      nm=self.name
      if self.ssmbit==6:   #L2DATA or L2DATR
        if self.data[0]==1:
          nm='L2DATR'   # L2r data
      rettxt= "%7d: %s:%s"%(self.bc, nm, self.getdata())
      self.data=[]
      self.clear()
      ACTIVES=ACTIVES-1
      ##ACTIVESa[self.ssmbit]=0
    else:
      rettxt=""
    return rettxt
  def getdata(self):
    ibit=0; outstr=""; self.words8=[]
    while 1:
      w8=0
      for i1 in range(8):
        #w8= w8 | (self.data[ibit]<<i1)     # 7..0 15..8 ...
        w8= w8 | (self.data[ibit]<<(7-i1))  # 0..7 8..15 ...
        ibit=ibit+1
        if ibit>= self.length: break
      self.words8.append(w8)   # byte-form, remember for signal dep. data
      von="""
      if outstr!="":
        outstr= outstr+"%2.2x"%(w8)
      else:
        outstr= "%2.2x"%(w8)
      """
      if ibit>= len(self.data): break
    #add signal dependent info:
    #outstr= outstr+" "
    if self.ssmbit==13:
      depi= self.getttcl(w8)
    elif self.ssmbit==14:
      depi= self.getttcm(w8)
    elif self.ssmbit==4:
      depi=self.getl1d()
    elif self.ssmbit==6:
      depi=self.getl2d()
    else:
      depi='ERROR: unknown data'
    return depi
  def getttcl(self,w8):
    return "%2.2x"%(w8)
  def getttcm(self,w8):
    symbhead= ["ZERO", "L1h", "L1d", "L2h", "L2d", "L2r", "RoIh", "RoId"]
    depi= "%2.2x"%(w8)
    header= (w8&0xf0)>>4
    depi= depi +" "+ symbhead[header]
    if header==1:   # L1h
      if w8&0x08:
        depi= depi+" Spare"
      if w8&0x04:
        depi= depi+" CIT"
      if w8&0x03 != 0:
        depi= depi+" RoC43=0x%x"%(w8&0x3)
    if header==3 or header==5:   # L2h or L2r
      depi= depi+" BCID129=0x%x"%(w8&0xf)
    return depi
  def getl1d(self):
    depi= self.bighexla(1-1,58-1)
    depi= depi+" "+self.bighexra(9-1,58-1)
    if self.data[1-1]: depi= depi+" Spare"
    if self.data[2-1]: depi= depi+" CIT"
    roc= (self.words8[0]&0x36)>>2
    if roc!=0: depi= depi+" ROC=0x%x"%(roc)
    if self.data[7-1]: depi= depi+" ESR"
    if self.data[8-1]: depi= depi+" L1SwC"
    return depi
  def getl2d(self):
    depi= self.bighexla(2-1,97-1)
    depi= depi + " "+self.bighexra(48-1,97-1) + " " +\
      self.bighexra(42-1,47-1)   #L2class + L2Cluster
    if self.data[38-1]: depi= depi+" Spare38"
    if self.data[39-1]: depi= depi+" Spare39"
    if self.data[40-1]: depi= depi+" CIT"
    if self.data[41-1]: depi= depi+" L2SwC"
    return depi
  def bighexra(self,mi,ma):   # right aligned
    """ get hexa mi-ma (mi,ma included)
    """
    l1c=0L
    for i in range(mi,ma+1):
      l1c= l1c<<1
      l1c= l1c + self.data[i]
    return hex(l1c)[:-1]
  def bighexla(self,mi,ma):   # left aligned (as sent in triples)
    w12=0; outstr=""; i1=0
    for ibit in range(mi,ma+1):
      #w8= w8 | (self.data[ibit]<<i1)
      w12= w12<<1
      w12= w12 | self.data[ibit]  # 0..7 8..15 ...
      i1=i1+1
      if i1>=12: 
        if outstr!="":
          outstr= outstr+".%3.3x"%(w12)
        else:
          outstr= "%3.3x"%(w12)
        #print "ibit:",ibit,outstr
        i1=0; w12=0
    if i1>0:   # flush it
      outstr= outstr+".%3.3x"%(w12<<(12-i1))
    return outstr
 
class tSIGNAL(tBC):
  """Signal. Duration: >1BC. 
  End of signal: falling edge (if length==0) or
  # of BCs is counted
  """
  def __init__(self, bit, name):
    tBC.__init__(self,bit, name)
    self.lastbc=None
    #self.length= length
  def update(self, bc, wrd):
    global ACTIVES,ACTIVESa
    #print 'isbsselfbc: -YES',self.bc
    rettxt=""
    bit= (wrd>>self.ssmbit)&0x1
    if bit==1:       # the signal start
      self.lastbc=bc
      #if self.name=="VMEM":
      #  print "vm:%x"%(wrd)
      if self.bc==None:
        ACTIVES=ACTIVES+1
        ##ACTIVESa[self.ssmbit]=1
        self.bc=bc   # remember start of signal
    else:
      if self.bc!=None:
        rettxt= "%7d: %s/%d"%(self.bc,self.name,(self.lastbc-self.bc+1))
        self.clear()
        ACTIVES=ACTIVES-1
        ##ACTIVESa[self.ssmbit]=0
    return rettxt

class tSSM:
  bithx=[1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536,131072]
  def __init__(self, fname):
    self.cols=[]   # now keeping the order of bits
    self.cols.append(tSIGNAL( 0,"ORBIT"))
    self.cols.append(tSIGNAL( 1,"PP"))
    self.cols.append(tBC(     2,"L0"))
    self.cols.append(tBC(     3,"L1S"))
    self.cols.append(tDATA(   4,"L1DATA",58, 3))
    self.cols.append(tBC(     5,"L2S"))
    self.cols.append(tDATA(   6,"L2DATA",97, 5))
    self.cols.append(tSIGNAL( 7,"SBUSY"))
    self.cols.append(tSIGNAL( 8,"ALLBUSY"))
    self.cols.append(tSIGNAL( 9,"L1NF"))
    self.cols.append(tSIGNAL(10,"L2NF"))
    self.cols.append(tSIGNAL(11,"LBHALT"))
    self.cols.append(tSIGNAL(12,"VMEM"))
    self.cols.append(tDATA(  13,"TTCLS",8, 13))
    self.cols.append(tDATA(  14,"TTCMS",8, 14))
    self.cols.append(tSIGNAL(15,"VMES"))
    self.cols.append(tBC(    16,"ALLSTART"))
    self.cols.append(tBC(    17,"ANYERR"))
    self.BCSIGBITS= [0,1,2,3,5,7,8,9,10,11,12,15,16,17]
    self.DATABITS= [4,6,14,13]
    #self.SIGDATABITS= [0,1,4,6,7,8,9,10,11,12,13,14,15]
    #self.BCBITS=[2,3,5,16,17]
    self.fname=fname
    self.f=open(fname,'rb')
  def lstdump(self, bcfrom=0, n=1024*1024,lstname=None):
    import struct
    global ACTIVES,ACTIVESa
    ACTIVESa=[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]; ACTIVES=emptywords=0
    self.lstf=None
    if lstname:
      self.lstf=open(lstname,"w")
    #self.f.seek(bcfrom*4,0)
    # followinf lines didn't improve speed too much:
    #  1:30m -conversion in batch (1048576i at once),
    #  1:44m -conversion word by word
    wmega= self.f.read()
    wimega=struct.unpack('1048576i',wmega)
    for bc in range(bcfrom,bcfrom+n):
      ##w= self.f.read(4)
      ##if not w:
      w=wimega[bc]
      if w==None:
        if n!= 0:
          print "unexpected EOF:",self.fname
        break
      ##w=struct.unpack('i',w)[0]
      #print bc,':',"%x"% w,':',type(w)
      # if no signal active, and w==0, don't look for signal:
      # -improvement!: 1:44m -> 0:14m
      # - and after 'struct in batch' it is below 5secs.
      if w==0 and ACTIVES==0: 
        #emptywords=emptywords+1
        continue
      ##for ssmbit in self.BCSIGBITS+self.DATABITS:
      for ssmbit in self.DATABITS+self.BCSIGBITS:
          line=""
        ##if ACTIVESa[ssmbit] or w:   # active, or something not processed yet
          line= self.cols[ssmbit].update(bc, w)
          if line!= "":
            #print line
            self.lstf.write(line+"\n")
          ##hx= tSSM.bithx[ssmbit]
          ##w= w & (~hx)
    self.flush(bcfrom+n)  
    #print "empty words:",emptywords
  def flush(self,bc):
    nf=""
    for ssmbit in self.DATABITS:
      nf= nf+' '+self.cols[ssmbit].flush()
    line= "%7d: not finished:%s"%(bc,nf)
    self.lstf.write(line+"\n")
    #print line
  def close(self):
    self.f.close()
    self.lstf.close()

def makelst(fn=None):
  global ssm
  log=''
  if fn==None:
    fn= os.path.join(os.environ['VMEWORKDIR'],"WORK","SSM.dump")
    lstfn= os.path.join(os.environ['VMEWORKDIR'],"WORK","ssm.lst")
    #print "Using def. file:",fn
  else:
    lstfn=fn+'.lst'
  log= log+fn+" -> "+lstfn +"\n"
  ssm= tSSM(fn)
  #print "making list to: ",lstfn
  log= log+ "making list to: "+lstfn+"\n"
  #ssm.lstdump(0,10000,lstfn)
  ssm.lstdump(0,1024*1024,lstfn)
  ssm.close()
  #print "       sorting: ",lstfn
  #log= log+ "       sorting: "+lstfn+"\n"
  #os.system("sort -s -n -k 1,7 -o "+lstfn+" "+lstfn)
  return log

def main():
  #import psyco
  #psyco.full()
  if len(sys.argv) < 2:
    fn=None
    #print "usage: ssman ssm.dump ->creates ssm.lst file in current directory"
  else:
    fn=sys.argv[1]
  print makelst(fn)

if __name__ == "__main__":
    main()

