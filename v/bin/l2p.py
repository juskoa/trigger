#!/usr/bin/python
# master copy of this file is in: jusko1:afs/scripts
# ssh $dagw 'cat .ssh/id_rsa.pub' >>~/.ssh/authorized_keys
#24.7.:
# - symbolic links -just comparing destinations. If different,
#   the date/time is taken into account
# - L option added
# 2.10.2009 -better ssh (only stdout taked, stderr goes to dev/null)
# 29.11.2009: todo:
# L:
# ~/labpit.log
# src: bin/l2ptar_backup.bash  (vd)
# dest: bin/l2ptar_backup.bash (v)
import os,string,sys #,time
#import pdb
FILTER_DEFAULT='*.[ch] *.cpp *.cxx *.py *eadme* *akefile* *mak* *.txt *.bash *.sh *.csh *.i'
ALLDIRS=('../vmeb *','../vmeb/vmeblib','SSMANA','VME2FPGA', 
  'ltu','ltu/ltulib','ltu_proxy','ltudim','tinproxy','tinproxy/acorde',
  'ctp','ctp/ctplib','ctp_proxy','ctpcnts','toobusy',
  'orbitsync','dimcdistrib','MORELTUS','pydim','ttcmidaemons','TRG_DBED',
  'dimcoff','fanio','fanio/dim','switch','switchgui','cosmicfo', 'validate',
  '../bin *','simple','rfrx','ttcmi','corde','bobr','filling','monitor',
  'inputs','smaq','monscal','monscal++','ttcit','dimsc',
  'CNTRRD', 'CNTRRD/htmls *', 'CNTWEB *', 'CNTWEB/exs *', 
  'PHPMON *', 'PHPMON/status *')

def endswith(name, list):
  for rstr in list:
    ix=string.find(name, rstr)
    if ix == -1: continue
    if (len(rstr) + string.find(name, rstr)) == len(name):
      return True
  return False
class node:
  def __init__(self, node, bdir):
    self.node=node
    self.bdir=bdir
    self.ldir= ''
  def mydir(self):
    d= self.bdir
    if self.ldir!='':
      d= d+'/'+self.ldir
    return d
  def getlist(self, ldir, filter=FILTER_DEFAULT):
    """ set self.all dictionary. 'self.all == {}' indicates 
    error (not existing) or empty directory
    """
    #print "getlist:", ldir, "f:",filter
    #pdb.set_trace()
    if ldir==None: ldir=''
    self.all={}
    #if not os.access(ldir,os.X_OK): return blbina
    if filter==None or filter=='': filter= FILTER_DEFAULT
    bdir =self.bdir
    self.ldir = ldir
    lsf= os.popen("ssh -q "+self.node+" '(cd "+self.mydir()+"; test $? -eq 0 && ls -G -g -l -t --time-style=long-iso "+filter+")' 2>/dev/null","r")
    #lsf= os.popen("ssh -q "+self.node+" '(cd "+self.mydir()+"; ls -G -g -l -t --time-style=long-iso "+filter+")' 2>&1","r")
    #lsf,lsfin,lsferr= os.popen3("ssh "+self.node+" '(cd "+self.mydir()+"; ls -lt)'","r")
    ls=lsf.readlines(); lsf.close; 
    #print "node:%s:"%self.node
    for ilinenl in range(len(ls)):
      #line= string.rstrip(lsf.readline())
      #if ilinenl==0: continue   # total 240
      line= string.strip(ls[ilinenl])
      if line==None or line=='': break
      if line[:3]=='ls:': continue
      linea= string.split(line)
      #print linea
      #drwxr-xr-x  8    2048 2008-06-12 16:58 rf2ttc
      # we should not see ls: bash lines... (redirected to /dev/null)
      if linea[0]=='ls:': 
        print "!:",linea   # should not be here 
        continue
      if linea[0][0]=='d': 
        print "!d:",linea
        continue
      if linea[0]=='bash:': 
        print "!ERROR:",line
        self.all={}
        break   # error
      if len(linea)<6:
        print "Too short ls line:"
        print linea
      # no help with following lines:
      #elif linea[2]=='directory': 
      #  print "!ERROR:",line
      #  self.all={}
      #  break   # error
      #elif linea[3]=='such': 
      #  print "!ERROR:",line
      #  self.all={}
      #  break   # error
      else:
        if linea[0][0]=='l':   # link
          linkdest= linea[7]
        else:
          linkdest= None
        #print "stored:%s\t%s %s %s"%(linea[2],linea[3], linea[4], linea[5])
        #self.all.append((linea[2],linea[3], linea[4], linea[5]))
        #        filename    length   date       time     linkdest
        #                    4867     2007-01-15 08:21
        if not endswith(linea[5], ('.make','_cf.c','_cf.py')):
          self.all[linea[5]]= (linea[2],linea[3], linea[4], linkdest)
        #else:
        #  print "endwith:", linea[5]
      #if ilinenl>8: break
  def gettar(self,fnames):
    cmd= "ssh -q "+self.node+" '(cd "+self.mydir()+"; tar -czf - "+\
      fnames+ ")' >~/l2p.tgz"
    scpf= os.popen(cmd)
    lout=scpf.readlines(); scpf.close; print "tar - >~/l2p.tgz\n"
  def puttar(self,fnames):
    cmd= "ssh -q "+self.node+" '(cd "+self.mydir()+\
      "; tar -zxf - )' <~/l2p.tgz"
    scpf= os.popen(cmd)
    lout=scpf.readlines(); scpf.close; print "tar - <~/l2p.tgz\n"
  def get1(self,fname):
    scpf= os.popen("scp -pq "+self.node+":"+self.mydir()+'/'+fname+' /tmp/')
    lout=scpf.readlines(); scpf.close; print "scp %s ->/tmp/\n"%fname, lout
  def put1(self,fnames, where='d'):
    if where=='d':
      destin= self.node+":"+self.mydir()+'/'
    else:
      destin= self.node+":/tmp/"
    scpf= os.popen("cd /tmp ; scp -pq "+fnames+' '+ destin)
    lout=scpf.readlines(); scpf.close; print "scp /tmp/%s ->%s:%s/\n"%\
      (fnames,self.node, self.mydir()), lout

