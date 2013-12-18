#!/usr/bin/python
"""
Translate .slm (mnemonic SLM instructions) to .seq (binary-like) file.
.seq can be than processed by ltu6.tcl (gui) or loaded into LTU-SLM

24.1.2006 -now, with L0 classes/clusters don't have to be defined
           similarly for L2R (L2classe/clusters) and CL0, CPP
11.5. ESR now is coded in L2msg too (i.e. 2 bits: 0.9 and 4.13)
28.3.2008 NOSWC flag added -forcing ClT, L1SwC and L2SwC to 0
11.12.2013 run2 support added (with slmdefs.py)
16.12. run2 32-bits SLM memory (L2data in parallel with L1data)
"""
import os.path, os, string, slmdefs

sdf= slmdefs.Slmdefs_run2
slmseqpath='WORK/slmseq.seq'
errtext=''
wartext=''
def Err(text=None, warn=None, line=None):
  global errtext,wartext
  if text==None: 
    errtext=""; wartext=""
  else:
    if warn:
      if line: wartext= wartext+line+'\n'
      wartext= wartext+"Warning:"+text+'\n'
    else:
      if line: errtext= errtext+line+'\n'
      errtext= errtext+"Error:"+text+'\n'

class Sequence:
  def __init__(self, line):
    #print line
    self.s=[]   # 8 or 15 words of 1 trg. sequence
    for ix in range(sdf.wordseq):
      self.s.append(0)
    self.noswc=0               # NOSWC flag not present
    self.line=line
    atms= string.split(line)
    code=None
    if atms[0]=='L0': code=1
    if atms[0]=='L2A': code=2
    if atms[0]=='L2R': code=3
    if atms[0]=='CPP': code=4
    if atms[0]=='CL0': code=5
    if atms[0]=='CL2A': code=6
    if atms[0]=='CL2R': code=7
    if not code:
      Err("Unknown sequence code:"+atms[0])
      return
    if len(atms)==1:
      atms.append("0x0")
      if code==2 or code==3 or code==6 or code==7:
        Err("No L1 classes","Warning",line)
    #clas1= self.proccls(atms[1], 3, 0, 50)
    clas1= self.proccls(atms[1], sdf.L1cls[0], sdf.L1cls[1], sdf.L1cls[2])
    if not clas1:
      Err("Bad definition of L1 classes:"+atms[1])
      return
    # Move bits self.s[0][1..0] to right place (L1Class 50-49):
    # and put Instr. code to bits self.s[0][2..1]
    self.s[0]= ((self.s[0] & 3)<<sdf.L1cls2) | code
    if code>3: self.setCIT()
    if len(atms)==2:
      atms.append("0x0")
      if code==2 or code==6:
        Err("No L2 classes","Warning",line)
    #if len(atms)>2:       to be finished (L1 does not need l2classes)
    # if atms[2][:2]=='0x':
    #  if (code==2) or (code==6):
    #clas2= self.proccls(atms[2], 7, 3, 50)
    lbitsh=0
    if sdf.version==3: lbitsh=16
    clas2= self.proccls(atms[2], sdf.L2cls[0], sdf.L2cls[1], sdf.L2cls[2], lbitsh)
    if not clas2:
      Err("Bad definition of L2 classes:"+atms[2])
      return
    if len(atms)==3:
      atms.append("0x0")
      if code==2 or code==6:
        Err("No Cluster defined","Warning",line)
    #cluster= self.proccls(atms[3], 4, 5, 6)
    cluster= self.proccls(atms[3], sdf.Clust[0], sdf.Clust[1], sdf.Clust[2], lbitsh)
    if not cluster:
      Err("Bad definition of L2 clusters:"+atms[3], None,line)
      return
    for fix in range(4, len(atms)):
      flag= self.procflag(atms[fix])
      if not flag:
        Err("Unknown flag or bad definition of flag:"+atms[fix],None,line)
        return
    if self.noswc==1: self.unsetCIT()
  def proccls(self, lhexa, wix, bix, bitlength, bits_shift=0):
    """
    lhexa: "0x123456789abcd" (max. 13/25 hex. digits)
    wix,bix: 3,0 for L1Class  7,3 for L2Class, 4,5 for L2Cluster
             i.e. starting position of 1st bit: bits are placed than from right
             to the left (in bits 0..1 or 16..31) in words from the last word (wix)
    bitlength   : max. number of bits
    bits_shift: 0: bits 15..0      16: bits 31..16
    returns: None in case of error
      1: bit string correct, words 3-0 or 7-4 or 4 filled in correspondingly
    """
    if lhexa[:2]!='0x':
      Err("hexadecimal number not beginning with 0x:"+lhexa)
      return None
    word=wix; bitn=1
    bit=bix - bits_shift
    #print "proccls:",lhexa, wix, bix, bitlength
    #0x400000000000f000000000003
    for ix in range(len(lhexa)-1,1,-1):
      hdig= eval('0x'+lhexa[ix])
      #for ix4 in range(3,-1,-1):
      for ix4 in range(4):
        if bitn>bitlength: break
        if hdig & (1<<ix4):
          self.s[word]= self.s[word] | (1<<(bit+bits_shift))
          #print "proccls:word biti bitn:", word, bit, bitn
        bit= bit+1
        if bit==16:
          bit=0; word=word-1
        bitn= bitn+1
      if bitn>bitlength: break
    return 1
  def setCIT(self):
    w= sdf.CITflg[0]
    self.s[w]= self.s[w] | sdf.CITflg[1]  # CIT + L1SwC flag
    w= sdf.CITflg[2]
    self.s[w]= self.s[w] | sdf.CITflg[3]   # CIT + L2SwC flag
  def unsetCIT(self):
    w= sdf.CITflg[0]
    self.s[w]= self.s[w] & ~sdf.CITflg[1]   # CIT + L1SwC flag
    w= sdf.CITflg[2]
    self.s[w]= self.s[w] & ~sdf.CITflg[3]   # CIT + L2SwC flag
    Err("ClT, L1SwC and L2SwC forced to 0 (NOSWC)","Warning", self.line)
  def procflag(self, flag):
    rc= 1
    if (flag== "ClT") or (flag== "CIT"): self.setCIT()
    elif flag[0:4]=="roc=":
      try:
        roc= int(flag[4:])
      except:
        Err("roc flag has to be integer 0..15 (%s is incorrect), setting 0..."%flag)
        roc=0
      if roc>15:
        Err("roc=%d    >15, setting 4 least significant bits...)"%(roc))
      self.s[0]= self.s[0] | ((roc&0xf)<<sdf.rocsh)
    elif flag=="ESR":
      self.s[0]= self.s[0] | 0x200
      self.s[sdf.esr2[0]]= self.s[sdf.esr2[0]] | sdf.esr2[1]
    elif flag=="L1SwC":
      self.s[0]= self.s[0] | 0x100
    elif flag=="L2SwC":
      self.s[sdf.l2swc[0]]= self.s[sdf.l2swc[0]] | sdf.l2swc[1]
    elif flag=="NOSWC":
      self.noswc=1
    elif flag=="ErrProne":
      self.s[0]= self.s[0] | 0x20
    elif flag=="Restart":
      self.s[0]= self.s[0] | 0x8
    elif flag=="Last":
      self.s[0]= self.s[0] | 0x10
    elif flag=="spare1":
      self.s[0]= self.s[0] | 0x8000
    elif flag=="spare2":
      self.s[sdf.spare23w]= self.s[sdf.spare23w[0]] | sdf.spare23w[1]
    elif flag=="spare3":       # only L2-bit of ESR required
      self.s[sdf.spare23w]= self.s[sdf.spare23w[0]] | sdf.spare23w[2]
    else:
      rc=None
    return rc
  def savebin(self, of):
    if sdf.version==3:
      w_width= 31
    else:
      w_width= 15
    for ix in range(sdf.wordseq):
      l=''
      #print "%d: %8.8x"%(ix,self.s[ix])
      for i15 in range(w_width,-1,-1):
        if self.s[ix] & (1<<i15):
          c='1'
        else:
          c='0'
        l= l+c
      of.write( l+'\n')
     
