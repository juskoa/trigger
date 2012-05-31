#!/usr/bin/python
import sys,string,time
ORBITL=3564
def chrep(ch,rep):
  if rep==1:
    return ch
  else:
    if len(ch)==1:
      return "%d%s"%(rep, ch)
    else:
      return "%d(%s)"%(rep, ch)
def shrink(msk,width=1,start=0):
  """ correct result only for width=1 with any mask.
  Idea with 'width' seems not applicable:
  H1 -not repeatable, i.e. string must be finished by H, L or )
  in the case of ), the corresponding nmber of ( must match
  examples of good/bad msk:
  
  """
  ix1=start; ous= msk[:start]; lenmsk=len(msk);
  while ix1<lenmsk:
    ix2= ix1+width; 
    rep=1; 
    if ix2>=lenmsk:
      ch=msk[ix1:]
      ous= ous+chrep(ch,rep)
      break
    ch=msk[ix1:ix2]
    print "shrink:%d:ch:"%ix1,ch
    time.sleep(0.1)
    while ix2<lenmsk:
      if ch==msk[ix2:(ix2+width)]:
        rep= rep+1; ix2= ix2+width
        print "rep:",rep,ix2
      else:
        #ous= ous+chrep(ch,rep)
        #ix1=ix2
        break
    ous= ous+chrep(ch,rep)
    print "  ",chrep(ch,rep),"ix1:",ix1
    ix1=ix2
  #ous= ous+chrep(ch,rep)
  return ous
class ctpmask:
  def __init__(self, schname):
    lsf= open(schname+".mask","r")
    for line in lsf.readlines():
      #line= line.rstrip()
      #print ":%s:"%string.rstrip(line)
      mline= string.split(string.rstrip(line))
      #print mline[0]
      if mline[0]=="bcmD":
        self.mask= mline[1]
        break   # take 1st bcmD
    lsf.close()
    self.maar=[]   # 0..3563 L-ok(triggering) H: disabled
    rc=self.domask(self.mask)
    print "rc domask:", rc
  def domask(self, msk):
    #print "start domask:",msk
    ixline=0; allx=lastnum=0
    while 1:
      lex= self.lex(msk, ixline)
      l0= lex[0]
      if l0==None: break
      elif  l0=='H' or l0=='L':
        if lastnum==0: lastnum=1
        for ix in range(lastnum):
          self.maar.append(l0)
        #bc=bc+lastnum; 
        lastnum=0; ixline= ixline+ lex[1] ; allx=allx+ lex[1]
      elif l0=='r':
        lastnum= lex[1]; ixline= ixline+ lex[2]
        allx= allx+lex[2]
        #print "r domask: lastnum:%d lex[2]:%d"%(lastnum,lex[2])
      elif l0=='(':
        if lastnum==0: lastnum=1
        ixline= ixline+ lex[1] ; allx= allx+lex[1]
        #print "repeat %dx domask:"%lastnum,msk[ixline:]
        for ix in range(lastnum):
          rc= self.domask(msk[ixline:])
        ixline= ixline+ rc ; allx= allx+rc
      elif l0==')': 
        allx= allx+lex[1]
        break
      if ixline>=len(msk):break
    #print "end domask %d:"%allx,msk
    return allx
  def lex(self,line,ix):
    """ rc: 
    (c,1)    c is one of: 'H' 'L' '(' ')' 
    (None, None)
    ('r',intnumber,lexlen)
    ('e', error_text)
    """
    if ix>=len(line): return (None,None)
    c=line[ix]
    if c=='H' or c=='L' or c=='(' or c==')':
      return (c,1)
    if c>='0' and c<='9':
      ixx=ix ; num= 0; lexlen=0;
      while 1:
        if c>='0' and c<='9':
          num= num*10 + int(c); lexlen= lexlen+1; ixx=ixx+1
        else:
          break
        if ixx>=len(line): break
        else: c= line[ixx]
      return ('r', num, lexlen)
    return ('e',line[ix:])
  def prt_maar(self):
    l=h=0; mask=''
    for ix in range(len(self.maar)):
      lh= self.maar[ix]
      mask=mask+lh
      if lh=='L': 
        l= l+1
      else:
        h= h+1
    print "L:%d H:%d mask:"%(l,h)
    #print mask
  def prt(self):
    ixline=0; bc=0; enabled=allx=0; lastnum=0
    while 1:
      lex= self.lex(self.mask, ixline)
      l0= lex[0]
      if l0==None: break
      if  l0=='H':
        if lastnum==0: lastnum=1
        allx=allx+lastnum
        bc=bc+lastnum; lastnum=0; ixline= ixline+ lex[1]
      if  l0=='L':
        if lastnum==0: lastnum=1
        print "L: bc:%d-%d %d"%(bc,(bc+lastnum-1),lastnum)
        enabled= enabled + lastnum
        allx=allx+lastnum
        bc=bc+lastnum; lastnum=0; ixline= ixline+ lex[1]
      if l0=='r':
        lastnum= lex[1]; ixline= ixline+ lex[2]
      if ixline>=len(self.mask):break
    print "Enabled:%d all:%d"%(enabled,allx)
def main():
  # shrink business:
  #msk="2H5H1L5H1L5H1L5H1L5H1L5H1L5H1L281H1L5H1L5H1L5H1L5H1L5H1L5H1L5H1L3175H"
  #msk="HHHHHHLL"
  #msk="1L1LH1L5H"
  #print shrink(msk,int(sys.argv[1]), int(sys.argv[2]) )
  #print shrink(msk, width=4,start=2)
  # ctpmask (read mask in, calculate L,H):
  msk= ctpmask(sys.argv[1]); msk.prt_maar()

if __name__ == "__main__":                                                      
    main()
