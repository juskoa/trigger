#!/usr/bin/python
import sys,string,os,os.path,popen2
orbitl=3564
def b_sync(fdf):
  df= int(fdf)
  ndf= df%orbitl   # normalized (0..3563)
  wn= df/orbitl    # integer part
  # 3564= 3**4 * 2*2 * 11
  upper= bottom= ndf
  points=0
  while (points<1):
    if (upper%2 != 0) and (upper%3 != 0) and (upper%11 != 0):
      upperx= upper + wn*orbitl
      #print "%7.2fx   %d =      %7.2f"%(df, upperx-1, upperx)
      points= points+1
    upper= upper+1
  return upperx-1
def b_syncReal(n):
  return n+1
def r_sync( fdf):
  n= int(0x7fffffff/fdf)
  #realdf= 0x7fffffff/float(n)
  #print " request   n     real downscaling"
  #print "%7.2fx   %d =      %7.2f"%(fdf, n, realdf)
  return n
def r_syncReal( n):
  return 0x7fffffff/float(n)
def classDown(fdf):
  return 100./fdf
def classDownn(fdf):
  pr= 100./fdf
  n= int(round(0x1fffff*((100-pr))/100.))
  return n       # 21 bits into hw
def iopipe(cmd, grep=''):
   #print "popen2("+cmd+")"
   iop= popen2.popen2(cmd, 1) #0- unbuffered, 1-line buffered
   line=''
   if iop:
     if grep=='1':
       line= iop[0].readline()
     elif grep!='':   # return 1st line starting with given string
       while True:
         line= iop[0].readline()
         if line =='': break
         if string.find(line, grep)==0:
           break
     iop[0].close()
     iop[1].close()
   return line

def getactv0rate():
  exe= os.path.join(os.environ['VMECFDIR'], "ctp_proxy/linux/act.exe")
  print exe
  rc= string.split(iopipe(exe+" VALUE V0ANDrate", "VALUE"))
  if len(rc)==2:
    v0rate= rc[1]
  else:
    v0rate=None
  return v0rate
class Downscaling:
  def __init__(self, v0rate=None):
    self.v0and=None   # was error if None
    if v0rate==None: 
      # BC1 RND2 RND1 MSL
      self.snms=["$MB","$CENT","$SCENT","$MSL"]
      return
    self.snms=["123456787","123456788","123456789","0.0%"]
    # e.g. "0.5": this instance for this rate
    import trigdb
    fname= os.path.join(trigdb.TRGDBDIR, "downscaling.rates")
    if os.path.exists(fname):
      f= open(fname,"r")
      for line in f.readlines():
        if line[0]!= "#":
          sl= string.split(line)
          if sl[0]!=v0rate: continue
          self.initSYMS(line)
      f.close()
  def replace_inline(self, line):
    src= line ; change=0
    for ix in range(4):
      orig= self.snms[ix]
      ix1= string.find(src, orig)
      if ix1==-1: continue
      # check before repalcement: BC1 RND2 RND1 C
      if orig=="123456787":
        if string.find(line,"BC1")==-1:
          print "ERROR: %s not in BC1 line"%orig
          continue
      elif orig=="123456788":
        if string.find(line,"RND2")==-1:
          print "ERROR: %s not in RND2 line"%orig
          continue
      elif orig=="123456789":
        if string.find(line,"RND1")==-1:
          print "ERROR: %s not in RND1 line"%orig
          continue
      elif orig=="0.0%":
        if string.find(line,"C")!=0:
          print "ERROR: %s not in Classes line"%orig
          continue
      rep= string.replace(src, orig, self.symb[ix])
      change= change+1
    if change==0:
      rep= src
    return rep
  def initSYMS(self, iline):
    sl= string.split(iline)
    if len(sl)< 4: return
    self.v0and= sl[0] ; self.df=[] ; self.symb=[]
    for ix in range(4):
      #MB CENT SCENT MSL
      self.symb.append(sl[ix+1])  
  def initDFS(self, iline):
    sl= string.split(iline)
    #print "downscaling:",sl
    if len(sl) != 5: return
    self.v0and= sl[0] ; self.df=[] ; self.symb=[]
    for ix in range(4):
      #downscaling factors for MB CENT SCENT MSL
      self.df.append(float(sl[ix+1]))  
      self.symb.append("")
  def doSymbols(self):
    self.symb[0]= str(b_sync(self.df[0]))
    self.symb[1]= str(r_sync(self.df[1]))
    self.symb[2]= str(r_sync(self.df[2]))
    #self.symb[1]= "%7.2f"%r_sync(self.df[1])
    #self.symb[2]= "%7.2f"%r_sync(self.df[2])
    self.symb[3]= "%2.3f%%"%classDown(self.df[3])
  def prt(self):
    print "#real\t%d\t%.2f\t%.2f\t%s"%(int(self.symb[0])+1,\
      r_syncReal(float(self.symb[1])),\
      r_syncReal(float(self.symb[2])),\
      self.symb[3])
    print "%s\t%s\t%s\t%s\t%s"%(self.v0and,self.symb[0],self.symb[1],self.symb[2],self.symb[3])
  def prt_dfs(self):
    print "#%s\t%.2f\t%.2f\t%.2f\t%.2f"%(self.v0and,self.df[0],self.df[1],self.df[2],self.df[3])
  def repargs(self):
    """ prepare args for 'replace' command
"""
    ra=""
    for ix in range(4):
      ra= ra+'"' + self.snms[ix] + '" "' + self.symb[ix] + '" '
    return ra
  def replace_syms(self, orig):
    src= orig
    for ix in range(4):
      rep= string.replace(src, self.snms[ix], self.symb[ix])
      if rep==src:
        print "ERROR:", self.snms[ix], "not found"
      else:
        src= rep
    return rep
    
