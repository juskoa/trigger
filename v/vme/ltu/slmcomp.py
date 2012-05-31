#!/usr/bin/python
"""slm backward compiler
Usage: 
slmcomp.py file.seq
  -> converts 'binary' form to human readable form
Input file (file.seq):
- first 2 lines are ignored (name and # of valid sequences in the file)
- 3. line contains 7 bits for ERROR_SELECTOR word
  From left to right, i.e. 1. char in line is bit0, last char is bit6:
  Order of the bits in file:
  PP L0 L1 L1M L1&L1M L2aM L2rWord
  Order of the bits in ERROR_STATUS word is reversed!
- 1 sequence is encoded in 8 lines
  1 word of the sequence is encoded in 1 line, which
  consists of '0'/'1' chars
    1. char in the line corresponds to bit15, 
    the last one corresponds to bit0
1 sequence:
word bits
0    15     spare1
     14     ClT
     13..10 roc
      9     ESR
      8     L1SwC
      7.. 6 L1Class[50..49]
      5     ErrProne
      4     Last
      3     Restart
      2..0  see Seq.SNAME
1    15.. 0 L1Class[48..33]
2    15.. 0 L1Class[32..17]
3    15.. 0 L1Class[16..1]
4    15     not used (set to 0 in SLM)
     14     spare2
     13     ESR
     12     ClT
     11     L2SwC
     10.. 5 L2Cluster[6..1]
      4.. 0 L2Class[50..46]
5    15.. 0 L2Class[45..30]
6    15.. 0 L2Class[29..14]
7    15.. 3 L2Class[13..1]
      2.. 0 not used (set to 0 in SLM)

11.5.2006 spare3 4.13 is now ESR (i.e. should == 0.9)
"""
#import os.path, os, string, sys, glob
import os.path, os, string
class Seq:
  SNAME=['ZERO', 'L0  ', 'L2A ', 'L2R ', 'CPP ', 'CL0 ', 'CL2A', 'CL2R']
  def __init__(self):
    self.s=[0,0,0,0,0,0,0,0]   # 8 words of 1 trg. sequence
    self.l1c=[]
    self.l2c=[]
    for i in range(51): 
      self.l1c.append(0)
      self.l2c.append(0)
    self.errs=self.warns=''
  def SCode(self):
    return self.s[0]&0x7
  def RoC(self):
    #print "RoC:", (self.s[0]&0x3c00)>>10
    return (self.s[0]&0x3c00)>>10
  def ESR(self):
    return (self.s[0]&0x200)>>9
  def L1SwC(self):
    return (self.s[0]&0x100)>>8
  def L2SwC(self):
    return (self.s[4]&0x800)>>10
  def ClT1(self):
    return (self.s[0]&0x4000)>>14
  def ClT2(self):
    return (self.s[4]&0x1000)>>12
  def spare1(self):
    return (self.s[0]&0x8000)>>15
  def spare2(self):
    return (self.s[4]&0x4000)>>14 #13
  def spare3(self):
    return (self.s[4]&0x2000)>>13 #12
  def ErrorProne(self):
    return (self.s[0]&0x20)>>5
  def Last(self):
    return (self.s[0]&0x10)>>4
  def Restart(self):
    return (self.s[0]&0x8)>>3
  def warn(self, m):
    self.warns=self.warns+', '+m
  def word(self,ix,line):
    """
    ix: 0,1,...,7 -sequence word index
    ret: 0-OK, 1-error
    """
    bit=0   # bit number in the word (0..15)
    for i in range(15,-1,-1):
      if line[i]=='1':
        self.s[ix]= self.s[ix] | 1<<bit
        if ix==0:
          if bit==7: self.l1c[50]=1
          if bit==6: self.l1c[49]=1
        elif ix==1:
          self.l1c[33+bit]=1
        elif ix==2:
          self.l1c[17+bit]=1
        elif ix==3:
          self.l1c[1+bit]=1
        elif ix==4:
          if bit<=4: self.l2c[46+bit]=1
        elif ix==5:
          self.l2c[30+bit]=1
        elif ix==6:
          self.l2c[14+bit]=1
        elif ix==7:
          if bit>=3: self.l2c[bit-2]=1
      elif line[i]!='0':
        return 1
      bit= bit+1
    #print "word:%d:%x"%(ix,self.s[ix]) 
    #if ix==0:
    #  self.SCode= bit & 0x7
    #  self.Restart= bit & 0x8
    #elif ix==1:
    return 0
  #def longhex(self,*args):
  def longhex(self,l12c):
    """
    l12c -list. l12c[0] -not used, l12c[1..50] - 0 or 1
    """
    together=''
    a= (l12c[50]<<1) | l12c[49] 
    together= together+"%.1x"%(a)
    for i in range(48,0,-1):
      if i%4==0: 
        a= (l12c[i]<<3) | (l12c[i-1]<<2) | \
          (l12c[i-2]<<1) | l12c[i-3] 
        together= together+"%.1x"%(a)
      #together=together+a
      #print "longhex:",a
    # following is OK with python 2.2.2:
    #together= together.lstrip('0')
    for i in range(len(together)):
      if together[i] != '0': break
    together= together[i:]
    if together=='': together='0'
    #print "longhex:",together
    return '0x'+together
  def printcheck(self):
    seqok=1
    #print Seq.SNAME[self.s[0]&0x7],':',self.s[0]
    #print 'l1c:',self.l1c 
    #print 'l2c:',self.l2c 
    l1class= self.longhex(self.l1c)
    l2class= self.longhex(self.l2c)
    l2cluster= "0x%x"%((self.s[4]&0x7e0)>>5)
    flags=''
    roc= self.RoC()
    if roc:
      flags= flags+'roc='+str(roc)+' '
    if self.Restart():
      flags= flags+ 'Restart '
    if self.Last():
      flags= flags+ 'Last '
    if self.ErrorProne():
      flags= flags+ 'ErrProne '
    if self.ESR():
      flags= flags+ 'ESR '
    if self.L1SwC():
      flags= flags+ 'L1SwC '
    if self.L2SwC():
      flags= flags+ 'L2SwC '
      if self.L1SwC()==0:
        self.warn("L1SwC not set with L2SwC")
    if self.ClT1() or self.ClT2():
      flags= flags+ 'ClT '
    if self.spare1():
      flags= flags+ 'spare1 '
    if self.spare2():
      flags= flags+ 'spare2 '
    #if self.spare3():
    #  flags= flags+ 'ESR '
    #check ESR:
    if self.spare3() != self.ESR():
      self.warn("ESR flag missing (in word0.bit9 or word4.bit13)")
    #check ClT:
    if (self.ClT1()+self.ClT2()) == 1:
      self.warn("ClT flag missing (in word0 or word4)")
    if self.SCode()>3 and (self.ClT1()==0):
      self.warn("ClT flag missing in word0")
    if self.SCode()>3 and (self.ClT2()==0):
      self.warn("ClT flag missing in word4")
    instrline= "%s %s %s %s %s\n" % (Seq.SNAME[self.SCode()],
      l1class, l2class, l2cluster, flags)
    if self.warns:
      instrline= instrline+"Warnings: "+ self.warns+"\n"
    return instrline,seqok
