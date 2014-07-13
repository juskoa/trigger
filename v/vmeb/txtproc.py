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
        #print "text:",text, self.length
        if self.length>(len(text)-1): 
          PrintError(text+" -premature end of mask definition (,L or H expected")
          break
        if text[self.length] in string.digits:
          self.length=self.length+1
        else: break
      self.type= ST.REP 
      self.value= int(text[:self.length])
    else:
      PrintError(" bad string:"+ text)
    #print "ST:", self.type, self.length, "val=",self.value
class BCmask:
  def __init__(self,bcmexp, name=""):
    """
    bcmexp: "25L 25(2H2LH 3(23HL))"
          - H/h -> 1  L/l -> 0
          - spaces, new lines are white characters
          - ignore comment lines starting with #
          - max. 3564 bits (warning for less or more)
    output: list of XXX words, 32 bits per word
    """
    self.bcmstr=bcmexp
    self.name=name
    #print "BCmask2input:",bcmexp
    self.level=0
    #bcm=self.bcm2bits(0)
    #print "BCmask2:",bcm
  def prt(self):
    print "BCmask:", self.bcmexp, "level:", self.level
  def getst(self, ix):
    """
    ix -0..  -points to self.bcmstr
    rc: token,ix   ix -> points just after token
    """
    st= ST(self.bcmstr[ix:])
    return st
  def bcm2bits(self,ix):
    """ bcm2bits(0) returns:
    ([array_of_bits],string_length_of self.bcmstr)
    """
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
        if ErrorMessage=='':
          return "Warning: %d: short BCmask definition for %s (bits above %d not touched)\n"%(
            len(bcm), self.name, len(bcm))
        else:
          return ErrorMessage
      else:
        return None
    else:
      return ErrorMessage
  def checkSyntax(self):
    global ErrorMessage
    ErrorMessage=''
    locbcmasks=[]; 
    for i in range(ORBITLENGTH):
      locbcmasks.append(0)
    errmsg= self.setbits([locbcmasks, 0])
    return errmsg

