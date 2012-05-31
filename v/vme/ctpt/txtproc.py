#!/usr/bin/python
"""
Text rocessing for LUT (Look up table) or BCmask
28.1.2006 bug fixed (space couldn't appear anywhere)
"""

import string, types

ORBITLENGTH=3564

ErrorMessage=''
def PrintError(fstr):
  global ErrorMessage
  print "Error:",fstr
  ErrorMessage= ErrorMessage+fstr+'\n'

class ST:
  RIGHT=1 # bracket
  LEFT=2
  BIT=3   # bit (H or L -value 1 or 0)
  REP=4   # repetitions (int)
  EOM=5   # End Of Mask
  def __init__(self, text):
    self.type=None
    self.value=None
    self.length=0
    if text=='': 
      self.type=ST.EOM; self.length=0
      #print "ST:EOM"
      return
    while 1:
      if text[self.length]==' ': 
        self.length=self.length+1
        if self.length==len(text):
          self.type=ST.EOM; return
      else: break
    if text[self.length]=='L' or text[self.length]=='l':
      self.type=ST.BIT; self.value=0; self.length=self.length+1
    elif text[self.length]=='H' or text[self.length]=='h':
      self.type=ST.BIT; self.value=1; self.length=self.length+1
    elif text[self.length]=='(':
      self.type=ST.LEFT;  self.length=self.length+1
    elif text[self.length]==')':
      self.type=ST.RIGHT;  self.length=self.length+1
    elif text[self.length] in string.digits:
      self.length=self.length+1
      while 1:
        if text[self.length] in string.digits:
          self.length=self.length+1
        else: break
      self.type= ST.REP 
      self.value= int(text[:self.length])
    else:
      PrintError(" bad string:"+ text)
    #print "ST:", self.type, self.length, "val=",self.value
class BCmask:
  def __init__(self,bcmexp):
    """
    bcmexp: "25L 25(2H2LH 3(23HL))"
          - H/h -> 1  L/l -> 0
          - spaces, new lines are white characters
          - ignore comment lines starting with #
          - max. 3560 bits (warning for less or more)
    output: list of XXX words, 32 bits per word
    """
    self.bcmstr=bcmexp
    #print "BCmask2input:",bcmexp
    self.level=0
    #bcm=self.bcm2bits(0)
    #print "BCmask2:",bcm
  def getst(self, ix):
    """
    ix -0..  -points to self.bcmstr
    rc: token,ix   ix -> points just after token
    """
    st= ST(self.bcmstr[ix:])
    return st
  def bcm2bits(self,ix):
    self.level= self.level+1
    if self.level>=10:return [2],0
    #print "bcm2bitsSTART:",self.level,ix
    repc=1
    ix1=ix
    bitstr=[]
    while 1:
      st= self.getst(ix1)
      if st.type==ST.BIT:    # push
        for i in range(repc):
          bitstr.append(st.value)
        repc=1
        ix1=ix1+st.length
      elif st.type==ST.REP:
        repc=st.value
        ix1=ix1+st.length
      elif st.type==ST.LEFT:
        ix1=ix1+st.length
        bitstr2,ix2= self.bcm2bits(ix1)
        for i in range(repc):
          bitstr= bitstr+ bitstr2
        repc=1
        ix1=ix1+ix2
      elif st.type==ST.RIGHT:    # pop
        ix1=ix1+st.length
        if self.level<=1:
          PrintError(") incorrectly placed at"+str(ix))       
        break
      elif st.type==ST.EOM:    # pop
        ix1=ix1+st.length
        if self.level>1:
          PrintError(") missing")       
        break
      else:
        PrintError("incorrect BC mask")       
        break
    self.level= self.level-1
    #print "bcm2bits:",bitstr,ix1
    return bitstr,ix1-ix
  def setbits(self,lst=None):
    """lst: [pointer to list, bitnumber]
    bitnumber: 0..3
    """
    bcm,procesed=self.bcm2bits(0)
    if len(bcm)>ORBITLENGTH:
      return "Error: Too long BCmask definition\n"
    if procesed== len(self.bcmstr):
      bitmask= 1<<lst[1]
      #print "setbits1:",bitmask,bcm,procesed
      #lst[0][1:2]=[10,11]
      for ix in range(len(bcm)):
        if bcm[ix]==1:
          lst[0][ix]= lst[0][ix] | bitmask
        else:
          lst[0][ix]= lst[0][ix] & (0xf & ~bitmask)
      #print "setbits2:",lst[0][0:9]
      if len(bcm)<ORBITLENGTH:
        return "Warning: %d: short BCmask definition (bits above %d not touched)\n"%(len(bcm),len(bcm))
      else:
        return None
    else:
      return ErrorMessage
def log2tab(logexp, vo=["a","b","c","d"]):
  """
  logexp: python log. expression with i0,...,i3 variables or
          0xabcd  -LUT defined directly by table
  vo:     ["var0","var1","var2","var3"]  -names of 4 or less
          variables in expression. len(vo) identifies
          number of output bits (1:1 2:4 3:8 4:16)
  output: 16 bits -results of all the possible (0000,0001,...,1111)
          input combinations
          None -if error occured during conversion
          None -if input string is: 'n/a'
  """
  if type(logexp) != types.StringType: return None
  if len(logexp)>=2:
    if logexp[0:2]=='0x': return logexp
  if logexp=='n/a': return None
  res=0
  ixproc=[0,0,0,0]         # all items not processed
  exp2t=[2,4,8,16]
  valbitsinres=[1,0xf, 0xff, 0xffff]
  lenvo=len(vo); exp2= exp2t[lenvo-1]
  for ix in range(lenvo):      #was 4
    # find the lenghtest one, process and mark as done:
    maxlen=0
    for ix2 in range(lenvo):   #was 4
      if ixproc[ix2]==1: continue
      if maxlen<len(vo[ix2]):
        maxlen=len(vo[ix2])
        maxix=ix2
    #print vo[maxix],'-> i',maxix
    logexp= string.replace(logexp,vo[maxix],"i%0d"%maxix)
    ixproc[maxix]= 1
  for ix in range(exp2):       #was 16
    #if ix&1:eval(vo[0]+"=1")
    #else: eval(vo[0]+"=0")
    if ix&1: i0=1    
    else: i0=0
    if ix&2: i1=1
    else: i1=0
    if ix&4: i2=1
    else: i2=0
    if ix&8: i3=1
    else: i3=0
    try:
      #print i3,i2,i1,i0,':',eval(logexp)
      res= res | (eval(logexp) <<ix)
      # if too many 1s, error:
      if res & (~(valbitsinres[lenvo-1])):
        PrintError("too many 1s in %s LUT: %x"%(logexp,res))
        return None
    except:
      return None
  return "0x%x"%(res)
def main():
  #print hex(log2tab("blai0&ai3&~ai33",["blai0","ai3","i2","ai33"]))
  print string.digits
  #bcm= BCmask("3L4H 3(H2(L3h)LH)")
  bcm= BCmask("3L4H 3(LH)")
  bcm= BCmask("")
  bcm= BCmask("3X("); bcm.setbits()

if __name__ == "__main__":
    main()