class labpit:
  def __init__(self, lab, pit, tarshf, pref):
    self.lab=lab; self.pit= pit
    self.tarshf= tarshf; self.pref= pref
    self.l2pit={}; self.p2lab={}
    for lname in lab.all.keys():
      #print "labpit:",lname
      if pit.all.has_key(lname):
        labisnewer=self.newer(lab.all[lname], pit.all[lname])
        if labisnewer==1:
          # lenlab/pit, labdate/time, pitdate/time
          self.l2pit[lname]= (lab.all[lname][0],pit.all[lname][0], 
            lab.all[lname][1], lab.all[lname][2],
            pit.all[lname][1], pit.all[lname][2])
        elif labisnewer==-1:
          # lenlab/pit, labdate/time, pitdate/time
          self.p2lab[lname]= (lab.all[lname][0], pit.all[lname][0], 
            lab.all[lname][1], lab.all[lname][2],
            pit.all[lname][1], pit.all[lname][2])
        elif lab.all[lname][0] == pit.all[lname][0]:
          # length and dates equal
          continue
        else:
          print "ERROR: equal dates, but different lenghts:", lname
      else:   # new file (present in lab only)
        self.l2pit[lname]= (lab.all[lname][0], '-',
          lab.all[lname][1], lab.all[lname][2],
          '-','-')
    for pname in pit.all.keys():
      if not lab.all.has_key(pname):   # pit only
        self.p2lab[pname]= ('-', pit.all[pname][0],
          '-','-',
          pit.all[pname][1], pit.all[pname][2])
  def newer(self, a1,a2):
    if a1[3] and a2[3]: #link on both sides
      if a1[3] == a2[3]:
        return 0        # and the same destination
    if a1[1] > a2[1]: return 1   # date
    elif a1[1] < a2[1]: return -1
    elif a1[2] > a2[2]: return 1 # time
    elif a1[2] < a2[2]: return -1
    else: return 0
  def show(self):
    """
    self.tarshf, self.pref: if !=None, create  l2ptar.bash script (tarshf)
    ONLY for 'u' files (i.e. lab->pit files !
    """
    for k in self.l2pit.keys():
      print "%9s\t%s\t%s\t%s %s   %s %s"%(k, 
        self.l2pit[k][0], self.l2pit[k][1], 
        self.l2pit[k][2], self.l2pit[k][3],
        self.l2pit[k][4], self.l2pit[k][5])
      if self.tarshf:
        self.tarshf.write(self.pref+'/'+k+'\n')
    p2lkeys= self.p2lab.keys()
    if len(p2lkeys)>0:
      print '! Newer on receiving side: <- i.e. use d !!!'
      for k in p2lkeys:
        print "%9s\t%s\t%s\t%s %s   %s %s"%(k, 
          self.p2lab[k][0], self.p2lab[k][1], 
          self.p2lab[k][2], self.p2lab[k][3],
          self.p2lab[k][4], self.p2lab[k][5])
  def update(self, fname):
    if fname==None or fname=='':
      fnames=''
      for k in self.l2pit.keys(): fnames= fnames+' '+k
      self.lab.gettar(fnames)
      self.pit.puttar(fnames)
    else:
      self.lab.get1(fname)
      self.pit.put1(fname)
  def download(self, fname, dest):
    if fname==None or fname=='':
      fnames=''
      for k in self.p2lab.keys(): fnames= fnames+' '+k
      self.pit.gettar(fnames)
      self.lab.puttar(fnames)
    else:
      self.pit.get1(fname)
      self.lab.put1(fname, dest)