def log2tab(logexp, vo=["a","b","c","d"]):
  """
  logexp: python log. expression using i0,...,i3 variables or
          0xabcd  -LUT defined directly by table
          Operators used in expretion are: | & ~ ^, i.e.
          bitwise operators. 
          ~ = not  has to be in brackets, i.e. a & (~b)
          Lookup table is calculated 
          in loop for all possible input combinations through
          eval(logexp)  -logexp's values are 1/0 constants.
  vo:     ["var0","var1","var2","var3"]  -names of 4 or less (old LUTs), or
          up to 12 variables used in logexp. len(vo) identifies
          number of output bits (1:1 2:4 3:8 4:16 5:32 ... 12:4096)
  output: char string:
    16 bits -results of all the possible (0000,0001,...,1111)
          input combinations
    None -if error occured during conversion
    None -if input string is: 'n/a'
  """
  if type(logexp) != types.StringType: return None
  ixc= string.find(logexp,'#')   # ignore comment at the end of log. expression
  if ixc >=0: logexp= logexp[0:ixc]
  if len(logexp)>=2:
    if logexp[0:2]=='0x': return logexp
  if logexp=='n/a': return None
  res=0
  ixproc=[0,0,0,0,0,0,0,0,0,0,0,0]   # all items not processed
  #von exp2t=[2,4,8,16]
  valbitsinres=[1,0xf, 0xff, 0xffff]
  lenvo=len(vo); exp2= 2**lenvo;  # exp2= exp2t[lenvo-1]
  for ix in range(lenvo):      #was 4
    # find the lenghtest one, process and mark as done:
    maxlen=0
    for ix2 in range(lenvo):   #was 4
      if vo[ix2]==None: continue
      if ixproc[ix2]==1: continue  # taken already
      if maxlen<len(vo[ix2]):
        maxlen=len(vo[ix2])
        maxix=ix2
    if maxlen==0: break   
    #print vo[maxix],'---> i',maxix
    logexp= string.replace(logexp,vo[maxix],"x%d"%maxix)
    logexp= string.replace(logexp,"~"," not ")
    ixproc[maxix]= 1
  #print "exp2:",exp2,"lenvo::",lenvo,"logexp:",logexp
  res4=res4inv=0 ; res4096="" ; res4096inv=""
  for ix in range(exp2):       #was 16
    # set up to 12 i0..i11 variables for eval():
    for iix in range(lenvo):
      if ix & (1<<iix):
        #set_cmd= "i%d= True"%iix
        bova= True
      else:
        #set_cmd= "i%d= False"%iix
        bova= False
      #eval(set_cmd)
      ixproc[iix]= bova
    x0=ixproc[0] ; x1=ixproc[1] ; x2=ixproc[2] ; x3=ixproc[3]; x4=ixproc[4]
    x5=ixproc[5]; x6=ixproc[6]; x7=ixproc[7]; x8=ixproc[8]; x9=ixproc[9];
    x10=ixproc[10]; x11=ixproc[11]
    try:
      tf= eval(logexp)
      #print ix,':',x3,x2,x1,x0,':', tf
      if tf: tf10= 1 
      else: tf10= 0
      if lenvo<=4:
        res= res | (tf10 <<ix)
      if tf10==1:
        res4= res4 | (1<<(ix%4))
        res4inv= res4inv | (1<<(3-(ix%4)))
      #print "res4:",res4, "tf10:",tf10
      if ((ix+1)%4) == 0:
        res4096= "%x"%res4+res4096
        res4096inv= res4096inv+"%x"%res4inv
        #print "res4096:",res4096
        res4=res4inv= 0
      # if too many 1s, error (test only for lenvo<=4):
      if lenvo<=4:
        if res & (~(valbitsinres[lenvo-1])):
          PrintError("too many 1s in %s LUT: %x"%(logexp,res))
          return None
    except:
      #print "exception:",ix
      return None
  if lenvo<=4:
    #print "lenvo<=4. 0x%x"%(res) #return "0x%x"%(res)
    return "0x"+res4096
  else:
    return "0x"+res4096inv

wordchars = '012abcdfeghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-.'
# 012 -input names are '0... 1... 2...'
hexadigits = 'abcdfefABCDEF0123456789'
operators='~&|() '

def varsInExpr(exst):
  """
  Find all symb. names in exst.
  Symb name consists of aphabetical characters, digits, and chars: - _ .
  Allowed operators (syntax is not checked): & | ~ ( )
  Specal case: hexadecimal number 0x... 
  Return: OK,[sname1, sname2,...] -list of found symb. names
          OK,[]                   -in case of 0x...
          ERROR, error message
  """
  rc="OK" ; names=[]
  if (len(exst)>2) and (exst[:2]=='0x'):   #special case: hexa number
    ix=2
    while(1):
      if ix >= len(exst): break
      ch=exst[ix]
      if not (ch in hexadigits):
        rc="ERROR"
        names="Illegal hexadecimal character:"+ch
        break
      ix=ix+1   
    return rc,names
  ix=0 
  while(1):
    if ix >= len(exst): break
    ch=exst[ix]
    if ch in operators:
      #print "Operator:",ch,":"
      pass
    elif ch in wordchars:   #sym.name
      name=ch
      ix=ix+1
      while(1):
        if ix >= len(exst):
          #print "Name:", name, " +End of expression"
          notfound=1
          for ixn in range(len(names)):
            if names[ixn] == name: notfound=None;break
          if notfound: names.append(name)
          break
        ch=exst[ix]
        if (ch in wordchars) or (ch in string.digits): 
          name=name+ch
          ix=ix+1
          continue
        elif ch in operators:
          #print "Name:", name, "+operator:",ch,":"
          notfound=1
          for ixn in range(len(names)):
            if names[ixn] == name: notfound=None;break
          if notfound: names.append(name)
          break
        else:
          rc="ERROR"
          names="Illegal character:"+ch
          break
    else:
      rc="ERROR"
      names="Symbolic name or "+operators+" expected, got:"+ch
    if rc != "OK": break
    ix=ix+1  
  return rc,names
    