def printhelp():
  print """
t table_name 
  -convert 'downscale factors table' to the format readable by trigger sw. 
   stdout HAS TO saved in file downscaling.rates
p partFile V0and_rate
  partFile  : partition file definition (NO .partition OR ANY OTHER EXTENSION )
              Study PbPbNOPF11 example. look for symbols: $MB $CENT $SCENT $MSL -
              these will be replaced by corresponding values in downscaling.rates
  V0and_rate : index (1st column) in downscaling.rates table 
               all: go through all indexes (column1 values) in the table
  Operation: create partFile_X.Xkhz.partition file
"""
def main():
  if len(sys.argv)<=1:
    printhelp()
    sys.exit()
  if sys.argv[1]=='t':
    f= open(sys.argv[2],"r")
    for line in f.readlines():
      if line[0]== "#":
        print line[:-1]
      elif line=='\n':
        continue
      else:
        ds= Downscaling(); ds.initDFS(line)
        ds.doSymbols()
        #print "#", line[:-1]
        ds.prt_dfs()
        ds.prt()
    f.close()
  if sys.argv[1]=='p':
    f= open("downscaling.rates","r")
    dws= {}
    for line in f.readlines():
      if line[0]!= "#":
        ds= Downscaling(); ds.initSYMS(line)
        dws[ds.v0and]= ds
    f.close()
    v0rate= sys.argv[3]
    if v0rate=='all':
      rates= dws.keys()
    else:
      rates= [v0rate]
    ifile= sys.argv[2]
    f= open(ifile,"r"); pattern_part= f.read() ; f.close()
    for v0rate in rates:
      ofile= sys.argv[2]+ '_' + v0rate + 'khz.partition'
      print ofile,'...'
      # replace not available always...
      #cmd= "replace "+dws[v0rate].repargs() +"<" + ifile + " >" + ofile
      #print cmd
      #os.system(cmd)
      if dws.has_key(v0rate):
        out_part= dws[v0rate].replace_syms(pattern_part)
        f= open(ofile,"w"); f.write(out_part) ; f.close()
      else:
        print "ERROR: %s rate not defined in dowscaling.rates file"%v0rate
if __name__ == "__main__":
    main()