class Cmpslm:
  def __init__(self, slmfile, run="lturun2"):
    global sdf
    if run=="-run1":
      sdf= slmdefs.Slmdefs
    if run=="-run2":
      sdf= slmdefs.Slmdefs_run2
    Err()
    self.slm=[]   # max. 32/17 items (sequences)
    self.allowederrs=[0,0,0,0,0,0,0]   # allowed errors
    if os.access(slmfile, os.R_OK) == 0:
      Err(slmfile+" doesn't exist")
      return
    self.bname= os.path.basename(slmfile)
    sf= open(slmfile, 'r')
    for line in sf.readlines():
      #print "Cmpslm:",line
      if line[:7]== 'Errors:':
        for er in string.split(line[7: -1]):
          if er=='PP': self.allowederrs[0]=1
          elif er=='L0': self.allowederrs[1]=1
          elif er=='L1': self.allowederrs[2]=1
          elif er=='L1M': self.allowederrs[3]=1
          elif er=='L1&L1M': self.allowederrs[4]=1
          elif er=='L2aM': self.allowederrs[5]=1
          elif er=='L2rWord': self.allowederrs[6]=1
          else: Err("Unknown allowed errors flag:"+er)
        continue
      if line[0]=='#': 
        #print line
        continue
      if line[0]=='\n': continue
      line= line[0:-1]
      self.slm.append(Sequence(line))
    if len(self.slm)>0 and ((self.slm[-1].s[0] & 0x10)==0):
      self.slm[-1].s[0]= self.slm[-1].s[0] | 0x10
      Err("LAST flag forced to 1 in the last sequence", "Warning")
    sf.close()
  def savefile(self, fn):
    of= open(fn, "w")
    of.write( string.split(os.path.basename(fn),'.')[0]+'\n')
    of.write( str(len(self.slm))+'\n')
    l=''
    for i7 in range(7):
      if self.allowederrs[i7]!=0:
        c='1'
      else:
        c='0'
      l= l+c
    of.write( l+'\n')
    for ix in range(len(self.slm)):
      self.slm[ix].savebin(of)
    of.close()
def main():
  global errtext, wartext
  import sys
  if len(sys.argv) < 2:
    print """
Convert .slm file to .seq file.
See CFG/ltu/SLM/all.slm file for example of .slm file

Usage: cd $VMEWORKDIR
       $VMECFDIR/slmcmp.py [-run1] name.slm

name.slm: abs. path or relative path. name  to VMEWORKDIR
(e.g. CFG/ltu/SLM/all.slm)
-run1: optional parameter (by default, run2 data file is created

Operation:
File WORK/slmseq.seq is created, which can be than:
 -loaded to LTU seq. memory (see ltu.c SLMload()) or
 -reverse-compiled by slmcomp.py ->which should return
  on the output original .slm file
"""
    #a= disslm("all.slm")
    #print a.getlist()
  else:
    #os.chdir(os.environ['VMEWORKDIR'])
    run= "-run2"
    for bn in sys.argv[1:]:
      if bn=="-run1":
        run= "-run1"
        continue
      if bn=="-run2":
        run= "-run2"
        continue
      a= Cmpslm(bn, run)
      if wartext:
        print wartext
      if errtext:
        print errtext
      else:
        a.savefile(slmseqpath)
        print "See %s"%slmseqpath 
if __name__ == "__main__":
  main()