def getpar(line, n):
  pars= string.split(string.strip(line))
  if len(pars)>n:
    ldir= pars[n]
  else:
    ldir= ''
  return ldir

def prthelpexit():
  print """Usage:
l2p.py lab pit
where lab is:
5vd   -trigger@pcalicebhm05:/data/ClientCommonRootFs/usr/local/trigger/vd/vme
       DEFAULT
triad -triad@altri1:/usr/local/trigger/v/vme

pit is:
pit   -trigger@alidcscom188:/data/ClientCommonRootFs/usr/local/trigger/v/vme
       DEFAULT
5v    -trigger@pcalicebhm05:/data/ClientCommonRootFs/usr/local/trigger/v/vme

"""
  sys.exit(4)

def showdirfil(ldir, filter, lab, pit, isvd, tarshf=None):
  global globl2p
  lab.getlist(ldir, filter)
  if len(lab.all)==0:
    print "EMPTY -> ..." #; continue
  pit.getlist(ldir, filter)
  if len(pit.all)==0:
    print "... -> EMPTY" #; continue
  #print "%s:%s/%s ->\n%s:%s/%s"%(lab.node,lab.bdir,lab.ldir, pit.node,pit.bdir,pit.ldir)
  #print "%s%s ->%s"%(lab.node, isvd, pit.node)
  print
  pref=None
  if tarshf!=None:
    if ldir[:6]=='../../':
      pref= ldir[6:]
    elif ldir[:3]=='../':
      pref= '$vd/'+ldir[3:]
    else:
      pref= '$vd/vme/'+ldir
  globl2p= labpit(lab, pit, tarshf, pref)
  print "                      %s %s"%(ldir,filter)
  globl2p.show()

def main():
  global globl2p
  labnode= ("trigger@pcalicebhm10", "/home/dl/root/usr/local/trigger/vd/vme")
  isvd='/vd'
  pitnode= ("trigger@alidcscom188", "/data/dl/root/usr/local/trigger/v/vme")
  if len(sys.argv)>1:
    if sys.argv[1]=='05':
      labnode= ("trigger@pcalicebhm05", "/data/ClientCommonRootFs/usr/local/trigger/vd/vme")
    elif sys.argv[1]=='5vd':
      labnode= ("trigger@pcalicebhm05", "/data/ClientCommonRootFs/usr/local/trigger/vd/vme")
    elif sys.argv[1]=='triad':
      labnode= ("triad@altri1", "/usr/local/trigger/v/vme")
      isvd=''
    else:
      prthelpexit()
  if len(sys.argv)>2:
    if sys.argv[2]=='pit':
      pitnode= ("trigger@alidcscom188", "/data/ClientCommonRootFs/usr/local/trigger/v/vme")
    elif sys.argv[2]=='5v':
      pitnode= ("trigger@pcalicebhm05", "/data/ClientCommonRootFs/usr/local/trigger/v/vme")
    else:
      prthelpexit()
  if len(sys.argv)>3:
    pass
  lab=node(labnode[0], labnode[1])
  pit=node(pitnode[0], pitnode[1])
  while 1:
    print """l directory [filter]  -list  |  %s%s->%s
q          -quit                          | L -creates labpit.log (takes 20secs)
u [file]   -update   (lab -> pit)
d [file]   -download (lab <- pit)   or    D file   -download to /tmp in lab
>"""%(lab.node, isvd, pit.node)
    line=raw_input()
    if len(line)<1:
      print "l, q, u, d or D expected to be 1st letter in your answer"
    elif line[0]=='l':
      ldir= getpar(line,1); filter= getpar(line,2)
      showdirfil(ldir, filter, lab, pit, isvd)
    elif line[0]=='L':
      log='labpit.log' ; l2ptar='l2ptar.bash'
      print "Creating l2ptar.bash and labpit.log in home dir"
      saveout= sys.stdout
      fsock = open(log, 'w')
      sys.stdout = fsock ; tarshf= open(l2ptar,'w')
      tarshf.write("""#!/bin/bash
vd='.'
cd $VMECFDIR/..
tar -czvf ~/l2ptar.tgz -T - <<-EOF
$vd/bin/l2ptar.bash
""")
      for line in ALLDIRS:
        ldir= getpar(line,0); filter= getpar(line,1)
        #time.sleep(1)
        showdirfil(ldir, filter, lab, pit, isvd, tarshf)
      sys.stdout= saveout ; 
      tarshf.write("EOF\n"); tarshf.close()
      fsock.close()
    elif line[0]=='u':
      fname= getpar(line,1)
      globl2p.update(fname)
    elif line[0]=='d' or line[0]=='D':
      fname= getpar(line,1)
      globl2p.download(fname,line[0])
    elif line[0]=='q':break
    else:
      print ">%s<"%line
if __name__ == "__main__":
    main()