def main():
  #print log2tab("blai0&ai3&~ai33",["blai0","ai3","i2","ai33"])
  #print log2tab("(b&d)|(c&d)|(b&c)",["a","b","c","d"])
  #print log2tab("(0VBA&0SM2) | (0VBC&0SM2) | (0VBA&0VBC)",[None,"0VBA","0VBC","0SM2"])
  #expr= "(0VBA&0SM2) | (0VBC&0SM2) | (0VBA&0VBC)"
  #expr= "~0VBA&~0VBC"   nebavi
  #expr= "~0VBC" ; tbl= [None, '0VBA', '0VBC', None]
  #expr= "(~0VBA)&(~0VBC)"
  # check & amp:
  #expr= "(~0VBA)&amp;(~0VBC)" ; tbl= [None, '0VBA', '0VBC', None]
  # check longer luts:
  expr= "(0VBA | 0VBC) & (0SMB|0BPA)" ; tbl= ['0BPC','0BPA', '0VBA', '0VBC', None,'0SMB']
  #expr= "a|b|c|e" ; tbl= ["a","b","c","d","e","f","g","h","i"]
  expr= "a & (~b) & (~c)& (~d)& (~e)& (~f)& (~g)& (~h)& (~i)& (~j)& (~k)& (~l)" 
  #expr= "(~a) & b & (~c)& (~d)& (~e)& (~f)& (~g)& (~h)& (~i)& (~j)& (~k)& (~l)" 
  #expr= "(~a) & (~b) & (~c)& (~d)& (~e)& (~f)& (~g)& (~h)& (~i)& (~j)& (~k)& (~l)" 
  #expr= "a & b & (~c)& (~d)& (~e)& (~f)& (~g)& (~h)& (~i)& (~j)& (~k)& (~l)" 
  #expr= "a & ~b & ~c & ~d & ~e & ~f & ~g & ~h & ~i & ~j & ~k & ~l " 
  tbl= ["a","b","c","d","e","f","g","h","i","j","k","l"]
  #expr= "a|b|c|e" ; tbl= ["a","b","c","d","e","f"]
  #expr= "a&e" ; tbl= ["a","b","c","d","e","f"]
  #expr= "(~0VBA)&(~0VBC)"
  #expr= "0SMB & 0VBA&0VBC" ; tbl= ['0SMB', '0VBA', '0VBC', '0BPA']  # 0x8080
  #expr= "0VBA&0VBC" ; tbl= ['0SMB', '0VBA', '0VBC', '0BPA']   # 0x4040
  #
  txtlut= log2tab(expr, tbl)
  print expr, tbl, txtlut
  expr= "a & (~b)& (~c)& (~d)" ; tbl= ["a","b","c","d"]
  #expr= "(~a) & b& (~c)& (~d)" ; tbl= ["a","b","c","d"]
  #expr= "(~a) & (~b)& (~c)& (~d)" ; tbl= ["a","b","c","d"]
  #expr= "a & b & (~c)& (~d)" ; tbl= ["a","b","c","d"]
  txtlut= log2tab(expr, tbl) ; print expr, tbl, txtlut
  #print log2tab("0xabRc0")
  #print string.digits
  #print varsInExpr("T0 | TRFpre & blabla")
  #bcm= BCmask("3L4H 3(H2(L3h)LH)")
  #bcm= BCmask("3L4H 3(LH)3600L") ; print str(bcm.bcm2bits(0))
  #bcm= BCmask("")
  #bcm= BCmask("3X("); bcm.setbits()

if __name__ == "__main__":
    main()