class Disslm:
  ALERRS= ["PP", "L0", "L1", "L1M", "L1&L1M", "L2aM", "L2rWord"]
  def __init__(self, seqfile):
    self.loglist= ''
    self.AllowedErrors= 0
    self.fileok= 1   # 0 if load not recommended
    self.slm= []   #max. 32 items (sequences)
    s= None
    if os.access(seqfile, os.R_OK) == 0:
      self.error(seqfile+" doesn't exist")
      return
    sf= open(seqfile, 'r')
    ln=-4   #ignore first 3 lines
    for line in sf.readlines():
      if line=='' or line==None or len(line)<=1: 
        self.error("Error:too short line");
        self.fileok=0
      ln= ln+1
      if ln==-1:   # allowed errors flags
        self.AllowedErrors= self.a2bin(line,7)
        continue
      if ln<0: 
        continue
      #if ln>20: break
      #print ln,':',line[:-1]
      if (ln % 8) == 0:
        #print ln,'::',line[:-1]
        if line[13:16]=='000': 
          self.error("Warning: seq. code 0 found in input file")
          #break
        #s=Seq()
        self.slm.append(Seq())
      if self.slm[-1].word(ln%8, line):
        self.error("Error: incorrect format of input file")
        del(self.slm[-1])
        self.fileok=0
        break
      if (ln % 8) == 7:
        if self.slm[-1].Last(): break
    if ln<(11-4):
      self.error("Error:not enough lines (at least 11 expected)");
      self.fileok=0
    sf.close()
  def a2bin(self,line,bits):
    """Convert binary string 10111 into 0x17"""
    binword=bit=0
    #print "a2bin:",line
    #for i in range(bits-1,-1,-1):
    if len(line)<bits+1:
      self.error("Incorrect ascii binary(short line):"+line[:-1])
      return binword
    for i in range(bits):
      if line[i]=='1':
        binword= binword | 1<<bit
      elif line[i]!='0':
        self.error("Incorrect ascii binary:"+line[:-1])
        break
      bit= bit+1
    return binword
  def add2list(self,text):
    self.loglist= self.loglist+text
  def error(self,text):
    self.add2list(text+"\n")
  def getaerrs(self):   # called form ltu_u.py
    return self.AllowedErrors
  def printaerrs(self):
    aers=''
    for i in range(7):
      if ((1<<i) & self.AllowedErrors) != 0:
        aers= aers+ " "+ Disslm.ALERRS[i]
    if aers != '':
      aers= "Allowed errors:" + aers +"\n"
    return aers
  def getlist(self):
    n=0     # sequence number (printed from 1:)
    aers= self.printaerrs()
    if aers != '':
      self.add2list(aers)
    for sq in self.slm:
      lst,sqok= sq.printcheck(); n=n+1
      self.add2list("%2d: "%(n)+lst)
      if sqok==0:
        self.fileok=0   # load not recommended
    if len(self.slm)>0 and self.slm[-1].Last()==0:
      self.error("Error: LAST flag not set in the last sequence")
      self.fileok=0
    return self.loglist

def main():
  import sys
  if len(sys.argv) < 2:
    print """

Reverse compilation of .seq file (text file consisting from 0,1
representing bits in LTU-sequencer memory). The format of .seq
file is described in file slmcomp.py.

Usage: slmcomp.py name.seq
Expected abs. path or relative path. name  to VMECFDIR 
(e.g. CFG/ltu/SLM/all.seq)
Operation: .slm file printed to stdout
"""
    #a= Disslm("all.slm")
    #print a.getlist()
  else:
    #os.chdir(os.environ['VMECFDIR'])
    for bn in sys.argv[1:]:
      a= Disslm(bn)
      print a.getlist()

if __name__ == "__main__":
  main()

